"""The physics scenario map generator (tools/quality/physics_lab_maps.py):
each map is deterministic, its entity I/O is closed (wildcard targets match
something), every template, thruster, constraint and filter names an entity,
every counter has a scene that restarts it, and only declared models are used."""

import re
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import physics_lab_maps as lab  # noqa: E402

SPECIAL_TARGETS = {"!self", "!activator", "!caller", "!player"}


def entities(text):
    """(classname, keyvalues, outputs) for every entity block of a VMF."""
    found = []
    for block in text.split("\nentity\n")[1:]:
        keys = dict(re.findall(r'^\t"([^"]+)" "([^"]*)"$', block, re.MULTILINE))
        outputs = re.findall(r'^\t\t"(On\w+)" "([^"]*)"$', block, re.MULTILINE)
        found.append((keys.get("classname"), keys, outputs))
    return found


def matches(pattern, names):
    if pattern.endswith("*"):
        return any(name.startswith(pattern[:-1]) for name in names)
    return pattern in names


class MapTests(unittest.TestCase):
    def check_map(self, name):
        text = lab.MAPS[name]()
        self.assertEqual(text, lab.MAPS[name](), "not deterministic")
        self.assertEqual(text.count("{"), text.count("}"))
        found = entities(text)
        names = {kv["targetname"] for _, kv, _ in found if "targetname" in kv}
        classes = [classname for classname, _, _ in found]
        self.assertEqual(classes.count("info_player_start"), 1)
        self.assertEqual(classes.count("point_servercommand"), 1)

        renamed = set()
        for classname, kv, outputs in found:
            for event, connection in outputs:
                target, action, param = connection.split(",")[:3]
                if action == "AddOutput" and param.startswith("targetname "):
                    renamed.add(param.split(" ", 1)[1])
        for classname, kv, outputs in found:
            for event, connection in outputs:
                target = connection.split(",")[0]
                if target in SPECIAL_TARGETS:
                    continue
                self.assertTrue(matches(target, names | renamed),
                                "%s: %s %s -> %s" % (name, classname, event, target))
            for key in ("attach1", "attach2", "filtername", "targetentityname"):
                value = kv.get(key)
                if value and classname != "filter_activator_name":
                    self.assertIn(value, names, "%s: %s %s=%s" % (name, classname, key, value))
            for key, value in kv.items():
                if re.fullmatch(r"Template\d\d", key):
                    self.assertTrue(matches(value, names), "%s: template %s" % (name, value))
            if classname == "filter_activator_name":
                self.assertTrue(matches(kv["filtername"], names), "%s: filter %s" % (name, kv["filtername"]))
            if "model" in kv and not kv["model"].startswith("*"):
                self.assertIn(kv["model"], lab.MODELS, "%s: undeclared model" % name)

        # Every scene is started at map load and has a button.
        started = {c.split(",")[0] for cls, _, outs in found if cls == "logic_auto" for _, c in outs}
        relays = {kv["targetname"] for cls, kv, _ in found
                  if cls == "logic_relay" and kv["targetname"].startswith("restart_")}
        self.assertTrue(relays)
        self.assertEqual(relays, started)
        pressed = {c.split(",")[0] for cls, _, outs in found if cls == "func_button" for _, c in outs}
        self.assertLessEqual(relays, pressed)

        # Counted objects are renamed counted_<scene>, which the scene's restart kills.
        for prefix in renamed:
            scene = prefix[len("counted_"):].split("_")[0]
            self.assertTrue(any(r.startswith("restart_" + scene) for r in relays), prefix)
        return found

    def test_every_map(self):
        for name in lab.MAPS:
            with self.subTest(map=name):
                self.check_map(name)

    def test_tunnel_lanes(self):
        found = self.check_map("phys_tunnel")
        thrusters = [kv for cls, kv, _ in found if cls == "phys_thruster"]
        self.assertEqual(len(thrusters), lab.TUNNEL_SHOTS * len(lab.TUNNEL_LANES))
        for kv in thrusters:
            # Start active, linear force, mass independent (an acceleration), at the center.
            self.assertEqual(int(kv["spawnflags"]), 0x01 | 0x02 | 0x10 | 0x20)
            self.assertGreaterEqual(float(kv["force"]) * float(kv["forcetime"]), lab.SPEED_LIMIT)
        counters = [kv for cls, kv, _ in found if cls == "trigger_multiple" and kv["spawnflags"] == "8"]
        self.assertEqual(len(counters), len(lab.TUNNEL_LANES))

    def test_cylinder_faces_point_outward(self):
        vmf = lab.gyro.Vmf()
        text = lab.cylinder_y(vmf, (0, 0, 0), 16, 40, 24, lab.BODY)
        planes = re.findall(r'"plane" "\(([^)]*)\) \(([^)]*)\) \(([^)]*)\)"', text)
        self.assertEqual(len(planes), 26)
        for p0, p1, p2 in planes:
            a, b, c = ([float(x) for x in p.split()] for p in (p0, p1, p2))
            normal = lab.gyro.cross(lab.gyro.sub(a, b), lab.gyro.sub(c, b))
            # Outward: the plane's normal points away from the center.
            self.assertGreater(lab.gyro.dot(normal, b), 0)


if __name__ == "__main__":
    unittest.main()
