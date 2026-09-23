"""Validate the versioned supplemental Cycles directions for one SH L1 fit."""

import hashlib
import json
import math
import re


def load_plan(path):
    raw = path.read_bytes()
    document = json.loads(raw)
    if (document.get("schema") != 1 or
            document.get("id") != "rfc0008-sh-l1-supplemental-v1" or
            not isinstance(document.get("additional"), list) or
            not 1 <= len(document["additional"]) <= 32):
        raise ValueError("invalid SH L1 direction plan")
    ids = set()
    for entry in document["additional"]:
        if (set(entry) != {"id", "normal"} or
                not isinstance(entry["id"], str) or
                re.fullmatch(r"sh[0-9]{2}", entry["id"]) is None or
                entry["id"] in ids or
                not isinstance(entry["normal"], list) or
                len(entry["normal"]) != 3 or
                any(type(value) not in (int, float) or not math.isfinite(value)
                    for value in entry["normal"])):
            raise ValueError("invalid or duplicate SH L1 direction")
        ids.add(entry["id"])
        x, y, z = entry["normal"]
        if z < 0 or abs(x * x + y * y + z * z - 1.0) > 1e-6:
            raise ValueError("SH L1 direction must be unit length and upper hemisphere")
    return document, hashlib.sha256(raw).hexdigest()
