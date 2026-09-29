# RFC 0017: LAN Discovery and Portal 2 Co-op Pairing

- Status: Proposed (2026-09-28), revised after an independent review the same
  day. Nothing is implemented.
- Date: 2026-09-28
- Scope: A narrow, versioned LAN discovery contract with mDNS providers, and
  its first consumer: the imported Portal 2 matchmaking framework's `lan`
  network type, so two players can find each other and start co-op without
  Steam.
- Composition: [RFC 0001](0001-capability-based-platform-architecture.md)
  owns provider selection, iOS static composition and product profiles.
- Execution: [RFC 0003](0003-dependency-aware-job-system.md) and the
  `platform.task-runner.v1` contract (R10) own runners and sequences.
- Ownership and synchronization: [RFC 0006](0006-modern-cpp-ownership-and-synchronization.md)
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
  (Q-FOUNDATION for the contract, Q-PRODUCT for the two-process flow)
- Progress: none yet (`RFC/0017-progress.md` is created when work starts)

## Decision and boundary

Portal 2 co-op needs two clients to find each other, agree on a game, and
connect. Retail did this through Steam lobbies and friend invites. This
engine builds without Steam and its imported matchmaking framework has no
online sessions. This RFC fills the gap without Steam and without importing
the leaked Steam lobby code:

1. **Discovery is a contract, not a Steam emulation.**
   `platform.lan-discovery.v1` (`public/platform/contracts/lan_discovery.h`)
   advertises and browses named services on the local network. It knows
   nothing about Portal 2, lobbies or Steam. Providers implement it: mDNS on
   each platform, and a fake for tests.
2. **The session rides the framework's existing `lan` network type.** The
   imported framework already has a LAN path: `PlayerManager` LAN search,
   `OnNetLanConnectionlessPacket`, `PlayerFriend` entries and their `Join`,
   and the `FGT_SYSLINK` game lists. Discovery feeds that path instead of
   inventing a co-op-only online session type. A new `CMatchSessionLobby`
   is the session object behind it (see "Session").
3. **The session channel stays open until connect.** The PC co-op flow needs
   replicated session state and host/client commands after pairing (see
   "Observed starting point"). Pairing therefore opens a session channel
   that carries them until the game connection is made.
4. **Pairing gates admission; it is not confidentiality.** The Source
   netchannel is not encrypted, and this RFC does not change that.
5. **The provider is selected by the application root** and reaches the
   framework through a typed engine binding. No service locator, no filename
   discovery, no `dlopen` (iOS static composition).
6. **LAN scope is explicit.** mDNS does not cross NAT or subnets. Internet
   co-op is a non-goal; the session layer leaves room for another provider.

## Observed starting point (2026-09-28)

Facts read from source at revision `c5e61bb4` plus a dirty tree. Line numbers
were checked at that revision and drift; the progress record re-checks them.

Framework and build:

- `matchmaking/README.md`: the framework is imported from the CS:GO leak and
  built into the Portal 2 server game module. The online sessions
  (`sys_session`, `mm_session_online_*`, `ds_searcher`, `match_searcher`,
  `searchmanager`, `mm_netmgr`) and the X360/PS3/Steam lobby files are not
  imported. Online `CreateSession`/`MatchSession` broadcast
  `OnMatchSessionUpdate { state error, error n/a }`
  (`mm_framework.cpp:198-207, 723-727, 773-806`). Code needing them is
  behind `MM_PORTAL2_ONLINE_SESSIONS`, which is never defined. Under `SWDS`
  they compile out, so the dedicated server needs no provider.
- `NO_STEAM` is defined for matchmaking (`matchmaking/wscript`) and, in
  effect, for the client through `gameui/wscript` (defined unconditionally
  in the shared configure environment). `engine/wscript` defines it only for
  dedicated builds. `CClientSteamContext` is inert.
- The server module owns the framework (`Portal2_InitMatchFramework`,
  `game/server/portal2/portal2_matchmaking.cpp:173`) and publishes it
  through `IMatchFrameworkHost`. The client and GameUI are borrowers
  (`Portal2_ConnectMatchFramework`, `game/client/cdll_client_int.cpp:931`).
  The server module reaches its services through the string-keyed
  `appSystemFactory`.

The co-op UI is not the flow the first draft assumed:

- On PC, `CoopPlay` goes to `InitiateOnlineCoopPlay("playonline")`
  (`vcoopmode.cpp:60`). `vstartcoopgame` is reached only on consoles or from
  the leaderboard. The PC path forces a `LIVE` session (`bOnline = true`,
  `uigamedata.cpp:1697-1701`); `lan` is offered only under `_X360`.
- Under `NO_STEAM`: `CheckAndDisplayErrorIfNotLoggedIn`
  (`basemodframe.cpp:1183-1210`) always shows its error on PC, so
  `vfoundgames` is unreachable; `PvpLobby` lists fake "Friend #1/#2"
  (`vpvplobby.cpp:1990-1996`) and `SendInvite` does nothing
  (`vpvplobby.cpp:398-457`); nothing raises the `OnInvite` event that
  `uigamedata_invite` waits for.
- After "ready", the host opens `WT_PVP_LOBBY` (`basemodpanel.cpp:2336`).
  The joiner sends `Portal2::ClientReadyToStart` to the host
  (`basemodpanel.cpp:2226-2242`). The host issues `Start` only when
  `members/numMachines > 1` and `SessionMembersFindPlayer(clxuid)` succeeds
  (`basemodpanel.cpp:2491-2540`). `PvpLobby` consumes `state updated` events
  (`vpvplobby.cpp:1521-1540`). So the session needs replicated `members/*`
  and `game/state` settings, `run=host/clients` command routing and
  `GetSessionSystemData`.
- A `system/network "lan"` session already flows through `PlayerManager`
  LAN search (`playermanager.cpp:468-476, 530-610`) into `PlayerFriend`
  entries, the `FGT_SYSLINK` lists (`vpvplobby.cpp:1963-1986`,
  `vfoundgames.cpp:1575-1600`), `PlayerFriend::Join` (`player.cpp:213`) and
  QoS game-details packing. Much of it is gated for X360 or Steam.
- Under `NO_STEAM` every local XUID is `1` (`player.cpp:822`). Two machines
  therefore share one identity, which breaks `clxuid` member lookup and the
  player manager's self-filter.
- The listen-server start path for co-op is `CMatchTitle::StartServerMap`
  (`mm_title.cpp:226`) → `ApplyGameSettings` (`portal2_matchmaking.cpp:102-137`),
  where `members/numSlots` becomes `sv_portal_players`/`coop`. The joiner's
  connect exists as `QueueConnect` (`mm_session_offline_custom.cpp:95-120`).
- The console `connect` command takes `address [source]`; its second argument
  is the connection source, not a password (`engine/cl_main.cpp:861-876`).
  The client `password` convar is `FCVAR_ARCHIVE`
  (`engine/baseclientstate.cpp:177`).
- No engine main-loop single-thread task runner exists; runners live in
  `platform/runners` and in Hammer's `GlibTaskRunner`.
- No mDNS, Bonjour or zeroconf code exists in the tree (other matches are
  binaries or an unrelated device tool). No crypto dependency is pinned
  beyond `utils/lzma/C/Sha256.c`.
- `public/platform/contracts/` holds clock, dynamic library, paths, record
  store, sequence checker, task runner, tool process and achievement service.

Unknown and measured at G0, not assumed: whether two processes on one host
reach a shared co-op map with `connect` alone; which engine mechanism can
carry a per-session admission secret (see "Admission"); and which X360/Steam
gates in the LAN path can be lifted without changing console behavior.

## Goals

- Two Portal 2 clients on one LAN find each other through the existing PC
  co-op menus and reach a shared map, with no Steam and no manual IP entry.
- A reusable discovery contract with a shared suite that catches bad
  providers.
- Failure is visible: permission denial, no multicast and timeouts reach the
  UI as named errors, never as an empty list that looks like "nobody there".
- Every supported client profile declares co-op discovery as `required`,
  `optional` or `absent` (below).

## Non-goals

- Internet co-op, NAT traversal, relays, friends lists or presence.
- Encrypting game traffic, authenticating players beyond the pairing gate,
  anti-cheat, or a signing service.
- Matchmaking queues, rating, or the competitive lobby.
- Other Steam features (achievements, cloud, leaderboards).
- Persisting co-op progress across sessions.

## Layers and owners

| Piece | Owner | Notes |
| --- | --- | --- |
| `platform.lan-discovery.v1` contract | `public/platform/contracts/` | Strict C++20 header, no OS types |
| Providers (`mdns_posix`, `mdns_apple`, `mdns_android`) | `platform/<name>` | Native SDK types stay inside |
| Fake provider and shared suite | `unittests/platformtest` | Includes bad providers |
| Typed engine binding for discovery and the framework's runner | engine, beside `render_core_binding.h`/`linked_game_modules.h` | Replaces string lookup for this dependency; also serves static composition |
| Engine main-loop task runner | engine | A `platform.task-runner.v1` single-thread runner over the host frame; new deliverable |
| Install identity (per-install XUID) | matchmaking `player` | Random, persisted in the record store; the record store is not composed for matchmaking today (its only in-tree user is `game/shared/achievementmgr.cpp`), so G1 adds that composition |
| `CMatchSessionLobby`, `lan` network wiring | `matchmaking/` | Written to the observed calls, no leaked lobby import |
| Pairing and session channel protocol | `public/matchmaking/portal2/` (messages) and `matchmaking/` | One state machine |
| UI un-gating | `game/client/portal2/gameui` | Explicit list under "UI work" |

Dependency direction: matchmaking → discovery contract ← providers. The
contract depends on nothing. The dedicated server links no provider.

## Discovery contract (`platform.lan-discovery.v1`)

Behavioral obligations, each a suite clause.

- `Advertise(ServiceRecord, sequence) -> Expected<AdvertHandle, DiscoveryError>`.
  `ServiceRecord` is a service type, an instance name (UTF-8, length
  bounded), a port and TXT key/values. Keys and the total size are bounded to
  the DNS-SD limits; oversized records fail with `InvalidRecord`, they are
  not truncated.
- `AdvertHandle::Update(txt)` changes TXT in place; observers see one
  `Updated`, not a remove and add.
- Withdrawal (explicit or destroying the handle) sends a goodbye (TTL 0) and
  guarantees no further callbacks for that advert. It is idempotent and legal
  from inside the advert's own callback; the handle is then inert, and
  nothing is delivered after the callback returns.
- `Browse(filter, observer, sequence) -> Expected<BrowseHandle, DiscoveryError>`
  reports `Added`, `Updated` and `Removed` for instances of one service
  type. `Added` carries the resolved address and port; an instance that
  cannot be resolved is not reported as added. Browse handles obey the same
  withdrawal rule.
- **Bounded delivery.** Per browse, at most `N` distinct instances are
  tracked (`N` declared, default 64). Events are coalesced per instance:
  the latest state wins, so a flood of `Updated` for one instance costs one
  slot. `Removed` is never lost: it is stored as the instance's pending
  state, not queued, so no queue-full case exists for it. When an unseen
  instance would exceed `N`, it is ignored and the browse reports one
  `Truncated` notice (counted, not an error), so fake adverts cannot end
  discovery. Nothing else is silently dropped.
- **Self-detection is the consumer's job.** Platform services (`dns_sd`,
  `NsdManager`) do not flag your own advert. The contract does not promise
  it; hosts put a random instance id in TXT and consumers compare.
- **Interface changes.** Loss of every usable interface reports
  `Unavailable` on active handles and keeps them; return of an interface
  re-announces adverts and re-emits `Added` for live instances. Provider
  state after an interface change is observable through events, not by
  polling.
- Errors are a closed set: `PermissionDenied`, `Unavailable`,
  `InvalidRecord`, `NameConflict`, `Cancelled`. Unsupported behavior fails
  the call; nothing succeeds silently.
- Callbacks run on the sequence the caller supplies (a
  `platform.task-runner.v1` sequence). No provider delivers on a thread the
  caller cannot see; internal native threads post to the sequence.
- Destroying a provider with live handles is a violation the fake detects.
  A callback in flight when a handle is destroyed either completes before
  the destructor returns or is never delivered.
- On a name conflict the provider appends a suffix per DNS-SD convention and
  reports the final instance name; it never overwrites another host's
  record.
- Service type: the RFC's service is `_p2coop._tcp` because the session
  channel is TCP (RFC 6763 section 7 ties the label to the transport). The
  type string is one constant used by the provider, the plist key and the
  suite.

Bad providers for the suite (each detected on its own clause): never sends
goodbye; delivers after handle destruction; reports an unresolved instance;
truncates oversized TXT; floods `Updated` without coalescing; delivers on a
private thread; reports the same instance twice without a change; swallows
`PermissionDenied` as an empty browse; loses a `Removed` under load; lets
fake adverts end discovery; deadlocks when withdrawn from its own callback.

## Providers

| Profile | Provider | Constraints |
| --- | --- | --- |
| Linux | Embedded mDNS responder/querier (RFC 6762/6763 subset) over `getifaddrs` and multicast UDP. Optional Avahi provider behind the same contract for hosts that run it. | The responder binds 5353 with `SO_REUSEADDR`/`SO_REUSEPORT` so it coexists with Avahi and `systemd-resolved`; unicast-response behavior is tested against a running Avahi. Loopback for tests. Firewalls (ufw, firewalld) can block 5353 and the session port: the failure is diagnosed (`Unavailable` with a reason), not left as an empty list. |
| macOS, iOS, tvOS | Bonjour through `dns_sd` (or `Network.framework`), private to the Apple bridge. | `NSLocalNetworkUsageDescription` and `NSBonjourServices` (`_p2coop._tcp`) in the plist, on iOS and macOS 15. The permission also covers the unicast session and game connection, not only discovery. A backgrounded iOS host suspends its listen server; the lobby closes on background. No private APIs. |
| Android | `NsdManager` through JNI. | No `MulticastLock` for `NsdManager`; a lock only if a raw-socket fallback is added. `Updated` needs `registerServiceInfoCallback` (API 34); earlier levels resolve one service at a time. G6 measures the target SDK's local-network restrictions and pins the minimum API. Declared permissions only. |
| Windows, FreeBSD (legacy profiles) | The POSIX provider if it builds; otherwise declared unsupported. | Not a north-star target. |

The Linux embedded responder is the only provider with DNS wire code. It is
a bounded subset with a packet fuzz corpus: malformed names,
compression-pointer loops, oversized records, truncated packets, mismatched
counts. Apple and Android wrap platform services and need no parser.

Multicast realities are tested, not assumed: client isolation on access
points, VPNs capturing interfaces, IPv6 link-local scope ids, Wi-Fi roaming.

## Session

### Identity

Each install gets a random 64-bit XUID persisted through the record store,
replacing the constant `1` for LAN sessions. The player manager's self-filter,
`clxuid` lookup and the handshake's `joiner_id` use it.

### Advertisement

Service `_p2coop._tcp`, port = the session port. TXT: `v` (protocol
version), `gv` (game build, so a joiner can refuse a mismatch before
pairing), `id` (random instance id, for self-detection), `map`, `slots`
(free/total), `mode` (`open` or `coded`), `nonce` (16 random bytes, base32).
The pairing code is never advertised. Discovery results become `PlayerFriend` entries through the existing LAN
path. The syslink lists show only entries whose game details carry
`game/state == lobby`, `members/numPlayers == 1` and `system/network == lan`
(`vpvplobby.cpp:1968-1980`), so TXT (`map`, `slots`, `mode`) is mapped to
those game details by one function with its own tests; TXT is not extended
to imitate them.

Retail's LAN flow is symmetric: each player opens a lobby and one selects the
other's, which calls `Join` (`vpvplobby.cpp:431-437`). The RFC keeps that
shape. A player who joins another's lobby withdraws their own advert and
closes their own lobby first; the same rule applies if both select each
other at once (the lower instance id stays host).

### Pairing handshake (TCP session port)

1. The joiner picks a game and sends `Hello{v, xuid, nonce}`.
2. `mode=open`: the host replies `Accept`. `mode=coded`: the host replies
   `Challenge{c}` (fresh random).
3. The joiner replies `Proof = HMAC-SHA256(code, nonce || c || xuid)`. The
   host verifies in constant time. Three failures or a timeout close coded
   mode and rotate the code; the UI shows this.
4. On success both sides derive an admission secret from the code and the
   handshake transcript (HKDF-SHA256). The secret never crosses the wire
   (see "Admission").
5. The TCP connection stays open as the session channel (below). The host
   advertises the lobby full and, when the flow starts the game, both sides
   run the framework's existing start path.

Threat model, stated so it is not oversold: the code stops a stranger on the
same network from joining by accident or by browsing. A passive sniffer that
records a handshake can guess a short code offline against `Proof`, and the
game traffic is unencrypted. That is accepted for LAN co-op. If it is judged
too weak the replacement is a PAKE (decision D4), which changes only step 3.
In `open` mode there is no admission secret; the host accepts the first
joiner and closes the lobby, and any client can be that joiner.

### Session channel

The channel carries what the PC lobby needs after pairing, not only
create/join:

- replicated session settings (`system/*`, `game/*`, `members/*`,
  `members/numMachines`, `members/machineN/*`, `game/state`), raised to the
  framework as `state updated` events;
- command routing (`run=host`, `run=clients`), including
  `Portal2::ClientReadyToStart` and the host's `Start`;
- the `GetSessionSystemData` type and the ready/leave events the lobby and
  the found-games list consume;
- close, leave and timeout with visible reasons.

`CMatchSessionLobby` is the object behind `IMatchSession` for the `lan`
network type. It reuses the framework's `ApplyGameSettings` /
`CMatchTitle::StartServerMap` for the host (player count through
`members/numSlots`, not a raw `maxplayers`) and `QueueConnect` for the joiner.
It replaces the offline custom session's in-memory settings with settings
replicated to the peer. The protocol and its state machine have one owner;
message set and versioning are in `public/matchmaking/portal2/`.

Host states: `Idle → Advertising(open|coded) → Pairing → Paired →
InLobby → Starting → Connected → Closed`. Every state has a cancel path and
a timeout; every failure transition is explicit. If the joiner does not
connect within a bound, the host reopens the lobby.

### Admission

The engine's `connect` command takes no password, and the client `password`
convar is archived to config. A per-session secret must not be written to
disk. G0 determines which mechanism carries it:

- (a) the admission secret set through a non-archived password path on both
  sides and cleared after connect, or
- (b) the host admitting by the joiner's paired address through a server-side
  hook, with no shared password, or
- (c) if neither is workable without an engine change, an explicit statement
  that admission on the game port is by pairing only, and a lobby that is
  full or closed refuses further connects.

Decision D7 picks between them at the end of G0 on the recorded facts. In
every case the pairing code itself is never the password. Under (a) the
client writes the `password` convar into the connect packet in plaintext
(`baseclientstate.cpp:524`), so the secret is single-use, per session, and
exposed on the wire only at connect, when the slot fills; the lobby is
closed to further connects as soon as it is used. This is no protection
against a sniffer present at connect time, and the RFC does not claim it.

### Framework integration

- The framework receives the discovery provider and a main-loop runner
  through one typed binding, not by string lookup. The server module is
  dynamically loaded and today gets its services, `IMatchFrameworkHost`
  included, from `appSystemFactory` in `DLLInit`
  (`portal2_matchmaking.cpp:175`). The binding therefore reaches it as one
  versioned engine host interface (`IMatchFrameworkHost`'s successor,
  carrying the provider and the runner), recorded as the reviewed exception
  in `architecture/modules.json`; static products call the same interface
  directly. Client and GameUI borrowers are unchanged. The binding is R93
  work (G1), not a prerequisite from another row.
- **Profile states.** `required`: composition fails without a provider.
  `optional`: co-op discovery entries appear only if a provider composes,
  and the profile says which. `absent`: the entries are not offered and the
  profile says so. There is no state where the entries exist and always show
  nothing. Manual "address + code" entry (D5) needs no discovery provider,
  so an `absent` profile can still offer it.

## UI work (explicit; the flow is not unchanged)

Listed so it is planned, tested and reviewed rather than discovered:

1. PC co-op reaches the `lan` network type: `InitiateOnlineCoopPlay` and
   `CUIGameData` route to a LAN co-op session when discovery composes. The
   `LIVE`-only PC path and the `_X360`-only `lan` branch are changed with
   console behavior preserved.
2. `CheckAndDisplayErrorIfNotLoggedIn` no longer blocks a LAN session; a
   Steam-required feature still shows its error.
3. `PvpLobby` and `vfoundgames` show discovery results instead of the fake
   friend list; `FGT_SYSLINK` branches are un-gated for this path.
4. Invites: the `OnInvite` event is raised by the LAN path when a peer is
   paired (or the invite entry is removed for LAN sessions).
5. Code entry reuses the existing password dialog (`WT_PASSWORDENTRY`,
   `uigamedata.cpp:839`).
6. Manual entry and the error states (`PermissionDenied`, `Unavailable`,
   `Timeout`, wrong code, lobby closed) have text.

## Delivery plan and gates

Each gate is machine-decided and has negative controls. No gate accepts
skipped coverage.

| Gate | Work | Evidence |
| --- | --- | --- |
| G0 | Baseline: record the LAN path's gates and what lifts them, including the matchmaking-side ones (`MATCHTITLE_PLAYERMGR_DISABLED` at `mm_title.cpp:107`, friend updates at `playermanager.cpp:1147-1183`, LAN packets dropped unless a search is running at `playermanager.cpp:533`); decide the symmetric-join rule and whether the `lan` session survives `QueueConnect` (it auto-closes, while the host UI returns to the lobby on disconnect reason `lobby`, `basemodpanel.cpp:2338-2345`); boot two Portal 2 processes on one host and reach a shared co-op map by `connect` alone; decide D1/D3/D7 on facts; re-check the line references above | Progress record with commands, logs and decisions; no code claims |
| G1 | Contract, fake provider, shared suite, bad providers; engine main-loop runner; typed binding | Suite passes the fake and each bad provider is detected on its own clause; runner passes the shared runner suite; g++ and clang++, default and release |
| G2 | Linux mDNS provider and fuzz corpus | Shared suite; two-process browse/advertise on a veth pair and loopback; coexistence with a running Avahi; malformed-packet corpus; interface down/up; TSan lane |
| G3 | Install identity, session channel, `CMatchSessionLobby`, pairing state machine against the fake provider | State-machine suite: every transition and failure injection; wrong code, replay, full lobby, racing joiners, host close, timeout, rollback at each stage; replicated-settings equality on both peers |
| G4 | Product flow: two `play_p2` processes pair through the real PC menus and start a shared map | Required UI-driven test in an isolated compositor (headless mutter, AT-SPI, RemoteDesktop input, per the Hammer UI-test recipe), with wrong-code and lobby-full negative controls; shared-map check on both peers |
| G5 | Apple provider (iOS, tvOS, macOS) | Shared suite on device; permission-denial case; plist keys asserted by the package check; static composition check with the provider linked; optional-runner rules apply |
| G6 | Android provider on the Fold7 | Shared suite on device; API minimum pinned; background/foreground; permissions checked against the declared set. A missing device leaves the profile unverified |

A support claim for a profile needs its provider's suite, its native run and
the co-op flow. A cross-build alone certifies nothing.

## Roadmap

Row R93. Prerequisites: R06 (composition), R10 (runners), and R29 for the
Apple and Android gates. The typed binding is R93's own G1 work, so R39 is
not a prerequisite. Android must pass on the Fold7 for the row to close; a
missing device leaves the profile unverified and the row `partial`. Rank
placement is an agent decision; the user can move it.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| The LAN path is more X360/Steam-gated than it looks | G0 records the gates first; D1 is provisional until then |
| Networks block multicast (client isolation, VPNs, firewalls) | `Unavailable` with a reason; manual address + code entry over the same handshake |
| Embedded mDNS has parsing bugs | Bounded subset, fuzz corpus, sanitizers; Apple and Android use platform services |
| Coexisting with Avahi on 5353 | `SO_REUSEPORT` tests against a running Avahi at G2 |
| Android local-network behavior varies | `NsdManager`, measured on the Fold7; API minimum pinned |
| iOS permission surprises and background suspension | Explicit `PermissionDenied`, plist assertions, lobby closes on background |
| Retail lobby behavior is undocumented here | Written to the calls the UI makes in this tree, not to recalled behavior |
| Short-code guessing | Attempt limit, rotation, honest threat model, PAKE upgrade path |
| Crypto dependency | Pin one primitive source (HMAC-SHA256, HKDF) and a secure random source at G1, or use the platform's; no hand-rolled crypto |
| Leaked-source provenance | No new import of leaked lobby files; the repository warning stands |

## Alternatives considered

- **A co-op-mode online session type separate from the `lan` network path.**
  Rejected after review: the imported LAN path already has lists, friend
  entries, join and QoS packing; a parallel path duplicates it.
- **Emulate the Steam API and compile Steam back in.** Rejected: it needs the
  Steam runtime that Android and iOS cannot assume.
- **Import the leaked Steam lobby session files.** Rejected: provenance and
  console-era dependencies.
- **Avahi as the only Linux provider.** Rejected as the only path: it is a
  system daemon. It stays an optional provider.
- **A UDP broadcast beacon instead of mDNS.** Kept as a fallback if G2 shows
  mDNS reliability problems; it doesn't use the mobile platforms' service
  APIs and doesn't generalize.
- **A rendezvous server for internet play.** Out of scope; a later provider.

## Decisions (2026-09-28, agent; D1, D3 and D7 confirmed or changed at G0)

- D1: reuse the framework's `lan` path with a new `CMatchSessionLobby`; no
  leaked lobby import.
- D2: discovery is a platform contract with a provider per OS; Linux gets an
  embedded responder with an optional Avahi provider.
- D3: the session channel is TCP, service type `_p2coop._tcp`.
- D4: HMAC pairing with attempt limits for v1; PAKE only if the threat model
  is raised.
- D5: manual address + code entry is included as the multicast-hostile
  fallback, over the same handshake.
- D6: coded mode is the default; open-on-LAN is an explicit host choice.
- D7: the admission mechanism is chosen at G0 from (a)/(b)/(c).
- D8: a random per-install XUID for LAN sessions.

## Open questions

- Which of the X360/Steam gates in the LAN path can be lifted without
  changing console behavior? (G0)
- Should `platform.lan-discovery.v1` cover wide-area DNS-SD now to avoid a v2
  for a rendezvous provider, or wait for that consumer?
- Is one instance id in TXT enough for self-detection on Android API < 34,
  where updates arrive late?

## Source references

- [DNS-SD, RFC 6763](https://www.rfc-editor.org/rfc/rfc6763) and
  [mDNS, RFC 6762](https://www.rfc-editor.org/rfc/rfc6762)
- Apple local network privacy and Bonjour Info.plist keys; Android
  `NsdManager` documentation. They describe dependencies, not evidence of
  support here.
- [`matchmaking/README.md`](../matchmaking/README.md)

## Proposed decision

Adopt this RFC as the design for LAN discovery and co-op pairing, start at
G0, and keep the roadmap row `planned` until G0 records its baseline.
