#!/usr/bin/env python3
"""Make a hash-valid BSP2 with one non-finite WMSH vertex for client rejection."""

import argparse
from pathlib import Path
import struct

from bsp2_reader import Bsp2File, fourcc, write_bsp2
from worldstage_mesh_compare import read_payload


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bsp2", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    source = Bsp2File(args.bsp2.read_bytes(), known=(fourcc("WMSH"),))
    if fourcc("WMSH") not in source.by_id:
        raise ValueError("source BSP2 lacks WMSH")
    lumps = []
    for entry in source.entries:
        payload = source.lump(entry)
        if entry["fourcc"] == fourcc("WMSH"):
            read_payload(payload)
            changed = bytearray(payload)
            offset = struct.unpack_from("<Q", changed, 56)[0]
            struct.pack_into("<I", changed, offset, 0x7FC00000)
            payload = bytes(changed)
            try:
                read_payload(payload)
            except ValueError:
                pass
            else:
                raise ValueError("independent reader accepted the seeded corruption")
        lumps.append((entry["fourcc"], entry["version"], entry["flags"],
                      entry["alignment"], payload))
    output = write_bsp2(source.revision, lumps)
    Bsp2File(output, known=(fourcc("WMSH"),))
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(output)
    print(args.out)


if __name__ == "__main__":
    main()
