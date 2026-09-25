# RFC 0003: Dependency-Aware Job System and Frame Scheduling

- Status: Proposed
- Date: 2026-09-21
- Scope: Engine frame orchestration, CPU job scheduling, and incremental subsystem migration
- Depends on: [RFC 0001: Capability-Based Platform Architecture](0001-capability-based-platform-architecture.md), especially its task-runner, ownership, and render-threading contracts
- Verification: [RFC 0005: Quality and Correctness Harnesses](0005-quality-and-correctness-harnesses.md)
- Language and synchronization: [RFC 0006: C++20, Ownership, and Synchronization](0006-modern-cpp-ownership-and-synchronization.md)
- Evidence: Static inspection of this repository at proposal time. Later scheduler microbenchmarks, pool budgets and in-game frame measurements are in the progress records; no frame, latency or mobile budget is set
- Implementation status: Partial; no phase gate is complete. The graph runtime, executors, engine-pool bridge, serial host frame graph, declared frame-graph regions and several cohort migrations are installed. State and evidence: [0003-progress.md](0003-progress.md) and the records it links

## Summary

Source should execute suitable subsystem work through a dependency-aware job
scheduler. Long-lived subsystems retain ownership of their state and publish
bounded work with explicit inputs, outputs, prerequisites, and execution
constraints. The application host composes that work into simulation-tick and
render-frame graphs.

The primary migration pattern is:

```text
gather and establish ownership
             |
   independent computation jobs
             |
 ordered publication or commit
```

This pattern already exists in particle simulation, query-cache maintenance,
and entity packing. The proposal extends those seams into a coherent execution
model while preserving the ordering of legacy callbacks until each callback's
dependencies and side effects have been audited.

The scheduler builds on the execution contracts in RFC 0001: independent task
runners, ordered sequences, and runners with physical-thread affinity where an
API requires it. A portable scheduler runtime owns compute workers; the host
owns frame policy. Features receive only the execution and data capabilities
they require.

The initial delivery introduces graph construction, validation, completion
tracking, tracing, and a deterministic executor. Existing parallel kernels are
the first production migrations. Entity simulation, AI decision application,
physics integration, and network snapshot sending require additional ownership
work before broader concurrency is enabled.

## Motivation

The current engine has useful parallel work but coordinates most of it through
local fork/join calls. Each caller finishes one batch before advancing to the
next phase, limiting opportunities to overlap independent work across systems.
Other work uses dedicated pools or coarse jobs with manually placed waits.

Adding workers alone does not expose hidden dependencies. Entity callbacks,
model caches, shared frame state, and lifecycle operations still assume
particular ordering. Some operations also have physical-thread requirements.
The proposed graph makes these restrictions visible so independent work can
proceed without relying on accidental call order.

This is a proposal for this Source tree using dependency graphs and bounded
computation jobs. It does not claim to reproduce proprietary id Tech 6 or 7
scheduler internals. The repository evidence below is the basis for the
migration decisions.

## Goals

- Expose dependencies between computational phases and overlap independent work.
- Reuse existing parallel kernels and preserve their correctness boundaries.
- Separate computation from entity mutation and resource publication.
- Preserve tick ordering, prediction behavior, and observable gameplay during each migration.
- Support client, listen-server, dedicated-server, tool, and test compositions.
- Provide explicit sequence and physical-thread affinity.
- Coordinate compute capacity across migrated services without starving device or blocking-I/O work.
- Make job lifetime, cancellation, completion, and shutdown behavior testable.
- Measure critical-path time, scheduling overhead, contention, and tail latency.
- Permit incremental adoption with a serial reference path and subsystem rollback.

## Non-goals

- Rewriting all entities or converting the engine to an ECS.
- Making arbitrary existing virtual callbacks safe to execute concurrently.
- Changing gameplay, network protocol, simulation tick rate, or demo format.
- Adding a frame of input or prediction latency as a default scheduling technique.
- Replacing VPhysics or introducing a GPU render graph.
- Moving every asynchronous facility into one physical thread pool.
- Requiring fibers or coroutines, or upgrading unrelated legacy targets as a prerequisite.
- Extending the legacy module ABI with new C++ graph interfaces.
- Guaranteeing a particular speedup before representative profiling.

## Current implementation and constraints

### Job and thread-pool facilities

[`public/vstdlib/jobthread.h`](../public/vstdlib/jobthread.h) defines `CJob`,
`IThreadPool`, `CJobSet`, and `CParallelProcessor`. Jobs have priorities, flags,
status, completion events, and reference-counted lifetime. There is no public
dependency-edge or continuation graph contract.

`CParallelProcessor::Run` queues worker calls, runs work on the caller, and
finishes the batch before returning. Workers claim elements through a shared
atomic pointer. This is dynamic distribution within a batch, distinct from
stealing ready tasks between worker-local queues.

[`vstdlib/jobthread.cpp`](../vstdlib/jobthread.cpp) contains several policies
that the new runtime must account for:

- `CGlobalThreadPool::Start` sets a four-worker cap unless `-threads` overrides it.
- The default PC compute-pool path in `CThreadPool::Start` separately limits the count to three workers.
- `CThreadPool::AddJob` can execute ordinary compute work inline if no worker is idle; `JF_QUEUE` and `JF_IO` alter that behavior.
- `YieldWait` executes shared queued work, then waits on completion events when the queue is empty. Its comment acknowledges that newly spawned worker jobs do not wake that waiting caller to help.
- Workers use direct queues and a shared priority queue; this implementation does not provide general worker-local work stealing.

These are observations about the checked-in implementation, not measurements
of the effective configuration in a particular running game. The R20
scheduler-trust increment changed the last two: a wait now runs only the jobs
it waits for (when eligible), never unrelated queued work, and workers own
bounded steal deques. See
[the scheduler trust record](0003-scheduler-trust-progress.md).

### Host scheduling

[`engine/host.cpp`](../engine/host.cpp) contains the serial frame loop and a
threaded listen-server path. The latter uses previous-frame client timing,
submits `_Host_RunFrame_Server_Async` as one job, proceeds through rendering,
sound, and client update, and waits before executing server-deferred work.
`host_thread_mode` defaults to zero on PC in this tree. Since the R10
scheduler-trust increment, the frame after admission runs as an ordered
serial graph by default (`host_frame_graph 1`); the hand-ordered body is the
rollback.

The single-player wait contains a warning about executing queued AI operations
at the wrong point while helping the pool. This is evidence that a wait is also
an execution-policy boundary: helping arbitrary work can violate assumptions
even when every individual job is internally thread-safe.

### Existing subsystem seams

| Area | Evidence | Implication |
| --- | --- | --- |
| Particles | `CParticleMgr::UpdateNewEffects` in [`particlemgr.cpp`](../game/client/particlemgr.cpp) runs entity-dependent `Update`, parallel `ProcessPSystem`, then serial `DetectChanges` | Use as the first gather/compute/commit reference migration |
| Bones | `C_BaseAnimating::ThreadedBoneSetup` in [`c_baseanimating.cpp`](../game/client/c_baseanimating.cpp) and shadow/view bone batches | Preserve parent, attachment, cache, and model-lifetime dependencies when exposing graph nodes |
| Client leaf work | `CClientLeafSystem::PreRender` and `ComputeTranslucentRenderLeaf` in [`clientleafsystem.cpp`](../game/client/clientleafsystem.cpp) | Some render preparation already has audited parallel regions |
| Entity packing | `PackEntities_Normal` in [`sv_packedentities.cpp`](../engine/sv_packedentities.cpp), controlled by `sv_parallel_packentities` | Preserve snapshot ownership and per-entity output separation |
| Query cache | `UpdateQueryCache` in [`querycache.cpp`](../game/shared/querycache.cpp) splits hash-chain work, then merges obsolete entries | Existing independent outputs and serial merge are useful boundaries |
| Navigation | `CPostFrameNavigationHook` in [`ai_basenpc.cpp`](../game/server/ai_basenpc.cpp) queues navigation queries | Preserve the intended post-frame window and entity lifetime |
| Navigation visibility | Parallel visibility calculation in [`nav_area.cpp`](../game/server/nav_area.cpp) | Useful batch example; do not assume this is a frequent gameplay-frame workload |
| Render submission | Queued contexts and `m_pMatQueueThreadPool` in [`cmaterialsystem.cpp`](../materialsystem/cmaterialsystem.cpp) | Adapt existing ordered execution before changing backend threading |
| Asset I/O | `CFileAsyncJob` and a separate pool in [`filesystem_async.cpp`](../filesystem/filesystem_async.cpp) | Retain an appropriate blocking-I/O executor; bridge completion into compute dependencies |
| Save work | Async save pool in [`host_saverestore.cpp`](../engine/host_saverestore.cpp) | Separate state capture from background persistence |
| Audio | `S_Update_Thread` and `S_Update_` in [`snd_dma.cpp`](../engine/audio/snd_dma.cpp) | Mixing has cadence requirements; the dedicated path here is gated by console/configuration checks |

### Mutable simulation and ordered callbacks

`CServerGameDLL::GameFrame` in
[`gameinterface.cpp`](../game/server/gameinterface.cpp) explicitly orders game
systems, navigation/bots, query-cache work, entity thinking, post-think systems,
event dispatch, deletion cleanup, and client-data updates.

`Physics_RunThinkFunctions` in
[`physics_main.cpp`](../game/server/physics_main.cpp) copies the simulation
list, resets `gpGlobals->curtime` while iterating, and calls entity simulation.
Immediate deletion is disabled during the loop. `Physics_SimulateEntity` also
uses model-cache scopes and prediction event suppression. Distinct entity
pointers do not establish independent write ownership.

On the client, `SimulateEntities` in
[`cdll_client_int.cpp`](../game/client/cdll_client_int.cpp) invokes arbitrary
entity callbacks sequentially. `OnRenderStart` orders interpolation, camera
work, animation, data-change events, client simulation, physics, and bone work.
`UpdateClientSideAnimations` is also a serial callback loop.

[`igamesystem.h`](../game/shared/igamesystem.h) specifies registration-order
updates. [`igamesystem.cpp`](../game/shared/igamesystem.cpp) invokes callbacks
sequentially under model-cache critical-section scopes. The current interface
does not describe runtime data dependencies.

### A documented concurrency failure

The comment beside `sv_parallel_sendsnapshot` in
[`sv_main.cpp`](../engine/sv_main.cpp) records crashes involving concurrent
access to `g_FrameSnapshotManager.m_FrameSnapshots`. That option defaults to
disabled. The parallel path also excludes HLTV and replay because they modify
global state.

This establishes a concrete migration requirement: per-client work must use
stable snapshot inputs and independently owned outputs, and shared snapshot
bookkeeping must have an explicit owner. Enabling the existing option is not an
acceptance criterion for this RFC.

## Terminology

| Term | Meaning |
| --- | --- |
| Job | A bounded unit of computation with declared inputs, outputs, and execution requirements |
| Dependency | A completion and memory-publication edge from a producer to a consumer |
| Graph | A directed acyclic set of jobs and dependencies |
| Sequence | Ordered, non-overlapping execution; successive jobs may use different physical threads |
| Affine runner | Execution on a specified physical thread required by a provider contract |
| Scope | Lifetime and completion owner for related jobs and their storage |
| Snapshot | A stable view whose referenced data remains valid and unchanged for its consumers |
| Commit | Ordered application of computed results or mutations to authoritative state |
| Epoch | A generation identifying a tick, frame, level, or resource lifetime |
| Legacy phase | An opaque callback region whose existing order and execution context remain enforced |

## Architectural placement

RFC 0001 defines the execution vocabulary. This RFC adds dependencies and
structured completion to that vocabulary.

The scheduler runtime is portable infrastructure built over foundation
atomics, synchronization, thread, clock, and memory primitives. Native worker
creation and wake mechanisms stay in the permitted backend boundary. The host
chooses worker budgets, executor bindings, and frame policy.

Subsystems receive an `ITaskRunner`, `ISequencedTaskRunner`, or
`ISingleThreadTaskRunner` when that is sufficient. A subsystem contributing
frame jobs receives a scoped graph-building interface. Only infrastructure
that must construct or manage scopes receives the full scheduler service.
There MUST NOT be a new globally discoverable capability bag or scheduler
singleton for migrated consumers.

The long-lived service dependency graph from RFC 0001 and the per-tick/frame
work graphs serve different purposes. Construction dependencies do not prove
that two update functions conflict, and separate service instances do not prove
that their updates can run together.

### Layer responsibilities

| Layer | Owns | Must not own |
| --- | --- | --- |
| Foundation/backend | Physical thread and synchronization mechanisms | Gameplay frame policy |
| Scheduler runtime | Ready queues, job state, completion publication, worker wakeup, executor routing | Entity ordering or render semantics |
| Host frame coordinator | Tick/frame graphs, legacy phase boundaries, admission and latency policy | Hidden access to a subsystem's private mutable state |
| Subsystem | State, job inputs/outputs, resource declarations, merge rules | Independent compute-worker pools without a justified infrastructure requirement |
| Device or blocking service | Provider-specific submission or blocking execution | Arbitrary synchronous waits on the frame's compute workers |

## Job and graph contracts

### Definition and submission

Every job MUST declare a stable diagnostic name, a scope, an executor, its
prerequisites, and any shared resource access relevant to scheduling. Data
ownership and lifetime must be established before the job becomes runnable.

A graph is built privately, validated, sealed, and submitted. No root job may
start while edges or resource declarations are still being added. Submission
failure must leave the graph unexecuted and return a structured error.

The initial implementation SHOULD use reusable graph templates with per-frame
bindings and bounded batch expansion. Graph topology follows actual host modes:
dedicated server, remote client, and listen server need different graphs.

The following shape is illustrative, not a committed ABI or compilable API:

```cpp
struct JobDesc_t
{
    JobName_t name;
    ExecutorToken_t executor;
    JobPriority_t priority;
    JobFunction_t function;
    JobPayload_t payload;
};

abstract_class IJobGraphBuilder
{
public:
    virtual JobHandle_t AddJob( const JobDesc_t &desc ) = 0;
    virtual void AddDependency( JobHandle_t producer, JobHandle_t consumer ) = 0;
    virtual void Read( JobHandle_t job, ResourceVersion_t resource ) = 0;
    virtual void Write( JobHandle_t job, ResourceVersion_t resource ) = 0;
    virtual Expected<SealedGraph_t, GraphError_t> Seal() = 0;
};

abstract_class IJobScheduler
{
public:
    virtual Expected<JobScope_t, JobError_t> Submit( SealedGraph_t graph ) = 0;
};
```

Final signatures must define ownership transfer explicitly and follow the
validated C++20 target policy in RFC 0006. The `Expected` convention comes from
RFC 0001 and uses a project type on C++20. Preserve the established dialect of
legacy bridge headers until their callers migrate. Public extension boundaries
use the size-versioned C contracts required there; these internal C++ sketches
do not extend `CreateInterface`.

### Dependencies and publication

When a prerequisite reaches a terminal state, the scheduler updates its
dependents exactly once. A successful producer's output writes MUST happen
before a consumer reads them. Dependency completion requires acquire/release
publication or an equivalent synchronization guarantee.

Only jobs with satisfied prerequisites enter a ready queue. An unfinished job
must never consume a worker merely to wait for another job. Fan-in nodes and
continuations represent joins.

CPU completion and GPU completion are separate events. Finishing a render
submission job does not permit recycling a buffer still in use by the GPU.
The render service retains resources until its own fence or completion
contract permits release.

### Resource declarations

The first validator checks explicit dependencies against declared resource
conflicts. Two unordered jobs may read the same immutable version. Overlapping
writes, or a read concurrent with a write to the same version, require explicit
ordering or a declared disjoint partition.

The initial validator MUST reject ambiguous conflicts instead of inventing an
order from job registration. Resource identities include the relevant epoch
and, where needed, an element range or partition. Aliasing declarations must
map overlapping views to the same underlying ownership domain.

Declarations are review and diagnostic tools, not automatic detection of all
C++ memory accesses. Each migration includes an audit of transitive calls,
static state, caches, allocators, random generators, and callbacks. A legacy
callback is treated as an opaque phase with conservative conflicts until that
audit is complete. Making a pointer `const` does not prove its caches or its
pointees are immutable.

### Sequences and physical affinity

Ordered work uses the sequenced runner from RFC 0001. An execution lane in a
frame diagram is a binding to that contract; it does not necessarily allocate
a dedicated thread.

For the first graph API, a sequence submission position is reserved when the
graph is admitted. Jobs on that sequence have a defined order, even if later
jobs become dependency-ready first. These implicit ordering edges participate
in cycle validation. Multi-producer gameplay ordering must use explicit stable
order keys or dependencies rather than a race between producers.

A physical-thread runner is reserved for a documented requirement such as
window events or an existing render backend's device ownership. The render
provider may eventually permit independent command recording on workers; the
scheduler must follow that provider's capabilities.

Main-thread participation executes only jobs eligible for that thread and for
the currently open phase. It must not drain unrelated legacy or future-phase
work during a wait. Main-thread-affine jobs require explicit pump points in the
host graph; waiting on a scope that needs an unpumped affine executor is an
error detectable in diagnostics.

### Dynamic work

Independent posting through task runners remains supported. Dynamic child
graphs inside a frame scope are a later extension, after sealed graphs are
stable.

That extension must reserve child completion ownership before publishing any
child work. Scope completion includes all admitted children. Once closed to
new producers, a scope cannot acquire additional children. Children may depend
on already completed work or other admitted child nodes; they must not depend
on the completion of the parent scope that is waiting for them.

No API may implement nested parallelism by blocking every worker on child jobs.
If a calculation needs child results, it splits into a producer and a
continuation. Fibers are an optional later implementation choice, not part of
the initial contract.

### Blocking operations and external completion

Blocking filesystem, module, and other potentially long-running operations
target an executor that declares blocking behavior. The compute pool must not
absorb arbitrary disk waits by adding unbounded replacement workers.

External I/O completions can satisfy dependency tokens through a supported
completion adapter. Registration, completion-before-registration, cancellation,
and repeated notification races must have defined exactly-once behavior.
External callbacks publish results and schedule continuations; they do not
directly enter entity or renderer mutation code from an arbitrary thread.

### Job state, errors, and cancellation

The observable lifecycle is created, admitted, ready, running, then succeeded,
failed, or canceled. Waiting-for-prerequisites is an admitted state, not a
blocking worker operation. A job reaches one terminal state exactly once.

Expected failures are explicit results. A dependent requiring valid producer
output does not run after that producer fails or is canceled. Cleanup and
error-reporting continuations may opt into terminal-state dependencies, with
no right to read an absent output as valid data.

Cancellation is cooperative. Running code is not forcibly terminated.
Canceling pending work does not release its payload or referenced state before
the scheduler acknowledges terminal completion. Partial outputs remain private
and are never committed as complete results.

Required gameplay work is not silently dropped on a deadline. Optional visual
or background work may be canceled only when the subsystem defines its visible
fallback. A required frame failure uses the host's defined error path after
quiescing the affected scope; it must not leave half-applied state.

## Tick and render-frame composition

Simulation ticks and render frames are separate scheduling scopes. One render
frame may contain zero, one, or several simulation ticks. Ticks that mutate the
same authoritative world remain ordered.

Initially, the coordinator represents the current call order as explicit
legacy phases and replaces only audited regions. It MUST preserve the existing
host mode's packet application, input sampling, prediction, and publication
points. Changing `host_thread_mode` latency behavior is a separately measured
decision, not a prerequisite for this RFC.

An illustrative server-tick migration is:

```text
apply commands and pending authoritative changes
                      |
          pre-think legacy systems
                      |
       capture stable query inputs
                      |
       parallel sensing/query batches
                      |
       consume results in legacy order
                      |
       entity simulation / physics phases
                      |
       ordered events and entity cleanup
                      |
       capture state for network packing
                      |
       parallel packing -> ordered publication
```

This diagram does not authorize moving existing AI queries before or after
their current phase. A system whose queries require partially updated world
state needs narrower snapshots or remains in its original serial phase.

For render preparation, interpolation, prediction, entity updates, and camera
state first establish stable inputs. Independent branches can then process
audited animation batches, particle simulation, visibility, and render-list
work. The graph records cross-branch dependencies explicitly: for example, a
particle control point attached to a bone must wait for that bone's producer,
and visibility that uses newly calculated bounds must wait for those bounds.

Command preparation joins the required outputs and dispatches to the render
provider's permitted submission runner. Unrelated streaming work may continue
across the frame boundary in a separate scope with retained resource ownership.

### Latency and work spanning frames

The first migration adds no new frame delay to input, player movement,
prediction, collision results required by gameplay, or packet application.
Scheduling throughput is not sufficient justification for moving those results
to the next frame.

Results that may intentionally arrive later include certain AI sensing caches,
nonessential visual effects, asset processing, or visibility hints. Each such
consumer must define maximum age, epoch validation, fallback behavior, and
whether using stale results can change gameplay. Cross-frame behavior is
reviewed and measured per subsystem.

## Data ownership and deterministic commit

### Snapshot and output storage

A snapshot may be a copied compact structure, a pinned immutable resource, or
a versioned view whose producer cannot modify it until readers finish. It need
not be a copy of the entire world. The contract must describe transitive
ownership, including models, entity handles, and spatial-query structures.

Each job owns its output range or private result buffer. Shared append buffers
require a reviewed concurrent container and a deterministic merge if their
order is observable. Copy cost, cache pressure, and retained-memory cost are
part of the performance review.

Entity handles carry generation validation, but validation alone does not keep
an entity alive during a read. Jobs either read captured values or hold a
scope-based lifetime pin while mutation/deletion is excluded. Model and
material references likewise remain valid through the relevant consumer scope.

### Entity command buffers

Migrated simulation jobs produce commands instead of directly invoking broad
world-mutation APIs. Commands identify their target generation, operation,
payload, simulation epoch, and stable application order.

The initial order reproduces the legacy traversal and callback order. A key
may include tick, phase, legacy entity-list ordinal, and per-producer command
ordinal. It must not use worker ID, completion time, or an unrelated entity sort
that changes current behavior.

Conflicting commands need explicit semantics: sequential application,
accumulation, rejection, or a defined winner. Deletion, damage, movement,
triggers, sounds, and event dispatch cannot share an unspecified generic merge.
Invalidated targets are handled according to the command contract.

Deferred commit can itself change behavior. If entity B historically observes
entity A's mutation during the same traversal, calculating both from the old
snapshot changes the simulation. Such dependencies require ordered stages,
conflict groups, or continued serial execution. Command buffers are a mechanism
for controlled migration, not a proof of equivalence.

### Time, randomness, and numerical behavior

Jobs receive explicit tick/frame time values. Migrated computation must not
write shared `gpGlobals` to establish its local time context. Compatibility
callbacks requiring mutable globals stay in the appropriate legacy phase.

Random draws must preserve the subsystem's existing observable sequence. A new
per-entity or per-job seeded stream is permitted only as an explicit behavior
change with its own validation. Where needed, gather precomputes the legacy
random inputs for parallel consumers.

Floating-point reductions use stable grouping when results affect gameplay or
recorded state. Single-worker execution is a useful reference, but it does not
by itself guarantee bitwise agreement across compiler, architecture, or SIMD
implementations. Each subsystem declares exact or tolerance-based comparison
requirements before conversion.

## Subsystem migration matrix

| Subsystem/work | Initial action | Required boundary | Priority |
| --- | --- | --- | --- |
| Particle simulation | Expose existing compute batches as jobs | Preserve serial control-point gathering, callbacks, spatial preparation, and change publication | First pilot |
| Bone setup | Migrate existing eligible batches | Pin models; represent parent/attachment dependencies; retain cache synchronization | Early |
| Query-cache maintenance | Replace internal join with graph completion | Preserve disjoint hash chains and ordered victim-list merge | Early |
| Entity packing | Adapt current parallel work | Stable snapshot lifetime and disjoint entity output; audit encoding helpers | Early |
| Leaf and shadow preparation | Migrate existing parallel regions | Preserve cache locks and shared-tree rules until output ownership improves | Early |
| Animation evaluation | Extract numerical pose work from callbacks | Stable animation inputs, model pins, ordered transforms/events, parent dependencies | After pilots |
| Culling/render-list construction | Extract batches and merge lists | Stable camera/bounds, private lists, preserved transparency and draw-order requirements | After pilots |
| AI sensing/pathfinding | Migrate audited query windows | Stable world view, request generation, query scratch isolation, ordered action application | After pilots |
| Network send preparation | Refactor before enabling | Immutable per-tick snapshot, per-client buffers, sequence-owned shared bookkeeping; retain HLTV/replay constraints | Later |
| Client entity simulation | Keep callbacks as legacy phases; extract selected kernels | Audit every migrated entity family and transitive callback; preserve inter-entity observations | Later |
| Server entity simulation | Keep legacy order; migrate selected computations | Time/random inputs, command semantics, lifetime pins, deterministic commit | Last major stage |
| VPhysics | Initially preserve solver ordering and callbacks | Overlap a whole step only after shared services are audited; internal islands depend on solver support | Separate investigation |
| Asset loading | Bridge I/O completions; schedule decode/decompression | Blocking-I/O executor, retained buffers, sequence-owned cache insertion, render upload contract | Incremental |
| Audio | Preserve cadence-sensitive execution; assess independent decoding | Deadline/fallback policy, private buffers, channel ownership; avoid waiting in device callbacks | Separate measured pilot |
| Save/restore | Preserve capture/restore barriers; schedule independent persistence work | Immutable captured state and completion before level/module teardown | Incremental |
| UI, commands, input, lifecycle | Represent as explicit legacy/affine phases | Existing sequence and physical-thread requirements | Retain until separately audited |

Existing uses of `ParallelProcess` are evidence of an available seam, not proof
that they can overlap arbitrary neighboring phases. For example, a leaf-tree
update may be parallel internally while still requiring exclusive access
relative to visibility consumers.

## Scheduler implementation and capacity policy

The target runtime uses bounded worker-local ready queues with work stealing
for eligible compute jobs, plus an injection path for external producers.
Affinity-bound work stays on its executor. Release/acquire publication and the
sleep/wake protocol must prevent a worker from missing newly admitted work.

Work stealing is an implementation choice for the production compute executor;
the public graph contract does not depend on a particular deque algorithm.
The deterministic executor chooses a stable topological order, allowing graph
validation and subsystem testing before the parallel executor is complete.

Bounded injection/completion rings follow RFC 0006: declare producer/consumer
topology, publication and reuse ordering, capacity, overload policy, and drain.
A work-stealing deque is not interchangeable with a FIFO ring. Required work
cannot be silently dropped or overwritten. Use C++20 atomics/wait facilities
where the selected profile supports them, with an explicit happens-before
argument; notification alone is not publication. Prefer a simple synchronized
queue until a more complex implementation has correctness and performance evidence.

CPU dependency completion and GPU retirement remain distinct. Scheduler scopes
may bridge device completion tokens, but a completed submission job cannot
recycle an upload range or destroy a still-used GPU resource. Those tests belong
to Q-JOBS and Q-PRESENTATION together.

The host chooses a process-wide compute budget based on available processors,
deployment, and measurements. It must account for existing material, I/O,
audio, and legacy workers during migration. Dedicated-server deployments need
configuration for several processes sharing a machine. Removing the historic
three/four-worker policies is a measured change, not a universal instruction
to occupy every logical processor.

Jobs should cover enough work to amortize queue and tracing overhead. Batch
size is determined from measurements of duration and variability; no universal
microsecond threshold is mandated. A subsystem with only a small batch may
choose serial execution before submitting the graph.

Job priority expresses importance among runnable jobs and does not bypass
dependencies or imply an OS real-time priority. A high-priority consumer must
not starve behind lower-priority prerequisites; the implementation must provide
dependency-aware priority handling or an equivalent bounded scheduling policy.
Background admission must leave measured capacity for frame-critical work.

## Compatibility and incremental integration

### Legacy pool bridge

Existing `CJob`, `IThreadPool`, flags, event waits, and reference-count behavior
remain operational during migration. Their semantics must not silently change
to match the new graph contract.

A first bridge may submit ready, independent graph work through a dedicated
legacy pool binding using explicit queue flags. It must prevent inline
reentrancy from violating graph publication, account for completion exactly
once, and never implement graph dependencies by blocking pool jobs.

Because legacy waits can execute arbitrary queued work, mixing new graph jobs
with legacy queues requires proof that every helping caller is eligible to run
them. Until that proof exists, use a controlled executor boundary with a shared
capacity budget. An adapter that double-counts workers or relies on accidental
queue separation is not a completed migration.

### Game-system integration

Add a separate, internal frame-work contributor contract for migrated systems.
It accepts a scoped builder and declared data bindings. Existing `IGameSystem`
callbacks keep their ordered invocation behavior; their vtable is not expanded
as an incidental step in this RFC.

The host initially emits one ordered legacy node per existing phase. Migrated
contributors replace selected regions with gather, batch, and commit nodes.
Legacy callbacks remain conservative mutation barriers for the relevant world
or service domain. Registration order remains an explicit dependency where it
is still part of the contract.

Broad model-cache scopes must be audited before moving work across them. Cache
pinning, mutation exclusion, and physical mutex ownership are separate
concerns; moving a lock scope to a different worker is not automatically valid.

### Configuration and rollback

Graph orchestration, parallel execution, and each subsystem migration have
separate diagnostic switches. Exact ConVar names are an implementation detail.
This permits legacy execution, serial graph execution, and parallel graph
execution to be compared independently.

A switch takes effect only at a quiescent frame/tick boundary after affected
scopes finish. Rollback drains admitted work and restores the legacy path; it
must not run both implementations' side effects. State-comparison modes use
captured inputs and private outputs, not duplicate calls into the live world.

## Lifetime, shutdown, and memory

Tick/frame scopes own graph nodes, payload storage, and completion records.
Their memory cannot be recycled until every job, continuation, cancellation
acknowledgment, and retained external completion has relinquished its reference.
Generation-tagged handles prevent stale scope references from becoming valid
after arena reuse.

Worker scratch storage is temporary and must not escape a job. Result storage
belongs to the scope or a separately owned resource. Cross-frame jobs use their
own scope; they must not retain borrowed stack or reclaimed frame-arena data.
Capacity limits and allocation failure paths must be explicit and measured.

Level transitions and shutdown follow this sequence:

1. Close admission for the affected level or service epoch.
2. Cancel optional pending work and retain required cleanup operations.
3. Drain running work and required continuations while servicing permitted affine runners.
4. Finish or invalidate external requests without permitting late callbacks into destroyed state.
5. Release job payloads, resource pins, and graph storage.
6. Destroy consumers, then providers, in the order required by RFC 0001.

Scope ownership includes function pointers and payload destructors implemented
in modules. Those modules must stay loaded through job destruction, not merely
through function execution. Shutdown must not stop a provider or runner needed
to finish another scope. Diagnostic timeouts report the blocked dependency
chain; they do not make forced destruction safe.

## Instrumentation and validation

### Tracing

Each admitted job should expose its name, scope/epoch, executor, prerequisites,
enqueue time, ready time, start/end times, terminal result, and worker/sequence
identity. Tracing also captures queue depth, steals, wakeups, idle intervals,
commit time, blocking boundaries, graph construction cost, and lock contention
where available.

Reports distinguish dependency wait from ready-queue delay and useful execution
from contention. Critical-path analysis includes serial gather/commit phases
and device handoff. Existing VPROF/Telemetry integration should be reused where
practical, with correct per-worker registration and bounded trace overhead.

### Scheduler conformance tests

- Chains, fan-out, fan-in, independent roots, empty graphs, and exactly-once execution.
- Graph cycles, sequence-induced cycles, conflicting resource accesses, and stale handles.
- Output visibility across dependency edges under stress.
- Completion arriving before registration, during cancellation, or after admission closes.
- Worker sleep/wake races and enqueue while all workers are idle.
- Zero compute-worker and single-worker modes while respecting affine executor requirements.
- Sequence ordering, main-thread pump boundaries, and rejection of forbidden nested waits.
- Failure propagation, canceled prerequisites, cleanup continuation behavior, and payload release.
- Scope exhaustion, graph allocation failure, partial startup, and orderly shutdown.
- Cross-executor dependencies and detection of waits that cannot make progress.

Dynamic child-scope tests are required before enabling that later extension.

### Independent model and adversarial execution

Q-JOBS in RFC 0005 supplies an independent small graph/state model, bounded
schedule exploration, real-thread stress, and reproducible failure artifacts.
Do not validate the production graph implementation solely by running its own
serial executor. Model tests include rejected graphs executing zero jobs and
terminal-state dependencies never exposing failed/canceled output as valid.

Persist seeds, selected schedules, graph/resource declarations, and the first
divergent event. Test very small queue/arena capacities, wraparound/reuse, delayed
external completion, priority pressure, and all workers sleeping at enqueue.
Negative executors/queues deliberately misorder publication, duplicate completion,
or release payloads early to verify that the suite detects those defects.

Real-thread tests and supported TSan builds complement deterministic exploration;
a single-thread simulation cannot establish native memory visibility. Exercise
supported weakly ordered architectures when claiming their support. ASan/UBSan,
sequence assertions, and lifetime/resource checks run in appropriate separate
configurations. A sanitizer success does not establish legacy gameplay ordering.

### Subsystem equivalence and stress tests

Each migration defines observable outputs and compares legacy, serial-graph,
and parallel-graph execution on the same captured inputs. Use exact comparisons
for deterministic state and protocol bytes where applicable; document justified
tolerances for visual floating-point outputs.

Randomize ready-job selection, vary batch size and worker count, and insert
controlled scheduling yields to expose hidden dependencies. Use race detection
on supported builds, plus sequence, lifetime, and resource assertions where
tooling coverage is incomplete.

Representative scenarios include entity spawn/removal during gameplay,
parented animation and attachments, particle control points, model eviction,
network reconnect, save/load, level change, and shutdown while jobs or I/O are
pending. Listen-server, remote-client, and dedicated-server paths require
separate coverage. Relevant HLTV/replay/demo configurations remain in the
regression set.

### Performance acceptance

Before enabling a migration by default, record a baseline and a comparison on
the same revision/configuration, maps or captures, hardware, warmup policy, and
sample count. Record median and tail frame/tick time, critical-path duration,
CPU time, scheduling overhead, retained memory, and any latency metric relevant
to the subsystem.

Acceptance requires:

- the agreed equivalence and lifecycle tests pass;
- a workload where the subsystem is material shows a repeatable improvement;
- representative small workloads and low-core configurations stay within a regression budget agreed before measurement;
- input/prediction latency or audio deadlines do not regress where affected;
- extra memory, snapshot copying, and job-management cost stay within the recorded budget;
- traces explain the improvement through less critical-path work or better overlap.

Numerical thresholds are set per pilot before benchmarking. This RFC does not
invent engine-wide gains or accept higher CPU utilization as proof of success.

## Delivery plan

| Phase | Deliverable | Exit criteria |
| --- | --- | --- |
| A: Inventory and baseline | Catalog current queues, waits, phase boundaries, thread affinity, global mutations, and representative captures | Reviewed ownership map; reproducible baseline; first particle workload selected |
| B: Contracts and serial graph | Internal graph builder, validator, scopes, resource declarations, tracing, deterministic executor, and runner bindings | Conformance tests pass; an ordered host graph preserves baseline behavior |
| C: Compute executor and bridge | Bounded worker budget, wake protocol, ready queues, affinity routing, and legacy integration | Stress/lifecycle tests pass; no forbidden helping or worker starvation; scheduler overhead measured |
| D: Particle reference migration | Gather/compute/commit contributor with both serial and parallel paths | Equivalence, attachment/lifetime tests, measured improvement, and working rollback |
| E: Existing parallel kernels | Bones, query cache, entity packing, and audited leaf/shadow work | Each migration independently satisfies correctness and performance gates |
| F: New computation seams | Animation evaluation, render preparation, AI sensing/navigation, and streaming transforms | Stable inputs/outputs, cross-system dependencies, and per-subsystem budgets verified |
| G: Stateful systems | Snapshot-send ownership refactor and selected entity/physics integration | Legacy-order equivalence or explicitly approved behavior change; no shared-state regressions |
| H: Consolidation | Retire redundant compute queues/waits where safe; revise host overlap policy if measurements justify it | All affected callers migrated; total worker budget controlled; legacy adapters removed only when unused |

Phase A establishes captured workloads, budgets, and the supported profiles.
Phase B requires the independent model and legacy-versus-serial comparison;
Phase C adds adversarial/native concurrency and queue lifetime evidence. Every
Phase D–G migration requires three-mode comparison, first-divergence diagnostics,
its own performance/latency gate, and tested rollback at a quiescent boundary.
Tests execute comparisons against captured inputs/private outputs, never duplicate
live side effects. Phase H also proves zero consumers of the retired facility.

Runtime promotion uses the shared evidence and product gates in RFC 0005.
An integration of Box3D tasks must additionally pass nested-work, total worker
budget, callback-sequence, and shutdown tests from RFC 0004; neither scheduler's
standalone test results establish that bridge. The cross-RFC order and tracked
completion state live in [AGENTS.md](../AGENTS.md).

Phase A can proceed independently of completing RFC 0001's provider migrations.
Phase B uses its runner and ownership contracts and may initially adapt existing
queues. No phase requires rewriting the entire platform foundation first.

For every migrated region, retain a short implementation record identifying the
owner, source entry points, declared resources, supported execution modes,
captured workloads, correctness expectations, benchmark results, and remaining
legacy dependencies. A later machine-readable inventory may support
architecture checks; this RFC does not create a speculative baseline before
the scheduler API exists.

## Alternatives considered

### Increase the current worker count

Useful as a bounded experiment, but it does not create dependencies, remove
global-state conflicts, or permit independent batches to overlap. It also
changes contention and process CPU allocation.

### Put each subsystem on its own thread

This can isolate a device or a blocking service, but subsystem execution costs
vary by frame. Reserving a compute thread per system wastes idle capacity and
leaves dependencies expressed through blocking waits. Ordered sequences are
the preferred serialization mechanism.

### Dispatch existing entity callbacks in parallel under broad locks

Locks may prevent selected races while preserving most of the serial critical
path. They do not establish deterministic callback order, safe entity
lifetime, or valid reentrancy. Computation extraction and ordered commit provide
a clearer boundary for each migrated family.

### Introduce a complete ECS or new physics engine first

Either may be a separate long-term choice, but both greatly enlarge the scope.
Existing particle, animation, packing, and query kernels allow incremental
progress without replacing the entity API or solver.

### Require fibers or a third-party scheduler

Fibers can support suspension but add stack, TLS, lock-ownership, and debugging
complexity. Continuations satisfy the initial workload. An external scheduler
can be evaluated behind the contract if its compiler/platform support,
licensing, memory behavior, affinity, and lifecycle semantics fit the engine.
Selecting a library does not remove the subsystem ownership work.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| Hidden mutation behind nominally read-only APIs | Audit transitive calls; pin resources; conservative legacy barriers; stress and race testing |
| Deferred writes alter gameplay order | Preserve legacy ordering; stage dependent entities; compare captured outputs |
| More workers increase contention | Trace cache locks and queue delay; measure multiple worker budgets |
| Graph overhead dominates small workloads | Reusable templates, batching, bounded allocation, and measured serial fast paths |
| Mixed executors deadlock | No worker blocking on dependencies; explicit pump points; sequence-edge validation and shutdown drain tests |
| Frame storage or module code is freed early | Scope ownership, epoch handles, cancellation acknowledgment, and provider/module drain barriers |
| Device work misses deadlines | Preserve provider-specific execution; isolate blocking work; measure audio and presentation latency |
| Compatibility bridge becomes permanent | Track callers and ownership; retire adapters only with explicit migration evidence |
| Metadata gives false confidence | Treat declarations as contracts requiring code review and runtime verification |

## Open implementation decisions

- Exact storage representation and reuse policy for graph templates and frame bindings.
- Production deque implementation and priority-propagation strategy.
- Initial compute budget and worker-affinity policy for supported deployment classes.
- Resource-token granularity for model-cache, spatial-partition, and entity domains.
- First particle capture set and numerical acceptance budgets.
- Whether later dynamic workloads justify child graphs or fiber-backed suspension.

These choices do not reopen the required semantics: explicit ownership,
nonblocking dependency scheduling, declared execution constraints, preserved
legacy phase order during migration, and measured acceptance.

## Decision

If accepted, new frame-related asynchronous work should use explicit runner and
scope contracts, declare its data and execution requirements, and provide a
defined publication point. The first implementation should deliver a serial
graph and particle pilot, then extend to the existing parallel kernels.

Broader entity and physics concurrency remains gated on data ownership and
behavioral equivalence. Completion of this RFC means the engine can express,
validate, execute, observe, and incrementally expand correct job graphs; it does
not mean every subsystem must execute concurrently on every frame.
