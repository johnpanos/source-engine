#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Explain a bounded native render capture without equating draw success with visibility.

Run: python3 tools/quality/render_trace.py --input render.jsonl --out report.json
Exit 0 means a valid diagnostic capture with no observed blocking condition;
1 means observed rendering problems, and 2 means incomplete or invalid evidence.
Neither exit 0 nor a successful draw establishes that a scene was visible.
"""

import argparse
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path
import sys


SCHEMA = "source-render-trace/v1"
REPORT_SCHEMA = "source-render-report/v1"
LIMITATION = ("Successful draw submissions do not prove visible pixels. Occlusion, shader "
              "math, texture contents and final presentation require image evidence.")


class TraceError(ValueError):
    pass


def _object(pairs):
    result = {}
    for name, value in pairs:
        if name in result:
            raise TraceError("duplicate JSON field: " + name)
        result[name] = value
    return result


def _int(event, field, minimum=0):
    value = event.get(field)
    if type(value) is not int or value < minimum:
        raise TraceError("%s requires integer %s >= %d" % (event.get("event"), field, minimum))
    return value


def _text(event, field):
    if not isinstance(event.get(field), str):
        raise TraceError("%s requires string %s" % (event.get("event"), field))


def _shader(value):
    if not isinstance(value, dict):
        raise TraceError("draw requires shader request object")
    _text(value, "name")
    _int(value, "static_index", -2147483648)
    _int(value, "dynamic_index", -2147483648)
    if type(value.get("failed")) is not bool:
        raise TraceError("shader request requires boolean failed")


def _validate(event):
    if not isinstance(event, dict) or not isinstance(event.get("event"), str):
        raise TraceError("each line requires an event object")
    kind = event["event"]
    fields = {
        "capture_begin": ("first_frame", "frame_count", "max_draws"),
        "capture_end": ("frames", "draw_details", "shader_failures"),
        "frame": ("frame", "draws", "primitives", "draw_errors", "failed_shader_requests", "skipped"),
        "shader_failure": ("frame", "dynamic_count"),
        "draw": ("frame", "target_format", "target", "vs_bound", "ps_bound", "texture_mask",
                 "depth_test", "depth_write", "color_write", "cull", "blend", "query_errors"),
        "skipped": ("frame",),
        "present": ("frame",),
    }
    if kind not in fields:
        raise TraceError("unknown event: " + kind)
    for field in fields[kind]:
        _int(event, field)
    if kind == "capture_begin":
        if event.get("schema") != SCHEMA:
            raise TraceError("unsupported capture schema")
        if event["frame_count"] == 0 or event["max_draws"] == 0:
            raise TraceError("capture requested no frame or draw evidence")
    elif kind == "draw":
        for field in ("material", "kind"):
            _text(event, field)
        for field in ("result", "primitives"):
            _int(event, field, -2147483648)
        for field in ("viewport", "target_size"):
            size = event.get(field)
            if not isinstance(size, list) or len(size) != 2 or any(type(x) is not int or x < 0 for x in size):
                raise TraceError("draw requires nonnegative two-element " + field)
        for stage in ("vs", "ps"):
            _shader(event.get(stage + "_requested"))
    elif kind == "shader_failure":
        _text(event, "name")
        _int(event, "static_index", -2147483648)
    elif kind == "skipped":
        _text(event, "material")
        _text(event, "reason")
    elif kind == "present":
        _int(event, "result", -2147483648)


def read_events(path):
    try:
        raw = Path(path).read_bytes()
        text = raw.decode("utf-8")
    except (OSError, UnicodeError) as error:
        raise TraceError("cannot read UTF-8 capture: " + str(error)) from error
    if not text or not text.endswith("\n"):
        raise TraceError("empty or truncated capture (missing final newline)")
    events = []
    for line, content in enumerate(text.splitlines(), 1):
        try:
            event = json.loads(content, object_pairs_hook=_object)
            _validate(event)
        except (json.JSONDecodeError, TraceError) as error:
            raise TraceError("line %d: %s" % (line, error)) from error
        events.append(event)
    return events, hashlib.sha256(raw).hexdigest()


def analyze(events):
    if not events:
        raise TraceError("empty capture")
    for event in events:
        _validate(event)
    if events[0]["event"] != "capture_begin" or events[-1]["event"] != "capture_end":
        raise TraceError("capture requires initial capture_begin and terminal capture_end")
    if any(e["event"] in ("capture_begin", "capture_end") for e in events[1:-1]):
        raise TraceError("nested capture or data after capture_end")
    begin, end = events[0], events[-1]
    frames = [e for e in events if e["event"] == "frame"]
    draws = [e for e in events if e["event"] == "draw"]
    presents = [e for e in events if e["event"] == "present"]
    failures = [e for e in events if e["event"] == "shader_failure"]
    if not frames or len(frames) != end["frames"] or any(e["frame"] != index for index, e in enumerate(frames, 1)):
        raise TraceError("missing, duplicate or out-of-order frame summaries")
    if end["draw_details"] != len(draws) or len(draws) > begin["max_draws"]:
        raise TraceError("draw detail count contradicts capture bounds or terminal count")
    if len(failures) != min(end["shader_failures"], 256):
        raise TraceError("shader failure count contradicts bounded terminal count")
    if not draws:
        raise TraceError("no draw details; frame summaries report %d primitives" %
                         sum(e["primitives"] for e in frames))
    if not presents:
        raise TraceError("capture contains no presentation evidence")
    frame_by_id = {e["frame"]: e for e in frames}
    detail_by_frame = defaultdict(list)
    for event in events[1:-1]:
        if event.get("frame", 0) > end["frames"]:
            raise TraceError("event references a frame beyond capture_end")
    for draw in draws:
        number = draw["frame"]
        if number not in frame_by_id or not begin["first_frame"] <= number < begin["first_frame"] + begin["frame_count"]:
            raise TraceError("draw lies outside captured frame interval")
        detail_by_frame[number].append(draw)
    for number, details in detail_by_frame.items():
        summary = frame_by_id[number]
        checks = ((len(details), "draws"),
                  (sum(max(e["primitives"], 0) for e in details), "primitives"),
                  (sum(e["result"] < 0 for e in details), "draw_errors"),
                  (sum(e["vs_requested"]["failed"] or e["ps_requested"]["failed"] for e in details),
                   "failed_shader_requests"))
        if any(actual > summary[key] for actual, key in checks):
            raise TraceError("draw details exceed frame %d summary" % number)

    materials, targets, observations = {}, {}, {}
    prior_bindings = {"vs": {}, "ps": {}}

    def observe(code, event, stage=None, previous=None, severity="problem"):
        material = event.get("material", "")
        key = (code, material, stage)
        if key not in observations:
            first = {"frame": event.get("frame", 0)}
            for field in ("kind", "primitives", "result", "viewport", "target_size", "color_write", "query_errors"):
                if field in event:
                    first[field] = event[field]
            if stage:
                first["requested"] = event[stage + "_requested"]
                first["bound"] = event[stage + "_bound"]
                if previous:
                    first["previous_request_at_bound_pointer"] = previous
            observations[key] = {"code": code, "material": material, "stage": stage,
                                 "severity": severity, "count": 0, "first": first}
            if code == "no_color_writes":
                observations[key]["explanation"] = "May be an intentional depth or occlusion pass."
            elif code == "no_geometry":
                observations[key]["explanation"] = "This submission contains no primitives; the pass may intentionally be empty."
        observations[key]["count"] += 1

    for event in events[1:-1]:
        kind = event["event"]
        if kind == "present":
            if event["result"] < 0:
                observe("present_error", event)
            continue
        if kind not in ("draw", "skipped"):
            continue
        name = event["material"]
        material = materials.setdefault(name, {"draws": 0, "primitives": 0, "skipped": 0,
            "skip_reasons": Counter(), "targets": set(), "texture_masks": set(),
            "shader_requests": {"vs": set(), "ps": set()}})
        if kind == "skipped":
            material["skipped"] += 1
            material["skip_reasons"][event["reason"]] += 1
            continue
        material["draws"] += 1
        material["primitives"] += max(event["primitives"], 0)
        material["targets"].add(event["target"])
        material["texture_masks"].add(event["texture_mask"])
        target = targets.setdefault(str(event["target"]), {"draws": 0, "sizes": set(),
            "formats": set(), "materials": set()})
        target["draws"] += 1
        target["sizes"].add(tuple(event["target_size"]))
        target["formats"].add(event["target_format"])
        target["materials"].add(name)
        state_known = event["query_errors"] == 0
        conditions = ((event["primitives"] <= 0, "no_geometry", "observation"),
                      (not state_known, "state_query_unavailable", "problem"),
                      (state_known and 0 in event["viewport"], "zero_viewport", "problem"),
                      (state_known and (0 in event["target_size"] or not event["target"]), "missing_target", "problem"),
                      (event["result"] < 0, "draw_error", "problem"),
                      (state_known and event["color_write"] == 0, "no_color_writes", "observation"))
        for condition, code, severity in conditions:
            if condition:
                observe(code, event, severity=severity)
        for stage in ("vs", "ps"):
            requested, bound = event[stage + "_requested"], event[stage + "_bound"]
            identity = (requested["name"], requested["static_index"], requested["dynamic_index"])
            material["shader_requests"][stage].add(identity)
            if not state_known:
                continue
            previous = prior_bindings[stage].get(bound)
            if requested["failed"]:
                observe("failed_request_with_bound_shader" if bound else "failed_request_without_shader",
                        event, stage, previous)
                if bound and previous and identity != (previous["name"], previous["static_index"], previous["dynamic_index"]):
                    observe("stale_shader_binding", event, stage, previous)
            elif requested["name"] and not bound:
                observe("requested_shader_unbound", event, stage)
            elif bound and requested["name"]:
                prior_bindings[stage][bound] = dict(requested)

    for material in materials.values():
        material["skip_reasons"] = dict(sorted(material["skip_reasons"].items()))
        for key in ("targets", "texture_masks"):
            material[key] = sorted(material[key])
        for stage in ("vs", "ps"):
            material["shader_requests"][stage] = [dict(zip(("name", "static_index", "dynamic_index"), value))
                                                    for value in sorted(material["shader_requests"][stage])]
    for target in targets.values():
        for key in ("sizes", "formats", "materials"):
            target[key] = sorted(target[key])
    observed = sorted(observations.values(), key=lambda e: (e["severity"], e["code"], e["material"], e["stage"] or ""))
    selected = [e for e in frames if begin["first_frame"] <= e["frame"] < begin["first_frame"] + begin["frame_count"]]
    total_errors = sum(e["draw_errors"] for e in frames)
    total_failed = sum(e["failed_shader_requests"] for e in frames)
    issues = bool(total_errors or total_failed or end["shader_failures"] or
                  any(e["severity"] == "problem" for e in observed))
    return {"schema": REPORT_SCHEMA, "status": "issues_observed" if issues else "complete",
            "visibility_verified": False, "limitation": LIMITATION,
            "coverage": {"frames": len(frames), "draw_details": len(draws),
                "selected_frames": len(selected), "requested_frames": begin["frame_count"],
                "detail_limit_reached": len(draws) == begin["max_draws"],
                "undetailed_selected_draws": sum(e["draws"] for e in selected) - len(draws)},
            "totals": {"draws": sum(e["draws"] for e in frames),
                "primitives": sum(e["primitives"] for e in frames), "draw_errors": total_errors,
                "failed_shader_requests": total_failed, "shader_load_failures": end["shader_failures"],
                "skipped": sum(e["skipped"] for e in frames), "presents": len(presents),
                "present_errors": sum(e["result"] < 0 for e in presents)},
            "observations": observed,
            "actionable_errors": [e for e in observed if e["severity"] == "problem"],
            "materials": dict(sorted(materials.items())),
            "targets": targets, "shader_load_failures": failures}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    args = parser.parse_args(argv)
    try:
        events, digest = read_events(args.input)
        report = analyze(events)
        report["input"] = {"path": str(args.input.resolve()), "sha256": digest}
        code = 1 if report["status"] == "issues_observed" else 0
    except TraceError as error:
        report = {"schema": REPORT_SCHEMA, "status": "invalid", "visibility_verified": False,
                  "error": str(error), "limitation": LIMITATION}
        code = 2
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")
    print("Render trace: " + report["status"])
    if code == 2:
        print(report["error"], file=sys.stderr)
    else:
        totals = report["totals"]
        print("%d draws, %d primitives, %d materials, %d presents; %d draw errors, %d failed shader requests" %
              (totals["draws"], totals["primitives"], len(report["materials"]), totals["presents"],
               totals["draw_errors"], totals["failed_shader_requests"]))
        for item in report["observations"]:
            print("  %s: %s%s (%d)" % (item["material"] or "<frame>", item["code"],
                  " [" + item["stage"] + "]" if item["stage"] else "", item["count"]))
    print(LIMITATION)
    print("Report: " + str(args.out))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
