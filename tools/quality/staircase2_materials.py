"""Solid base colors shared by the staircase2 Cycles and Source adapters.

The PBRT scene owns the linear reflectances. Source VTF base-color bytes are
sRGB-encoded values for those same reflectances.
"""

import math
import re


def _material_body(source, name):
    match = re.search(r'^MakeNamedMaterial\s+"' + re.escape(name) +
                      r'"\s*\n(.*?)(?=^MakeNamedMaterial|^NamedMaterial|\Z)',
                      source, re.M | re.S)
    if not match:
        raise ValueError("PBRT material is missing: " + name)
    return match.group(1)


def _rgb(body, parameter):
    match = re.search(r'"rgb ' + re.escape(parameter) + r'"\s*\[([^\]]+)\]', body)
    if not match:
        raise ValueError("PBRT RGB parameter is missing: " + parameter)
    values = tuple(float(value) for value in match.group(1).split())
    if len(values) != 3 or not all(math.isfinite(value) and value >= 0 for value in values):
        raise ValueError("PBRT RGB parameter is invalid: " + parameter)
    return values


def solid_linear_base_colors(source):
    wall = _rgb(_material_body(source, "Wall"), "reflectance")
    colors = {"wall": wall}
    for name in ("Metal", "Chrome"):
        body = _material_body(source, name)
        eta = _rgb(body, "eta")
        extinction = _rgb(body, "k")
        colors[name.lower()] = tuple(((e - 1.0) ** 2 + k ** 2) /
                                     ((e + 1.0) ** 2 + k ** 2)
                                     for e, k in zip(eta, extinction))
    if not all(value <= 1.0 for color in colors.values() for value in color):
        raise ValueError("PBRT solid base reflectance exceeds one")
    return colors


def linear_to_srgb_byte(value):
    if not math.isfinite(value) or not 0.0 <= value <= 1.0:
        raise ValueError("linear base color is outside [0, 1]")
    encoded = value * 12.92 if value <= 0.0031308 else 1.055 * value ** (1.0 / 2.4) - 0.055
    return round(encoded * 255.0)
