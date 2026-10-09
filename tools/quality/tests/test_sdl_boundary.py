"""Self-tests for tools/quality/sdl_boundary.py (RFC 0001 R18): the boundary
accepts its declared owners and rejects each seeded violation."""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import sdl_boundary  # noqa: E402


class SdlBoundaryTest(unittest.TestCase):
    def tree(self, files, allowed_files=(), allowed_prefixes=("platform/sdl3/",)):
        root = Path(tempfile.mkdtemp())
        subprocess.run(["git", "init", "-q"], cwd=root, check=True)
        (root / "architecture").mkdir()
        (root / "architecture/sdl_boundary.json").write_text(json.dumps({
            "schema": "sdl-boundary/v1", "excluded_prefixes": ["thirdparty/"],
            "allowed_prefixes": list(allowed_prefixes), "allowed_files": list(allowed_files)}))
        for path, text in files.items():
            (root / path).parent.mkdir(parents=True, exist_ok=True)
            (root / path).write_text(text)
        return root

    def check(self, root):
        return sdl_boundary.check(root)

    def test_owners_pass(self):
        root = self.tree({"platform/sdl3/a.cpp": "#include <SDL3/SDL.h>\n",
                          "inputsystem/b.cpp": "void f() { SDL_PumpEvents(); }\n"},
                         allowed_files=["inputsystem/b.cpp"])
        self.assertEqual(self.check(root), 0)

    def test_new_include_fails(self):
        root = self.tree({"engine/host.cpp": '#include "SDL.h"\n'})
        self.assertEqual(self.check(root), 1)

    def test_new_call_fails(self):
        root = self.tree({"gameui/x.cpp": "void f() { SDL_OpenURL( \"x\" ); }\n"})
        self.assertEqual(self.check(root), 1)

    def test_stale_entry_fails(self):
        root = self.tree({"inputsystem/b.cpp": "void f() {}\n"}, allowed_files=["inputsystem/b.cpp"])
        self.assertEqual(self.check(root), 1)

    def test_comments_strings_and_opaque_types_are_not_uses(self):
        root = self.tree({"vgui2/a.cpp": "// SDL_GetMouseState( &x )\n/* SDL_Foo( */\n"
                                         "const char *s = \"SDL_Init(\";\n"
                                         "typedef struct SDL_Window SDL_Window;\n"
                                         "void SetMouseCursor( SDL_Cursor *c );\n"})
        self.assertEqual(self.check(root), 0)

    def test_excluded_third_party(self):
        root = self.tree({"thirdparty/SDL/SDL.h": "#include \"SDL_main.h\"\n"})
        self.assertEqual(self.check(root), 0)


if __name__ == "__main__":
    unittest.main()
