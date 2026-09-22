# RFC 0006: C++20, Ownership, and Synchronization

- Status: Proposed
- Date: 2026-09-21
- Scope: New and migrated first-party targets, result/ownership vocabulary, queues and CPU/GPU synchronization
- Related: [Platform](0001-capability-based-platform-architecture.md), [Hammer](0002-hammer-responsibility-factorization.md), [jobs](0003-dependency-aware-job-system.md), [physics](0004-box3d-primary-physics-backend.md), [harnesses](0005-quality-and-correctness-harnesses.md)
- Implementation status: Specification with an installed mechanical-style checker (see below). Existing targets are not upgraded by this document.

## Summary

Use C++20 in new and deliberately migrated first-party targets, with explicit
results, scoped resources, typed ownership, and tested synchronization. Select
facilities because they make an invariant clearer or an operation measurably
better. Preserve advertised legacy interfaces and separately compiled native
dependencies, including Box3D's C17 sources.

The existing Waf configuration selects C++11 for applicable C++ compilation.
Implementation therefore begins with an explicit per-target compiler and
standard-library capability policy; it cannot assume C++20 is already active.

## Mechanical style gate

The installed [style checker](../tools/stylelint/README.md) and root
[format policy](../.clang-format) implement the mechanical portion of this RFC.
The pinned formatter preserves Source tabs, Allman braces, declarator-side
pointer/reference sigils, and nonempty-parenthesis spacing, with a 100-column
target. Include order is preserved; a lexical ratchet rejects new includes after
literal `memdbgon.h`. Existing files use changed syntactic regions, new files
whole-file checks, and vendor code keeps its own conventions.

The read-only checker, regression/negative fixtures, and PR/master CI lane are
the R04-STYLE child tracked in [AGENTS.md](../AGENTS.md). Missing dependencies or
invalid output fail; documentation-only diffs report not-applicable. The format
configuration's C++20 mode is not a build migration. Ownership, results, DRY,
behavioral substitution, ring topology, memory ordering, and GPU completion
still require the contract and native verification below; token bans cannot
prove those properties. Do not mark M0–M3 done on the strength of a style pass.

## Build and compatibility policy

New strict targets SHOULD use C++20. Every such target declares a supported
compiler/standard-library/platform profile and has compile/link/run evidence.
Implement an explicit Waf target property or equivalent mechanism that prevents
the legacy global standard flag from silently overriding the target's choice.
Record the final compiler invocation in build evidence.

Legacy targets may remain on their established dialect until migrated. C++20
headers must not become transitive requirements of an unmigrated target through
a shared public header. Introduce a narrow compatible facade, migrate the caller
cohort, or move the boundary to values/functions with an appropriate ABI.

The same process can contain separately built targets only after their shared
boundary is validated. Language-mode compatibility does not establish allocator,
runtime-library, standard-library, exception, layout, or binary compatibility.
Test supported shared-library combinations on each profile.

Published VPhysics and other legacy vtables/layouts remain unchanged unless a
new version is intentionally introduced. New genuine extension ABIs use the
family-owned, size-versioned C tables from RFC 0001. Do not expose STL containers,
smart pointers, `Expected`, exceptions, or allocator ownership across those ABIs.
Private first-party C++ interfaces may use modern types where both sides share
a tested toolchain and ownership contract.

Box3D remains a private C17 target with its required floating-point, precision,
alignment, PIC, linkage, and runtime settings. Do not apply a whole-tree C++20
flag or a global fast-math change as part of this work.

## Results and value types

Use the no-exception `Expected<T, E>` / `Expected<void, E>` vocabulary in RFC 0001
for recoverable failures. `std::expected` is C++23; a C++20 build uses a tested
project implementation or an explicitly approved equivalent. The result
wrapper performs no hidden logging or allocation; its payloads may allocate
only as allowed by their own contracts. Errors remain compact and domain-owned.

Use `[[nodiscard]]` where ignoring a result loses a required decision. Use
`std::optional` for meaningful absence, not to erase error information. Prefer
strong IDs, scoped enums, and named quantity/conversion functions over ambiguous
integers, flags, or generic native handles.

Use `std::span` and `std::string_view` for bounded non-owning views where their
lifetime and mutability are explicit. Neither extends storage lifetime. Jobs,
callbacks, caches, and queued commands cannot retain a view into a temporary,
reallocated vector, stack frame, or retired frame arena.

Prefer values and ordinary functions for deterministic transformations. Use
templates/concepts or compile-time checks when they enforce a useful invariant
with understandable diagnostics. Do not turn simple operations into a framework
merely because C++20 enables it. Runtime behavioral substitution still requires
contract tests; satisfying a concept is not proof of lifetime or semantic parity.

## Ownership and scope

- Prefer value ownership and RAII. A scope releases resources on success,
  failure, cancellation acknowledgment, and partial initialization paths.
- Use `std::unique_ptr` for exclusive heap ownership and custom deleters for
  resources released by a provider API. Destruction must occur on the permitted
  sequence while that provider and its code remain alive.
- Use references, observer pointers, IDs, or spans for borrowing, with explicit
  validity. Raw pointers at preserved ABIs are adapted at the boundary.
- Use `std::shared_ptr` only when multiple parties actually own a lifetime.
  Document cycles, synchronization, allocation cost, and the destruction
  context. Atomic reference counting does not make the pointee thread-safe.
- Use weak references or generation-checked delivery tokens for late callbacks
  where appropriate. A successful generation check alone does not pin an object
  against concurrent destruction.

Native library handles remain move-only scoped resources with borrowed symbols.
Physics wrappers and scene identities are not replaced by raw backend IDs or
shared pointers merely for convenience. Shutdown drains work before destruction;
RAII cannot repair a dangling borrower or an unsafe destruction sequence.

Allocation budgets are explicit. Prefer bounded/reused storage on frame paths;
do not insert shared ownership or per-job allocations without measurement.
An arena or pool retains destructors and module lifetime until all users finish.

## Ring buffers and bounded queues

Ring buffers are appropriate for measured bounded producer/consumer transport,
such as telemetry, commands, streaming, and selected scheduler queues. They are
not the default container for every sequence. Choose a proven simple mutex queue
when it meets the contract and budget; lock-free operation needs its own evidence.

Every queue contract declares:

1. Producer/consumer topology: SPSC, MPSC, SPMC, or MPMC; physical-thread versus
   sequence restrictions; whether reentrancy is allowed.
2. Capacity, reservation/publication, ordering, close/drain, and full/empty
   behavior. FIFO means a specified publication/admission order, not an invented
   gameplay order between racing producers.
3. Payload ownership, construction/destruction, cancellation, and borrow lifetime.
4. Overflow/backpressure policy: reject, defer, block on an allowed executor, or
   explicitly drop permitted telemetry/optional work with an observable count.
   Required gameplay work cannot be silently overwritten or dropped.
5. Progress guarantees and memory ordering, including wraparound/generation
   handling, slot reuse, and shutdown with producers/consumers in flight.

SPSC assumptions cannot be reused for multiple producers without a separately
designed algorithm. A work-stealing deque is not interchangeable with a FIFO
ring. A bounded FIFO is not a substitute for job dependencies or GPU completion.
Payloads cannot be overwritten while the consumer, a callback, or the GPU still
borrows them. Test very small capacities and rapid reuse to expose these errors.

## CPU publication and synchronization

Use standard atomics, mutexes, condition variables, scoped locks, and applicable
C++20 wait/notification facilities through the foundation boundary. Native
worker creation stays in provider/infrastructure code. Features use the runner
contracts rather than creating a thread for serialization.

Every lock-free publication documents its happens-before argument. A producer
completes payload writes before publishing availability with release semantics;
the consumer acquires the matching publication before reading. Storage reuse
requires the corresponding consumer-completion/producer observation protocol.
More complex topologies require an algorithm-specific proof, not this sketch.

Prefer synchronization attached to the atomic state transition. A standalone
`atomic_thread_fence` is permitted only with a documented matching atomic
operation and ordering argument. `volatile` is not thread synchronization.
`notify_one`/`notify_all` do not replace publication: waiters check an atomic
predicate in a loop, and shutdown updates and signals the relevant state.

Start with clear ordering; weaken it only with an explained argument and tests.
Sequential consistency does not fix non-atomic data races or incorrect ownership.
False sharing, spin counts, padding, and relaxed operations are measured tuning.

`std::jthread` and stop tokens may simplify infrastructure ownership where the
selected standard library supports them. Stop is cooperative; a request does
not imply task completion or permit payload release. Blocking joins/waits are
restricted to contexts where progress can be made. Job dependencies use ready
work and continuations, not a worker blocked on work queued to its own pool.

## CPU task completion and GPU fences

Keep three contracts distinct:

| Mechanism | Establishes | Does not establish |
| --- | --- | --- |
| CPU atomic/mutex publication | Visibility/order between participating CPU accesses | GPU completion or safe device-resource reuse |
| Job completion/dependency token | Terminal task state and declared CPU output publication | Completion of commands merely submitted to a GPU |
| GPU fence/timeline completion | Provider-defined completion of submitted device work | Arbitrary CPU visibility without required waits/cache transitions |

The render provider owns native fences, timeline values, barriers, and cache
transitions. Portable consumers receive typed completion tokens and scoped
resources, not native fence handles. Tokens identify their device/epoch so stale
values cannot release resources after device recreation.

Upload rings and buffered frames associate each occupied range with the GPU
completion that permits recycling it. Exhaustion has an explicit policy; frame
number modulo buffer count is not evidence that the GPU has finished. Prefer
deferred retirement/polling on an appropriate runner over blocking frame compute
workers. Device loss has a provider-defined invalidation/drain protocol.

GTK texture publication follows its own immutable/handed-off resource contract.
A CPU render callback returning cannot by itself authorize texture overwrite.

## Required verification

Use [RFC 0005](0005-quality-and-correctness-harnesses.md) for evidence and gates.
Tests include:

- Target-level C++20 compiler/library probes and actual mixed-target ABI calls;
  frozen legacy consumer fixtures; no C++20 requirement leaked to legacy headers.
- Result success/error/move/destruction, move-only payloads, allocation policy,
  wrong-state handling, and ignored-result diagnostics where enabled.
- Scoped resource transfer, partial failure, custom deleters, cancellation,
  provider/module lifetime, and no double release or unsafe late callback.
- Queue full/empty/wraparound, exact supported ordering, payload publication,
  slot reuse, nontrivial payload destruction, close/drain, and contention.
- Negative implementations missing publication, releasing a slot early, or
  dropping required work; the suite must detect these seeded defects.
- Real-thread sanitizer/stress runs and bounded schedule/model exploration.
  Memory-order tests on supported weakly ordered architectures complement x86
  results; a single-thread scheduler model cannot prove atomic correctness.
- Delayed GPU completion, ring exhaustion, stale device epochs, multi-frame
  ownership, context/device loss, and native validation where available.
- Queue throughput/tail latency, memory and allocation costs under realistic
  workloads before performance claims or replacing a simpler implementation.

## Delivery and acceptance

| Stage | Deliverable | Done |
| --- | --- | --- |
| M0 | Per-target toolchain policy | Actual C++20 compile/link/run profile; legacy and C17 targets retain their settings; supported ABI combinations tested |
| M1 | Results and scoped ownership | Vocabulary and lifetime tests pass; first composition/consumer uses them with no ABI leakage |
| M2 | Bounded transport | A named real consumer has a documented queue contract, overload behavior, passing concurrency tests, and measured cost |
| M3 | Completion and retirement | Scheduler publication and render resource reuse pass separate CPU/GPU lifetime tests and native integration |
| M4 | Incremental adoption | Each migrated caller removes obsolete ownership/synchronization code and passes domain/product gates |

M2 and M3 are introduced with actual scheduler/render/telemetry consumers; they
are not prerequisites to all editor or physics extraction. Coroutines, fibers,
modules, custom allocators, and lock-free structures require a concrete need and
their own compatibility/verification evidence. They are not program milestones
simply because the language makes them possible.

## Decision

Adopt C++20 deliberately at first-party target boundaries. Make ownership and
failure visible, use bounded queues with explicit publication and overload
semantics, and distinguish CPU ordering, task completion, and GPU completion.
Preserve ABI and measure the cost of changes to hot paths.
