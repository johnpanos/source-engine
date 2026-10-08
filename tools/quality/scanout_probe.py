#!/usr/bin/env python3
"""Direct-scanout probe: does the compositor scan a fullscreen client out?

Read-only. Samples the framebuffers that the display's active planes scan out
(DRM GETPLANE/GETFB2 on the card node; no DRM master needed) before, during
and after a client runs, and parses the client's Wayland protocol log
(WAYLAND_DEBUG=1) for zwp_linux_dmabuf_feedback_v1 scanout tranches.

Verdict (`run`):
  scanout   the compositor offered a scanout tranche and the full-size plane
            scanned framebuffers that exist only while the client ran (the
            client's own buffers, imported for direct scanout);
  composited  the plane only ever scanned the compositor's own buffers.

It opens a window on the display the client is given: run it on a real
session only with the user's consent (an isolated headless compositor has no
planes, which is a valid negative control: no scanout tranche, no new FBs).

  scanout_probe.py planes
  scanout_probe.py run --seconds 20 --out DIR -- ./kiln play portal2 -- -fullscreen ...
"""
import argparse
import array
import fcntl
import glob
import json
import os
import re
import struct
import subprocess
import sys
import time


def _iowr(nr, size):
    return (3 << 30) | (size << 16) | (ord('d') << 8) | nr


SET_CLIENT_CAP = _iowr(0x0D, 16)
GETPLANERESOURCES = _iowr(0xB5, 16)
GETPLANE = _iowr(0xB6, 32)
GETFB2 = _iowr(0xCE, 104)
CAP_UNIVERSAL_PLANES = 2


def fourcc(code):
    return ''.join(chr((code >> (8 * i)) & 0xFF) for i in range(4))


def open_card(path=None):
    for candidate in ([path] if path else sorted(glob.glob('/dev/dri/card*'))):
        try:
            fd = os.open(candidate, os.O_RDWR | os.O_CLOEXEC)
        except OSError:
            continue
        fcntl.ioctl(fd, SET_CLIENT_CAP, struct.pack('QQ', CAP_UNIVERSAL_PLANES, 1))
        return fd, candidate
    raise SystemExit('scanout_probe: no readable /dev/dri/card* node')


def plane_ids(fd):
    buf = bytearray(struct.pack('QI4x', 0, 0))
    fcntl.ioctl(fd, GETPLANERESOURCES, buf)
    count = struct.unpack_from('QI', buf)[1]
    ids = array.array('I', [0] * count)
    addr = ids.buffer_info()[0]
    buf = bytearray(struct.pack('QI4x', addr, count))
    fcntl.ioctl(fd, GETPLANERESOURCES, buf)
    return list(ids)


def sample(fd, planes):
    """One snapshot: [(plane, crtc, fb, width, height, format, modifier)]."""
    out = []
    for plane in planes:
        buf = bytearray(struct.pack('6IQ', plane, 0, 0, 0, 0, 0, 0))
        try:
            fcntl.ioctl(fd, GETPLANE, buf)
        except OSError:
            continue
        _, crtc, fb = struct.unpack_from('3I', buf)
        if not fb:
            continue
        info = bytearray(104)
        struct.pack_into('I', info, 0, fb)
        try:
            fcntl.ioctl(fd, GETFB2, info)
            width, height, fmt = struct.unpack_from('3I', info, 4)
            modifier = struct.unpack_from('Q', info, 72)[0]  # u64 modifier[4], 8-byte aligned
        except OSError:
            width = height = fmt = modifier = 0
        out.append((plane, crtc, fb, width, height, fourcc(fmt) if fmt else '?', modifier))
    return out


def collect(fd, planes, seconds, interval=0.004):
    seen = {}
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        for entry in sample(fd, planes):
            seen.setdefault(entry[2], entry)
        time.sleep(interval)
    return seen


def full_size(entries):
    """FBs on the largest planes (the display-sized primary scanout)."""
    if not entries:
        return {}
    area = max(e[3] * e[4] for e in entries.values())
    return {fb: e for fb, e in entries.items() if e[3] * e[4] == area}


TRANCHE = re.compile(r'zwp_linux_dmabuf_feedback_v1#\d+\.tranche_flags\((\d+)\)')


def parse_feedback(log_text):
    flags = [int(m.group(1)) for m in TRANCHE.finditer(log_text)]
    return {'tranches': len(flags), 'scanout_tranches': sum(1 for f in flags if f & 1)}


def session_pids(sid):
    """Live processes in session 'sid' (the client and anything it spawned
    or re-executed into, which keeps the session)."""
    pids = []
    for stat in glob.glob('/proc/[0-9]*/stat'):
        try:
            with open(stat) as f:
                fields = f.read().rsplit(')', 1)[1].split()
        except OSError:
            continue
        if fields[0] != 'Z' and int(fields[3]) == sid:
            pids.append(int(stat.split('/')[2]))
    return pids


def stop_session(sid):
    for sig, wait in ((15, 10.0), (9, 5.0)):
        for pid in session_pids(sid):
            try:
                os.kill(pid, sig)
            except OSError:
                pass
        end = time.monotonic() + wait
        while session_pids(sid) and time.monotonic() < end:
            time.sleep(0.2)
    return not session_pids(sid)


def describe(entry):
    plane, crtc, fb, w, h, fmt, mod = entry
    return {'plane': plane, 'crtc': crtc, 'fb': fb, 'size': [w, h], 'format': fmt,
            'modifier': hex(mod)}


def cmd_planes(args):
    fd, card = open_card(args.card)
    print(json.dumps({'card': card, 'planes': [describe(e) for e in sample(fd, plane_ids(fd))]},
                     indent=1))


def cmd_run(args):
    if not args.command:
        raise SystemExit('scanout_probe: run needs a client command after --')
    os.makedirs(args.out, exist_ok=True)
    fd, card = open_card(args.card)
    planes = plane_ids(fd)
    before = collect(fd, planes, args.baseline)
    log_path = os.path.join(args.out, 'client.log')
    env = dict(os.environ, WAYLAND_DEBUG='1')
    with open(log_path, 'w') as log:
        client = subprocess.Popen(args.command, stdout=log, stderr=subprocess.STDOUT, env=env,
                                  start_new_session=True)
        sid = client.pid  # start_new_session: the client leads its session
        time.sleep(args.settle)
        during = collect(fd, planes, args.seconds) if session_pids(sid) else {}
        alive = bool(session_pids(sid))
        stopped = stop_session(sid)
        client.wait()
        if not stopped:
            print('scanout_probe: client session survived SIGKILL', file=sys.stderr)
    after = collect(fd, planes, args.baseline)
    with open(log_path, errors='replace') as log:
        feedback = parse_feedback(log.read())
    compositor = set(before) | set(after)
    client_fbs = {fb: e for fb, e in full_size(during).items() if fb not in compositor}
    scanout = alive and bool(client_fbs) and feedback['scanout_tranches'] > 0
    result = {
        'card': card,
        'client_alive_through_sampling': alive,
        'feedback': feedback,
        'compositor_fbs': [describe(e) for e in full_size({**before, **after}).values()],
        'client_fbs_scanned': [describe(e) for e in client_fbs.values()],
        'verdict': 'scanout' if scanout else 'composited',
    }
    with open(os.path.join(args.out, 'scanout.json'), 'w') as f:
        json.dump(result, f, indent=1)
    print(json.dumps(result, indent=1))
    return 0 if scanout or args.expect == 'composited' else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--card', help='DRM card node (default: first readable)')
    sub = parser.add_subparsers(dest='cmd', required=True)
    sub.add_parser('planes', help='print the planes and FBs being scanned now')
    run = sub.add_parser('run', help='sample planes around a client run')
    run.add_argument('--out', required=True)
    run.add_argument('--seconds', type=float, default=15.0, help='sampling while the client runs')
    run.add_argument('--settle', type=float, default=15.0, help='wait before sampling')
    run.add_argument('--baseline', type=float, default=2.0, help='sampling before and after')
    run.add_argument('--expect', choices=['scanout', 'composited'], default='scanout')
    run.add_argument('command', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    if getattr(args, 'command', None) and args.command[0] == '--':
        args.command = args.command[1:]
    return cmd_planes(args) if args.cmd == 'planes' else cmd_run(args)


if __name__ == '__main__':
    sys.exit(main() or 0)
