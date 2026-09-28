#!/usr/bin/env python3
"""End-to-end check of the Hammer MCP server (RFC 0002, R08-MCP).

Starts the real `hammer_cli --mcp` process and talks to it the way an MCP
client does, over newline-delimited JSON-RPC on stdin/stdout. Every line the
server writes is parsed with Python's json module, independently of the
server's own JSON code. Through tool calls it authors the room of the
UI-driven suite (a 384x384 block hollowed with 16-unit walls, a player start
and a light) and builds it with build_map; tools/quality/hammer_ui_test.py's
oracle then judges the saved VMF and the build record.

Negative controls go through the same stream:

- a malformed line must get a -32700 error with a null id;
- an unknown tool must get -32602;
- a failing command (a degenerate block) must be a tool result with isError;
- `info` must be the same before and after these three.

  hammer_mcp_check.py --cli build-r03-tools/hammer/cli/hammer_cli --out quality-results/hammer-mcp

Results are reported as checks-v1.
"""

import argparse
import json
import shutil
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import hammer_ui_test  # noqa: E402  (the room oracle)
from conformance_result import Checks  # noqa: E402

ROOT = HERE.parents[1]
H = hammer_ui_test.ROOM_HALF


class Client:
    def __init__(self, argv, log):
        self.proc = subprocess.Popen(argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                     stderr=log, text=True, bufsize=1)
        self.next_id = 0
        self.lines = []

    def send(self, text):
        self.proc.stdin.write(text + "\n")
        self.proc.stdin.flush()

    def receive(self):
        line = self.proc.stdout.readline()
        self.lines.append(line)
        return json.loads(line)  # raises on anything that is not one JSON document

    def request(self, method, params=None):
        self.next_id += 1
        message = {"jsonrpc": "2.0", "id": self.next_id, "method": method}
        if params is not None:
            message["params"] = params
        self.send(json.dumps(message))
        reply = self.receive()
        if reply.get("id") != self.next_id:
            raise RuntimeError("reply id %r for request %d" % (reply.get("id"), self.next_id))
        return reply

    def call(self, name, **arguments):
        reply = self.request("tools/call", {"name": name, "arguments": arguments})
        result = reply.get("result", {})
        text = "".join(c.get("text", "") for c in result.get("content", []))
        return reply, text, result.get("isError")

    def close(self):
        self.proc.stdin.close()
        rest = self.proc.stdout.read()
        return self.proc.wait(timeout=30), rest


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--cli", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()

    checks = Checks()
    out = args.out.resolve()
    shutil.rmtree(out, ignore_errors=True)
    work = out / "author"
    work.mkdir(parents=True)
    log = open(out / "hammer_cli.stderr", "w")
    client = Client([str(args.cli.resolve()), "--mcp", "--root", str(work), "--repo", str(ROOT),
                     "--builds", str(out / "builds")], log)
    started = time.monotonic()
    try:
        init = client.request("initialize", {"protocolVersion": "2025-06-18", "capabilities": {},
                                             "clientInfo": {"name": "hammer_mcp_check", "version": "1"}})
        checks.equal(init.get("result", {}).get("protocolVersion"), "2025-06-18", "initialize.version")
        client.send(json.dumps({"jsonrpc": "2.0", "method": "notifications/initialized"}))

        tools = client.request("tools/list").get("result", {}).get("tools", [])
        catalog = subprocess.run([str(args.cli.resolve()), "--commands"], capture_output=True,
                                 text=True).stdout
        names = [line.split()[0] for line in catalog.splitlines() if line and not line[0].isspace()]
        checks.equal([t["name"] for t in tools], names, "tools.match-catalog")
        checks.check(all(t["inputSchema"]["type"] == "object" for t in tools), "tools.object-schemas")

        _, _, error = client.call("new_map")
        checks.equal(error, False, "call.new-map")
        _, block, error = client.call("create_block", mins=[-H, -H, 0], maxs=[H, H, 128])
        checks.check(not error and block.isdigit(), "call.create-block", block)
        _, walls, error = client.call("hollow", id=block, thickness=16)
        checks.check(error is False, "call.hollow", walls)
        for classname, (x, z) in (("info_player_start", hammer_ui_test.PLAYER),
                                  ("light", hammer_ui_test.LIGHT)):
            _, text, error = client.call("place_entity", classname=classname, origin=[x, 0, z])
            checks.check(error is False, "call.place-" + classname, text)

        # Negative controls through the same stream.
        _, before, _ = client.call("info")
        client.send('{"jsonrpc":"2.0","id":999,"method":"tools/call",')
        malformed = client.receive()
        checks.check(malformed.get("error", {}).get("code") == -32700 and malformed.get("id") is None,
                     "control.malformed-line", json.dumps(malformed))
        unknown, _, _ = client.call("no_such_tool")
        checks.equal(unknown.get("error", {}).get("code"), -32602, "control.unknown-tool")
        _, text, error = client.call("create_block", mins="0 0 0", maxs="0 0 0")
        checks.check(error is True and "rejected" in text, "control.command-error", text)
        _, after, _ = client.call("info")
        checks.check(before == after and before.startswith("solids=6 entities=2"),
                     "control.state-unchanged", "%s / %s" % (before, after))

        _, built, error = client.call("build_map", path="ui_room.vmf", quality="fast", publish=False)
        checks.check(not error and built == "pass", "call.build-map", built)
    except (RuntimeError, ValueError, KeyError, OSError) as failure:
        checks.check(False, "protocol.stream", str(failure))
    code, rest = client.close()
    checks.equal(code, 0, "exit.clean")
    checks.equal(rest, "", "stdout.only-replies")
    elapsed = time.monotonic() - started

    verdict = hammer_ui_test.judge(work / "ui_room.vmf", out / "builds" / "ui_room" / "build.json")
    for name, (ok, detail) in verdict.items():
        checks.check(ok, "room." + name, detail)
    (out / "hammer-mcp.json").write_text(json.dumps(
        {"schema": "hammer-mcp/v1", "elapsed_seconds": round(elapsed, 2),
         "verdict": {k: {"ok": ok, "detail": d} for k, (ok, d) in verdict.items()},
         "transcript_lines": len(client.lines)}, indent=2) + "\n")
    print("hammer_mcp_check: author -> built map in %.1f s" % elapsed)
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
