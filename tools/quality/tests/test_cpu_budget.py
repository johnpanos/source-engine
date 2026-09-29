import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import cpu_budget  # noqa: E402


class CpuBudgetTest(unittest.TestCase):
    def cgroup(self, files):
        root = Path(tempfile.mkdtemp())
        for name, text in files.items():
            (root / name).parent.mkdir(parents=True, exist_ok=True)
            (root / name).write_text(text)
        return root

    def test_a_v2_quota_caps_the_affinity(self):
        """The rented host: 96 CPUs visible, cpu.max 4608000/100000 (46.08)."""
        root = self.cgroup({"cpu.max": "4608000 100000\n"})
        with mock.patch.object(os, "sched_getaffinity", lambda pid: set(range(96))):
            self.assertAlmostEqual(cpu_budget.quota(root), 46.08)
            self.assertEqual(cpu_budget.available_cpus(root), 46)

    def test_no_quota_keeps_the_affinity(self):
        root = self.cgroup({"cpu.max": "max 100000\n"})
        with mock.patch.object(os, "sched_getaffinity", lambda pid: set(range(12))):
            self.assertIsNone(cpu_budget.quota(root))
            self.assertEqual(cpu_budget.available_cpus(root), 12)

    def test_v1_quota_and_a_fraction_of_one_cpu(self):
        root = self.cgroup({"cpu/cpu.cfs_quota_us": "50000\n", "cpu/cpu.cfs_period_us": "100000\n"})
        with mock.patch.object(os, "sched_getaffinity", lambda pid: set(range(8))):
            self.assertEqual(cpu_budget.quota(root), 0.5)
            self.assertEqual(cpu_budget.available_cpus(root), 1)
        root = self.cgroup({"cpu/cpu.cfs_quota_us": "-1\n", "cpu/cpu.cfs_period_us": "100000\n"})
        self.assertIsNone(cpu_budget.quota(root))

    def test_no_cgroup_files(self):
        with mock.patch.object(os, "sched_getaffinity", lambda pid: set(range(4))):
            self.assertEqual(cpu_budget.available_cpus(Path(tempfile.mkdtemp())), 4)


if __name__ == "__main__":
    unittest.main()
