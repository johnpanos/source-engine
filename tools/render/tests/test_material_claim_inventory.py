"""Negative fixtures for the static material requirements report."""

import sys
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import material_claim_inventory as audit


def row(claim=None, gap=None, **inputs):
    result = {"scene_inputs": inputs}
    result.update({"claim": claim} if claim else {"gap": gap})
    return result


class ClaimInventoryTest(unittest.TestCase):
    def test_structural_failure_precedes_proxy_uncertainty(self):
        mesh = [row(gap="shader SpriteCard maps to no family the model draws") for _ in range(4)]
        world = [row(gap="shader SpriteCard maps to no family the model draws") for _ in range(2)]
        status, candidates = audit.classify("materials/particle.vmt", {"proxies": ["Animated"]},
                                            mesh, world)
        self.assertEqual(status, "unsupported")
        self.assertEqual(candidates, [])

    def test_scene_inputs_are_minimal_and_explicit(self):
        mesh = [row(gap="need probes", native_reflection_probes=False,
                    linear_scene_color=False),
                row(gap="need probes", native_reflection_probes=False,
                    linear_scene_color=True),
                row(claim="alpha", native_reflection_probes=True,
                    linear_scene_color=False),
                row(claim="alpha", native_reflection_probes=True,
                    linear_scene_color=True)]
        world = [row(gap="no world point", world_stage=False),
                 row(gap="no world point", world_stage=True)]
        status, candidates = audit.classify("materials/models/glass.vmt", {"proxies": []},
                                            mesh, world)
        self.assertEqual(status, "supported_with_requirements")
        self.assertEqual(candidates, [{"geometry": "model_mesh", "pass": "blended",
                                       "blend": "alpha",
                                       "scene_inputs": {"native_reflection_probes": True}}])

    def test_proxy_is_dynamic_even_when_baseline_claims(self):
        mesh = [row(claim="opaque", native_reflection_probes=False,
                    linear_scene_color=False)]
        status, candidates = audit.classify("materials/models/animated.vmt",
                                            {"proxies": ["TextureScroll"]}, mesh, [])
        self.assertEqual(status, "dynamically_unresolved")
        self.assertTrue(candidates)

    def test_world_and_mesh_geometry_are_distinct(self):
        mesh = [row(claim="opaque", native_reflection_probes=False,
                    linear_scene_color=False)]
        world = [row(gap="no world point", world_stage=False),
                 row(gap="no world point", world_stage=True)]
        status, candidates = audit.classify("materials/effects/glow.vmt",
                                            {"proxies": []}, mesh, world)
        self.assertEqual(status, "supported_with_requirements")
        self.assertEqual(candidates[0]["geometry"], "model_mesh")

    def test_bad_protocol_and_blend_fail_closed(self):
        with self.assertRaisesRegex(ValueError, "15 fields"):
            audit.result_rows(["C", "0"] * 3)
        with self.assertRaisesRegex(ValueError, "unknown.*blend"):
            audit.result_rows(["C", "99"] * 6 + ["", "", "unlit"])

    def test_vmt_profile_drift_fails_closed(self):
        profile = audit.profile_record(audit.DEFAULT_PROFILES["portal2"], "portal2")
        good = ("CLAIM-BATCH/3\tdx=95\tps20b=1\thdr=1\tsrgb=1\tgpu=3\tlowfill=0"
                "\tsymbols=WIN32,LINUX,POSIX")
        audit.verify_claim_profile(good, profile)
        with self.assertRaisesRegex(ValueError, "profile differs"):
            audit.verify_claim_profile(good.replace("gpu=3", "gpu=2"), profile)

    def test_game_profile_is_explicit(self):
        portal = audit.profile_record(audit.DEFAULT_PROFILES["portal"], "portal")
        self.assertFalse(portal["core_only_declared"])
        with self.assertRaisesRegex(ValueError, "another game"):
            audit.profile_record(audit.DEFAULT_PROFILES["portal"], "portal2")

    def test_special_pass_is_named_by_imported_family(self):
        mesh = [row(claim="opaque", native_reflection_probes=False,
                    linear_scene_color=False)]
        status, candidates = audit.classify("materials/models/portal.vmt",
                                            {"proxies": []}, mesh, [], "portal-mask")
        self.assertEqual(status, "supported_with_requirements")
        self.assertEqual(candidates[0]["pass"], "portal_mask")

    def test_blended_phase_remains_a_requirement(self):
        mesh = [row(claim="alpha", native_reflection_probes=False,
                    linear_scene_color=False)]
        status, candidates = audit.classify("materials/models/glass.vmt",
                                            {"proxies": []}, mesh, [], "refract")
        self.assertEqual(status, "supported_with_requirements")
        self.assertEqual(candidates[0]["pass"], "blended")

    def test_missing_include_is_not_counted_as_supported(self):
        game = mock.Mock()
        game.vmts.return_value = ["materials/models/good.vmt", "materials/models/bad.vmt"]
        materials = {
            "materials/models/good.vmt": {"shader_raw": "VertexLitGeneric", "vars": {},
                                           "proxies": []},
            "materials/models/bad.vmt": {"error": "missing include x.vmt"},
        }
        # Four mesh claims, two world gaps, two empty diagnostics and the family.
        answer = "\t".join(["C", "0"] * 4 + ["G", "no world", "G", "no world",
                                       "", "", "vertexlit"]) + "\n"
        header = ("CLAIM-BATCH/3\tdx=95\tps20b=1\thdr=1\tsrgb=1\tgpu=3\tlowfill=0"
                  "\tsymbols=WIN32,LINUX,POSIX\n")
        process = SimpleNamespace(returncode=0, stdout=(header + answer).encode(), stderr=b"")
        with mock.patch.object(audit.vmt_corpus, "Game", return_value=game), \
             mock.patch.object(audit.material_inventory, "load_material",
                               side_effect=lambda path, _: materials[path]), \
             mock.patch.object(audit.subprocess, "run", return_value=process):
            report = audit.collect("portal2", Path("/tmp/build/render/lab/render_lab"), "models")
        self.assertEqual(report["status_counts"], {"statically_supported": 1,
                                                    "unsupported": 1})
        self.assertEqual(report["by_shader"]["<parse error>"]["unsupported"], 1)
        self.assertEqual(report["refusal_features"]["vmt:missing_include"]
                         ["refused_materials"], 1)

    def test_refusal_features_count_materials_not_repeated_claims(self):
        shader_gap = "shader SpriteCard maps to no family the model draws (legacy)"
        parameters = "the family does not draw $spriteorigin and $vertexalpha"
        materials = {
            "materials/particle.vmt": {
                "status": "unsupported", "gap": shader_gap,
                "mesh_claims": [row(gap=shader_gap)] * 4,
                "world_claims": [row(gap=parameters)] * 2,
                "unmapped_keys": ["$spriteorigin", "$spriteorigin"],
            },
            "materials/other.vmt": {
                "status": "unsupported", "gap": parameters,
                "mesh_claims": [row(gap=shader_gap)], "world_claims": []},
            "materials/supported.vmt": {
                "status": "statically_supported", "gap": shader_gap,
            },
        }
        groups = audit.summarize_refusals(materials)
        self.assertEqual(groups["shader_family:spritecard"]["refused_materials"], 2)
        self.assertEqual(groups["shader_family:spritecard"]["first_refusals"], 1)
        self.assertEqual(groups["parameter:$spriteorigin"]["refused_materials"], 2)
        self.assertEqual(groups["parameter:$spriteorigin"]["unmapped_materials"], 1)
        self.assertEqual(groups["parameter:$vertexalpha"]["refused_materials"], 2)
        self.assertEqual(materials["materials/particle.vmt"]["mesh_claims"],
                         [row(gap=shader_gap)] * 4)

    def test_refusal_feature_names_keep_view_resources_distinct(self):
        self.assertEqual(audit.refusal_features(
            "the model does not bind the per-view texture env_cubemap ($envmap)"),
            ("per_view_texture:env_cubemap",))
        self.assertEqual(audit.refusal_features("family vertexlit has no program yet"),
                         ("family_program:vertexlit",))
        self.assertEqual(audit.refusal_features("family lightmapped has no mesh point yet"),
                         ("mesh_point:lightmapped",))
        self.assertEqual(audit.refusal_features(
            "shader Spritecard maps to no family the model draws (legacy)"),
            ("shader_family:spritecard",))


if __name__ == "__main__":
    unittest.main()
