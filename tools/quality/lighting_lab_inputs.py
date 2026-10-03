#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Shared fixture inputs for the matched game/render_lab capture."""

import os
import re
from pathlib import Path

import lighting_fixtures as lf


def lab_environment(lab):
    # render_lab needs tier0 from its own tree: <tree>/render/lab/render_lab.
    tree = lab.parents[2]
    env = dict(os.environ)
    env["LD_LIBRARY_PATH"] = str(tree / "tier0") + (
        ":" + env["LD_LIBRARY_PATH"] if env.get("LD_LIBRARY_PATH") else "")
    return env


def probe_model(fixture, state=None):
    """The fixture's dynamic model for render_lab, in Source units."""
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
    origin = [float(v) * lf.SOURCE_UNITS_PER_METER for v in match.group(1).split(",")]
    entry = lf.map_for(fixture, state)
    game = str((lf.ROOT / entry["bsp"]).resolve()).rsplit("/maps/", 1)[0]
    published = "models/%s/probesphere.mdl" % entry["name"]
    model = published if (Path(game) / published).is_file() else probe["model"]
    return model, origin


def fog_scale(fixture, state):
    """Zero the medium in a clear state of a fixture with another foggy state."""
    media = fixture["lighting"].get("media") or {}
    if any(media.values()) and not media.get(state):
        return 0.0
    return None
