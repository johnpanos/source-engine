# VBSP collision source provenance

`ivp.cpp`, `ivp.h`, `disp_ivp.cpp`, and `disp_ivp.h` restore the VBSP
collision and displacement implementations missing from this checkout. Their
source is Valve's official Source SDK 2013 repository at commit
`b8cfb12c0e083a2ef5b2f9f9b50f3902fa034474`, under
[`src/utils/vbsp/`](https://github.com/ValveSoftware/source-sdk-2013/tree/b8cfb12c0e083a2ef5b2f9f9b50f3902fa034474/src/utils/vbsp).
The original file SHA-256 values, in that order, are:

```
5fdb5297fbaa8977900a042df9653f600cef09ed562a6888f062eacfbcede91a
3e26cb335d5e490ab90575e149b1f76a92ee39a86e735a4d8f7dfe0ca41ce51e
ab895f390fb6181f9d5a2c5cd47d05a854ff3c8987fb89f4e5a05e09d6172162
fe21148ecb0eed85d27727926164b0f5206ad412f8996a98a6b701c2a4b8196c
```

The only source changes before mechanical `.clang-format` formatting were
include paths for this Linux checkout: `KeyValues.h` to
`tier1/KeyValues.h`, `UtlBuffer.h` to `utlbuffer.h`, and
`../../public/disp_tesselate.h` to `disp_tesselate.h`. Copyright headers are
retained. Distribution remains subject to the repository's `LICENSE` and
`thirdpartylegalnotices.txt`.

Both translation units compile in the isolated Linux host-tool profile. VBSP
now links and is registered in the `--tools` product. The self-contained
host-tool smoke compiles a v21 BSP with physics and verifies a byte-exact BSP2
convert/export round trip. This does not establish byte-identical output
against Valve's VBSP or Portal 2 content compatibility.
