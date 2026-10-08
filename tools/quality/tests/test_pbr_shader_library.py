# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Source guard for the PBR stages (RFC 0007 R47).

The BRDF has one GLSL copy, render/shaders/common/pbr_brdf.glsl, which the
render.material surface program includes; no stage defines its own GGX, Smith
or Schlick. Includes resolve against the including file, as glslc's do. The
render.pbr-brdf.glsl GPU suite compares that copy with public/render/pbr_brdf.h.
(The native backend's PBR stages were deleted with its draw path, RFC 0016 K9.)

The negative cases feed the check a stage that breaks it.
"""

import pathlib
import posixpath
import re
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
LIBRARY = "pbr_brdf.glsl"  # its file name; render/shaders/common/ holds it
PBR_STAGES = ("render/material/families/surface.frag",)
# A BRDF term defined outside the library: a GGX denominator, or a function
# whose name says it is one.
BRDF_DEFINITION = re.compile(
    r"alphaSquared\s*-\s*1\.0|"
    r"^\s*(?:float|vec3)\s+\w*(?:Ggx|GGX|Smith|Schlick|Fresnel)\w*\s*\(",
    re.MULTILINE)
INCLUDE = re.compile(r'^\s*#include\s+"([^"]+)"', re.MULTILINE)


def is_library(name):
    return pathlib.PurePosixPath(name).name == LIBRARY


def includes(name, read, seen=None):
    """Every file `name` includes, transitively, as paths resolved against
    the including file."""
    seen = set() if seen is None else seen
    for text in INCLUDE.findall(read(name)):
        included = posixpath.normpath(posixpath.join(posixpath.dirname(name), text))
        if included not in seen:
            seen.add(included)
            includes(included, read, seen)
    return seen


def library_problems(stages, read):
    problems = []
    for stage in stages:
        if not any(is_library(name) for name in includes(stage, read)):
            problems.append("%s does not include %s" % (stage, LIBRARY))
    for name in sorted({*stages, *(i for s in stages for i in includes(s, read))}):
        if not is_library(name) and BRDF_DEFINITION.search(read(name)):
            problems.append("%s defines a BRDF term outside %s" % (name, LIBRARY))
    return problems


def read_shader(name):
    return (ROOT / name).read_text()


class PbrShaderLibraryTest(unittest.TestCase):
    def test_every_stage_uses_the_one_brdf(self):
        self.assertEqual(library_problems(PBR_STAGES, read_shader), [])

    def test_a_private_ggx_is_rejected(self):
        files = {
            "stage.frag": '#include "pbr_brdf.glsl"\n'
                          "float d = a / ( n * n * ( alphaSquared - 1.0 ) + 1.0 );\n",
            LIBRARY: "",
        }
        self.assertEqual(library_problems(("stage.frag",), files.__getitem__),
                         ["stage.frag defines a BRDF term outside pbr_brdf.glsl"])

    def test_a_private_fresnel_function_is_rejected(self):
        files = {"stage.frag": '#include "helper.glsl"\n',
                 "helper.glsl": '#include "pbr_brdf.glsl"\nfloat SchlickF( float f0 )\n',
                 LIBRARY: ""}
        self.assertEqual(library_problems(("stage.frag",), files.__getitem__),
                         ["helper.glsl defines a BRDF term outside pbr_brdf.glsl"])

    def test_a_stage_without_the_library_is_rejected(self):
        files = {"stage.frag": "void main() {}\n"}
        self.assertEqual(library_problems(("stage.frag",), files.__getitem__),
                         ["stage.frag does not include pbr_brdf.glsl"])


if __name__ == "__main__":
    unittest.main()
