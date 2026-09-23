#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Make a labeled v19-header derivative of a pinned Portal v20 map.

This probes the native v19 container path. It is not an authored v19 map: every
payload byte remains v20 content, so it cannot certify v19 content semantics.
"""

import argparse
import hashlib
import json
from pathlib import Path
import sys

import bsp2_reader


SOURCE_NAME = "testchmb_a_01.bsp"
OUTPUT_NAME = "rfc0008_v19_testchmb.bsp"


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def derive(portal_maps, inventory_path, output):
    portal_maps = Path(portal_maps).resolve()
    output = Path(output).resolve()
    if output == portal_maps or portal_maps in output.parents:
        raise ValueError("derivative output must be outside the source map directory")
    expected, inventory = bsp2_reader.load_expected_inventory(inventory_path)
    if SOURCE_NAME not in expected:
        raise ValueError("source map is absent from the pinned inventory")
    source = portal_maps / SOURCE_NAME
    data = source.read_bytes()
    if data[:4] != b"VBSP" or int.from_bytes(data[4:8], "little", signed=True) != 20:
        raise ValueError("source must be a VBSP v20 map")
    if (sha256(data), 20) != expected[SOURCE_NAME]:
        raise ValueError("source map differs from the pinned inventory")
    derivative = data[:4] + (19).to_bytes(4, "little") + data[8:]
    output.mkdir(parents=True, exist_ok=True)
    map_path = output / OUTPUT_NAME
    map_path.write_bytes(derivative)
    derived_inventory = {"schema": "bsp2-corpus-inventory/v1", "maps": [
        {"name": OUTPUT_NAME, "version": 19, "sha256": sha256(derivative)}]}
    inventory_output = output / "inventory.json"
    inventory_output.write_text(json.dumps(derived_inventory, indent=2) + "\n")
    evidence = {"schema": "bsp2-v19-derivative-evidence/v1", "status": "pass",
                "source": str(source), "source_sha256": sha256(data),
                "source_inventory": inventory,
                "derivative": str(map_path), "derivative_sha256": sha256(derivative),
                "changed_bytes": [index for index in range(8) if data[index] != derivative[index]],
                "limitation": "v20 payload with a v19 header; not authored v19 content"}
    (output / "derivation.json").write_text(json.dumps(evidence, indent=2) + "\n")
    return evidence


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--portal-maps", required=True)
    parser.add_argument("--inventory", default="quality/fixtures/bsp2-corpus-v20.json")
    parser.add_argument("--out", required=True)
    args = parser.parse_args(argv)
    try:
        evidence = derive(args.portal_maps, args.inventory, args.out)
    except (OSError, ValueError) as error:
        print("BSP2 v19 derivative: fail (%s)" % error)
        return 1
    print("BSP2 v19 derivative: pass (%s)" % evidence["derivative"])
    return 0


if __name__ == "__main__":
    sys.exit(main())
