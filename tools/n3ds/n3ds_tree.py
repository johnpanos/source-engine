"""Where the 3DS client is built: the kiln tree of the portal2-3ds profile
(RFC 0027; quality/product_profiles/portal2-3ds.json). The one owner of the
path for the harness, the packager and the reports.

N3DS_FLAVOR selects the profile's flavor (default dev).
"""

import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROFILE = "portal2-3ds"
FLAVOR = os.environ.get("N3DS_FLAVOR", "dev")
TREE = ROOT / "out" / PROFILE / FLAVOR
ELF = TREE / "launcher_main" / "hl2_launcher"
