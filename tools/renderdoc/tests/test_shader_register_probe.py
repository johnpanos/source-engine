"""The diagnostic must freeze only the requested specialization constant."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from shader_register_probe import freeze_integer


ASSEMBLY = """
               OpDecorate %10 SpecId 0
               OpDecorate %11 SpecId 102
        %int = OpTypeInt 32 1
         %10 = OpSpecConstant %int 2048
         %11 = OpSpecConstant %int 0
"""


class ShaderRegisterProbeTest(unittest.TestCase):
    def test_only_selected_constant_is_frozen(self):
        frozen = freeze_integer(ASSEMBLY, 102, 4)
        self.assertIn("%10 SpecId 0", frozen)
        self.assertIn("%10 = OpSpecConstant %int 2048", frozen)
        self.assertIn("%11 = OpConstant %int 4", frozen)
        self.assertNotIn("SpecId 102", frozen)

    def test_zero_is_a_real_calibration_value(self):
        self.assertIn("%11 = OpConstant %int 0", freeze_integer(ASSEMBLY, 102, 0))

    def test_missing_and_duplicate_decorations_fail(self):
        with self.assertRaises(ValueError):
            freeze_integer(ASSEMBLY, 99, 4)
        with self.assertRaises(ValueError):
            freeze_integer(ASSEMBLY + "OpDecorate %12 SpecId 102\n", 102, 4)

    def test_non_integer_and_wrong_width_fail(self):
        for declaration in ("OpTypeFloat 32", "OpTypeInt 64 1"):
            with self.subTest(declaration=declaration), self.assertRaises(ValueError):
                freeze_integer(ASSEMBLY.replace("OpTypeInt 32 1", declaration), 102, 4)

    def test_signed_and_unsigned_ranges_are_checked(self):
        self.assertIn("OpConstant %int -1", freeze_integer(ASSEMBLY, 102, -1))
        with self.assertRaises(ValueError):
            freeze_integer(ASSEMBLY, 102, 1 << 31)
        unsigned = ASSEMBLY.replace("OpTypeInt 32 1", "OpTypeInt 32 0")
        self.assertIn("4294967295", freeze_integer(unsigned, 102, (1 << 32) - 1))
        with self.assertRaises(ValueError):
            freeze_integer(unsigned, 102, -1)


if __name__ == "__main__":
    unittest.main()
