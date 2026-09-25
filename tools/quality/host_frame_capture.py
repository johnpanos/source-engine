#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Compare host frame captures (engine -hostframetrace, RFC 0003 R10).

A capture is "host-frame-trace/v1": one "frame ..." line per host frame
followed by that frame's host-level calls, each with the tick and simulation
state it observed. Two captures of the same deterministic run (fixed
host_framerate) taken with host_frame_graph 0 (legacy hand-ordered frame) and
1 (serial frame graph) must be identical frame for frame. Lines starting with
"#" are metadata (such as the frame path in use) and are reported, not
compared.

Comparison is exact. --tolerance FIELD=ABS accepts an absolute difference in
one numeric snapshot field (for example ia, the interpolation amount, which
carries the wall-clock tick remainder left by frames that ran before
host_framerate applied). Every tolerated difference is counted and reported;
event order, arguments and all other fields stay exact.

Exit status: 0 identical, 1 divergent, 2 invalid capture.
"""

import argparse
import json
import sys

HEADER = "host-frame-trace/v1"


class CaptureError(ValueError):
    pass


def parse(text, name="capture", metadata=None):
    """Returns [(frame_line, [event lines])]; "#" lines go to metadata."""
    lines = text.splitlines()
    if not lines or lines[0].strip() != HEADER:
        raise CaptureError("%s: missing %s header" % (name, HEADER))
    frames = []
    for number, line in enumerate(lines[1:], start=2):
        line = line.rstrip()
        if not line:
            continue
        if line.startswith("#"):
            if metadata is not None:
                metadata.append({"frame": len(frames), "line": line[1:].strip()})
            continue
        if line.startswith("frame "):
            frames.append((line, []))
        elif not frames:
            raise CaptureError("%s:%d: event before the first frame" % (name, number))
        else:
            frames[-1][1].append(line)
    if not frames:
        raise CaptureError("%s: no frames" % name)
    return frames


def split_event(line):
    """("Input 1", {"ht": "1", ...}) for "Input 1 | ht=1 ..."."""
    head, _, snapshot = line.partition(" | ")
    fields = {}
    for item in snapshot.split():
        name, _, value = item.partition("=")
        fields[name] = value
    return head, fields


def events_match(want, got, tolerances, tolerated):
    if want == got:
        return True
    if not tolerances:
        return False
    want_head, want_fields = split_event(want)
    got_head, got_fields = split_event(got)
    if want_head != got_head or want_fields.keys() != got_fields.keys():
        return False
    differing = [name for name in want_fields if want_fields[name] != got_fields[name]]
    for name in differing:
        if name not in tolerances:
            return False
        try:
            delta = abs(float(want_fields[name]) - float(got_fields[name]))
        except ValueError:
            return False
        if delta > tolerances[name] + 1e-12:
            return False
    for name in differing:
        tolerated[name] = tolerated.get(name, 0) + 1
    return True


def compare(expected, actual, skip_frames=0, tolerances=None):
    """Frame-by-frame comparison; returns a JSON-serializable report."""
    tolerances = tolerances or {}
    tolerated = {}
    report = {"expected_frames": len(expected), "actual_frames": len(actual),
              "skipped_frames": skip_frames, "compared_frames": 0, "compared_events": 0,
              "tolerances": dict(tolerances), "tolerated_differences": tolerated,
              "identical": False, "first_divergence": None}
    count = min(len(expected), len(actual))
    for index in range(skip_frames, count):
        (expected_head, expected_events), (actual_head, actual_events) = expected[index], actual[index]
        if expected_head != actual_head:
            report["first_divergence"] = {"frame": index, "event": None,
                                          "expected": expected_head, "actual": actual_head}
            return report
        for event, (want, got) in enumerate(zip(expected_events, actual_events)):
            if not events_match(want, got, tolerances, tolerated):
                report["first_divergence"] = {"frame": index, "event": event,
                                              "expected": want, "actual": got}
                return report
        if len(expected_events) != len(actual_events):
            event = min(len(expected_events), len(actual_events))
            report["first_divergence"] = {
                "frame": index, "event": event,
                "expected": expected_events[event] if event < len(expected_events) else "<end of frame>",
                "actual": actual_events[event] if event < len(actual_events) else "<end of frame>"}
            return report
        report["compared_frames"] += 1
        report["compared_events"] += len(expected_events)
    if len(expected) != len(actual):
        report["first_divergence"] = {"frame": count, "event": None,
                                      "expected": expected[count][0] if count < len(expected) else "<end of capture>",
                                      "actual": actual[count][0] if count < len(actual) else "<end of capture>"}
        return report
    if report["compared_frames"] == 0:
        raise CaptureError("no frames compared")
    report["identical"] = True
    return report


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("expected", help="reference capture (legacy frame)")
    parser.add_argument("actual", help="capture under test (frame graph)")
    parser.add_argument("--skip-frames", type=int, default=0,
                        help="leading frames to exclude (never needed for a deterministic run)")
    parser.add_argument("--tolerance", action="append", default=[], metavar="FIELD=ABS",
                        help="accept an absolute difference in one numeric snapshot field")
    parser.add_argument("--json", help="write the report here")
    args = parser.parse_args(argv)
    tolerances = {}
    for item in args.tolerance:
        name, _, value = item.partition("=")
        try:
            tolerances[name] = float(value)
        except ValueError:
            parser.error("tolerance must be FIELD=ABS: %r" % item)
    expected_metadata, actual_metadata = [], []
    try:
        with open(args.expected) as stream:
            expected = parse(stream.read(), args.expected, expected_metadata)
        with open(args.actual) as stream:
            actual = parse(stream.read(), args.actual, actual_metadata)
        report = compare(expected, actual, args.skip_frames, tolerances)
    except (OSError, CaptureError) as error:
        print("host_frame_capture: %s" % error, file=sys.stderr)
        return 2
    report["expected"], report["actual"] = args.expected, args.actual
    report["expected_metadata"], report["actual_metadata"] = expected_metadata, actual_metadata
    for label, metadata in (("expected", expected_metadata), ("actual", actual_metadata)):
        for entry in metadata:
            print("%s metadata at frame %d: %s" % (label, entry["frame"], entry["line"]))
    if args.json:
        with open(args.json, "w") as stream:
            json.dump(report, stream, indent=2)
    if report["identical"]:
        tolerated = ", ".join("%s: %d" % item for item in sorted(report["tolerated_differences"].items()))
        print("identical: %d frames, %d events%s" % (report["compared_frames"], report["compared_events"],
                                                     " (tolerated %s)" % tolerated if tolerated else ""))
        return 0
    divergence = report["first_divergence"]
    print("divergent at frame %s event %s (after %d identical frames)\n  expected: %s\n  actual:   %s"
          % (divergence["frame"], divergence["event"], report["compared_frames"],
             divergence["expected"], divergence["actual"]))
    return 1


if __name__ == "__main__":
    sys.exit(main())
