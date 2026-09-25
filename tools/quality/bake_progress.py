#!/usr/bin/env python3
"""Readable progress from a Blender/Cycles bake's log, for the map pipeline.

`pbrt_map_build.py` runs Blender with Cycles' debug log (`--log-level debug
--log cycles`) and feeds every output line here. Cycles logs when it starts a
bake (`Using tile size of (w, h)`), each finished sample batch (`Rendered N
samples in T seconds (...)`) and each finished tile (`Write tile result.`).
A bake script may name its passes with a line

    PROGRESS {"event": "bake", "label": "direct", "size": [4096, 4096], "samples": 1024}

before `bpy.ops.object.bake` (`progress_line` writes one). With the image size
the tile count is known, and with the samples the fraction of a tile done:
adaptive sampling may finish a tile early, so the fraction never overstates.
Other `PROGRESS {"message": ...}` lines are passed through. `CyclesProgress.feed`
returns the lines to show, throttled to one per `interval` seconds within a
tile.
"""

import json
import math
import re
import time

TILE_SIZE = re.compile(r"Using tile size of \((\d+), (\d+)\)")
RENDERED = re.compile(r"Rendered (\d+) samples in ([0-9.]+) seconds \(")
# Logged once per finished tile, whether the bake has one tile or writes
# several to disk (then `Write tile at x, y` follows).
TILE_WRITTEN = re.compile(r"Write tile result\.$")
PREFIX = "PROGRESS "


def progress_line(**record):
    """The line a bake script prints to label its next step or pass."""
    return PREFIX + json.dumps(record, sort_keys=True)


def duration(seconds):
    seconds = int(round(seconds))
    if seconds < 60:
        return "%ds" % seconds
    if seconds < 3600:
        return "%dm%02ds" % divmod(seconds, 60)
    hours, rest = divmod(seconds, 3600)
    return "%dh%02dm" % (hours, rest // 60)


class CyclesProgress:
    def __init__(self, clock=time.monotonic, interval=10.0):
        self.clock = clock
        self.interval = interval
        self.passes = 0
        self.pending = {}
        self.label = None
        self.size = None
        self.total_samples = None
        self.tiles = None
        self.done = 0
        self.samples = 0
        self.started = None
        self.last_shown = None

    def feed(self, line):
        """Messages (without the step prefix) that `line` produces."""
        text = line.strip()
        if text.startswith(PREFIX):
            try:
                record = json.loads(text[len(PREFIX):])
            except ValueError:
                return [text[len(PREFIX):]]
            if record.get("event") == "bake":
                self.pending = record
                return []
            return [str(record.get("message", record))]
        match = TILE_SIZE.search(text)
        if match:
            self.passes += 1
            # A label names the one bake that follows it.
            self.label, self.size = self.pending.get("label"), self.pending.get("size")
            self.total_samples = self.pending.get("samples")
            self.pending = {}
            tile = int(match.group(1)), int(match.group(2))
            self.tiles = (math.ceil(self.size[0] / tile[0]) * math.ceil(self.size[1] / tile[1])
                          if self.size else None)
            self.done = 0
            self.samples = 0
            self.started = self.last_shown = self.clock()
            return ["%s started (%s)" % (self.name(), "%d tiles" % self.tiles if self.tiles
                                         else "tile %dx%d" % tile)]
        if self.started is None:
            return []
        match = RENDERED.search(text)
        if match:
            self.samples += int(match.group(1))
            now = self.clock()
            if now - self.last_shown < self.interval:
                return []
            self.last_shown = now
            elapsed = now - self.started
            if self.tiles and self.total_samples:
                fraction = (self.done + min(1.0, self.samples / self.total_samples)) / self.tiles
                return ["%s: tile %d/%d, %d/%d samples (%d%%), %s elapsed, ~%s left" % (
                    self.name(), self.done + 1, self.tiles, self.samples, self.total_samples,
                    100 * fraction, duration(elapsed), duration(elapsed / fraction - elapsed))]
            return ["%s: tile %d%s, %d samples, %s elapsed" % (
                self.name(), self.done + 1, "/%d" % self.tiles if self.tiles else "",
                self.samples, duration(elapsed))]
        if TILE_WRITTEN.search(text):
            self.done += 1
            self.samples = 0
            now = self.clock()
            self.last_shown = now
            elapsed = now - self.started
            if self.tiles:
                left = elapsed / self.done * (self.tiles - self.done)
                message = "%s: tile %d/%d (%d%%), %s elapsed, ~%s left" % (
                    self.name(), self.done, self.tiles, 100 * self.done // self.tiles,
                    duration(elapsed), duration(left))
            else:
                message = "%s: tile %d done, %s elapsed" % (self.name(), self.done,
                                                           duration(elapsed))
            if self.tiles and self.done >= self.tiles:
                self.started = None
            return [message]
        return []

    def name(self):
        return "pass %d%s" % (self.passes, " (%s)" % self.label if self.label else "")
