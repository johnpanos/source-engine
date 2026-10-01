#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""The lab gallery: render_lab beside Cycles for every lighting fixture view.

    python3 tools/quality/lighting_fixtures.py gallery [--lab PATH] [--fixture NAME]...
                                                       [--out DIR]

RFC 0016 K11 "Gallery for review": for each view of each fixture in
quality/fixtures/lighting/, render_lab renders the view's camera, the image is
scored against the Cycles reference with lighting_fixtures.compare (the same
metric and tolerance as the K11 check), and one self-contained HTML page shows
per view:

- render_lab and Cycles with the same exposure (the reference's log-average
  luminance, then one tone curve), so brightness differences are real;
- the receiver error map: per pixel |lab - Cycles| (largest channel) over the
  reference's mean receiver luminance, the K11 metric, colored from 0 to twice
  the fixture's p99 tolerance, with skipped pixels in grey;
- a visible-emitter error map and diagnostic, separately scaled by the
  reference emitter luminance, with a black-emitter negative control;
- mean and p99 against the tolerance, and pass or fail.

The page embeds every image, so it can be opened directly or published. A
gallery never records a comparison; `compare --record` does that.

render_lab is found from --lab, then $RENDER_LAB, then the render-core
worktree's and this tree's build-rc-lab. Nothing here changes a fixture.
"""

import base64
import datetime
import html
import io
import json
import os
import re
import subprocess
from pathlib import Path

import lighting_fixtures as lf

METERS_TO_UNITS = lf.SOURCE_UNITS_PER_METER
LAB_CANDIDATES = (
    lf.ROOT / "build-rc-lab" / "render" / "lab" / "render_lab",
    lf.ROOT.parent / "source-engine-render-core" / "build-rc-lab" / "render" / "lab" / "render_lab",
)


def find_lab(explicit=None):
    for candidate in ([explicit] if explicit else []) + (
            [os.environ["RENDER_LAB"]] if os.environ.get("RENDER_LAB") else []) + list(
            LAB_CANDIDATES):
        path = Path(candidate)
        if path.is_file() and os.access(path, os.X_OK):
            return path.resolve()
    raise SystemExit("render_lab not found: build it (WAFLOCK=.lock-waf-rc-lab ./waf build "
                     "--target=render_lab in a build-rc-lab tree) or pass --lab")


def lab_environment(lab):
    # render_lab needs tier0 from its own tree: <tree>/render/lab/render_lab.
    tree = lab.parents[2]
    env = dict(os.environ)
    env["LD_LIBRARY_PATH"] = str(tree / "tier0") + (
        ":" + env["LD_LIBRARY_PATH"] if env.get("LD_LIBRARY_PATH") else "")
    return env


def probe_model(fixture, state=None):
    """The fixture's dynamic model for render_lab (--model, --model-origin in
    Source units), from its stage, or None; published with the state's map."""
    probe = fixture.get("probe_model")
    if not probe or not fixture.get("dynamic_models"):
        return None
    stage = fixture["directory"] / fixture["stage"] if fixture.get("stage") else None
    if not stage or not stage.is_file():
        return None
    text = stage.read_text(errors="replace")
    match = re.search(r'sourceEngine:model = "%s"[\s\S]*?xformOp:translate = \(([^)]+)\)'
                      % re.escape(probe["model"]), text)
    if not match:
        return None
    origin = [float(v) * METERS_TO_UNITS for v in match.group(1).split(",")]
    # The map pipeline publishes the stage's model per map as
    # models/<map>/probesphere.mdl; the stage names the source model.
    entry = lf.map_for(fixture, state)
    game = str((lf.ROOT / entry["bsp"]).resolve()).rsplit("/maps/", 1)[0]
    published = "models/%s/probesphere.mdl" % entry["name"]
    model = published if (Path(game) / published).is_file() else probe["model"]
    return model, origin


def fog_scale(fixture, state):
    """render_lab's --fog-scale for a state: 0 for a medium-free state of a
    fixture whose other states have a medium (every map's entity lump carries
    the medium; the density-zero state scales it to 0), else None (the map's
    own medium)."""
    media = fixture["lighting"].get("media") or {}
    if any(media.values()) and not media.get(state):
        return 0.0
    return None


# `gallery --direct`: the runtime direct diffuse light alone, as Cycles'
# DiffDir x DiffCol shows it (every term but the lights off, diffuse lobe).
DIRECT_ARGS = ["--core-direct", "--debug-brdf", "1", "--debug-term",
               "baked,probes,ibl,ssr,ao,specular_occlusion,emission,volumetric", "--no-ao",
               "--no-ssr", "--no-bounce"]


def render_camera(lab, fixture, camera_name, out, scale=None, state=None, resolution=1,
                  core_direct=False, direct=False, terms_off=None):
    """render_lab's frame of a camera from the state's map (lf.map_for: a
    state that owns a map, such as a medium state baked with its medium,
    renders from it). With `core_direct` the lab draws the indirect layer and
    every light's direct light at runtime (--core-direct), as the product
    does under RFC 0016's runtime direct light."""
    camera = fixture["cameras"][camera_name]
    bsp = (lf.ROOT / lf.map_for(fixture, state)["bsp"]).resolve()
    game = str(bsp).rsplit("/maps/", 1)[0]
    film = fixture.get("film", {"width": 512, "height": 384})
    command = [str(lab), "--game", game, "--map", str(bsp),
               "--eye", ",".join("%.4f" % (v * METERS_TO_UNITS) for v in camera["eye"]),
               "--forward", ",".join("%.6f" % v for v in camera["forward"]),
               "--up", ",".join("%.6f" % v for v in camera["up"]),
               "--hfov", str(fixture.get("horizontal_fov_degrees", 90)),
               "--size", "%dx%d" % (film["width"] * resolution, film["height"] * resolution),
               "--out", str(out)]
    model = probe_model(fixture, state)
    if model:
        # The references make every prop a receiver only (gi_reference_blender.py).
        command += ["--model", model[0], "--model-origin", ",".join("%.4f" % v for v in model[1]),
                    "--model-no-shadow"]
    # The state's movers (lighting.movers: boxes the bake did not see, such as
    # a closed door), in Source units, with their world material.
    for mover in fixture["lighting"].get("movers", {}).get(state or "", []):
        command += ["--mover", ",".join("%.4f" % v for v in list(mover["min"]) +
                                        list(mover["max"])) + "," + mover["material"]]
    if scale is not None:
        command += ["--fog-scale", "%g" % scale]
    if direct:
        command += DIRECT_ARGS
    elif core_direct:
        command.append("--core-direct")
    if terms_off:
        command += ["--debug-term", terms_off]
    result = subprocess.run(command, env=lab_environment(lab), capture_output=True, text=True,
                            timeout=600)
    message = (result.stdout + result.stderr).strip().splitlines()
    return result.returncode == 0 and out.is_file(), (message[-1] if message else ""), model


def tone_map(rgb, key):
    """Exposure to middle grey at the reference's log-average luminance, a
    Reinhard curve, sRGB encoding: the same curve for both images."""
    import numpy as np
    x = np.maximum(rgb, 0.0) * (0.18 / max(key, 1e-6))
    x = np.clip(x / (1.0 + x) * 1.6, 0.0, 1.0)
    s = np.where(x <= 0.0031308, 12.92 * x, 1.055 * np.power(x, 1.0 / 2.4) - 0.055)
    return (np.clip(s, 0.0, 1.0) * 255.0 + 0.5).astype(np.uint8)


def log_average(rgb):
    import numpy as np
    return float(np.exp(np.mean(np.log(np.maximum(lf.luminance(rgb), 1e-4)))))


def error_map(test, reference, mask, limit):
    """The K11 metric per pixel, colored from 0 (dark) through amber to red at
    `limit`; skipped pixels grey."""
    import numpy as np
    scale = float(lf.luminance(reference[mask]).mean()) if mask.any() else 1.0
    error = np.max(np.abs(test - reference), axis=-1) / max(scale, 1e-9)
    t = np.clip(np.nan_to_num(error, nan=limit, posinf=limit) / max(limit, 1e-9), 0.0, 1.0)
    ramp = np.array([[18, 20, 26], [60, 80, 150], [230, 170, 40], [215, 40, 40]], np.float64)
    position = t * (len(ramp) - 1)
    low = np.floor(position).astype(int).clip(0, len(ramp) - 2)
    frac = (position - low)[..., None]
    color = ramp[low] * (1.0 - frac) + ramp[low + 1] * frac
    color[~mask] = (128, 128, 128)
    return color.astype(np.uint8)


def jpeg_uri(pixels, quality=86):
    from PIL import Image
    buffer = io.BytesIO()
    Image.fromarray(pixels).save(buffer, "JPEG", quality=quality)
    return "data:image/jpeg;base64," + base64.b64encode(buffer.getvalue()).decode()


def write_pfm(path, rgb):
    import numpy as np
    height, width = rgb.shape[:2]
    with open(path, "wb") as stream:
        stream.write(b"PF\n%d %d\n-1.0\n" % (width, height))
        stream.write(np.ascontiguousarray(rgb[::-1, :, :3], "<f4").tobytes())


def downsampled(lab_image, factor):
    """A box-filtered copy of a lab frame rendered `factor` times the
    reference's size, beside it, for the comparison (the metric is judged at
    the reference's resolution)."""
    import numpy as np
    image = np.asarray(lf.read_image(lab_image), np.float64)[..., :3]
    height, width = image.shape[0] // factor, image.shape[1] // factor
    small = image[:height * factor, :width * factor].reshape(
        height, factor, width, factor, 3).mean(axis=(1, 3))
    path = Path(str(lab_image).replace(".pfm", ".down%d.pfm" % factor))
    write_pfm(path, small)
    return path


def view_images(fixture, state, camera, lab_image, resolution=1, kind="total"):
    """(lab, reference, receiver error, emitter error) as display pixels, and the result
    against the view's `kind` reference (total, or direct). With a
    resolution factor the lab frame shows at its own size and the reference
    and error map at theirs, enlarged to it (nearest)."""
    import numpy as np
    from PIL import Image
    full_image = lab_image
    if resolution > 1:
        lab_image = downsampled(lab_image, resolution)
    result = lf.compare(fixture["name"], state, camera, lab_image, kind=kind)
    references = fixture["directory"] / "references"
    record = json.loads((references / "references.json").read_text())
    view = record["views"]["%s.%s" % (state, camera)]
    reference = np.asarray(lf.read_rgb_exr(references / view["files"][kind]["file"]),
                           np.float64)[..., :3]
    index_image = np.asarray(Image.open(references / view["files"]["index"]["file"]), np.int64)
    test = np.asarray(lf.read_image(lab_image), np.float64)[..., :3]
    key = log_average(reference)
    mask = lf.judged_mask(index_image, view["emitter_indices"])
    emitters = lf.emitter_mask(index_image, view["emitter_indices"])
    result["emitters"] = lf.emitter_error_stats(test, reference, index_image,
                                                view["emitter_indices"])
    limit = 2.0 * result["tolerance"]["p99"]
    images = [tone_map(test, key), tone_map(reference, key),
              error_map(test, reference, mask, limit),
              error_map(test, reference, emitters, 2.0)]
    if resolution > 1:
        full = np.asarray(lf.read_image(full_image), np.float64)[..., :3]
        images = [tone_map(full, key)] + [
            np.repeat(np.repeat(i, resolution, axis=0), resolution, axis=1) for i in images[1:]]
    return tuple(images), result, record["status"]


def lab_revision(lab):
    tree = lab.parents[2]
    for candidate in (tree, tree.parent):
        probe = subprocess.run(["git", "-C", str(candidate), "log", "-1", "--format=%h %s"],
                               capture_output=True, text=True)
        if probe.returncode == 0 and probe.stdout.strip():
            return probe.stdout.strip()
    return "unknown"


PAGE_STYLE = """
<style>
/* Layout: a summary strip, then one row per view: four images side by side
   (lab, Cycles, receiver error, emitter error), stacking on phones. */
:root {
  --bg: #f4f5f7; --panel: #ffffff; --fg: #1c2028; --muted: #5b6372;
  --line: #d9dde4; --pass: #1f7a4d; --fail: #b3261e; --warn: #8a5a00;
  --font-body: "IBM Plex Sans", "Segoe UI", system-ui, sans-serif;
  --font-data: "IBM Plex Mono", ui-monospace, "SFMono-Regular", monospace;
}
@media (prefers-color-scheme: dark) { :root:not([data-theme="light"]) {
  --bg: #111318; --panel: #1a1d24; --fg: #e6e8ec; --muted: #9aa2b1;
  --line: #2c313b; --pass: #5cc98f; --fail: #ff7a70; --warn: #e0b04a; color-scheme: dark } }
:root[data-theme="dark"] {
  --bg: #111318; --panel: #1a1d24; --fg: #e6e8ec; --muted: #9aa2b1;
  --line: #2c313b; --pass: #5cc98f; --fail: #ff7a70; --warn: #e0b04a; color-scheme: dark }
body { background: var(--bg); color: var(--fg); font-family: var(--font-body);
  font-size: 15px; line-height: 1.5; padding-inline: 16px; padding-block: 24px 48px }
main { max-width: 1320px; margin-inline: auto; display: grid; gap: 28px }
h1 { font-size: 1.6rem; margin: 0; text-wrap: balance }
h2 { font-size: 1.05rem; margin: 0 }
.meta { color: var(--muted); font-family: var(--font-data); font-size: 0.8rem;
  display: flex; flex-wrap: wrap; gap: 6px 18px }
.summary { display: flex; flex-wrap: wrap; gap: 10px }
.pill { font-family: var(--font-data); font-size: 0.78rem; padding: 2px 10px;
  border-radius: 999px; border: 1px solid var(--line); white-space: nowrap }
.pill.pass { color: var(--pass); border-color: var(--pass) }
.pill.fail { color: var(--fail); border-color: var(--fail) }
.pill.warn { color: var(--warn); border-color: var(--warn) }
.note { color: var(--muted); max-width: 72ch; margin: 0 }
.view { background: var(--panel); border: 1px solid var(--line); border-radius: 6px;
  padding: 14px; display: grid; gap: 10px }
.view header { display: flex; flex-wrap: wrap; align-items: baseline; gap: 8px 14px }
.numbers { font-family: var(--font-data); font-size: 0.8rem; color: var(--muted);
  font-variant-numeric: tabular-nums }
.images { display: grid; grid-template-columns: repeat(4, minmax(0, 1fr)); gap: 8px }
figure { margin: 0; display: grid; gap: 4px; min-width: 0 }
figure img { width: 100%; height: auto; display: block; border-radius: 3px }
figcaption { font-size: 0.72rem; letter-spacing: 0.06em; text-transform: uppercase;
  color: var(--muted) }
.failed { color: var(--fail); font-family: var(--font-data); font-size: 0.82rem }
@media (max-width: 720px) { .images { grid-template-columns: minmax(0, 1fr) } }
</style>
"""


def page(entries, lab, revision, generated):
    passed = sum(1 for e in entries if e.get("result", {}).get("pass"))
    failed_render = sum(1 for e in entries if "error" in e)
    preview = any(e.get("status") == "preview" for e in entries)
    parts = ['<title>Lighting Lab Gallery</title>',
             '<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=IBM+Plex+'
             'Mono:wght@400;500&family=IBM+Plex+Sans:wght@400;600&display=swap">',
             PAGE_STYLE, "<main>", "<section style=\"display:grid;gap:10px\">",
             "<h1>Lighting lab gallery</h1>",
             '<div class="meta"><span>render_lab %s</span><span>%s</span><span>%s</span></div>'
             % (html.escape(str(lab)), html.escape(revision), html.escape(generated)),
             '<div class="summary"><span class="pill pass">%d pass</span>'
             '<span class="pill fail">%d fail</span>%s%s</div>'
             % (passed, len(entries) - passed - failed_render,
                ('<span class="pill fail">%d not rendered</span>' % failed_render)
                if failed_render else "",
                '<span class="pill warn">preview references</span>' if preview else ""),
             '<p class="note">Each row: render_lab, the Cycles reference at the same exposure, '
             'the receiver error map (red at twice the fixture\'s p99 tolerance), and a '
             'separate visible-emitter error map (red at twice the reference emitter\'s mean '
             'luminance). Grey pixels are outside each mask. Emitter values are diagnostic '
             'until their own tolerances are fixed; black control is the error if the emitter '
             'renders black. '
             'Preview references certify nothing.</p>', "</section>"]
    for e in entries:
        title = "%s / %s / %s" % (e["fixture"], e["state"], e["camera"])
        parts.append('<article class="view"><header><h2>%s</h2>' % html.escape(title))
        if "error" in e:
            parts.append('<span class="pill fail">not rendered</span></header>'
                         '<p class="failed">%s</p></article>' % html.escape(e["error"]))
            continue
        r = e["result"]
        parts.append('<span class="pill %s">%s</span><span class="numbers">mean %.3f / %.2f'
                     ' &middot; p99 %.2f / %.2f%s</span></header>'
                     % ("pass" if r["pass"] else "fail", "pass" if r["pass"] else "fail",
                        r["mean"], r["tolerance"]["mean"], r["p99"], r["tolerance"]["p99"],
                        (" &middot; model %s" % html.escape(e["model"])) if e.get("model")
                        else ""))
        if r.get("emitters"):
            emitter = r["emitters"]
            parts.append('<span class="numbers">emitter pixels %d &middot; mean %.3f '
                         '&middot; p99 %.3f &middot; black control %.3f</span>'
                         % (emitter["pixels"], emitter["mean"], emitter["p99"],
                            emitter["black_control_mean"]))
        parts.append('<div class="images">')
        for caption, uri in zip(("render_lab", "Cycles (%s)" % e["status"],
                                 "receiver error", "emitter error"),
                                e["images"]):
            parts.append('<figure><img alt="%s, %s" src="%s"><figcaption>%s</figcaption>'
                         '</figure>' % (html.escape(title), caption, uri, html.escape(caption)))
        parts.append("</div></article>")
    parts.append("</main>")
    return "\n".join(parts)


def cmd_gallery(args):
    lab = find_lab(args.lab)
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    out = Path(args.out or (lf.ROOT / "quality-results" / "lighting-gallery" / stamp))
    (out / "lab").mkdir(parents=True, exist_ok=True)
    names = lf.cycles_oracle_names(args.fixture)
    entries = []
    for name in names:
        fixture = lf.load_fixture(name)
        record = json.loads((fixture["directory"] / "references" / "references.json").read_text())
        rendered = {}
        for key in sorted(record["views"]):
            state, camera = key.split(".", 1)
            entry = {"fixture": name, "state": state, "camera": camera}
            scale = fog_scale(fixture, state)
            map_name = lf.map_for(fixture, state)["name"]
            # A state with movers renders on its own (the movers are its).
            movers = state if fixture["lighting"].get("movers", {}).get(state) else None
            view = (camera, scale, map_name, movers)
            if view not in rendered:
                image = out / "lab" / ("%s.%s%s%s%s.pfm" % (
                    name, camera, "" if map_name == fixture["lighting"]["map"]["name"]
                    else "." + map_name, "" if scale is None else ".fog-scale-%g" % scale,
                    "" if movers is None else "." + movers))
                rendered[view] = render_camera(lab, fixture, camera, image, scale, state,
                                               getattr(args, "resolution", 1),
                                               getattr(args, "core_direct", False),
                                               getattr(args, "direct", False)) + (image,)
            ok, message, model, image = rendered[view]
            if not ok:
                entry["error"] = message or "render_lab failed"
            else:
                try:
                    pixels, result, status = view_images(
                        fixture, state, camera, image, getattr(args, "resolution", 1),
                        "direct" if getattr(args, "direct", False) else "total")
                    entry.update(result=result, status=status, model=model and model[0],
                                 images=[jpeg_uri(p) for p in pixels])
                except (ValueError, KeyError, OSError) as error:
                    entry["error"] = "compare: %s" % error
            entries.append(entry)
            print("%-18s %-10s %-8s %s" % (name, state, camera,
                  entry["error"] if "error" in entry else "mean %.3f p99 %.2f %s" % (
                      entry["result"]["mean"], entry["result"]["p99"],
                      "pass" if entry["result"]["pass"] else "fail")))
    index = out / "index.html"
    index.write_text(page(entries, lab, lab_revision(lab),
                          datetime.datetime.now(datetime.timezone.utc).isoformat(
                              timespec="seconds")))
    summary = [{k: v for k, v in e.items() if k != "images"} for e in entries]
    (out / "summary.json").write_text(json.dumps(summary, indent=2, sort_keys=True, default=str)
                                      + "\n")
    print("gallery: %s (%d views)" % (index, len(entries)))
    return 0
