# Hammer contract conformance suites (RFC 0002)

Each replaceable editor port gets a contract record and a provider-parameterized
conformance suite here, per RFC 0002 "Required contract record" and RFC 0005
Q-EDITOR / Q-CONTENT / Q-PRESENTATION.

A contract record MUST specify: purpose and consumers; accepted inputs, units,
coordinate spaces, encoding, and validation limits; results, error taxonomy, and
post-success/failure/cancel guarantees; ownership, borrowed-value validity,
threading, ordering, and reentrancy; state invariants and legal operation
sequences; permitted side effects and performance requirements; and the
conformance suite plus the providers/configurations that execute it.

Every provider claiming a contract — including test fakes — runs the same suite.
A deliberately non-conforming ("negative") provider proves the suite detects
precondition, result-guarantee, ordering, and ownership violations.

Status: scaffolding only. No contracts are installed yet; the first records land
with HAM-GEOMETRY-001 (`geometry.boundbox.v1`) and HAM-SEL-001
(`selection.change.v1`, `property_draft.commit.v1`, `transform.request.v1`).
