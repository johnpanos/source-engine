# Authored OpenUSD scenes → playable BSP2 maps

These manifests use the same one-command pipeline as the PBRT scenes
([`../pbrt-maps/README.md`](../pbrt-maps/README.md)); a `.usd`/`.usda`/`.usdc`
`scene` is read by `tools/quality/usd_scene.py` in the `scene` step.

| Manifest | Scene | Source |
| --- | --- | --- |
| `usd-room.json` | `room/room.usda` (checked in) | hand-authored fixture |
| `world-lobby.json` | `World_Lobby/World_Lobby.usd` (not checked in) | NVIDIA Omniverse "World Lobby" sample, `Collected_World_Lobby.zip` |

## World Lobby

The sample is about 3.4 GB zipped and is not redistributed. Unpack it at the
repository root without its redundant USDZ copy and thumbnails. `/World_Lobby/`
is gitignored:

```sh
mkdir World_Lobby && cd World_Lobby
unzip -q /path/to/Collected_World_Lobby.zip -x 'USDZ/*' '*.thumbs/*'
chmod -R u+rwX .   # the archive stores directories without the x bit
cd ..
python3 tools/quality/pbrt_map_build.py \
  --manifest quality/fixtures/usd-maps/world-lobby.json \
  --out quality-results/world-lobby-map --boot
./play world_lobby
```

Scene decisions recorded in the manifest:

- Omniverse materials are MDL. The reader uses each material's baked
  `UsdPreviewSurface` fallback (textures under `Materials/Bake/` and
  `*/baked_textures_*`).
- The camera (`CamLobbyReception`) has `exposure = -10`. The reader applies
  the camera's linear exposure scale to every UsdLux light, so bakes see
  display-level light. Preview-surface emission is already display-level
  and is kept as authored.
- The three stone beds under the planter bowls are 1.87M triangles each.
  `simplify` vertex-clusters them to a 1 cm grid, about 0.19M each; the
  scene model's `approximations` record the exact counts.
- Collision uses the floor and ceiling bounds as the shell, plus the plaster
  walls, columns, furniture, planters and lamps as solids. Each wall with a
  door is one connected part, so its convex solid closes the doorway. The
  player stays inside the furnished lobby.
- Light shaping (cone/IES) on the fixture disc lights is ignored, as the
  reader records.
