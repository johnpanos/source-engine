# Contract: `app.session_commands.v1`

Module: `hammer.app`
Headers: `public/hammer/app/session_commands.h`, `command_script.h`, `editor_settings.h` ·
Impl: `hammer/core/app/session_commands.cpp`, `command_script.cpp`
Conformance: `unittests/hammertest/app/test_session_commands.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The named, serializable command layer over `EditSession`: one catalog shared by
scripts, UI actions, UI-driven tests and the MCP server. `command_script` is
the shared vocabulary (errors, catalog entries, argument validation, the
script format and value parsers) that the older `EditorCommands` also uses.
Required.

## 2. Accepted inputs

A command name and string arguments. Script ids are the low 32 bits of runtime
ids; id lists are space separated; faces are `solid:side` pairs (side VMF ids);
vectors are `"x y z"`. Commands that take `ids` default to the selection, and
face commands to the selected faces.

## 3. Results and guarantees

- Unknown commands, missing and undeclared arguments, malformed numbers,
  vectors, ids and faces, and unknown ids are structured errors naming the
  command; nothing changes.
- Every document change is one `EditSession::Execute`, so one command is one
  undo step with a readable label.
- Creating commands return the new ids; queries return key=value text.
- Commands needing a service the composition lacks (codec, store, builder,
  materials) are rejected by name.
- A script stops at the first error, which carries its line.

## 4. Ownership, threading

Borrows the session, the settings and the services. Single sequence.

## 5. Invariants

The catalog and dispatch come from one table, so they cannot disagree; names
are unique.

## 6. Side effects and performance

File I/O and builds only through the injected ports.

## 7. Conformance suite and providers

`test_session_commands.cpp` (63 checks): a scripted sealed room saved and
reopened through the fake codec and in-memory store, every command family,
settings, history and every error path.
