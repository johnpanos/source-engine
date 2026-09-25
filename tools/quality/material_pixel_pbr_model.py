# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Oracle for the "pbr-model" family of material pixel conformance.

The harness (unittests/shaderextensiontest/material_pixel_modellight.cpp,
RunPbrModelCases) draws the modellight family's quads, ambient cubes, local
lights and placements with RFC 0007 PBRMetalRough materials. Native Vulkan draws
them through shaders/model_pbr.frag. D3D9 has no PBR shader (it draws the
material's $fallbackmaterial), so this oracle is the only judge:

* the per-vertex inputs are skin_vs20's, evaluated as the modellight oracle
  does: world position through the case's placement, each light's attenuation
  (common_vs_fxc.h GetVertexAttenForLight, lights sorted spot, point,
  directional), the world tangent frame and the eye vector; they are
  interpolated over each triangle at the pixel centers;
* per pixel, the layered metal/roughness BRDF of public/render/pbr_brdf.h:
  GGX distribution with alpha = roughness^2, height-correlated Smith visibility,
  Schlick Fresnel from F0 = lerp( 0.04, base, metalness ), the multiple-
  scattering energy compensation 1 + F0 (1 / (A + B) - 1), and a Lambertian
  layer weighted by 1 - the compensated split-sum directional albedo (the
  table in public/render/pbr_split_sum_table.h, sampled bilinearly as the GPU
  samples it). Source's model light units make a local light incident radiance
  pi * color * attenuation, and the ambient cube a Lambertian return; with no
  probe resident the cube in the reflected direction is the specular image
  light. $emissiontexture adds its sRGB-decoded color times $emissionscale.

Every pixel well inside a quad must match within PBR_MODEL_TOLERANCE levels;
pixels well outside must be the clear color. The seeded controls in
tools/quality/tests show that the oracle rejects plausible wrong shaders.
"""

import math
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
import material_pixel_frames  # noqa: E402
import material_pixel_modellight as modellight  # noqa: E402

CASES = ("pbr_ambient", "pbr_lights", "pbr_four_lights", "pbr_metal", "pbr_bumped",
         "pbr_emissive", "pbr_model_transform", "pbr_skinned")
# Double-precision model against single-precision shading, 8-bit textures and
# the GPU's fixed-point bilinear weights in the split-sum lookup.
PBR_MODEL_TOLERANCE = 2
# Pixels whose shading normal is this close to perpendicular to the view are not
# judged: at N.V = 0 the shader and the model both switch the specular term and
# the layered albedo, so a sub-pixel difference in where the pixel is sampled
# decides the side (as EDGE_MARGIN does for coverage at quad edges).
HORIZON_MARGIN = 0.04
SPLIT_SUM_HEADER = (Path(__file__).resolve().parents[2] /
                    "public/render/pbr_split_sum_table.h")

CaptureError = material_pixel_frames.CaptureError


def validate(path, report):
    material_pixel_frames.validate(path, report, CASES)
    if len(report.get("quads", [])) != 9:
        raise CaptureError("%s has no quad layout" % path)
    return report


_SPLIT_SUM = None


def split_sum_table():
    """The generated table, as the engine and the GPU texture hold it."""
    global _SPLIT_SUM
    if _SPLIT_SUM is None:
        text = SPLIT_SUM_HEADER.read_text()
        size = int(re.search(r"kSplitSumSize = (\d+);", text).group(1))
        body = text[text.index("kSplitSumTable"):]
        pairs = re.findall(r"\{\s*([-0-9.e+]+)f,\s*([-0-9.e+]+)f\s*\}", body)
        if len(pairs) != size * size:
            raise CaptureError("split-sum table holds %d entries, expected %d"
                               % (len(pairs), size * size))
        _SPLIT_SUM = (size, [(float(a), float(b)) for a, b in pairs])
    return _SPLIT_SUM


def sample_split_sum(n_dot_v, roughness):
    """render::pbr::SampleSplitSum: bilinear over texels at i / (size - 1)."""
    size, table = split_sum_table()
    x = min(max(n_dot_v, 0.0), 1.0) * (size - 1)
    y = min(max(roughness, 0.0), 1.0) * (size - 1)
    x0 = max(0, min(int(math.floor(x)), size - 1))
    y0 = max(0, min(int(math.floor(y)), size - 1))
    x1, y1 = min(x0 + 1, size - 1), min(y0 + 1, size - 1)
    fx = min(max(x - x0, 0.0), 1.0)
    fy = min(max(y - y0, 0.0), 1.0)
    result = []
    for k in range(2):
        bottom = modellight._lerp(table[y0 * size + x0][k], table[y0 * size + x1][k], fx)
        top = modellight._lerp(table[y1 * size + x0][k], table[y1 * size + x1][k], fx)
        result.append(modellight._lerp(bottom, top, fy))
    return result


class PbrMaterial:
    """PBRMetalRough's textures and scalars from the capture."""

    def __init__(self, report, params):
        texel = modellight._texel
        self.base = [modellight.srgb_to_linear(c) for c in texel(report, params["$basetexture"])[:3]]
        self.alpha = texel(report, params["$basetexture"])[3]
        self.mrao = texel(report, params["$mraotexture"])[:3]
        self.normal = texel(report, params["$bumpmap"]) if "$bumpmap" in params else None
        self.emission = ([modellight.srgb_to_linear(c)
                          for c in texel(report, params["$emissiontexture"])[:3]]
                         if "$emissiontexture" in params else None)
        self.emission_scale = float(params.get("$emissionscale", "1"))


def pbr_pixel(case, report, material, v, scale, defect=None):
    """model_pbr.frag for one pixel's interpolated inputs; the linear color.
    `defect` seeds a named mistake (for the controls)."""
    cube = case["cube"]
    origin = report["lighting_origin"]
    lights = []
    for light in modellight.sorted_lights(case["lights"]):
        if light["type"] == "directional":
            position = [o - d * modellight.DIRECTIONAL_DISTANCE
                        for o, d in zip(origin, light["direction"])]
        else:
            position = light["position"]
        lights.append((light["color"], position))

    metalness = min(max(material.mrao[0], 0.0), 1.0)
    roughness = max(material.mrao[1], 0.02)
    occlusion = min(max(material.mrao[2], 0.0), 1.0)
    if defect == "mrao_srgb":
        metalness = modellight.srgb_to_linear(metalness)
        roughness = max(modellight.srgb_to_linear(material.mrao[1]), 0.02)
    if defect == "no_occlusion":
        occlusion = 1.0
    alpha_sq = (roughness * roughness) ** 2
    n = modellight._normalize(v["n"])
    if material.normal is not None and defect != "no_normal_map":
        x, y = material.normal[0] * 2.0 - 1.0, material.normal[1] * 2.0 - 1.0
        z = math.sqrt(max(0.0, 1.0 - x * x - y * y))
        s, t = modellight._normalize(v["s"]), modellight._normalize(v["t"])
        n = modellight._normalize([s[k] * x + t[k] * y + n[k] * z for k in range(3)])
    view = modellight._normalize(v["eye"])
    n_dot_v = max(modellight._dot(n, view), 0.0)
    f0 = [modellight._lerp(0.04, b, metalness) for b in material.base]
    a, b = sample_split_sum(n_dot_v, roughness)
    # Multiple-scattering energy compensation (render::pbr::
    # SpecularEnergyCompensation): 1 + F0 (1 / (A + B) - 1).
    compensation = [1.0 + f * (1.0 / max(a + b, 1e-4) - 1.0) for f in f0]
    albedo = [min(1.0, (f0[k] * a + b) * compensation[k]) for k in range(3)]
    if defect == "no_diffuse_layering":
        albedo_diffuse = [0.0] * 3
    else:
        albedo_diffuse = albedo
    diffuse = [material.base[k] * (1.0 - metalness) * (1.0 - albedo_diffuse[k]) for k in range(3)]
    ambient = modellight._ambient_ps(cube, n)
    color = [diffuse[k] * ambient[k] * occlusion for k in range(3)]
    for i, (light_color, position) in enumerate(lights):
        light = modellight._normalize([p - w for p, w in zip(position, v["pos"])])
        n_dot_l = max(modellight._dot(n, light), 0.0)
        if n_dot_l <= 0.0:
            continue
        atten = 1.0 if defect == "no_attenuation" else v["atten"][i]
        incident = [c * atten for c in light_color]
        lit = [diffuse[k] * incident[k] * n_dot_l for k in range(3)]
        if n_dot_v > 0.0:
            half = modellight._normalize([view[k] + light[k] for k in range(3)])
            n_dot_h = max(modellight._dot(n, half), 0.0)
            v_dot_h = max(modellight._dot(view, half), 0.0)
            denominator = n_dot_h * n_dot_h * (alpha_sq - 1.0) + 1.0
            distribution = alpha_sq / (math.pi * denominator * denominator)
            lambda_v = math.sqrt(alpha_sq + (1.0 - alpha_sq) * n_dot_v * n_dot_v)
            lambda_l = math.sqrt(alpha_sq + (1.0 - alpha_sq) * n_dot_l * n_dot_l)
            visibility = 0.5 / (n_dot_v * lambda_l + n_dot_l * lambda_v)
            grazing = (1.0 - v_dot_h) ** 5
            radiance_scale = 1.0 if defect == "specular_without_pi" else math.pi
            for k in range(3):
                fresnel = f0[k] + (1.0 - f0[k]) * grazing
                lit[k] += radiance_scale * incident[k] * fresnel * distribution * \
                    visibility * compensation[k] * n_dot_l
        color = [color[k] + lit[k] for k in range(3)]
    reflected = [2.0 * modellight._dot(n, view) * n[k] - view[k] for k in range(3)]
    environment = modellight._ambient_ps(cube, modellight._normalize(reflected))
    color = [color[k] + environment[k] * albedo[k] * occlusion for k in range(3)]
    if material.emission is not None:
        emission = material.emission
        if defect == "emission_bytes":
            emission = [modellight.linear_to_srgb(c) for c in emission]
        color = [color[k] + emission[k] * material.emission_scale for k in range(3)]
    return [c * scale for c in color]


def shading_n_dot_v(material, v):
    """N.V of the pixel's shading normal (the normal map applied), unclamped."""
    n = modellight._normalize(v["n"])
    if material.normal is not None:
        x, y = material.normal[0] * 2.0 - 1.0, material.normal[1] * 2.0 - 1.0
        z = math.sqrt(max(0.0, 1.0 - x * x - y * y))
        s, t = modellight._normalize(v["s"]), modellight._normalize(v["t"])
        n = modellight._normalize([s[k] * x + t[k] * y + n[k] * z for k in range(3)])
    return modellight._dot(n, modellight._normalize(v["eye"]))


class PbrCaseModel(modellight.CaseModel):
    """The expected color of each pixel of one pbr-model case."""

    def __init__(self, report, case, defect=None):
        self.width, self.height = report["frame"]
        self.scale = report["tone_scale"]
        self.case = case
        self.report = report
        self.defect = defect
        params = report.get("materials", {}).get(case.get("material"), {})
        self.material = PbrMaterial(report, params)
        self.quads = []
        matrix = case["model_matrix"]
        for quad in report["quads"]:
            world = [modellight._transform(matrix, (x, y, report["quad_z"]), 1.0)
                     for x, y in quad["corners"]]
            normal = modellight._transform(matrix, quad["normal"], 0.0)
            tangent = modellight._transform(matrix, quad["tangent"][:3], 0.0) + \
                [quad["tangent"][3]]
            inputs = [modellight._phong_vertex(case, report, position, normal, tangent)
                      for position in world]
            self.quads.append(([p[:2] for p in world], inputs))

    def expected(self, x, y):
        px, py = self._ndc(x, y)
        for corners, inputs in self.quads:
            distance = self._edge_distance_px(corners, px, py)
            if distance < -modellight.EDGE_MARGIN:
                continue
            if distance < modellight.EDGE_MARGIN:
                return None
            for a, b, c in self.TRIANGLES:
                weights = modellight._barycentric(corners[a], corners[b], corners[c], (px, py))
                if weights is None:
                    continue
                interpolated = {key: [sum(w * inputs[v][key][k]
                                          for w, v in zip(weights, (a, b, c)))
                                      for k in range(len(inputs[a][key]))]
                                for key in inputs[a]}
                if abs(shading_n_dot_v(self.material, interpolated)) < HORIZON_MARGIN:
                    return None
                color = pbr_pixel(self.case, self.report, self.material, interpolated,
                                  self.scale, self.defect)
                return tuple(round(modellight.linear_to_srgb(x) * 255.0) for x in color)
            return None
        return modellight.CLEAR


def disagreements(report, case, defect=None, stride=1):
    """(pixels judged, pixels outside tolerance, worst level difference, first)."""
    model = PbrCaseModel(report, case, defect)
    width, height = report["frame"]
    rgb = material_pixel_frames.decode_frame(case, report["frame"])
    judged = 0
    wrong = []
    worst = 0
    for y in range(0, height, stride):
        for x in range(0, width, stride):
            expected = model.expected(x, y)
            if expected is None:
                continue
            judged += 1
            pixel = rgb[(y * width + x) * 3:(y * width + x) * 3 + 3]
            delta = max(abs(p - e) for p, e in zip(pixel, expected))
            if delta > PBR_MODEL_TOLERANCE:
                wrong.append((x, y, list(pixel), list(expected)))
                worst = max(worst, delta)
    return judged, wrong, worst


# Each case with a seeded defect the capture must reject: the defect changes
# the model far beyond the tolerance for that case's inputs.
CONTROLS = {
    "pbr_ambient": ("no_occlusion", "no_diffuse_layering"),
    "pbr_lights": ("no_attenuation",),
    "pbr_metal": ("specular_without_pi", "mrao_srgb"),
    "pbr_bumped": ("no_normal_map",),
    "pbr_emissive": ("emission_bytes",),
}


def check_model(report):
    failures = []
    for case in report["cases"]:
        judged, wrong, worst = disagreements(report, case)
        if judged == 0:
            failures.append("%s: the model judged no pixel" % case["name"])
        if wrong:
            x, y, pixel, expected = wrong[0]
            failures.append("%s: %d pixels disagree with the PBR model (worst %d); first at "
                            "(%d, %d): %s, expected %s"
                            % (case["name"], len(wrong), worst, x, y, pixel, expected))
        for defect in CONTROLS.get(case["name"], ()):
            _, seeded, _ = disagreements(report, case, defect, stride=4)
            if not seeded:
                failures.append("%s: seeded defect %s is not separated from the capture"
                                % (case["name"], defect))
    return failures


def evaluate(report, reference=None):
    failures = modellight.check_capture(report)
    if failures:
        return failures
    failures += check_model(report)
    if reference is not None:
        failures.append("pbr-model has no cross-backend reference (D3D9 draws the fallback)")
    return failures
