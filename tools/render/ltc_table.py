#!/usr/bin/env python3
"""The GGX linearly-transformed-cosine table (RFC 0016 K11, the area-light term).

    python3 tools/render/ltc_table.py write   regenerate public/render/pbr_ltc_table.h
    python3 tools/render/ltc_table.py check   regenerate and compare with the committed table

tools/render/ltc_fit/ltc_fit.cpp fits the table (Heitz et al. 2016) to the
RFC 0007 GGX lobe of public/render/pbr_brdf.h. This script builds it with the
host C++ compiler (-O2, no floating-point contraction, one thread), runs it and
writes the header. `check` fails when any entry differs from the committed one
by more than 1e-5 relative (libm differences between hosts move the last
digits; a changed BRDF or fitter moves far more), and when the committed header
is missing or malformed. It prints `CONFORMANCE <checks> <failures>`.
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "tools/render/ltc_fit/ltc_fit.cpp"
HEADER = ROOT / "public/render/pbr_ltc_table.h"
TOLERANCE = 1e-5


def fit(workdir):
    compiler = os.environ.get("CXX") or shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        sys.exit("ltc_table: no C++ compiler (set CXX)")
    binary = Path(workdir) / "ltc_fit"
    subprocess.run([compiler, "-std=c++20", "-O2", "-ffp-contract=off", "-o", str(binary),
                    str(SOURCE)], check=True)
    out = subprocess.run([str(binary)], check=True, capture_output=True, text=True).stdout
    lines = out.split("\n")
    size = int(lines[0])
    rows = [tuple(float(x) for x in line.split()) for line in lines[1:1 + size * size]]
    if len(rows) != size * size or any(len(r) != 4 for r in rows):
        sys.exit("ltc_table: the fitter printed a malformed table")
    return size, rows


def literal(value):
    text = "%.9g" % value
    if "." not in text and "e" not in text and "inf" not in text and "nan" not in text:
        text += ".0"
    return text + "f"


def header_text(size, rows):
    entries = "\n".join("    { %s, %s, %s, %s }," % tuple(literal(v) for v in row) for row in rows)
    return """//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Generated GGX linearly-transformed-cosine table (RFC 0016 K11, the
//          area-light term; Heitz et al. 2016) for the RFC 0007 lobe of
//          public/render/pbr_brdf.h. Run tools/render/ltc_table.py write to
//          regenerate; do not edit.
//
//          Entry [i * kLtcSize + j]: perceptual roughness i / (kLtcSize - 1),
//          sqrt(1 - N.V) j / (kLtcSize - 1). Each holds the inverse matrix's
//          four free entries, normalized by its middle one, as GLSL builds it:
//          mat3( vec3( m00, 0, m02 ), vec3( 0, 1, 0 ), vec3( m20, 0, m22 ) ),
//          in the frame whose x axis is the view projected on the surface.
//          The lobe's magnitude and Fresnel split are the split-sum table's
//          A and B (pbr_split_sum_table.h).
//===========================================================================//

#ifndef RENDER_PBR_LTC_TABLE_H
#define RENDER_PBR_LTC_TABLE_H

namespace render::pbr
{
struct LtcInverse
{
	float m00;
	float m02;
	float m20;
	float m22;
};

constexpr int kLtcSize = %d;
constexpr LtcInverse kLtcTable[kLtcSize * kLtcSize] = {
%s
};
} // namespace render::pbr

#endif // RENDER_PBR_LTC_TABLE_H
""" % (size, entries)


def committed():
    if not HEADER.exists():
        return None, None
    text = HEADER.read_text()
    match = re.search(r"kLtcSize = (\d+);", text)
    if not match:
        return None, None
    rows = [tuple(float(v.rstrip("f")) for v in m.groups())
            for m in re.finditer(r"\{ ([^,]+), ([^,]+), ([^,]+), ([^ ]+) \},", text)]
    return int(match.group(1)), rows


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["write", "check"])
    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as workdir:
        size, rows = fit(workdir)
    if args.command == "write":
        HEADER.write_text(header_text(size, rows))
        print("wrote %s (%d entries)" % (HEADER.relative_to(ROOT), len(rows)))
        return 0
    checks = failures = 0

    def check(condition, name, detail=""):
        nonlocal checks, failures
        checks += 1
        if not condition:
            failures += 1
            print("FAIL %s %s" % (name, detail))
    old_size, old_rows = committed()
    check(old_size == size, "ltc.committed-table-has-the-fitted-size", "%s vs %s" % (old_size, size))
    check(old_rows is not None and len(old_rows) == len(rows), "ltc.committed-table-is-complete")
    worst = 0.0
    if old_rows and len(old_rows) == len(rows):
        for new, old in zip(rows, old_rows):
            for a, b in zip(new, old):
                scale = max(abs(a), abs(b), 1e-3)
                worst = max(worst, abs(a - b) / scale)
    check(worst <= TOLERANCE, "ltc.committed-table-matches-the-fit", "worst %.3g" % worst)
    finite = all(abs(v) < 1e30 for row in rows for v in row)
    check(finite, "ltc.every-entry-is-finite")
    # Normal incidence is isotropic: the view-frame shear is zero there.
    check(all(abs(rows[i * size][1]) < 1e-6 and abs(rows[i * size][2]) < 1e-6
              for i in range(size)), "ltc.normal-incidence-is-isotropic")
    print("INFO worst relative difference %.3g" % worst)
    print("CONFORMANCE %d %d" % (checks, failures))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
