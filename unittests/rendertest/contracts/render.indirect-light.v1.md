# render.indirect-light.v1 and render.indirect-switching

`public/render/indirect_light.h` owns RFC 0011's producer contract: the
`IProducer` interface, `ProducerCaps`, `IndirectError`, published volumes and
retire tickets. Every baked, radiosity, SDF or ray-query producer, and the
contract's scripted fake, passes one shared suite
(`unittests/rendertest/test_indirect_light.cpp`). Its obligations:

- `Begin` validates the scene (a probe volume; the policy the producer claims)
  and fails without leaving resources behind.
- Publications are complete, keep the scene's grid topology and carry
  strictly increasing epochs.
- The first publication is not darker than the seed beyond the producer's
  seed tolerance.
- Publications keep the seed's visibility: the contract seed's dark side stays
  dark.
- Total minus indirect stays the seed's direct light under every policy (no
  double count).
- A claimed response converges to the scenario's steady state within
  `convergenceFrames`, and no response is claimed without it.
- Resources release behind the completion serial `End`'s ticket names, and
  are gone once it completes.
- `Schedule` returns without waiting for a stalled GPU (5 ms bound).

The seed is `quality/fixtures/gi/prbv/contract.prbv`: the leak fixture's wall
geometry, lit with the furnace's light (total 0.75, indirect 0.45) on one side.
The suite passes the Baked producer and the scripted fake. It rejects each
bad producer for its defect:

- one that publishes before seeding;
- one that ignores visibility;
- one that frees a volume a pending frame reads;
- one that reuses an epoch;
- one that claims `GeometryMotion` without responding;
- one that double-counts under `RuntimeIndirect`;
- one that blocks on the GPU in `Schedule`.

`public/render/indirect_switcher.h` owns runtime switching. The same program
runs its scenarios against a fake GPU timeline:

- baked → fake and back, with no frame's mean below 95% of the seed's, the
  declared fade, and peak residency of at most three volumes;
- ended producers destroyed only after their tickets, with every resource
  released;
- a failed `Begin` keeping the old producer and changing nothing;
- an unoffered saved producer reported, with Baked kept;
- device loss mid-fade republishing the baked volume at once, and recovery
  restoring the selection;
- backgrounding scheduling no work, and resume continuing;
- a map change retiring every producer behind its ticket.

The native switching evidence (real frames, real completion serials) is
`render.indirect-switching.native-pixels`.
