"""Seeded trees for tools/render/vulkan_scans.py: each defect must be reported,
and each clean case must pass (RFC 0016 K1 render.vulkan.allocation-sites and
render.vulkan.idle-waits)."""
import copy
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import vulkan_scans

TEARDOWN = "void Device::Shutdown()\n{\n\tvkDeviceWaitIdle( m_device );\n}\n"
FRAME = "bool CVulkanContext::BeginFrame( bool *outSkip )\n{\n\tvkQueueWaitIdle( m_queue );\n\treturn true;\n}\n"
LISTED = {"path": "render/device/vulkan/device.cpp", "function": "Device::Shutdown", "count": 1,
          "class": "teardown", "reason": "teardown"}


def reviewed(allocations=(), idle=(LISTED,)):
    return {"schema": vulkan_scans.SCHEMA, "allocation_exceptions": list(allocations),
            "idle_waits": [dict(entry) for entry in idle]}


class ScanTree:
    def __init__(self, files):
        self.temp = tempfile.TemporaryDirectory(prefix="vulkan-scans-test-")
        self.root = Path(self.temp.name)
        subprocess.run(["git", "init", "-q", str(self.root)], check=True)
        for name, text in files.items():
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text, encoding="utf-8")

    def failures(self, data, kind):
        return vulkan_scans._scan_failures(self.root, data, kind)

    def close(self):
        self.temp.cleanup()


class VulkanScansTest(unittest.TestCase):
    def scan(self, files, data, kind):
        tree = ScanTree(files)
        try:
            return tree.failures(data, kind)
        finally:
            tree.close()

    def test_allocation_in_product_path_fails(self):
        failures = self.scan({"materialsystem/shaderapivulkan/vulkan_device.cpp":
                              "void F()\n{\n\tvkAllocateMemory( d, &i, nullptr, &m );\n}\n"},
                             reviewed(idle=()), "allocation")
        self.assertTrue(any("outside-adapter" in f and "vulkan_device.cpp:3" in f for f in failures))

    def test_allocation_in_adapter_passes(self):
        failures = self.scan({"render/device/vulkan/resources.cpp":
                              "void F()\n{\n\tvkCreateBuffer( d, &i, nullptr, &b );\n"
                              "\tvkCreateImage( d, &i, nullptr, &m );\n}\n"},
                             reviewed(idle=()), "allocation")
        self.assertEqual(failures, [])

    def test_commented_and_quoted_calls_are_ignored(self):
        text = ("// vkAllocateMemory( d, &i, nullptr, &m );\n"
                "/* vkCreateImage(\n   d ) */\nconst char *s = \"vkCreateBuffer\";\n"
                "void F()\n{\n\t// vkDeviceWaitIdle( d );\n}\n")
        tree = {"engine/x.cpp": text}
        self.assertEqual(self.scan(tree, reviewed(idle=()), "allocation"), [])
        self.assertEqual(self.scan(tree, reviewed(idle=()), "idle"), [])

    def test_view_creation_is_not_an_allocation(self):
        failures = self.scan({"engine/x.cpp": "void F()\n{\n\tvkCreateImageView( d, &i, nullptr, &v );\n"
                              "\tvkCreateBufferView( d, &i, nullptr, &v );\n}\n"},
                             reviewed(idle=()), "allocation")
        self.assertEqual(failures, [])

    def test_exception_outside_unittests_is_rejected(self):
        product = "materialsystem/shaderapivulkan/vulkan_device.cpp"
        failures = self.scan({product: "void F()\n{\n\tvkCreateImage( d, &i, nullptr, &m );\n}\n"},
                             reviewed(allocations=[{"path": product, "reason": "seeded"}], idle=()),
                             "allocation")
        self.assertTrue(any("allocation-exception-in-product" in f for f in failures))
        self.assertTrue(any("outside-adapter" in f for f in failures))

    def test_reviewed_test_fixture_exception_passes(self):
        fixture = "unittests/shaderapivulkantest/test_x.cpp"
        failures = self.scan({fixture: "int main()\n{\n\tvkCreateImage( d, &i, nullptr, &m );\n}\n"},
                             reviewed(allocations=[{"path": fixture, "reason": "own device"}], idle=()),
                             "allocation")
        self.assertEqual(failures, [])

    def test_unlisted_idle_wait_fails(self):
        failures = self.scan({"render/device/vulkan/device.cpp": TEARDOWN,
                              "materialsystem/shaderapivulkan/vulkan_device.cpp": FRAME},
                             reviewed(), "idle")
        self.assertTrue(any("unlisted" in f and "CVulkanContext::BeginFrame" in f for f in failures))

    def test_listed_idle_wait_passes(self):
        self.assertEqual(self.scan({"render/device/vulkan/device.cpp": TEARDOWN}, reviewed(), "idle"), [])

    def test_count_overrun_fails(self):
        doubled = TEARDOWN.replace("}\n", "\tvkDeviceWaitIdle( m_device );\n}\n")
        failures = self.scan({"render/device/vulkan/device.cpp": doubled}, reviewed(), "idle")
        self.assertTrue(any("idle-waits.count" in f for f in failures))

    def test_stale_entry_fails(self):
        failures = self.scan({"render/device/vulkan/device.cpp": "void Device::Shutdown()\n{\n}\n"},
                             reviewed(), "idle")
        self.assertTrue(any("stale" in f for f in failures))

    def test_pending_entry_fails_until_replaced(self):
        pending = dict(LISTED, **{"class": "frame-path", "pending": True})
        failures = self.scan({"render/device/vulkan/device.cpp": TEARDOWN}, reviewed(idle=[pending]), "idle")
        self.assertTrue(any("pending-review" in f for f in failures))

    def test_unknown_class_is_rejected(self):
        wrong = dict(LISTED, **{"class": "frame-path"})
        failures = self.scan({"render/device/vulkan/device.cpp": TEARDOWN}, reviewed(idle=[wrong]), "idle")
        self.assertTrue(any("list.idle-wait-class" in f for f in failures))

    def test_enclosing_function_heuristic(self):
        code = vulkan_scans.strip_code(
            "namespace n\n{\nclass Dev final : public IDev\n{\npublic:\n"
            "\t~Dev() override\n\t{\n\t\tauto f = [&]() { vkDeviceWaitIdle( d ); };\n\t}\n};\n}\n"
            "static const int kX[] = { 1, 2 };\n"
            "bool CVulkanContext::Foo( int a = 0 )\n{\n\tif ( a )\n\t{\n\t\tvkQueueWaitIdle( q );\n\t}\n}\n")
        owners = vulkan_scans.functions_by_line(code)
        lines = code.split("\n")
        idle = [i + 1 for i, line in enumerate(lines) if "WaitIdle" in line]
        self.assertEqual([vulkan_scans.short_function(owners[n]) for n in idle],
                         ["Dev::~Dev", "CVulkanContext::Foo"])

    def test_seeded_frame_wait_against_real_list_fails(self):
        real = vulkan_scans.load_list(vulkan_scans.DEFAULT_LIST)
        root = vulkan_scans.repo_root()
        files = {}
        for name in vulkan_scans.source_files(root):
            text = (root / name).read_text(encoding="utf-8", errors="replace")
            if vulkan_scans.IDLE_WAIT.search(text):
                files[name] = text
        tree = ScanTree(files)
        try:
            control = tree.failures(copy.deepcopy(real), "idle")
            self.assertFalse(any("unlisted" in f or "stale" in f for f in control), control[:3])
            seeded = tree.root / "materialsystem/shaderapivulkan/vulkan_seeded_frame.cpp"
            seeded.parent.mkdir(parents=True, exist_ok=True)
            seeded.write_text(FRAME, encoding="utf-8")
            added = [f for f in tree.failures(copy.deepcopy(real), "idle") if f not in control]
            self.assertEqual(len(added), 1, added)
            self.assertIn("vulkan_seeded_frame.cpp", added[0])
        finally:
            tree.close()

    def test_real_list_schema_is_valid(self):
        real = vulkan_scans.load_list(vulkan_scans.DEFAULT_LIST)
        failures = [f for f in self.scan({}, real, "allocation") if f.startswith("FAIL list.")]
        self.assertEqual(failures, [])


if __name__ == "__main__":
    unittest.main()
