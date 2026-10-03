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
| `vgui/cursors/*` (13) | 32×32 | alpha | the software cursors `vguimatsurface/Cursor.cpp` loads |

Each entry's probe texels in the manifest are literal values, written apart
from the code that draws the pixels. `check` reads them back with
`tools/quality/vtf_decode.py` and with the engine's own VTF container reader
(`texturecontainer::vtf`). A material's declared blend must be the one the
surface derives from its VMT flags.

The fixtures need a game directory to mount them and a scheme and fonts to
draw text; neither is part of this set yet.
