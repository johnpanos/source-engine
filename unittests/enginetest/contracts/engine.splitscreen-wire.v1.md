# engine.splitscreen-wire.v1

Bit-level payloads of the local split-screen net messages
(`common/splitscreen_wire.h`). Derived from retail Portal 2's
`ReadFromBuffer` functions (`RFC/portal2-splitscreen-retail-engine.md`); ids are
this fork's own (18 clc, 34 svc, 35 net) and are used only after both ends
advertise the extension, so one-player peers are unaffected.

| Message | Fields |
| --- | --- |
| `net_SplitScreenUser` | slot, 1 bit |
| `svc_SplitScreen` | action 1 bit (0 add, 1 remove), slot 1 bit, entity index 11 bits (>= 1) |
| `clc_SplitPlayerConnect` | count 8 bits (1..2), then per player two NUL-terminated strings (max 0x104 bytes); the second is carried and ignored |

Obligations:

1. A write refuses out-of-range input before writing any bit.
2. A read that fails (overflow, out-of-range slot, entity 0, count 0 or above 2,
   truncated strings) leaves the output unchanged.
3. Round trips are exact, in exactly the stated number of bits.
4. Message ids are distinct and fit the 6-bit type field.

Suites: `engine.splitscreen-wire` (real codecs) and
`engine.splitscreen-wire.sensitivity` (four deliberately bad codecs must be caught).
This covers encoding only: no engine state, admission, or netchannel behavior.
