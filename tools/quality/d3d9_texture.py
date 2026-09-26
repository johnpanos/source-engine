#!/usr/bin/env python3
"""Small CPU textures and a sampler callback for `d3d9_shader_vm`.

These are deliberately simple reference samplers for procedurally generated
test textures, not a model of any GPU's filtering hardware:

    base = Texture2D.from_function(4, 4, lambda x, y: (x / 3, y / 3, 0, 1))
    env = TextureCube.constant((0.2, 0.4, 0.6, 1))
    samplers = SamplerSet({0: base, 1: env})
    result = d3d9_shader_vm.run_pixel(code, inputs, consts, samplers)

Conventions (D3D9):

* Texel (i, j) covers [i/w, (i+1)/w) x [j/h, (j+1)/h); v grows downwards
  (row 0 is the top row). Bilinear filtering samples around `u*w - 0.5`.
* Addressing per axis: "wrap", "clamp", "mirror" or "border".
* `srgb=True` decodes each texel with the exact sRGB transfer function
  before filtering (D3DSAMP_SRGBTEXTURE on post-DX9 hardware); alpha is linear.
* Level of detail: with no derivatives the implicit LOD is 0. texldl uses
  `extra["lod"]`, texldb adds `extra["bias"]` to 0, texldd derives a LOD from
  the explicit gradients. Mip levels are optional (`mips=[level1, ...]`);
  "point" filtering picks the nearest level, "bilinear" is trilinear between
  levels. Anisotropy is not modelled.
* Cube face selection follows the D3D9 table (+X, -X, +Y, -Y, +Z, -Z); the
  face images use the same texel convention.
* Values are returned as float (the VM rounds them to float32). Texel format
  quantisation (8-bit UNORM, DXT, ...) is the caller's job: supply the texel
  values the real format would decode to.
"""

import math


def srgb_to_linear(value):
    if value <= 0.04045:
        return value / 12.92
    return ((value + 0.055) / 1.055) ** 2.4


def _vec4(value):
    values = [float(v) for v in value]
    while len(values) < 4:
        values.append(1.0 if len(values) == 3 else 0.0)
    return values[:4]


def _address(index, size, mode):
    if mode == "wrap":
        return index % size
    if mode == "clamp":
        return min(max(index, 0), size - 1)
    if mode == "mirror":
        period = index % (2 * size)
        return period if period < size else 2 * size - 1 - period
    if mode == "border":
        return index if 0 <= index < size else None
    raise ValueError("unknown address mode %r" % mode)


class _Image:
    """One 2D or 3D mip level of RGBA float texels."""

    def __init__(self, texels, width, height, depth=1):
        self.width = width
        self.height = height
        self.depth = depth
        self.texels = texels  # flat list, index ((z*h)+y)*w+x -> [r,g,b,a]


def _image_from(source, width=None, height=None, depth=None):
    if isinstance(source, _Image):
        return source
    if callable(source):
        if depth:
            texels = [_vec4(source(x, y, z)) for z in range(depth)
                      for y in range(height) for x in range(width)]
            return _Image(texels, width, height, depth)
        texels = [_vec4(source(x, y)) for y in range(height) for x in range(width)]
        return _Image(texels, width, height)
    rows = list(source)
    if depth or (rows and rows[0] and rows[0][0] and isinstance(rows[0][0][0], (list, tuple))):
        # slices[z][y][x]
        slices = rows
        texels = [_vec4(t) for s in slices for row in s for t in row]
        return _Image(texels, len(slices[0][0]), len(slices[0]), len(slices))
    texels = [_vec4(t) for row in rows for t in row]
    return _Image(texels, len(rows[0]), len(rows))


class _Texture:
    def __init__(self, levels, filter="bilinear", address="wrap", srgb=False,
                 border=(0.0, 0.0, 0.0, 0.0)):
        if filter not in ("point", "bilinear"):
            raise ValueError("filter must be 'point' or 'bilinear'")
        self.levels = levels
        self.filter = filter
        if isinstance(address, str):
            self.address = (address,) * 3
        else:
            self.address = tuple(address) + ("wrap",) * (3 - len(address))
        self.srgb = srgb
        self.border = _vec4(border)

    def _texel(self, image, x, y, z=0):
        x = _address(x, image.width, self.address[0])
        y = _address(y, image.height, self.address[1])
        z = _address(z, image.depth, self.address[2]) if image.depth > 1 else 0
        if x is None or y is None or z is None:
            return list(self.border)
        texel = image.texels[(z * image.height + y) * image.width + x]
        if self.srgb:
            return [srgb_to_linear(texel[0]), srgb_to_linear(texel[1]),
                    srgb_to_linear(texel[2]), texel[3]]
        return texel

    def _sample_level(self, image, u, v, w=0.0):
        if self.filter == "point":
            x = math.floor(u * image.width)
            y = math.floor(v * image.height)
            z = math.floor(w * image.depth) if image.depth > 1 else 0
            return list(self._texel(image, x, y, z))
        fx = u * image.width - 0.5
        fy = v * image.height - 0.5
        x0, y0 = math.floor(fx), math.floor(fy)
        ax, ay = fx - x0, fy - y0
        if image.depth > 1:
            fz = w * image.depth - 0.5
            z0 = math.floor(fz)
            az = fz - z0
            zs = ((z0, 1.0 - az), (z0 + 1, az))
        else:
            zs = ((0, 1.0),)
        out = [0.0, 0.0, 0.0, 0.0]
        for z, wz in zs:
            for y, wy in ((y0, 1.0 - ay), (y0 + 1, ay)):
                for x, wx in ((x0, 1.0 - ax), (x0 + 1, ax)):
                    weight = wx * wy * wz
                    if weight:
                        texel = self._texel(image, x, y, z)
                        for i in range(4):
                            out[i] += weight * texel[i]
        return out

    def _sample_lod(self, lod, u, v, w=0.0):
        last = len(self.levels) - 1
        lod = min(max(lod, 0.0), float(last))
        if self.filter == "point" or last == 0:
            return self._sample_level(self.levels[int(math.floor(lod + 0.5))], u, v, w)
        lo = int(math.floor(lod))
        hi = min(lo + 1, last)
        t = lod - lo
        a = self._sample_level(self.levels[lo], u, v, w)
        if t == 0.0 or hi == lo:
            return a
        b = self._sample_level(self.levels[hi], u, v, w)
        return [a[i] + (b[i] - a[i]) * t for i in range(4)]

    def lod(self, extra):
        lod = 0.0
        if extra.get("lod") is not None:
            lod = float(extra["lod"])
        elif extra.get("ddx") is not None and extra.get("ddy") is not None:
            base = self.levels[0]
            dx = [extra["ddx"][0] * base.width, extra["ddx"][1] * base.height]
            dy = [extra["ddy"][0] * base.width, extra["ddy"][1] * base.height]
            rho = max(math.hypot(*dx), math.hypot(*dy))
            lod = math.log2(rho) if rho > 0 else -math.inf
        if extra.get("bias") is not None:
            lod += float(extra["bias"])
        return lod


class Texture2D(_Texture):
    """A 2D texture. `texels` is rows[y][x] of RGBA tuples (1-4 components,
    padded to (r, 0, 0, 1)); use `from_function` for procedural content."""

    def __init__(self, texels, mips=(), **options):
        levels = [_image_from(texels)] + [_image_from(m) for m in mips]
        super().__init__(levels, **options)

    @classmethod
    def from_function(cls, width, height, fn, **options):
        return cls(_image_from(fn, width, height), **options)

    @classmethod
    def constant(cls, rgba, **options):
        return cls([[rgba]], **options)

    def sample(self, coords, extra=None):
        return self._sample_lod(self.lod(extra or {}), coords[0], coords[1])


class TextureVolume(_Texture):
    """A 3D texture: slices[z][y][x] of RGBA tuples."""

    def __init__(self, slices, mips=(), **options):
        levels = [_image_from(slices, depth=True)] + [_image_from(m, depth=True) for m in mips]
        super().__init__(levels, **options)

    @classmethod
    def from_function(cls, width, height, depth, fn, **options):
        return cls(_image_from(fn, width, height, depth), **options)

    def sample(self, coords, extra=None):
        return self._sample_lod(self.lod(extra or {}), coords[0], coords[1], coords[2])


def cube_face(x, y, z):
    """(face, u, v) for a direction, D3D9 face order +X -X +Y -Y +Z -Z."""
    ax, ay, az = abs(x), abs(y), abs(z)
    if ax >= ay and ax >= az:
        face, ma, sc, tc = (0, ax, -z, -y) if x >= 0 else (1, ax, z, -y)
    elif ay >= az:
        face, ma, sc, tc = (2, ay, x, z) if y >= 0 else (3, ay, x, -z)
    else:
        face, ma, sc, tc = (4, az, x, -y) if z >= 0 else (5, az, -x, -y)
    if ma == 0:
        return face, 0.5, 0.5
    return face, 0.5 * (sc / ma + 1.0), 0.5 * (tc / ma + 1.0)


class TextureCube:
    """Six Texture2D faces (+X, -X, +Y, -Y, +Z, -Z). Filtering happens within
    the selected face (no seamless cube filtering)."""

    def __init__(self, faces):
        if len(faces) != 6:
            raise ValueError("a cube map needs six faces")
        self.faces = list(faces)

    @classmethod
    def constant(cls, rgba, **options):
        return cls([Texture2D.constant(rgba, **options) for _ in range(6)])

    @classmethod
    def from_direction(cls, size, fn, **options):
        """Faces filled by fn(direction xyz) evaluated at texel centres."""
        faces = []
        for face in range(6):
            def texel(x, y, face=face):
                s = 2.0 * (x + 0.5) / size - 1.0
                t = 2.0 * (y + 0.5) / size - 1.0
                direction = (
                    (1.0, -t, -s), (-1.0, -t, s), (s, 1.0, t),
                    (s, -1.0, -t), (s, -t, 1.0), (-s, -t, -1.0))[face]
                return fn(direction)
            faces.append(Texture2D.from_function(size, size, texel, **options))
        return cls(faces)

    def sample(self, coords, extra=None):
        face, u, v = cube_face(coords[0], coords[1], coords[2])
        return self.faces[face].sample((u, v), extra)


class SamplerSet:
    """`sampler_fn` for the VM: maps sampler stages to textures.

    Unbound stages raise KeyError unless `default` (an RGBA tuple) is given.
    Every call is appended to `self.calls` as (stage, coords, kind, extra) so
    tests can check what the shader fetched."""

    def __init__(self, textures=None, default=None):
        self.textures = dict(textures or {})
        self.default = None if default is None else _vec4(default)
        self.calls = []

    def __call__(self, stage, coords, kind, extra):
        self.calls.append((stage, tuple(coords), kind, dict(extra)))
        texture = self.textures.get(stage)
        if texture is None:
            if self.default is None:
                raise KeyError("no texture bound to sampler stage %d" % stage)
            return list(self.default)
        return texture.sample(coords, extra)
