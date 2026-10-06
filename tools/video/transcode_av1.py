#!/usr/bin/env python3
"""Transcode an installed game's Bink movies to AV1 offline.

    python3 tools/video/transcode_av1.py --source "<Steam>/Portal 2" --out run/media-av1

Every loose ``<game dir>/media/*.bik`` under --source becomes
``<out>/<game dir>/media/<name>.webm``: AV1 (SVT-AV1) video in 8-bit 4:2:0 at
the source's size and frame rate, and Opus audio for each source audio track.
The runtime's ``av1`` video provider (video/video_bink, --video-provider=av1 or
ffmpeg) plays these; the game still asks for ``media/<name>.bik`` and video
services resolves the request to the ``.webm`` beside it in the search path.

Each output is checked against its source before it is kept: the decoded frame
counts must be equal and the mean SSIM of the decoded AV1 against the decoded
Bink must reach --min-ssim. Each clip walks a CRF ladder and keeps the first
(smallest) encode that passes; grainy cinematics need a lower CRF than menu
loops. A clip that fails at every rung is not written. ``<out>/manifest.json``
records, per clip, the source size and modification time, the encoder settings
and the measured SSIM; a clip whose record matches is not transcoded again.

Nothing here is redistributable: the outputs are derived from the user's
installed, licensed content and stay in the local run directory.
"""

import argparse
import concurrent.futures
import json
import os
from pathlib import Path
import re
import subprocess
import sys

# Encoder settings; any change here re-encodes every clip (the manifest key).
SETTINGS = {
    "version": 2,
    "video": ["-c:v", "libsvtav1", "-preset", "4", "-pix_fmt", "yuv420p",
              "-svtav1-params", "tune=0"],
    "crf_ladder": [24, 18, 12],
    # Two seconds between keyframes at most: SetTime and looping seek to the
    # previous keyframe and decode forward.
    "keyframe_seconds": 2,
    "audio": ["-c:a", "libopus", "-b:a", "128k"],
}
MANIFEST = "manifest.json"


def run(command, timeout=None):
    return subprocess.run(command, check=True, capture_output=True, text=True, timeout=timeout)


def probe(path):
    out = run(["ffprobe", "-v", "error", "-count_frames", "-show_entries",
               "stream=index,codec_type,codec_name,pix_fmt,width,height,avg_frame_rate,"
               "r_frame_rate,nb_read_frames", "-of", "json", str(path)]).stdout
    return json.loads(out)["streams"]


def frame_rate(stream):
    num, _, den = stream["r_frame_rate"].partition("/")
    return float(num) / float(den or 1)


def find_sources(source):
    clips = []
    for game in sorted(p for p in source.iterdir() if p.is_dir()):
        media = game / "media"
        if media.is_dir():
            clips += sorted(m for m in media.iterdir() if m.suffix.lower() == ".bik")
    return clips


def source_key(clip):
    stat = clip.stat()
    return {"size": stat.st_size, "mtime_ns": stat.st_mtime_ns, "settings": SETTINGS}


def transcode(clip, target, min_ssim):
    """Encode at each CRF of the ladder until one passes; return its record."""
    failures = []
    for crf in SETTINGS["crf_ladder"]:
        try:
            record = encode(clip, target, min_ssim, crf)
        except SsimBelowGate as error:
            failures.append("crf %d: %s" % (crf, error))
            continue
        record["crf"] = crf
        return record
    raise ValueError("; ".join(failures))


class SsimBelowGate(ValueError):
    pass


def encode(clip, target, min_ssim, crf):
    streams = probe(clip)
    video = [s for s in streams if s["codec_type"] == "video"]
    if len(video) != 1 or video[0].get("pix_fmt") not in ("yuv420p", "yuvj420p"):
        raise ValueError("expected one yuv420p video stream, found %s" %
                         [(s["codec_name"], s.get("pix_fmt")) for s in video])
    rate = frame_rate(video[0])
    gop = max(1, round(rate * SETTINGS["keyframe_seconds"]))
    target.parent.mkdir(parents=True, exist_ok=True)
    partial = target.with_name(target.stem + ".partial.webm")
    command = ["ffmpeg", "-v", "error", "-y", "-i", str(clip), "-map", "0:v:0", "-map", "0:a?",
               *SETTINGS["video"], "-crf", str(crf), "-g", str(gop), "-fps_mode", "passthrough",
               *SETTINGS["audio"], str(partial)]
    try:
        run(command)
        encoded = probe(partial)
        out_video = [s for s in encoded if s["codec_type"] == "video"]
        if len(out_video) != 1 or out_video[0]["codec_name"] != "av1" or \
                out_video[0]["pix_fmt"] != "yuv420p":
            raise ValueError("output is not one AV1 yuv420p stream")
        frames_in = int(video[0]["nb_read_frames"])
        frames_out = int(out_video[0]["nb_read_frames"])
        if frames_in != frames_out:
            raise ValueError("frame count %d, source %d" % (frames_out, frames_in))
        if (out_video[0]["width"], out_video[0]["height"]) != (video[0]["width"],
                                                               video[0]["height"]):
            raise ValueError("size changed")
        audio_in = sum(1 for s in streams if s["codec_type"] == "audio")
        audio_out = sum(1 for s in encoded if s["codec_type"] == "audio")
        if audio_in != audio_out:
            raise ValueError("audio tracks %d, source %d" % (audio_out, audio_in))
        # Both decoded and paired by frame index (counts are equal above): WebM
        # rounds timestamps to milliseconds, so pairing by time would compare
        # neighbouring frames of a 1199/50 fps clip. The reference is the Bink decode.
        pairing = ("[0:v]settb=1/1000,setpts=N[a];[1:v]settb=1/1000,setpts=N[b];"
                   "[a][b]ssim=shortest=1")
        log = subprocess.run(["ffmpeg", "-v", "info", "-nostats", "-i", str(partial),
                              "-i", str(clip), "-lavfi", pairing, "-f", "null", "-"],
                             capture_output=True, text=True, check=True).stderr
        match = re.search(r"SSIM .*All:([0-9.]+)", log)
        if not match:
            raise ValueError("no SSIM result")
        ssim = float(match.group(1))
        if ssim < min_ssim:
            raise SsimBelowGate("SSIM %.4f below %.4f" % (ssim, min_ssim))
        os.replace(partial, target)
    finally:
        partial.unlink(missing_ok=True)
    return {"frames": frames_out, "ssim": round(ssim, 5), "bytes": target.stat().st_size,
            "source_bytes": clip.stat().st_size, "audio_tracks": audio_out}


def load_manifest(out):
    try:
        return json.loads((out / MANIFEST).read_text())
    except (OSError, ValueError):
        return {"clips": {}}


def stale(source, out, manifest):
    """Source clips whose transcode is missing or was made from other inputs."""
    result = []
    for clip in find_sources(source):
        name = clip.relative_to(source).as_posix()
        record = manifest["clips"].get(name)
        target = out / Path(name).with_suffix(".webm")
        if not record or record.get("key") != source_key(clip) or not target.is_file():
            result.append((name, clip, target))
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--source", type=Path, required=True, help="installed game root")
    parser.add_argument("--out", type=Path, required=True, help="AV1 content root")
    parser.add_argument("--min-ssim", type=float, default=0.97)
    parser.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 4) // 8),
                        help="clips encoded at once (SVT-AV1 threads each one itself)")
    parser.add_argument("--check", action="store_true",
                        help="only report stale clips; exit 1 when any is")
    args = parser.parse_args(argv)
    sys.stdout.reconfigure(line_buffering=True)
    source, out = args.source.resolve(), args.out.resolve()
    if not find_sources(source):
        parser.exit(1, "transcode_av1: no media/*.bik under %s\n" % source)
    manifest = load_manifest(out)
    todo = stale(source, out, manifest)
    if args.check:
        for name, _, _ in todo:
            print("stale: " + name)
        return 1 if todo else 0
    if not todo:
        print("transcode_av1: %d clips current in %s" % (len(manifest["clips"]), out))
        return 0
    out.mkdir(parents=True, exist_ok=True)
    print("transcode_av1: encoding %d clips into %s" % (len(todo), out))
    failures = 0
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        futures = {pool.submit(transcode, clip, target, args.min_ssim): (name, clip)
                   for name, clip, target in todo}
        for future in concurrent.futures.as_completed(futures):
            name, clip = futures[future]
            try:
                record = future.result()
            except (subprocess.CalledProcessError, ValueError, OSError) as error:
                detail = getattr(error, "stderr", "") or str(error)
                print("transcode_av1: FAILED %s: %s" % (name, detail.strip()[-400:]))
                manifest["clips"].pop(name, None)
                failures += 1
                continue
            record["key"] = source_key(clip)
            manifest["clips"][name] = record
            print("transcode_av1: %s %d frames, crf %d, SSIM %.4f, %d -> %d KiB" % (
                name, record["frames"], record["crf"], record["ssim"],
                record["source_bytes"] // 1024,
                record["bytes"] // 1024))
            # Written after every clip, so an interrupted run keeps its progress.
            (out / MANIFEST).write_text(json.dumps(manifest, indent=1, sort_keys=True) + "\n")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
