# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Oracle for the "portal" family of material pixel conformance.

The harness (unittests/shaderextensiontest/material_pixel_portal.cpp) renders
Portal's stencil portals with the client's exact draw and stencil sequence
(CPortalRender::DrawPortalsUsingStencils) and the real Portal materials, and
writes one full frame per case. This module judges those frames:

* the backend must have a stencil buffer and reach the client's recursion depth
  (r_portal_stencil_depth 2); without stencil bits the client renders portals
  with textures instead, which is a different algorithm;
* a ray-traced model of the same scene, independent of any backend, says which
  recursion level and which object every pixel shows. Every pixel the model can
  decide robustly (its footprint does not straddle an edge, and no portal
  overlay effect covers it) must show that object's color. This is what proves
  the stencil recursion, the depth clears under the stencil and the custom clip
  plane of each nested view;
* the whole frame, overlay effects included, must agree with the D3D9 reference
  capture: the portal shaders' own math (stencil hole, static overlay flames,
  opening refraction) is judged there.

Frames travel inside the capture as base64 of zlib-compressed RGB rows.
"""

import base64
import math
from pathlib import Path
import zlib

CASES = ("recursion", "opening", "static")
# r_portal_stencil_depth's default.
PORTAL_STENCIL_DEPTH = 2
# Solid surfaces under UnlitGeneric: the texel, up to output rounding.
COLOR_TOLERANCE = 3
# Per-channel agreement with the reference, and how many pixels may exceed it.
# Both backends run on the same GPU with the same sampling configuration, so
# they agree to float-rounding differences between the FXC bytecode and its
# GLSL port: one channel of one flame pixel by one level when measured.
PIXEL_TOLERANCE = 1
MAX_DIFFERING_PIXELS = 8
# The sampling configuration the harness pins on every backend: one sample per
# pixel, and D3D9's dxsupport texture filter (trilinear, no forced anisotropy).
PINNED_SAMPLING = {"aa_samples": 0, "force_trilinear": True, "force_anisotropy": 1}
# Portal effect bands (portal_refract_ps2x.fxc): the overlay's flames reach this
# far inside and outside the opening radius; a margin keeps samples clear.
INNER_BORDER = 0.075 * 4.0
OUTER_BORDER = 0.075
BAND_MARGIN = 0.05


# How the D3D9 (DXVK) reference is rendered; see material_pixel_conformance.run.
DXVK_CONFIG = "dxvk.enableGraphicsPipelineLibrary = False"


class PortalCaptureError(ValueError):
    pass


def encode_frame(rgb):
    return base64.b64encode(zlib.compress(bytes(rgb), 9)).decode("ascii")


def decode_frame(case, frame):
    data = zlib.decompress(base64.b64decode(case["frame"]))
    if len(data) != frame[0] * frame[1] * 3:
        raise PortalCaptureError("case %s holds %d bytes, not a %dx%d RGB frame"
                                 % (case["name"], len(data), frame[0], frame[1]))
    return data


def embed_frames(report, directory):
    """Moves each case's raw frame file into the capture."""
    width, height = report["frame"]
    for case in report["cases"]:
        path = Path(directory) / case.pop("frame_file")
        rgb = path.read_bytes()
        if len(rgb) != width * height * 3:
            raise PortalCaptureError("%s holds %d bytes, not a %dx%d RGB frame"
                                     % (path, len(rgb), width, height))
        case["frame"] = encode_frame(rgb)
        path.unlink()
    return report


def validate(path, report):
    names = [case.get("name") for case in report.get("cases", [])]
    if names != list(CASES):
        raise PortalCaptureError("%s has portal cases %s, expected %s" % (path, names, list(CASES)))
    if len(report.get("frame", [])) != 2 or "scene" not in report:
        raise PortalCaptureError("%s has no frame size or scene" % path)
    for case in report["cases"]:
        if "frame" not in case:
            raise PortalCaptureError("%s: case %s has no frame" % (path, case["name"]))
        decode_frame(case, report["frame"])
    return report


def _smoothstep(edge0, edge1, x):
    t = min(max((x - edge0) / (edge1 - edge0), 0.0), 1.0)
    return t * t * (3.0 - 2.0 * t)


class SceneModel:
    """The harness scene, ray traced. View space of the level-0 camera: x right,
    y up, looking down -z; each nested level's camera is moved by camera_step."""

    def __init__(self, report, case):
        scene = report["scene"]
        self.width, self.height = report["frame"]
        self.max_depth = report["max_depth"]
        self.tan_half = math.tan(math.radians(scene["fov_x"]) / 2.0)
        self.aspect = self.width / self.height
        self.z_near = scene["z_near"]
        self.center = scene["portal_center"]
        self.half = scene["portal_half_size"]
        self.portal_z = self.center[2] + scene["portal_forward_offset"]
        self.step = scene["camera_step"]
        self.exit_distance = scene["exit_portal_distance"]
        self.wall_z = scene["wall_z"]
        self.blocker = scene["blocker"]
        self.colors = scene["level_colors"]
        opening = _smoothstep(0.0, 1.0, min(max(case["open_amount"], 0.0), 1.0))
        self.open_sq = opening * opening
        self.static = case["static_amount"]
        self.opening = 0.0 < case["open_amount"] < 1.0

    def camera(self, level):
        return [s * level for s in self.step]

    def trace(self, fx, fy):
        """(label, clean) of the point at frame fractions fx, fy (from the top
        left). label names what the frame shows there; clean is False when a
        portal effect (overlay flames, static, opening refraction) covers the
        point, so only the reference can judge it."""
        ndc_x = fx * 2.0 - 1.0
        ndc_y = 1.0 - fy * 2.0
        direction = (ndc_x * self.tan_half, ndc_y * self.tan_half / self.aspect, -1.0)
        clean = True
        for level in range(self.max_depth + 1):
            origin = self.camera(level)
            hits = []
            # Depth along the view axis is t (direction.z == -1); the near plane
            # culls anything closer than z_near.
            t = origin[2] - self.wall_z
            if t >= self.z_near:
                hits.append((t, "wall"))
            if direction[1] < 0.0:
                t = (origin[1] - self.blocker["y"]) / -direction[1]
                point = [o + d * t for o, d in zip(origin, direction)]
                inside = (self.blocker["x"][0] <= point[0] <= self.blocker["x"][1] and
                          self.blocker["z"][0] <= point[2] <= self.blocker["z"][1])
                clip_z = origin[2] - self.exit_distance + 0.5
                if inside and t >= self.z_near and (level == 0 or point[2] <= clip_z):
                    hits.append((t, "blocker"))
            t = origin[2] - self.portal_z
            portal_dist = None
            if t >= self.z_near:
                point = [o + d * t for o, d in zip(origin, direction)]
                u = (point[0] - self.center[0] + self.half[0]) / (2.0 * self.half[0])
                v = (self.center[1] + self.half[1] - point[1]) / (2.0 * self.half[1])
                if 0.0 <= u <= 1.0 and 0.0 <= v <= 1.0:
                    # portal_refract_vs20: the opening is drawn in a slightly
                    # shrunken oval of the quad's texture coordinates.
                    su = (u * 1.075 - 0.0375) * 2.0 - 1.0
                    sv = (v * 1.075 - 0.0375) * 2.0 - 1.0
                    portal_dist = math.hypot(su, sv)
                    hits.append((t, "portal"))
            t, what = min(hits)
            if what != "portal":
                return (level, what), clean
            # The overlay is drawn over the portal at every level; its flames
            # (and, while opening, the refraction) cover these bands.
            if self.open_sq - INNER_BORDER - BAND_MARGIN <= portal_dist <= \
                    self.open_sq + OUTER_BORDER + BAND_MARGIN:
                clean = False
            if self.opening and portal_dist <= 1.0 + BAND_MARGIN:
                clean = False
            if portal_dist > self.open_sq:
                # Outside the hole: the portal quad writes no color there.
                hits.remove((t, what))
                t, what = min(hits)
                return (level, what), clean
            if self.static >= 1.0 or level == self.max_depth:
                # All static: the hole holds the overlay's static; at the last
                # level the hole shows the overlay over this level's scene.
                if self.static >= 1.0:
                    return (level, "static"), False
                hits.remove((t, what))
                t, what = min(hits)
                return (level, what), clean
        raise AssertionError("unreachable")

    def label(self, x, y):
        """The model's label for pixel (x, y) if the pixel's whole footprint
        agrees and no effect covers it; None when only the reference can tell."""
        # D3D9 puts pixel centers on integer window coordinates (the native
        # backend shifts its viewports by half a pixel to match), so pixel (x, y)
        # samples the scene at (x, y) and its footprint spans half a pixel around.
        labels = set()
        for dx, dy in ((0.0, 0.0), (-0.48, -0.48), (0.48, -0.48), (-0.48, 0.48), (0.48, 0.48)):
            label, clean = self.trace((x + dx) / self.width, (y + dy) / self.height)
            if not clean:
                return None
            labels.add(label)
        return labels.pop() if len(labels) == 1 else None

    def color(self, label):
        level, what = label
        return self.colors[level] if what == "wall" else self.blocker["colors"][level]


def check_capture(report):
    failures = []
    unpinned = sorted(key for key, value in PINNED_SAMPLING.items() if report.get(key) != value)
    if unpinned:
        failures.append("sampling configuration not pinned (%s): frames from different "
                        "filters or sample counts are not comparable"
                        % ", ".join("%s=%r" % (key, report.get(key)) for key in unpinned))
    if report.get("stencil_bits", 0) <= 0:
        return failures + ["backend reports no stencil bits; the client would render portals "
                           "with textures instead of stencil recursion"]
    expected_depth = min(PORTAL_STENCIL_DEPTH, (1 << min(report["stencil_bits"], 4)) - 1)
    if report["max_depth"] != expected_depth or expected_depth < PORTAL_STENCIL_DEPTH:
        failures.append("recursion depth %d with %d stencil bits; the client draws %d nested "
                        "views" % (report["max_depth"], report["stencil_bits"],
                                   PORTAL_STENCIL_DEPTH))
    return failures


def check_model(report):
    """Every robustly decidable pixel shows what the ray-traced scene says."""
    failures = []
    width, height = report["frame"]
    for case in report["cases"]:
        model = SceneModel(report, case)
        rgb = decode_frame(case, report["frame"])
        wrong = []
        seen = set()
        for y in range(height):
            for x in range(width):
                label = model.label(x, y)
                if label is None:
                    continue
                seen.add(label)
                expected = model.color(label)
                pixel = rgb[(y * width + x) * 3:(y * width + x) * 3 + 3]
                if any(abs(p - e) > COLOR_TOLERANCE for p, e in zip(pixel, expected)):
                    wrong.append((x, y, label, list(pixel), expected))
        if wrong:
            x, y, label, pixel, expected = wrong[0]
            failures.append("%s: %d pixels disagree with the scene model; first at (%d, %d): "
                            "%s, expected %s (%s of level %d)"
                            % (case["name"], len(wrong), x, y, pixel, expected, label[1],
                               label[0]))
        # The model must reach every level the case draws, or the check proved
        # nothing about the recursion.
        levels = {level for level, _ in seen}
        # A half-open portal's hole lies entirely under the opening refraction and
        # the flames, so only the reference judges what shows through it.
        see_through = case["static_amount"] < 1.0 and case["open_amount"] >= 1.0
        wanted = set(range(report["max_depth"] + 1)) if see_through else {0}
        if not wanted <= levels:
            failures.append("%s: the model decided no pixel of levels %s"
                            % (case["name"], sorted(wanted - levels)))
        if case["name"] == "recursion" and (1, "blocker") not in seen:
            failures.append("recursion: no pixel shows the clipped blocker in the first "
                            "nested view")
    return failures


def compare(report, reference):
    if report["frame"] != reference["frame"]:
        return ["frame %s, reference %s; recapture at the same drawable size"
                % (report["frame"], reference["frame"])]
    if report["scene"] != reference["scene"] or \
            [(c["open_amount"], c["static_amount"]) for c in report["cases"]] != \
            [(c["open_amount"], c["static_amount"]) for c in reference["cases"]]:
        return ["portal scene or case inputs differ from the reference capture"]
    failures = []
    width = report["frame"][0]
    for case, ref in zip(report["cases"], reference["cases"]):
        rgb = decode_frame(case, report["frame"])
        expected = decode_frame(ref, reference["frame"])
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
                               first % width, first // width, list(rgb[first * 3:first * 3 + 3]),
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


def write_png(path, width, height, rgb):
    """A plain PNG of a frame, for looking at captures."""
    raw = b"".join(b"\x00" + bytes(rgb[y * width * 3:(y + 1) * width * 3])
                   for y in range(height))

    def chunk(kind, data):
        body = kind + data
        return (len(data).to_bytes(4, "big") + body +
                (zlib.crc32(body) & 0xffffffff).to_bytes(4, "big"))

    header = width.to_bytes(4, "big") + height.to_bytes(4, "big") + b"\x08\x02\x00\x00\x00"
    Path(path).write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) +
                           chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))
