# Source Engine Hard Limits

This document records the largest content and protocol limits around maps,
materials, textures, and entities in this source tree. It distinguishes between
compiler/BSP limits, runtime object limits, renderer limits, and limits implied
by field widths. Those limits are not interchangeable: for example, a map may
contain more compiled entity records than the server can keep alive as
networked edicts.

The BSP counts below use the normal `!BSP_USE_LESS_MEMORY` configuration. The
`BSP_USE_LESS_MEMORY` branch sets many static-array sizes to `2` for Xbox 360;
those values are storage placeholders, not content limits.

## Summary

| Area | Limit | Meaning |
| --- | ---: | --- |
| World bounds | -16,384 to +16,384 per axis | A 32,768-unit-wide world cube |
| Map/compiler entities | 8,192 | Entities accepted while reading or compiling map data |
| Runtime entity handles | 4,096 | Networked plus client-only/server-only entity slots |
| Networked edicts | 2,048 | Total network-visible entity slots, including world and players |
| Unique VBSP material references | 1,024 | Distinct source material/texture names |
| BSP texture-data mappings | 2,048 | `texdata` records, including generated or aliased mappings |
| BSP surface mappings | 12,288 | `texinfo` material/transform/flag combinations |
| Studio-model materials | 32 | Total texture/material slots in one model |
| Linux ToGL 2D texture | 4,096 x 4,096 | Hard-coded renderer capability |

## World size and coordinates

`MAX_COORD_INTEGER` is 16,384, with the corresponding minimum at -16,384.
The server checks the loaded world model and terminates the map if any axis
extends beyond that range. The resulting maximum world span is 32,768 units on
each axis.

Sources:

- [`public/worldsize.h`](public/worldsize.h#L15-L35)
- [`engine/world.cpp`](engine/world.cpp#L49-L56)

This is not an isolated map-loader constant. Coordinates use 14 integer bits on
the network, and `coordsize.h` has a compile-time check requiring the bit count
and `MAX_COORD_INTEGER` to agree. BSP nodes and leaves also store bounds in
shorts. Increasing the world size therefore changes network encoding and BSP
storage assumptions, not just collision or editor bounds.

Sources:

- [`public/coordsize.h`](public/coordsize.h#L13-L40)
- [`public/worldsize.h`](public/worldsize.h#L17-L19)

## BSP and map geometry

The principal BSP v21 design limits are defined together in `bspfile.h`:

| BSP structure | Limit |
| --- | ---: |
| Models/submodels | 1,024 |
| Brushes | 8,192 |
| Entities | 8,192 |
| Planes | 65,536 |
| Nodes | 65,536 |
| Brush sides | 65,536 |
| Leaves | 65,536 |
| Vertices | 65,536 |
| Faces | 65,536 |
| Leaf faces | 65,536 |
| Leaf brushes | 65,536 |
| Portals | 65,536 |
| Visibility clusters | 65,536 |
| Vertex normals | 256,000 |
| Vertex-normal indices | 256,000 |
| Portal vertices | 128,000 |
| Edges | 256,000 |
| Surfedges | 512,000 ceiling; the renderer loader requires the count to be lower |
| Areas | 256 |
| Area portals | 1,024 |
| Visibility lump | 16 MiB |
| World lights | 8,192 |
| Cubemap samples | 1,024 |
| Ordinary overlays | 512 |
| Water overlays | 16,384 |
| Primitives | 32,768 |
| Primitive vertices | 65,536 |
| Primitive indices | 65,536 |

Sources:

- [`public/bspfile.h`](public/bspfile.h#L56-L102)
- [`engine/cmodel_bsp.cpp`](engine/cmodel_bsp.cpp#L650-L665)
- [`engine/cmodel_bsp.cpp`](engine/cmodel_bsp.cpp#L832-L845)
- [`engine/cmodel_bsp.cpp`](engine/cmodel_bsp.cpp#L870-L925)
- [`engine/cmodel_bsp.cpp`](engine/cmodel_bsp.cpp#L978-L986)
- [`engine/modelloader.cpp`](engine/modelloader.cpp#L2648-L2661)

### Displacements

A map may contain 4,096 displacement-info records. Displacement power is limited
to 4, producing at most 17 x 17, or 289, vertices and 512 triangles for an
individual displacement. The declared aggregate displacement-vertex capacity is
1,183,744 (`4096 * 289`).

Source: [`public/bspfile.h`](public/bspfile.h#L46-L69)

### BSP file size

There is no single `MAX_BSP_FILE_SIZE` constant. The BSP header has 64 lumps,
and each lump stores its offset, stored length, and uncompressed length in
signed 32-bit integers. This creates an approximate sub-2-GiB format envelope,
but the per-structure and per-lump limits above normally fail first.

Source: [`public/bspfile.h`](public/bspfile.h#L371-L393)

## Materials

### Map materials

VBSP admits at most 1,024 unique input material names. This is enforced by
`FindMiptex`. The resulting BSP can contain up to 2,048 `texdata` mappings and
12,288 `texinfo` records:

- A material name is the source-level reference.
- A `texdata` record represents a unique texture mapping and may include
  compiler-generated or aliased mappings.
- A `texinfo` record combines material data with texture/lightmap transforms and
  surface flags, so many `texinfo` records may refer to the same material.

Sources:

- [`utils/vbsp/textures.cpp`](utils/vbsp/textures.cpp#L50-L74)
- [`utils/vbsp/textures.cpp`](utils/vbsp/textures.cpp#L394-L428)
- [`utils/common/bsplib.cpp`](utils/common/bsplib.cpp#L2622-L2627)
- [`public/bspfile.h`](public/bspfile.h#L60-L64)

The texture-name storage additionally declares 256,000 bytes of string data and
65,536 string-table entries. The 2,048 `texdata` ceiling is normally the tighter
material-mapping constraint.

Source: [`public/bspfile.h`](public/bspfile.h#L92-L98)

### Model materials

A studio model is limited to 32 total texture/material slots. `studiomdl`
enforces this during model compilation.

Sources:

- [`public/studio.h`](public/studio.h#L72-L83)
- [`utils/studiomdl/studiomdl.cpp`](utils/studiomdl/studiomdl.cpp#L921-L923)

### Runtime material registry

`MaterialHandle_t` is an unsigned 16-bit value, and the material dictionary uses
it as its tree index. The allocator reserves `(MaterialHandle_t)-1`, or `0xffff`,
as the invalid index. The implied type ceiling is therefore 65,535 usable
indices (`0` through `65534`). This is a representation ceiling, not a tested or
recommended operational material budget.

Sources:

- [`public/materialsystem/imaterialsystem.h`](public/materialsystem/imaterialsystem.h#L553-L560)
- [`materialsystem/cmaterialdict.h`](materialsystem/cmaterialdict.h#L88-L95)
- [`public/tier1/utlmemory.h`](public/tier1/utlmemory.h#L88-L94)

The game DLL also creates a separate network material-name table with 1,024
slots. Some consumers reserve entry 1,023 as an invalid sentinel, leaving 1,023
usable entries for those consumers. This table is unrelated to the global
material registry.

Sources:

- [`game/server/networkstringtable_gamedll.h`](game/server/networkstringtable_gamedll.h#L18-L25)
- [`game/server/gameinterface.cpp`](game/server/gameinterface.cpp#L1412-L1421)

## Textures and lightmaps

### Renderer texture dimensions

The Linux ToGL path reports the following fixed capabilities:

- Maximum 2D width: 4,096
- Maximum 2D height: 4,096
- Maximum volume-texture extent: 1,024

Source: [`togl/linuxwin/dxabstract.cpp`](togl/linuxwin/dxabstract.cpp#L1148-L1156)

The Direct3D 9 path instead copies the dimensions reported by the hardware.
Texture loading consults the active hardware configuration and repeatedly skips
mip levels until width, height, and depth fit. Consequently, a VTF larger than
the active renderer limit may be downsampled rather than used at its original
resolution.

Sources:

- [`materialsystem/shaderapidx9/shaderdevicedx8.cpp`](materialsystem/shaderapidx9/shaderdevicedx8.cpp#L820-L837)
- [`materialsystem/ctexture.cpp`](materialsystem/ctexture.cpp#L4438-L4468)

### VTF representation and loader guard

VTF 7.x stores width, height, frame count, and, in applicable versions, depth as
unsigned 16-bit fields. Mip count is an unsigned 8-bit field. These are file
representation limits rather than guarantees that the renderer can allocate
such a texture.

Source: [`public/vtf/vtf.h`](public/vtf/vtf.h#L468-L544)

The VTF loader also applies a nominal 32-million aggregate-dimension guard. It
sums `Q_log2` values for width, height, depth, and frame count and rejects values
above `log2(32 * 1024 * 1024)`. For ordinary power-of-two data this corresponds
to at most 33,554,432 width-height-depth-frame elements. It is not a byte-size
calculation and does not replace renderer capability checks.

Source: [`vtf/vtf.cpp`](vtf/vtf.cpp#L1063-L1080)

### Lightmaps

Individual lightmap blocks are capped at:

- Brush surfaces: 32 x 32 without border, 35 x 35 including border
- Displacements: 125 x 125 without border, 128 x 128 including border

Source: [`public/bspfile.h`](public/bspfile.h#L28-L41)

The runtime material system normally packs those blocks into 512 x 256 lightmap
pages, clamped down if the renderer reports smaller texture limits.

Source: [`materialsystem/cmatlightmaps.cpp`](materialsystem/cmatlightmaps.cpp#L107-L137)

## Entities and networking

The relevant entity counts form a narrowing pipeline:

```text
8,192 map/compiler entity records
        |
        | compile-time stripping, conversion, and runtime filtering
        v
4,096 total runtime entity-handle slots
        |
        | only the first half is networkable
        v
2,048 networked edicts
```

### Map/compiler entities: 8,192

Both the map reader and BSP tools use `MAX_MAP_ENTITIES`. This count includes
entities that will not remain ordinary runtime entities. For example, VBSP
converts `prop_static` entities into a static-prop game lump and removes them
from the ordinary entity lump.

Sources:

- [`public/bspfile.h`](public/bspfile.h#L60-L64)
- [`utils/common/map_shared.cpp`](utils/common/map_shared.cpp#L63-L73)
- [`utils/vbsp/staticprop.cpp`](utils/vbsp/staticprop.cpp#L591-L669)

### Runtime handles: 4,096

`NUM_ENT_ENTRY_BITS` is one bit wider than `MAX_EDICT_BITS`, producing 4,096
entity-handle entries. The first 2,048 entries are networkable; the remainder
are client-only or server-only.

Sources:

- [`public/const.h`](public/const.h#L64-L87)
- [`game/shared/entitylist_base.h`](game/shared/entitylist_base.h#L104-L110)
- [`public/basehandle.h`](public/basehandle.h#L58-L66)

Runtime map spawning allocates its deferred spawn arrays with exactly 4,096
entries. Entries are appended without a separate local bounds check, relying on
entity allocation, filtering, and immediately removed transient entities to
keep the surviving list within the entity-handle capacity. Thus an 8,192-entry
BSP does not imply that 8,192 game entities can coexist.

Source: [`game/server/mapentities.cpp`](game/server/mapentities.cpp#L306-L318),
[`game/server/mapentities.cpp`](game/server/mapentities.cpp#L417-L434)

### Networked edicts: 2,048

`MAX_EDICT_BITS` is 11, producing 2,048 edict indices. This total includes the
world, players, and all other network-visible entities, so fewer than 2,048
slots are available for arbitrary gameplay objects.

Source: [`public/const.h`](public/const.h#L64-L72)

The bit width is embedded throughout packet encoding, snapshots, entity
handles, effects, sounds, and fixed-size bit vectors and arrays. Raising the
edict limit is therefore a protocol and data-layout change rather than a local
constant adjustment.

Examples:

- [`common/netmessages.cpp`](common/netmessages.cpp#L1518-L1537)
- [`engine/sv_ents_write.cpp`](engine/sv_ents_write.cpp#L844-L852)
- [`game/server/sendproxy.cpp`](game/server/sendproxy.cpp#L24-L33)

### Other entity-related limits

| Resource | Limit | Source |
| --- | ---: | --- |
| Networkable server classes | 512 | [`public/const.h`](public/const.h#L69-L72) |
| Packed snapshot data per entity | 16 KiB | [`public/const.h`](public/const.h#L90-L95) |
| Delta properties per entity | 4,096 | [`public/const.h`](public/const.h#L90-L95) |
| Model precache entries | 4,096 | [`common/qlimits.h`](common/qlimits.h#L26-L31) |
| Generic precache entries | 512 | [`common/qlimits.h`](common/qlimits.h#L26-L33) |
| Base decals | 512 | [`common/qlimits.h`](common/qlimits.h#L30-L33) |
| Protocol absolute player limit | 255 | [`public/const.h`](public/const.h#L35-L37) |
| Normal game-DLL player array | 33 slots | [`game/shared/shareddefs.h`](game/shared/shareddefs.h#L219-L231) |
| Counter-Strike game-DLL player array | 65 slots | [`game/shared/shareddefs.h`](game/shared/shareddefs.h#L219-L231) |

The 33- and 65-slot game-DLL values include an extra slot used when HLTV or
Replay is enabled; game rules choose a playable maximum below the applicable
array limit.

## Change-risk summary

| Limit family | Coupling | Relative change risk |
| --- | --- | --- |
| World extent | BSP short bounds, collision, traces, 14-bit network coordinates | Very high |
| Networked edicts | Wire bit widths, EHANDLE encoding, snapshots, fixed arrays and bit vectors | Very high |
| Runtime entity handles | EHANDLE layout and game/client entity arrays | High |
| BSP geometry counts | Compiler arrays, loader validation, and several 16-bit indices | High |
| Map materials/`texdata`/`texinfo` | VBSP tables, BSP lumps, loaders, and material sorting | Medium to high |
| Texture dimensions | VTF loader guard, renderer capabilities, mip selection, and memory | Backend-dependent |
| Studio-model materials | Model compiler arrays and studio-model format assumptions | Medium to high |

Before raising a limit, search for both the symbolic constant and independently
encoded field widths. Several of the strongest couplings—network bit counts,
unsigned-short indices, and fixed arrays—do not necessarily reference the
headline limit at every use site.
