# Contract: `platform.record-store.v1`

Module: `platform.contracts` · Types: `platform::IRecordStore`,
`platform::RecordStoreError`, `platform::IsValidRecordKey`
Header: `public/platform/contracts/record_store.h`
Shared suite: `unittests/platformtest/record_store/record_store_conformance.h`
Conformance: `unittests/platformtest/record_store/test_record_store.cpp` (+ `_negative`),
`test_record_store_user_defaults.mm` (Apple devices)
Test backend: `unittests/platformtest/record_store/fake_record_store.h`
RFC: 0001 (foundation capability, player data) · Migration: `PLAT-RECORDS-001`
Domain: Q-FOUNDATION

## 1. Purpose, consumers, providers

A record store keeps small named byte records across runs: player state
such as the achievement manager's `gamestate.txt`. The record's format
belongs to the caller; the store decides only where and how the bytes are
kept. The application root selects the provider:

| Provider | Module | Products | Where a record lives |
| --- | --- | --- | --- |
| `FileRecordStore` | `platform.records` (`platform/records/`) | desktop, Android, iOS (the default) | the file `<directory>/<key>`; the achievement manager passes the game's write directory, where the engine's file system wrote `gamestate.txt` before (it lowercases names on POSIX) |
| `UserDefaultsRecordStore` | `platform.apple` (`platform/apple/`) | tvOS | an `NSData` under `source.records.<key>` in the standard user defaults, tvOS's only persistent local storage |

Consumer: `CAchievementMgr` (`game/shared/achievementmgr.cpp`) through
`public/game/game_platform_services.h`. A module the root does not bind uses
the file store.

## 2. Accepted inputs

- Keys: 1 to 64 ASCII letters, digits, `_`, `-`, `.`, not starting with `.`
  (`IsValidRecordKey`, the one definition). Keys compare exactly; callers must
  not use two keys that differ only in case, since a file store on a
  case-insensitive file system aliases them.
- Records: any bytes, including NUL bytes and the empty record, up to
  `MaxRecordBytes()`.

## 3. Results and guarantees

1. `MaxRecordBytes()` is positive and constant for the store's lifetime.
2. An invalid key fails with `kInvalidKey` on `Load` and `Commit`, touching nothing.
3. `Load` of a key never committed fails with `kNotFound`, not `kIoFailure`:
   callers start from empty state on `kNotFound` but must not overwrite a
   record they failed to read (the achievement manager stops saving until a
   load succeeds).
4. `Load` returns exactly the bytes of the last successful `Commit`; an empty
   record is a record.
5. `Commit` replaces the whole record.
6. Keys are independent.
7. A record of exactly `MaxRecordBytes()` is accepted; a larger one fails with
   `kTooLarge` and the previous record stays.
8. A new instance over the same storage (a restart) returns the last committed
   records.
9. Every method may be called from any thread, concurrently. Per key, `Commit`
   is atomic: a `Load` returns the whole of some committed record, never a mix.
   A failed `Commit` leaves the previous record.

Durability after `Commit` returns is each provider's documented guarantee:
the file store does not flush to the device, and the user defaults write on
their own schedule and when the app leaves the foreground, so power loss or a
crash may drop the last commit. The file store's replace is atomic where
`rename` replaces its target (POSIX); on Windows it removes the old file
first.

## 4. Evidence

- `platform.record_store`: the shared suite against the fake and the file
  store, plus the file store's clauses (the `gamestate.txt` location, a stale
  staging file, a missing directory, an oversized file).
- `platform.record_store.sensitivity`: ten broken stores, each rejected.
- `platform.record_store.user_defaults` (profile `apple-uikit-device`): the
  shared suite against the real user defaults on an iOS or tvOS device, plus
  the layout and foreign-value clauses, run by `tools/quality/ios_conformance.py`.
  A restart across processes is modeled by a new instance in one process.
