#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Self-tests for tools/quality/hammer_mcp_check.py (RFC 0002, R08-MCP).

The end-to-end check is trusted only after it is shown to fail, not crash, on
a server that breaks the stdio transport: a banner line on stdout, and a
server that answers correctly but then writes an unsolicited line.
"""

import os
import stat
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
CHECK = HERE.parent / "hammer_mcp_check.py"

# A stand-in hammer_cli: --commands lists one command; --mcp answers every
# request with an empty result (and a malformed line with -32700), after an
# optional banner and before an optional trailing line.
FAKE = r'''#!%s
import json, sys
if "--commands" in sys.argv:
    print("info\n    Report counts.")
    sys.exit(0)
if %r:
    print("hammer_cli ready")
    sys.stdout.flush()
for line in sys.stdin:
    try:
        message = json.loads(line)
    except ValueError:
        print(json.dumps({"jsonrpc": "2.0", "id": None, "error": {"code": -32700, "message": "parse"}}))
        sys.stdout.flush()
        continue
    if "id" in message:
        print(json.dumps({"jsonrpc": "2.0", "id": message["id"], "result": {}}))
        sys.stdout.flush()
if %r:
    print("goodbye")
'''


class TransportTest(unittest.TestCase):
    def run_with(self, banner, trailer):
        tmp = Path(tempfile.mkdtemp(prefix="hammer-mcp-check-"))
        cli = tmp / "hammer_cli"
        cli.write_text(FAKE % (sys.executable, banner, trailer))
        cli.chmod(cli.stat().st_mode | stat.S_IEXEC)
        return subprocess.run([sys.executable, str(CHECK), "--cli", str(cli), "--out", str(tmp / "out")],
                              capture_output=True, text=True, timeout=60, cwd=os.getcwd())

    def test_banner_on_stdout_fails(self):
        result = self.run_with(banner=True, trailer=False)
        self.assertNotEqual(0, result.returncode, result.stdout + result.stderr)
        self.assertIn("FAIL protocol.stream", result.stdout)

    def test_unsolicited_trailing_line_fails(self):
        result = self.run_with(banner=False, trailer=True)
        self.assertNotEqual(0, result.returncode)
        self.assertIn("FAIL stdout.only-replies", result.stdout)


if __name__ == "__main__":
    unittest.main()
