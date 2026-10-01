# bcdec source record

This directory vendors `bcdec.h` v0.985 from
<https://github.com/StrataSource/bcdec> at commit
`80859ed3b7afb1c527a2a99d70c61457bea72d0c` (2026-09-02).

The file is used under Alternative A, the MIT license reproduced in `LICENSE`.
The vendored header is unmodified. Its SHA-256 digest is
`134520764d96f70a27db814173616c89ad85db95fbd3328417c7a8c77e7ca189`.

Only the BC7 decompressor entry point is called by `hammer.formats`; no encoder
or other BC format entry point is used or linked into the decoder target.
