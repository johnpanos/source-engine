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
- the error map: per pixel |lab - Cycles| (largest channel) over the
  reference's mean luminance, the K11 metric, colored from 0 to twice the
  fixture's p99 tolerance, with pixels the metric skips (background, visible
  emitters) in grey;
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


def probe_model(fixture):
    """The fixture's dynamic model for render_lab (--model, --model-origin in
    Source units), from its stage, or None."""
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
    game = str((lf.ROOT / fixture["lighting"]["map"]["bsp"]).resolve()).rsplit("/maps/", 1)[0]
    published = "models/%s/probesphere.mdl" % fixture["lighting"]["map"]["name"]
    model = published if (Path(game) / published).is_file() else probe["model"]
    return model, origin


def render_camera(lab, fixture, camera_name, out):
    camera = fixture["cameras"][camera_name]
    bsp = (lf.ROOT / fixture["lighting"]["map"]["bsp"]).resolve()
    game = str(bsp).rsplit("/maps/", 1)[0]
    film = fixture.get("film", {"width": 512, "height": 384})
    command = [str(lab), "--game", game, "--map", str(bsp),
               "--eye", ",".join("%.4f" % (v * METERS_TO_UNITS) for v in camera["eye"]),
               "--forward", ",".join("%.6f" % v for v in camera["forward"]),
               "--up", ",".join("%.6f" % v for v in camera["up"]),
               "--hfov", str(fixture.get("horizontal_fov_degrees", 90)),
               "--size", "%dx%d" % (film["width"], film["height"]), "--out", str(out)]
    model = probe_model(fixture)
    if model:
        command += ["--model", model[0], "--model-origin", ",".join("%.4f" % v for v in model[1])]
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


def view_images(fixture, state, camera, lab_image):
    """(lab, reference, error) as display pixels, and the comparison result."""
    import numpy as np
    from PIL import Image
    result = lf.compare(fixture["name"], state, camera, lab_image)
    references = fixture["directory"] / "references"
    record = json.loads((references / "references.json").read_text())
    view = record["views"]["%s.%s" % (state, camera)]
    reference = np.asarray(lf.read_rgb_exr(references / view["files"]["total"]["file"]),
                           np.float64)[..., :3]
    index_image = np.asarray(Image.open(references / view["files"]["index"]["file"]), np.int64)
    test = np.asarray(lf.read_image(lab_image), np.float64)[..., :3]
    key = log_average(reference)
    mask = lf.judged_mask(index_image, view["emitter_indices"])
    limit = 2.0 * result["tolerance"]["p99"]
    return (tone_map(test, key), tone_map(reference, key),
            error_map(test, reference, mask, limit)), result, record["status"]


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
/* Layout: a summary strip, then one row per view: three images side by side
   (lab, Cycles, error), stacking to one column on phones. */
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
.images { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 8px }
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
             'and the error map of the K11 metric (|lab - Cycles| over the reference\'s mean '
             'luminance; red at twice the fixture\'s p99 tolerance; grey where the metric skips '
             'background and visible emitters). Preview references are noisy and certify '
             'nothing.</p>', "</section>"]
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
        parts.append('<div class="images">')
        for caption, uri in zip(("render_lab", "Cycles (%s)" % e["status"], "error"),
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
    names = args.fixture or sorted(p.parent.name for p in lf.FIXTURES.glob("*/fixture.json"))
    entries = []
    for name in names:
        fixture = lf.load_fixture(name)
        record = json.loads((fixture["directory"] / "references" / "references.json").read_text())
        rendered = {}
        for key in sorted(record["views"]):
            state, camera = key.split(".", 1)
            entry = {"fixture": name, "state": state, "camera": camera}
            if camera not in rendered:
                image = out / "lab" / ("%s.%s.pfm" % (name, camera))
                rendered[camera] = render_camera(lab, fixture, camera, image) + (image,)
            ok, message, model, image = rendered[camera]
            if not ok:
                entry["error"] = message or "render_lab failed"
            else:
                try:
                    pixels, result, status = view_images(fixture, state, camera, image)
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
