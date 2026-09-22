# Composition contract v1 — RFC 0001 rank 3 / R06

The application root exclusively owns a single-sequence composition. Factories
are linked, typed functions; names are diagnostics, never lookup identities.
Only a factory sees its declared dependency view. It copies narrow references
into the consumer; it must not retain the view or the composition itself.
The root may borrow capabilities only while the composition is running.

Validation precedes all factory calls. Duplicate names/exports, self/cyclic
dependencies and absent required capabilities fail with the consumer and
capability named. Optional absence is a null reference. Optional presence adds
the same lifetime/order dependency as required presence. Registration order
breaks ties among ready providers. Duplicate requirements are rejected.

Factories construct in dependency order. All providers then connect in that
order, then initialize in that order. No externally visible capability exists
until every initializer succeeds. A null successful factory result is an error.
Factory failure releases its private resources with RAII; no instance escapes.

Shutdown runs in reverse initialization order, including the failed initializer
on a partial startup. Disconnect runs in reverse connection order, including
the failed connector. Those methods must tolerate a partially completed attempt,
finish cancellation/drain synchronously, release subscriptions, and never throw.
Objects are destroyed in reverse construction order, after shutdown/disconnect.
Dependencies remain alive through their borrowers' destructors.

Stop is idempotent. Destruction stops a running instance. Retrying startup after
failure and a second start after stop reconstruct fresh providers. Add/Start/Stop
during lifecycle callbacks return Busy and never mutate the active traversal.
Two compositions are independent. A provider is never replaced while running.
Factory/lifecycle callbacks return recoverable errors; exceptions must not cross
this no-exception contract. Allocation exhaustion follows the process policy.
Returned diagnostics own their text and remain valid after composition teardown.

Legacy bindings are domain-owned typed RAII scopes. A binding rejects an occupied
slot before mutation, publishes only after successful acquisition, clears on
reset/destruction, and outlives legacy consumers. Put a binding in a bridge
provider's Connect/Disconnect, with explicit edges from its consumers. This
keeps the bridge alive during initialization, shutdown, and drain. The kernel
does not publish a global capability registry.

The shared outcome oracle checks traces, dependency lifetime, work drain,
rollback, retry, isolation and visibility. Deliberately bad providers (leaked
borrow, unacknowledged task, retained subscription, bad bridge clear) must fail
the same oracle. Assertions remain active with NDEBUG. The reference unittest
root links suites explicitly and requires nonzero reconciled test/check counts.
