# Contract: `platform.tool_process.v1`

Module: `platform.contracts` · Types: `platform::IToolProcessProvider`,
`platform::ToolProcessRequest`, `platform::ToolProcessResult`
Header: `public/platform/contracts/tool_process.h`
Consumer: `platform::ToolProcessClient`
Shared suite: `unittests/platformtest/tool_process/tool_process_conformance.h`
Test backend: `unittests/platformtest/tool_process/fake_tool_process.h`
RFC: 0001 Phase E · Migration: `TOOL-PROCESS-001` · Domain: Q-FOUNDATION

## 1. Purpose and scope

This is the process boundary used when a tool needs crash isolation, an alternate
version, or remote-execution-compatible orchestration. It replaces filename-based
launchable-DLL indirection with a structured child-process request. This slice
defines and tests portable semantics only. It provides no native process backend
and migrates no legacy tool.

## 2. Accepted requests

- `argv` is an ordered vector. `argv[0]` names the executable; every later item,
  including an empty string or a string containing spaces, is one exact argument.
  Providers never join it into an unescaped command string.
- `workingDirectory` is explicit and applies only to the child.
- Environment entries override the inherited child environment. An engaged value
  sets the exact value (including empty); `nullopt` removes the variable. Names are
  non-empty, unique, contain neither `=` nor NUL, and values contain no NUL.
- Execution and cancellation timeouts are positive. `executionTimeout` is the
  deadline before termination begins; `cancellationTimeout` is the grace period
  for a clean stop before forced termination. Expiring that grace period never
  permits a provider to return while a child or provider task remains live. The
  optional cancellation object is borrowed only during `Run` and may be queried
  from a provider thread.
- Empty `argv`, empty `argv[0]`, empty cwd, embedded NUL, invalid/duplicate
  environment names, or non-positive timeouts return `kInvalidRequest` without
  spawning or partially mutating the parent process.

## 3. Results and guarantees

- `kExited` carries the child's exact exit status in `exitCode`. Exit zero is the
  only successful result; nonzero exit is still a normal observed exit, not a
  provider error.
- `kSpawnFailed` has no exit code and carries a structured spawn error such as
  `kExecutableNotFound`; a successful exit may never stand in for spawn failure.
- Captured stdout and stderr remain separate, owning byte strings. They retain
  data captured before nonzero exit, timeout, or cancellation.
- On deadline, `Run` terminates the child/process tree and returns `kTimedOut` only
  after cancellation is acknowledged. When external cancellation is observed it
  likewise returns `kCanceled` only after acknowledgment.
- Every provider return, including validation and spawn failure, has
  `cleanupComplete == true` and restores `LiveProcessCount()` to its pre-call
  value. The provider does not retain request views, cancellation pointers,
  handles, pipes, tasks, or child processes after return.
- `Run` does not mutate its request or the parent's cwd/environment. Calls are
  synchronous; concurrent `Run` support is not promised by v1.

## 4. Ownership and lifetime

Requests and results own their strings and vectors. The provider borrows the
request and optional cancellation object for `Run` only. The consumer borrows a
provider that outlives it. Result output is independent of provider lifetime.

## 5. Conformance and sensitivity

The positive suite runs the real portable `ToolProcessClient` against a
deterministic provider. It covers an empty argument, spaces, quotes, UTF-8 and a
backslash in argv; explicit cwd; set/empty/unset environment overrides; binary
stdout distinct from stderr; zero and nonzero exit; invalid request; executable
not found; retained partial output on timeout; acknowledged external cancellation;
request immutability; and zero live processes after every call.

The sensitivity suite feeds the same predicate ten deliberately broken providers:
flattened argv, ignored cwd, ignored environment, merged streams, wrong exit code,
spawn failure reported as success, timeout reported as exit, unacknowledged
cancellation, incomplete cleanup, and a leaked process. Every defect must be
detected before this suite can gate a native provider.

## 6. Non-claims

The fake has no OS process, pipe, signal, or process tree. Passing it proves the
contract and oracle are usable and sensitive, not that POSIX, Windows, Apple, or
Android process behavior works. Each native provider must run this same suite
with a fixture executable on its declared product/profile. Installed tool
workflows and content outputs remain separate Q-CONTENT/Q-PRODUCT evidence.
