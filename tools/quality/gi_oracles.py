#!/usr/bin/env python3
"""Declarative relational oracles for the RFC 0011 GI fixtures.

A fixture's `fixture.json` may declare `oracles`: checks over the per-region
light means of its rendered views (Cycles references, or any other source of
the same `{region: {light: [r, g, b]}}` statistics). Each oracle names the
state, camera, region and light (`total`, `indirect`, `direct`, ...) it reads.
Kinds:

  superposition  light transport is linear in emission: a state with several
                 lights on equals the sum of the states with each part on
                 alone, per channel
  equal          two (state, camera, region) means agree after a channel
                 mapping: mirror symmetry, a light moved to its mirror
                 position, colors swapped between mirrored lights
  value          a mean equals a closed-form RGB value
  zero           selected channels are exactly dark: a sealed room, a color
                 a light does not emit, a switched-off scene (`absolute`
                 widens the bound for quantized captures)
  dominant       one channel exceeds others by a factor (color bleeding)
  tint_order     a channel ratio decreases along a list of regions
  ordered        luminance increases along a list of regions by a factor
  scale          a state equals another times a factor (intensity ladders)
  chromaticity   a mean's color (divided by its largest channel) is fixed
  uniform        several (state, camera, region) means agree per channel (the
                 walls of an integrating sphere)

Every oracle also has a seeded negative control: its statistics or its
declaration perturbed the way a real producer defect would (a dropped light,
a luminance-only producer, a 1% leak, a mirror mapping left out, a biased
closed form), and the control must fail. `evaluate` reports both.

Comparisons are per channel: |observed - expected| <= relative * scale +
absolute. Relations between renders (superposition, equal, scale) scale each
channel by its own level, floored at 5% of the colors' brightest channel so a
nearly empty channel of a saturated color is not judged by noise alone; a
closed form (value) scales by its largest expected channel.
"""

import copy

KINDS = ("superposition", "equal", "value", "zero", "dominant", "tint_order", "ordered",
         "scale", "chromaticity", "uniform")
LIGHTS = ("total", "indirect", "direct", "albedo", "emission", "environment")
CHANNELS = "RGB"
# Default tolerances. Region means of 2048-sample references carry well under
# 1% noise; sums and ratios of independent renders need a little more.
DEFAULT_RELATIVE = {"superposition": 0.02, "equal": 0.02, "value": 0.01, "scale": 0.02,
                    "chromaticity": 0.02, "uniform": 0.03}
# A `zero` channel is exactly dark in an unbiased render (no path carries
# light of that color); the bound only absorbs float summation.
ZERO_ABSOLUTE = 1e-6
ABSOLUTE_FLOOR = 1e-6
CHANNEL_FLOOR = 0.05


def luminance(rgb):
    return 0.2126 * rgb[0] + 0.7152 * rgb[1] + 0.0722 * rgb[2]


class Views:
    """Region means by (state, camera): {(state, camera): {region: {light: rgb}}}."""

    def __init__(self, views, transform=None):
        self.views = views
        self.transform = transform

    def get(self, state, camera, region, light):
        view = self.views.get((state, camera))
        if view is None:
            raise KeyError("no view %s.%s" % (state, camera))
        if region not in view:
            raise KeyError("view %s.%s has no region %s" % (state, camera, region))
        if light not in view[region]:
            raise KeyError("view %s.%s region %s has no %s light" % (state, camera, region, light))
        value = [float(v) for v in view[region][light]]
        if self.transform:
            value = self.transform(state, camera, region, light, value)
        return value


def channel_scale(a, b, c):
    """The level channel `c` of two compared colors is judged against: its own
    level, but at least CHANNEL_FLOOR of the colors' brightest channel, so a
    nearly empty channel of a saturated color is not judged by noise alone."""
    return max(abs(a[c]), abs(b[c]), CHANNEL_FLOOR * max(max(map(abs, a)), max(map(abs, b))))


def close(observed, expected, relative, absolute=ABSOLUTE_FLOOR, scale=None):
    scale = max(abs(v) for v in expected) if scale is None else scale
    return all(abs(o - e) <= relative * scale + absolute for o, e in zip(observed, expected))


def refs(oracle, key="entries"):
    """(state, camera, region) references an oracle reads, for validation."""
    kind = oracle["kind"]
    if kind == "superposition":
        return [(s, oracle["camera"], r) for s in [oracle["sum"]] + oracle["parts"]
                for r in oracle["regions"]]
    if kind == "equal":
        return [(e["state"], e["camera"], e["region"]) for e in (oracle["a"], oracle["b"])]
    if kind == "scale":
        return [(s, oracle["camera"], r) for s in (oracle["base"], oracle["scaled"])
                for r in oracle["regions"]]
    if kind in ("tint_order", "ordered", "uniform"):
        return [(e["state"], e["camera"], e["region"]) for e in oracle[key]]
    return [(oracle["state"], oracle["camera"], r) for r in oracle_regions(oracle)]


def oracle_regions(oracle):
    return oracle["regions"] if "regions" in oracle else [oracle["region"]]


def lights_of(oracle):
    return oracle.get("lights") or [oracle.get("light", "total")]


def describe(entry):
    return "%s.%s/%s" % (entry["state"], entry["camera"], entry["region"])


def run(oracle, views):
    """[{check, expected, observed, ok}] with one scalar check per channel or
    relation, so reports stay in the analytic checks' format."""
    kind = oracle["kind"]
    name = oracle.get("name") or kind
    relative = oracle.get("tolerance", DEFAULT_RELATIVE.get(kind, 0.02))
    checks = []

    def add(label, expected, observed, ok):
        checks.append({"check": "%s: %s" % (name, label), "expected": float(expected),
                       "observed": float(observed), "ok": bool(ok)})

    if kind == "superposition":
        for light in lights_of(oracle):
            for region in oracle["regions"]:
                total = views.get(oracle["sum"], oracle["camera"], region, light)
                parts = [views.get(state, oracle["camera"], region, light)
                         for state in oracle["parts"]]
                summed = [sum(p[c] for p in parts) for c in range(3)]
                for c in range(3):
                    add("%s %s %s %s = sum of %s" % (oracle["camera"], region, light,
                                                     CHANNELS[c], "+".join(oracle["parts"])),
                        total[c], summed[c],
                        close([summed[c]], [total[c]], relative,
                              scale=channel_scale(total, summed, c)))
    elif kind == "equal":
        a, b = oracle["a"], oracle["b"]
        for light in lights_of(oracle):
            va = views.get(a["state"], a["camera"], a["region"], light)
            vb = views.get(b["state"], b["camera"], b["region"], light)
            ca, cb = a.get("channels", [0, 1, 2]), b.get("channels", [0, 1, 2])
            left, right = [va[c] for c in ca], [vb[c] for c in cb]
            for i, (x, y) in enumerate(zip(left, right)):
                # The floor refers to each side's whole color, not just the
                # compared channels.
                scale = max(abs(x), abs(y), CHANNEL_FLOOR * max(max(map(abs, va)),
                                                                max(map(abs, vb))))
                add("%s %s %s = %s %s" % (describe(a), light, CHANNELS[ca[i]], describe(b),
                                          CHANNELS[cb[i]]),
                    x, y, close([y], [x], relative, scale=scale))
    elif kind == "value":
        for light in lights_of(oracle):
            expected = oracle["expected"][light] if isinstance(oracle["expected"], dict) \
                else oracle["expected"]
            for region in oracle_regions(oracle):
                observed = views.get(oracle["state"], oracle["camera"], region, light)
                scale = max(abs(v) for v in expected)
                for c in range(3):
                    add("%s.%s/%s %s %s = closed form" % (oracle["state"], oracle["camera"],
                                                          region, light, CHANNELS[c]),
                        expected[c], observed[c],
                        close([observed[c]], [expected[c]], relative, scale=scale))
    elif kind == "zero":
        for light in lights_of(oracle):
            for region in oracle_regions(oracle):
                observed = views.get(oracle["state"], oracle["camera"], region, light)
                for c in oracle.get("channels", [0, 1, 2]):
                    add("%s.%s/%s %s %s is dark" % (oracle["state"], oracle["camera"], region,
                                                    light, CHANNELS[c]),
                        0.0, observed[c],
                        abs(observed[c]) <= oracle.get("absolute", ZERO_ABSOLUTE))
    elif kind == "dominant":
        factor = oracle["factor"]
        for light in lights_of(oracle):
            for region in oracle_regions(oracle):
                observed = views.get(oracle["state"], oracle["camera"], region, light)
                c = oracle["channel"]
                for other in oracle["over"]:
                    add("%s.%s/%s %s %s >= %g x %s" % (
                        oracle["state"], oracle["camera"], region, light, CHANNELS[c], factor,
                        CHANNELS[other]),
                        factor, observed[c] / max(observed[other], 1e-12),
                        observed[c] > 0 and observed[c] >= factor * observed[other])
    elif kind == "tint_order":
        factor = oracle.get("factor", 1.0)
        c, d = oracle["channel"], oracle["over"]
        for light in lights_of(oracle):
            ratios = []
            for entry in oracle["entries"]:
                value = views.get(entry["state"], entry["camera"], entry["region"], light)
                ratios.append(value[c] / max(value[d], 1e-12))
            for i in range(len(ratios) - 1):
                add("%s %s %s/%s >= %g x that of %s" % (
                    describe(oracle["entries"][i]), light, CHANNELS[c], CHANNELS[d], factor,
                    describe(oracle["entries"][i + 1])),
                    factor, ratios[i] / max(ratios[i + 1], 1e-12),
                    ratios[i] >= factor * ratios[i + 1])
    elif kind == "ordered":
        factor = oracle.get("factor", 1.0)
        for light in lights_of(oracle):
            levels = [luminance(views.get(e["state"], e["camera"], e["region"], light))
                      for e in oracle["entries"]]
            for i in range(len(levels) - 1):
                add("%s %s luminance >= %g x %s" % (
                    describe(oracle["entries"][i + 1]), light, factor,
                    describe(oracle["entries"][i])),
                    factor, levels[i + 1] / max(levels[i], 1e-12),
                    levels[i] > 0 and levels[i + 1] >= factor * levels[i])
    elif kind == "scale":
        for light in lights_of(oracle):
            for region in oracle["regions"]:
                base = views.get(oracle["base"], oracle["camera"], region, light)
                scaled = views.get(oracle["scaled"], oracle["camera"], region, light)
                expected = [oracle["factor"] * v for v in base]
                for c in range(3):
                    add("%s %s %s %s = %g x %s" % (oracle["camera"], region, light, CHANNELS[c],
                                                   oracle["factor"], oracle["base"]),
                        expected[c], scaled[c],
                        close([scaled[c]], [expected[c]], relative,
                              scale=channel_scale(expected, scaled, c)))
    elif kind == "chromaticity":
        expected = normalized(oracle["expected"])
        for light in lights_of(oracle):
            for region in oracle_regions(oracle):
                observed = normalized(views.get(oracle["state"], oracle["camera"], region,
                                                light))
                for c in range(3):
                    add("%s.%s/%s %s chromaticity %s" % (oracle["state"], oracle["camera"],
                                                         region, light, CHANNELS[c]),
                        expected[c], observed[c],
                        close([observed[c]], [expected[c]], relative, scale=1.0))
    elif kind == "uniform":
        for light in lights_of(oracle):
            values = [views.get(e["state"], e["camera"], e["region"], light)
                      for e in oracle["entries"]]
            mean = [sum(v[c] for v in values) / len(values) for c in range(3)]
            scale = max(mean)
            for entry, value in zip(oracle["entries"], values):
                for c in range(3):
                    add("%s %s %s = mean of %d regions" % (describe(entry), light, CHANNELS[c],
                                                           len(values)),
                        mean[c], value[c], close([value[c]], [mean[c]], relative, scale=scale))
    else:
        raise ValueError("unknown GI oracle kind " + str(kind))
    if not checks:
        raise ValueError("GI oracle %s produced no checks" % name)
    return checks


def normalized(rgb):
    peak = max(rgb)
    return [v / peak for v in rgb] if peak > 0 else [0.0, 0.0, 0.0]


def grey(state, camera, region, light, value):
    """A luminance-only producer: every channel carries the luminance."""
    level = luminance(value)
    return [level, level, level]


def control(oracle):
    """(description, perturbed oracle, statistics transform) that a working
    oracle must reject."""
    kind = oracle["kind"]
    perturbed = copy.deepcopy(oracle)
    if kind == "superposition":
        dropped = perturbed["parts"].pop()
        return "sum state missing part %s (a dropped light)" % dropped, perturbed, None
    if kind == "equal":
        b = perturbed["b"]
        target = (b["state"], b["camera"], b["region"])
        channels = set(b.get("channels", [0, 1, 2]))

        def biased(state, camera, region, light, value):
            if (state, camera, region) != target:
                return value
            return [v * 1.08 if c in channels else v for c, v in enumerate(value)]
        return "compared channels of one side biased 8%", perturbed, biased
    if kind in ("value", "scale"):
        bias = 1 + 5 * perturbed.get("tolerance", DEFAULT_RELATIVE[kind])
        if kind == "scale":
            perturbed["factor"] *= bias
            return "factor off by %.0f%%" % (100 * (bias - 1)), perturbed, None
        perturbed["expected"] = ({k: [v * bias for v in e] for k, e in
                                  perturbed["expected"].items()}
                                 if isinstance(perturbed["expected"], dict)
                                 else [v * bias for v in perturbed["expected"]])
        return "closed form off by %.0f%%" % (100 * (bias - 1)), perturbed, None
    if kind == "zero":
        bound = oracle.get("absolute", ZERO_ABSOLUTE)

        def leak(state, camera, region, light, value):
            return [v + max(0.01 * max(max(value), 1e-3), 3 * bound) for v in value]
        return "leak of 1% of the region's light (at least 3x the bound)", perturbed, leak
    if kind in ("dominant", "tint_order"):
        return "luminance-only producer", perturbed, grey
    if kind == "ordered":
        perturbed["entries"].reverse()
        return "order reversed", perturbed, None
    if kind == "chromaticity":
        if max(oracle["expected"]) - min(oracle["expected"]) > 0.05 * max(oracle["expected"]):
            return "luminance-only producer", perturbed, grey
        perturbed["expected"] = [oracle["expected"][0] * 0.9] + oracle["expected"][1:]
        return "red chromaticity off by 10%", perturbed, None
    if kind == "uniform":
        first = oracle["entries"][0]
        target = (first["state"], first["camera"], first["region"])

        def lifted(state, camera, region, light, value):
            return [v * 1.15 for v in value] if (state, camera, region) == target else value
        return "one region 15% brighter", perturbed, lifted
    raise ValueError("unknown GI oracle kind " + str(kind))


def validate(fixture):
    """Problems with a fixture's oracle declarations against its states,
    cameras and regions (independent of any render)."""
    problems = []
    for index, oracle in enumerate(fixture.get("oracles", [])):
        label = oracle.get("name") or "oracle %d" % index
        if oracle.get("kind") not in KINDS:
            problems.append("%s: unknown kind %s" % (label, oracle.get("kind")))
            continue
        for light in lights_of(oracle):
            if light not in LIGHTS:
                problems.append("%s: unknown light %s" % (label, light))
        for state, camera, region in refs(oracle):
            if state not in fixture["states"]:
                problems.append("%s: unknown state %s" % (label, state))
            if camera not in fixture["cameras"]:
                problems.append("%s: unknown camera %s" % (label, camera))
            elif region not in fixture["regions"].get(camera, {}):
                problems.append("%s: camera %s has no region %s" % (label, camera, region))
    return problems


def mapping_matters(oracle, views):
    """Whether an `equal` oracle with a channel mapping would fail without it:
    whether the scene's colors make the mirror's channel swap observable."""
    a, b = oracle["a"], oracle["b"]
    if a.get("channels", [0, 1, 2]) == b.get("channels", [0, 1, 2]):
        return None
    unmapped = copy.deepcopy(oracle)
    unmapped["b"]["channels"] = a.get("channels", [0, 1, 2])
    return not all(c["ok"] for c in run(unmapped, Views(views)))


def evaluate(fixture, views):
    """[{oracle, kind, checks, ok, control: {description, rejected}}] for every
    declared oracle over `views` ({(state, camera): {region: {light: rgb}}}).
    An oracle whose statistics are missing (a region no pixel covers) is one
    failed check naming what is missing."""
    results = []
    for index, oracle in enumerate(fixture.get("oracles", [])):
        name = oracle.get("name") or "%s %d" % (oracle["kind"], index)
        try:
            checks = run(oracle, Views(views))
            description, perturbed, transform = control(oracle)
            rejected = not all(c["ok"] for c in run(perturbed, Views(views, transform)))
        except KeyError as error:
            checks = [{"check": "%s: statistics present" % name, "expected": 1.0,
                       "observed": 0.0, "ok": False, "error": str(error.args[0])}]
            description, rejected = "not evaluated", False
        result = {"oracle": name, "kind": oracle["kind"], "checks": checks,
                  "ok": all(c["ok"] for c in checks),
                  "control": {"description": description, "rejected": rejected}}
        if oracle["kind"] == "equal" and result["ok"]:
            result["channel_mapping_observable"] = mapping_matters(oracle, views)
        results.append(result)
    return results


def views_from_references(record):
    """{(state, camera): regions} from a gi-references/v1 record."""
    return {(view["state"], view["camera"]): view["regions"]
            for view in record["views"].values()}


def for_capture(fixture, state, relative, absolute):
    """The fixture's oracles a single captured state's indirect light can be
    judged by: those reading only `state`, restricted to the indirect light,
    with relative tolerances widened to `relative` and zero bounds to
    `absolute` (an 8-bit capture is not an unbiased render)."""
    selected = []
    for oracle in fixture.get("oracles", []):
        if oracle["kind"] == "superposition" or "indirect" not in lights_of(oracle):
            continue
        if any(ref[0] != state for ref in refs(oracle)):
            continue
        adapted = copy.deepcopy(oracle)
        adapted.pop("light", None)
        adapted["lights"] = ["indirect"]
        if adapted["kind"] == "value":
            if not isinstance(adapted["expected"], dict) or "indirect" not in \
                    adapted["expected"]:
                continue
            adapted["expected"] = {"indirect": adapted["expected"]["indirect"]}
        if adapted["kind"] in DEFAULT_RELATIVE:
            adapted["tolerance"] = max(adapted.get("tolerance", DEFAULT_RELATIVE[
                adapted["kind"]]), relative)
        if adapted["kind"] == "zero":
            adapted["absolute"] = absolute
        selected.append(adapted)
    return selected


def summary(results):
    checks = sum(len(r["checks"]) for r in results)
    failed = [c for r in results for c in r["checks"] if not c["ok"]]
    unrejected = [r["oracle"] for r in results if not r["control"]["rejected"]]
    return {"oracles": len(results), "checks": checks, "failed": len(failed),
            "controls_unrejected": unrejected}


if __name__ == "__main__":
    import json
    import sys
    from pathlib import Path
    fixture = json.loads(Path(sys.argv[1]).read_text())
    record = json.loads((Path(sys.argv[1]).parent / "references" / "references.json").read_text())
    for result in evaluate(fixture, views_from_references(record)):
        bad = [c for c in result["checks"] if not c["ok"]]
        print("%-4s %-50s %3d checks, control %s%s" % (
            "ok" if result["ok"] else "FAIL", result["oracle"], len(result["checks"]),
            "rejected" if result["control"]["rejected"] else "NOT REJECTED",
            "".join("\n       %s: observed %.5g expected %.5g" % (c["check"], c["observed"],
                                                                    c["expected"])
                    for c in bad[:6])))
