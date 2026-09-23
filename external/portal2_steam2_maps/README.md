# Historical Portal 2 VMF instance sources

These are Portal 2 Puzzle Maker instance VMFs extracted from Steam2 depot
`841` on its later, smaller update branch. The source paths are preserved
under `841_8`, `841_22`, and `841_24`. The respective extractions contain
70, 89, and 103 map source files. The original depot blob CRCs are
`296ad036`, `5b1ad7f4`, and `88c65099`.

Every copied VMF was compared byte for byte with its extracted source.
Compared with the installed Portal 2 authoring tools, version 8 has 22 VMFs
absent from that installation, including earlier panels, paint droppers,
doors, and a laser relay floor. It also has 36 changed VMFs and 12 identical
VMFs. Version 22 has 6 absent, 45 changed, and 38 identical VMFs; version 24
has 2 absent, 30 changed, and 71 identical VMFs. These historical maps are
reference inputs; the installed Portal 2 content remains the runtime
authority. Nothing here is selected by Waf or `./play_p2`.

SHA-256 of sorted relative names, NUL separators, and each file's binary
SHA-256 digest:

| Version | Aggregate SHA-256 |
| --- | --- |
| `841_8` | `ad6619660ea7cfaec7f1fded2ddffd89876e14dc0971b96b23235b7523dba2cc` |
| `841_22` | `cdaf3a96972576cf8379f7d8a7b79cf955461d6747ac0ce6d54f3168858ccf1f` |
| `841_24` | `16bf9077cc78e6125f27ff951857f1c59aae2037c49d5716fec5d55bafad3589` |

Source metadata: [Steam2 depot 841 manifest](https://femtendo.github.io/steam2-catalog/#depot-841).
The repository's provenance and distribution warning applies.
