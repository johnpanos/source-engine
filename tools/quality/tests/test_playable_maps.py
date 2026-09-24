import json
import os
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import playable_maps  # noqa: E402


def content_root(root, name, extra=None):
    content = root / ("content-" + name)
    (content / "maps").mkdir(parents=True)
    (content / "materials" / name).mkdir(parents=True)
    (content / "maps" / (name + ".bsp")).write_bytes(b"bsp2")
    (content / "materials" / name / "wall.vmt").write_text("vmt")
    if extra:
        (content / extra).write_text("bad")
    return content


def summary(content, name, status="pass", failed=()):
    return {"map": name, "status": status, "failed_gates": list(failed),
            "content_root": str(content), "bsp2_sha256": "00"}


class PlayableMapsTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.root = Path(self.directory.name)
        self.store = self.root / "store"
        self.runtime = self.root / "runtime"
        (self.runtime / "portal" / "maps").mkdir(parents=True)

    def tearDown(self):
        self.directory.cleanup()

    def test_publish_replaces_previous_copy(self):
        content = content_root(self.root, "room")
        playable_maps.publish(summary(content, "room"), self.store)
        (content / "maps" / "room.bsp").write_bytes(b"newer")
        record = playable_maps.publish(summary(content, "room", "gate-failed", ["traversal"]),
                                       self.store)
        self.assertEqual(b"newer", (self.store / "room/maps/room.bsp").read_bytes())
        self.assertEqual(["traversal"], record["failed_gates"])
        self.assertEqual(["room"], [path.name for path in self.store.iterdir()])

    def test_publish_rejects_invalid_content(self):
        content = content_root(self.root, "room", extra="maps/notes.txt")
        with self.assertRaisesRegex(ValueError, "unsupported file"):
            playable_maps.publish(summary(content, "room"), self.store)
        content = content_root(self.root, "other")
        with self.assertRaisesRegex(ValueError, "no maps/room.bsp"):
            playable_maps.publish(summary(content, "room"), self.store)
        self.assertFalse((self.store / "room").exists())

    def test_mount_links_published_maps_and_drops_stale_mounts(self):
        playable_maps.publish(summary(content_root(self.root, "room"), "room"), self.store)
        playable_maps.publish(summary(content_root(self.root, "hall"), "hall"), self.store)
        mounted, skipped = playable_maps.mount(self.runtime, self.store)
        self.assertEqual({"hall", "room"}, set(mounted))
        self.assertEqual({}, skipped)
        link = self.runtime / "portal/custom/pbrt-room"
        self.assertTrue(link.is_symlink())
        self.assertFalse(os.path.isabs(os.readlink(link)))
        self.assertEqual(b"bsp2", (link / "maps/room.bsp").read_bytes())
        playable_maps.remove("hall", self.store)
        mounted, _ = playable_maps.mount(self.runtime, self.store)
        self.assertEqual({"room"}, set(mounted))
        self.assertFalse((self.runtime / "portal/custom/pbrt-hall").is_symlink())

    def test_mount_never_shadows_a_base_map_or_unmanaged_directory(self):
        playable_maps.publish(summary(content_root(self.root, "room"), "room"), self.store)
        (self.runtime / "portal/maps/room.bsp").write_bytes(b"shipped")
        mounted, skipped = playable_maps.mount(self.runtime, self.store)
        self.assertEqual({}, mounted)
        self.assertIn("room", skipped)
        self.assertFalse((self.runtime / "portal/custom/pbrt-room").exists())
        (self.runtime / "portal/maps/room.bsp").unlink()
        (self.runtime / "portal/custom/pbrt-mine").mkdir(parents=True)
        with self.assertRaisesRegex(ValueError, "unmanaged"):
            playable_maps.mount(self.runtime, self.store)

    def test_launch_arguments_only_for_published_maps(self):
        playable_maps.publish(summary(content_root(self.root, "room"), "room"), self.store)
        self.assertEqual(["+sv_cheats 1", "+r_worldmesh_draw 2"],
                         playable_maps.launch_arguments("room", self.store))
        self.assertEqual([], playable_maps.launch_arguments("testchmb_a_01", self.store))

    def test_invalid_record_is_rejected(self):
        playable_maps.publish(summary(content_root(self.root, "room"), "room"), self.store)
        record = self.store / "room" / playable_maps.RECORD
        data = json.loads(record.read_text())
        data["map"] = "other"
        record.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, "invalid published map"):
            playable_maps.published(self.store)


if __name__ == "__main__":
    unittest.main()
