#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""The 3DS content set's sounds: a closure, converted for the DSP's voices.

The 3DS plays fully loaded 16-bit PCM sounds on the DSP's hardware voices
(engine/audio/snd_dev_n3ds.cpp); everything else goes through the engine's
software mixer on the CPU. Retail Portal 2 ships 3.9 GB of sound, as PCM,
ADPCM and MP3-in-WAV at up to 44.1 kHz. This module stages only what a map
can play and converts each file so the hardware can play it:

  * closure: soundscript entries (scripts/game_sounds_manifest.txt) named by
    the map's entities or by the always-loaded gameplay families
    (ALWAYS_PREFIXES: player, portal gun, physics, UI, ...), plus .wav paths
    the entities name directly. Map references come first; staging stops at
    --sound-budget-mb and reports what it left out;
  * conversion (ffmpeg): 16-bit PCM, the source's channel count, 22050 Hz (or
    11025 Hz for sources at or below it; the engine mixes only 11/22/44 kHz),
    the loop point rescaled into a cue chunk, the VDAT (lip-sync) chunk kept.
    The file keeps its name, so soundscripts and code still find it.

The DSP's output runs at about 32.7 kHz, so 22.05 kHz loses little and halves
the memory the hardware voices' sample cache needs.
"""

import fnmatch
import re
import struct
import subprocess

DEFAULT_BUDGET_MB = 64
# Above this a sound is converted again at 11025 Hz mono (long music and
# dialogue); above LONG_LIMIT even then, it is left out.
SHORT_LIMIT = 768 * 1024
LONG_LIMIT = 2 * 1024 * 1024

# Soundscript families every map plays from code (the player, the gun,
# physics impacts, UI). Glob patterns against lower-case entry names.
ALWAYS_PREFIXES = (
    "player.*", "portalplayer.*", "portal.*", "weapon_portalgun.*", "physics.*",
    "physicscannister.*", "ui.*", "default.*", "solidmetal.*", "metal_box.*",
    "metal.*", "concrete.*", "glass.*", "plastic*", "cardboard.*", "wood.*",
    "world.*", "doors.*", "portal_button.*", "prop_*", "props.*", "portalgun.*",
    "paintblob.*", "ambient.*",
)

# Leading characters of sound names that are flags, not path (public/soundchars.h).
SOUND_CHARS = "*#@><^)}$!?&~`+%"

TOKEN = re.compile(r'//[^\n]*|"([^"]*)"|([{}])|([^\s{}"]+)')


def tokens(text):
    for match in TOKEN.finditer(text):
        if match.group(0).startswith("//"):
            continue
        quoted, brace, bare = match.groups()
        yield brace or bare or quoted or ""


def parse_soundscript(text):
    """{entry name (lower): [wave paths]} from one soundscript file."""
    entries = {}
    stream = list(tokens(text))
    i = 0
    while i < len(stream):
        name = stream[i]
        if i + 1 < len(stream) and stream[i + 1] == "{":
            depth, j, waves = 1, i + 2, []
            while j < len(stream) and depth:
                token = stream[j]
                if token == "{":
                    depth += 1
                elif token == "}":
                    depth -= 1
                elif token.lower() == "wave" and j + 1 < len(stream):
                    waves.append(stream[j + 1])
                    j += 1
                j += 1
            entries[name.lower()] = waves
            i = j
        else:
            i += 1
    return entries


def sound_path(wave):
    """The logical file path of a soundscript wave (flag characters stripped)."""
    wave = wave.replace("\\", "/").lstrip(SOUND_CHARS).lower()
    return "sound/" + wave


def load_soundscripts(content):
    manifest, _origin = content.read("scripts/game_sounds_manifest.txt")
    if manifest is None:
        return {}, []
    entries, scripts = {}, []
    for script in re.findall(r'"(?:precache|preload)_file"\s+"([^"]+)"',
                             manifest.decode("latin-1")):
        data, _origin = content.read(script)
        if data is None:
            continue
        scripts.append(script.lower())
        for name, waves in parse_soundscript(data.decode("latin-1")).items():
            entries.setdefault(name, waves)
    return entries, scripts


# --------------------------------------------------------------------------
# WAV
# --------------------------------------------------------------------------
def riff_chunks(data):
    if data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        return None
    chunks, offset = {}, 12
    while offset + 8 <= len(data):
        tag, size = data[offset:offset + 4], struct.unpack_from("<I", data, offset + 4)[0]
        chunks.setdefault(tag, data[offset + 8:offset + 8 + size])
        offset += 8 + size + (size & 1)
    return chunks


def wav_info(data):
    """(channels, rate, loop start sample or -1, vdat bytes or None) of a RIFF WAVE."""
    chunks = riff_chunks(data)
    if not chunks or b"fmt " not in chunks:
        return None
    _fmt, channels, rate = struct.unpack_from("<HHI", chunks[b"fmt "], 0)
    loop = -1
    if b"cue " in chunks and len(chunks[b"cue "]) >= 28:
        count = struct.unpack_from("<I", chunks[b"cue "], 0)[0]
        if count:
            loop = struct.unpack_from("<I", chunks[b"cue "], 4 + 20)[0]
    if loop < 0 and b"smpl" in chunks and len(chunks[b"smpl"]) >= 36 + 24:
        if struct.unpack_from("<I", chunks[b"smpl"], 28)[0]:
            loop = struct.unpack_from("<I", chunks[b"smpl"], 36 + 8)[0]
    return channels, rate, loop, chunks.get(b"VDAT")


def chunk(tag, payload):
    return tag + struct.pack("<I", len(payload)) + payload + (b"\0" if len(payload) & 1 else b"")


def build_wav(pcm, channels, rate, loop, vdat):
    fmt = struct.pack("<HHIIHH", 1, channels, rate, rate * channels * 2, channels * 2, 16)
    body = chunk(b"fmt ", fmt)
    if loop >= 0:
        body += chunk(b"cue ", struct.pack("<II", 1, 0) + struct.pack("<I4sIII", 0, b"data", 0, 0, loop))
    body += chunk(b"data", pcm)
    if vdat is not None:
        body += chunk(b"VDAT", vdat)
    return b"RIFF" + struct.pack("<I", 4 + len(body)) + b"WAVE" + body


def convert_wav(data, long_form=False):
    """The DSP-ready form of one WAV (any codec ffmpeg reads), or None.
    long_form: 11025 Hz mono, for sounds too long at the normal form."""
    info = wav_info(data)
    if info is None:
        return None
    channels, rate, loop, vdat = info
    channels = 1 if long_form or channels < 2 else 2
    target = 11025 if long_form or rate <= 11025 else 22050
    result = subprocess.run(
        ["ffmpeg", "-v", "error", "-i", "pipe:0", "-ac", str(channels), "-ar", str(target),
         "-f", "s16le", "-acodec", "pcm_s16le", "pipe:1"],
        input=data, capture_output=True)
    if result.returncode != 0 or not result.stdout:
        return None
    if loop >= 0 and rate:
        loop = min(int(loop * target / rate), max(0, len(result.stdout) // (2 * channels) - 1))
    return build_wav(result.stdout, channels, target, loop, vdat)


# --------------------------------------------------------------------------
# Closure
# --------------------------------------------------------------------------
def entity_sound_references(bsp, entries):
    """(soundscript entry names, direct wave paths) named by a map's entities."""
    names, waves = set(), set()
    for entity in bsp.entities():
        for _key, value in entity.items():
            for text in value if isinstance(value, list) else [value]:
                lowered = text.strip().lower()
                if lowered in entries:
                    names.add(lowered)
                elif lowered.endswith(".wav"):
                    waves.add(sound_path(lowered))
    return names, waves


def stage_sounds(stager, bsps, budget_mb=DEFAULT_BUDGET_MB):
    """Stage the maps' sound closure through stager.put; returns the report entry."""
    content = stager.content
    entries, scripts = load_soundscripts(content)
    for script in scripts:
        stager.copy(script, "soundscripts")
    stager.copy("scripts/game_sounds_manifest.txt", "soundscripts")

    wanted = []  # (wave path, referrer), in priority order
    for bsp, map_name in bsps:
        names, waves = entity_sound_references(bsp, entries)
        for name in sorted(names):
            wanted += [(sound_path(w), "%s entity %s" % (map_name, name)) for w in entries[name]]
        wanted += [(w, "%s entity" % map_name) for w in sorted(waves)]
    for name in sorted(entries):
        if any(fnmatch.fnmatchcase(name, pattern) for pattern in ALWAYS_PREFIXES):
            wanted += [(sound_path(w), "always %s" % name) for w in entries[name]]
    names = content.names()
    wanted += [(n, "always ui") for n in sorted(names) if n.startswith("sound/ui/")]

    budget = budget_mb * 1024 * 1024
    total, staged, over, failed, missing, too_long, long_form = 0, 0, [], [], [], [], []
    seen = set()
    for path, referrer in wanted:
        if path in seen or path in stager.written:
            continue
        seen.add(path)
        data, origin = content.read(path)
        if data is None:
            missing.append(path)
            continue
        converted = convert_wav(data) if path.endswith(".wav") else data
        if converted is not None and len(converted) > SHORT_LIMIT and path.endswith(".wav"):
            converted = convert_wav(data, long_form=True)
            long_form.append(path)
        if converted is None:
            failed.append(path)
            continue
        if len(converted) > LONG_LIMIT:
            too_long.append(path)
            continue
        if total + len(converted) > budget:
            over.append(path)
            continue
        stager.put(path, converted, len(data), "sound", origin)
        total += len(converted)
        staged += 1
    return {"files": staged, "bytes": total, "budget_bytes": budget,
            "long_form": long_form, "too_long": too_long, "over_budget": over,
            "conversion_failed": failed, "missing": missing}
