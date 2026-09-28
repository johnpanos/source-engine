# Contract: `presenters.action_catalog.v1`

Module: `hammer.presenters`
Header: `public/hammer/presenters/action_catalog.h` · Impl: `hammer/core/presenters/action_catalog.cpp`
Conformance: `unittests/hammertest/presenters/test_action_catalog.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The editor's actions for menus, toolbars and keys as data over the one
command authority, `app::SessionCommands`. Consumers: GTK menus, toolbars
and key handling; the status bar (messages).

## 2. Accepted inputs

A session, `EditorSettings`, the commands and the clipboard they use. Action
ids, chords in any spelling, host arguments (the map path).

## 3. Results and guarantees

- Every Command action names a catalog command, passes only arguments it
  declares and provides every argument it requires (fixed, toggle, grid or
  host). Tool actions return the `hammer.tools` tool name; Host actions
  return a host action name.
- Ids and chords are unique; chords are stored canonical (Ctrl, Alt, Shift,
  Meta; upper-case letters; canonical key names) and looked up in any
  modifier order or case.
- Enabled rules read history, selection kinds, clipboard, hidden objects and
  the grid; checked rules read `EditorSettings`. A disabled action runs
  nothing (Disabled).
- Execute returns the command output, or Disabled, UnknownAction,
  MissingArgument or CommandFailed (with the `CommandError`); every call
  leaves `LastMessage()` for the status bar. `UndoLabel()` is "Undo <label>".
- Key conflicts and their resolution are listed in the header.

## 4. Ownership, threading

Owns only the last message. Single sequence. RAII subscription.

## 5. Invariants

One chord maps to one action; one action id is stable across releases.

## 6. Side effects and performance

Rule evaluation is O(selection) (AnyHidden is O(objects)).

## 7. Conformance suite and providers

`test_action_catalog.cpp` (46 checks, gcc and clang); negative checks for
disabled actions, unknown ids and chords, malformed chords, missing host
arguments and failing commands.
