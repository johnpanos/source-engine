# VGUI fixture materials (`vgui-fixture-materials/v1`)

Repository-owned materials to draw VGUI with, for RFC 0010's fixed-screen
corpus, the surface counters and, later, the draw-list recorder and its
consumers, without licensed game content.

`materials.json` is the recorded manifest. The files themselves are not
committed: `tools/vgui/vgui_fixture_content.py` draws every texture from
closed-form rules and writes byte-identical files each time.

```sh
python3 tools/vgui/vgui_fixture_content.py write OUT_DIR   # OUT_DIR/materials/...
python3 tools/vgui/vgui_fixture_content.py check           # suite vgui.fixture-materials
```

| Material | Size | Blend | Exercises |
| --- | --- | --- | --- |
| `vgui/fixture/quadrants` | 64×64 | alpha | orientation and texture coordinates (red, green, blue and white quarters), vertex color |
| `vgui/fixture/alpha_ramp` | 256×4 | alpha | alpha blending: alpha equals the column |
| `vgui/fixture/additive` | 64×64 | additive | `$additive` |
| `vgui/fixture/opaque` | 64×64 | opaque | the texture's alpha is 0 and must be ignored |
| `vgui/fixture/checker` | 64×64 | alpha | one-texel checkerboard, point sampled: texel alignment |
| `vgui/fixture/atlas` | 128×128 | alpha | 4×4 cells for `DrawTexturedSubRect` |
| `vgui/fixture/frames` | 16×16, 3 frames | alpha | `DrawSetTextureFrame` (`$frame`) |
| `vgui/fixture/wide` | 64×16 | alpha | a non-square texture |
| `vgui/fixture/tinted` | (quadrants) | alpha | `$color`: state only the material holds |
| `vgui/hud/800corner1`–`4` | 32×32 | alpha | `vgui_controls`' rounded panel corners |
| `vgui/fixture/titlebar_icon`, `_disabled` | 16×16 | alpha | the Frame system button's icons (named by the fixture scheme) |
| `vgui/cursors/*` (13) | 32×32 | alpha | the software cursors `vguimatsurface/Cursor.cpp` loads |

Each entry's probe texels in the manifest are literal values, written apart
from the code that draws the pixels. `check` reads them back with
`tools/quality/vtf_decode.py` and with the engine's own VTF container reader
(`texturecontainer::vtf`). A material's declared blend must be the one the
surface derives from its VMT flags.

## The fixture game (`game/`)

`game/` is the rest of a game directory, committed: `gameinfo.txt`, the
fixture scheme `resource/FixtureScheme.res` and the packaged font
`resource/DejaVuSans.ttf` (DejaVu Sans 2.37, Bitstream Vera license, recorded
in `resource/DejaVuSans-LICENSE.txt`). The scheme registers the font under the
fixture-only name `VGUIFixtureSans`, so no system font can stand in for it,
and names the fixture title-bar icons. `tools/vgui/vgui_fixture_host.py`
stages `game/` plus the generated materials as the game `vguifixture` and runs
the VGUI fixture host in it (suite `vgui.fixture-host`).

Linux's font manager still asks fontconfig for a foreign-script fallback
(`WenQuanYi Zen Hei`) for code points above U+00FF; the fixture screens draw
only ASCII. If that fallback is missing, the font manager retries the whole
font as the system's "DejaVu Sans", and the host's oracle fails on that
request rather than accept a substitute.
