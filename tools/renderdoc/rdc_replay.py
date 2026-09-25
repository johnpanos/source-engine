"""RenderDoc replay worker: runs inside qrenderdoc's embedded Python.

`rdc.py` starts `qrenderdoc --python rdc_replay.py` headless with a request
(RDC_REQUEST, JSON: command, capture, arguments) and reads the response
(RDC_RESPONSE, JSON: result or error). Only the standard library and the
`renderdoc` module qrenderdoc embeds are available here. Every command opens
the capture, answers from the replay controller and exits the process; an
exception becomes the response's `error` so the host never waits on a dialog.
"""

import json
import os
import sys
import traceback

import renderdoc as rd

STAGES = {"vertex": rd.ShaderStage.Vertex, "pixel": rd.ShaderStage.Pixel,
          "compute": rd.ShaderStage.Compute, "geometry": rd.ShaderStage.Geometry,
          "hull": rd.ShaderStage.Hull, "domain": rd.ShaderStage.Domain}
SAVE_TYPES = {".png": rd.FileType.PNG, ".exr": rd.FileType.EXR, ".dds": rd.FileType.DDS,
              ".hdr": rd.FileType.HDR, ".jpg": rd.FileType.JPG, ".tga": rd.FileType.TGA}


class Replay:
    """One open capture: the controller plus name and texture lookups."""

    def __init__(self, path):
        self.file = rd.OpenCaptureFile()
        status = self.file.OpenFile(path, "", None)
        if status != rd.ResultCode.Succeeded:
            raise RuntimeError("cannot open %s: %s" % (path, status))
        if not self.file.LocalReplaySupport():
            raise RuntimeError("%s cannot be replayed on this host" % path)
        status, self.controller = self.file.OpenCapture(rd.ReplayOptions(), None)
        if status != rd.ResultCode.Succeeded:
            raise RuntimeError("cannot replay %s: %s" % (path, status))
        self.names = {r.resourceId: r.name for r in self.controller.GetResources()}
        self.textures = {t.resourceId: t for t in self.controller.GetTextures()}
        self.ids = {int(rid): rid for rid in self.names}
        self.structured = self.controller.GetStructuredFile()

    def close(self):
        self.controller.Shutdown()
        self.file.Shutdown()

    def actions(self):
        """Every action in event order, with its marker path."""
        def walk(actions, path):
            for action in actions:
                name = action.GetName(self.structured)
                yield action, path
                yield from walk(action.children, path + [name])
        return list(walk(self.controller.GetRootActions(), []))

    def draws(self):
        return [(a, path) for a, path in self.actions()
                if a.flags & (rd.ActionFlags.Drawcall | rd.ActionFlags.Dispatch)]

    def resource(self, value):
        """A resource by integer id (as printed) or exact name."""
        if isinstance(value, int) or str(value).isdigit():
            rid = self.ids.get(int(value))
        else:
            rid = next((r for r, n in self.names.items() if n == value), None)
        if rid is None:
            raise ValueError("no resource %r in the capture" % value)
        return rid

    def texture_info(self, rid):
        texture = self.textures.get(rid)
        info = {"id": int(rid), "name": self.names.get(rid, str(rid))}
        if texture is not None:
            info.update(width=texture.width, height=texture.height, depth=texture.depth,
                        array=texture.arraysize, mips=texture.mips, cube=bool(texture.cubemap),
                        format=texture.format.Name(), samples=texture.msSamp)
        return info

    def last_draw(self):
        draws = self.draws()
        if not draws:
            raise ValueError("the capture has no draws")
        return draws[-1][0].eventId


def variable_value(var):
    """A ShaderVariable's components as a flat Python list (or members)."""
    if len(var.members):
        return {member.name: variable_value(member) for member in var.members}
    count = max(1, var.rows * var.columns)
    kind = var.type
    if kind in (rd.VarType.Float, rd.VarType.Half):
        return list(var.value.f32v[:count])
    if kind == rd.VarType.Double:
        return list(var.value.f64v[:count])
    if kind in (rd.VarType.SInt, rd.VarType.SShort, rd.VarType.SByte):
        return list(var.value.s32v[:count])
    if kind == rd.VarType.Bool:
        return [bool(v) for v in var.value.u32v[:count]]
    return list(var.value.u32v[:count])


def binding_label(declared):
    """`set.binding`, plus the GLSL name when the SPIR-V kept it (glslc -O
    strips names; RenderDoc then invents resNNNN)."""
    label = "%d.%d" % (declared.fixedBindSetOrSpace, declared.fixedBindNumber)
    if declared.name and not (declared.name.startswith("res") and declared.name[3:].isdigit()):
        label += " " + declared.name
    return label


def pipeline_stage_info(replay, state, stage):
    """Shader identity, bound resources (by reflected set/binding/name) and
    constant blocks of one stage at the current event."""
    reflection = state.GetShaderReflection(stage)
    if reflection is None:
        return None
    info = {"shader": int(state.GetShader(stage)), "entry": state.GetShaderEntryPoint(stage),
            "debug_info": bool(reflection.debugInfo.files), "resources": [], "constants": []}
    if reflection.debugInfo.files:
        info["source_files"] = [f.filename for f in reflection.debugInfo.files]
    for used in state.GetReadOnlyResources(stage):
        access = used.access
        entry = {"index": access.index, "array_element": access.arrayElement}
        if 0 <= access.index < len(reflection.readOnlyResources):
            declared = reflection.readOnlyResources[access.index]
            entry.update(name=declared.name, set=declared.fixedBindSetOrSpace,
                         binding=declared.fixedBindNumber)
        rid = used.descriptor.resource
        entry["resource"] = replay.texture_info(rid) if rid in replay.textures else {
            "id": int(rid), "name": replay.names.get(rid, str(rid))}
        entry["statically_unused"] = bool(access.staticallyUnused)
        info["resources"].append(entry)
    pipeline = state.GetGraphicsPipelineObject() if stage != rd.ShaderStage.Compute \
        else state.GetComputePipelineObject()
    for index, block in enumerate(reflection.constantBlocks):
        used = state.GetConstantBlock(stage, index, 0)
        descriptor = used.descriptor
        variables = replay.controller.GetCBufferVariableContents(
            pipeline, reflection.resourceId, stage, state.GetShaderEntryPoint(stage), index,
            descriptor.resource, descriptor.byteOffset, descriptor.byteSize)
        info["constants"].append({
            "name": block.name, "set": block.fixedBindSetOrSpace,
            "binding": block.fixedBindNumber, "push_constants": not block.bufferBacked,
            "values": {v.name: variable_value(v) for v in variables}})
    return info


def command_info(replay, args):
    frame = replay.controller.GetFrameInfo()
    properties = replay.controller.GetAPIProperties()
    draws = replay.draws()
    return {"api": str(properties.pipelineType), "frame": frame.frameNumber,
            "actions": len(replay.actions()), "draws": len(draws),
            "textures": len(replay.textures), "resources": len(replay.names),
            "last_draw": draws[-1][0].eventId if draws else None}


def command_draws(replay, args):
    rows = []
    stage = STAGES[args.get("stage", "pixel")]
    for action, path in replay.draws():
        replay.controller.SetFrameEvent(action.eventId, True)
        state = replay.controller.GetPipelineState()
        outputs = [replay.texture_info(d.resource) for d in state.GetOutputTargets()
                   if d.resource != rd.ResourceId.Null()]
        row = {"event": action.eventId, "name": action.GetName(replay.structured),
               "markers": path, "indices": action.numIndices, "instances": action.numInstances,
               "outputs": outputs}
        if args.get("bindings"):
            row["stage"] = pipeline_stage_info(replay, state, stage)
        else:
            reflection = state.GetShaderReflection(stage)
            row["shader"] = int(state.GetShader(stage)) if reflection else None
            row["resources"] = [
                binding_label(reflection.readOnlyResources[u.access.index])
                for u in state.GetReadOnlyResources(stage)
                if reflection and 0 <= u.access.index < len(reflection.readOnlyResources)
                and not u.access.staticallyUnused]
        rows.append(row)
    return rows


def command_bindings(replay, args):
    event = args.get("event") or replay.last_draw()
    replay.controller.SetFrameEvent(event, True)
    state = replay.controller.GetPipelineState()
    stages = [args["stage"]] if args.get("stage") else ["vertex", "pixel"]
    return {"event": event, "stages": {name: pipeline_stage_info(replay, state, STAGES[name])
                                       for name in stages}}


def pixel_target(replay, state, args):
    if args.get("texture") is not None:
        return replay.resource(args["texture"])
    targets = [d.resource for d in state.GetOutputTargets() if d.resource != rd.ResourceId.Null()]
    if not targets:
        raise ValueError("the event writes no colour target; pass --texture")
    return targets[args.get("target", 0)]


def modification_value(value):
    return {"color": list(value.col.floatValue), "depth": value.depth,
            "stencil": value.stencil} if value.IsValid() else None


def command_pixel(replay, args):
    event = args.get("event") or replay.last_draw()
    x, y = args["x"], args["y"]
    replay.controller.SetFrameEvent(event, True)
    state = replay.controller.GetPipelineState()
    target = pixel_target(replay, state, args)
    subresource = rd.Subresource(args.get("mip", 0), args.get("slice", 0), 0)
    picked = replay.controller.PickPixel(target, x, y, subresource, rd.CompType.Typeless)
    history = replay.controller.PixelHistory(target, x, y, subresource, rd.CompType.Typeless)
    flags = ("backfaceCulled", "depthClipped", "viewClipped", "scissorClipped",
             "shaderDiscarded", "depthTestFailed", "stencilTestFailed", "depthBoundsFailed",
             "sampleMasked", "predicationSkipped", "unboundPS", "directShaderWrite")
    return {"event": event, "texture": replay.texture_info(target), "x": x, "y": y,
            "value_at_event": list(picked.floatValue),
            "history": [{"event": m.eventId, "primitive": m.primitiveID, "passed": m.Passed(),
                         "flags": [f for f in flags if getattr(m, f)],
                         "shader_out": modification_value(m.shaderOut),
                         "before": modification_value(m.preMod),
                         "after": modification_value(m.postMod)} for m in history]}


def command_debug_pixel(replay, args):
    """Step the pixel shader at (x, y) of `event` and report each source
    variable's last value (SPIR-V debug info names them) or else each
    register's last value."""
    event = args["event"]
    replay.controller.SetFrameEvent(event, True)
    inputs = rd.DebugPixelInputs()
    if args.get("primitive") is not None:
        inputs.primitive = args["primitive"]
    trace = replay.controller.DebugPixel(args["x"], args["y"], inputs)
    if trace is None or trace.debugger is None:
        raise ValueError("no fragment of event %d covers (%d, %d), or its shader cannot be "
                         "debugged" % (event, args["x"], args["y"]))
    registers, executed, steps = {}, [], 0
    for var in list(trace.inputs) + list(trace.constantBlocks):
        registers.setdefault(var.name, var)
    while True:
        states = replay.controller.ContinueDebug(trace.debugger)
        if not states:
            break
        for state in states:
            steps += 1
            executed.append(state.nextInstruction)
            for change in state.changes:
                if change.after.name:
                    registers[change.after.name] = change.after
    wanted = set(args.get("variables") or [])
    source = {}
    info = {inst.instruction: inst for inst in trace.instInfo}
    for instruction in executed:
        entry = info.get(instruction)
        if entry is None:
            continue
        for mapping in entry.sourceVars:
            if wanted and mapping.name not in wanted:
                continue
            values = []
            for reference in mapping.variables:
                var = registers.get(reference.name)
                if var is None:
                    values = None
                    break
                flat = variable_value(var)
                values.append(flat[reference.component] if isinstance(flat, list)
                              and reference.component < len(flat) else flat)
            if values is not None:
                source[mapping.name] = values
    replay.controller.FreeTrace(trace)
    result = {"event": event, "x": args["x"], "y": args["y"], "steps": steps,
              "source_variables": source}
    if not source or args.get("registers"):
        result["registers"] = {name: variable_value(var) for name, var in registers.items()
                               if not wanted or name in wanted}
    return result


def command_save_texture(replay, args):
    event = args.get("event") or replay.last_draw()
    replay.controller.SetFrameEvent(event, True)
    rid = replay.resource(args["texture"])
    save = rd.TextureSave()
    save.resourceId = rid
    out = args["out"]
    extension = os.path.splitext(out)[1].lower()
    if extension not in SAVE_TYPES:
        raise ValueError("unsupported output type %s (use %s)" % (extension, sorted(SAVE_TYPES)))
    save.destType = SAVE_TYPES[extension]
    save.mip = args.get("mip", 0)
    save.slice.sliceIndex = args.get("slice", 0)
    save.alpha = rd.AlphaMapping.Preserve
    status = replay.controller.SaveTexture(save, out)
    if status != rd.ResultCode.Succeeded:
        raise RuntimeError("SaveTexture failed: %s" % status)
    return {"event": event, "texture": replay.texture_info(rid), "out": out}


def command_script(replay, args):
    """Run a user script with `replay` (this module's Replay), `rd`, `args`
    and a `result` dict it fills; `result` is the response."""
    namespace = {"replay": replay, "controller": replay.controller, "rd": rd,
                 "args": args.get("script_args", []), "result": {},
                 "pipeline_stage_info": pipeline_stage_info, "variable_value": variable_value,
                 "STAGES": STAGES}
    path = args["script"]
    with open(path) as handle:
        code = compile(handle.read(), path, "exec")
    exec(code, namespace)
    return namespace["result"]


COMMANDS = {"info": command_info, "draws": command_draws, "bindings": command_bindings,
            "pixel": command_pixel, "debug-pixel": command_debug_pixel,
            "save-texture": command_save_texture, "script": command_script}


def main():
    response_path = os.environ["RDC_RESPONSE"]
    response = {"schema": "rdc-response/v1"}
    try:
        with open(os.environ["RDC_REQUEST"]) as handle:
            request = json.load(handle)
        rd.InitialiseReplay(rd.GlobalEnvironment(), [])
        replay = Replay(request["capture"])
        try:
            response["result"] = COMMANDS[request["command"]](replay, request.get("args", {}))
        finally:
            replay.close()
        rd.ShutdownReplay()
    except Exception:
        response["error"] = traceback.format_exc()
    with open(response_path, "w") as handle:
        json.dump(response, handle, indent=1, default=str)
    sys.stdout.flush()
    # qrenderdoc would otherwise open its main window after the script.
    os._exit(0)


main()
