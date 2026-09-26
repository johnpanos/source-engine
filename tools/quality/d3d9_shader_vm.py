#!/usr/bin/env python3
"""Decode, disassemble and interpret Direct3D 9 shader bytecode on the CPU.

This is an independent reference oracle for porting Source's stdshader_dx9
shaders to native Vulkan: it runs the shipped fxc-compiled token streams
(see `source_vcs.py`) for chosen inputs so a port can be compared against the
bytecode the retail game actually executed.

    shader = parse(code)                      # Shader: version, CTAB, instructions
    print(disassemble(shader))                # fxc-like text with CTAB names
    vs = run_vertex(code, {"position": (1, 2, 3, 1)}, {"c0": (1, 0, 0, 0)})
    ps = run_pixel(code, {"texcoord0": (0.5, 0.5)},
                   {"cModulationColor": (1, 1, 1, 1)}, sampler_fn)
    ps["oC0"], ps.discarded

Supported: vs_1_1, vs_2_0/2_x, vs_3_0, ps_1_1..ps_1_4, ps_2_0/2_x (ps_2_b is
emitted as ps_2_x), ps_3_0.

Inputs are keyed by register ("v0", "t1", "vPos", "vFace") or by semantic
("position", "normal0", "texcoord1", "color0"; tuples ("texcoord", 1) too).
For ps_1_x/ps_2_x, texcoordN means tN and colorN means vN. Vectors with fewer
than four components are padded with (0, 0, 0, 1); a bare number is
replicated. Declared inputs that are not supplied read as zero and are listed
in `result.missing_inputs`; semantics a shader does not declare are ignored.

Constants: {"c3": (..4..), "i0": (count, start, step, 0), "b1": True}, or
{"c": {3: ...}, "i": {...}, "b": {...}}, or CTAB names ("cModulationColor":
a vector, or a list of register rows for arrays and matrices -- the rows are
the registers, so a column_major float4x4 takes its columns). Instruction
`def`/`defi`/`defb` values override caller constants, as in D3D9.

`sampler_fn(stage, coords, kind, extra)` returns RGBA. `kind` is "texld",
"texldp", "texldb", "texldl", "texldd" or a ps_1_x name ("tex", "texm3x3tex",
...). For texldp the coordinates are already divided by w. `extra` holds
"texture_type" ("2d", "cube", "volume" or None), plus "bias", "lod",
"ddx"/"ddy" and "vertex" (vertex texture fetch) where relevant. See
`d3d9_texture.SamplerSet` for a ready-made implementation.

Semantics (from the D3D9 instruction reference; implementation-defined points
are resolved as noted):

* Every operation result is rounded to IEEE float32 (computed in double and
  rounded once, which is exactly float32 for + - * / sqrt). Multi-step
  instructions round each step as hardware would: mad = round(round(a*b)+c)
  (unfused), dp3/dp4/m4x4 accumulate left to right, lrp = src0*(src1-src2)+src2,
  pow = exp2(src1 * log2(|src0|)), nrm = src * rsq(dp3(src)). Real GPUs may
  fuse multiply-adds and use approximate transcendentals (typically within
  1-2 ulp for rcp/rsq and 2^-21 relative for exp/log), so compare against
  the GPU with a tolerance. Denormals are kept (hardware usually flushes).
* _pp partial precision and _centroid are ignored (full float32 everywhere).
  ps_1_x registers are not clamped to the implementation's [-MaxValue,
  MaxValue]; only texcoord (ps_1_1-1_3) clamps to [0, 1].
* Scalar instructions (rcp, rsq, exp, expp, log, logp, pow, sincos) take the
  swizzled w component, which is the replicate component for SM2+ and the
  vs_1_1 default. rcp(0) = rsq(0) = +inf (rcp(-0) = +inf per the reference
  pseudo-code), rcp(1) = rsq(1) = 1 exactly, rsq and log use |x|,
  log(0) = -inf (the vs_1_1 reference says -FLT_MAX; modern hardware and
  translators return -inf). vs_1_1 expp/logp produce the documented
  four-component partial results; SM2+ expp/logp equal exp/log.
* sincos returns exact cos/sin of the input (x = cos, y = sin, z/w kept); the
  SM2 form's two Taylor-constant sources are ignored. Results outside
  [-pi, pi] are undefined in D3D9 and exact here.
* _sat clamps to [0, 1] with NaN -> 0. min/max/cmp/slt/sge follow the
  reference pseudo-code, so NaN picks the second operand of min and cmp's
  src2.
* mova rounds to nearest, halves away from zero; vs_1_1 `mov a0.x` uses
  floor(x + 0.5) (as Wine does). Non-finite addresses become 0.
  Out-of-range relative constant reads return (0, 0, 0, 0).
* texkill discards when any of x, y, z is negative for ps_1_x and when any
  write-mask component is negative for SM2+ (as the reference rasterizer).
  Execution continues after a discard; `result.discarded` reports it.
* dsx/dsy return zero unless `derivative_fn(kind, instruction, value)` is
  supplied (a single pixel has no neighbours).
* The implicit texture LOD is left to the sampler (d3d9_texture uses 0).
* Unwritten output components read as 0. `ShaderResult.as_pixel_inputs()`
  clamps vs_1_x/vs_2_x oD# to [0, 1] as the rasterizer does.
"""

import argparse
import math
import re
import struct
import sys
from collections import namedtuple

# -- register types (D3DSHADER_PARAM_REGISTER_TYPE) ---------------------------
REG_TEMP = 0
REG_INPUT = 1
REG_CONST = 2
REG_ADDR = 3          # vertex shaders
REG_TEXTURE = 3       # pixel shaders
REG_RASTOUT = 4
REG_ATTROUT = 5
REG_TEXCRDOUT = 6     # vs < 3.0
REG_OUTPUT = 6        # vs_3_0
REG_CONSTINT = 7
REG_COLOROUT = 8
REG_DEPTHOUT = 9
REG_SAMPLER = 10
REG_CONST2 = 11
REG_CONST3 = 12
REG_CONST4 = 13
REG_CONSTBOOL = 14
REG_LOOP = 15
REG_TEMPFLOAT16 = 16
REG_MISCTYPE = 17
REG_LABEL = 18
REG_PREDICATE = 19

CONST_BANK_OFFSET = {REG_CONST: 0, REG_CONST2: 2048, REG_CONST3: 4096, REG_CONST4: 6144}
MAX_FLOAT_CONSTANTS = 8192

# -- source modifiers (D3DSHADER_PARAM_SRCMOD_TYPE) ---------------------------
SRC_NONE, SRC_NEG, SRC_BIAS, SRC_BIASNEG, SRC_SIGN, SRC_SIGNNEG, SRC_COMP, \
    SRC_X2, SRC_X2NEG, SRC_DZ, SRC_DW, SRC_ABS, SRC_ABSNEG, SRC_NOT = range(14)

# -- opcodes (D3DSHADER_INSTRUCTION_OPCODE_TYPE) ------------------------------
OP = dict(
    NOP=0, MOV=1, ADD=2, SUB=3, MAD=4, MUL=5, RCP=6, RSQ=7, DP3=8, DP4=9, MIN=10,
    MAX=11, SLT=12, SGE=13, EXP=14, LOG=15, LIT=16, DST=17, LRP=18, FRC=19,
    M4x4=20, M4x3=21, M3x4=22, M3x3=23, M3x2=24, CALL=25, CALLNZ=26, LOOP=27,
    RET=28, ENDLOOP=29, LABEL=30, DCL=31, POW=32, CRS=33, SGN=34, ABS=35, NRM=36,
    SINCOS=37, REP=38, ENDREP=39, IF=40, IFC=41, ELSE=42, ENDIF=43, BREAK=44,
    BREAKC=45, MOVA=46, DEFB=47, DEFI=48, TEXCOORD=64, TEXKILL=65, TEX=66,
    TEXBEM=67, TEXBEML=68, TEXREG2AR=69, TEXREG2GB=70, TEXM3x2PAD=71,
    TEXM3x2TEX=72, TEXM3x3PAD=73, TEXM3x3TEX=74, TEXM3x3SPEC=76,
    TEXM3x3VSPEC=77, EXPP=78, LOGP=79, CND=80, DEF=81, TEXREG2RGB=82,
    TEXDP3TEX=83, TEXM3x2DEPTH=84, TEXDP3=85, TEXM3x3=86, TEXDEPTH=87, CMP=88,
    BEM=89, DP2ADD=90, DSX=91, DSY=92, TEXLDD=93, SETP=94, TEXLDL=95,
    BREAKP=96, PHASE=0xFFFD, COMMENT=0xFFFE, END=0xFFFF)
OP_NAME = {code: name.lower() for name, code in OP.items()}

# (destination params, source params) for the common case; see _operand_counts.
OPERANDS = {
    OP["NOP"]: (0, 0), OP["MOV"]: (1, 1), OP["ADD"]: (1, 2), OP["SUB"]: (1, 2),
    OP["MAD"]: (1, 3), OP["MUL"]: (1, 2), OP["RCP"]: (1, 1), OP["RSQ"]: (1, 1),
    OP["DP3"]: (1, 2), OP["DP4"]: (1, 2), OP["MIN"]: (1, 2), OP["MAX"]: (1, 2),
    OP["SLT"]: (1, 2), OP["SGE"]: (1, 2), OP["EXP"]: (1, 1), OP["LOG"]: (1, 1),
    OP["LIT"]: (1, 1), OP["DST"]: (1, 2), OP["LRP"]: (1, 3), OP["FRC"]: (1, 1),
    OP["M4x4"]: (1, 2), OP["M4x3"]: (1, 2), OP["M3x4"]: (1, 2), OP["M3x3"]: (1, 2),
    OP["M3x2"]: (1, 2), OP["CALL"]: (0, 1), OP["CALLNZ"]: (0, 2),
    OP["LOOP"]: (0, 2), OP["RET"]: (0, 0), OP["ENDLOOP"]: (0, 0),
    OP["LABEL"]: (0, 1), OP["POW"]: (1, 2), OP["CRS"]: (1, 2), OP["SGN"]: (1, 3),
    OP["ABS"]: (1, 1), OP["NRM"]: (1, 1), OP["SINCOS"]: (1, 3), OP["REP"]: (0, 1),
    OP["ENDREP"]: (0, 0), OP["IF"]: (0, 1), OP["IFC"]: (0, 2), OP["ELSE"]: (0, 0),
    OP["ENDIF"]: (0, 0), OP["BREAK"]: (0, 0), OP["BREAKC"]: (0, 2),
    OP["MOVA"]: (1, 1), OP["TEXCOORD"]: (1, 0), OP["TEXKILL"]: (1, 0),
    OP["TEX"]: (1, 0), OP["TEXBEM"]: (1, 1), OP["TEXBEML"]: (1, 1),
    OP["TEXREG2AR"]: (1, 1), OP["TEXREG2GB"]: (1, 1), OP["TEXM3x2PAD"]: (1, 1),
    OP["TEXM3x2TEX"]: (1, 1), OP["TEXM3x3PAD"]: (1, 1), OP["TEXM3x3TEX"]: (1, 1),
    OP["TEXM3x3SPEC"]: (1, 2), OP["TEXM3x3VSPEC"]: (1, 1), OP["EXPP"]: (1, 1),
    OP["LOGP"]: (1, 1), OP["CND"]: (1, 3), OP["TEXREG2RGB"]: (1, 1),
    OP["TEXDP3TEX"]: (1, 1), OP["TEXM3x2DEPTH"]: (1, 1), OP["TEXDP3"]: (1, 1),
    OP["TEXM3x3"]: (1, 1), OP["TEXDEPTH"]: (1, 0), OP["CMP"]: (1, 3),
    OP["BEM"]: (1, 2), OP["DP2ADD"]: (1, 3), OP["DSX"]: (1, 1), OP["DSY"]: (1, 1),
    OP["TEXLDD"]: (1, 4), OP["SETP"]: (1, 2), OP["TEXLDL"]: (1, 2),
    OP["BREAKP"]: (0, 1), OP["PHASE"]: (0, 0),
}

COMPARISON = {1: "gt", 2: "eq", 3: "ge", 4: "lt", 5: "ne", 6: "le"}
USAGE_NAMES = ["position", "blendweight", "blendindices", "normal", "psize",
               "texcoord", "tangent", "binormal", "tessfactor", "positiont",
               "color", "fog", "depth", "sample"]
SAMPLER_TYPES = {1: "1d", 2: "2d", 3: "cube", 4: "volume"}
SWIZZLE_CHARS = "xyzw"

FLOW_OPS = {OP[n] for n in ("CALL", "CALLNZ", "LOOP", "RET", "ENDLOOP", "LABEL", "REP",
                            "ENDREP", "IF", "IFC", "ELSE", "ENDIF", "BREAK", "BREAKC",
                            "BREAKP")}
DECLARATION_OPS = {OP["DCL"], OP["DEF"], OP["DEFI"], OP["DEFB"]}

CTAB_FOURCC = 0x42415443
REGISTER_SETS = {0: "bool", 1: "int4", 2: "float4", 3: "sampler"}
PARAMETER_CLASSES = {0: "scalar", 1: "vector", 2: "matrix_rows", 3: "matrix_columns",
                     4: "object", 5: "struct"}
PARAMETER_TYPES = {0: "void", 1: "bool", 2: "int", 3: "float", 4: "string",
                   5: "texture", 6: "texture1D", 7: "texture2D", 8: "texture3D",
                   9: "textureCUBE", 10: "sampler", 11: "sampler1D", 12: "sampler2D",
                   13: "sampler3D", 14: "samplerCUBE"}


class ShaderDecodeError(ValueError):
    """The token stream is malformed or uses an unknown opcode."""


class ShaderRuntimeError(RuntimeError):
    """Interpretation failed (missing sampler, runaway loop, bad flow)."""


class UnsupportedInstruction(NotImplementedError):
    """A decoded instruction the interpreter does not implement."""


# -- float32 helpers ----------------------------------------------------------

_F32 = struct.Struct("<f")
INF = math.inf
NAN = math.nan


def f32(value):
    """Round a Python float to the nearest float32 (overflow -> +-inf)."""
    try:
        return _F32.unpack(_F32.pack(value))[0]
    except OverflowError:
        return math.copysign(INF, value)


def _bits_to_float(bits):
    return struct.unpack("<f", struct.pack("<I", bits & 0xFFFFFFFF))[0]


def _float_to_bits(value):
    return struct.unpack("<I", struct.pack("<f", value))[0]


def _div(a, b):
    if b == 0.0:
        if a == 0.0 or math.isnan(a):
            return NAN
        return math.copysign(INF, a) * math.copysign(1.0, b)
    return f32(a / b)


def _mul(a, b):
    return f32(a * b)


def _add(a, b):
    return f32(a + b)


def _dot(a, b, count):
    acc = f32(a[0] * b[0])
    for i in range(1, count):
        acc = f32(acc + f32(a[i] * b[i]))
    return acc


def _floor(x):
    if math.isinf(x) or math.isnan(x):
        return x
    return float(math.floor(x))


def _frc(x):
    if math.isinf(x) or math.isnan(x):
        return NAN
    return f32(x - math.floor(x))


def _exp2(x):
    if math.isnan(x):
        return NAN
    try:
        return f32(math.pow(2.0, x))
    except OverflowError:
        return INF


def _log2_abs(x):
    a = abs(x)
    if math.isnan(a):
        return NAN
    if a == 0.0:
        return -INF
    if math.isinf(a):
        return INF
    return f32(math.log2(a))


def _rcp(x):
    if x == 1.0:
        return 1.0
    if x == 0.0:
        return INF
    return f32(1.0 / x)


def _rsq(x):
    a = abs(x)
    if a == 1.0:
        return 1.0
    if a == 0.0:
        return INF
    if math.isnan(a):
        return NAN
    if math.isinf(a):
        return 0.0
    return f32(1.0 / math.sqrt(a))


def _pow(x, y):
    return _exp2(f32(y * _log2_abs(x)))


def _sat(x):
    if math.isnan(x):
        return 0.0
    return min(max(x, 0.0), 1.0)


def _trig(fn, x):
    if math.isnan(x) or math.isinf(x):
        return NAN
    return f32(fn(x))


def _round_address(x, legacy):
    if math.isnan(x) or math.isinf(x):
        return 0
    if legacy:
        return int(math.floor(x + 0.5))
    return int(math.copysign(math.floor(abs(x) + 0.5), x))


def _compare(kind, a, b):
    if kind == 1:
        return a > b
    if kind == 2:
        return a == b
    if kind == 3:
        return a >= b
    if kind == 4:
        return a < b
    if kind == 5:
        return a != b
    if kind == 6:
        return a <= b
    raise ShaderRuntimeError("bad comparison code %d" % kind)


def _vec4(value, pad=(0.0, 0.0, 0.0, 1.0)):
    if isinstance(value, (int, float)):
        return [f32(float(value))] * 4
    values = [f32(float(v)) for v in value]
    if len(values) > 4:
        raise ValueError("a register holds at most four components, got %d" % len(values))
    return values + [pad[i] for i in range(len(values), 4)]


def format_float(value):
    """Shortest decimal that round-trips the float32 value."""
    if math.isnan(value):
        return "nan"
    if math.isinf(value):
        return "inf" if value > 0 else "-inf"
    if value == int(value) and abs(value) < 1e9:
        return "%d" % value if value or math.copysign(1.0, value) > 0 else "-0"
    for digits in range(1, 10):
        text = "%.*g" % (digits, value)
        if f32(float(text)) == value:
            return text
    return repr(value)


# -- decoded structures -------------------------------------------------------

RelAddr = namedtuple("RelAddr", "type num component")


class DestParam:
    __slots__ = ("type", "num", "mask", "saturate", "partial", "centroid", "shift", "rel")

    def __init__(self, token, rel=None):
        self.type = ((token >> 28) & 0x7) | ((token >> 8) & 0x18)
        self.num = token & 0x7FF
        self.mask = tuple(bool(token & (1 << (16 + i))) for i in range(4))
        self.saturate = bool(token & (1 << 20))
        self.partial = bool(token & (1 << 21))
        self.centroid = bool(token & (1 << 22))
        shift = (token >> 24) & 0xF
        self.shift = shift - 16 if shift & 0x8 else shift
        self.rel = rel


class SrcParam:
    __slots__ = ("type", "num", "swizzle", "modifier", "rel")

    def __init__(self, token, rel=None):
        self.type = ((token >> 28) & 0x7) | ((token >> 8) & 0x18)
        self.num = token & 0x7FF
        self.swizzle = tuple((token >> (16 + 2 * i)) & 3 for i in range(4))
        self.modifier = (token >> 24) & 0xF
        self.rel = rel


class Declaration:
    __slots__ = ("type", "num", "usage", "usage_index", "sampler_type", "mask", "centroid",
                 "partial")

    def __init__(self, dcl_token, dst):
        self.type = dst.type
        self.num = dst.num
        self.usage = dcl_token & 0x1F
        self.usage_index = (dcl_token >> 16) & 0xF
        self.sampler_type = (dcl_token >> 27) & 0xF
        self.mask = dst.mask
        self.centroid = dst.centroid
        self.partial = dst.partial

    @property
    def usage_name(self):
        return USAGE_NAMES[self.usage] if self.usage < len(USAGE_NAMES) else "usage%d" % self.usage

    @property
    def semantic(self):
        return "%s%d" % (self.usage_name, self.usage_index)


class Instruction:
    __slots__ = ("opcode", "control", "predicated", "coissue", "dst", "pred", "srcs",
                 "values", "decl", "offset", "index")

    def __init__(self, opcode, control, predicated, coissue, offset):
        self.opcode = opcode
        self.control = control
        self.predicated = predicated
        self.coissue = coissue
        self.dst = None
        self.pred = None
        self.srcs = []
        self.values = None
        self.decl = None
        self.offset = offset
        self.index = -1

    @property
    def name(self):
        return OP_NAME.get(self.opcode, "op%d" % self.opcode)


class ConstantInfo:
    __slots__ = ("name", "register_set", "index", "count", "type_class", "type", "rows",
                 "columns", "elements", "default")

    def __init__(self, **fields):
        for key, value in fields.items():
            setattr(self, key, value)

    @property
    def register_prefix(self):
        return {0: "b", 1: "i", 2: "c", 3: "s"}.get(self.register_set, "?")

    @property
    def type_name(self):
        base = PARAMETER_TYPES.get(self.type, "type%d" % self.type)
        if self.type_class == 5:
            base = "struct"
        if self.type_class == 1:
            base += "%d" % self.columns
        elif self.type_class in (2, 3):
            base += "%dx%d" % (self.rows, self.columns)
        if self.type_class == 2:
            base = "row_major " + base
        if self.elements > 1:
            return "%s %s[%d]" % (base, self.name, self.elements)
        return "%s %s" % (base, self.name)


class Shader:
    """A decoded shader: version, CTAB, declarations and instructions."""

    def __init__(self, data):
        if isinstance(data, (bytes, bytearray, memoryview)):
            data = bytes(data)
            if len(data) % 4:
                raise ShaderDecodeError("bytecode length %d is not a multiple of 4" % len(data))
            self.tokens = list(struct.unpack("<%dI" % (len(data) // 4), data))
        else:
            self.tokens = [t & 0xFFFFFFFF for t in data]
        if not self.tokens:
            raise ShaderDecodeError("empty token stream")
        version = self.tokens[0]
        kind = version >> 16
        if kind == 0xFFFF:
            self.kind = "ps"
        elif kind == 0xFFFE:
            self.kind = "vs"
        else:
            raise ShaderDecodeError("bad version token 0x%08x" % version)
        self.major = (version >> 8) & 0xFF
        self.minor = version & 0xFF
        if (self.major, self.minor) not in ((1, 0), (1, 1), (1, 2), (1, 3), (1, 4), (2, 0),
                                            (2, 1), (3, 0)):
            raise ShaderDecodeError("unsupported shader version %d.%d" % (self.major, self.minor))
        self.instructions = []
        self.comments = []
        self.constants = []
        self.ctab_creator = None
        self.ctab_target = None
        self.declarations = []
        self.float_defs = {}
        self.int_defs = {}
        self.bool_defs = {}
        self._decode()
        self._analyse_flow()

    # -- properties ----------------------------------------------------------
    @property
    def profile(self):
        minor = "x" if self.minor == 1 and self.major == 2 else str(self.minor)
        return "%s_%d_%s" % (self.kind, self.major, minor)

    @property
    def is_pixel(self):
        return self.kind == "ps"

    def inputs(self):
        types = (REG_INPUT, REG_MISCTYPE) + ((REG_TEXTURE,) if self.is_pixel else ())
        return [d for d in self.declarations if d.type in types]

    def outputs(self):
        if self.is_pixel:
            return []
        return [d for d in self.declarations if d.type == REG_OUTPUT]

    def sampler_types(self):
        return {d.num: SAMPLER_TYPES.get(d.sampler_type) for d in self.declarations
                if d.type == REG_SAMPLER}

    def constant(self, name):
        for info in self.constants:
            if info.name == name:
                return info
        return None

    def constant_for_register(self, register_set, index):
        """(ConstantInfo, element offset) covering a register, or (None, 0)."""
        for info in self.constants:
            if info.register_set == register_set and info.index <= index < info.index + info.count:
                return info, index - info.index
        return None, 0

    # -- decoding --------------------------------------------------------------
    def _decode(self):
        tokens = self.tokens
        count = len(tokens)
        pos = 1
        sm1 = self.major < 2
        extra_rel_token = self.major >= 2

        def take():
            nonlocal pos
            if pos >= count:
                raise ShaderDecodeError("token stream truncated at token %d" % pos)
            token = tokens[pos]
            pos += 1
            return token

        def read_rel(param_token):
            if not param_token & (1 << 13):
                return None
            if not extra_rel_token:
                return RelAddr(REG_ADDR, 0, 0)   # vs_1_1: implicit a0.x
            token = take()
            if not token & 0x80000000:
                raise ShaderDecodeError("relative address token 0x%08x lacks bit 31" % token)
            reg_type = ((token >> 28) & 0x7) | ((token >> 8) & 0x18)
            return RelAddr(reg_type, token & 0x7FF, (token >> 16) & 3)

        def read_dst():
            token = take()
            if not token & 0x80000000:
                raise ShaderDecodeError("parameter token 0x%08x at %d lacks bit 31"
                                        % (token, pos - 1))
            return DestParam(token, read_rel(token))

        def read_src():
            token = take()
            if not token & 0x80000000:
                raise ShaderDecodeError("parameter token 0x%08x at %d lacks bit 31"
                                        % (token, pos - 1))
            return SrcParam(token, read_rel(token))

        while True:
            if pos >= count:
                raise ShaderDecodeError("missing end token")
            start = pos
            token = take()
            if token == 0x0000FFFF:
                if pos != count:
                    # fxc streams end exactly here; trailing data means corruption.
                    raise ShaderDecodeError("%d tokens after the end token" % (count - pos))
                break
            opcode = token & 0xFFFF
            if opcode == OP["COMMENT"]:
                length = (token >> 16) & 0x7FFF
                if pos + length > count:
                    raise ShaderDecodeError("comment at token %d overruns the stream" % start)
                payload = tokens[pos:pos + length]
                pos += length
                self._comment(payload)
                continue
            if token & 0x80000000:
                raise ShaderDecodeError("instruction token 0x%08x at %d has bit 31 set"
                                        % (token, start))
            if opcode not in OPERANDS and opcode not in DECLARATION_OPS:
                raise ShaderDecodeError("unknown opcode 0x%04x at token %d" % (opcode, start))
            ins = Instruction(opcode, (token >> 16) & 0xFF, bool(token & (1 << 28)),
                              bool(token & (1 << 30)), start)
            length = (token >> 24) & 0xF
            if opcode == OP["DCL"]:
                dcl_token = take()
                ins.dst = read_dst()
                ins.decl = Declaration(dcl_token, ins.dst)
                self.declarations.append(ins.decl)
            elif opcode == OP["DEF"]:
                ins.dst = read_dst()
                ins.values = [_bits_to_float(take()) for _ in range(4)]
                self.float_defs[ins.dst.num + CONST_BANK_OFFSET.get(ins.dst.type, 0)] = ins.values
            elif opcode == OP["DEFI"]:
                ins.dst = read_dst()
                ins.values = [struct.unpack("<i", struct.pack("<I", take()))[0] for _ in range(4)]
                self.int_defs[ins.dst.num] = ins.values
            elif opcode == OP["DEFB"]:
                ins.dst = read_dst()
                ins.values = [bool(take())]
                self.bool_defs[ins.dst.num] = ins.values[0]
            else:
                dst_count, src_count = self._operand_counts(opcode)
                if dst_count:
                    ins.dst = read_dst()
                if ins.predicated:
                    ins.pred = read_src()
                ins.srcs = [read_src() for _ in range(src_count)]
            if not sm1 and pos - start - 1 != length:
                raise ShaderDecodeError(
                    "%s at token %d: length field %d but %d operand tokens decoded"
                    % (ins.name, start, length, pos - start - 1))
            self.instructions.append(ins)

    def _operand_counts(self, opcode):
        dst, src = OPERANDS[opcode]
        if opcode == OP["TEX"]:
            if self.major >= 2:
                return 1, 2
            return (1, 1) if self.minor >= 4 else (1, 0)
        if opcode == OP["TEXCOORD"] and self.major == 1 and self.minor >= 4:
            return 1, 1
        if opcode in (OP["SINCOS"], OP["SGN"]) and self.major >= 3:
            return 1, 1
        return dst, src

    def _comment(self, payload):
        data = struct.pack("<%dI" % len(payload), *payload)
        self.comments.append(data)
        if not payload or payload[0] != CTAB_FOURCC:
            return
        ctab = data[4:]
        try:
            (_size, creator, _version, num_constants, info_offset, _flags,
             target) = struct.unpack_from("<7I", ctab, 0)
            self.ctab_creator = _cstring(ctab, creator)
            self.ctab_target = _cstring(ctab, target)
            for i in range(num_constants):
                (name, register_set, index, reg_count, _reserved, type_info,
                 default) = struct.unpack_from("<IHHHHII", ctab, info_offset + 20 * i)
                (type_class, type_, rows, columns, elements, _members,
                 _member_info) = struct.unpack_from("<HHHHHHI", ctab, type_info)
                default_values = None
                if default and register_set in (1, 2):
                    floats = struct.unpack_from("<%df" % (4 * reg_count), ctab, default)
                    default_values = [list(floats[j:j + 4]) for j in range(0, len(floats), 4)]
                self.constants.append(ConstantInfo(
                    name=_cstring(ctab, name), register_set=register_set, index=index,
                    count=reg_count, type_class=type_class, type=type_, rows=rows,
                    columns=columns, elements=elements, default=default_values))
        except struct.error as exc:
            raise ShaderDecodeError("malformed CTAB comment: %s" % exc)

    def _analyse_flow(self):
        """Executable program (declarations removed) plus matching tables."""
        self.program = [ins for ins in self.instructions if ins.opcode not in DECLARATION_OPS]
        self.jump = {}
        self.labels = {}
        stack = []
        for index, ins in enumerate(self.program):
            ins.index = index
            op = ins.opcode
            if op in (OP["IF"], OP["IFC"], OP["REP"], OP["LOOP"]):
                stack.append(index)
            elif op == OP["ELSE"]:
                if not stack or self.program[stack[-1]].opcode not in (OP["IF"], OP["IFC"]):
                    raise ShaderDecodeError("else without if at instruction %d" % index)
                self.jump[stack[-1]] = index
                stack[-1] = index
            elif op == OP["ENDIF"]:
                if not stack or self.program[stack[-1]].opcode not in (OP["IF"], OP["IFC"],
                                                                       OP["ELSE"]):
                    raise ShaderDecodeError("endif without if at instruction %d" % index)
                self.jump[stack.pop()] = index
            elif op in (OP["ENDREP"], OP["ENDLOOP"]):
                opener = OP["REP"] if op == OP["ENDREP"] else OP["LOOP"]
                if not stack or self.program[stack[-1]].opcode != opener:
                    raise ShaderDecodeError("%s without its opener at instruction %d"
                                            % (ins.name, index))
                start = stack.pop()
                self.jump[start] = index
                self.jump[index] = start
            elif op == OP["LABEL"]:
                self.labels[ins.srcs[0].num] = index
        if stack:
            raise ShaderDecodeError("unterminated %s" % self.program[stack[-1]].name)


def _cstring(data, offset):
    end = data.index(b"\0", offset)
    return data[offset:end].decode("latin-1")


def parse(code):
    """Decode bytecode (bytes or a token list); a Shader passes through."""
    return code if isinstance(code, Shader) else Shader(code)


# -- disassembly --------------------------------------------------------------

def register_name(shader, reg_type, num):
    if reg_type == REG_TEMP:
        return "r%d" % num
    if reg_type == REG_INPUT:
        return "v%d" % num
    if reg_type in CONST_BANK_OFFSET:
        return "c%d" % (num + CONST_BANK_OFFSET[reg_type])
    if reg_type == 3:
        return "t%d" % num if shader.is_pixel else "a%d" % num
    if reg_type == REG_RASTOUT:
        return ("oPos", "oFog", "oPts")[num] if num < 3 else "rastout%d" % num
    if reg_type == REG_ATTROUT:
        return "oD%d" % num
    if reg_type == 6:
        return "o%d" % num if shader.major >= 3 else "oT%d" % num
    if reg_type == REG_CONSTINT:
        return "i%d" % num
    if reg_type == REG_COLOROUT:
        return "oC%d" % num
    if reg_type == REG_DEPTHOUT:
        return "oDepth"
    if reg_type == REG_SAMPLER:
        return "s%d" % num
    if reg_type == REG_CONSTBOOL:
        return "b%d" % num
    if reg_type == REG_LOOP:
        return "aL"
    if reg_type == REG_TEMPFLOAT16:
        return "half%d" % num
    if reg_type == REG_MISCTYPE:
        return ("vPos", "vFace")[num] if num < 2 else "misc%d" % num
    if reg_type == REG_LABEL:
        return "l%d" % num
    if reg_type == REG_PREDICATE:
        return "p%d" % num
    return "reg%d_%d" % (reg_type, num)


def _swizzle_text(swizzle):
    if swizzle == (0, 1, 2, 3):
        return ""
    chars = [SWIZZLE_CHARS[c] for c in swizzle]
    while len(chars) > 1 and chars[-1] == chars[-2]:
        chars.pop()
    return "." + "".join(chars)


def _rel_text(shader, rel):
    name = register_name(shader, rel.type, rel.num)
    return name if rel.type == REG_LOOP else "%s.%s" % (name, SWIZZLE_CHARS[rel.component])


def _register_text(shader, reg_type, num, rel):
    name = register_name(shader, reg_type, num)
    if rel is None:
        return name
    base = re.match(r"[a-zA-Z]+", name).group(0)
    offset = num + CONST_BANK_OFFSET.get(reg_type, 0)
    if offset:
        return "%s[%s + %d]" % (base, _rel_text(shader, rel), offset)
    return "%s[%s]" % (base, _rel_text(shader, rel))


def _dst_text(shader, dst, show_mask=True):
    text = _register_text(shader, dst.type, dst.num, dst.rel)
    if show_mask and dst.mask != (True, True, True, True):
        text += "." + "".join(SWIZZLE_CHARS[i] for i in range(4) if dst.mask[i])
    return text


def _src_text(shader, src):
    text = _register_text(shader, src.type, src.num, src.rel) + _swizzle_text(src.swizzle)
    mod = src.modifier
    if mod in (SRC_NEG,):
        return "-" + text
    if mod == SRC_BIAS:
        return text + "_bias"
    if mod == SRC_BIASNEG:
        return "-" + text + "_bias"
    if mod == SRC_SIGN:
        return text + "_bx2"
    if mod == SRC_SIGNNEG:
        return "-" + text + "_bx2"
    if mod == SRC_COMP:
        return "1 - " + text
    if mod == SRC_X2:
        return text + "_x2"
    if mod == SRC_X2NEG:
        return "-" + text + "_x2"
    if mod == SRC_DZ:
        return text + "_dz"
    if mod == SRC_DW:
        return text + "_dw"
    if mod == SRC_ABS:
        return text + "_abs"
    if mod == SRC_ABSNEG:
        return "-" + text + "_abs"
    if mod == SRC_NOT:
        return "!" + text
    return text


def instruction_name(shader, ins):
    op = ins.opcode
    name = ins.name
    if op == OP["TEX"]:
        if shader.major >= 2:
            name = {0: "texld", 1: "texldp", 2: "texldb"}.get(ins.control & 3, "texld?")
        elif shader.minor >= 4:
            name = "texld"
    elif op == OP["TEXCOORD"] and shader.major == 1 and shader.minor >= 4:
        name = "texcrd"
    elif op in (OP["IFC"], OP["BREAKC"], OP["SETP"]):
        name += "_" + COMPARISON.get(ins.control & 7, "?%d" % (ins.control & 7))
    elif op == OP["DCL"]:
        decl = ins.decl
        if decl.type == REG_SAMPLER:
            name = "dcl_" + SAMPLER_TYPES.get(decl.sampler_type, "type%d" % decl.sampler_type)
        elif (shader.is_pixel and shader.major < 3) or decl.type == REG_MISCTYPE:
            name = "dcl"
        else:
            name = "dcl_" + decl.usage_name + (str(decl.usage_index) if decl.usage_index else "")
    dst = ins.dst
    if dst is not None:
        if dst.shift:
            name += {1: "_x2", 2: "_x4", 3: "_x8", -1: "_d2", -2: "_d4", -3: "_d8"}.get(
                dst.shift, "_shift%d" % dst.shift)
        if dst.saturate:
            name += "_sat"
        if dst.partial:
            name += "_pp"
        if dst.centroid:
            name += "_centroid"
    return name


def _operand_comments(shader, ins):
    notes = []
    for src in ins.srcs:
        if src.type in CONST_BANK_OFFSET:
            reg_set, index, prefix = 2, src.num + CONST_BANK_OFFSET[src.type], "c"
        elif src.type == REG_CONSTINT:
            reg_set, index, prefix = 1, src.num, "i"
        elif src.type == REG_CONSTBOOL:
            reg_set, index, prefix = 0, src.num, "b"
        elif src.type == REG_SAMPLER:
            reg_set, index, prefix = 3, src.num, "s"
        else:
            continue
        info, element = shader.constant_for_register(reg_set, index)
        if info is None:
            continue
        label = info.name if info.count == 1 else "%s[%d]" % (info.name, element)
        note = "%s%d = %s" % (prefix, index, label)
        if src.rel is not None:
            # Register rows, not HLSL elements: a struct/matrix element spans
            # several registers.
            note = "%s[%s + %d] = %s[%s + %d]" % (prefix, _rel_text(shader, src.rel), index,
                                                  info.name, _rel_text(shader, src.rel), element)
        if note not in notes:
            notes.append(note)
    if ins.opcode == OP["TEX"] and shader.major == 1 and ins.dst is not None:
        info, _ = shader.constant_for_register(3, ins.dst.num)
        if info is not None:
            notes.append("s%d = %s" % (ins.dst.num, info.name))
    return notes


def format_instruction(shader, ins, comments=True):
    op = ins.opcode
    name = instruction_name(shader, ins)
    if op == OP["DEF"]:
        text = "def %s, %s" % (_dst_text(shader, ins.dst, False),
                               ", ".join(format_float(v) for v in ins.values))
    elif op == OP["DEFI"]:
        text = "defi %s, %s" % (_dst_text(shader, ins.dst, False),
                                ", ".join(str(v) for v in ins.values))
    elif op == OP["DEFB"]:
        text = "defb %s, %s" % (_dst_text(shader, ins.dst, False),
                                "true" if ins.values[0] else "false")
    else:
        operands = []
        if ins.dst is not None:
            show_mask = not (op == OP["DCL"] and ins.decl.type == REG_SAMPLER)
            operands.append(_dst_text(shader, ins.dst, show_mask))
        operands.extend(_src_text(shader, src) for src in ins.srcs)
        text = name + (" " + ", ".join(operands) if operands else "")
    if ins.pred is not None:
        text = "(%s) %s" % (_src_text(shader, ins.pred), text)
    if ins.coissue:
        text = "+" + text
    if comments:
        notes = _operand_comments(shader, ins)
        if notes:
            text = "%-40s // %s" % (text, ", ".join(notes))
    return text


def disassemble(code, header=True):
    """fxc-style listing with the CTAB parameter table as a header comment."""
    shader = parse(code)
    lines = []
    if header:
        if shader.ctab_creator:
            lines.append("//")
            lines.append("// Generated by %s" % shader.ctab_creator)
        if shader.constants:
            lines.append("//")
            lines.append("// Parameters:")
            lines.append("//")
            for info in sorted(shader.constants, key=lambda c: c.name.lower()):
                lines.append("//   %s;" % info.type_name)
            lines.append("//")
            lines.append("//")
            lines.append("// Registers:")
            lines.append("//")
            width = max(max(len(c.name) for c in shader.constants), 4)
            lines.append("//   %-*s Reg   Size" % (width, "Name"))
            lines.append("//   %s ----- ----" % ("-" * width))
            for info in sorted(shader.constants, key=lambda c: (c.register_set, c.index)):
                lines.append("//   %-*s %-5s %4d" % (width, info.name, "%s%d" % (
                    info.register_prefix, info.index), info.count))
        lines.append("//")
        lines.append("")
    lines.append("    " + shader.profile)
    depth = 0
    for ins in shader.instructions:
        if ins.opcode in (OP["ENDIF"], OP["ENDREP"], OP["ENDLOOP"], OP["ELSE"]):
            depth = max(depth - 1, 0)
        lines.append("    " + "  " * depth + format_instruction(shader, ins))
        if ins.opcode in (OP["IF"], OP["IFC"], OP["REP"], OP["LOOP"], OP["ELSE"]):
            depth += 1
    lines.append("")
    lines.append("// approximately %d instruction slots used" % len(shader.program))
    return "\n".join(lines)


# -- interpretation -----------------------------------------------------------

class ShaderResult:
    """Outputs of one invocation. Index by register name ("oPos", "oT0",
    "o3", "oC0", "oDepth") or by semantic ("position0", "texcoord1")."""

    def __init__(self, shader):
        self.shader = shader
        self.registers = {}
        self.semantics = {}
        self.discarded = False
        self.steps = 0
        self.missing_inputs = []
        self.ignored_inputs = []

    def __getitem__(self, key):
        if key in self.registers:
            return self.registers[key]
        if key in self.semantics:
            return self.registers[self.semantics[key]]
        raise KeyError("%s is not written by this %s (written: %s)"
                       % (key, self.shader.profile, ", ".join(sorted(self.registers))))

    def __contains__(self, key):
        return key in self.registers or key in self.semantics

    def get(self, key, default=None):
        try:
            return self[key]
        except KeyError:
            return default

    def as_pixel_inputs(self):
        """{semantic: value} to feed run_pixel; oD# is clamped to [0, 1] for
        vs < 3.0 as the fixed-function rasterizer does."""
        out = {}
        for semantic, register in self.semantics.items():
            if semantic.startswith("position"):
                continue
            value = self.registers[register]
            if self.shader.major < 3 and register.startswith("oD"):
                value = tuple(_sat(v) for v in value)
            out[semantic] = value
        return out


class _LoopFrame:
    __slots__ = ("is_loop", "start", "end", "remaining", "counter", "step")

    def __init__(self, is_loop, start, end, remaining, counter, step):
        self.is_loop = is_loop
        self.start = start
        self.end = end
        self.remaining = remaining
        self.counter = counter
        self.step = step


class _Machine:
    def __init__(self, shader, inputs, consts, sampler_fn, derivative_fn, bump_env,
                 max_steps):
        self.shader = shader
        self.pixel = shader.is_pixel
        self.major = shader.major
        self.minor = shader.minor
        self.sampler_fn = sampler_fn
        self.derivative_fn = derivative_fn
        self.bump_env = bump_env or {}
        self.max_steps = max_steps
        self.sampler_types = shader.sampler_types()
        self.regs = {}
        self.float_consts = {}
        self.int_consts = {}
        self.bool_consts = {}
        self.addr = [0, 0, 0, 0]
        self.pred = [False, False, False, False]
        self.loops = []
        self.texcoords = {}
        self.tex_pad = []
        self.result = ShaderResult(shader)
        self._load_constants(consts or {})
        self._load_inputs(inputs or {})

    # -- setup -----------------------------------------------------------------
    def _load_constants(self, consts):
        for key, value in consts.items():
            if key in ("c", "i", "b") and isinstance(value, dict):
                for index, item in value.items():
                    self._set_constant(key, int(index), item)
                continue
            if isinstance(key, tuple):
                self._set_constant(key[0], int(key[1]), value)
                continue
            match = re.fullmatch(r"([cib])(\d+)", key)
            if match:
                self._set_constant(match.group(1), int(match.group(2)), value)
                continue
            info = self.shader.constant(key)
            if info is None:
                raise KeyError("constant %r is not in this shader's CTAB (%s)"
                               % (key, ", ".join(c.name for c in self.shader.constants)))
            if info.register_set == 3:
                raise KeyError("%r is a sampler; bind it through sampler_fn (s%d)"
                               % (key, info.index))
            prefix = {0: "b", 1: "i", 2: "c"}[info.register_set]
            rows = self._rows(value, info)
            for row, item in enumerate(rows[:info.count]):
                self._set_constant(prefix, info.index + row, item)
        for index, values in self.shader.float_defs.items():
            self.float_consts[index] = [f32(v) for v in values]
        for index, values in self.shader.int_defs.items():
            self.int_consts[index] = list(values)
        for index, value in self.shader.bool_defs.items():
            self.bool_consts[index] = value

    @staticmethod
    def _rows(value, info):
        if info.register_set == 0:
            return value if isinstance(value, (list, tuple)) else [value]
        if isinstance(value, (int, float)):
            return [[value]]
        items = list(value)
        if items and isinstance(items[0], (list, tuple)):
            return items
        return [items[i:i + 4] for i in range(0, len(items), 4)]

    def _set_constant(self, prefix, index, value):
        if prefix == "c":
            self.float_consts[index] = _vec4(value, (0.0, 0.0, 0.0, 0.0))
        elif prefix == "i":
            values = [value] if isinstance(value, (int, float)) else list(value)
            values = [int(v) for v in values] + [0] * (4 - len(values))
            self.int_consts[index] = values[:4]
        elif prefix == "b":
            self.bool_consts[index] = bool(value)
        else:
            raise KeyError("unknown constant register file %r" % prefix)

    def _input_register(self, key):
        """(register type, number) for an input key, or None if undeclared."""
        if isinstance(key, tuple):
            key = "%s%d" % (key[0], key[1])
        if key == "vPos":
            return (REG_MISCTYPE, 0)
        if key == "vFace":
            return (REG_MISCTYPE, 1)
        match = re.fullmatch(r"([vt])(\d+)", key)
        if match:
            if match.group(1) == "t" and not self.pixel:
                raise KeyError("vertex shaders have no t# inputs (%r)" % key)
            return (REG_TEXTURE if match.group(1) == "t" else REG_INPUT, int(match.group(2)))
        match = re.fullmatch(r"([a-z]+?)(\d*)", key.lower())
        if not match or match.group(1) not in USAGE_NAMES:
            raise KeyError("unrecognised input %r" % key)
        usage = USAGE_NAMES.index(match.group(1))
        index = int(match.group(2) or 0)
        if self.pixel and self.major < 3:
            if match.group(1) == "texcoord":
                return (REG_TEXTURE, index)
            if match.group(1) == "color":
                return (REG_INPUT, index)
            return None
        for decl in self.shader.inputs():
            if decl.usage == usage and decl.usage_index == index and decl.type == REG_INPUT:
                return (decl.type, decl.num)
        return None

    def _load_inputs(self, inputs):
        for key, value in inputs.items():
            register = self._input_register(key)
            if register is None:
                self.result.ignored_inputs.append(key)
                continue
            self.regs[register] = _vec4(value)
        for decl in self.shader.inputs():
            if (decl.type, decl.num) not in self.regs:
                self.result.missing_inputs.append(register_name(self.shader, decl.type, decl.num))
        if self.pixel and self.major == 1:
            for n in range(8):
                self.texcoords[n] = list(self.regs.get((REG_TEXTURE, n), [0.0] * 4))
            for n in range(2):
                if (REG_INPUT, n) in self.regs:
                    self.regs[(REG_INPUT, n)] = [_sat(v) for v in self.regs[(REG_INPUT, n)]]

    # -- operand access ----------------------------------------------------------
    def _loop_counter(self):
        for frame in reversed(self.loops):
            if frame.is_loop:
                return frame.counter
        raise ShaderRuntimeError("aL used outside a loop")

    def _rel_offset(self, rel):
        if rel.type == REG_LOOP:
            return self._loop_counter()
        if rel.type == REG_ADDR and not self.pixel:
            return self.addr[rel.component]
        raise ShaderRuntimeError("unsupported relative address register type %d" % rel.type)

    def _fetch(self, reg_type, num, rel=None, row=0):
        num += row
        if rel is not None:
            num += self._rel_offset(rel)
        if reg_type in CONST_BANK_OFFSET:
            index = num + CONST_BANK_OFFSET[reg_type]
            if not 0 <= index < MAX_FLOAT_CONSTANTS:
                return [0.0, 0.0, 0.0, 0.0]
            return self.float_consts.get(index, [0.0, 0.0, 0.0, 0.0])
        if reg_type == REG_CONSTINT:
            return [float(v) for v in self.int_consts.get(num, [0, 0, 0, 0])]
        if reg_type == REG_CONSTBOOL:
            value = 1.0 if self.bool_consts.get(num, False) else 0.0
            return [value] * 4
        if reg_type == REG_LOOP:
            return [float(self._loop_counter())] * 4
        if reg_type == REG_ADDR and not self.pixel:
            return [float(v) for v in self.addr]
        if reg_type == REG_PREDICATE:
            return [1.0 if p else 0.0 for p in self.pred]
        if reg_type == REG_SAMPLER:
            raise ShaderRuntimeError("sampler s%d read as a value" % num)
        return self.regs.get((reg_type, num), [0.0, 0.0, 0.0, 0.0])

    def _src(self, src, row=0):
        value = self._fetch(src.type, src.num, src.rel, row)
        v = [value[c] for c in src.swizzle]
        mod = src.modifier
        if mod == SRC_NONE or mod == SRC_NOT:
            return v
        if mod == SRC_NEG:
            return [-x for x in v]
        if mod == SRC_BIAS:
            return [f32(x - 0.5) for x in v]
        if mod == SRC_BIASNEG:
            return [-f32(x - 0.5) for x in v]
        if mod == SRC_SIGN:
            return [f32(2.0 * f32(x - 0.5)) for x in v]
        if mod == SRC_SIGNNEG:
            return [-f32(2.0 * f32(x - 0.5)) for x in v]
        if mod == SRC_COMP:
            return [f32(1.0 - x) for x in v]
        if mod == SRC_X2:
            return [f32(2.0 * x) for x in v]
        if mod == SRC_X2NEG:
            return [-f32(2.0 * x) for x in v]
        if mod == SRC_DZ:
            return [_div(v[0], v[2]), _div(v[1], v[2]), v[2], v[3]]
        if mod == SRC_DW:
            return [_div(v[0], v[3]), _div(v[1], v[3]), v[2], v[3]]
        if mod == SRC_ABS:
            return [abs(x) for x in v]
        if mod == SRC_ABSNEG:
            return [-abs(x) for x in v]
        raise ShaderRuntimeError("unknown source modifier %d" % mod)

    def _condition(self, src):
        """Boolean from a b# or p0 source (with the not modifier)."""
        if src.type == REG_CONSTBOOL:
            value = self.bool_consts.get(src.num, False)
        elif src.type == REG_PREDICATE:
            value = self.pred[src.swizzle[0]]
        else:
            raise ShaderRuntimeError("condition source must be b# or p0, got type %d" % src.type)
        return (not value) if src.modifier == SRC_NOT else value

    def _pred_mask(self, pred):
        mask = [self.pred[c] for c in pred.swizzle]
        if pred.modifier == SRC_NOT:
            mask = [not m for m in mask]
        return mask

    # -- result writing ------------------------------------------------------------
    def _write(self, ins, values):
        dst = ins.dst
        mask = list(dst.mask)
        if ins.pred is not None:
            mask = [m and p for m, p in zip(mask, self._pred_mask(ins.pred))]
        if dst.type == REG_PREDICATE:
            for i in range(4):
                if mask[i]:
                    self.pred[i] = bool(values[i])
            return
        if dst.type == REG_ADDR and not self.pixel:
            legacy = ins.opcode == OP["MOV"]
            for i in range(4):
                if mask[i]:
                    self.addr[i] = _round_address(values[i], legacy)
            return
        scale = 2.0 ** dst.shift if dst.shift else None
        out = []
        for i in range(4):
            v = values[i]
            if scale is not None:
                v = f32(v * scale)
            if dst.saturate:
                v = _sat(v)
            out.append(f32(v))
        num = dst.num
        if dst.rel is not None:
            num += self._rel_offset(dst.rel)
        key = (dst.type, num)
        reg = self.regs.get(key)
        reg = list(reg) if reg is not None else [0.0, 0.0, 0.0, 0.0]
        for i in range(4):
            if mask[i]:
                reg[i] = out[i]
        self.regs[key] = reg

    # -- main loop -------------------------------------------------------------------
    def run(self):
        program = self.shader.program
        jump = self.shader.jump
        count = len(program)
        call_stack = []
        pc = 0
        steps = 0
        while pc < count:
            steps += 1
            if steps > self.max_steps:
                raise ShaderRuntimeError("exceeded %d executed instructions (runaway loop?)"
                                         % self.max_steps)
            ins = program[pc]
            op = ins.opcode
            if op in FLOW_OPS:
                if op == OP["RET"] or op == OP["LABEL"]:
                    if op == OP["RET"] and call_stack:
                        pc, depth = call_stack.pop()
                        del self.loops[depth:]
                        continue
                    break  # end of the main program
                if op == OP["CALL"] or (op == OP["CALLNZ"] and self._condition(ins.srcs[1])):
                    if len(call_stack) >= 32:
                        raise ShaderRuntimeError("call stack overflow")
                    label = ins.srcs[0].num
                    if label not in self.shader.labels:
                        raise ShaderRuntimeError("call to undefined label l%d" % label)
                    call_stack.append((pc + 1, len(self.loops)))
                    pc = self.shader.labels[label] + 1
                    continue
                if op == OP["CALLNZ"]:
                    pc += 1
                    continue
                if op in (OP["IF"], OP["IFC"]):
                    if op == OP["IF"]:
                        taken = self._condition(ins.srcs[0])
                    else:
                        a = self._src(ins.srcs[0])[0]
                        b = self._src(ins.srcs[1])[0]
                        taken = _compare(ins.control & 7, a, b)
                    if taken:
                        pc += 1
                    else:
                        target = jump[pc]
                        pc = target + 1  # past else (into the else body) or endif
                    continue
                if op == OP["ELSE"]:
                    pc = jump[pc] + 1
                    continue
                if op == OP["ENDIF"]:
                    pc += 1
                    continue
                if op in (OP["REP"], OP["LOOP"]):
                    if op == OP["REP"]:
                        values = self.int_consts.get(ins.srcs[0].num, [0, 0, 0, 0])
                        iterations, start, step = values[0], 0, 0
                    else:
                        values = self.int_consts.get(ins.srcs[1].num, [0, 0, 0, 0])
                        iterations, start, step = values[0], values[1], values[2]
                    iterations = min(max(iterations, 0), 255)
                    if iterations == 0:
                        pc = jump[pc] + 1
                        continue
                    self.loops.append(_LoopFrame(op == OP["LOOP"], pc + 1, jump[pc],
                                                 iterations, start, step))
                    pc += 1
                    continue
                if op in (OP["ENDREP"], OP["ENDLOOP"]):
                    frame = self.loops[-1]
                    frame.remaining -= 1
                    if frame.remaining > 0:
                        frame.counter += frame.step
                        pc = frame.start
                    else:
                        self.loops.pop()
                        pc += 1
                    continue
                if op in (OP["BREAK"], OP["BREAKC"], OP["BREAKP"]):
                    if op == OP["BREAK"]:
                        taken = True
                    elif op == OP["BREAKC"]:
                        taken = _compare(ins.control & 7, self._src(ins.srcs[0])[0],
                                         self._src(ins.srcs[1])[0])
                    else:
                        taken = self._condition(ins.srcs[0])
                    if taken:
                        if not self.loops:
                            raise ShaderRuntimeError("break outside a loop")
                        frame = self.loops.pop()
                        pc = frame.end + 1
                    else:
                        pc += 1
                    continue
            nxt = program[pc + 1] if pc + 1 < count else None
            if nxt is not None and nxt.coissue:
                # ps_1_x co-issue: both halves read the pre-pair register state.
                first = self._compute(ins)
                second = self._compute(nxt)
                if first is not None:
                    self._write(ins, first)
                if second is not None:
                    self._write(nxt, second)
                pc += 2
                steps += 1
                continue
            values = self._compute(ins)
            if values is not None:
                self._write(ins, values)
            pc += 1
        self.result.steps = steps
        return self._collect()

    def _collect(self):
        result = self.result
        shader = self.shader
        if self.pixel and self.major == 1:
            r0 = self.regs.get((REG_TEMP, 0))
            if r0 is not None:
                self.regs[(REG_COLOROUT, 0)] = r0
        output_types = ((REG_COLOROUT, REG_DEPTHOUT) if self.pixel
                        else (REG_RASTOUT, REG_ATTROUT, REG_OUTPUT))
        for (reg_type, num), value in sorted(self.regs.items()):
            if reg_type in output_types:
                result.registers[register_name(shader, reg_type, num)] = tuple(value)
        if not self.pixel:
            if self.major >= 3:
                for decl in shader.outputs():
                    name = register_name(shader, decl.type, decl.num)
                    if name in result.registers:
                        result.semantics[decl.semantic] = name
            else:
                for name in result.registers:
                    if name == "oPos":
                        result.semantics["position0"] = name
                    elif name == "oFog":
                        result.semantics["fog0"] = name
                    elif name == "oPts":
                        result.semantics["psize0"] = name
                    elif name.startswith("oD"):
                        result.semantics["color" + name[2:]] = name
                    elif name.startswith("oT"):
                        result.semantics["texcoord" + name[2:]] = name
        else:
            for name in result.registers:
                if name.startswith("oC"):
                    result.semantics["color" + name[2:]] = name
                elif name == "oDepth":
                    result.semantics["depth0"] = name
        return result

    # -- instruction semantics ---------------------------------------------------------
    def _compute(self, ins):
        handler = _HANDLERS.get(ins.opcode)
        if handler is None:
            raise UnsupportedInstruction("%s (opcode %d) is not implemented for %s"
                                         % (ins.name, ins.opcode, self.shader.profile))
        return handler(self, ins)

    def _s(self, ins, i):
        return self._src(ins.srcs[i])

    def _scalar(self, ins, i=0):
        return self._src(ins.srcs[i])[3]

    def op_nop(self, ins):
        return None

    def op_mov(self, ins):
        return self._s(ins, 0)

    def op_add(self, ins):
        a, b = self._s(ins, 0), self._s(ins, 1)
        return [_add(a[i], b[i]) for i in range(4)]

    def op_sub(self, ins):
        a, b = self._s(ins, 0), self._s(ins, 1)
        return [f32(a[i] - b[i]) for i in range(4)]

    def op_mul(self, ins):
        a, b = self._s(ins, 0), self._s(ins, 1)
        return [_mul(a[i], b[i]) for i in range(4)]

    def op_mad(self, ins):
        a, b, c = self._s(ins, 0), self._s(ins, 1), self._s(ins, 2)
        return [_add(_mul(a[i], b[i]), c[i]) for i in range(4)]

    def op_rcp(self, ins):
        return [_rcp(self._scalar(ins))] * 4

    def op_rsq(self, ins):
        return [_rsq(self._scalar(ins))] * 4

    def op_dp3(self, ins):
        return [_dot(self._s(ins, 0), self._s(ins, 1), 3)] * 4

    def op_dp4(self, ins):
        return [_dot(self._s(ins, 0), self._s(ins, 1), 4)] * 4

    def op_min(self, ins):
        a, b = self._s(ins, 0), self._s(ins, 1)
        return [a[i] if a[i] < b[i] else b[i] for i in range(4)]

    def op_max(self, ins):
        a, b = self._s(ins, 0), self._s(ins, 1)
        return [a[i] if a[i] >= b[i] else b[i] for i in range(4)]

    def op_slt(self, ins):
        a, b = self._s(ins, 0), self._s(ins, 1)
        return [1.0 if a[i] < b[i] else 0.0 for i in range(4)]

    def op_sge(self, ins):
        a, b = self._s(ins, 0), self._s(ins, 1)
        return [1.0 if a[i] >= b[i] else 0.0 for i in range(4)]

    def op_exp(self, ins):
        return [_exp2(self._scalar(ins))] * 4

    def op_expp(self, ins):
        x = self._scalar(ins)
        if self.pixel or self.major >= 2:
            return [_exp2(x)] * 4
        v = _floor(x)
        partial = _exp2(x)
        if not (math.isinf(partial) or math.isnan(partial)):
            partial = _bits_to_float(_float_to_bits(partial) & 0xFFFFFF00)
        return [_exp2(v), f32(x - v), partial, 1.0]

    def op_log(self, ins):
        return [_log2_abs(self._scalar(ins))] * 4

    def op_logp(self, ins):
        x = self._scalar(ins)
        if self.pixel or self.major >= 2:
            return [_log2_abs(x)] * 4
        v = abs(x)
        if v == 0.0:
            return [-INF, 1.0, -INF, 1.0]
        if math.isinf(v) or math.isnan(v):
            return [_log2_abs(v), NAN, _log2_abs(v), 1.0]
        bits = _float_to_bits(f32(v))
        exponent = float(((bits >> 23) & 0xFF) - 127)
        mantissa = _bits_to_float((bits & 0x7FFFFF) | 0x3F800000)
        partial = _bits_to_float(_float_to_bits(_log2_abs(v)) & 0xFFFFFF00)
        return [exponent, mantissa, partial, 1.0]

    def op_lit(self, ins):
        s = self._s(ins, 0)
        out = [1.0, 0.0, 0.0, 1.0]
        power = min(max(s[3], -127.9961), 127.9961)
        if s[0] > 0.0:
            out[1] = s[0]
            if s[1] > 0.0:
                out[2] = _pow(s[1], f32(power))
        return out

    def op_dst(self, ins):
        a, b = self._s(ins, 0), self._s(ins, 1)
        return [1.0, _mul(a[1], b[1]), a[2], b[3]]

    def op_lrp(self, ins):
        a, b, c = self._s(ins, 0), self._s(ins, 1), self._s(ins, 2)
        return [_add(_mul(a[i], f32(b[i] - c[i])), c[i]) for i in range(4)]

    def op_frc(self, ins):
        return [_frc(x) for x in self._s(ins, 0)]

    def _matrix(self, ins, rows, size):
        vector = self._s(ins, 0)
        out = [0.0, 0.0, 0.0, 0.0]
        for row in range(rows):
            out[row] = _dot(vector, self._src(ins.srcs[1], row), size)
        return out

    def op_m4x4(self, ins):
        return self._matrix(ins, 4, 4)

    def op_m4x3(self, ins):
        return self._matrix(ins, 3, 4)

    def op_m3x4(self, ins):
        return self._matrix(ins, 4, 3)

    def op_m3x3(self, ins):
        return self._matrix(ins, 3, 3)

    def op_m3x2(self, ins):
        return self._matrix(ins, 2, 3)

    def op_pow(self, ins):
        return [_pow(self._scalar(ins, 0), self._scalar(ins, 1))] * 4

    def op_crs(self, ins):
        a, b = self._s(ins, 0), self._s(ins, 1)
        return [f32(_mul(a[1], b[2]) - _mul(a[2], b[1])),
                f32(_mul(a[2], b[0]) - _mul(a[0], b[2])),
                f32(_mul(a[0], b[1]) - _mul(a[1], b[0])), 0.0]

    def op_sgn(self, ins):
        return [1.0 if x > 0 else (-1.0 if x < 0 else 0.0) for x in self._s(ins, 0)]

    def op_abs(self, ins):
        return [abs(x) for x in self._s(ins, 0)]

    def op_nrm(self, ins):
        s = self._s(ins, 0)
        scale = _rsq(_dot(s, s, 3))
        return [_mul(x, scale) for x in s]

    def op_sincos(self, ins):
        x = self._scalar(ins, 0)
        return [_trig(math.cos, x), _trig(math.sin, x), 0.0, 0.0]

    def op_mova(self, ins):
        return self._s(ins, 0)

    def op_cmp(self, ins):
        a, b, c = self._s(ins, 0), self._s(ins, 1), self._s(ins, 2)
        return [b[i] if a[i] >= 0.0 else c[i] for i in range(4)]

    def op_cnd(self, ins):
        a, b, c = self._s(ins, 0), self._s(ins, 1), self._s(ins, 2)
        return [b[i] if a[i] > 0.5 else c[i] for i in range(4)]

    def op_dp2add(self, ins):
        a, b, c = self._s(ins, 0), self._s(ins, 1), self._s(ins, 2)
        return [_add(_dot(a, b, 2), c[3])] * 4

    def op_setp(self, ins):
        a, b = self._s(ins, 0), self._s(ins, 1)
        kind = ins.control & 7
        return [_compare(kind, a[i], b[i]) for i in range(4)]

    def op_dsx(self, ins):
        return self._derivative("dsx", ins)

    def op_dsy(self, ins):
        return self._derivative("dsy", ins)

    def _derivative(self, kind, ins):
        value = self._s(ins, 0)
        if self.derivative_fn is None:
            return [0.0, 0.0, 0.0, 0.0]
        return _vec4(self.derivative_fn(kind, ins, value))

    # -- texturing -------------------------------------------------------------
    def _sample(self, stage, coords, kind, extra=None):
        if self.sampler_fn is None:
            raise ShaderRuntimeError("%s samples s%d but no sampler_fn was given"
                                     % (self.shader.profile, stage))
        extra = dict(extra or {})
        extra.setdefault("texture_type", self.sampler_types.get(stage))
        if not self.pixel:
            extra["vertex"] = True
        value = self.sampler_fn(stage, tuple(coords), kind, extra)
        return _vec4(value)

    def op_tex(self, ins):
        if self.major >= 2:
            coords = self._s(ins, 0)
            stage = ins.srcs[1].num
            control = ins.control & 3
            if control == 1:
                w = coords[3]
                coords = [_div(coords[0], w), _div(coords[1], w), _div(coords[2], w), 1.0]
                return self._sample(stage, coords, "texldp")
            if control == 2:
                return self._sample(stage, coords, "texldb", {"bias": coords[3]})
            return self._sample(stage, coords, "texld")
        stage = ins.dst.num
        if self.minor >= 4:
            return self._sample(stage, self._s(ins, 0), "texld")
        return self._sample(stage, self.texcoords[stage], "tex")

    def op_texldl(self, ins):
        coords = self._s(ins, 0)
        return self._sample(ins.srcs[1].num, coords, "texldl", {"lod": coords[3]})

    def op_texldd(self, ins):
        coords = self._s(ins, 0)
        return self._sample(ins.srcs[1].num, coords, "texldd",
                            {"ddx": self._s(ins, 2), "ddy": self._s(ins, 3)})

    def op_texkill(self, ins):
        dst = ins.dst
        if self.major == 1 and dst.type == REG_TEXTURE and self.minor < 4:
            value = self.texcoords.get(dst.num, [0.0] * 4)
            lanes = (0, 1, 2)
        else:
            value = self._fetch(dst.type, dst.num, dst.rel)
            lanes = (0, 1, 2) if self.major == 1 else [i for i in range(4) if dst.mask[i]]
        if ins.pred is not None:
            mask = self._pred_mask(ins.pred)
            lanes = [i for i in lanes if mask[i]]
        if any(value[i] < 0.0 for i in lanes):
            self.result.discarded = True
        return None

    def op_texcoord(self, ins):
        if self.minor >= 4:
            s = self._s(ins, 0)
            return [s[0], s[1], s[2], 1.0]
        t = self.texcoords[ins.dst.num]
        return [_sat(t[0]), _sat(t[1]), _sat(t[2]), 1.0]

    def _tc_dot(self, ins):
        return _dot(self.texcoords[ins.dst.num], self._s(ins, 0), 3)

    def op_texm3x2pad(self, ins):
        self.tex_pad = [self._tc_dot(ins)]
        return None

    def op_texm3x3pad(self, ins):
        if len(self.tex_pad) >= 2:
            self.tex_pad = []
        self.tex_pad.append(self._tc_dot(ins))
        return None

    def _pads(self, count):
        if len(self.tex_pad) != count:
            raise ShaderRuntimeError("texture matrix used without its %d pad instructions" % count)
        pads = self.tex_pad
        self.tex_pad = []
        return pads

    def op_texm3x2tex(self, ins):
        u = self._pads(1)[0]
        return self._sample(ins.dst.num, [u, self._tc_dot(ins), 0.0, 1.0], "texm3x2tex")

    def op_texm3x2depth(self, ins):
        z = self._pads(1)[0]
        w = self._tc_dot(ins)
        depth = 1.0 if w == 0.0 else _div(z, w)
        self.regs[(REG_DEPTHOUT, 0)] = [depth] * 4
        return None

    def op_texm3x3tex(self, ins):
        u, v = self._pads(2)
        return self._sample(ins.dst.num, [u, v, self._tc_dot(ins), 1.0], "texm3x3tex")

    def op_texm3x3(self, ins):
        u, v = self._pads(2)
        return [u, v, self._tc_dot(ins), 1.0]

    def _reflect(self, normal, eye):
        nn = _dot(normal, normal, 3)
        ne = _dot(normal, eye, 3)
        scale = _div(f32(2.0 * ne), nn)
        return [f32(_mul(normal[i], scale) - eye[i]) for i in range(3)] + [1.0]

    def op_texm3x3spec(self, ins):
        u, v = self._pads(2)
        normal = [u, v, self._tc_dot(ins)]
        eye = self._s(ins, 1)
        return self._sample(ins.dst.num, self._reflect(normal, eye), "texm3x3spec")

    def op_texm3x3vspec(self, ins):
        u, v = self._pads(2)
        n = ins.dst.num
        normal = [u, v, self._tc_dot(ins)]
        eye = [self.texcoords.get(n - 2, [0.0] * 4)[3], self.texcoords.get(n - 1, [0.0] * 4)[3],
               self.texcoords[n][3]]
        return self._sample(n, self._reflect(normal, eye), "texm3x3vspec")

    def op_texdp3tex(self, ins):
        return self._sample(ins.dst.num, [self._tc_dot(ins), 0.0, 0.0, 1.0], "texdp3tex")

    def op_texdp3(self, ins):
        return [self._tc_dot(ins)] * 4

    def op_texreg2ar(self, ins):
        s = self._s(ins, 0)
        return self._sample(ins.dst.num, [s[3], s[0], 0.0, 1.0], "texreg2ar")

    def op_texreg2gb(self, ins):
        s = self._s(ins, 0)
        return self._sample(ins.dst.num, [s[1], s[2], 0.0, 1.0], "texreg2gb")

    def op_texreg2rgb(self, ins):
        s = self._s(ins, 0)
        return self._sample(ins.dst.num, [s[0], s[1], s[2], 1.0], "texreg2rgb")

    def _bump_matrix(self, stage):
        env = self.bump_env.get(stage, {})
        m00, m01, m10, m11 = env.get("mat", (0.0, 0.0, 0.0, 0.0))
        return m00, m01, m10, m11, env.get("lscale", 0.0), env.get("loffset", 0.0)

    def _bumped(self, ins):
        stage = ins.dst.num
        s = self._s(ins, 0)
        t = self.texcoords[stage]
        m00, m01, m10, m11, lscale, loffset = self._bump_matrix(stage)
        u = f32(t[0] + _mul(m00, s[0]) + _mul(m10, s[1]))
        v = f32(t[1] + _mul(m01, s[0]) + _mul(m11, s[1]))
        return stage, s, [u, v, 0.0, 1.0], lscale, loffset

    def op_texbem(self, ins):
        stage, _s, coords, _l, _o = self._bumped(ins)
        return self._sample(stage, coords, "texbem")

    def op_texbeml(self, ins):
        stage, s, coords, lscale, loffset = self._bumped(ins)
        value = self._sample(stage, coords, "texbeml")
        luminance = f32(_mul(s[2], lscale) + loffset)
        return [_mul(x, luminance) for x in value]

    def op_bem(self, ins):
        a, b = self._s(ins, 0), self._s(ins, 1)
        m00, m01, m10, m11, _l, _o = self._bump_matrix(ins.dst.num)
        return [f32(a[0] + _mul(m00, b[0]) + _mul(m10, b[1])),
                f32(a[1] + _mul(m01, b[0]) + _mul(m11, b[1])), a[2], a[3]]

    def op_texdepth(self, ins):
        r = self._fetch(REG_TEMP, ins.dst.num)
        depth = 1.0 if r[1] == 0.0 else _div(r[0], r[1])
        self.regs[(REG_DEPTHOUT, 0)] = [depth] * 4
        return None


_HANDLERS = {}
for _name, _code in OP.items():
    _method = getattr(_Machine, "op_" + _name.lower(), None)
    if _method is not None:
        _HANDLERS[_code] = _method
_HANDLERS[OP["PHASE"]] = _Machine.op_nop


def _run(code, kind, inputs, consts, sampler_fn, derivative_fn, bump_env, max_steps):
    shader = parse(code)
    if shader.kind != kind:
        raise ValueError("expected a %s shader, got %s" % (kind, shader.profile))
    machine = _Machine(shader, inputs, consts, sampler_fn, derivative_fn, bump_env, max_steps)
    return machine.run()


def run_vertex(code, inputs=None, consts=None, sampler_fn=None, derivative_fn=None,
               max_steps=1000000):
    """Run a vertex shader once. Returns a ShaderResult (oPos, oD#, oT#/o#,
    oFog, oPts)."""
    return _run(code, "vs", inputs, consts, sampler_fn, derivative_fn, None, max_steps)


def run_pixel(code, inputs=None, consts=None, sampler_fn=None, derivative_fn=None,
              bump_env=None, max_steps=1000000):
    """Run a pixel shader once. Returns a ShaderResult (oC0..3, oDepth,
    .discarded). `bump_env` maps a stage to {"mat": (m00, m01, m10, m11),
    "lscale": s, "loffset": o} for the ps_1_x bump instructions."""
    return _run(code, "ps", inputs, consts, sampler_fn, derivative_fn, bump_env, max_steps)


def main(argv=None):
    parser = argparse.ArgumentParser(description="Disassemble raw D3D9 shader bytecode.")
    parser.add_argument("path", help="file holding the raw token stream")
    args = parser.parse_args(argv)
    with open(args.path, "rb") as stream:
        print(disassemble(stream.read()))
    return 0


if __name__ == "__main__":
    sys.exit(main())
