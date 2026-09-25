# client.particle-plane-crossing.v1

`game/client/particle_plane_crossing.h` decides when a particle's step hits
one of the collision planes that `CParticleCollision` (the old-style spark,
debris and trail effects) finds around an effect's origin.

Obligations:

- A step from in front of the plane (beyond the epsilon) to on or behind it
  hits, at the step's fraction where it meets the plane.
- A step from on the plane (within the epsilon) to behind it hits. The
  network quantizes effect origins (TE origins to 1/32 unit, or to whole
  units in multiplayer), so an effect placed just off a surface can start
  exactly on it. Before this rule its particles fell through the surface.
- A particle behind the plane never hits it. A particle leaving the plane,
  or sliding within the epsilon band, does not hit it.

The client also re-probes a plane from one unit back when the first probe
starts in solid, and keeps the coarse plane when the exact retest starts in
solid. Those parts need engine traces and are checked in game (an
`env_spark` on a floor keeps its sparks, and their light, above the floor).

The sensitivity build restores the original predicate, which misses a
particle starting on the plane; the oracle must reject it.
