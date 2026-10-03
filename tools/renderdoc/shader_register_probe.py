"""Measure one integer specialization's register impact in captured pipelines.

Run through rdc.py script, with arguments OUT SPEC_ID VALUE EVENTS. EVENTS are
comma-separated integers. VALUE is an integer or 'original'. Run a fresh replay
for each value: RenderDoc caches executable disassembly by original pipeline ID
and can otherwise return stale statistics after a shader replacement.
Values 0, 4, 4096, 4100 at SpecId 102 isolate the core debug area's/shadow's
compile-time branches. Zero is the unchanged-output calibration, not a speedup.

Uses the captured SPIR-V, never the working tree's shader source. Only the named
specialization becomes an ordinary constant; all other pipeline specializations
remain captured values. Requires spirv-dis, spirv-as and spirv-val on PATH.
Disabling effects is diagnostic only. No frame timings or product changes.
"""

import hashlib
import json
import pathlib
import re
import subprocess


def freeze_integer(assembly, spec_id, value):
    """Freeze exactly one 32-bit integer, refusing absent/ambiguous targets."""
    decoration = re.compile(
        rf"^\s*OpDecorate\s+(%\w+)\s+SpecId\s+{spec_id}\s*$", re.M)
    targets = decoration.findall(assembly)
    if len(targets) != 1:
        raise ValueError(f"expected one SpecId {spec_id}, found {len(targets)}")
    target = re.escape(targets[0])
    declaration = re.compile(
        rf"^(\s*{target}\s*=\s*)OpSpecConstant\s+(%\w+)\s+(-?\d+)\s*$", re.M)
    constants = declaration.findall(assembly)
    if len(constants) != 1:
        raise ValueError("specialization must be a scalar integer constant")
    type_id = re.escape(constants[0][1])
    integer = re.search(rf"^\s*{type_id}\s*=\s*OpTypeInt\s+32\s+([01])\s*$",
                        assembly, re.M)
    if not integer:
        raise ValueError("specialization must be a 32-bit integer")
    minimum, maximum = (-(1 << 31), (1 << 31) - 1) if integer[1] == "1" else (0, (1 << 32) - 1)
    if not minimum <= value <= maximum:
        raise ValueError("specialization value is out of range")
    frozen = declaration.sub(lambda m: f"{m[1]}OpConstant {m[2]} {value}", assembly)
    return decoration.sub("", frozen)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def measure(replay, controller, rd, arguments):
    if len(arguments) != 4:
        raise ValueError("expected OUT SPEC_ID VALUE EVENTS")
    output = pathlib.Path(arguments[0]).resolve()
    output.mkdir(parents=True, exist_ok=True)
    spec_id = int(arguments[1])
    value = None if arguments[2] == "original" else int(arguments[2], 0)
    events = [int(event) for event in arguments[3].split(",")]
    if not events:
        raise ValueError("need events")
    rows = []
    sources = {}
    target = None
    for event in events:
        controller.SetFrameEvent(event, False)
        state = controller.GetPipelineState()
        shader = state.GetShader(rd.ShaderStage.Pixel)
        pipeline = state.GetGraphicsPipelineObject()
        reflection = state.GetShaderReflection(rd.ShaderStage.Pixel)
        if not reflection or not reflection.rawBytes:
            raise ValueError(f"missing fragment SPIR-V at event {event}")
        source = bytes(reflection.rawBytes)
        digest = sha256(source)
        if digest not in sources:
            path = output / f"original-{digest}.spv"
            path.write_bytes(source)
            assembly = subprocess.check_output(["spirv-dis", str(path)], text=True)
            sources[digest] = {"bytes": source, "assembly": assembly}
        targets = state.GetOutputTargets()
        if not targets or not int(targets[0].resource):
            raise ValueError(f"missing color attachment at event {event}")
        if target is not None and target != targets[0].resource:
            raise ValueError("selected pipelines must share their first color attachment")
        target = targets[0].resource
        rows.append({"event": event, "pipeline": int(pipeline), "shader": int(shader),
                     "original_sha256": digest})
    texture = next(item for item in controller.GetTextures() if item.resourceId == target)

    def images():
        controller.SetFrameEvent(replay.last_draw(), True)
        samples = []
        for index in range(texture.msSamp):
            subresource = rd.Subresource()
            subresource.sample = index
            data = bytes(controller.GetTextureData(target, subresource))
            if not data:
                raise ValueError("empty attachment readback")
            samples.append({"sample": index, "bytes": len(data), "sha256": sha256(data)})
        return samples

    report = {"specialization_id": spec_id, "pipelines": rows,
              "target": {"resource": int(target), "width": texture.width,
                         "height": texture.height, "samples": texture.msSamp},
              "tools": {tool: subprocess.check_output([tool, "--version"], text=True).strip()
                        for tool in ("spirv-dis", "spirv-as", "spirv-val")}, "variants": []}
    label = "original" if value is None else f"value-{value}"
    shaders = {}
    replacements = set()
    variant = {"value": value, "sources": {}, "pipelines": []}
    try:
        if value is not None:
            for digest, source in sources.items():
                assembly_path = output / f"{label}-{digest}.spvasm"
                binary_path = assembly_path.with_suffix(".spv")
                assembly_path.write_text(freeze_integer(source["assembly"], spec_id, value))
                subprocess.run(["spirv-as", "--target-env", "vulkan1.1", "--preserve-numeric-ids",
                                str(assembly_path), "-o", str(binary_path)], check=True)
                subprocess.run(["spirv-val", "--target-env", "vulkan1.1", str(binary_path)], check=True)
                data = binary_path.read_bytes()
                shader, errors = controller.BuildTargetShader(
                    "main", rd.ShaderEncoding.SPIRV, data, rd.ShaderCompileFlags(), rd.ShaderStage.Pixel)
                if shader == rd.ResourceId.Null():
                    raise RuntimeError(errors)
                shaders[digest] = shader
                variant["sources"][digest] = {"sha256": sha256(data), "messages": errors}
            for row in rows:
                if row["shader"] not in replacements:
                    controller.ReplaceResource(replay.ids[row["shader"]], shaders[row["original_sha256"]])
                    replacements.add(row["shader"])
        variant["images"] = images()
        for row in rows:
            controller.SetFrameEvent(row["event"], False)
            state = controller.GetPipelineState()
            pipeline = state.GetGraphicsPipelineObject()
            # Reflection must identify the replacement, not the original module.
            reflection = (state.GetShaderReflection(rd.ShaderStage.Pixel) if value is None else
                          controller.GetShader(pipeline, shaders[row["original_sha256"]],
                                               rd.ShaderEntryPoint("main", rd.ShaderStage.Pixel)))
            assembly = controller.DisassembleShader(pipeline, reflection, "KHR_pipeline_executable_properties")
            if not assembly.startswith("======== fragment"):
                raise RuntimeError("driver did not return fragment executable statistics")
            path = output / f"{label}-pipeline-{row['pipeline']}.txt"
            path.write_text(assembly)
            header = assembly.split("==== Internal Representations ====")[0]
            stats = {match[1]: int(match[2]) for match in
                     re.finditer(r"^([^:\n]+): (\d+)\s+//", header, re.M)}
            for required in ("VGPRs", "Subgroup Size", "Subgroups per SIMD", "Scratch size"):
                if required not in stats:
                    raise RuntimeError(f"missing {required}")
            variant["pipelines"].append({"pipeline": row["pipeline"], "stats": stats,
                                         "assembly": str(path)})
        report["variants"].append(variant)
        (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    finally:
        for shader in replacements:
            controller.RemoveReplacement(replay.ids[shader])
        for shader in shaders.values():
            controller.FreeTargetResource(shader)
    return report


if "controller" in globals():
    result = measure(replay, controller, rd, args)
