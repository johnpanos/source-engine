# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Source guards for the native PBR stages (RFC 0007 R47 / RFC 0001 R29).

* The BRDF has one GLSL copy, shaders/pbr_brdf.glsl, which every PBR stage
  includes (directly or through world_pbr_probe.glsl); no stage defines its
  own GGX, Smith or Schlick. The render.pbr-brdf.glsl GPU suite compares that
  copy with public/render/pbr_brdf.h.
* The PBR and GI stages bind their descriptors grouped by update frequency
  (vulkan_descriptor_groups.h): no set above 2, so they fit the four sets
  every Vulkan device binds.

The negative cases feed each check a stage that breaks it.
"""

import pathlib
import re
import unittest

SHADERS = pathlib.Path(__file__).resolve().parents[3] / "materialsystem/shaderapivulkan/shaders"
LIBRARY = "pbr_brdf.glsl"
PBR_STAGES = ("pbr_direct.frag", "world_pbr.frag", "world_pbr_glass.frag", "model_pbr.frag")
# Grouped stages: frame (0), material (1), constants (2).
GROUPED_STAGES = ("world_pbr.frag", "world_pbr_glass.frag", "model_pbr.frag")
MAX_SET = 2
# A BRDF term defined outside the library: a GGX denominator, or a function
# whose name says it is one.
BRDF_DEFINITION = re.compile(
    r"alphaSquared\s*-\s*1\.0|"
    r"^\s*(?:float|vec3)\s+\w*(?:Ggx|GGX|Smith|Schlick|Fresnel)\w*\s*\(",
    re.MULTILINE)
INCLUDE = re.compile(r'^\s*#include\s+"([^"]+)"', re.MULTILINE)
SET = re.compile(r"layout\(\s*set\s*=\s*(\d+)")


def includes(name, read, seen=None):
    """Every file `name` includes, transitively."""
    seen = set() if seen is None else seen
    for included in INCLUDE.findall(read(name)):
        if included not in seen:
            seen.add(included)
            includes(included, read, seen)
    return seen


def library_problems(stages, read):
    problems = []
    for stage in stages:
        if LIBRARY not in includes(stage, read):
            problems.append("%s does not include %s" % (stage, LIBRARY))
    for name in sorted({*stages, *(i for s in stages for i in includes(s, read))}):
        if name != LIBRARY and BRDF_DEFINITION.search(read(name)):
            problems.append("%s defines a BRDF term outside %s" % (name, LIBRARY))
    return problems


def set_problems(stages, read):
    problems = []
    for stage in stages:
        for name in [stage, *sorted(includes(stage, read))]:
            for number in SET.findall(read(name)):
                if int(number) > MAX_SET:
                    problems.append("%s (for %s) binds set %s" % (name, stage, number))
    return problems


def read_shader(name):
    return (SHADERS / name).read_text()


class PbrShaderLibraryTest(unittest.TestCase):
    def test_every_stage_uses_the_one_brdf(self):
        self.assertEqual(library_problems(PBR_STAGES, read_shader), [])

    def test_grouped_stages_fit_four_sets(self):
        self.assertEqual(set_problems(GROUPED_STAGES, read_shader), [])

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

    def test_an_ungrouped_set_is_rejected(self):
        files = {"stage.frag": '#include "volume.glsl"\n'
                               "layout( set = 1, binding = 0 ) uniform sampler2D a;\n",
                 "volume.glsl": "layout( set = 8, binding = 0 ) uniform sampler2D b;\n"}
        self.assertEqual(set_problems(("stage.frag",), files.__getitem__),
                         ["volume.glsl (for stage.frag) binds set 8"])


if __name__ == "__main__":
    unittest.main()
