#!/usr/bin/env python3
"""Read VMF files and collapse `func_instance` files into a host map.

The pinned vbsp collapses geometry instances but predates Portal 2's
instance I/O (`func_instance_io_proxy`, `instance:` connections), and it
looks instances up by Windows-style relative paths. A map generator that
wants the Portal 2 SDK's authored pieces (door frames, droppers, fizzlers,
button bases, light panels, observation rooms) collapses them here instead,
the way Hammer's own collapse does:

  * every point is rotated by the instance's angles and moved to its origin;
    texture axes rotate with the geometry and their shifts follow the move;
  * entity origins and angles compose with the instance transform (a
    light_spot's `pitch` is kept equal to its angles' pitch);
  * with fixup style 0 (prefix) names become `<instance>-<name>`, except
    names starting with `@` or `!` (global and special names); style 2 leaves
    names alone; `$parm` replacements apply to every value;
  * nested instances collapse recursively; I/O proxies are dropped, so a host
    map addresses the fixed-up entity names directly.

    python3 tools/quality/vmf_instances.py <instance.vmf>   # entity summary

A VMF is kept as a tree: a list of (key, value) pairs whose value is either a
string or a nested list (a block such as `solid`, `side` or `connections`).
"""

import math
import re
import sys
from pathlib import Path

# Keys whose values name other entities (Hammer's target_destination and
# target_source fields in the Portal 2 FGD that instances here use).
NAME_KEYS = {"targetname", "parentname", "target", "filtername", "damagefilter",
             "entitytemplate", "areaportalwindow", "lightingorigin", "laserentity",
             "portaltwo", "linkedportal", "measuretarget", "shadowtarget", "attach1",
             "attach2", "spawnpositionname", "landmark", "source", "destination"}
NAME_KEYS |= {"template%02d" % i for i in range(1, 17)}
TOKEN = re.compile(r'"([^"]*)"|(\{)|(\})')
PLANE = re.compile(r"\(\s*([-\d.eE+]+)\s+([-\d.eE+]+)\s+([-\d.eE+]+)\s*\)")
AXIS = re.compile(r"\[\s*([-\d.eE+]+)\s+([-\d.eE+]+)\s+([-\d.eE+]+)\s+([-\d.eE+]+)\s*\]\s*"
                  r"([-\d.eE+]+)")
# Portal 2 VMFs separate connection fields with ESC; older tools use commas.
IO_SEPARATOR = "\x1b"


# ---------------------------------------------------------------- reading
def parse(text):
    """The VMF text as a tree. Bare words (unquoted block names) are allowed."""
    tokens = re.findall(r'"[^"]*"|\{|\}|[^\s{}"]+', text)
    position = 0

    def block():
        nonlocal position
        items = []
        while position < len(tokens):
            token = tokens[position]
            if token == "}":
                position += 1
                return items
            key = token.strip('"')
            following = tokens[position + 1]
            if following == "{":
                position += 2
                items.append((key, block()))
            else:
                items.append((key, following[1:-1] if following.startswith('"') else following))
                position += 2
        return items

    return block()


def read(path):
    return parse(Path(path).read_text(errors="replace"))


def value(block, key, default=None):
    for k, v in block:
        if k == key and isinstance(v, str):
            return v
    return default


def set_value(block, key, new):
    for index, (k, v) in enumerate(block):
        if k == key and isinstance(v, str):
            block[index] = (k, new)
            return
    block.append((key, new))


def children(block, key):
    return [v for k, v in block if k == key and isinstance(v, list)]


def serialize(tree, depth=0):
    lines = []
    indent = "\t" * depth
    for key, item in tree:
        if isinstance(item, list):
            lines += [indent + key, indent + "{", serialize(item, depth + 1), indent + "}"]
        else:
            lines.append('%s"%s" "%s"' % (indent, key, item))
    return "\n".join(line for line in lines if line != "")


# ------------------------------------------------------------ transforms
def angle_matrix(pitch, yaw, roll):
    """Source's AngleMatrix: the columns are forward, left and up."""
    sp, cp = math.sin(math.radians(pitch)), math.cos(math.radians(pitch))
    sy, cy = math.sin(math.radians(yaw)), math.cos(math.radians(yaw))
    sr, cr = math.sin(math.radians(roll)), math.cos(math.radians(roll))
    forward = (cp * cy, cp * sy, -sp)
    left = (sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, sr * cp)
    up = (cr * sp * cy + sr * sy, cr * sp * sy - sr * cy, cr * cp)
    return [[forward[r], left[r], up[r]] for r in range(3)]


def matrix_angles(m):
    """Source's MatrixAngles for a rotation whose columns are forward, left, up."""
    forward = [m[r][0] for r in range(3)]
    left = [m[r][1] for r in range(3)]
    up = [m[r][2] for r in range(3)]
    xy = math.hypot(forward[0], forward[1])
    if xy > 0.001:
        yaw = math.degrees(math.atan2(forward[1], forward[0]))
        pitch = math.degrees(math.atan2(-forward[2], xy))
        roll = math.degrees(math.atan2(left[2], up[2]))
    else:
        yaw = math.degrees(math.atan2(-left[0], left[1]))
        pitch = math.degrees(math.atan2(-forward[2], xy))
        roll = 0.0
    return pitch, yaw, roll


def multiply(a, b):
    return [[sum(a[r][k] * b[k][c] for k in range(3)) for c in range(3)] for r in range(3)]


def clean(number):
    """Round away rotation noise so 90-degree turns keep integer coordinates."""
    rounded = round(number)
    if abs(number - rounded) < 1e-4:
        number = rounded
    number = round(number, 4)
    return 0.0 if number == 0 else number


def fmt(number):
    return ("%.4f" % clean(number)).rstrip("0").rstrip(".")


class Transform:
    """World from instance: p -> R p + origin."""

    def __init__(self, origin=(0.0, 0.0, 0.0), angles=(0.0, 0.0, 0.0)):
        self.origin = tuple(float(c) for c in origin)
        self.rotation = angle_matrix(*angles)

    def point(self, p):
        return tuple(sum(self.rotation[r][k] * p[k] for k in range(3)) + self.origin[r]
                     for r in range(3))

    def vector(self, v):
        return tuple(sum(self.rotation[r][k] * v[k] for k in range(3)) for r in range(3))

    def angles(self, angles):
        return matrix_angles(multiply(self.rotation, angle_matrix(*angles)))

    def then(self, outer):
        """This transform followed by `outer` (an outer instance's)."""
        composed = Transform()
        composed.rotation = multiply(outer.rotation, self.rotation)
        composed.origin = outer.point(self.origin)
        return composed


def parse_vector(text):
    parts = [float(p) for p in text.replace(",", " ").split()]
    return tuple((parts + [0.0, 0.0, 0.0])[:3])


def vector_text(v):
    return " ".join(fmt(c) for c in v)


def transform_side(side, transform):
    plane = value(side, "plane")
    points = [transform.point(tuple(float(c) for c in m)) for m in PLANE.findall(plane)]
    set_value(side, "plane", " ".join("(%s)" % vector_text(p) for p in points))
    for key in ("uaxis", "vaxis"):
        text = value(side, key)
        if not text:
            continue
        match = AXIS.search(text)
        axis = transform.vector(tuple(float(match.group(i)) for i in (1, 2, 3)))
        shift, scale = float(match.group(4)), float(match.group(5))
        # A texel at world point w maps to dot(w, axis) / scale + shift; keep
        # the moved geometry's texels where they were on the instance.
        moved = shift - sum(a * o for a, o in zip(axis, transform.origin)) / (scale or 1.0)
        set_value(side, key, "[%s %s] %s" % (vector_text(axis), fmt(moved), fmt(scale)))
    dispinfo = children(side, "dispinfo")
    if dispinfo:
        raise ValueError("displacements in instances are not supported")


def transform_solid(solid, transform, ids):
    set_value(solid, "id", str(ids()))
    for side in children(solid, "side"):
        set_value(side, "id", str(ids()))
        transform_side(side, transform)
    return solid


# ---------------------------------------------------------------- naming
def fixup_name(name, prefix):
    if not prefix or not name or name[0] in "@!":
        return name
    return "%s-%s" % (prefix, name)


def replace_parms(text, parms):
    for key in sorted(parms, key=len, reverse=True):
        text = text.replace(key, parms[key])
    return text


def fixup_connection(text, prefix, parms):
    text = replace_parms(text, parms)
    separator = IO_SEPARATOR if IO_SEPARATOR in text else ","
    fields = text.split(separator)
    fields[0] = fixup_name(fields[0], prefix)
    return ",".join(fields) if separator == IO_SEPARATOR and all(
        "," not in f for f in fields) else separator.join(fields)


# -------------------------------------------------------------- collapse
class Collapsed:
    """World solids and entities of collapsed instances, in host space."""

    def __init__(self):
        self.world = []
        self.entities = []


def instance_parms(tree):
    """The `$name default` pairs a func_instance_parms entity declares."""
    parms = {}
    for entity in children(tree, "entity"):
        if value(entity, "classname") == "func_instance_parms":
            for key, text in entity:
                if key.startswith("parm") and isinstance(text, str) and text.split():
                    parms[text.split()[0]] = ""
    return parms


def collapse(path, root, origin=(0, 0, 0), angles=(0, 0, 0), name="", fixup_style=0,
             replace=None, ids=None, out=None, world=True, outer=None):
    """Collapse the instance file `path` (relative to `root`, the SDK's maps
    directory) placed at `origin`/`angles`, into `out` (a Collapsed).

    `replace` maps `$parm` to its value. `world=False` drops the instance's
    world solids (keeping entity brushes such as func_detail and triggers).
    `ids` is the host's id allocator (a callable returning a fresh id)."""
    out = out or Collapsed()
    ids = ids or _counter()
    transform = Transform(origin, angles)
    if outer is not None:
        transform = transform.then(outer)
    prefix = name if fixup_style == 0 else ""
    tree = read(Path(root) / path)
    parms = instance_parms(tree)
    parms.update(replace or {})
    if world:
        for block in children(tree, "world"):
            for solid in children(block, "solid"):
                out.world.append(transform_solid(copy(solid), transform, ids))
    for entity in children(tree, "entity"):
        classname = value(entity, "classname")
        if classname in ("func_instance_io_proxy", "func_instance_parms"):
            continue
        if classname == "func_instance":
            inner = {}
            for key, text in entity:
                if key.startswith("replace") and isinstance(text, str) and text.split():
                    parts = text.split(None, 1)
                    inner[parts[0]] = replace_parms(parts[1] if len(parts) > 1 else "", parms)
            # Hammer fixes up the inner instance's names first, then the
            # outer prefix applies to the result: a named style-0 child adds
            # its own name, and any other child keeps just the outer prefix.
            child_name = value(entity, "targetname", "")
            style = int(value(entity, "fixup_style", "0") or 0)
            child_prefix = fixup_name(child_name, prefix) if child_name and style == 0 else prefix
            collapse(value(entity, "file"), root,
                     parse_vector(value(entity, "origin", "0 0 0")),
                     parse_vector(value(entity, "angles", "0 0 0")),
                     child_prefix, 0, inner, ids, out, world, transform)
            continue
        out.entities.append(transform_entity(copy(entity), transform, prefix, parms, ids))
    return out


def transform_entity(entity, transform, prefix, parms, ids):
    result = []
    for key, item in entity:
        if isinstance(item, list):
            if key == "solid":
                item = transform_solid(item, transform, ids)
            elif key == "connections":
                item = [(k, fixup_connection(v, prefix, parms)) for k, v in item]
            elif key == "hidden":
                item = [(k, transform_solid(v, transform, ids) if k == "solid" else v)
                        for k, v in item]
            result.append((key, item))
            continue
        item = replace_parms(item, parms)
        lower = key.lower()
        if lower == "id":
            item = str(ids())
        elif lower == "origin":
            item = vector_text(transform.point(parse_vector(item)))
        elif lower == "angles" and value(entity, "pitch") is None:
            item = vector_text(transform.angles(parse_vector(item)))
        elif lower in NAME_KEYS:
            item = fixup_name(item, prefix)
        result.append((key, item))
    if value(result, "pitch") is not None:
        transform_light_angles(result, transform)
    return result


def transform_light_angles(entity, transform):
    """Lights with a `pitch` key (light_spot, light_environment, light_dynamic)
    use Hammer's light convention: vrad's direction is (cos p cos y, cos p sin
    y, sin p) for p = `pitch`, or the angles' pitch when `pitch` is 0 (map_utils
    SetupLightNormalFromProps), so a positive pitch points up, against the
    engine's angles. Compose in engine terms and store the light convention."""
    pitch = float(value(entity, "pitch", "0") or 0)
    angles = parse_vector(value(entity, "angles", "0 0 0"))
    light_pitch = pitch or angles[0]
    world = transform.angles((-light_pitch, angles[1], angles[2]))
    set_value(entity, "angles", vector_text((-world[0], world[1], world[2])))
    set_value(entity, "pitch", fmt(-world[0]))


def copy(tree):
    return [(k, copy(v) if isinstance(v, list) else v) for k, v in tree]


def _counter(start=100000):
    state = [start]

    def next_id():
        state[0] += 1
        return state[0]

    return next_id


def main():
    for path in sys.argv[1:]:
        print("=== " + path)
        for entity in children(read(path), "entity"):
            keys = " ".join("%s=%s" % (k, v) for k, v in entity
                            if isinstance(v, str) and k != "id")
            print(keys)


if __name__ == "__main__":
    main()
