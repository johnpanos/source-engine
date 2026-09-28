# Contract: `render.graph.v1`

Module: `render.graph` (RFC 0016 layer 2)
Headers: `public/render/graph/` (`graph_builder.h`, `compiled_graph.h`,
`executor.h`, `trace.h`, `validate.h`)
Suites: `unittests/rendertest/core/graph/test_graph.cpp` (on `render.device.null`;
also under TSan), and the bad graphs in `test_graph_negative.cpp`
Rows: R87 (RFC 0016 K2)

| Clause | Obligation |
| --- | --- |
| G1 | A pass is kept if it has a side effect or writes a resource a kept pass accesses or that is imported; others are culled and their transients never created |
| G2 | Transitions: an access whose usage differs from its state owner's current usage gets one, and so does a write in the same usage as an earlier pass's write; imported resources end in their final usage. The state owner is the import, or a transient's physical resource. Transients of one shape (usages aside) whose lifetimes over the kept passes do not overlap share one physical resource, created with the union of its members' usages; memory aliasing across shapes is the unclaimed `kTransientAliasing` capability |
| G3 | Reading a transient before any pass wrote it fails (`kReadBeforeWrite`) |
| G4 | One pass using one resource in two usages fails (`kConflictingAccess`) |
| G5 | A write usage declared as a read (or the reverse) fails (`kUsageKindMismatch`); a kept pass without an execute function fails (`kNoExecute`) |
| G6 | The serial executor records kept passes in order with their transitions, submits once, and releases transients behind the submission token |
| G7 | On 1,000 seeded random graphs the compiler agrees with an independent reference model in the suite (fixpoint culling, first-fit slots, transitions per state owner), sharing no code with the compiler, on kept passes, transitions and alias sets; at least 10% of the graphs share a physical transient |
| G8 | `ValidateCompiledGraph` reports nothing for every compiled graph, and the null device, which validates every transition against its own state, accepts every execution. Bad graphs, each seeded into every applicable random graph, are reported with their kind every time: a missing transition (write after write included), overlapping live aliases, a culled side-effect pass, two dependent passes reordered, and a read whose only writer was removed |
| G9 | The pooled executor records each kept pass into its own encoder as a `jobs.graph` job (an injected `jobsystem::IGraphExecutor`), submits them with a final encoder in pass order, and the device's recorded stream equals the serial executor's |
| G10 | A `TransientPool` keeps physical transients between executions: a later execution of the same graph creates none, a pooled resource continues from the usage its last execution left it in, an entry unused for the pool's idle count of executions is released behind its token, and destroying the pool releases the rest |

The Vulkan lane (`test_graph_vulkan.cpp`, `render.graph.v1.vulkan`) runs the
random graphs with real work for their writes on render.device.vulkan,
serially and pooled, under the validation layer.

Open for K2: pass merging (an
adapter-private render-pass merge), and the product passes (present, gamma,
MSAA resolve, scene capture, compute) on the graph with sync validation.
