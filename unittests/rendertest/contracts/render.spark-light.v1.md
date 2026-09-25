# render.spark-light.v1

`public/render/spark_light.h` owns the light a burst of trail sparks emits,
part of the RFC 0011 runtime light set. The client
(`game/client/fx_sparks.cpp`) carries each lit burst as one dlight, which
the engine publishes in the frame's light set
(`engine/light_set_publisher.cpp`, `render.light-set.v1`). The native world
path lights with it as an unbaked point light, models through the engine's
dynamic model lighting, and the legacy path through its dlight lightmaps.

Obligations of the policy:

- **Ramp.** A spark's drawn brightness ramp is `CTrailParticles`' original
  expression, bit for bit, with and without fade-in and fade-out.
- **Emission.** A spark emits its ramp times its remaining life fraction
  (the share of its trail still drawn). A dead spark, or one with no life,
  emits nothing.
- **One light per burst.** The light sits at the emission-weighted centroid
  of the live sparks. A spark that emits nothing does not move it.
- **Fade.** The light's strength is the frame's total emission over the
  highest total the burst has reached. A burst starts at full strength,
  fades as its sparks fade, and is dark once no spark emits.
- **Batches.** A frame's sparks may arrive in several batches (one per
  material). The light read after a batch covers every spark added that
  frame so far.
- **Budget.** At most `fx_spark_lights` bursts hold a light at once. Each
  release returns one place, never more, and a lowered budget applies to
  new bursts. The client also never takes a dlight slot another light holds.

The suite checks the ramp against a copy of the original expression, then
runs scripted bursts, including a burst of falling sparks, and the budget.
The sensitivity builds replace the policy with a count-weighted (unweighted)
centroid, a light that never fades, and a budget that leaks on release. The
oracle must reject each.
