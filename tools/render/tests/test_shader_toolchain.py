"""Seeded fixtures for tools/render/shader_toolchain.py (RFC 0016 K0,
shader.toolchain-pin): a compiler reporting another version and a one-byte change
to a committed module must each fail the check; the real pin must pass.

The hermetic tests use fake compilers (shell scripts) and need no network and no
real compiler. The tests against the real tree need the pinned compiler
(tools/render/shader_toolchain.py build) and are skipped without it; the
conformance row shader.toolchain-pin is the gate and never skips.
"""
import io
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import shader_toolchain as st  # noqa: E402

PIN = st.load_pin()
MODULE = [st.SPIRV_MAGIC, 0x00010000, 0x000D000B, 0x00000010, 0, 0x00020011, 0x00000001]


def spirv_bytes(words):
    return b"".join(word.to_bytes(4, "little") for word in words)


def fake_compiler(directory, identity, module=None):
    """A glslc that prints `identity` for --version and writes `module` to -o."""
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    blob = directory / "module.spv"
    if module is not None:
        blob.write_bytes(spirv_bytes(module))
    script = directory / "glslc"
    script.write_text(
        "#!/bin/sh\n"
        "if [ \"$1\" = --version ]; then printf '%%s\\n' %s; exit 0; fi\n"
        "while [ $# -gt 0 ]; do\n"
        "  if [ \"$1\" = -o ]; then cp %s \"$2\"; exit $?; fi\n"
        "  shift\n"
        "done\n"
        "exit 1\n" % (" ".join("'%s'" % line for line in identity), blob))
    script.chmod(0o755)
    return str(script)


def header(name, words):
    body = "".join("    0x%08xu,\n" % word for word in words)
    return "#include <cstdint>\n\nstatic const uint32_t %s[] = {\n%s};\n" % (name, body)


def pinned_compiler_available():
    try:
        path, _ = st.resolve(PIN["compiler"]["executable"], st.ENV_GLSLC, PIN)
        return st.identity_problem(path, PIN) is None
    except st.ToolchainError:
        return False


class EnvironmentOverride:
    def __init__(self, **values):
        self.values = values

    def __enter__(self):
        self.saved = {key: os.environ.get(key) for key in self.values}
        os.environ.update(self.values)
        st.glslc.cache_clear()
        st.glslang_validator.cache_clear()

    def __exit__(self, *_):
        for key, value in self.saved.items():
            if value is None:
                os.environ.pop(key, None)
            else:
                os.environ[key] = value
        st.glslc.cache_clear()
        st.glslang_validator.cache_clear()


class IdentityTest(unittest.TestCase):
    def test_pin_record_is_complete(self):
        self.assertEqual(PIN["compiler"]["executable"], "glslc")
        self.assertTrue(PIN["compiler"]["identity"])
        for component in PIN["components"].values():
            self.assertRegex(component["sha256"], r"^[0-9a-f]{64}$")
            self.assertRegex(component["commit"], r"^[0-9a-f]{40}$")

    def test_foreign_compiler_fails_naming_both_identities(self):
        with tempfile.TemporaryDirectory() as tmp:
            foreign = fake_compiler(tmp, st.FOREIGN_IDENTITY)
            problem = st.identity_problem(foreign, PIN)
            self.assertIsNotNone(problem)
            self.assertIn("shaderc v2025.5", problem)
            self.assertIn(PIN["compiler"]["identity"][0], problem)
            with EnvironmentOverride(**{st.ENV_GLSLC: foreign}):
                with self.assertRaises(st.ToolchainError):
                    st.glslc()
                checks, evidence = st.run_check(out=Path(tmp) / "out",
                                                stream=io.StringIO())
            self.assertEqual(checks.failures, 1)
            self.assertEqual([r["name"] for r in checks.results if not r["ok"]],
                             ["toolchain.identity"])
            self.assertEqual(evidence["observed_identity"], list(st.FOREIGN_IDENTITY))

    def test_pinned_identity_passes(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.assertIsNone(st.identity_problem(
                fake_compiler(tmp, PIN["compiler"]["identity"]), PIN))

    def test_one_changed_identity_line_fails(self):
        identity = list(PIN["compiler"]["identity"])
        identity[2] = identity[2] + "-dirty"
        with tempfile.TemporaryDirectory() as tmp:
            self.assertIsNotNone(st.identity_problem(fake_compiler(tmp, identity), PIN))

    def test_regenerators_refuse_a_foreign_compiler(self):
        with tempfile.TemporaryDirectory() as tmp:
            env = dict(os.environ)
            env[st.ENV_GLSLC] = fake_compiler(tmp, st.FOREIGN_IDENTITY)
            for _, script, _ in st.REGENERATORS:
                result = subprocess.run([sys.executable, str(st.ROOT / script), "--check"],
                                        capture_output=True, text=True, env=env)
                self.assertEqual(result.returncode, 2, script)
                self.assertIn("the pin", result.stderr, script)


class ModuleTest(unittest.TestCase):
    """The rebuild comparison with a fake pinned compiler that emits MODULE."""

    def run_embedded(self, root, compiler):
        table = (("fixture.h", "kModule", "module.vert", ("-O",)),)
        (Path(root) / "module.vert").write_text("#version 450\nvoid main() {}\n")
        checks = st.RecordingChecks(io.StringIO())
        st.check_embedded(checks, compiler, root, root, table)
        return checks

    def test_identical_module_passes(self):
        with tempfile.TemporaryDirectory() as tmp:
            compiler = fake_compiler(Path(tmp) / "bin", PIN["compiler"]["identity"], MODULE)
            (Path(tmp) / "fixture.h").write_text(header("kModule", MODULE))
            checks = self.run_embedded(tmp, compiler)
            self.assertEqual((checks.checks, checks.failures), (2, 0))

    def test_seeded_byte_is_detected(self):
        with tempfile.TemporaryDirectory() as tmp:
            compiler = fake_compiler(Path(tmp) / "bin", PIN["compiler"]["identity"], MODULE)
            path = Path(tmp) / "fixture.h"
            path.write_text(header("kModule", MODULE))
            before = path.read_text()
            self.assertEqual(st.flip_one_byte(path), ("kModule", 5))
            after = path.read_text()
            self.assertEqual(sum(a != b for a, b in zip(before, after)), 1)
            words = st.embedded_arrays(after)["kModule"]
            changed = [i for i, (a, b) in enumerate(zip(words, MODULE)) if a != b]
            self.assertEqual(changed, [5])
            self.assertEqual(sum(a != b for a, b in zip(spirv_bytes(words),
                                                         spirv_bytes(MODULE))), 1)
            checks = self.run_embedded(tmp, compiler)
            self.assertEqual(checks.failures, 1)
            self.assertEqual([r["name"] for r in checks.results if not r["ok"]],
                             ["module.fixture.h:kModule"])

    def test_array_without_a_source_row_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            compiler = fake_compiler(Path(tmp) / "bin", PIN["compiler"]["identity"], MODULE)
            (Path(tmp) / "fixture.h").write_text(header("kModule", MODULE) +
                                                 header("kStray", MODULE))
            checks = self.run_embedded(tmp, compiler)
            self.assertEqual([r["name"] for r in checks.results if not r["ok"]],
                             ["inventory.arrays.fixture.h"])

    def test_multi_word_lines_parse(self):
        text = "inline constexpr std::uint32_t kA[] = { 0x07230203, 0x00010300,\n  0x1 };\n"
        self.assertEqual(st.embedded_arrays(text), {"kA": [st.SPIRV_MAGIC, 0x00010300, 1]})


class InventoryTest(unittest.TestCase):
    def test_uncovered_file_fails_and_covered_or_exempt_pass(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "covered.h").write_text(header("kA", MODULE))
            (root / "stray.cpp").write_text(header("kB", MODULE))
            (root / "exempt.cpp").write_text("std::vector<uint32_t> c = { 0x07230203u, 0u };\n")
            (root / "reads.cpp").write_text("bool ok = words[0] == 0x07230203u;\n")
            checks = st.RecordingChecks(io.StringIO())
            st.check_inventory(checks, root, regenerators=(),
                               table=(("covered.h", "kA", "a.vert", ()),),
                               exempt={"exempt.cpp": "stub"})
            self.assertEqual(sorted((r["name"], r["ok"]) for r in checks.results),
                             [("inventory.files.covered.h", True),
                              ("inventory.files.exempt.cpp", True),
                              ("inventory.files.stray.cpp", False)])


@unittest.skipUnless(pinned_compiler_available(),
                     "the pinned compiler is not built (tools/render/shader_toolchain.py build)")
class RealTreeTest(unittest.TestCase):
    def test_real_pin_passes(self):
        with tempfile.TemporaryDirectory() as tmp:
            checks, evidence = st.run_check(out=tmp, stream=io.StringIO())
        failed = [r["name"] for r in checks.results if not r["ok"]]
        self.assertEqual(failed, [])
        names = [r["name"] for r in checks.results]
        self.assertIn("generator.material", names)
        self.assertIn("generator.legacy", names)
        self.assertEqual(sum(name.startswith("module.") for name in names), len(st.EMBEDDED))

    def test_seeded_byte_in_every_committed_file_is_detected(self):
        with tempfile.TemporaryDirectory() as tmp:
            checks, evidence = st.run_check(out=tmp, seed_fault="flip-byte",
                                            stream=io.StringIO())
        failed = {r["name"] for r in checks.results if not r["ok"]}
        self.assertIn("generator.material", failed)
        self.assertIn("generator.legacy", failed)
        for path in {row[0] for row in st.EMBEDDED}:
            array, _ = evidence["seeded"][path].split(" word ")
            self.assertIn("module.%s:%s" % (path, array), failed)
        # Only the seeded modules fail.
        self.assertEqual(len(failed), 2 + len({row[0] for row in st.EMBEDDED}))

    def test_foreign_wrapper_around_the_real_compiler_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            checks, _ = st.run_check(out=tmp, seed_fault="foreign-compiler",
                                     stream=io.StringIO())
        self.assertEqual([r["name"] for r in checks.results if not r["ok"]],
                         ["toolchain.identity"])


if __name__ == "__main__":
    unittest.main()
