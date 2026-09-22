"""Permutation and binary-format conformance, including deliberately wrong packs."""

from pathlib import Path
import copy
import json
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import shader_artifacts as artifacts


BYTECODE = struct.pack("<3I", 0xFFFF0200, 0, 0x0000FFFF)
PLAN = {
    "dynamic": [("FOG", 0, 1), ("LIGHTS", 0, 2)],
    "static": [("BUMPMAP", 0, 1), ("DETAIL", 0, 1)],
    "dynamic_count": 6, "total": 24, "flags": 0, "centroid": 4, "skip": "0",
}

WORKLOAD = {
    "schema": "source-shader-workload/v1", "id": "fixture-v1", "coverage": "observed-static-sets",
    "observations": [{"map": "fixture_map", "console_sha256": "a" * 64}],
    "shaders": [{"name": "fixture_ps20b", "static_bases": [0, 12], "selector_sha256": "b" * 64}],
}


def independent_unpack(data):
    """Test reader written from the VCS v6 wire format, not the packer's helpers."""
    version, total, dynamic_count, flags, centroid, static_count, crc = struct.unpack_from("<7I", data)
    if version != 6 or not dynamic_count or static_count < 2:
        raise ValueError("invalid header")
    rows = [struct.unpack_from("<II", data, 28 + i * 8) for i in range(static_count)]
    if rows[-1] != (0xFFFFFFFF, len(data)):
        raise ValueError("missing sentinel or truncated file")
    if [s for s, _ in rows] != sorted(set(s for s, _ in rows)):
        raise ValueError("noncanonical dictionary")
    duplicate_count, = struct.unpack_from("<I", data, 28 + static_count * 8)
    if duplicate_count or rows[0][1] != 32 + static_count * 8:
        raise ValueError("unexpected dictionary layout")
    result = {}
    for (static_id, start), (_, stop) in zip(rows, rows[1:]):
        pos = start
        combos = {}
        while pos < stop:
            length, = struct.unpack_from("<I", data, pos)
            pos += 4
            if length == 0xFFFFFFFF:
                if pos != stop:
                    raise ValueError("trailing static bytes")
                break
            if length >> 30 != 2 or not (0 < (length & 0x3FFFFFFF) < 131072):
                raise ValueError("bad uncompressed block")
            end = pos + (length & 0x3FFFFFFF)
            if end > stop - 4:
                raise ValueError("block exceeds static record")
            while pos < end:
                dynamic_id, size = struct.unpack_from("<II", data, pos)
                pos += 8
                if dynamic_id >= dynamic_count or dynamic_id in combos or pos + size > end:
                    raise ValueError("bad dynamic record")
                combos[dynamic_id] = data[pos:pos + size]
                pos += size
            if pos != end:
                raise ValueError("dynamic records do not cover block")
        else:
            raise ValueError("missing static terminator")
        result[static_id] = combos
    return (total, dynamic_count, flags, centroid, crc), result


class PermutationTests(unittest.TestCase):
    def test_first_declared_macro_is_least_significant(self):
        self.assertEqual({"FOG": 1, "LIGHTS": 2}, artifacts.decode_index(5, PLAN["dynamic"]))
        self.assertEqual({"BUMPMAP": 0, "DETAIL": 1}, artifacts.decode_index(2, PLAN["static"]))

    def test_nonzero_range_decoding(self):
        self.assertEqual({"A": 2, "B": 4}, artifacts.decode_index(3, [("A", 1, 2), ("B", 3, 4)]))

    def test_out_of_range_indices_fail(self):
        for index in (-1, 6):
            with self.assertRaises(ValueError):
                artifacts.decode_index(index, PLAN["dynamic"])

    def test_static_bases_use_source_dynamic_count_not_installed_vcs_count(self):
        text = 'psh 0x38: static combo: 12 dynamic combos: 288 refcount: 7 "fixture_ps20b"'
        demand = artifacts.demands_from_log(text)
        self.assertEqual({"fixture_ps20b": {12}}, demand)
        combos = artifacts.valid_combos(PLAN, demand["fixture_ps20b"])
        self.assertEqual({2}, {s for s, _, _ in combos})
        self.assertEqual(set(range(6)), {d for _, d, _ in combos})

    def test_missing_and_duplicate_demand_handling(self):
        self.assertEqual({}, artifacts.demands_from_log("map loaded"))
        line = "Shader 'shaders/fxc/fixture_ps20b.vcs' - Couldn't load combo 12 of shader (dyn=288)\n"
        self.assertEqual({"fixture_ps20b": {12}}, artifacts.demands_from_log(line + line))

    def test_skipped_permutations_keep_original_dynamic_indices(self):
        combos = artifacts.valid_combos(dict(PLAN, skip="$FOG || $LIGHTS == 1"), [12])
        self.assertEqual([0, 4], [d for _, d, _ in combos])

    def test_skip_preserves_undefined_variable_semantics(self):
        combos = artifacts.valid_combos(dict(PLAN, skip="defined $HDRTYPE && !$HDRTYPE"), [0])
        self.assertEqual(6, len(combos))

    def test_skip_last_zero_value_is_false_not_newline_string(self):
        combos = artifacts.valid_combos(dict(PLAN, skip="$LIGHTS"), [0])
        self.assertEqual([0, 1], [d for _, d, _ in combos])

    def test_empty_all_skipped_and_misaligned_demands_fail(self):
        for bases, skip in (([], "0"), ([0], "1"), ([7], "0"), ([24], "0")):
            with self.assertRaises(ValueError):
                artifacts.valid_combos(dict(PLAN, skip=skip), bases)

    def test_path_escape_and_unknown_shader_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            for name in ("../fixture", "unknown_ps20b"):
                with self.assertRaises(ValueError):
                    artifacts.resolve_source(Path(directory), name)

    def test_checked_in_lightmap_selector_rejects_wrong_dynamic_count(self):
        directory = artifacts.ROOT / "materialsystem/stdshaders"
        path = artifacts.ROOT / "materialsystem/stdshaders/fxctmp9/lightmappedgeneric_ps20b.inc"
        header = path.read_text()
        with tempfile.TemporaryDirectory() as temporary:
            work = Path(temporary)
            for source in directory.iterdir():
                if source.is_file():
                    (work / source.name).symlink_to(source)
            subprocess.run(["perl", str(artifacts.ROOT / "devtools/bin/fxc_prep.pl"), "-nocpp",
                            "lightmappedgeneric_ps2x.fxc-----lightmappedgeneric_ps20b"],
                           cwd=work, capture_output=True, check=True, timeout=30)
            plan = artifacts.parse_plan((work / "filelistgen.txt").read_text())
        self.assertEqual(96, plan["dynamic_count"])
        artifacts.verify_selector(header, "lightmappedgeneric_ps20b", plan)
        fake = dict(plan, dynamic_count=288)
        with self.assertRaises(ValueError):
            artifacts.verify_selector(header, "lightmappedgeneric_ps20b", fake)

    def test_product_profile_references_verified_compiler_authority(self):
        path, profile = artifacts.load_compiler_profile()
        self.assertTrue(path.is_file())
        self.assertEqual("source-shader-compiler/v1", profile["schema"])
        self.assertEqual("observed-static-sets", profile["coverage"])

    def test_wrong_compiler_binary_rejected(self):
        with patch.object(artifacts, "digest", return_value="0" * 64):
            with self.assertRaisesRegex(ValueError, "differs from pinned"):
                artifacts.load_compiler_profile()


class PackingTests(unittest.TestCase):
    def test_independent_reader_recovers_exact_shader_ids_and_bytes(self):
        shaders = {3: {5: BYTECODE, 0: BYTECODE}, 1: {2: BYTECODE}}
        header, recovered = independent_unpack(artifacts.pack_vcs(PLAN, shaders, 0x12345678))
        self.assertEqual((24, 6, 0, 4, 0x12345678), header)
        self.assertEqual(shaders, recovered)

    def test_wire_encoding_matches_handwritten_one_shader_fixture(self):
        data = artifacts.pack_vcs(PLAN, {2: {4: BYTECODE}}, 7)
        expected = struct.pack("<7I", 6, 24, 6, 0, 4, 2, 7)
        expected += struct.pack("<5I", 2, 48, 0xFFFFFFFF, 76, 0)
        expected += struct.pack("<3I", 0x80000014, 4, 12) + BYTECODE + b"\xff" * 4
        self.assertEqual(expected, data)

    def test_block_splitting_preserves_every_record(self):
        large = BYTECODE[:4] + b"\x00" * 68000 + BYTECODE[-4:]
        shaders = {0: {0: large, 1: large, 2: large}}
        _, recovered = independent_unpack(artifacts.pack_vcs(PLAN, shaders, 0))
        self.assertEqual(shaders, recovered)

    def test_empty_static_and_out_of_range_records_fail(self):
        for shaders in ({}, {0: {}}, {4: {0: BYTECODE}}, {0: {6: BYTECODE}}):
            with self.assertRaises(ValueError):
                artifacts.pack_vcs(PLAN, shaders, 0)

    def test_truncated_or_nond3d_compiler_output_fails(self):
        for data in (b"", b"success", BYTECODE[:-1], b"\x00" * 12, BYTECODE[:-4] + b"\x00" * 4):
            with self.assertRaises(ValueError):
                artifacts.pack_vcs(PLAN, {0: {0: data}}, 0)

    def test_reader_detects_seeded_dynamic_id_corruption(self):
        data = bytearray(artifacts.pack_vcs(PLAN, {0: {0: BYTECODE}}, 0))
        struct.pack_into("<I", data, 52, 6)
        with self.assertRaises(ValueError):
            independent_unpack(data)

    def test_reader_detects_seeded_missing_terminator(self):
        data = bytearray(artifacts.pack_vcs(PLAN, {0: {0: BYTECODE}}, 0))
        data[-4:] = b"\x00" * 4
        with self.assertRaises(ValueError):
            independent_unpack(data)

    def test_zero_exit_without_bytecode_is_not_success(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            with patch.object(artifacts.subprocess, "run", return_value=subprocess.CompletedProcess([], 0, "", "")):
                with self.assertRaises(RuntimeError):
                    artifacts.compile_shader(path / "fxc.exe", path / "source.fxc", [], {}, path / "result.o", 1)


class WorkloadTests(unittest.TestCase):
    def test_committed_default_workload_reproduces_both_captured_chambers(self):
        profile = artifacts.product_profile.load_profile()
        path = artifacts.ROOT / profile["intent"]["shader_pipeline"]["default_workload"]
        document, demands, selectors = artifacts.load_workload(path)
        self.assertEqual({"testchmb_a_00", "testchmb_a_01"}, {o["map"] for o in document["observations"]})
        self.assertEqual(60, len(demands))
        self.assertEqual(121, sum(map(len, demands.values())))
        headers = {p.name.lower(): p for p in (artifacts.ROOT / "materialsystem/stdshaders/fxctmp9").glob("*.inc")}
        for name, expected in selectors.items():
            self.assertEqual(expected, artifacts.digest(headers[name.lower() + ".inc"].read_bytes()))

    def test_fixture_preserves_indices_and_selector_provenance(self):
        demands, selectors = artifacts.parse_workload(WORKLOAD)
        self.assertEqual({"fixture_ps20b": {0, 12}}, demands)
        self.assertEqual({"fixture_ps20b": "b" * 64}, selectors)

    def test_missing_and_empty_fixture_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "missing.json"
            with self.assertRaises(OSError):
                artifacts.load_workload(path)
            path.write_text("")
            with self.assertRaises(ValueError):
                artifacts.load_workload(path)

    def test_schema_and_empty_requests_fail(self):
        for document in ({}, [], dict(WORKLOAD, schema="future/v2"), dict(WORKLOAD, shaders=[]),
                         dict(WORKLOAD, shaders=None), dict(WORKLOAD, coverage="complete-library")):
            with self.assertRaises(ValueError):
                artifacts.parse_workload(document)

    def test_duplicate_shader_names_are_rejected_case_insensitively(self):
        document = copy.deepcopy(WORKLOAD)
        duplicate = dict(document["shaders"][0], name="FIXTURE_PS20B")
        document["shaders"].append(duplicate)
        with self.assertRaisesRegex(ValueError, "duplicate"):
            artifacts.parse_workload(document)

    def test_duplicate_indices_fail(self):
        document = copy.deepcopy(WORKLOAD)
        document["shaders"][0]["static_bases"] = [0, 0]
        with self.assertRaisesRegex(ValueError, "duplicate"):
            artifacts.parse_workload(document)

    def test_invalid_names_and_indices_fail(self):
        for name in (None, "", "../escape", "fixture.vcs", "with space"):
            document = copy.deepcopy(WORKLOAD)
            document["shaders"][0]["name"] = name
            with self.assertRaises(ValueError):
                artifacts.parse_workload(document)
        for indices in (None, [], [-1], [True], [0.0], ["0"], [2147483648]):
            document = copy.deepcopy(WORKLOAD)
            document["shaders"][0]["static_bases"] = indices
            with self.assertRaises(ValueError):
                artifacts.parse_workload(document)

    def test_missing_or_invalid_schema_and_capture_hashes_fail(self):
        for value in (None, "", "not-a-sha", "c" * 63):
            document = copy.deepcopy(WORKLOAD)
            document["shaders"][0]["selector_sha256"] = value
            with self.assertRaises(ValueError):
                artifacts.parse_workload(document)
        with self.assertRaises(ValueError):
            artifacts.parse_workload(dict(WORKLOAD, observations=[]))

    def test_duplicate_json_fields_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "duplicate.json"
            path.write_text('{"schema":"source-shader-workload/v1","schema":"source-shader-workload/v1"}')
            with self.assertRaisesRegex(ValueError, "duplicate"):
                artifacts.load_workload(path)


if __name__ == "__main__":
    unittest.main()
