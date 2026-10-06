#!/usr/bin/env python3
"""A virtual Xbox 360 controller on Linux (uinput), driven by a small timed script.

Real input for the SDL input provider: the device shows up as a joystick with a built-in
gamepad mapping, so the product sees it like hardware. Used by the split-screen acceptance
runs to give each local player a controller (RFC/portal2-splitscreen-progress.md).

    uinput_pad.py --name pad1 --script "wait 20; hold A 2; axis LX 20000 3; wait 1"

Script commands, separated by ';':
    wait <s>                 sleep
    press <BTN>              press and release
    hold <BTN> <s>           press, wait, release
    axis <AXIS> <v> <s>      set an axis (sticks -32768..32767, LT/RT 0..255) for <s> seconds, then center
Buttons: A B X Y LB RB BACK START LS RS UP DOWN LEFT RIGHT. Axes: LX LY RX RY LT RT.
Needs read/write access to /dev/uinput. The device exists until the script ends.
"""

import argparse
import ctypes
import fcntl
import os
import struct
import sys
import time

# linux/input-event-codes.h
EV_SYN, EV_KEY, EV_ABS = 0, 1, 3
BUTTONS = {"A": 0x130, "B": 0x131, "X": 0x133, "Y": 0x134, "LB": 0x136, "RB": 0x137,
           "BACK": 0x13A, "START": 0x13B, "LS": 0x13D, "RS": 0x13E}
HATS = {"UP": (0x11, -1), "DOWN": (0x11, 1), "LEFT": (0x10, -1), "RIGHT": (0x10, 1)}
AXES = {"LX": 0x00, "LY": 0x01, "LT": 0x02, "RX": 0x03, "RY": 0x04, "RT": 0x05}
AXIS_RANGE = {"LX": (-32768, 32767), "LY": (-32768, 32767), "RX": (-32768, 32767),
              "RY": (-32768, 32767), "LT": (0, 255), "RT": (0, 255)}

UI_SET_EVBIT = 0x40045564
UI_SET_KEYBIT = 0x40045565
UI_SET_ABSBIT = 0x40045567
UI_DEV_CREATE = 0x5501
UI_DEV_DESTROY = 0x5502
ABS_CNT = 64


def create(name):
    fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
    fcntl.ioctl(fd, UI_SET_EVBIT, EV_KEY)
    fcntl.ioctl(fd, UI_SET_EVBIT, EV_ABS)
    for code in BUTTONS.values():
        fcntl.ioctl(fd, UI_SET_KEYBIT, code)
    for axis in list(AXES.values()) + [0x10, 0x11]:
        fcntl.ioctl(fd, UI_SET_ABSBIT, axis)
    absmin = [0] * ABS_CNT
    absmax = [0] * ABS_CNT
    for key, code in AXES.items():
        absmin[code], absmax[code] = AXIS_RANGE[key]
    for code in (0x10, 0x11):
        absmin[code], absmax[code] = -1, 1
    # struct uinput_user_dev: name[80], input_id{bustype,vendor,product,version}, ff_effects_max,
    # absmax[64], absmin[64], absfuzz[64], absflat[64]
    dev = struct.pack("80sHHHHi", name.encode()[:79], 3, 0x045E, 0x028E, 0x0114, 0)
    dev += struct.pack("64i", *absmax) + struct.pack("64i", *absmin)
    dev += struct.pack("64i", *([0] * ABS_CNT)) + struct.pack("64i", *([0] * ABS_CNT))
    os.write(fd, dev)
    fcntl.ioctl(fd, UI_DEV_CREATE)
    return fd


def emit(fd, etype, code, value):
    now = time.time()
    os.write(fd, struct.pack("llHHi", int(now), int((now % 1) * 1e6), etype, code, value))
    os.write(fd, struct.pack("llHHi", int(now), int((now % 1) * 1e6), EV_SYN, 0, 0))


def set_button(fd, name, down):
    if name in HATS:
        axis, value = HATS[name]
        emit(fd, EV_ABS, axis, value if down else 0)
    else:
        emit(fd, EV_KEY, BUTTONS[name], 1 if down else 0)


def run(fd, script):
    for command in [c.strip() for c in script.split(";") if c.strip()]:
        words = command.split()
        verb = words[0]
        if verb == "wait":
            time.sleep(float(words[1]))
        elif verb == "press":
            set_button(fd, words[1].upper(), True)
            time.sleep(0.15)
            set_button(fd, words[1].upper(), False)
        elif verb == "hold":
            set_button(fd, words[1].upper(), True)
            time.sleep(float(words[2]))
            set_button(fd, words[1].upper(), False)
        elif verb == "axis":
            axis = words[1].upper()
            emit(fd, EV_ABS, AXES[axis], int(words[2]))
            time.sleep(float(words[3]))
            emit(fd, EV_ABS, AXES[axis], 0)
        else:
            raise SystemExit("uinput_pad: unknown command %r" % command)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--name", default="Virtual Xbox 360 Pad")
    parser.add_argument("--script", default="wait 60", help="commands, separated by ';'")
    parser.add_argument("--ready", help="file to create once the device exists")
    args = parser.parse_args()
    fd = create(args.name)
    try:
        time.sleep(0.5)  # let udev and SDL notice it
        if args.ready:
            open(args.ready, "w").close()
        run(fd, args.script)
    finally:
        fcntl.ioctl(fd, UI_DEV_DESTROY)
        os.close(fd)
    return 0


if __name__ == "__main__":
    sys.exit(main())
