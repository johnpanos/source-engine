# render.spark-light.v1

`public/render/spark_light.h` owns the light a burst of sparks emits, part
of the RFC 0011 runtime light set. It covers both kinds of spark:

- old-style trail sparks (`game/client/fx_sparks.cpp`);
- particle systems (PCF) drawn with a spark material
  (`game/client/particles_new.cpp`).

The client carries each lit burst as one dlight
(`game/client/particle_light.cpp`), which the engine publishes in the
frame's light set (`engine/light_set_publisher.cpp`, `render.light-set.v1`).
The native world path lights with it as an unbaked point light (ranked by
`render.direct-light-selection.v1`), models through the engine's dynamic
model lighting, and the legacy path through its dlight lightmaps.

Obligations of the policy:

- **Ramp.** A trail spark's drawn brightness ramp is `CTrailParticles`'
  original expression, bit for bit, with and without fade-in and fade-out.
- **Emission.** A trail spark emits its ramp times its remaining life
  fraction (the share of its trail still drawn). A dead spark, or one with
  no life, emits nothing. A system particle emits its alpha, clamped to
  0..1, times its tint's brightest channel.
- **Spark materials.** A system draws sparks when a collection's material
  name contains "spark", in any case and with either slash. That covers
  every Portal 2 spark material and `effects/spark`, and nothing else.
- **One light per burst.** The light sits at the emission-weighted centroid
  of the live sparks, in their emission-weighted tint (brightest channel 1).
  A spark that emits nothing does not move it.
- **Reach.** When a burst has a reach, only sparks within it of the burst's
  source count. Trail sparks use the effect origin; systems use control
  point 0. The reach is the light's radius. A spark that has flown off, or
  fallen out of the world, neither moves nor lights the burst.
- **Fade.** The light's strength is the frame's total emission over the
  highest total the burst has reached. A system burst uses at least a full
  burst's emission (eight fully drawn sparks) instead. A burst starts at
  full strength, fades as its sparks fade, and is dark once no spark emits.
- **Batches.** A frame's sparks may arrive in several batches (one per
  material). The light read after a batch covers every spark added that
  frame so far.
- **Budget.** At most `fx_spark_lights` bursts hold a light at once. Each
  release returns one place, never more, and a lowered budget applies to
  new bursts. The client also never takes a dlight slot another light holds.

Data flow for particle systems (the particle job graph):

1. Each effect's simulate item, on a pool worker, gathers its light after it
   simulates. The gather reads only that effect's collections and writes
   only that effect's burst.
2. The ordered host pass after the batch (`CParticleMgr::UpdateNewEffectsEnd`,
   in the render-start graph's `ParticlesCommit` node) commits every effect's
   light in effect order. An effect that did not gather this frame, because
   it is asleep or was not simulated, releases its light.
3. The render-start graph declares the dlight table as
   `RS_DOMAIN_DYNAMIC_LIGHTS`. Only the particle gather and commit host
   nodes write it; the batch items never touch it.

The suite checks the following:

- the ramp against a copy of the original expression;
- system emission, spark-material recognition, weighted color, the
  full-burst floor and the reach;
- scripted bursts, including a burst of falling sparks;
- the budget.

The sensitivity builds replace the policy with four defects:

- a count-weighted (unweighted) centroid;
- a light that never fades;
- a budget that leaks on release;
- a light that follows sparks beyond its reach.

The oracle must reject each.
