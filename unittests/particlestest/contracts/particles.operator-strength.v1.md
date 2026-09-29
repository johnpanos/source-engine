# particles.operator-strength.v1

`public/particles/particle_operator_strength.h` decides an operator's strength
in one particle system instance. `CParticleCollection::CheckIfOperatorShouldRun`
applies it to every emitter, initializer, operator, constraint, force and
renderer; an operator runs when its strength is above 0, and emitters and
operators scale their work by it.

Obligations, as Portal 2's retail `CheckIfOperatorShouldRun` (Mac
`client.dylib` build 841, decompiled) and CS:GO's `particles.cpp`:

- End cap state -1 runs always, 0 only while the system plays, 1 only in its
  end cap.
- A nonzero "operator time offset seed" shifts the operator's clock by a draw
  in [min, max] from the instance's random stream at that sample id, clamped
  at 0.
- A nonzero "operator time scale seed" runs the clock past the fade-in start
  at 1 / max( 0.0001, draw in [min, max] ).
- A fade oscillation period replaces the clock with the phase of the
  unmodulated clock.
- The fade window then gives the strength, and a nonzero "operator strength
  scale seed" multiplies it: strength *= max( 0, strength * draw ). The fade
  strength is squared; that is the retail arithmetic.
- The draws come only from the instance's own stream (`CParticleCollection::
  RandomFloat( int, float, float )`: the shared constant table at the
  instance's seed plus the sample id). The result depends on nothing else, so
  serial and pooled simulation (`r_particle_job_graph` 1 and 2) agree, and
  instances of one system no longer run their operators in lockstep.
- With the seeds at their default 0 nothing is drawn and the result has the
  exact bits of the fade-only rule this replaces, whatever the ranges hold.

The unpack list reads "operator time strength random scale max" and then
"operator strength random scale max" into the same field, as CS:GO does; the
second wins, and retail content always sets them equal.

Not in the rule: CS:GO's `m_bStrengthFastPath` shortcut (strength 1 for an
operator with every modulation field at its default, even before time 0) and
`ShouldRun( bApplyingParentKillList )`.

The sensitivity build replaces the rule with the fade-only one; the
hand-computed cases and the lockstep check must reject it.
