#!/usr/bin/env python3
"""Run a JSON plan of armorpaint-mcp tool calls over the real MCP stdio protocol.

Run this with the armorpaint-mcp virtual environment's Python (it needs the `mcp`
SDK). The plan is `{"calls": [{"tool": ..., "args": {...}, "save": "name"}]}`.
An argument value `{"$ref": "name"}` is replaced by the `result.id` that the
call saved as `name` returned; ArmorPaint assigns node ids, so graphs are
written against names. Every call is appended to the transcript as one JSON
line. The run stops at the first transport error or `ok: false` reply.
"""

from __future__ import annotations

import asyncio
import json
import os
from pathlib import Path
import sys
import time

from mcp import ClientSession, StdioServerParameters
from mcp.client.stdio import stdio_client


def resolve(value, saved: dict):
    if isinstance(value, dict):
        if set(value) == {"$ref"}:
            return saved[value["$ref"]]
        return {key: resolve(item, saved) for key, item in value.items()}
    if isinstance(value, list):
        return [resolve(item, saved) for item in value]
    return value


async def run(plan: dict, transcript: Path, server_dir: Path, armorpaint_dir: Path) -> int:
    params = StdioServerParameters(
        command=sys.executable, args=["-m", "armorpaint_mcp"], cwd=str(server_dir),
        env={**os.environ, "ARMORPAINT_DIR": str(armorpaint_dir)},
    )
    saved: dict[str, int] = {}
    async with stdio_client(params) as (reader, writer):
        async with ClientSession(reader, writer) as session:
            await session.initialize()
            for step in plan["calls"]:
                args = resolve(step.get("args", {}), saved)
                started = time.monotonic()
                reply = await session.call_tool(step["tool"], args)
                text = "\n".join(item.text for item in reply.content if item.type == "text")
                record = {"stage": plan.get("stage", ""), "tool": step["tool"], "args": args,
                          "ms": round((time.monotonic() - started) * 1000),
                          "error": bool(reply.isError), "result": text[:4000]}
                with transcript.open("a", encoding="utf-8") as log:
                    log.write(json.dumps(record) + "\n")
                try:
                    body = json.loads(text)
                except ValueError:
                    body = {"ok": False, "error": text}
                if reply.isError or body.get("ok") is False:
                    print(f"{step['tool']} failed: {text}", file=sys.stderr)
                    return 1
                if "save" in step:
                    saved[step["save"]] = body["result"]["id"]
    print(json.dumps(saved))
    return 0


def main() -> int:
    if len(sys.argv) != 5:
        print("usage: armorpaint_mcp_plan.py <plan.json> <transcript.jsonl> <mcp-server-dir> "
              "<armorpaint-dir>", file=sys.stderr)
        return 2
    plan = json.loads(Path(sys.argv[1]).read_text(encoding="utf-8"))
    return asyncio.run(run(plan, Path(sys.argv[2]), Path(sys.argv[3]), Path(sys.argv[4])))


if __name__ == "__main__":
    raise SystemExit(main())
