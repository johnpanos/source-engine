# Hammer characterization fixtures (RFC 0002)

Small, redistributable or generated maps, golden observations, seeds, and
versioned semantic expectations for the editor migration.

Each fixture records provenance in `provenance.json` beside it: applicable game
profile, feature tags, expected diagnostics, generation seed (if generated), and
the semantic-comparator version that produced its goldens. Product assets needed
for rendering tests are declared separately from redistributable headless
fixtures (do not commit non-redistributable game content here).

Minimum coverage the corpus builds toward (HAM-CORPUS-001): convex and invalid
brushes, clipping and transforms, texture axes/locks, displacements and sewing,
entity I/O connections, groups and visgroups, overlays, nested instances, unknown
VMF content, missing assets, malformed files, and differing path case/encoding.
Tool traces cover commit, cancel, focus/capture loss, document switch, and
undo/redo.

A legacy result is evidence, not automatically the desired specification. An
observed bug is recorded as a separate behavior change with its own expected
result; updating a golden must not silently redefine a contract.

Status: scaffolding only. Fixtures are added as the geometry/scene seams (H1) and
persistence slice (H2) land.
