# render.indirect-policy.v1

`public/render/indirect_policy.h` is the single owner of RFC 0011's
indirect-light policy. Exactly one policy is active per map and producer:

| Policy | World reads | Dynamic objects read |
| --- | --- | --- |
| `Baked` | the lightmap's total layer | the baked volume |
| `BakedPlusDelta` | total + the producer's delta | the baked volume + delta |
| `RuntimeIndirect` | direct layer + the producer's indirect | the runtime volume |

`RuntimeIndirect` requires an LMAP v2 with direct and indirect layers. The
policy is part of the world shader's variant key:

- a variant that adds producer indirect reads the direct layer;
- one that reads the total layer adds none.

`WorldDiffuseLight` is the composition every renderer implements.

The suite checks the table, availability, and the furnace fixture's
composition (ρ 0.6, Le 0.3: total 0.75 = direct 0.3 + indirect 0.45). Every
policy with the baked producer reproduces 0.75, and a producer that doubles
its indirect is detected. The sensitivity build reads the total layer under
the `RuntimeIndirect` variant (the double count); the furnace oracle must
reject it.

The native Vulkan world path adds a GPU check (`render.world-pbr.native-pixels`):

- `world_pbr.frag -DRUNTIME_INDIRECT` reads the direct layer plus the
  producer's indirect atlas (at G2, the baked producer's LMAP indirect layer);
- on a furnace-valued lightmap it matches the `Baked` variant, and the seeded
  double count differs.
