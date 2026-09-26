"""D3D9 shader VM and .vcs reader: handcrafted bytecode per instruction family,
decode/disassembly round trips, negative streams, and checks against the
retail stdshader_dx9 bytecode in hl2_misc_dir.vpk (skipped when absent)."""

from pathlib import Path
import bz2
import contextlib
import io
import lzma
import math
import re
import struct
import sys
import unittest

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "quality"))

import d3d9_shader_vm as vm  # noqa: E402
import d3d9_texture as tex  # noqa: E402
import source_vcs as sv  # noqa: E402

# -- a tiny assembler for the disassembler's syntax ---------------------------

REGISTER_PATTERNS = [
    (r"oPos", vm.REG_RASTOUT, 0), (r"oFog", vm.REG_RASTOUT, 1), (r"oPts", vm.REG_RASTOUT, 2),
    (r"oDepth", vm.REG_DEPTHOUT, 0), (r"vPos", vm.REG_MISCTYPE, 0), (r"vFace", vm.REG_MISCTYPE, 1),
    (r"aL", vm.REG_LOOP, 0), (r"oD(\d+)", vm.REG_ATTROUT, None),
    (r"oT(\d+)", vm.REG_TEXCRDOUT, None),
    (r"oC(\d+)", vm.REG_COLOROUT, None), (r"o(\d+)", vm.REG_OUTPUT, None),
    (r"r(\d+)", vm.REG_TEMP, None), (r"v(\d+)", vm.REG_INPUT, None),
    (r"c(\d+)", vm.REG_CONST, None),
    (r"a(\d+)", vm.REG_ADDR, None), (r"t(\d+)", vm.REG_TEXTURE, None),
    (r"i(\d+)", vm.REG_CONSTINT, None), (r"s(\d+)", vm.REG_SAMPLER, None),
    (r"b(\d+)", vm.REG_CONSTBOOL, None), (r"l(\d+)", vm.REG_LABEL, None),
    (r"p(\d+)", vm.REG_PREDICATE, None),
]
SRC_SUFFIX = {"_bias": vm.SRC_BIAS, "_bx2": vm.SRC_SIGN, "_x2": vm.SRC_X2, "_dz": vm.SRC_DZ,
              "_dw": vm.SRC_DW, "_abs": vm.SRC_ABS}
NEGATED = {vm.SRC_NONE: vm.SRC_NEG, vm.SRC_BIAS: vm.SRC_BIASNEG, vm.SRC_SIGN: vm.SRC_SIGNNEG,
           vm.SRC_X2: vm.SRC_X2NEG, vm.SRC_ABS: vm.SRC_ABSNEG}
SHIFTS = {"x2": 1, "x4": 2, "x8": 3, "d2": 0xF, "d4": 0xE, "d8": 0xD}
COMPARE = {name: code for code, name in vm.COMPARISON.items()}


def _type_bits(reg_type):
    return ((reg_type & 7) << 28) | ((reg_type & 0x18) << 8)


def _register(text):
    for pattern, reg_type, fixed in REGISTER_PATTERNS:
        match = re.fullmatch(pattern, text)
        if match:
            return reg_type, fixed if fixed is not None else int(match.group(1))
    raise ValueError("bad register %r" % text)


def _split_register(text, major):
    """(type, num, rel tokens list, rel flag) for 'c5', 'c[a0.x + 5]', 'o[aL]'."""
    match = re.fullmatch(r"([a-zA-Z]+)\[(a0\.[xyzw]|aL)(?:\s*\+\s*(\d+))?\]", text)
    if not match:
        reg_type, num = _register(text)
        return reg_type, num, []
    base, rel, offset = match.group(1), match.group(2), int(match.group(3) or 0)
    reg_type, _ = _register(base + "0")
    if rel == "aL":
        rel_token = 0x80000000 | _type_bits(vm.REG_LOOP)
    else:
        comp = "xyzw".index(rel[-1])
        rel_token = 0x80000000 | _type_bits(vm.REG_ADDR) | (comp * 0x55 << 16)
    return reg_type, offset, [rel_token] if major >= 2 else []


def _dst(text, mods, shift, major):
    match = re.fullmatch(r"(.+?)(?:\.([xyzw]+))?", text)
    reg_type, num, rel = _split_register(match.group(1), major)
    mask = 0xF
    if match.group(2):
        mask = sum(1 << "xyzw".index(c) for c in match.group(2))
    token = 0x80000000 | _type_bits(reg_type) | num | (mask << 16) | (mods << 20) | (shift << 24)
    if rel or (major < 2 and "[" in text):
        token |= 1 << 13
    return [token] + rel


def _src(text, major):
    text = text.strip()
    mod = vm.SRC_NONE
    negate = False
    if text.startswith("1 - "):
        mod, text = vm.SRC_COMP, text[4:]
    if text.startswith("!"):
        mod, text = vm.SRC_NOT, text[1:]
    if text.startswith("-"):
        negate, text = True, text[1:]
    for suffix, code in SRC_SUFFIX.items():
        if text.endswith(suffix):
            mod, text = code, text[:-len(suffix)]
            break
    if negate:
        mod = NEGATED[mod]
    match = re.fullmatch(r"(.+?)(?:\.([xyzw]{1,4}))?", text)
    reg_type, num, rel = _split_register(match.group(1), major)
    swizzle = match.group(2) or "xyzw"
    swizzle = swizzle + swizzle[-1] * (4 - len(swizzle))
    bits = sum("xyzw".index(c) << (2 * i) for i, c in enumerate(swizzle))
    token = 0x80000000 | _type_bits(reg_type) | num | (bits << 16) | (mod << 24)
    if rel or (major < 2 and "[" in text):
        token |= 1 << 13
    return [token] + rel


def _float_bits(value):
    return struct.unpack("<I", struct.pack("<f", float(value)))[0]


def assemble(text):
    """Assemble the disassembler's syntax back into bytes."""
    lines = [line.split("//")[0].strip() for line in text.strip().splitlines()]
    lines = [line for line in lines if line]
    profile = lines[0]
    kind, major, minor = re.fullmatch(r"(vs|ps)_(\d)_(\d|x)", profile).groups()
    major = int(major)
    minor = 1 if minor == "x" else int(minor)
    tokens = [(0xFFFF0000 if kind == "ps" else 0xFFFE0000) | (major << 8) | minor]
    for line in lines[1:]:
        coissue = line.startswith("+")
        line = line.lstrip("+")
        pred_tokens = []
        match = re.match(r"\((!?)(p0(?:\.[xyzw]+)?)\)\s*", line)
        if match:
            pred_tokens = _src(("!" if match.group(1) else "") + match.group(2), major)
            line = line[match.end():]
        name, _, rest = line.partition(" ")
        operands = [op.strip() for op in rest.split(",")] if rest.strip() else []
        control = 0
        body = []
        if name == "def":
            opcode = vm.OP["DEF"]
            body = _dst(operands[0], 0, 0, major) + [_float_bits(v) for v in operands[1:]]
        elif name == "defi":
            opcode = vm.OP["DEFI"]
            body = _dst(operands[0], 0, 0, major) + [int(v) & 0xFFFFFFFF for v in operands[1:]]
        elif name == "defb":
            opcode = vm.OP["DEFB"]
            body = _dst(operands[0], 0, 0, major) + [1 if operands[1] == "true" else 0]
        elif name.startswith("dcl"):
            opcode = vm.OP["DCL"]
            parts = name.split("_")[1:]
            dcl = 0x80000000
            mods = 0
            for part in parts:
                if part in ("2d", "cube", "volume"):
                    dcl |= {"2d": 2, "cube": 3, "volume": 4}[part] << 27
                elif part == "centroid":
                    mods |= 4
                elif part == "pp":
                    mods |= 2
                else:
                    usage = re.fullmatch(r"([a-z]+?)(\d*)", part)
                    dcl |= vm.USAGE_NAMES.index(usage.group(1)) | (int(usage.group(2) or 0) << 16)
            body = [dcl] + _dst(operands[0], mods, 0, major)
        else:
            parts = name.split("_")
            base = parts[0]
            mods = 0
            shift = 0
            for part in parts[1:]:
                if part == "sat":
                    mods |= 1
                elif part == "pp":
                    mods |= 2
                elif part == "centroid":
                    mods |= 4
                elif part in SHIFTS:
                    shift = SHIFTS[part]
                elif part in COMPARE:
                    control = COMPARE[part]
                else:
                    raise ValueError("bad modifier %r in %r" % (part, name))
            if base in ("texld", "texldp", "texldb", "tex"):
                opcode = vm.OP["TEX"]
                control = {"texldp": 1, "texldb": 2}.get(base, 0)
            elif base == "texcrd":
                opcode = vm.OP["TEXCOORD"]
            else:
                opcode = {n.lower(): c for n, c in vm.OP.items()}[base]
            dst_count, _src_count = vm.OPERANDS.get(opcode, (0, 0))
            if opcode == vm.OP["TEX"]:
                dst_count = 1
            ops = list(operands)
            if dst_count:
                body += _dst(ops.pop(0), mods, shift, major)
            body += pred_tokens
            for op in ops:
                body += _src(op, major)
        length = len(body) if major >= 2 else 0
        token = opcode | (control << 16) | (length << 24)
        if pred_tokens:
            token |= 1 << 28
        if coissue:
            token |= 1 << 30
        tokens.append(token)
        tokens.extend(body)
    tokens.append(0x0000FFFF)
    return struct.pack("<%dI" % len(tokens), *tokens)


def f32(value):
    return vm.f32(value)


class InstructionTests(unittest.TestCase):

    def ps(self, body, inputs=None, consts=None, **kw):
        return vm.run_pixel(assemble("ps_3_0\n" + body), inputs or {}, consts or {}, **kw)

    def vs(self, body, inputs=None, consts=None, profile="vs_3_0", **kw):
        return vm.run_vertex(assemble(profile + "\n" + body), inputs or {}, consts or {}, **kw)

    def assertVec(self, actual, expected, places=None):
        self.assertEqual(len(actual), 4)
        for a, e in zip(actual, expected):
            if e is None:
                continue
            if isinstance(e, float) and math.isnan(e):
                self.assertTrue(math.isnan(a), (actual, expected))
            elif places is None:
                self.assertEqual(a, e, (actual, expected))
            else:
                self.assertAlmostEqual(a, e, places=places, msg=(actual, expected))

    def test_arithmetic_family(self):
        consts = {"c0": (1.5, -2.0, 0.25, 3.0), "c1": (0.5, 4.0, -0.75, 2.0),
                  "c2": (0.1, 0.2, 0.3, 0.4)}
        cases = {
            "add": (2.0, 2.0, -0.5, 5.0),
            "sub": (1.0, -6.0, 1.0, 1.0),
            "mul": (0.75, -8.0, -0.1875, 6.0),
            "min": (0.5, -2.0, -0.75, 2.0),
            "max": (1.5, 4.0, 0.25, 3.0),
            "slt": (0.0, 1.0, 0.0, 0.0),
            "sge": (1.0, 0.0, 1.0, 1.0),
        }
        for op, expected in cases.items():
            out = self.ps("%s r0, c0, c1\nmov oC0, r0" % op, consts=consts)
            self.assertVec(out["oC0"], expected)
        out = self.ps("mad r0, c0, c1, c2\nmov oC0, r0", consts=consts)
        self.assertVec(out["oC0"], [f32(f32(a * b) + f32(c)) for a, b, c in zip(
            consts["c0"], consts["c1"], consts["c2"])])
        out = self.ps("dp3 r0, c0, c1\nmov oC0, r0", consts=consts)
        self.assertVec(out["oC0"], [0.75 - 8.0 - 0.1875] * 4)
        out = self.ps("dp4 r0, c0, c1\nmov oC0, r0", consts=consts)
        self.assertVec(out["oC0"], [0.75 - 8.0 - 0.1875 + 6.0] * 4)
        out = self.ps("dp2add r0, c0, c1, c2.w\nmov oC0, r0", consts=consts)
        self.assertVec(out["oC0"], [f32(0.75 - 8.0 + f32(0.4))] * 4)
        out = self.ps("lrp r0, c2, c0, c1\nmov oC0, r0", consts=consts)
        expected = [f32(f32(f32(s) * f32(a - b)) + b) for s, a, b in zip(
            consts["c2"], consts["c0"], consts["c1"])]
        self.assertVec(out["oC0"], expected)
        out = self.ps("cmp r0, c1, c0, c2\nmov oC0, r0", consts=consts)
        self.assertVec(out["oC0"], (1.5, -2.0, f32(0.3), 3.0))
        out = self.ps("frc r0, c0\nabs r1, c1\nmov oC0, r0\nmov oC1, r1", consts=consts)
        self.assertVec(out["oC0"], (0.5, 0.0, 0.25, 0.0))
        self.assertVec(out["oC1"], (0.5, 4.0, 0.75, 2.0))
        out = self.ps("sgn r0, c0\nmov oC0, r0", consts={"c0": (-3, 0, 2, -0.0)})
        self.assertVec(out["oC0"], (-1.0, 0.0, 1.0, 0.0))

    def test_float32_rounding(self):
        out = self.ps("add r0, c0, c1\nmov oC0, r0", consts={"c0": (1.0,) * 4, "c1": (1e-8,) * 4})
        self.assertEqual(out["oC0"][0], 1.0)  # 1 + 1e-8 is 1 in float32
        out = self.ps("mul r0, c0, c0\nmov oC0, r0", consts={"c0": (1e30,) * 4})
        self.assertEqual(out["oC0"][0], math.inf)
        # Unfused mad: 0.1*10 rounds to 1.0 before the add.
        out = self.ps("mad r0, c0, c1, c2\nmov oC0, r0",
                      consts={"c0": (0.1,) * 4, "c1": (10.0,) * 4, "c2": (-1.0,) * 4})
        self.assertEqual(out["oC0"][0], f32(f32(f32(0.1) * 10.0) - 1.0))
        # dp3 accumulates left to right in float32: (1 + 1e-8) rounds to 1 first.
        out = self.ps("dp3 r0, c0, c1\nmov oC0, r0",
                      consts={"c0": (1.0, 1e-8, -1.0, 0.0), "c1": (1.0, 1.0, 1.0, 0.0)})
        self.assertEqual(out["oC0"][0], 0.0)
        # cmp selects src1 for +0 and -0 (src0 >= 0), src2 for NaN.
        out = self.ps("cmp r0, c0, c1, c2\nmov oC0, r0",
                      consts={"c0": (0.0, -0.0, math.nan, -1e-30), "c1": (1,) * 4, "c2": (2,) * 4})
        self.assertVec(out["oC0"], (1.0, 1.0, 2.0, 2.0))

    def test_scalar_instructions(self):
        def scalar(op, x, y=None):
            consts = {"c0": (0.0, 0.0, 0.0, x)}
            if y is None:
                body = "%s r0, c0.w\nmov oC0, r0" % op
            else:
                consts["c1"] = (y,) * 4
                body = "%s r0, c0.w, c1.x\nmov oC0, r0" % op
            return self.ps(body, consts=consts)["oC0"]
        self.assertVec(scalar("rcp", 0.0), [math.inf] * 4)
        self.assertVec(scalar("rcp", -0.0), [math.inf] * 4)
        self.assertVec(scalar("rcp", 4.0), [0.25] * 4)
        self.assertVec(scalar("rcp", 1.0), [1.0] * 4)
        self.assertVec(scalar("rsq", -4.0), [0.5] * 4)
        self.assertVec(scalar("rsq", 0.0), [math.inf] * 4)
        self.assertVec(scalar("exp", 3.0), [8.0] * 4)
        self.assertVec(scalar("exp", -1.0), [0.5] * 4)
        self.assertVec(scalar("log", 8.0), [3.0] * 4)
        self.assertVec(scalar("log", -8.0), [3.0] * 4)
        self.assertVec(scalar("log", 0.0), [-math.inf] * 4)
        self.assertVec(scalar("pow", -2.0, 2.0), [4.0] * 4)
        self.assertVec(scalar("pow", 0.0, 2.0), [0.0] * 4)
        self.assertVec(scalar("pow", 0.0, 0.0), [math.nan] * 4)  # exp2(0 * -inf)
        self.assertVec(scalar("pow", 2.0, 0.5), [f32(math.sqrt(2.0))] * 4, places=6)

    def test_swizzle_mask_and_source_modifiers(self):
        consts = {"c0": (1.0, 2.0, 3.0, 4.0), "c1": (-0.25, 0.5, -1.5, 2.0)}
        out = self.ps("mov r0, c1\nmov r0.yw, c0.wzyx\nmov oC0, r0", consts=consts)
        self.assertVec(out["oC0"], (-0.25, 3.0, -1.5, 1.0))
        out = self.ps("mov r0, c0.zx\nmov oC0, r0", consts=consts)  # .zxxx
        self.assertVec(out["oC0"], (3.0, 1.0, 1.0, 1.0))
        out = self.ps("mov r0, -c1\nmov r1, c1_abs\nmov r2, -c1_abs\nmov oC0, r0\nmov oC1, r1\n"
                      "mov oC2, r2", consts=consts)
        self.assertVec(out["oC0"], (0.25, -0.5, 1.5, -2.0))
        self.assertVec(out["oC1"], (0.25, 0.5, 1.5, 2.0))
        self.assertVec(out["oC2"], (-0.25, -0.5, -1.5, -2.0))
        out = self.ps("mov_sat r0, c1\nmov oC0, r0", consts=consts)
        self.assertVec(out["oC0"], (0.0, 0.5, 0.0, 1.0))
        out = self.ps("mov_sat r0, c0\nmov oC0, r0", consts={"c0": (math.nan, -math.inf, 0.5, 9)})
        self.assertVec(out["oC0"], (0.0, 0.0, 0.5, 1.0))

    def test_ps_1_x_modifiers_shift_and_output(self):
        code = assemble("""ps_1_4
            def c0, 0.75, 0.25, 1, 0
            mov r1, c0_bx2
            mov r2, c0_bias
            mov r3, 1 - c0
            mov_x2 r4, c0
            mov_d2 r5, c0_x2
            add r0, -c0_bias, c0""")
        machine_result = vm.run_pixel(code, {})
        self.assertVec(machine_result["oC0"], (0.5, 0.5, 0.5, 0.5))
        shader = vm.parse(code)
        self.assertEqual(shader.profile, "ps_1_4")
        text = vm.disassemble(shader, header=False)
        self.assertIn("mov r1, c0_bx2", text)
        self.assertIn("mov r3, 1 - c0", text)
        self.assertIn("mov_d2 r5, c0_x2", text)
        # Evaluate the intermediate registers through the output register.
        for body, expected in (("mov r0, c0_bx2", (0.5, -0.5, 1.0, -1.0)),
                               ("mov r0, c0_bias", (0.25, -0.25, 0.5, -0.5)),
                               ("mov r0, 1 - c0", (0.25, 0.75, 0.0, 1.0)),
                               ("mov_x2 r0, c0", (1.5, 0.5, 2.0, 0.0)),
                               ("mov_d2 r0, c0_x2", (0.75, 0.25, 1.0, 0.0))):
            result = vm.run_pixel(assemble("ps_1_4\ndef c0, 0.75, 0.25, 1, 0\n" + body), {})
            self.assertVec(result["oC0"], expected)

    def test_ps_1_1_texture_ops_and_coissue(self):
        samplers = tex.SamplerSet({0: tex.Texture2D.constant((0.5, 0.25, 1.0, 0.8))})
        code = assemble("""ps_1_1
            def c0, 1, 1, 1, 0.5
            tex t0
            mul r0.rgb, t0, v0
            +mov r0.a, c0""".replace(".rgb", ".xyz").replace(".a,", ".w,"))
        out = vm.run_pixel(code, {"t0": (0.2, 0.3), "color0": (1.0, 2.0, 0.5, 1.0)}, {}, samplers)
        self.assertVec(out["oC0"], (0.5, 0.25, 0.5, 0.5))  # v0 is clamped to [0, 1]
        self.assertEqual(samplers.calls[0][:3], (0, (f32(0.2), f32(0.3), 0.0, 1.0), "tex"))
        # Co-issued halves read the register state before the pair.
        out = vm.run_pixel(assemble("""ps_1_1
            def c0, 1, 2, 3, 4
            mov r0, c0
            mov r0.xyz, r0.w
            +mov r0.w, r0.x"""), {})
        self.assertVec(out["oC0"], (4.0, 4.0, 4.0, 1.0))
        out = vm.run_pixel(assemble("ps_1_1\ntexcoord t0\nmov r0, t0"), {"t0": (-1, 0.5, 2, 7)})
        self.assertVec(out["oC0"], (0.0, 0.5, 1.0, 1.0))

    def test_matrix_and_vector_instructions(self):
        consts = {"c10": (1, 2, 3, 4), "c11": (0, 1, 0, 0), "c12": (0, 0, 2, 0),
                  "c13": (0, 0, 0, 1),
                  "c0": (1, 1, 1, 1), "c1": (3, 0, 4, 9)}
        out = self.vs("dcl_position o0\nm4x4 o0, c0, c10", consts=consts)
        self.assertVec(out["o0"], (10, 1, 2, 1))
        out = self.vs("dcl_position o0\nmov o0, c0\nm3x3 o0.xyz, c0, c10", consts=consts)
        self.assertVec(out["o0"], (6, 1, 2, 1))
        out = self.vs("dcl_position o0\nmov o0, c0\nm3x2 o0.xy, c0, c10", consts=consts)
        self.assertVec(out["o0"], (6, 1, 1, 1))
        out = self.vs("dcl_position o0\nmov o0, c0\nm4x3 o0.xyz, c0, c10", consts=consts)
        self.assertVec(out["o0"], (10, 1, 2, 1))
        out = self.vs("dcl_position o0\nm3x4 o0, c0, c10", consts=consts)
        self.assertVec(out["o0"], (6, 1, 2, 0))
        out = self.vs("dcl_position o0\nmov o0, c0\nnrm o0.xyz, c1", consts=consts)
        self.assertVec(out["o0"], (0.6, 0.0, 0.8, 1.0), places=6)
        out = self.vs("dcl_position o0\nmov o0, c0\ncrs o0.xyz, c11, c12", consts=consts)
        self.assertVec(out["o0"], (2, 0, 0, 1))
        out = self.vs("dcl_position o0\nmov o0, c0\nsincos o0.xy, c1.y", consts=consts)
        self.assertVec(out["o0"], (1.0, 0.0, 1.0, 1.0))
        out = self.vs("dcl_position o0\nmov o0, c0\nsincos o0.xy, c2.x",
                      consts=dict(consts, c2=(math.pi / 3, 0, 0, 0)))
        self.assertVec(out["o0"], (f32(math.cos(f32(math.pi / 3))), f32(math.sin(f32(math.pi / 3))),
                                   1.0, 1.0), places=6)
        # SM2 sincos carries two extra constant sources that do not affect the result.
        out = self.vs("mov oPos, c0\nsincos r0.xy, c2.x, c3, c4\nmov oT0, r0",
                      consts=dict(consts, c2=(1.0, 0, 0, 0)), profile="vs_2_0")
        self.assertVec(out["oT0"], (f32(math.cos(1.0)), f32(math.sin(1.0)), 0, 0))
        out = self.vs("dcl_position o0\nlit o0, c2", consts=dict(consts, c2=(0.5, 0.25, 0, 2)))
        self.assertVec(out["o0"], (1.0, 0.5, 0.0625, 1.0))
        out = self.vs("dcl_position o0\nlit o0, c2", consts=dict(consts, c2=(-0.5, 0.25, 0, 2)))
        self.assertVec(out["o0"], (1.0, 0.0, 0.0, 1.0))
        out = self.vs("dcl_position o0\ndst o0, c2, c3",
                      consts=dict(consts, c2=(9, 2, 3, 9), c3=(9, 5, 9, 7)))
        self.assertVec(out["o0"], (1.0, 10.0, 3.0, 7.0))

    def test_vs_1_1_partial_precision_and_address_register(self):
        consts = {"c0": (0, 0, 0, 2.5), "c5": (10, 0, 0, 0), "c6": (20, 0, 0, 0),
                  "c7": (30, 0, 0, 0), "c1": (0, 0, 0, 1.5)}
        out = vm.run_vertex(assemble("vs_1_1\nexpp oT0, c0.w\nlogp oT1, c6.x"), {}, consts)
        self.assertVec(out["oT0"], (4.0, 0.5, None, 1.0))
        self.assertAlmostEqual(out["oT0"][2], 2 ** 2.5, places=3)
        self.assertVec(out["oT1"], (4.0, 1.25, None, 1.0))
        self.assertAlmostEqual(out["oT1"][2], math.log2(20), places=4)
        # vs_1_1 `mov a0.x` rounds (floor(x + 0.5)); relative addressing is implicit a0.x.
        out = vm.run_vertex(assemble("vs_1_1\nmov a0.x, c1.w\nmov oPos, c[a0.x + 5]"), {}, consts)
        self.assertVec(out["oPos"], (30, 0, 0, 0))

    def test_mova_rounding_and_relative_addressing(self):
        consts = {"c%d" % n: (n, 0, 0, 0) for n in range(20)}
        for value, index in ((1.5, 12), (-1.5, 8), (2.49, 12), (-0.4, 10), (0.5, 11)):
            out = self.vs("dcl_position o0\nmova a0.x, c30.x\nmov o0, c[a0.x + 10]",
                          consts=dict(consts, c30=(value, 0, 0, 0)))
            self.assertEqual(out["o0"][0], index, value)
        # Out-of-range relative reads return zero.
        out = self.vs("dcl_position o0\nmova a0.y, c30.x\nmov o0, c[a0.y + 10]",
                      consts=dict(consts, c30=(-50, 0, 0, 0)))
        self.assertVec(out["o0"], (0, 0, 0, 0))

    def test_rep_loop_and_aL_addressing(self):
        consts = {"c10": (1, 0, 0, 0), "c11": (2, 0, 0, 0), "c12": (4, 0, 0, 0),
                  "c13": (8, 0, 0, 0), "c14": (16, 0, 0, 0)}
        body = """dcl_position o0
            defi i0, 3, 11, 1, 0
            defi i1, 4, 0, 0, 0
            mov r0, c20
            loop aL, i0
              add r0, r0, c[aL + 0]
            endloop
            rep i1
              add r0.y, r0.y, c10.x
            endrep
            mov o0, r0"""
        out = self.vs(body, consts=consts)
        self.assertVec(out["o0"], (2 + 4 + 8, 4, 0, 0))
        # rep with a zero count skips its body; caller i# is overridden by defi.
        out = self.vs("dcl_position o0\nmov r0, c10\nrep i2\nadd r0, r0, r0\nendrep\nmov o0, r0",
                      consts=dict(consts, i2=(0, 0, 0, 0)))
        self.assertVec(out["o0"], (1, 0, 0, 0))
        out = self.vs("dcl_position o0\ndefi i2, 1, 0, 0, 0\nmov r0, c10\nrep i2\nadd r0, r0, r0\n"
                      "endrep\nmov o0, r0", consts=dict(consts, i2=(5, 0, 0, 0)))
        self.assertVec(out["o0"], (2, 0, 0, 0))

    def test_branches_and_breaks(self):
        body = """dcl_position o0
            if b0
              mov r0, c1
            else
              mov r0, c2
            endif
            mov o0, r0"""
        consts = {"c1": (1, 1, 1, 1), "c2": (2, 2, 2, 2), "c3": (0.5, 0, 0, 0)}
        self.assertVec(self.vs(body, consts=dict(consts, b0=True))["o0"], (1, 1, 1, 1))
        self.assertVec(self.vs(body, consts=dict(consts, b0=False))["o0"], (2, 2, 2, 2))
        body = """dcl_position o0
            mov r0, c3
            ifc_lt c3.x, c1.x
              add r0.y, r0.y, c1.x
            endif
            if !b1
              add r0.z, r0.z, c1.x
            endif
            mov o0, r0"""
        self.assertVec(self.vs(body, consts=consts)["o0"], (0.5, 1, 1, 0))
        # break / breakc leave the innermost loop.
        body = """dcl_position o0
            defi i0, 10, 0, 1, 0
            mov r0, c0
            rep i0
              add r0.x, r0.x, c1.x
              breakc_ge r0.x, c4.x
            endrep
            rep i0
              add r0.y, r0.y, c1.x
              rep i0
                add r0.z, r0.z, c1.x
                break
              endrep
            endrep
            mov o0, r0"""
        out = self.vs(body, consts=dict(consts, c0=(0, 0, 0, 0), c4=(3, 0, 0, 0)))
        self.assertVec(out["o0"], (3, 10, 10, 0))

    def test_predication(self):
        body = """dcl_position o0
            defi i0, 10, 0, 1, 0
            mov r0, c0
            setp_gt p0, c1, c0
            (p0) mov r0, c2
            (!p0.x) mov r0.x, c3.x
            mov r1, c0
            rep i0
              add r1.x, r1.x, c3.x
              setp_ge p0.x, r1.x, c4.x
              breakp p0.x
            endrep
            mov o0, r0
            mov o1, r1"""
        consts = {"c0": (0, 0, 0, 0), "c1": (1, -1, 1, 0), "c2": (5, 6, 7, 8), "c3": (1, 1, 1, 1),
                  "c4": (4, 0, 0, 0)}
        body = body.replace("dcl_position o0", "dcl_position o0\ndcl_texcoord o1")
        out = self.vs(body, consts=consts)
        # p0 = (T, F, T, F): masked mov writes x and z; the negated x lane is false.
        self.assertVec(out["o0"], (5, 0, 7, 0))
        self.assertVec(out["o1"], (4, 0, 0, 0))

    def test_call_callnz_and_nested_aL(self):
        body = """dcl_position o0
            defi i0, 2, 0, 1, 0
            mov r0, c0
            call l0
            callnz l1, b0
            callnz l1, !b0
            loop aL, i0
              loop aL, i0
                add r0.w, r0.w, c[aL + 5].x
              endloop
              add r0.z, r0.z, c[aL + 5].x
            endloop
            mov o0, r0
            ret
            label l0
            add r0.x, r0.x, c1.x
            ret
            label l1
            add r0.y, r0.y, c1.x
            ret"""
        consts = {"c0": (0, 0, 0, 0), "c1": (1, 0, 0, 0), "c5": (1, 0, 0, 0), "c6": (10, 0, 0, 0)}
        out = self.vs(body, consts=consts)
        self.assertVec(out["o0"], (1, 1, 11, 22))

    def test_texkill(self):
        body = "texkill r0\nmov oC0, c0"
        self.assertFalse(self.ps("mov r0, c1\n" + body, consts={"c1": (0, 1, 2, 3)}).discarded)
        self.assertTrue(self.ps("mov r0, c1\n" + body, consts={"c1": (0, 1, 2, -3)}).discarded)
        self.assertFalse(self.ps("mov r0, c1\ntexkill r0.xyz\nmov oC0, c0",
                                 consts={"c1": (0, 1, 2, -3)}).discarded)
        # ps_1_x checks xyz of the texture coordinate.
        code = assemble("ps_1_1\ntexkill t0\nmov r0, v0")
        self.assertFalse(vm.run_pixel(code, {"t0": (0, 0, 0, -1)}).discarded)
        self.assertTrue(vm.run_pixel(code, {"t0": (0, -0.1, 0, 1)}).discarded)

    def test_texture_sampling_variants(self):
        samplers = tex.SamplerSet(default=(0.25, 0.5, 0.75, 1.0))
        body = """dcl_texcoord v0
            dcl_2d s0
            dcl_cube s1
            dcl_volume s2
            texld r0, v0, s0
            texldp r1, v0, s1
            texldb r2, v0, s2
            texldl r3, v0, s0
            texldd r4, v0, s0, c0, c1
            mov oC0, r0"""
        out = self.ps(body, {"texcoord0": (1.0, 2.0, 3.0, 4.0)},
                      {"c0": (0.1, 0, 0, 0), "c1": (0, 0.2, 0, 0)}, sampler_fn=samplers)
        self.assertVec(out["oC0"], (0.25, 0.5, 0.75, 1.0))
        stages = [(c[0], c[2], c[3]["texture_type"]) for c in samplers.calls]
        self.assertEqual(stages, [(0, "texld", "2d"), (1, "texldp", "cube"),
                                  (2, "texldb", "volume"), (0, "texldl", "2d"),
                                  (0, "texldd", "2d")])
        self.assertEqual(samplers.calls[1][1], (0.25, 0.5, 0.75, 1.0))
        self.assertEqual(samplers.calls[2][3]["bias"], 4.0)
        self.assertEqual(samplers.calls[3][3]["lod"], 4.0)
        self.assertEqual(samplers.calls[4][3]["ddx"][0], f32(0.1))
        with self.assertRaises(vm.ShaderRuntimeError):
            self.ps(body, {"texcoord0": (0, 0)})

    def test_derivatives(self):
        body = "dcl_texcoord v0\ndsx r0, v0\ndsy r1, v0\nadd oC0, r0, r1"
        self.assertVec(self.ps(body, {"texcoord0": (1, 2, 3, 4)})["oC0"], (0, 0, 0, 0))

        def derivative(kind, ins, value):
            return [v * (2 if kind == "dsx" else 3) for v in value]
        out = self.ps(body, {"texcoord0": (1, 2, 3, 4)}, derivative_fn=derivative)
        self.assertVec(out["oC0"], (5, 10, 15, 20))

    def test_inputs_outputs_and_semantics(self):
        code = assemble("""vs_3_0
            dcl_position v0
            dcl_texcoord2 v1
            dcl_position o0
            dcl_texcoord1 o1.xy
            dcl_color o2
            mov o0, v0
            mov o1.xy, v1
            mov o2, c0""")
        out = vm.run_vertex(code, {"position": (1, 2, 3), ("texcoord", 2): (0.5, 0.25),
                                   "normal": (0, 0, 1)}, {"c0": (2, -1, 0.5, 1)})
        self.assertVec(out["o0"], (1, 2, 3, 1))           # w padded with 1
        self.assertVec(out["texcoord1"], (0.5, 0.25, 0, 0))
        self.assertEqual(out.semantics["color0"], "o2")
        self.assertEqual(out.ignored_inputs, ["normal"])
        self.assertEqual(vm.run_vertex(code, {}).missing_inputs, ["v0", "v1"])
        # vs_2_0 colours are clamped when fed to the pixel stage.
        code = assemble("vs_2_0\nmov oPos, c0\nmov oD0, c0\nmov oT3, c0")
        out = vm.run_vertex(code, {}, {"c0": (2, -1, 0.5, 1)})
        self.assertEqual(out.as_pixel_inputs()["color0"], (1.0, 0.0, 0.5, 1.0))
        self.assertEqual(out.as_pixel_inputs()["texcoord3"], (2.0, -1.0, 0.5, 1.0))
        ps = vm.run_pixel(assemble("ps_2_0\ndcl t3\nmov oC0, t3"), out.as_pixel_inputs())
        self.assertVec(ps["oC0"], (2, -1, 0.5, 1))
        with self.assertRaises(KeyError):
            vm.run_vertex(code, {"bogus!": 1})
        with self.assertRaises(KeyError):
            out["oT7"]

    def test_vpos_vface_and_output_relative(self):
        code = assemble("""ps_3_0
            dcl vPos.xy
            dcl vFace
            cmp r0, vFace, c0, c1
            add r0.xy, r0, vPos
            mov oC0, r0
            mov oDepth, c0.x""")
        out = vm.run_pixel(code, {"vPos": (10, 20), "vFace": -1.0},
                           {"c0": (1, 1, 1, 1), "c1": (2, 2, 2, 2)})
        self.assertVec(out["oC0"], (12, 22, 2, 2))
        self.assertVec(out["oDepth"], (1, None, None, None))

    def test_constants_by_register_file_and_tuple(self):
        code = assemble("vs_3_0\ndcl_position o0\nif b3\nmov o0, c7\nendif")
        out = vm.run_vertex(code, {}, {"c": {7: (1, 2, 3, 4)}, "b": {3: True}})
        self.assertVec(out["o0"], (1, 2, 3, 4))
        out = vm.run_vertex(code, {}, {("c", 7): (5,), ("b", 3): 1})
        self.assertVec(out["o0"], (5, 0, 0, 0))
        with self.assertRaises(KeyError):
            vm.run_vertex(code, {}, {"cNotInCtab": (1, 2, 3, 4)})

    def test_runaway_loop_guard(self):
        body = "dcl_position o0\ndefi i0, 255, 0, 0, 0\nrep i0\nrep i0\nrep i0\nadd r0, r0, c0\n" \
               "endrep\nendrep\nendrep\nmov o0, r0"
        with self.assertRaises(vm.ShaderRuntimeError):
            self.vs(body, max_steps=10000)


class DecodeTests(unittest.TestCase):

    SAMPLE = """ps_2_x
        def c0, 1, 0.5, -0.25, 0
        dcl v0
        dcl t0.xy
        dcl_2d s0
        texld r0, t0, s0
        mad_sat r1.xyz, r0, v0, -c0_abs
        cmp r0, -r0.wzyx, c0.x, r1
        mov_pp oC0, r0"""

    def test_disassembly_round_trip(self):
        code = assemble(self.SAMPLE)
        text = vm.disassemble(code, header=False)
        again = assemble(re.sub(r"//.*", "", text))
        self.assertEqual(code, again)
        self.assertIn("mad_sat r1.xyz, r0, v0, -c0_abs", text)
        self.assertIn("cmp r0, -r0.wzyx, c0.x, r1", text)

    def test_instruction_lengths_and_defs(self):
        shader = vm.parse(assemble(self.SAMPLE))
        self.assertEqual(shader.profile, "ps_2_x")
        self.assertEqual(shader.float_defs[0], [1.0, 0.5, -0.25, 0.0])
        self.assertEqual(shader.sampler_types(), {0: "2d"})
        self.assertEqual([i.name for i in shader.program], ["tex", "mad", "cmp", "mov"])

    def test_unknown_opcode_rejected(self):
        tokens = list(struct.unpack("<%dI" % (len(assemble(self.SAMPLE)) // 4),
                                    assemble(self.SAMPLE)))
        index = next(i for i, t in enumerate(tokens) if t & 0xFFFF == vm.OP["MAD"] and i > 0
                     and not t & 0x80000000)
        tokens[index] = (tokens[index] & ~0xFFFF) | 0x77
        with self.assertRaisesRegex(vm.ShaderDecodeError, "unknown opcode 0x0077"):
            vm.parse(tokens)

    def test_corrupted_streams_rejected(self):
        code = assemble(self.SAMPLE)
        tokens = list(struct.unpack("<%dI" % (len(code) // 4), code))
        mad = next(i for i, t in enumerate(tokens) if i and t & 0xFFFF == vm.OP["MAD"]
                   and not t & 0x80000000)
        wrong_length = list(tokens)
        wrong_length[mad] = (tokens[mad] & ~(0xF << 24)) | (3 << 24)
        with self.assertRaisesRegex(vm.ShaderDecodeError, "length field"):
            vm.parse(wrong_length)
        with self.assertRaisesRegex(vm.ShaderDecodeError, "truncated|missing end"):
            vm.parse(tokens[:-3])
        with self.assertRaisesRegex(vm.ShaderDecodeError, "version"):
            vm.parse([0x12345678] + tokens[1:])
        with self.assertRaisesRegex(vm.ShaderDecodeError, "multiple of 4"):
            vm.parse(code[:-2])
        with self.assertRaisesRegex(vm.ShaderDecodeError, "lacks bit 31"):
            broken = list(tokens)
            broken[mad + 1] &= 0x7FFFFFFF
            vm.parse(broken)
        with self.assertRaisesRegex(vm.ShaderDecodeError, "after the end"):
            vm.parse(tokens + [0])
        with self.assertRaisesRegex(vm.ShaderDecodeError, "else without if"):
            vm.parse(assemble("vs_2_0\nelse\nmov oPos, c0"))
        with self.assertRaisesRegex(vm.ShaderDecodeError, "unterminated"):
            vm.parse(assemble("vs_2_0\nrep i0\nmov oPos, c0"))

    def test_unimplemented_instruction_is_reported(self):
        shader = vm.parse(assemble("ps_2_0\nmov oC0, c0"))
        shader.program[0].opcode = vm.OP["TEXDEPTH"] + 1000   # unknown to the interpreter
        with self.assertRaises(vm.UnsupportedInstruction):
            vm.run_pixel(shader, {})

    def test_float_formatting(self):
        self.assertEqual(vm.format_float(f32(0.1)), "0.1")
        self.assertEqual(vm.format_float(2.0), "2")
        self.assertEqual(vm.format_float(f32(-0.0001)), "-0.0001")
        self.assertEqual(vm.format_float(-0.0), "-0")


class TextureTests(unittest.TestCase):

    def test_point_bilinear_and_addressing(self):
        texture = tex.Texture2D([[(0, 0, 0, 1), (1, 0, 0, 1)], [(0, 1, 0, 1), (1, 1, 0, 1)]],
                                filter="bilinear", address="clamp")
        self.assertEqual(texture.sample((0.25, 0.25)), [0, 0, 0, 1])
        self.assertEqual(texture.sample((0.5, 0.25)), [0.5, 0, 0, 1])
        self.assertEqual(texture.sample((-3, 0.75)), [0, 1, 0, 1])
        wrap = tex.Texture2D(texture.levels[0], filter="point", address="wrap")
        self.assertEqual(wrap.sample((1.75, 0.25)), [1, 0, 0, 1])
        border = tex.Texture2D(texture.levels[0], filter="point", address="border",
                               border=(0.5, 0.5, 0.5, 0.5))
        self.assertEqual(border.sample((1.5, 0.5)), [0.5, 0.5, 0.5, 0.5])

    def test_srgb_mips_and_cube(self):
        texture = tex.Texture2D.constant((0.5, 0.5, 0.5, 0.5), srgb=True)
        self.assertAlmostEqual(texture.sample((0.3, 0.3))[0], 0.21404114, places=6)
        self.assertEqual(texture.sample((0.3, 0.3))[3], 0.5)
        mipped = tex.Texture2D.constant((1, 1, 1, 1), mips=[[[(0, 0, 0, 0)]]])
        self.assertEqual(mipped.sample((0, 0), {"lod": 0.25})[0], 0.75)
        self.assertEqual(mipped.sample((0, 0), {"bias": 1.0})[0], 0.0)
        cube = tex.TextureCube.from_direction(2, lambda d: (d[0], d[1], d[2], 1))
        for direction, face in (((1, 0.1, 0.2), 0), ((-1, 0, 0), 1), ((0, 1, 0), 2),
                                ((0, -1, 0), 3), ((0, 0, 1), 4), ((0.1, 0, -1), 5)):
            self.assertEqual(tex.cube_face(*direction)[0], face)
        value = cube.sample((0.2, 0.1, 1.0), {"lod": 0})
        self.assertGreater(value[2], 0.5)   # +Z face centre region points along +Z


# -- synthetic .vcs files -------------------------------------------------------

def _combo_block(combos):
    return b"".join(struct.pack("<II", cid, len(code)) + code for cid, code in combos)


def _valve_lzma(payload):
    filters = [{"id": lzma.FILTER_LZMA1, "dict_size": 1 << 16, "lc": 3, "lp": 0, "pb": 2}]
    raw = lzma.compress(payload, format=lzma.FORMAT_RAW, filters=filters)
    props = bytes([(2 * 5 + 0) * 9 + 3]) + struct.pack("<I", 1 << 16)
    return b"LZMA" + struct.pack("<II", len(payload), len(raw)) + props + raw


def build_vcs(static_blocks, dynamic_combos, aliases=(), version=6, total=None):
    """static_blocks: {static id: [(kind, [(combo id, code)])]}."""
    ids = sorted(static_blocks)
    header_size = 28 + 8 * (len(ids) + 1) + (4 + 8 * len(aliases) if version == 6 else 0)
    body = b""
    records = []
    for static_id in ids:
        records.append((static_id, header_size + len(body)))
        for kind, combos in static_blocks[static_id]:
            block = _combo_block(combos)
            if kind == "stored":
                body += struct.pack("<I", 0x80000000 | len(block)) + block
            elif kind == "lzma":
                packed = _valve_lzma(block)
                body += struct.pack("<I", 0x40000000 | len(packed)) + packed
            elif kind == "bzip2":
                packed = bz2.compress(block)
                body += struct.pack("<I", len(packed)) + packed
            elif kind == "bad":
                body += struct.pack("<I", 0xC0000004) + b"\0" * 4
        body += struct.pack("<I", 0xFFFFFFFF)
    records.append((0xFFFFFFFF, header_size + len(body)))
    total = total if total is not None else (max(ids) + 1) * dynamic_combos
    out = struct.pack("<iIiIIII", version, total, dynamic_combos, 0, 0, len(records), 0x1234)
    out += b"".join(struct.pack("<II", *r) for r in records)
    if version == 6:
        out += struct.pack("<i", len(aliases)) + b"".join(struct.pack("<II", *a) for a in aliases)
    return out + body


class VcsFormatTests(unittest.TestCase):

    def code(self, tag):
        return assemble("ps_2_0\ndef c0, %d, 0, 0, 0\nmov oC0, c0" % tag)

    def test_v6_blocks_aliases_and_combo_mapping(self):
        data = build_vcs({
            0: [("stored", [(0, self.code(1)), (2, self.code(2))])],
            2: [("lzma", [(0, self.code(3))]), ("bzip2", [(3, self.code(4))])],
        }, dynamic_combos=4, aliases=[(1, 2)], total=16)
        vcs = sv.VcsFile(data, name="synthetic")
        self.assertEqual(vcs.static_combo_ids(), [0, 1, 2])
        self.assertEqual(vcs.bytecode(0, 2), self.code(2))
        self.assertIsNone(vcs.bytecode(0, 1))            # skipped dynamic combo
        self.assertEqual(vcs.bytecode(8, 0), self.code(3))
        self.assertEqual(vcs.bytecode(8, 3), self.code(4))
        self.assertEqual(vcs.bytecode(4, 3), self.code(4))  # static id 1 aliases id 2
        self.assertIsNone(vcs.bytecode(12, 0))            # static id 3 not stored
        with self.assertRaisesRegex(ValueError, "multiple"):
            vcs.bytecode(5, 0)
        with self.assertRaisesRegex(ValueError, "outside"):
            vcs.bytecode(16, 0)
        with self.assertRaisesRegex(ValueError, "dynamic index"):
            vcs.bytecode(0, 4)

    def test_v5_full_combo_ids(self):
        # Version 5 stores full combo numbers (static index + dynamic index).
        data = build_vcs({1: [("stored", [(4, self.code(7)), (6, self.code(8))])]},
                         dynamic_combos=4, version=5)
        vcs = sv.VcsFile(data)
        self.assertEqual(vcs.bytecode(4, 0), self.code(7))
        self.assertEqual(vcs.bytecode(4, 2), self.code(8))

    def test_corrupt_files_rejected(self):
        data = build_vcs({0: [("bad", [])]}, dynamic_combos=1)
        with self.assertRaisesRegex(sv.VcsError, "compression"):
            sv.VcsFile(data).bytecode(0, 0)
        with self.assertRaisesRegex(sv.VcsError, "version"):
            sv.VcsFile(struct.pack("<i", 9) + b"\0" * 40)
        good = build_vcs({0: [("stored", [(0, self.code(1))])]}, dynamic_combos=1)
        with self.assertRaisesRegex(sv.VcsError, "overrun|off the end|invalid"):
            sv.VcsFile(good[:-12]).bytecode(0, 0)
        wrong = build_vcs({0: [("stored", [(5, self.code(1))])]}, dynamic_combos=2)
        with self.assertRaisesRegex(sv.VcsError, "does not belong"):
            sv.VcsFile(wrong).bytecode(0, 0)

    def test_diff_decoder(self):
        reference = bytes(range(40))
        # raw copy 3 bytes, copy 5 from reference offset +2, long copy of 4 from -4.
        diff = bytes([3, 9, 9, 9, 0x85, 2, 0, 4, 0, 0xFC, 0xFF])
        self.assertEqual(sv.apply_diffs(reference, diff),
                         b"\x09\x09\x09" + reference[2:7] + reference[3:7])


# -- retail content ---------------------------------------------------------------

VPK = sv.find_default_vpk()
SKIP_REASON = "retail hl2_misc_dir.vpk not found (looked in %s)" % ", ".join(
    str(p) for p in sv.default_vpk_paths())
ENGINE_VS_CONSTANTS = {"c0": (0.0, 1.0, 2.0, 0.5)}   # cConstants0, set by the shader API


@unittest.skipIf(VPK is None, SKIP_REASON)
class RetailShaderTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from source_content import VpkDirectory
        cls.vpk = VpkDirectory(str(VPK))

    def load(self, name):
        return sv.VcsFile.from_vpk(self.vpk, name)

    def decode_statics(self, vcs, static_indices):
        combos = 0
        for static_index in static_indices:
            for dyn, code in vcs.dynamic_combos(static_index).items():
                shader = vm.parse(code)
                text = vm.disassemble(shader)
                self.assertIn(shader.profile, text)
                combos += 1
        return combos

    def test_every_combo_of_small_shaders(self):
        counts = {}
        for name in ("unlitgeneric_ps20b", "unlitgeneric_vs20", "screenspaceeffect_vs20",
                     "modulate_ps20b", "water_ps20b", "eyes_ps20b", "teeth_ps20b",
                     "worldvertextransition_vs20", "unlitgeneric_ps11"):
            vcs = self.load(name)
            counts[name] = self.decode_statics(vcs, vcs.static_indices())
            self.assertGreater(counts[name], 0, name)
        self.assertEqual(counts["unlitgeneric_ps20b"], 2)
        self.assertEqual(counts["worldvertextransition_vs20"], 2)   # v2 diff-coded file

    def test_sampled_combos_of_large_shaders(self):
        for name, samples in (("vertexlit_and_unlit_generic_vs20", 4),
                              ("lightmappedgeneric_ps20b", 3)):
            vcs = self.load(name)
            indices = vcs.static_indices()
            step = max(1, len(indices) // samples)
            self.assertGreater(self.decode_statics(vcs, indices[::step][:samples]), 0, name)
        self.assertFalse(self.load("lightmappedgeneric_ps20b").total_combos_reliable)

    def test_combo_lookup_negative(self):
        vcs = self.load("teeth_bump_vs20")
        # USE_STATIC_CONTROL_FLOW (static 96) skips every NUM_LIGHTS > 0 dynamic combo.
        self.assertIsNotNone(vcs.bytecode(96, 0))
        self.assertIsNone(vcs.bytecode(96, 16))
        with self.assertRaises(ValueError):
            vcs.bytecode(97, 0)
        with self.assertRaises(FileNotFoundError):
            self.load("no_such_shader_ps20b")
        missing = sorted(set(range(vcs.num_static_combos)) - set(vcs.static_combo_ids()))
        if missing:
            self.assertIsNone(vcs.bytecode(missing[0] * vcs.header.dynamic_combos, 0))

    def test_cli(self):
        stdout = io.StringIO()
        with contextlib.redirect_stdout(stdout):
            self.assertEqual(sv.main(["unlitgeneric_ps20b", "--vpk", str(VPK), "--static", "0",
                                      "--disasm"]), 0)
        self.assertIn("texld r0, t0, s0", stdout.getvalue())
        self.assertIn("TextureSampler", stdout.getvalue())

    def test_unlitgeneric_ps20b_semantics(self):
        """result = vColor0 * tex2D(TextureSampler, uv) (unlitgeneric_ps2x.fxc),
        and with CONVERT_TO_SRGB the rgb goes through the 1D gamma table (s15)."""
        vcs = self.load("unlitgeneric_ps20b")
        texel = (0.8, 0.4, 0.2, 0.6)
        color = (0.5, 0.25, 1.0, 0.5)
        samplers = tex.SamplerSet({0: tex.Texture2D.constant(texel)})
        out = vm.run_pixel(vcs.bytecode(0, 0), {"texcoord0": (0.3, 0.7), "color0": color}, {},
                           samplers)
        self.assertEqual(out["oC0"], tuple(f32(f32(c) * f32(t)) for c, t in zip(color, texel)))
        self.assertEqual(samplers.calls[0][1][:2], (f32(0.3), f32(0.7)))

        gamma = tex.Texture2D.from_function(256, 1, lambda x, y: ((x / 255.0) ** 0.5,) * 4,
                                            filter="point", address="clamp")
        samplers = tex.SamplerSet({0: tex.Texture2D.constant(texel), 15: gamma})
        out = vm.run_pixel(vcs.bytecode(1, 0), {"texcoord0": (0.3, 0.7), "color0": color}, {},
                           samplers)
        linear = [f32(f32(c) * f32(t)) for c, t in zip(color, texel)]
        expected = [gamma.sample((v, v))[0] for v in linear[:3]]
        for actual, want in zip(out["oC0"][:3], expected):
            self.assertAlmostEqual(actual, want, places=6)
        self.assertEqual([c[0] for c in samplers.calls], [0, 15, 15, 15])

    def test_screenspaceeffect_vs20_semantics(self):
        vcs = self.load("screenspaceeffect_vs20")
        out = vm.run_vertex(vcs.bytecode(0, 0), {"position": (0.25, -0.5, 0.125),
                                                 "texcoord0": (0.1, 0.9)},
                            dict(ENGINE_VS_CONSTANTS, Texel_Sizes=(0, 0, 0.01, 0.02)))
        self.assertEqual(out["oPos"], (0.25, -0.5, 0.125, 1.0))
        self.assertEqual(out["texcoord0"][:2], (f32(0.1), f32(0.9)))
        self.assertEqual(out["texcoord1"][:2], (0.0, 0.0))
        self.assertEqual(out["texcoord2"][:2], (f32(f32(0.1) + f32(0.01)),
                                                f32(f32(0.9) + f32(0.02))))

    @staticmethod
    def vertex_atten(world_pos, light):
        """VertexAttenInternal from common_vs_fxc.h, in double precision."""
        color, direction, pos, spot, atten = light
        to_light = [p - w for p, w in zip(pos, world_pos)]
        dist2 = sum(v * v for v in to_light)
        oo_dist = 1.0 / math.sqrt(dist2)
        to_light = [v * oo_dist for v in to_light]
        v_dist = (1.0, dist2 * oo_dist, dist2)
        distance_atten = 1.0 / sum(a * d for a, d in zip(atten[:3], v_dist))
        cos_theta = -sum(d * l for d, l in zip(direction[:3], to_light))
        spot_atten = max(0.0001, (cos_theta - spot[2]) * spot[3]) ** spot[0]
        spot_atten = min(max(spot_atten, 0.0), 1.0)
        a = distance_atten + (distance_atten * spot_atten - distance_atten) * direction[3]
        return a + (1.0 - a) * color[3]

    def light_constants(self, lights):
        consts = {}
        for index, light in enumerate(lights):
            for row, value in enumerate(light):
                consts["c%d" % (27 + 5 * index + row)] = value
        return consts

    LIGHTS = [
        # color (w=1 directional), dir (w=1 spot), pos, spotParams (exp, _, cos, 1/(1-cos)), atten
        ((1.0, 0.5, 0.25, 0.0), (0.0, 0.0, -1.0, 1.0), (1.0, 2.0, 10.0, 0.0),
         (2.0, 0.0, 0.5, 2.0), (0.0, 0.1, 0.001, 0.0)),
        ((0.2, 0.4, 0.6, 1.0), (0.6, 0.0, -0.8, 0.0), (0.0, 0.0, 0.0, 0.0),
         (1.0, 0.0, 0.0, 1.0), (1.0, 0.0, 0.0, 0.0)),
        ((0.3, 0.3, 0.3, 0.0), (0.0, 0.0, 0.0, 0.0), (-4.0, 1.0, 3.0, 0.0),
         (1.0, 0.0, 0.0, 1.0), (0.5, 0.25, 0.0, 0.0)),
        ((1.0, 1.0, 1.0, 0.0), (0.0, 0.0, 0.0, 0.0), (0.0, 0.0, 5.0, 0.0),
         (1.0, 0.0, 0.0, 1.0), (1.0, 0.0, 0.0, 0.0)),
    ]

    def test_teeth_bump_vs20_static_branches(self):
        """USE_STATIC_CONTROL_FLOW: `if g_bLightEnabled[n]` selects between
        VertexAttenInternal and 0 for each light (teeth_bump_vs20.fxc)."""
        code = self.load("teeth_bump_vs20").bytecode(96, 0)
        self.assertIn("if b0", vm.disassemble(code))
        world = (0.5, -1.0, 2.0)
        consts = dict(ENGINE_VS_CONSTANTS)
        consts.update(self.light_constants(self.LIGHTS))
        consts.update({"cModel": [(1, 0, 0, 0), (0, 1, 0, 0), (0, 0, 1, 0)],
                       "cFlexScale": (0, 0, 0, 0), "cViewProj": [(1, 0, 0, 0), (0, 1, 0, 0),
                                                                 (0, 0, 1, 0), (0, 0, 0, 1)],
                       "cViewProjZ": (0, 0, 1, 0), "cFogParams": (0, 0, 0, 0),
                       "cEyePosWaterZ": (0, 0, 0, 0), "cTeethLighting": (0, 0, 1, 1)})
        inputs = {"position": world + (1.0,), "normal": (0, 0, 1, 0), "tangent": (1, 0, 0, 1),
                  "texcoord0": (0.5, 0.5)}
        for enabled in ((True, False, True, False), (False, True, False, True)):
            consts.update({"b%d" % i: e for i, e in enumerate(enabled)})
            out = vm.run_vertex(code, inputs, consts)
            got = list(out["texcoord6"][:2]) + list(out["texcoord7"][:2])
            for index, on in enumerate(enabled):
                want = self.vertex_atten(world, self.LIGHTS[index]) if on else 0.0
                self.assertAlmostEqual(got[index], want, delta=1e-5 * max(1.0, abs(want)),
                                       msg="light %d enabled=%s" % (index, on))
            self.assertEqual(out["oPos"], world + (1.0,))

    def test_vertexlit_vs30_rep_loop(self):
        """DoLighting: sum over g_nLightCount lights of color * max(N.L, 0) *
        atten plus AmbientLight(N) -- a rep i0 loop with a0-relative light
        constants and a mova-indexed ambient cube."""
        code = self.load("vertexlit_and_unlit_generic_vs30").bytecode(8192, 2)
        text = vm.disassemble(code)
        self.assertIn("rep i0", text)
        world = (0.5, -1.0, 2.0)
        normal = (0.36, -0.48, 0.8)
        ambient = {"cAmbientCubeX": [(0.1, 0, 0, 0), (0.2, 0, 0, 0)],
                   "cAmbientCubeY": [(0, 0.3, 0, 0), (0, 0.4, 0, 0)],
                   "cAmbientCubeZ": [(0, 0, 0.5, 0), (0, 0, 0.6, 0)]}
        consts = dict(ENGINE_VS_CONSTANTS, **ambient)
        consts.update(self.light_constants(self.LIGHTS))
        consts.update({"cModel": [(1, 0, 0, 0), (0, 1, 0, 0), (0, 0, 1, 0)],
                       "cFlexScale": (0, 0, 0, 0),
                       "cViewProj": [(1, 0, 0, 0), (0, 1, 0, 0), (0, 0, 1, 0), (0, 0, 0, 1)],
                       "cViewProjZ": (0, 0, 1, 0), "cFogParams": (0, 0, 0, 0),
                       "cBaseTexCoordTransform": [(1, 0, 0, 0), (0, 1, 0, 0)]})
        inputs = {"position": world + (1.0,), "normal": normal + (0.0,), "texcoord0": (0.25, 0.75)}
        n2 = [n * n for n in normal]
        amb = [n2[0] * ambient["cAmbientCubeX"][normal[0] < 0][i] +
               n2[1] * ambient["cAmbientCubeY"][normal[1] < 0][i] +
               n2[2] * ambient["cAmbientCubeZ"][normal[2] < 0][i] for i in range(3)]
        for count in (0, 1, 2, 4):
            consts["i0"] = (count, 0, 1, 0)
            out = vm.run_vertex(code, inputs, consts)
            expected = list(amb)
            for light in self.LIGHTS[:count]:
                color, direction, pos = light[0], light[1], light[2]
                if color[3]:
                    to_light = [-d for d in direction[:3]]
                else:
                    to_light = [p - w for p, w in zip(pos, world)]
                    length = math.sqrt(sum(v * v for v in to_light))
                    to_light = [v / length for v in to_light]
                ndotl = max(0.0, sum(n * l for n, l in zip(normal, to_light)))
                atten = self.vertex_atten(world, light)
                for i in range(3):
                    expected[i] += color[i] * ndotl * atten
            for actual, want in zip(out["texcoord2"][:3], expected):
                self.assertAlmostEqual(actual, want, delta=2e-6 * max(1.0, abs(want)),
                                       msg="light count %d" % count)
            self.assertEqual(out["texcoord2"][3], 0.0)
            self.assertEqual(out["texcoord0"][:2], (0.25, 0.75))

    def test_broad_smoke_all_fxc_shaders(self):
        """First stored combo of every shipped fxc shader decodes, disassembles
        and runs once on zero inputs with no unsupported instruction."""
        unsupported = {}
        failures = {}
        names = sv.list_vpk_shaders(self.vpk, "fxc")
        self.assertGreater(len(names), 300)
        for name in names:
            vcs = self.load("fxc/" + name)
            first = vcs.first_combo()
            if first is None:
                failures[name] = "no stored combos"
                continue
            try:
                shader = vm.parse(first[2])
                vm.disassemble(shader)
                run = vm.run_pixel if shader.is_pixel else vm.run_vertex
                run(shader, {}, {}, sampler_fn=lambda *args: (0.0, 0.0, 0.0, 0.0))
            except NotImplementedError as exc:
                unsupported[name] = str(exc)
            except Exception as exc:  # noqa: BLE001 - report every failure at once
                failures[name] = repr(exc)
        self.assertEqual(unsupported, {})
        self.assertEqual(failures, {})


if __name__ == "__main__":
    unittest.main()
