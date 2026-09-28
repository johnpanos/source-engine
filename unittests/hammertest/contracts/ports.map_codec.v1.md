# Contract: `ports.map_codec.v1`

Module: `hammer.ports` · Interface: `hammer::ports::IMapCodec`
Header: `public/hammer/ports/map_codec.h`
Real provider: `hammer::formats::VmfMapCodec` (`formats.vmf_map_codec.v1`)
Shared suite: `unittests/hammertest/formats/map_codec_oracle.h`
Conformance: `unittests/hammertest/formats/test_vmf_map_codec.cpp` (+ `_negative`)
Migration: `HAM-VMFCODEC-001` (proposed id; R08-DOMAIN)

Map persistence port. A codec turns file text into a detached
`hammer::scene::MapDocument` and back. `hammer.app` consumes the port and never
selects or constructs a codec; the composition root wires one.

## 1. Purpose, consumers, scope

- Consumers: the `hammer.app` edit session (open, save, revert), the save
  orchestrator, `hammer_cli`/MCP load and save commands, and any tool that needs
  the typed document instead of a keyvalues tree.
- Scope: whole-document text encode/decode. No I/O: bytes come from and go to an
  `IFileStore` owned by the caller. Required for every product that opens maps.

## 2. Accepted inputs

- `Decode( text, serial )`: any text. `serial` is the document serial every
  runtime id of the result carries (non-zero; see `map_objects.h`).
- `Encode( document )`: any `MapDocument`, including one built by edits whose
  runtime ids are in creation order rather than file order.

## 3. Results and guarantees

- **Detached decode.** `Decode` builds a new document and returns it only on
  success; it never touches an existing document. Text that is not the format
  fails with `CodecError { message, line }`, `line` the 1-based source line.
- **Consistent result.** A decoded document passes `MapDocument::Validate()`.
- **Warnings, not loss.** Recoverable oddities are `CodecDiagnostic` warnings
  (with a line when one applies); the affected data is kept verbatim where the
  format allows.
- **Serial.** Every runtime id of a decoded document carries `serial`.
- **Round trip from text.** `Decode(Encode(Decode(t)))` is `SameContent` with
  `Decode(t)` including equal runtime ids (the codec issues ids in a fixed
  order and encodes in that order), and the second encode is byte-identical.
- **Round trip from a document.** `Decode(Encode(d))` holds `d`'s content up to
  a renaming of runtime ids: objects pair by persistent (VMF) id and every
  reference (id, owner, group) maps through the pairing. The one permitted
  normalization is the cordon form (`map_document.h`): the single form only when
  it can express the content, the list form otherwise.
- **Encode failure.** `Encode` fails only for content the format cannot express
  (see the provider's contract), never by writing something that reads back
  differently.

## 4. Ownership, threading

- Stateless: both calls are pure functions of their arguments. No globals, no
  I/O. Safe to call concurrently on any thread for distinct inputs.
- The returned document and text are owned by the caller.

## 5. Invariants

- The codec never keeps two authorities: the decoded document is the only
  content; nothing is cached between calls.
- Persistent ids in the output are never lower than the document's `NextVmfId`
  floor would allow a collision with (fresh ids come from `AllocateVmfId` after
  every id in the input was noted).

## 6. Side effects and performance

- None beyond allocation. Linear in the text size; the two Portal 2 sample maps
  (1.5 MB and 0.4 MB, 986 solids) decode and re-encode in well under a second
  with `-O2`.

## 7. Conformance suite and providers

- `map_codec_oracle.h` is the shared suite. Every clause takes `const IMapCodec &`
  and returns `Findings` (empty = pass): `RoundTrip`, `DocumentRoundTrip`,
  `SemanticText` (exact or numbers-by-value), `CanonicalText`,
  `CanonicalFixture`, `SingleCordonFixture`, `OddityFixture`, and the
  comparator `SameContentUpToIds`.
- `test_vmf_map_codec.cpp` runs every clause against `VmfMapCodec`.
- `test_vmf_map_codec_negative.cpp` runs 14 seeded bad codecs (each wraps the
  real one and breaks one promise) and requires each to be detected by its named
  clause, with the real codec as the control; it also checks the comparator
  detects a changed field, a dropped reference and a settings change while
  accepting renamed ids.
- Both run headlessly with g++ and clang++ (`-std=c++20 -Wall -Wextra -Werror`)
  on `linux-headless-core`. A fake codec for application tests can reuse the
  same oracle.
