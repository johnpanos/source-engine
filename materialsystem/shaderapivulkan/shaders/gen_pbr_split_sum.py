#!/usr/bin/env python3
"""Generate the deterministic GGX split-sum table for CPU and GPU upload.

The table stores A and B such that directional specular albedo for Schlick F0
is F0*A+B. Importance samples use the same GGX alpha=roughness^2 and correlated
Smith visibility as render/pbr_brdf.h. No texture encoding is applied.
"""

import argparse
import math
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
SIZE = 32
SAMPLES = 4096


def radical_inverse(bits):
    bits = ((bits << 16) | (bits >> 16)) & 0xffffffff
    bits = (((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1))
    bits = (((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2))
    bits = (((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4))
    bits = (((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8))
    return bits / 4294967296.0


def integrate(normal_dot_view, roughness):
    alpha = roughness * roughness
    alpha_squared = alpha * alpha
    view_x = math.sqrt(1.0 - normal_dot_view * normal_dot_view)
    lambda_view = math.sqrt(alpha_squared +
                            (1.0 - alpha_squared) * normal_dot_view * normal_dot_view)
    a = 0.0
    b = 0.0
    for sample in range(SAMPLES):
        u = (sample + 0.5) / SAMPLES
        phi = 2.0 * math.pi * radical_inverse(sample)
        half_z = math.sqrt((1.0 - u) / (1.0 + (alpha_squared - 1.0) * u))
        half_x = math.sqrt(max(0.0, 1.0 - half_z * half_z)) * math.cos(phi)
        view_dot_half = view_x * half_x + normal_dot_view * half_z
        if view_dot_half <= 0.0:
            continue
        light_z = 2.0 * view_dot_half * half_z - normal_dot_view
        if light_z <= 0.0:
            continue
        lambda_light = math.sqrt(alpha_squared +
                                 (1.0 - alpha_squared) * light_z * light_z)
        denominator = normal_dot_view * lambda_light + light_z * lambda_view
        visibility = 2.0 * light_z * view_dot_half / (half_z * denominator)
        fresnel_grazing = (1.0 - view_dot_half) ** 5
        a += (1.0 - fresnel_grazing) * visibility
        b += fresnel_grazing * visibility
    return a / SAMPLES, b / SAMPLES


def values():
    return [integrate((x + 0.5) / SIZE, max(0.02, (y + 0.5) / SIZE))
            for y in range(SIZE) for x in range(SIZE)]


def literal(value, suffix=""):
    text = "%.9g" % value
    if "." not in text and "e" not in text:
        text += ".0"
    return text + suffix


def cpp_text(table):
    entries = "\n".join("    { %s, %s }," % (literal(a, "f"), literal(b, "f"))
                        for a, b in table)
    return """//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Generated RFC 0007 GGX split-sum BRDF table. Run
//          shaders/gen_pbr_split_sum.py to regenerate; do not edit.
//===========================================================================//

#ifndef RENDER_PBR_SPLIT_SUM_TABLE_H
#define RENDER_PBR_SPLIT_SUM_TABLE_H

namespace render::pbr
{
struct SplitSumCoefficients
{
\tfloat a;
\tfloat b;
};

constexpr int kSplitSumSize = %d;
constexpr int kSplitSumSamples = %d;
constexpr SplitSumCoefficients kSplitSumTable[kSplitSumSize * kSplitSumSize] = {
%s
};
} // namespace render::pbr

#endif // RENDER_PBR_SPLIT_SUM_TABLE_H
""" % (SIZE, SAMPLES, entries)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true",
                        help="fail when generated artifacts differ")
    args = parser.parse_args()
    table = values()
    artifacts = ((ROOT / "public/render/pbr_split_sum_table.h", cpp_text(table)),)
    for path, content in artifacts:
        if args.check:
            if not path.is_file() or path.read_text() != content:
                raise SystemExit("outdated split-sum artifact: %s" % path)
        else:
            path.write_text(content)


if __name__ == "__main__":
    main()
