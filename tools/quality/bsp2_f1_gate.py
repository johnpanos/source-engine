#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Run the pinned RFC 0008 F1 content and native product checks together.

Licensed Portal and Portal 2 content, both built Portal products, and bsp2tool
are required inputs. Each native check stages a private runtime.
"""

import argparse
import json
import os
from pathlib import Path
import sys

import bsp2_dedicated
import bsp2_reader
import bsp2_sparse_export
import bsp2_v19_derivative
import conformance


SCHEMA = "bsp2-f1-gate-evidence/v1"


def run_gate(args):
    root = Path(conformance.repo_root())
    output = Path(args.out).resolve()
    portal = Path(args.portal_runtime).resolve()
    portal2 = Path(args.portal2_runtime).resolve()
    dedicated = Path(args.dedicated_build).resolve()
    client = Path(args.client_build).resolve()
    tool = Path(args.tool).resolve()
    if any(source == output or source in output.parents for source in
           (portal, portal2, dedicated, client)):
        print("BSP2 F1 gate: fail (output must be outside source runtimes and builds)")
        return 1
    if output.exists() and not output.is_dir():
        print("BSP2 F1 gate: fail (output path is not a directory)")
        return 1
    evidence = {"schema": SCHEMA,
                "scope": "Q-CONTENT and Linux native map-container slice; not full RFC 0008 acceptance",
                "source": conformance.source_identity(root),
                "inputs": {"portal_runtime": str(portal), "portal2_runtime": str(portal2),
                           "dedicated_build": str(dedicated), "client_build": str(client),
                           "tool": str(tool)}, "steps": [], "failures": []}
    existing_output = output.exists() and any(output.iterdir())
    output.mkdir(parents=True, exist_ok=True)
    if existing_output:
        evidence["failures"].append("output directory is not empty; use a fresh evidence directory")
    required = ((portal / "portal/gameinfo.txt", "Portal runtime"),
                (portal2 / "portal2/gameinfo.txt", "Portal 2 runtime"),
                (dedicated, "dedicated build"), (client, "client build"),
                (tool, "bsp2tool"))
    for path, label in required:
        if not path.exists():
            evidence["failures"].append("required %s is missing: %s" % (label, path))
    if not os.access(tool, os.X_OK):
        evidence["failures"].append("required bsp2tool is not executable")

    def step(name, action, evidence_path, status_key):
        try:
            code = action()
            artifact = json.loads(evidence_path.read_text())
            observed = artifact.get(status_key)
            passed = code == 0 and observed == "pass"
            entry = {"name": name, "status": "pass" if passed else "fail",
                     "exit_code": code, "evidence": str(evidence_path),
                     "sha256": bsp2_dedicated.sha256(evidence_path)}
            if not passed:
                evidence["failures"].append("%s failed: exit=%s status=%s" % (name, code, observed))
        except Exception as error:
            entry = {"name": name, "status": "fail", "error": str(error)}
            evidence["failures"].append("%s failed: %s" % (name, error))
        evidence["steps"].append(entry)

    if not evidence["failures"]:
        v20_inventory = root / "quality/fixtures/bsp2-corpus-v20.json"
        v21_inventory = root / "quality/fixtures/bsp2-corpus-v21.json"
        v20_cases = root / "quality/fixtures/bsp2-dedicated-cases.json"
        v21_cases = root / "quality/fixtures/bsp2-v21-engine-cases.json"
        v19_cases = root / "quality/fixtures/bsp2-v19-derived-cases.json"

        for name, source_maps, inventory in (
                ("v20-corpus", portal / "portal/maps", v20_inventory),
                ("v21-corpus", portal2 / "portal2/maps", v21_inventory)):
            path = output / (name + ".json")
            step(name, lambda p=path, s=source_maps, i=inventory: bsp2_reader.main([
                "corpus", "--tool", str(tool), "--expect-manifest", str(i),
                "--out", str(p), str(s)]), path, "outcome")

        sparse_dir = output / "v20-sparse-export"
        sparse_path = sparse_dir / "evidence.json"
        step("v20-sparse-export", lambda: bsp2_sparse_export.main([
            "--legacy-map", str(portal / "portal/maps" / bsp2_sparse_export.SOURCE_NAME),
            "--tool", str(tool), "--inventory", str(v20_inventory),
            "--out", str(sparse_dir)]), sparse_path, "status")

        def native(name, product, source_maps, inventory, cases, build):
            path = output / name / "evidence.json"
            command = ["--runtime", str(portal), "--build", str(build), "--tool", str(tool),
                       "--source-inventory", str(inventory), "--cases", str(cases),
                       "--product", product, "--out", str(path.parent)]
            if source_maps != portal / "portal/maps":
                command += ["--map-source-root", str(source_maps)]
            step(name, lambda: bsp2_dedicated.main(command), path, "status")

        native("v20-dedicated", "dedicated", portal / "portal/maps",
               v20_inventory, v20_cases, dedicated)
        for product, build in (("dedicated", dedicated), ("client", client)):
            native("v21-" + product, product, portal2 / "portal2/maps",
                   v21_inventory, v21_cases, build)

        derivative_dir = output / "v19-derivative"
        derivative_path = derivative_dir / "derivation.json"
        def derive_v19():
            bsp2_v19_derivative.derive(portal / "portal/maps", v20_inventory, derivative_dir)
            return 0

        step("v19-derivative", derive_v19, derivative_path, "status")
        if evidence["steps"][-1]["status"] == "pass":
            for product, build in (("dedicated", dedicated), ("client", client)):
                native("v19-derived-" + product, product, derivative_dir,
                       derivative_dir / "inventory.json", v19_cases, build)
        else:
            evidence["failures"].append("v19 native checks require the derivative")
        try:
            (derivative_dir / bsp2_v19_derivative.OUTPUT_NAME).unlink(missing_ok=True)
        except OSError as error:
            evidence["failures"].append("failed to remove private v19 map copy: %s" % error)

    evidence["status"] = "pass" if len(evidence["steps"]) == 9 and not evidence["failures"] else "fail"
    path = output / "evidence.json"
    path.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("BSP2 F1 content/native gate: %s (%s)" % (evidence["status"], path))
    for failure in evidence["failures"]:
        print("  " + failure)
    return 0 if evidence["status"] == "pass" else 1


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--portal-runtime", required=True)
    parser.add_argument("--portal2-runtime", required=True)
    parser.add_argument("--dedicated-build", required=True)
    parser.add_argument("--client-build", required=True)
    parser.add_argument("--tool", required=True)
    parser.add_argument("--out", required=True)
    return run_gate(parser.parse_args(argv))


if __name__ == "__main__":
    sys.exit(main())
