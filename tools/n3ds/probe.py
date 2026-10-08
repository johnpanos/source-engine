#!/usr/bin/env python3
"""One fresh run of the 3DS build with console commands, judged after they ran.

  probe.py [--map n3ds_chamber] [--cmds "r_worldmesh_draw 0; gameui_hide"]
           [--frames 3] [--speed 500] [--keep] [--headless]

Stops any running emulator, boots the build with the map, and queues the
commands after the map load (`wait`, then each command, then a unique
`echo PROBE_<n>` marker). The engine's console streams through console.log;
the probe judges only what follows the marker: it prints the console lines
between the marker and the --frames'th "pica: frame" counter line, those
counters, and a screenshot of both screens (build-3ds/probe.png). It never
matches the command line itself, and it fails fast on a guest stop, a crash
or a stall. The emulator is closed afterwards unless --keep.
"""

import argparse
import random
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import azahar_harness as harness  # noqa: E402
import run_azahar  # noqa: E402
import guest_profile  # noqa: E402


def stop_running_emulator():
    try:
        harness.Session(connect_within=1.0).request("quit")
    except (Exception, SystemExit):
        pass
    deadline = time.time() + 15
    while harness.SOCKET.exists() and time.time() < deadline:
        try:
            harness.Session(connect_within=0.2)
        except (Exception, SystemExit):
            break
        time.sleep(0.2)


def report_samples(samples, top=40, total=None, label="samples of the main thread"):
    """Self time by the sampled PC's function; inclusive time by every
    function named by the PC, LR or a stack word (each counted once per
    sample: return addresses on the stack, with some stale-word noise)."""
    import collections
    addresses = set()
    for pc, lr, stack in samples:
        addresses.update([pc, lr] + stack)
    word = guest_profile.text_reader(harness.ELF)
    calls = {a for a in addresses if guest_profile.is_return_address(word, a)}
    names = harness.symbolize(addresses)
    function = lambda a: names.get(a, "").split(" (")[0] or None
    own, inclusive = collections.Counter(), collections.Counter()
    for pc, lr, stack in samples:
        own[function(pc) or "?"] += 1
        seen = {f for f in map(function, [pc] + [a for a in [lr] + stack if a in calls]) if f}
        inclusive.update(seen)
    count = total or len(samples)
    print("-- %d %s: self" % (len(samples), label))
    for name, hits in own.most_common(top):
        print("  %5.1f%%  %s" % (100.0 * hits / count, name[:150]))
    print("-- inclusive (pc, and lr and stack words that are return addresses)")
    for name, hits in inclusive.most_common(top):
        print("  %5.1f%%  %s" % (100.0 * hits / count, name[:150]))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--content", default="build-3ds-content/n3ds_chamber")
    parser.add_argument("--map", default="n3ds_chamber")
    parser.add_argument("--demo", default=None, help="play this demo (game-relative, e.g. intro4) instead of loading --map")
    parser.add_argument("--timedemo", default=None,
                        help="time this demo (timedemo: frames, seconds, fps and ms/frame on the guest clock)")
    parser.add_argument("--cmds", default="", help="console commands, separated by ';'")
    parser.add_argument("--launch", default="", help="extra launch arguments (e.g. -pica_dump_draws 400)")
    parser.add_argument("--wait", type=int, default=300, help="frames to wait after the map load")
    parser.add_argument("--frames", type=int, default=2, help="counter lines to collect after the marker")
    parser.add_argument("--speed", type=float, default=None, help="emulation speed percent (0 = unlimited)")
    parser.add_argument("--timeout", type=float, default=900)
    parser.add_argument("--stall", type=float, default=120)
    parser.add_argument("--keep", action="store_true", help="leave the emulator running")
    parser.add_argument("--headless", action="store_true")
    parser.add_argument("--guest-profile", type=float, default=0, metavar="US",
                        help="after the marker, sample core 0 inside the emulator every US microseconds of "
                             "emulated time until the run ends (Azahar harness 'profile'); prints idle "
                             "share, threads and the hottest functions by emulated time")
    parser.add_argument("--sample", type=int, default=0,
                        help="after the marker, sample the main thread's PC and stack this many times "
                             "and print the hottest functions (self, and inclusive from stack words)")
    options = parser.parse_args()

    marker = "PROBE_%08x" % random.getrandbits(32)
    commands = [c.strip() for c in options.cmds.split(";") if c.strip()]
    args = "+wait %d %s +echo %s" % (options.wait, " ".join("+" + c for c in commands), marker)

    stop_running_emulator()
    app = harness.package()
    start = ("+timedemo %s" % options.timedemo if options.timedemo
             else "+playdemo %s" % options.demo if options.demo else "+map %s" % options.map)
    run_azahar.stage(options.content, "%s %s %s" % (options.launch, start, args))
    console = harness.GAME / "console.log"
    if console.exists():
        console.unlink()
    harness.start_emulator(options.headless, app)
    session = harness.Session()
    if options.speed is not None:
        session.request("speed %g" % options.speed)

    samples = []
    tail = harness.ConsoleTail()
    seen_marker, after, counters = False, [], []
    start = last_progress = time.time()
    last_size = -1
    verdict = "timeout"
    while time.time() - start < options.timeout:
        for event in session.poll_events():
            if event.get("event") == "guest_stop":
                print("GUEST STOP (%s)" % event.get("reason"))
                if event.get("report"):
                    print(event["report"])
                harness.report_threads(session, reply=event.get("dump", {}))
                verdict = "guest_stop"
                break
        if verdict == "guest_stop":
            break
        for line in tail.pump(echo=False):
            text = line.strip()
            if not seen_marker:
                if text == marker:
                    seen_marker = True
                    print("marker reached at %.0f s" % (time.time() - start))
                    if options.guest_profile:
                        print("guest profile: %s" % session.request("profile start %g" % options.guest_profile))
                continue
            if text.startswith("pica: frame"):
                counters.append(text)
            elif not text.startswith("pica: textures"):
                after.append(text)
        if seen_marker and options.sample and len(samples) < options.sample:
            reply = session.request("threads 48")
            app = [t for t in reply.get("threads", [])
                   if t.get("status") != "dead" and int(t["regs"][15], 16) < harness.TEXT_END]
            main = [t for t in app if str(t.get("name", "")).endswith("-main")] or app[:1]
            if main:
                t = main[0]
                samples.append((int(t["regs"][15], 16), int(t["regs"][14], 16),
                                [int(w, 16) for w in t["stack"]]))
        if seen_marker and len(counters) >= options.frames and len(samples) >= options.sample:
            verdict = "ok"
            break
        size = console.stat().st_size if console.exists() else -1
        if size != last_size:
            last_size, last_progress = size, time.time()
        elif time.time() - last_progress > options.stall:
            verdict = "stall"
            break
        time.sleep(0.25)

    if options.guest_profile and seen_marker:
        dump = harness.azahar_ns.HOME / "guest_profile.bin"
        print("guest profile: %s (%s)" % (session.request("profile stop %s" % dump), dump))
    print("verdict: %s" % verdict)
    if options.guest_profile and seen_marker:
        guest_profile.report(harness.azahar_ns.HOME / "guest_profile.bin", harness.ELF)
    if samples:
        report_samples(samples)
    if after:
        print("-- console after the marker")
        for text in after[:80]:
            print("  " + text)
    print("-- frame counters after the marker")
    for text in counters:
        print("  " + text)
    shot = harness.azahar_ns.PROBE_PNG
    try:
        if shot.exists():
            shot.unlink()
        reply = session.request("screenshot %s" % shot)
        # The frontend writes the next presented frame; quitting first loses it.
        deadline = time.time() + 30
        while reply.get("ok") and not shot.exists() and time.time() < deadline:
            time.sleep(0.1)
        print("screenshot: %s" % (shot if shot.exists() else "none within 30 s (%s)" % reply))
    except Exception as error:
        print("screenshot failed: %s" % error)
    if not options.keep:
        session.request("quit")
    return 0 if verdict == "ok" else 1


if __name__ == "__main__":
    sys.exit(main())
