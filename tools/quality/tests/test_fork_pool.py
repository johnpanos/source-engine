"""ForkPool: forked results, and a dead worker falls back to serial instead of hanging."""
import os
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import fork_pool  # noqa: E402

PARENT = os.getpid()


def square(value):
    return value * value


def die_in_child(value):
    # A forked worker dies the way a crashing Blender child does: abruptly.
    if os.getpid() != PARENT:
        os._exit(11)
    return value * value


class ForkPoolTest(unittest.TestCase):
    def test_workers_return_every_result_in_order(self):
        with fork_pool.ForkPool(4) as pool:
            self.assertEqual(pool.map(square, range(50), chunksize=3), [v * v for v in range(50)])
            self.assertFalse(pool.broken)

    def test_a_dead_worker_falls_back_to_serial(self):
        with fork_pool.ForkPool(4) as pool:
            self.assertEqual(pool.map(die_in_child, range(20)), [v * v for v in range(20)])
            self.assertTrue(pool.broken)
            # Later maps on the broken pool run in the parent too.
            self.assertEqual(pool.map(die_in_child, [3, 4]), [9, 16])

    def test_one_worker_runs_in_the_parent(self):
        with fork_pool.ForkPool(1) as pool:
            self.assertIsNone(pool.executor)
            self.assertEqual(pool.map(die_in_child, [2, 5]), [4, 25])


if __name__ == "__main__":
    unittest.main()
