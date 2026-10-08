"""Parser and summary fixtures for tools/rgp/rgp.py (no RGP needed)."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import rgp

# RGP 2.7's copied instruction-timing rows: line, opcode, operands, hits,
# cost %, latency; a label row has no hit count. Two waves; the loop block
# runs three times per wave.
TABLE = "\n".join("\t".join(cells) for cells in [
    ["0    ", " _amdgpu_ps_main   ", " ", " ", " ", " "],
    ["1    ", "     s_setprio    ", " 2", " 2  ", " 0.00", " 1 clk  "],
    ["2    ", "     image_sample ", " v[0:3], v[4:5], s[0:7], s[8:11]", " 2  ", " 1.50", " 6 clk"],
    ["3    ", "     s_waitcnt    ", " vmcnt(0)", " 2  ", " 10.00", " 400 clk"],
    ["4    ", " _L0              ", " ", " ", " ", " "],
    ["5    ", "     buffer_load_b128 ", " v[0:3], v4, s[8:11], 0 offen", " 6 ", " 3.00", " 20 clk"],
    ["6    ", "     v_fma_f32    ", " v1, v2, v3, v4", " 6  ", " 20.00", " 4 clk"],
    ["7    ", "     s_cbranch_scc0 ", " _L0", " 6 ", " 0.50", " 2 clk"],
    ["8    ", "     s_endpgm     ", " ", " 0  ", " 0.00", " "],
]) + "\n"


class ParseTest(unittest.TestCase):
    def test_rows_and_blocks(self):
        rows = rgp.parse_instruction_table(TABLE)
        self.assertEqual(7, len(rows))
        self.assertEqual("_amdgpu_ps_main", rows[0]["block"])
        self.assertEqual("_L0", rows[3]["block"])
        self.assertEqual(400, rows[2]["latency_clk"])
        self.assertEqual(0, rows[-1]["hits"])

    def test_other_tables_are_rejected(self):
        for text in ("", "2378 vkCmdDrawIndexed(189, 1, 0, 0, 0)\n", "a\tb\tc\n"):
            with self.assertRaises(rgp.RgpError):
                rgp.parse_instruction_table(text)

    def test_a_malformed_row_fails(self):
        with self.assertRaises(rgp.RgpError):
            rgp.parse_instruction_table(TABLE.replace(" 20.00", " twenty"))


class SummaryTest(unittest.TestCase):
    def test_classes_blocks_and_loops(self):
        summary = rgp.summarize(rgp.parse_instruction_table(TABLE))
        self.assertEqual(2, summary["waves"])
        self.assertEqual(6, summary["executed_instructions"])
        self.assertEqual(20.0, summary["classes"]["valu"]["cost_pct"])
        self.assertEqual(10.0, summary["classes"]["wait"]["cost_pct"])
        self.assertEqual(4.5, summary["classes"]["vmem"]["cost_pct"])
        loop = summary["blocks"][0]
        self.assertEqual(("_L0", 3.0, 23.5), (loop["block"], loop["iterations_per_wave"], loop["cost_pct"]))
        self.assertEqual(["_L0"], summary["loops"])
        self.assertEqual("v_fma_f32", summary["top_instructions"][0]["opcode"])

    def test_a_shader_without_waves_fails(self):
        with self.assertRaises(rgp.RgpError):
            rgp.summarize(rgp.parse_instruction_table(TABLE.replace(" 2  ", " 0  ")))


class CommandLineTest(unittest.TestCase):
    def test_capture_needs_a_command(self):
        with self.assertRaises(SystemExit):
            rgp.parse_args(["capture", "--out", "x.rgp", "--after", "5"])
        args = rgp.parse_args(["capture", "--out", "x.rgp", "--after", "5", "--", "./kiln", "play", "portal2", "-x"])
        self.assertEqual(["./kiln", "play", "portal2", "-x"], args.run)

    def test_isa_needs_an_event(self):
        with self.assertRaises(SystemExit):
            rgp.parse_args(["isa", "t.rgp", "--out", "d"])


if __name__ == "__main__":
    unittest.main()
