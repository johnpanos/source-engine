#!/usr/bin/env python3
"""Portal 2 audio conformance against retail Portal 2.

The workload (quality/workloads/portal2-audio-v1) runs one VScript driver on
sp_a2_triple_laser. It plays soundscript entries (dialog, music, a chained
music entry, weapon and world sounds at three distances) and prints an
AUDIO_MARK line with the server time of each. The game mixes into SDL's disk
audio driver, so the capture is the exact stereo output of the mixer.

    measure  locate every event in a capture by normalized cross-correlation
             against the retail wave (decoded with ffmpeg from the installed
             VPKs), and derive levels, stereo image, falloff, ducking, the
             cut of a replaced line, the reverb tail and silence
    record   store a retail capture's metrics as the reference (numbers only;
             no content leaves the Steam installation)
    check    capture this build (or read --capture) and compare its metrics
             with the reference; prints one checks-v1 record
    capture  run this build or retail headless and keep the raw capture

Retail content is required (the waves are the oracle), so the suite is
optional and never certified without it. `check --seed-defect` applies a
deliberate defect to the capture (mute the right channel, remove the music) to
show the comparison rejects it; `capture --seed swap-line` swaps the two
dialog lines in the driver.

Reproduce the retail reference (32-bit retail binary, headless mutter with
Xwayland, the SDL2 disk driver; see game/shared/portal2/README.md):

    python3 tools/quality/portal2_audio.py capture --side retail --out <dir> \\
        --retail-libs <dir with 32-bit libbz2.so.1.0 and libpng12.so.0>
    python3 tools/quality/portal2_audio.py record --capture <dir>
"""

import argparse
import datetime
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import conformance_result  # noqa: E402

SCHEMA = "portal2-audio/v1"
METRICS_SCHEMA = "portal2-audio-metrics/v1"
REFERENCE_SCHEMA = "portal2-audio-reference/v1"
ROOT = Path(__file__).resolve().parents[2]
DEFAULT_WORKLOAD = ROOT / "quality/workloads/portal2-audio-v1/audio.json"
DEFAULT_STEAM_ROOT = Path.home() / ".local/share/Steam/steamapps/common/Portal 2"
RATE = 44100
DECIMATE = 4
LOW_RATE = RATE // DECIMATE
SILENT_DB = -120.0
QUIET_POWER = 1e-8
MARK_LINE = re.compile(r"AUDIO_MARK (\S+) (-?[\d.]+)")
CHANNELS_LINE = re.compile(r"channels=(\d+) freq=(\d+)")
SEEDED_DEFECTS = ("mute-right", "drop-music")
# Search order of the retail gameinfo.txt: update, DLC, then the base game.
CONTENT_DIRECTORIES = ("update", "portal2_dlc2", "portal2_dlc1", "portal2")
SOUND_PREFIX_CHARACTERS = "*#@)(^}<>!?$&~`+%"


class AudioError(Exception):
    pass


def numpy_modules():
    try:
        import numpy
        from scipy import signal as scipy_signal
    except ImportError as error:
        raise AudioError("numpy and scipy are required: %s" % error)
    return numpy, scipy_signal


# --- Workload -----------------------------------------------------------------

def load_workload(path):
    path = Path(path)
    try:
        workload = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as error:
        raise AudioError("cannot read %s: %s" % (path, error))
    if workload.get("schema") != SCHEMA:
        raise AudioError("%s: schema must be %s" % (path, SCHEMA))
    for key in ("driver", "script"):
        if not (path.parent / str(workload.get(key, ""))).is_file():
            raise AudioError("%s: %s script is missing" % (path, key))
    marks = workload.get("marks")
    if not isinstance(marks, list) or len(set(marks)) != len(marks) or not marks:
        raise AudioError("%s: marks must be a list of unique labels" % path)
    events = workload.get("events", {})
    for label, event in events.items():
        if event.get("mark", label) not in marks or not isinstance(event.get("wave"), str):
            raise AudioError("%s: event %s needs a declared mark and a wave" % (path, label))
    windows = workload.get("windows", {})
    for label, window in windows.items():
        if window.get("from") not in marks or window.get("to") not in marks:
            raise AudioError("%s: window %s must span declared marks" % (path, label))
    names = set()
    for check in workload.get("checks", []):
        name = check.get("name", "")
        if not re.fullmatch(r"[a-z0-9_]+(\.[a-z0-9_]+)+", name) or name in names:
            raise AudioError("%s: check names must be unique dotted names (%r)" % (path, name))
        names.add(name)
        if check.get("kind") not in CHECK_KINDS:
            raise AudioError("%s: %s has unknown kind %r" % (path, name, check.get("kind")))
    if not names:
        raise AudioError("%s: no checks" % path)
    return workload


# --- Retail waves ---------------------------------------------------------------

class Waves:
    """Decodes retail sound files from the installed VPKs, cached as float32."""

    def __init__(self, steam_root, cache):
        import source_content
        self.directories = []
        for name in CONTENT_DIRECTORIES:
            vpk = Path(steam_root) / name / "pak01_dir.vpk"
            if vpk.is_file():
                self.directories.append(source_content.VpkDirectory(str(vpk)))
        if not self.directories:
            raise AudioError("no Portal 2 VPKs under %s" % steam_root)
        self.loose = [Path(steam_root) / name for name in CONTENT_DIRECTORIES]
        self.cache = Path(cache)
        self.cache.mkdir(parents=True, exist_ok=True)
        self.memory = {}

    @staticmethod
    def normalize(name):
        return name.replace("\\", "/").lstrip(SOUND_PREFIX_CHARACTERS).lower()

    def read(self, name):
        relative = "sound/" + self.normalize(name)
        for directory in self.directories:
            try:
                data = directory.read(relative)
            except (KeyError, OSError, ValueError):
                data = None
            if data:
                return data
        for root in self.loose:
            if (root / relative).is_file():
                return (root / relative).read_bytes()
        raise AudioError("retail sound %s is missing" % relative)

    def decode(self, name, stereo=False):
        numpy, _ = numpy_modules()
        key = (self.normalize(name), stereo)
        if key in self.memory:
            return self.memory[key]
        digest = hashlib.sha256(repr(key).encode()).hexdigest()[:24]
        target = self.cache / (digest + ".f32")
        if not target.is_file():
            source = self.cache / (digest + ".src")
            source.write_bytes(self.read(name))
            try:
                subprocess.run(["ffmpeg", "-v", "error", "-y", "-i", str(source),
                                "-ac", "2" if stereo else "1", "-ar", str(RATE), "-f", "f32le",
                                str(target)], check=True, capture_output=True)
            except (OSError, subprocess.CalledProcessError) as error:
                raise AudioError("ffmpeg cannot decode %s: %s" % (name, error))
            finally:
                source.unlink(missing_ok=True)
        data = numpy.fromfile(target, dtype="<f4").astype(numpy.float64)
        if stereo:
            data = data.reshape(-1, 2)
        self.memory[key] = data
        return data


# --- Captures ------------------------------------------------------------------

def read_marks(directory):
    marks = {}
    for name in ("console.log", "engine.log"):
        path = Path(directory) / name
        if not path.is_file():
            continue
        for line in path.read_text(errors="replace").splitlines():
            match = MARK_LINE.search(line)
            if match and match.group(1) not in marks:
                marks[match.group(1)] = float(match.group(2))
        if marks:
            break
    return marks


def load_capture(directory):
    """Stereo float samples of a capture and its marks."""
    numpy, _ = numpy_modules()
    directory = Path(directory)
    stdout = (directory / "stdout.txt").read_text(errors="replace") \
        if (directory / "stdout.txt").is_file() else ""
    opened = CHANNELS_LINE.findall(stdout)
    # A device reopened with other settings truncates the file: the last open wins.
    channels, rate = (int(opened[-1][0]), int(opened[-1][1])) if opened else (2, RATE)
    if channels != 2 or rate != RATE:
        raise AudioError("%s: the capture is %d channels at %d Hz; stereo 44.1 kHz is required "
                         "(run with +snd_surround_speakers 2)" % (directory, channels, rate))
    raw = directory / "audio.raw"
    if not raw.is_file():
        raise AudioError("%s: no audio.raw" % directory)
    samples = numpy.fromfile(raw, dtype="<i2")
    samples = samples[:len(samples) // 2 * 2].reshape(-1, 2).astype(numpy.float64) / 32768.0
    marks = read_marks(directory)
    if not marks:
        raise AudioError("%s: no AUDIO_MARK lines in the console log" % directory)
    return samples, marks


def apply_seeded_defect(samples, marks, defect, workload):
    """A deliberate defect for the negative controls."""
    samples = samples.copy()
    if defect == "mute-right":
        samples[:, 1] = 0.0
    elif defect == "drop-music":
        for start, end in workload.get("seed_music_spans", []):
            begin = int(max(marks[start] - 1.0, 0.0) * RATE)
            finish = int((marks[end] + 1.0) * RATE)
            samples[begin:finish] = 0.0
    else:
        raise AudioError("unknown seeded defect %s" % defect)
    return samples


# --- Signal measurement --------------------------------------------------------

def to_db(value):
    return SILENT_DB if value <= 1e-6 else max(20.0 * math.log10(value), SILENT_DB)


def rms(numpy, data):
    return float(numpy.sqrt(numpy.mean(numpy.square(data)))) if len(data) else 0.0


def decimate(scipy_signal, data):
    return scipy_signal.resample_poly(data, 1, DECIMATE)


def best_match(numpy, scipy_signal, haystack, needle, low, high):
    """Normalized cross-correlation of needle within haystack; start in [low, high]."""
    low = max(int(low), 0)
    high = min(int(high), len(haystack) - len(needle))
    if high < low or len(needle) == 0:
        return None
    section = haystack[low:high + len(needle)]
    correlation = scipy_signal.fftconvolve(section, needle[::-1], mode="valid")
    energy = numpy.concatenate([[0.0], numpy.cumsum(numpy.square(section))])
    windowed = numpy.maximum(energy[len(needle):] - energy[:-len(needle)], 0.0)
    normalized = correlation / (numpy.sqrt(windowed * numpy.sum(numpy.square(needle))) + 1e-9)
    # Digital silence (below -80 dB RMS) matches nothing.
    normalized[windowed < len(needle) * QUIET_POWER] = 0.0
    index = int(numpy.argmax(normalized))
    return low + index, float(normalized[index])


def loudest_excerpt(numpy, reference, seconds):
    """Start (samples) of the loudest `seconds` of a reference, within its first 20 s."""
    length = min(int(seconds * RATE), len(reference))
    mono = reference if reference.ndim == 1 else reference.mean(axis=1)
    search = mono[:min(len(mono), 20 * RATE)]
    if len(search) <= length:
        return 0, length
    energy = numpy.concatenate([[0.0], numpy.cumsum(numpy.square(search))])
    windowed = energy[length:] - energy[:-length]
    step = RATE // 20
    starts = range(0, len(windowed), step)
    start = max(starts, key=lambda i: windowed[i])
    return int(start), length


def fit_gains(numpy, capture, references):
    """Least-squares gains of each reference column in a capture segment."""
    matrix = numpy.stack(references, axis=1)
    gains, *_ = numpy.linalg.lstsq(matrix, capture, rcond=None)
    return [float(value) for value in gains]


class Measurement:
    def __init__(self, workload, waves, samples, marks):
        self.numpy, self.signal = numpy_modules()
        self.workload, self.waves = workload, waves
        self.samples, self.marks = samples, marks
        self.low = decimate(self.signal, samples.mean(axis=1))
        missing = [mark for mark in workload["marks"] if mark not in marks]
        if missing:
            raise AudioError("marks missing from the capture: " + ", ".join(missing))
        self.offset = None
        self.events = {}
        self.track = []

    # Game time -> capture time. The first estimate matches the alignment
    # wave against every mark that plays it; later events track drift.
    def align(self):
        numpy = self.numpy
        alignment = self.workload["alignment"]
        reference = decimate(self.signal, self.waves.decode(alignment["wave"]))
        correlation = self.signal.fftconvolve(self.low, reference[::-1], mode="valid")
        energy = numpy.concatenate([[0.0], numpy.cumsum(numpy.square(self.low))])
        windowed = numpy.maximum(energy[len(reference):] - energy[:-len(reference)], 0.0)
        normalized = correlation / (numpy.sqrt(windowed * numpy.sum(numpy.square(reference)))
                                    + 1e-9)
        normalized[windowed < len(reference) * QUIET_POWER] = 0.0
        candidates = numpy.argsort(normalized)[::-1][:2000]
        anchor = self.marks[alignment["marks"][0]]
        best = None
        tried = set()
        for index in candidates:
            if normalized[index] < 0.5:
                break
            offset = index / LOW_RATE - anchor
            key = round(offset, 1)
            if key in tried:
                continue
            tried.add(key)
            score = 0.0
            for mark in alignment["marks"]:
                position = int((self.marks[mark] + offset) * LOW_RATE)
                low, high = max(position - LOW_RATE // 5, 0), position + LOW_RATE // 2
                if high < len(normalized) and high > low:
                    score += float(numpy.max(normalized[low:high]))
            if best is None or score > best[0]:
                best = (score, offset)
        if best is None:
            raise AudioError("the alignment wave %s is not in the capture" % alignment["wave"])
        self.offset = best[1]
        self.track.append((anchor, self.offset))
        self.alignment_score = round(best[0] / len(alignment["marks"]), 3)

    def locate(self, label, event):
        numpy = self.numpy
        stereo = bool(event.get("stereo"))
        reference = self.waves.decode(event["wave"], stereo=stereo)
        start, length = loudest_excerpt(numpy, reference, event.get("excerpt_seconds", 3.0))
        excerpt = reference[start:start + length]
        excerpt_mono = excerpt if excerpt.ndim == 1 else excerpt.mean(axis=1)
        needle = decimate(self.signal, excerpt_mono)
        mark = self.marks[event.get("mark", label)]
        predicted = mark + self.offset + start / RATE
        search = event.get("search_seconds", 1.5)
        found = best_match(numpy, self.signal, self.low, needle,
                           (predicted - 0.4) * LOW_RATE, (predicted + search) * LOW_RATE)
        if found is None:
            return {"present": False, "ncc": 0.0, "gain": 0.0}
        index, ncc = found
        # Refine the onset at the full rate around the decimated match.
        coarse = index * DECIMATE
        mono = self.samples.mean(axis=1)
        refined = best_match(numpy, self.signal, mono, excerpt_mono, coarse - 2 * DECIMATE,
                             coarse + 2 * DECIMATE)
        position = refined[0] if refined else coarse
        segment = self.samples[position:position + length]
        if len(segment) < length:
            excerpt = excerpt[:len(segment)]
        channels = [excerpt[:, channel] if excerpt.ndim == 2 else excerpt for channel in (0, 1)]
        gains = [fit_gains(numpy, segment[:, channel], [channels[channel]])[0]
                 for channel in (0, 1)]
        gain = math.sqrt((gains[0] ** 2 + gains[1] ** 2) / 2.0)
        onset = position / RATE - start / RATE
        threshold = event.get("present_ncc", self.workload.get("present_ncc", 0.5))
        result = {"present": ncc >= threshold, "ncc": round(ncc, 3),
                  "gain": round(gain, 4), "gain_left": round(gains[0], 4),
                  "gain_right": round(gains[1], 4), "onset": round(onset, 3),
                  "latency": round(onset - mark - self.offset, 3)}
        if result["present"] and event.get("track", True):
            self.offset = onset - mark
            self.track.append((mark, self.offset))
        return result

    def offset_at(self, time):
        """The drift-tracked offset of the last located event at or before `time`."""
        earlier = [offset for mark, offset in sorted(self.track) if mark <= time]
        return earlier[-1] if earlier else sorted(self.track)[0][1]

    def window(self, window):
        numpy = self.numpy
        offset = self.offset_at(self.marks[window["from"]])
        start = int((self.marks[window["from"]] + offset + window.get("start", 0.0)) * RATE)
        end = int((self.marks[window["to"]] + offset + window.get("end", 0.0)) * RATE)
        segment = self.samples[max(start, 0):max(end, 0)]
        left, right = segment[:, 0], segment[:, 1]
        correlation = 0.0
        if len(segment) > 1 and left.std() > 1e-7 and right.std() > 1e-7:
            correlation = float(numpy.corrcoef(left, right)[0, 1])
        return {"seconds": round(len(segment) / RATE, 3), "rms_db": round(to_db(rms(numpy, segment)), 2),
                "left_db": round(to_db(rms(numpy, left)), 2),
                "right_db": round(to_db(rms(numpy, right)), 2), "lr_corr": round(correlation, 3)}

    # Derived metrics: each names the events it needs in the workload.
    def continuation_gain(self, event, begin, seconds, others=()):
        """Gain of an event's wave continued at capture time `begin`, fitted beside others."""
        numpy = self.numpy
        located = self.events[event]
        if not located.get("present"):
            return None
        segment = self.samples[int(begin * RATE):int((begin + seconds) * RATE)].mean(axis=1)
        columns = []
        for name in (event,) + tuple(others):
            spec = self.workload["events"][name]
            reference = self.waves.decode(spec["wave"], stereo=bool(spec.get("stereo")))
            if reference.ndim == 2:
                reference = reference.mean(axis=1)
            offset = int((begin - self.events[name]["onset"]) * RATE)
            column = numpy.zeros(len(segment))
            if offset < len(reference):
                piece = reference[max(offset, 0):max(offset, 0) + len(segment)]
                column[max(-offset, 0):max(-offset, 0) + len(piece)] = piece[:len(segment) - max(-offset, 0)]
            columns.append(column)
        if not numpy.any(columns[0]):
            return None
        return abs(fit_gains(numpy, segment, columns)[0])

    def derive(self, spec):
        kind = spec["kind"]
        if kind == "cut":
            first, second = self.events[spec["first"]], self.events[spec["second"]]
            if not (first.get("present") and second.get("present")):
                return None
            before = self.continuation_gain(spec["first"], first["onset"] + 0.2,
                                            second["onset"] - first["onset"] - 0.3)
            after = self.continuation_gain(spec["first"], second["onset"] + 0.3, 2.0,
                                           (spec["second"],))
            if not before or after is None:
                return None
            return round(to_db(after / before), 2)
        if kind == "duck":
            music, line = self.events[spec["music"]], self.events[spec["line"]]
            if not (music.get("present") and line.get("present")):
                return None
            before = self.continuation_gain(spec["music"], line["onset"] - 2.5, 2.3)
            during = self.continuation_gain(spec["music"], line["onset"] + 1.0, 3.0,
                                            (spec["line"],))
            if not before or during is None:
                return None
            return round(to_db(during / before), 2)
        if kind == "ratio":
            near, far = self.events[spec["reference"]], self.events[spec["event"]]
            if not (near.get("present") and far.get("present")) or near["gain"] <= 0:
                return None
            return round(to_db(far["gain"] / near["gain"]), 2)
        if kind == "tail":
            event = self.events[spec["event"]]
            if not event.get("present"):
                return None
            wave = self.waves.decode(self.workload["events"][spec["event"]]["wave"])
            end = event["onset"] + len(wave) / RATE
            direct = self.samples[int(event["onset"] * RATE):int(end * RATE)]
            tail = self.samples[int((end + spec.get("start", 0.05)) * RATE):
                                int((end + spec.get("end", 0.45)) * RATE)]
            return round(to_db(rms(self.numpy, tail)) - to_db(rms(self.numpy, direct)), 2)
        raise AudioError("unknown derived metric kind %s" % kind)

    def run(self, defect=None):
        self.align()
        if defect:
            # Defects are placed in capture time, so they follow the alignment.
            at = {mark: time + self.offset for mark, time in self.marks.items()}
            self.samples = apply_seeded_defect(self.samples, at, defect, self.workload)
            self.low = decimate(self.signal, self.samples.mean(axis=1))
        events = self.workload["events"]
        order = sorted(events, key=lambda label: self.marks[events[label].get("mark", label)])
        for label in order:
            self.events[label] = self.locate(label, self.workload["events"][label])
        windows = {label: self.window(window)
                   for label, window in self.workload.get("windows", {}).items()}
        derived = {label: self.derive(spec)
                   for label, spec in self.workload.get("derived", {}).items()}
        return {"schema": METRICS_SCHEMA, "offset": round(self.offset, 3),
                "alignment_score": self.alignment_score, "marks": dict(self.marks),
                "track": [[mark, round(offset, 4)] for mark, offset in sorted(self.track)],
                "events": self.events, "windows": windows, "derived": derived}


def measure_capture(workload, waves, directory, defect=None):
    samples, marks = load_capture(directory)
    return Measurement(workload, waves, samples, marks).run(defect)


# --- Checks --------------------------------------------------------------------

def level_of(metrics, check):
    """dB of an event gain or window level relative to the check's reference."""
    if "event" in check:
        event, reference = metrics["events"][check["event"]], \
            metrics["events"][check["relative_to"]]
        if not event.get("present") or not reference.get("present") or reference["gain"] <= 0:
            return None
        return to_db(event["gain"] / reference["gain"])
    window, reference = metrics["windows"][check["window"]], \
        metrics["windows"][check["relative_to"]]
    return window["rms_db"] - reference["rms_db"]


def evaluate_check(check, ours, retail):
    """(ok, detail) of one check; the reference comes from the retail metrics."""
    kind = check["kind"]
    if kind == "present":
        event, expected = ours["events"][check["event"]], retail["events"][check["event"]]
        if not expected.get("present"):
            return False, "retail reference does not have this event"
        return bool(event.get("present")), "ncc %.3f (retail %.3f)" % (
            event.get("ncc", 0.0), expected["ncc"])
    if kind == "level":
        value, expected = level_of(ours, check), level_of(retail, check)
        if expected is None:
            return False, "retail reference has no level"
        if value is None:
            return False, "event absent (retail %+.1f dB)" % expected
        return abs(value - expected) <= check["tolerance_db"], "%+.1f dB, retail %+.1f dB" % (
            value, expected)
    if kind == "fidelity":
        # Retail plays these sounds sample-clean; a build that filters, remixes
        # or resamples them correlates less with the wave.
        event, expected = ours["events"][check["event"]], retail["events"][check["event"]]
        return event.get("ncc", 0.0) >= expected["ncc"] - check["tolerance"], \
            "ncc %.3f, retail %.3f" % (event.get("ncc", 0.0), expected["ncc"])
    if kind == "balance":
        window, expected = ours["windows"][check["window"]], retail["windows"][check["window"]]
        value = window["left_db"] - window["right_db"]
        reference = expected["left_db"] - expected["right_db"]
        return abs(value - reference) <= check["tolerance_db"], \
            "left-right %+.1f dB, retail %+.1f dB" % (value, reference)
    if kind == "width":
        value = ours["windows"][check["window"]]["lr_corr"]
        reference = retail["windows"][check["window"]]["lr_corr"]
        return abs(value - reference) <= check["tolerance"], \
            "left/right correlation %.2f, retail %.2f" % (value, reference)
    if kind == "silence":
        value = ours["windows"][check["window"]]["rms_db"]
        reference = retail["windows"][check["window"]]["rms_db"]
        return value <= check["max_db"], "%.1f dB, retail %.1f dB" % (value, reference)
    if kind == "derived":
        value, reference = ours["derived"].get(check["metric"]), \
            retail["derived"].get(check["metric"])
        if reference is None:
            return False, "retail reference has no %s" % check["metric"]
        if value is None:
            return False, "not measurable (retail %+.1f dB)" % reference
        if "at_most_db" in check:
            return value <= check["at_most_db"], "%+.1f dB, retail %+.1f dB" % (value, reference)
        return abs(value - reference) <= check["tolerance_db"], "%+.1f dB, retail %+.1f dB" % (
            value, reference)
    raise AudioError("unknown check kind %s" % kind)


CHECK_KINDS = ("present", "level", "fidelity", "balance", "width", "silence", "derived")


def run_checks(workload, ours, retail, stream=sys.stdout):
    checks = conformance_result.Checks(stream)
    for check in workload["checks"]:
        try:
            ok, detail = evaluate_check(check, ours, retail)
        except KeyError as error:
            ok, detail = False, "metric missing: %s" % error
        if ok:
            print("PASS %s %s" % (check["name"], detail), file=stream)
        checks.check(ok, check["name"], detail)
    return checks


# --- Comparator self-test --------------------------------------------------------

# Each seeded defect of the rendered reference and the check that must reject it.
RENDER_CONTROLS = {
    "mute-right": "music.balance",
    "drop-music": "music.present",
    "swap-line": "vo.present",
    "no-chain": "chain.starts_entry",
    "no-duck": "duck.music_under_line",
    "loud-dialog": "vo.level",
}


def render_reference(workload, waves, reference, seed=None):
    """A capture rebuilt from the retail metrics: each retail wave at its retail
    onset and gains, the dialog ducking, the reverb tail and the noise-like
    windows (ambience, the randomized portal gun) as noise of the same level and
    left/right correlation. Its metrics must pass every check."""
    numpy, _ = numpy_modules()
    marks, track = reference["marks"], reference["track"]
    events = reference["events"]

    def capture_time(mark):
        earlier = [offset for when, offset in track if when <= marks[mark]]
        return marks[mark] + (earlier[-1] if earlier else track[0][1])

    length = int((capture_time(workload["marks"][-1]) + 8.0) * RATE)
    samples = numpy.zeros((length, 2))
    generator = numpy.random.default_rng(20260925)
    duck = reference["derived"].get("duck")
    duck_line = workload["derived"]["duck"]["line"] if duck is not None else None
    for label, spec in workload["events"].items():
        event = events[label]
        if not event.get("present") or (seed == "no-chain" and label == "chain_started"):
            continue
        wave = spec["wave"]
        if seed == "swap-line" and label == "vo":
            wave = workload["events"]["line_b"]["wave"]
        data = waves.decode(wave, stereo=bool(spec.get("stereo")))
        if data.ndim == 1:
            data = numpy.stack([data, data], axis=1)
        start = int(event["onset"] * RATE)
        end = min(start + len(data), length)
        if "until" in spec:
            end = min(end, int(capture_time(spec["until"]) * RATE))
        piece = data[:end - start] * numpy.array([event["gain_left"], event["gain_right"]])
        if seed == "loud-dialog" and label == "vo":
            piece *= 10 ** (3.0 / 20.0)
        if duck is not None and seed != "no-duck" and label == workload["derived"]["duck"]["music"]:
            line = events[duck_line]
            line_wave = waves.decode(workload["events"][duck_line]["wave"])
            first = int((line["onset"] + 0.25) * RATE) - start
            last = int((line["onset"] + len(line_wave) / RATE) * RATE) - start
            piece[max(first, 0):max(last, 0)] *= 10 ** (duck / 20.0)
        samples[start:end] += piece
    for label, spec in workload.get("derived", {}).items():
        value = reference["derived"].get(label)
        if spec["kind"] != "tail" or value is None:
            continue
        event = events[spec["event"]]
        wave = waves.decode(workload["events"][spec["event"]]["wave"])
        begin, finish = int(event["onset"] * RATE), int((event["onset"] + len(wave) / RATE) * RATE)
        level = rms(numpy, samples[begin:finish]) * 10 ** (value / 20.0)
        span = int(0.6 * RATE)
        samples[finish:finish + span] += generator.normal(0.0, level, (span, 2))
    for label, spec in workload.get("windows", {}).items():
        if not spec.get("render_noise"):
            continue
        window = reference["windows"][label]
        begin = int((capture_time(spec["from"]) + spec.get("start", 0.0)) * RATE)
        finish = int((capture_time(spec["to"]) + spec.get("end", 0.0)) * RATE)
        common = generator.normal(0.0, 1.0, finish - begin)
        correlation = max(min(window["lr_corr"], 0.999), 0.0)
        for channel, key in ((0, "left_db"), (1, "right_db")):
            own = generator.normal(0.0, 1.0, finish - begin)
            noise = math.sqrt(correlation) * common + math.sqrt(1.0 - correlation) * own
            samples[begin:finish, channel] += noise * 10 ** (window[key] / 20.0)
    if seed == "mute-right":
        samples[:, 1] = 0.0
    elif seed == "drop-music":
        samples = apply_seeded_defect(samples, marks_in_capture_time(workload, reference),
                                      "drop-music", workload)
    return numpy.clip(samples, -1.0, 1.0), dict(marks)


def marks_in_capture_time(workload, reference):
    marks, track = reference["marks"], reference["track"]
    result = {}
    for mark in workload["marks"]:
        earlier = [offset for when, offset in track if when <= marks[mark]]
        result[mark] = marks[mark] + (earlier[-1] if earlier else track[0][1])
    return result


def command_selftest(args, workload):
    """The comparator against the rendered retail reference and its seeded defects."""
    reference = json.loads(reference_path(args.workload, workload).read_text())["metrics"]
    waves = Waves(args.steam_root, args.cache)
    checks = conformance_result.Checks()
    samples, marks = render_reference(workload, waves, reference)
    rendered = Measurement(workload, waves, samples, marks).run()
    for check in workload["checks"]:
        ok, detail = evaluate_check(check, rendered, reference)
        if ok:
            print("PASS render.%s %s" % (check["name"], detail))
        checks.check(ok, "render." + check["name"], detail)
    for seed, target in sorted(RENDER_CONTROLS.items()):
        samples, marks = render_reference(workload, waves, reference, seed)
        seeded = Measurement(workload, waves, samples, marks).run()
        check = next(item for item in workload["checks"] if item["name"] == target)
        ok, detail = evaluate_check(check, seeded, reference)
        name = "control.%s.rejected_by.%s" % (seed.replace("-", "_"), target)
        if not ok:
            print("PASS %s (%s)" % (name, detail))
        checks.check(not ok, name, "the seeded defect passed: " + detail)
    return checks.report()


# --- Captures: this build ------------------------------------------------------

def install_probe(workload_path, workload, game_directory, steam_root, seed=None, console=()):
    """Driver scripts under scripts/vscripts/qa, a mapspawn.nut hook and the
    diagnostic commands the driver executes after enabling cheats."""
    config = Path(game_directory) / "cfg" / "qa_audio_extra.cfg"
    if config.is_symlink():
        config.unlink()
    config.write_text("".join(command + "\n" for command in console))
    source = Path(workload_path).parent
    vscripts = Path(game_directory) / "scripts" / "vscripts"
    qa = vscripts / "qa"
    if qa.is_symlink() or qa.is_file():
        qa.unlink()
    qa.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(source / workload["driver"], qa / "qa_driver.nut")
    script = (source / workload["script"]).read_text()
    if seed == "swap-line":
        script, count = re.subn(r"^::AP_SWAP_LINES <- false$", "::AP_SWAP_LINES <- true",
                                script, flags=re.M)
        if count != 1:
            raise AudioError("the driver has no AP_SWAP_LINES switch")
    elif seed:
        raise AudioError("unknown driver seed %s" % seed)
    stem = Path(workload["script"]).stem
    (qa / (stem + ".nut")).write_text(script)
    # The hook is rebuilt from the installed game's own mapspawn.nut every run.
    hook = vscripts / "mapspawn.nut"
    pristine = Path(steam_root) / "portal2" / "scripts" / "vscripts" / "mapspawn.nut"
    original = pristine.read_text(errors="replace").rstrip() + "\n" if pristine.is_file() else ""
    if hook.is_symlink() or hook.is_file():
        hook.unlink()
    # Retail ignores +wait on the command line, so both builds start the
    # driver from mapspawn, one second after the map spawns.
    hook.write_text(original + "// PORTAL2_AUDIO_HOOK\nif ( GetMapName() == \"%s\" ) "
                    "EntFire( \"worldspawn\", \"RunScriptCode\", \"DoIncludeScript(\\\"qa/%s\\\", "
                    "getroottable())\", 1.0 )\n" % (workload["map"], stem))


def run_game(command, cwd, environment, out, timeout, console):
    out.mkdir(parents=True, exist_ok=True)
    for stale in (Path(cwd) / "sdlaudio.raw", console):
        stale.unlink(missing_ok=True)
    started = time.monotonic()
    timed_out = False
    with (out / "stdout.txt").open("wb") as stream:
        process = subprocess.Popen(command, cwd=cwd, env=environment, stdout=stream,
                                   stderr=subprocess.STDOUT, start_new_session=True)
        try:
            process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            timed_out = True
            os.killpg(process.pid, signal.SIGINT)
            try:
                process.wait(timeout=15)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
    audio = Path(cwd) / "sdlaudio.raw"
    if audio.is_file():
        shutil.move(str(audio), str(out / "audio.raw"))
    for log in (console, Path(cwd) / "engine.log"):
        if log.is_file():
            shutil.copyfile(log, out / log.name)
    record = {"command": command, "returncode": process.returncode, "timed_out": timed_out,
              "seconds": round(time.monotonic() - started, 1)}
    (out / "run.json").write_text(json.dumps(record, indent=2) + "\n")
    return record


def write_fake_zenity(directory):
    """Error() dialogs go through zenity; never let one open or block a run."""
    directory.mkdir(parents=True, exist_ok=True)
    zenity = directory / "zenity"
    zenity.write_text("#!/bin/sh\nif [ \"$1\" = --version ]; then echo 4.0.2; exit 0; fi\n"
                      "echo \"zenity $*\" >&2\nexit 1\n")
    zenity.chmod(0o755)


def capture_ours(args, workload, out):
    import portal_boot
    import stage_portal2_runtime
    runtime = Path(args.runtime).resolve()
    stage_portal2_runtime.stage_content(args.steam_root, runtime)
    portal_boot.install_build(args.build, runtime, game="portal2")
    install_probe(args.workload, workload, runtime / "portal2", args.steam_root, args.seed, args.console)
    write_fake_zenity(out / "tools")
    environment = dict(os.environ)
    for variable in ("DISPLAY", "WAYLAND_DISPLAY"):
        environment.pop(variable, None)
    environment.update({
        "SteamAppId": "620", "SteamGameId": "620", "LD_LIBRARY_PATH": str(runtime / "bin"),
        "SDL_VIDEODRIVER": "offscreen", "SDL_VIDEO_DRIVER": "offscreen",
        "SDL_AUDIO_DRIVER": "disk", "SDL_AUDIODRIVER": "disk",
        "PATH": str(out / "tools") + os.pathsep + environment.get("PATH", ""),
    })
    command = [str(runtime / "hl2_launcher"), "-game", "portal2", "-multirun", "-novid",
               "-insecure", "-windowed", "-w", "1024", "-h", "768", "-condebug",
               "+snd_mute_losefocus", "0", "+snd_surround_speakers", "2", *args.extra_arg,
               "+map", workload["map"]]
    return run_game(command, runtime, environment, out, workload["timeout_seconds"],
                    runtime / "portal2" / "console.log")


# --- Captures: retail ----------------------------------------------------------

def retail_mirror(steam_root, mirror):
    """Symlink mirror of the install with private cfg and scripts/vscripts."""
    steam_root, mirror = Path(steam_root).resolve(), Path(mirror)
    game = mirror / "portal2"
    scripts = game / "scripts"
    vscripts = scripts / "vscripts"
    vscripts.mkdir(parents=True, exist_ok=True)

    def link_children(source, target, skip):
        for child in source.iterdir():
            destination = target / child.name
            if child.name in skip or destination.exists() or destination.is_symlink():
                continue
            destination.symlink_to(child)

    link_children(steam_root, mirror, {"portal2"})
    link_children(steam_root / "portal2", game, {"cfg", "scripts"})
    if not (game / "cfg").exists():
        shutil.copytree(steam_root / "portal2" / "cfg", game / "cfg", symlinks=True)
    link_children(steam_root / "portal2" / "scripts", scripts, {"vscripts"})
    link_children(steam_root / "portal2" / "scripts" / "vscripts", vscripts, {"mapspawn.nut", "qa"})
    original = steam_root / "portal2" / "scripts" / "vscripts" / "mapspawn.nut"
    if not (vscripts / "mapspawn.nut").exists() and original.is_file():
        shutil.copyfile(original, vscripts / "mapspawn.nut")
    (mirror / "steam_appid.txt").write_text("620\n")
    return mirror


def capture_retail(args, workload, out):
    mirror = retail_mirror(args.steam_root, args.retail_mirror)
    install_probe(args.workload, workload, mirror / "portal2", args.steam_root, args.seed, args.console)
    write_fake_zenity(out / "tools")
    for tool in ("mutter", "dbus-run-session"):
        if not shutil.which(tool):
            raise AudioError("retail capture needs %s" % tool)
    libraries = [str(mirror / "bin" / "linux32")]
    if args.retail_libs:
        libraries.append(str(Path(args.retail_libs).resolve()))
    environment = dict(os.environ)
    environment.update({
        "SteamAppId": "620", "SteamGameId": "620", "LD_LIBRARY_PATH": os.pathsep.join(libraries),
        # The 32-bit retail binary crashes on SDL's Wayland backend; use Xwayland.
        "SDL_VIDEODRIVER": "x11", "SDL_AUDIO_DRIVER": "disk", "SDL_AUDIODRIVER": "disk",
        "PATH": str(out / "tools") + os.pathsep + environment.get("PATH", ""),
        "XDG_CONFIG_HOME": str(out / "xdg"),
    })
    display = "p2audio-%d" % os.getpid()
    command = ["dbus-run-session", "--", "mutter", "--headless", "--wayland", "--virtual-monitor",
               "1024x768@60", "--wayland-display", display, "--",
               "./portal2_linux", "-game", "portal2", "-nobreakpad", "-novid", "-multirun",
               "-condebug", "-windowed", "-w", "1024", "-h", "768", "+snd_mute_losefocus", "0",
               "+snd_surround_speakers", "2", *args.extra_arg, "+map", workload["map"]]
    record = run_game(command, mirror, environment, out, workload["timeout_seconds"] + 60,
                      mirror / "portal2" / "console.log")
    version = Path(args.steam_root) / "portal2" / "steam.inf"
    if version.is_file():
        record["retail_version"] = version.read_text(errors="replace").strip().splitlines()
        (out / "run.json").write_text(json.dumps(record, indent=2) + "\n")
    return record


# --- Commands ------------------------------------------------------------------

def reference_path(workload_path, workload):
    return Path(workload_path).parent / workload["reference"]


def command_capture(args, workload):
    out = args.out.resolve()
    if (out / "audio.raw").exists():
        raise AudioError("%s already holds a capture" % out)
    record = (capture_retail if args.side == "retail" else capture_ours)(args, workload, out)
    print(json.dumps(record, indent=2))
    return 0 if (out / "audio.raw").is_file() and not record["timed_out"] else 1


def command_measure(args, workload):
    waves = Waves(args.steam_root, args.cache)
    metrics = measure_capture(workload, waves, args.capture, args.seed_defect)
    print(json.dumps(metrics, indent=2))
    return 0


def command_record(args, workload):
    waves = Waves(args.steam_root, args.cache)
    metrics = measure_capture(workload, waves, args.capture)
    run = {}
    if (Path(args.capture) / "run.json").is_file():
        run = json.loads((Path(args.capture) / "run.json").read_text())
    reference = {
        "schema": REFERENCE_SCHEMA,
        "recorded_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "retail_version": run.get("retail_version"),
        "command": run.get("command"),
        "note": "Metrics of one retail Portal 2 capture of this workload "
                "(portal2_audio.py capture --side retail, then record). Numbers only.",
        "metrics": metrics,
    }
    missing = [check["name"] for check in workload["checks"]
               if check["kind"] == "present" and not metrics["events"][check["event"]]["present"]]
    if missing:
        raise AudioError("the retail capture lacks events its checks require: " + ", ".join(missing))
    path = reference_path(args.workload, workload)
    path.write_text(json.dumps(reference, indent=2) + "\n")
    print("wrote %s" % path)
    return 0


def command_check(args, workload):
    reference = json.loads(reference_path(args.workload, workload).read_text())
    if reference.get("schema") != REFERENCE_SCHEMA:
        raise AudioError("the retail reference has the wrong schema")
    capture = args.capture
    if capture is None:
        capture = args.out.resolve() / "capture"
        record = capture_ours(args, workload, capture)
        if record["timed_out"] or not (capture / "audio.raw").is_file():
            print("FAIL capture: the game run did not finish (%s)" % capture)
            return conformance_result.report_conformance(1, 1)
    waves = Waves(args.steam_root, args.cache)
    ours = measure_capture(workload, waves, capture, args.seed_defect)
    if args.out:
        args.out.mkdir(parents=True, exist_ok=True)
        (args.out / "metrics.json").write_text(json.dumps(ours, indent=2) + "\n")
    checks = run_checks(workload, ours, reference["metrics"])
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=("capture", "measure", "record", "check", "selftest"))
    parser.add_argument("--workload", type=Path, default=DEFAULT_WORKLOAD)
    parser.add_argument("--steam-root", type=Path, default=Path(os.environ.get(
        "P2_STEAM_ROOT", DEFAULT_STEAM_ROOT)))
    parser.add_argument("--build", type=Path, default=Path(os.environ.get(
        "SOURCE_PORTAL2_AUDIO_BUILD", ROOT / "build-p2")),
        help="Waf output configured with --build-games=portal2")
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2-audio",
                        help="private staged runtime for this build")
    parser.add_argument("--retail-mirror", type=Path, default=ROOT / "run/retail-p2-audio",
                        help="private symlink mirror of the retail install")
    parser.add_argument("--retail-libs", type=Path,
                        help="extra 32-bit libraries for the retail binary")
    parser.add_argument("--side", choices=("ours", "retail"), default="ours")
    parser.add_argument("--capture", type=Path, help="existing capture directory")
    parser.add_argument("--out", type=Path, help="new capture or evidence directory")
    parser.add_argument("--cache", type=Path, default=ROOT / "run/cache/portal2-audio",
                        help="decoded retail waves (private)")
    parser.add_argument("--seed", choices=("swap-line",),
                        help="negative control: a defect in the driver")
    parser.add_argument("--seed-defect", choices=SEEDED_DEFECTS,
                        help="negative control: a defect applied to the capture")
    parser.add_argument("--console", action="append", default=[],
                        help="diagnostic console command the driver runs after sv_cheats 1 "
                             "(repeatable), e.g. --console='snd_showstart 2'")
    parser.add_argument("--extra-arg", action="append", default=[],
                        help="extra engine argument before +map (repeatable)")
    args = parser.parse_args(argv)
    try:
        workload = load_workload(args.workload)
        if args.command in ("measure", "record") and args.capture is None:
            parser.error("%s needs --capture" % args.command)
        if args.command == "capture" and args.out is None:
            parser.error("capture needs --out")
        if args.command == "check" and args.capture is None and args.out is None:
            parser.error("check needs --capture or --out")
        return {"capture": command_capture, "measure": command_measure,
                "record": command_record, "check": command_check,
                "selftest": command_selftest}[args.command](args, workload)
    except AudioError as error:
        if args.command in ("check", "selftest"):
            print("FAIL setup: %s" % error)
            return conformance_result.report_conformance(1, 1)
        parser.exit(2, "portal2_audio: %s\n" % error)


if __name__ == "__main__":
    sys.exit(main())
