# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Oracle for the "modellight" family of material pixel conformance.

The harness (unittests/shaderextensiontest/material_pixel_modellight.cpp) lights
VertexLitGeneric quads as studiorender lights models: an ambient cube, up to
four local lights and a static-prop color mesh. Each quad's normal attribute
points in its own direction. View and projection are identity; most cases draw
with an identity MODEL matrix, so a vertex's world position is its clip-space
position, and two place the model by a MODEL matrix or by skinning bones. This
module judges the frames:

* an independent evaluation of the D3D9 lighting, from the inputs the capture
  records. Positions and normals go through the case's model matrix (the MODEL
  matrix, or bones that all hold it), normals without inverse-transposing, as
  SkinPositionAndNormal does, and are renormalized. The light constants are
  built exactly as CShaderAPIDx8::SetLight and CommitVertexShaderLighting build
  them (lights sorted spot, point, directional), then DoLighting of
  common_vs_fxc.h runs per vertex (static color, local lights, ambient cube),
  interpolated over each triangle at D3D9's pixel centers, times the
  sRGB-decoded base texture and the linear tone-mapping scale, and sRGB-encoded
  (vertexlit_and_unlit_generic_ps2x.fxc). Every pixel well inside a quad must
  match it; pixels well outside must be the clear color;
* the whole frame must agree with the D3D9 reference capture.
"""

import math
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
import material_pixel_frames  # noqa: E402

CASES = ("ambient_cube", "directional", "point", "spot", "four_lights", "half_lambert",
         "static_vertex", "static_and_dynamic", "no_light", "model_transform", "skinned")
CLEAR = (255, 0, 255)
# vertexlit_and_unlit_generic_vs20 gets four lights on ps_2_b hardware.
MAX_LIGHTS = 4
# Pixels closer than this to a quad edge (in pixels) are not judged by the model:
# their coverage depends on rasterization rules rather than lighting.
EDGE_MARGIN = 1.5
# The model evaluates in double precision what the GPU evaluates in single
# precision and 8-bit texels; one level of output rounding either way.
MODEL_TOLERANCE = 1
# Agreement with the reference, and how many pixels may exceed it.
PIXEL_TOLERANCE = 1
MAX_DIFFERING_PIXELS = 0
PINNED_SAMPLING = {"aa_samples": 0}
# D3D9 light type codes (vertexshaderdx8.h); SortLights orders by them.
LIGHT_ORDER = {"spot": 0, "point": 1, "directional": 2}

CaptureError = material_pixel_frames.CaptureError


def validate(path, report):
    material_pixel_frames.validate(path, report, CASES)
    if len(report.get("quads", [])) != 9 or "base_color" not in report:
        raise CaptureError("%s has no quad layout or base color" % path)
    return report


def srgb_to_linear(c):
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4


def linear_to_srgb(c):
    c = min(max(c, 0.0), 1.0)
    return c * 12.92 if c <= 0.0031308 else 1.055 * c ** (1.0 / 2.4) - 0.055


def _normalize(v):
    length = math.sqrt(sum(x * x for x in v))
    return [x / length for x in v]


def _dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def light_constants(light):
    """cLightInfo[i] as CShaderAPIDx8::SetLight and CommitVertexShaderLighting
    build it: color (w: directional), dir (w: spot), pos, spot parameters
    (exponent, stopdot, stopdot2, 1 / (stopdot - stopdot2)) and attenuation."""
    kind = light["type"]
    theta, phi = light["theta"], light["phi"]
    phi = min(phi, math.pi)
    if theta - phi > -1e-3:
        theta = phi - 1e-3
    if kind == "spot":
        stopdot = math.cos(theta * 0.5)
        stopdot2 = math.cos(phi * 0.5)
        oodot = 1.0 / (stopdot - stopdot2) if stopdot > stopdot2 else 0.0
        spot = (light["falloff"], stopdot, stopdot2, oodot)
    else:
        spot = (0.0, 1.0, 1.0, 1.0)
    return {"color": light["color"], "color_w": 1.0 if kind == "directional" else 0.0,
            "dir": light["direction"], "dir_w": 1.0 if kind == "spot" else 0.0,
            "pos": light["position"], "spot": spot, "atten": light["attenuation"]}


def sorted_lights(lights):
    """SortLights: an insertion sort by type code, stable for equal types."""
    order = []
    for light in lights:
        j = len(order)
        while j > 0 and LIGHT_ORDER[order[j - 1]["type"]] > LIGHT_ORDER[light["type"]]:
            j -= 1
        order.insert(j, light)
    return order


def _vertex_atten(c, world_pos):
    light_dir = [p - w for p, w in zip(c["pos"], world_pos)]
    dist_sq = _dot(light_dir, light_dir)
    oo_dist = 1.0 / math.sqrt(dist_sq)
    light_dir = [x * oo_dist for x in light_dir]
    # dst( distSq, ooDist ) = ( 1, dist, distSq, ooDist )
    distance_atten = 1.0 / _dot(c["atten"], (1.0, dist_sq * oo_dist, dist_sq))
    cos_theta = -_dot(c["dir"], light_dir)
    spot = (cos_theta - c["spot"][2]) * c["spot"][3]
    spot = max(0.0001, spot) ** c["spot"][0]
    spot = min(max(spot, 0.0), 1.0)
    atten = distance_atten + (distance_atten * spot - distance_atten) * c["dir_w"]
    return atten + (1.0 - atten) * c["color_w"]


def _cosine_term(c, world_pos, normal, half_lambert):
    light_dir = _normalize([p - w for p, w in zip(c["pos"], world_pos)])
    light_dir = [l + (-d - l) * c["color_w"] for l, d in zip(light_dir, c["dir"])]
    n_dot_l = _dot(normal, light_dir)
    if half_lambert:
        return (n_dot_l * 0.5 + 0.5) ** 2
    return max(0.0, n_dot_l)


def ambient_light(cube, normal):
    """AmbientLight: the cube faces +x, -x, +y, -y, +z, -z weighted by n^2."""
    color = [0.0, 0.0, 0.0]
    for axis in range(3):
        face = cube[2 * axis + (1 if normal[axis] < 0.0 else 0)]
        for k in range(3):
            color[k] += normal[axis] * normal[axis] * face[k]
    return color


def vertex_lighting(case, world_pos, normal, static_color):
    """DoLighting of common_vs_fxc.h, as vertexlit_and_unlit_generic_vs20 selects
    it: STATIC_LIGHT with a color mesh, DYNAMIC_LIGHT with an ambient cube or a
    local light (CShaderAPIDx8::GetDX9LightState). Neither leaves the color 0."""
    normal = _normalize(normal)
    color = [0.0, 0.0, 0.0]
    if case["static_color"]:
        for k in range(3):
            # GammaToLinear( staticLightingColor * cOverbright )
            color[k] += (static_color[k] / 255.0 * 2.0) ** 2.2
    ambient = any(v != 0.0 for face in case["cube"] for v in face)
    if ambient or case["lights"]:
        for light in sorted_lights(case["lights"]):
            c = light_constants(light)
            term = _cosine_term(c, world_pos, normal, case["half_lambert"]) * \
                _vertex_atten(c, world_pos)
            for k in range(3):
                color[k] += c["color"][k] * term
        for k, value in enumerate(ambient_light(case["cube"], normal)):
            color[k] += value
    return color


class CaseModel:
    """The expected color of each pixel of one case."""

    TRIANGLES = ((0, 1, 2), (0, 2, 3))

    def __init__(self, report, case):
        self.width, self.height = report["frame"]
        self.scale = report["tone_scale"]
        self.base = [srgb_to_linear(c / 255.0) for c in report["base_color"]]
        self.quads = []
        matrix = case["model_matrix"]
        for quad in report["quads"]:
            world = [_transform(matrix, (x, y, report["quad_z"]), 1.0)
                     for x, y in quad["corners"]]
            normal = _transform(matrix, quad["normal"], 0.0)
            lighting = [vertex_lighting(case, position, normal, quad["static_colors"][v])
                        for v, position in enumerate(world)]
            self.quads.append(([p[:2] for p in world], lighting))

    def _ndc(self, x, y):
        # D3D9 samples pixel (x, y) at window coordinates (x, y).
        return 2.0 * x / self.width - 1.0, 1.0 - 2.0 * y / self.height

    def _edge_distance_px(self, corners, px, py):
        """Signed distance in pixels from (px, py) to the quad's nearest edge,
        positive inside."""
        xs = [c[0] for c in corners]
        ys = [c[1] for c in corners]
        dx = min(px - min(xs), max(xs) - px) * self.width / 2.0
        dy = min(py - min(ys), max(ys) - py) * self.height / 2.0
        return min(dx, dy)

    def expected(self, x, y):
        """The expected sRGB color, CLEAR, or None where only the reference can
        judge (the pixel lies near a quad edge)."""
        px, py = self._ndc(x, y)
        for corners, lighting in self.quads:
            distance = self._edge_distance_px(corners, px, py)
            if distance < -EDGE_MARGIN:
                continue
            if distance < EDGE_MARGIN:
                return None
            for a, b, c in self.TRIANGLES:
                weights = _barycentric(corners[a], corners[b], corners[c], (px, py))
                if weights is None:
                    continue
                light = [sum(w * lighting[v][k] for w, v in zip(weights, (a, b, c)))
                         for k in range(3)]
                return tuple(round(linear_to_srgb(self.base[k] * light[k] * self.scale) * 255.0)
                             for k in range(3))
            return None
        return CLEAR


def _transform(matrix, vector, w):
    """A 3x4 column-vector matrix applied to a point (w 1) or a direction (w 0)."""
    return [sum(row[k] * vector[k] for k in range(3)) + row[3] * w for row in matrix]


def _barycentric(a, b, c, p):
    det = (b[1] - c[1]) * (a[0] - c[0]) + (c[0] - b[0]) * (a[1] - c[1])
    wa = ((b[1] - c[1]) * (p[0] - c[0]) + (c[0] - b[0]) * (p[1] - c[1])) / det
    wb = ((c[1] - a[1]) * (p[0] - c[0]) + (a[0] - c[0]) * (p[1] - c[1])) / det
    wc = 1.0 - wa - wb
    if min(wa, wb, wc) < -1e-9:
        return None
    return wa, wb, wc


def check_capture(report):
    failures = []
    unpinned = sorted(key for key, value in PINNED_SAMPLING.items() if report.get(key) != value)
    if unpinned:
        failures.append("sampling configuration not pinned (%s)"
                        % ", ".join("%s=%r" % (key, report.get(key)) for key in unpinned))
    if report.get("max_lights", 0) < MAX_LIGHTS:
        failures.append("backend reports %s local lights; vertexlit_and_unlit_generic lights "
                        "models with %d" % (report.get("max_lights"), MAX_LIGHTS))
    return failures


def check_model(report):
    """Every pixel the model decides shows the evaluated lighting."""
    failures = []
    width, height = report["frame"]
    for case in report["cases"]:
        model = CaseModel(report, case)
        rgb = material_pixel_frames.decode_frame(case, report["frame"])
        wrong = []
        judged = 0
        for y in range(height):
            for x in range(width):
                expected = model.expected(x, y)
                if expected is None:
                    continue
                judged += 1
                pixel = rgb[(y * width + x) * 3:(y * width + x) * 3 + 3]
                if any(abs(p - e) > MODEL_TOLERANCE for p, e in zip(pixel, expected)):
                    wrong.append((x, y, list(pixel), list(expected)))
        if wrong:
            x, y, pixel, expected = wrong[0]
            worst = max(max(abs(p - e) for p, e in zip(w[2], w[3])) for w in wrong)
            failures.append("%s: %d pixels disagree with the lighting model (worst %d); "
                            "first at (%d, %d): %s, expected %s"
                            % (case["name"], len(wrong), worst, x, y, pixel, expected))
        if judged == 0:
            failures.append("%s: the model judged no pixel" % case["name"])
    return failures


def compare(report, reference):
    if report["frame"] != reference["frame"]:
        return ["frame %s, reference %s; recapture at the same drawable size"
                % (report["frame"], reference["frame"])]
    inputs = ("tone_scale", "base_color", "quad_z", "quads")
    if any(report.get(key) != reference.get(key) for key in inputs) or \
            [{k: v for k, v in c.items() if k != "frame"} for c in report["cases"]] != \
            [{k: v for k, v in c.items() if k != "frame"} for c in reference["cases"]]:
        return ["modellight inputs differ from the reference capture"]
    failures = []
    width = report["frame"][0]
    for case, ref in zip(report["cases"], reference["cases"]):
        rgb = material_pixel_frames.decode_frame(case, report["frame"])
        expected = material_pixel_frames.decode_frame(ref, reference["frame"])
        differing = []
        worst = 0
        for index in range(0, len(rgb), 3):
            delta = max(abs(rgb[index + k] - expected[index + k]) for k in range(3))
            if delta > PIXEL_TOLERANCE:
                differing.append(index // 3)
                worst = max(worst, delta)
        if len(differing) > MAX_DIFFERING_PIXELS:
            first = differing[0]
            failures.append("%s: %d pixels differ from the reference by more than %d levels "
                            "(worst %d); first at (%d, %d): %s, reference %s"
                            % (case["name"], len(differing), PIXEL_TOLERANCE, worst,
                               first % width, first // width,
                               list(rgb[first * 3:first * 3 + 3]),
                               list(expected[first * 3:first * 3 + 3])))
    return failures


def evaluate(report, reference=None):
    failures = check_capture(report)
    if failures:
        return failures
    failures += check_model(report)
    if reference is not None:
        failures += compare(report, reference)
    return failures
