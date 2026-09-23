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
* $phong materials (skin_vs20 / skin_ps20b) are evaluated per pixel: the
  vertex light attenuation, eye vector and tangent frame interpolated, the
  normal map's normal, PixelShaderDoLighting (always half-Lambert, optionally
  through the lightwarp ramp), phong specular with the Fresnel ranges, the
  exponent map and boost, self-illumination and rim light, all from the
  procedural textures and material parameters the capture records;
* the whole frame must agree with the D3D9 reference capture.
"""

import math
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
import material_pixel_frames  # noqa: E402

CASES = ("ambient_cube", "directional", "point", "spot", "four_lights", "half_lambert",
         "static_vertex", "static_and_dynamic", "no_light", "model_transform", "skinned",
         "phong", "phong_four_lights", "phong_lightwarp", "phong_selfillum", "phong_constant",
         "phong_basealphamask", "phong_skinned")
# CShaderAPIDx8::CommitPixelShaderLighting places a directional light this far
# from the lighting origin, against its direction.
DIRECTIONAL_DISTANCE = 10000.0
# The camera (identity view): CShaderAPIDx8::CacheWorldSpaceCameraPosition.
EYE_POSITION = (0.0, 0.0, 0.0)
CLEAR = (255, 0, 255)
# vertexlit_and_unlit_generic_vs20 gets four lights on ps_2_b hardware.
MAX_LIGHTS = 4
# Pixels closer than this to a quad edge (in pixels) are not judged by the model:
# their coverage depends on rasterization rules rather than lighting.
EDGE_MARGIN = 1.5
# The model evaluates in double precision what the GPU evaluates in single
# precision and 8-bit texels; one level of output rounding either way.
MODEL_TOLERANCE = 1
# Per-pixel phong: D3D9 matches the model within one level except in the peaks
# of the specular highlights, where the exponent amplifies single-precision
# normalization and pow differences to two levels (measured).
PHONG_MODEL_TOLERANCE = 2
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


def _vector(text):
    return [float(x) for x in text.strip("[]").split()]


def _lerp(a, b, t):
    return a + (b - a) * t


def _saturate(x):
    return min(max(x, 0.0), 1.0)


def _texel(report, name):
    """A solid procedural texture's texel, 0..1 per channel."""
    return [c / 255.0 for c in report["textures"][name]["row"][0]]


def _sample_ramp(report, name, u):
    """tex1D on a procedural ramp: bilinear, clamped, texel centers at (i + 0.5) / w."""
    row = report["textures"][name]["row"]
    x = min(max(u * len(row) - 0.5, 0.0), len(row) - 1.0)
    i = int(math.floor(x))
    j = min(i + 1, len(row) - 1)
    f = x - i
    return [_lerp(row[i][k], row[j][k], f) / 255.0 for k in range(3)]


def _ambient_ps(cube, n):
    """PixelShaderAmbientLight."""
    color = [0.0, 0.0, 0.0]
    for axis in range(3):
        face = cube[2 * axis + (0 if n[axis] >= 0.0 else 1)]
        for k in range(3):
            color[k] += n[axis] * n[axis] * face[k]
    return color


def wants_skin_shader(params):
    """vertexlitgeneric_dx9_helper.cpp WantsSkinShader: $phong with a lightwarp,
    a bump map, or $basemapalphaphongmask 1; otherwise $phong is turned off."""
    if params.get("$phong") != "1":
        return False
    if "$lightwarptexture" in params:
        return True
    return params.get("$basemapalphaphongmask") == "1" or "$bumpmap" in params


class PhongMaterial:
    """skin_dx9_helper.cpp's constants and combos for a $phong material. After
    InitShaderParams, CShaderSystem::InitShaderParameters sets every numeric
    parameter the material leaves out to zero, so at draw time the helper sees
    them all defined: an unset $phongboost or $phongfresnelranges is 0 (no
    specular), not the helper's fallback. InitParams defaults (such as
    $selfillumtint) come first and survive."""

    def __init__(self, report, params):
        self.params = params
        self.bump = "$bumpmap" in params
        self.exponent_texture = params.get("$phongexponenttexture")
        self.light_warp = params.get("$lightwarptexture")
        self.self_illum = params.get("$selfillum") == "1"
        self.rim = params.get("$rimlight") == "1"
        self.base = _texel(report, params["$basetexture"])
        self.normal = _texel(report, params["$bumpmap"]) if self.bump else None
        # TEXTURE_WHITE when no exponent map is bound.
        self.exponent_map = (_texel(report, self.exponent_texture) if self.exponent_texture
                             else [1.0, 1.0, 1.0, 1.0])
        # FASTPATH_NOBUMP: no bump, exponent map, tint map, warps, rim, detail,
        # self-illum or tint blend, and an opaque base.
        self.base_alpha_phong_mask = 1.0 if params.get("$basemapalphaphongmask") == "1" else 0.0
        self.fast_path = (not self.bump and not self.exponent_texture and not self.rim and
                          not self.self_illum and
                          (self.base_alpha_phong_mask > 0.0 or self.base[3] >= 1.0))
        exponent = float(params.get("$phongexponent", "0"))
        self.constant_exponent = exponent if exponent > 0.0 else -1.0
        tint = _vector(params.get("$phongtint", "[0 0 0]"))
        # All zero: white (no $phongalbedotint map in these materials).
        self.tint = tint if any(tint) else [1.0, 1.0, 1.0]
        low, mid, high = _vector(params.get("$phongfresnelranges", "[0 0 0]"))
        self.fresnel = ((mid - low) * 2.0, mid, (high - mid) * 2.0)
        self.boost = float(params.get("$phongboost", "0"))
        self.self_illum_tint = (_vector(params["$selfillumtint"])
                                if "$selfillumtint" in params else [1.0, 1.0, 1.0])
        self.rim_exponent = max(float(params.get("$rimlightexponent", "0")), 1.0)
        self.rim_boost = float(params.get("$rimlightboost", "0"))
        self.rim_mask_control = float(params.get("$rimmask", "0"))


def _fresnel_ranges(n, e, ranges):
    f = _saturate(1.0 - _dot(n, e))
    f = f * f - 0.5
    return ranges[1] + (ranges[2] if f >= 0.0 else ranges[0]) * f


def _phong_vertex(case, report, world_pos, normal, tangent):
    """skin_vs20: the vertex's attenuation of each light (in SortLights order),
    and its world tangent frame."""
    lights = [light_constants(light) for light in sorted_lights(case["lights"])]
    atten = [_vertex_atten(c, world_pos) for c in lights]
    atten += [0.0] * (MAX_LIGHTS - len(atten))
    tangent_t = [(normal[1] * tangent[2] - normal[2] * tangent[1]) * tangent[3],
                 (normal[2] * tangent[0] - normal[0] * tangent[2]) * tangent[3],
                 (normal[0] * tangent[1] - normal[1] * tangent[0]) * tangent[3]]
    eye = [e - w for e, w in zip(EYE_POSITION, world_pos)]
    return {"pos": list(world_pos), "atten": atten, "n": _normalize(normal),
            "s": _normalize(tangent[:3]), "t": _normalize(tangent_t), "eye": eye}


def _phong_pixel(case, report, material, v, scale):
    """skin_ps20b for one pixel's interpolated inputs; the linear color."""
    cube = case["cube"]
    ambient = any(value != 0.0 for face in cube for value in face)
    ps_cube = cube if ambient else [[0.0] * 3] * 6
    lights = sorted_lights(case["lights"])
    origin = report["lighting_origin"]
    ps_lights = []
    for light in lights:
        if light["type"] == "directional":
            position = [o - d * DIRECTIONAL_DISTANCE for o, d in zip(origin, light["direction"])]
        else:
            position = light["position"]
        ps_lights.append((light["color"], position))

    base = [srgb_to_linear(c) for c in material.base[:3]]
    base_alpha = material.base[3]
    eye = _normalize(v["eye"])
    if material.fast_path:
        ts_normal = (0.0, 0.0, 1.0)
        spec_mask = base_alpha
    else:
        # TEXTURE_NORMALMAP_FLAT without a bump map.
        texel = material.normal if material.bump else [0.5, 0.5, 1.0, 1.0]
        ts_normal = [_lerp(2.0 * texel[k] - 1.0, (0.0, 0.0, 1.0)[k],
                           material.base_alpha_phong_mask) for k in range(3)]
        spec_mask = _lerp(texel[3], base_alpha, material.base_alpha_phong_mask)
    n = _normalize([v["s"][k] * ts_normal[0] + v["t"][k] * ts_normal[1] + v["n"][k] * ts_normal[2]
                    for k in range(3)])
    fresnel = _fresnel_ranges(n, eye, material.fresnel)
    rim_fresnel = _saturate(1.0 - _dot(n, eye)) ** 4

    # PixelShaderDoLighting( ..., bHalfLambert = true, lightwarp )
    diffuse = _ambient_ps(ps_cube, n)
    for i, (color, position) in enumerate(ps_lights):
        light_dir = _normalize([p - w for p, w in zip(position, v["pos"])])
        term = _saturate(_dot(n, light_dir) * 0.5 + 0.5)
        if material.light_warp:
            warp = [2.0 * c for c in _sample_ramp(report, material.light_warp, term)]
        else:
            warp = [term * term] * 3
        for k in range(3):
            diffuse[k] += color[k] * v["atten"][i] * warp[k]

    exponent_map = material.exponent_map
    if material.fast_path:
        spec_exponent = max(material.constant_exponent, 0.0)
        rim_mask = 0.0
        spec_tint = [1.0, 1.0, 1.0]
    else:
        rim_mask = _lerp(1.0, exponent_map[3], material.rim_mask_control)
        spec_exponent = (material.constant_exponent if material.constant_exponent >= 0.0
                         else 1.0 + 149.0 * exponent_map[0])
        spec_tint = material.tint

    specular = [0.0, 0.0, 0.0]
    rim = [0.0, 0.0, 0.0]
    reflect = [2.0 * n[k] * _dot(n, eye) - eye[k] for k in range(3)]
    for i, (color, position) in enumerate(ps_lights):
        light_dir = _normalize([p - w for p, w in zip(position, v["pos"])])
        l_dot_r = _saturate(_dot(reflect, light_dir))
        n_dot_l = _saturate(_dot(n, light_dir))
        for k in range(3):
            specular[k] += (l_dot_r ** spec_exponent) * n_dot_l * color[k] * v["atten"][i]
            if material.rim:
                rim[k] += (l_dot_r ** material.rim_exponent) * n_dot_l * color[k] * v["atten"][i]
    spec_mask *= fresnel
    specular = [x * spec_mask * material.boost for x in specular]

    albedo = base
    diffuse_component = [albedo[k] * diffuse[k] for k in range(3)]
    if material.self_illum:
        diffuse_component = [max(0.0, _lerp(diffuse_component[k],
                                            material.self_illum_tint[k] * albedo[k], base_alpha))
                             for k in range(3)]
    if material.rim:
        multiply = rim_mask * rim_fresnel
        rim = [x * multiply for x in rim]
        specular = [max(specular[k], rim[k]) for k in range(3)]
        rim_ambient = _ambient_ps(ps_cube, eye)
        weight = _saturate(multiply * n[2])
        specular = [specular[k] + rim_ambient[k] * material.rim_boost * weight for k in range(3)]
    return [(specular[k] * spec_tint[k] + diffuse_component[k]) * scale for k in range(3)]


class CaseModel:
    """The expected color of each pixel of one case."""

    TRIANGLES = ((0, 1, 2), (0, 2, 3))

    def __init__(self, report, case):
        self.width, self.height = report["frame"]
        self.scale = report["tone_scale"]
        self.base = [srgb_to_linear(c / 255.0) for c in report["base_color"]]
        self.quads = []
        self.case = case
        self.report = report
        params = report.get("materials", {}).get(case.get("material"), {})
        self.phong = PhongMaterial(report, params) if wants_skin_shader(params) else None
        matrix = case["model_matrix"]
        for quad in report["quads"]:
            world = [_transform(matrix, (x, y, report["quad_z"]), 1.0)
                     for x, y in quad["corners"]]
            normal = _transform(matrix, quad["normal"], 0.0)
            if self.phong:
                tangent = _transform(matrix, quad["tangent"][:3], 0.0) + [quad["tangent"][3]]
                lighting = [_phong_vertex(case, report, position, normal, tangent)
                            for position in world]
            else:
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
                if self.phong:
                    inputs = {key: [sum(w * lighting[v][key][k] for w, v in
                                        zip(weights, (a, b, c)))
                                    for k in range(len(lighting[a][key]))]
                              for key in lighting[a]}
                    color = _phong_pixel(self.case, self.report, self.phong, inputs, self.scale)
                    return tuple(round(linear_to_srgb(x) * 255.0) for x in color)
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
        tolerance = PHONG_MODEL_TOLERANCE if model.phong else MODEL_TOLERANCE
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
                if any(abs(p - e) > tolerance for p, e in zip(pixel, expected)):
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
