# render.spark-light.v2

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
  of every live spark of the burst, in their emission-weighted tint
  (brightest channel 1). A spark that emits nothing does not move it. A
  burst's emitters share its light: the little sparks of `FX_ElectricSpark`
  and `FX_Sparks` add to the big sparks' burst.
- **Reach.** The light's radius is its base radius plus 1.5 times the
  sparks' spread (their emission-weighted RMS distance from the light), at
  most 2.5 times the base radius. Its reach follows the sparks wherever they
  fly or fall, and covers 90% of their emission in a scripted burst that
  lands on a floor far below its source. (v1 counted only the sparks within
  the base radius of the source, so sparks that travelled went unlit and the
  light stayed near the source.) The spread keeps its precision far from the
  world origin.
- **Fade.** The light's strength is the frame's total emission over the
  highest total the burst has reached. A system burst uses at least a full
  burst's emission (eight fully drawn sparks) instead. A burst starts at
  full strength, fades as its sparks fade, and is dark once no spark emits.
- **Batches.** A frame's sparks may arrive in several batches (one per
  material). The light read after a batch covers every spark added that
  frame so far.
- **Selection.** Every frame, at most `fx_spark_lights` bursts hold a light:
  the most important at the main view. A burst's importance is its
  strength (its brightest linear channel) times `r^2 / (r^2 + d^2)`, with
  `r` its radius and `d` its distance from the view. A burst lit last frame
  counts 1.25 times, so two bursts of about the same importance do not trade
  the light. A burst of no strength is never lit; ties go to the burst that
  asked first. (v1 gave the budget to the first bursts to ask.) The client
  also never takes a dlight slot another light holds.

Data flow:

1. Trail sparks add to their burst while their emitters simulate on the
   host; the first emitter of the burst to add in a frame starts the
   burst's frame. Each emitter commits the burst's light after its batch.
2. Each particle system's simulate item, on a pool worker, gathers its light
   after it simulates. The gather reads only that effect's collections and
   writes only that effect's burst.
3. The ordered host pass after the batch (`CParticleMgr::UpdateNewEffectsEnd`,
   in the render-start graph's `ParticlesCommit` node) commits every system's
   light in effect order. An effect that did not gather this frame, because
   it is asleep or was not simulated, commits a dark light.
4. `SparkLights_Resolve`, at the end of the particle update
   (`CParticleMgr::UpdateAllEffectsEnd`, the same node), selects the lit
   bursts and writes the frame's dlights. The render-start graph declares the
   dlight table as `RS_DOMAIN_DYNAMIC_LIGHTS`; only the particle gather and
   commit host nodes write it, and the batch items never touch it.

The suite checks the following:

- the ramp against a copy of the original expression;
- system emission, spark-material recognition, weighted color, the
  full-burst floor, the spread and the radius;
- scripted bursts, including a burst of sparks that fall to a floor far
  below their source and slide outward;
- the selection and its hysteresis.

The sensitivity builds replace the policy with six defects:

- a count-weighted (unweighted) centroid;
- a light that never fades;
- v1's source reach;
- a radius that ignores the spread;
- v1's first-come budget;
- a selection without hysteresis.

The oracle must reject each.

In-engine fixtures (`tools/quality/spark_light_scene.py`, map `spark_lab`)
check the same obligations on the Portal product: coverage and liveness of
every burst, the little sparks carried by a light, and the selection among
more bursts than the budget.
