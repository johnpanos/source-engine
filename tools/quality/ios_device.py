#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""The connected iOS device, reached through the macOS host (`ssh macvm`).

Harnesses that run on the phone (ios_frame_pacing.py, ios_conformance.py)
share this one owner of the device plumbing: resolving the connected device,
copying files into and out of an app's data container, and launching an
installed app with arguments while capturing its console. Signing and
installing stay with ios-deploy.sh.

Everything goes through `xcrun devicectl` on the Mac. The Linux side never
talks to the device directly.
"""

import json
import re
import shlex
import subprocess
import tempfile
from pathlib import Path

DEFAULT_HOST = "macvm"
# devicectl's last line for an attached (--console) launch.
EXIT_PATTERN = re.compile(r"The app terminated with the exit code (-?\d+)")
SIGNAL_PATTERN = re.compile(r"(?:The app|App) terminated (?:due to|with) signal (\d+)")


LAUNCH_ATTEMPTS = 3


class DeviceError(RuntimeError):
    pass


class Device:
    """One connected iOS device and one app on it (by bundle identifier)."""

    def __init__(self, bundle_id, host=DEFAULT_HOST, identifier=None):
        self.host = host
        self.bundle_id = bundle_id
        self.identifier = identifier or self._resolve()

    # -- plumbing ---------------------------------------------------------
    def _ssh(self, script, timeout=None, check=True):
        """Runs a bash script on the Mac; returns (returncode, output)."""
        try:
            # One stream, so devicectl's status lines keep their place.
            proc = subprocess.run(["ssh", self.host, "bash -s"], input=script, text=True,
                                  stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                  timeout=timeout)
        except subprocess.TimeoutExpired as error:
            out = error.stdout.decode(errors="replace") if isinstance(error.stdout, bytes) \
                else (error.stdout or "")
            return None, out
        output = proc.stdout
        if check and proc.returncode != 0:
            raise DeviceError("ssh %s failed (%d): %s" % (self.host, proc.returncode,
                                                          output.strip()[-2000:]))
        return proc.returncode, output

    def _resolve(self):
        _, output = self._ssh(
            'T=$(mktemp -d); xcrun devicectl list devices --json-output "$T/d.json" >/dev/null; '
            'cat "$T/d.json"; rm -rf "$T"\n', timeout=120)
        start = output.find("{")
        devices = json.loads(output[start:])["result"]["devices"]
        for device in devices:
            if device.get("hardwareProperties", {}).get("platform") != "iOS":
                continue
            if device.get("connectionProperties", {}).get("tunnelState") == "unavailable":
                continue
            return device["identifier"]
        raise DeviceError("no connected iOS device (xcrun devicectl list devices)")

    def _devicectl(self, *args):
        return "xcrun devicectl " + " ".join(shlex.quote(a) for a in args)

    def _container(self):
        return ["--device", self.identifier, "--domain-type", "appDataContainer",
                "--domain-identifier", self.bundle_id]

    # -- files ------------------------------------------------------------
    def put(self, local, remote):
        """Copies a local file to `remote`, a path inside the app's container
        (e.g. Documents/portal/cfg/x.cfg)."""
        with tempfile.TemporaryDirectory() as _:
            staging = "/tmp/ios_device_%s" % Path(local).name
            subprocess.run(["scp", "-q", str(local), "%s:%s" % (self.host, staging)], check=True,
                           timeout=300)
            self._ssh("%s --quiet && rm -f %s\n" % (
                self._devicectl("device", "copy", "to", *self._container(), "--source", staging,
                                "--destination", remote), shlex.quote(staging)), timeout=300)

    def put_text(self, text, remote):
        with tempfile.NamedTemporaryFile("w", suffix="_" + Path(remote).name, delete=False) as f:
            f.write(text)
            path = f.name
        try:
            self.put(path, remote)
        finally:
            Path(path).unlink()

    def get(self, remote, local):
        """Copies `remote` from the app's container to `local`; False when absent."""
        staging = "/tmp/ios_device_get_%s" % Path(remote).name
        code, _ = self._ssh("rm -f %s; %s --quiet\n" % (
            shlex.quote(staging),
            self._devicectl("device", "copy", "from", *self._container(), "--source", remote,
                            "--destination", staging)), timeout=600, check=False)
        if code != 0:
            return False
        Path(local).parent.mkdir(parents=True, exist_ok=True)
        subprocess.run(["scp", "-q", "%s:%s" % (self.host, staging), str(local)], check=True,
                       timeout=600)
        self._ssh("rm -f %s\n" % shlex.quote(staging), check=False)
        return True

    # -- processes --------------------------------------------------------
    def launch(self, executable, args=(), timeout=600):
        """Launches the app attached to its console (terminating a running
        instance first) and waits for it to exit.

        Returns a dict: exit_code (None when unknown), signal, timed_out and
        console (the app's stdout and stderr, then devicectl's status lines).
        """
        # A launch the device never started (its connection dropped) is
        # retried: the app never ran, so there is no result to report.
        for attempt in range(LAUNCH_ATTEMPTS):
            code, output = self._ssh(self._devicectl(
                "device", "process", "launch", "--device", self.identifier,
                "--terminate-existing", "--console", self.bundle_id, *args) + "\n",
                timeout=timeout, check=False)
            if code is None or "Launched application with" in output:
                break
        result = {"exit_code": None, "signal": None, "timed_out": code is None,
                  "console": output}
        exit_match = EXIT_PATTERN.search(output)
        signal_match = SIGNAL_PATTERN.search(output)
        if exit_match:
            result["exit_code"] = int(exit_match.group(1))
        if signal_match:
            result["signal"] = int(signal_match.group(1))
        if result["timed_out"]:
            self.terminate(executable)
        return result

    def terminate(self, executable):
        """Stops the app if it is running (after a timed-out launch).
        `executable` is its bundle's executable name (CFBundleExecutable)."""
        script = (
            'T=$(mktemp -d); %s --json-output "$T/p.json" >/dev/null 2>&1; python3 -c '
            '"import json, sys; [print(p[\\"processIdentifier\\"]) for p in '
            'json.load(open(sys.argv[1]))[\\"result\\"][\\"runningProcesses\\"] '
            'if p.get(\\"executable\\", \\"\\").endswith(\\"/\\" + sys.argv[2])]" '
            '"$T/p.json" %s; rm -rf "$T"\n' % (
                self._devicectl("device", "info", "processes", "--device", self.identifier),
                shlex.quote(executable)))
        _, output = self._ssh(script, timeout=120, check=False)
        for pid in re.findall(r"^(\d+)$", output, re.M):
            self._ssh(self._devicectl("device", "process", "terminate", "--device",
                                      self.identifier, "--pid", pid) + "\n", timeout=120,
                      check=False)

    def describe(self):
        """Model, OS version and name of the device, for evidence."""
        _, output = self._ssh(
            'T=$(mktemp -d); %s --json-output "$T/d.json" >/dev/null; cat "$T/d.json"; '
            'rm -rf "$T"\n' % self._devicectl("device", "info", "details", "--device",
                                             self.identifier), timeout=120, check=False)
        try:
            result = json.loads(output[output.find("{"):])["result"]
        except (ValueError, KeyError):
            return {"identifier": self.identifier}
        hardware = result.get("hardwareProperties", {})
        properties = result.get("deviceProperties", {})
        return {"identifier": self.identifier,
                "model": hardware.get("marketingName") or hardware.get("productType"),
                "product_type": hardware.get("productType"),
                "cpu": hardware.get("cpuType", {}).get("name"),
                "os": "%s %s (%s)" % (hardware.get("platform"), properties.get("osVersionNumber"),
                                      properties.get("osBuildUpdate")),
                "name": properties.get("name")}


class ConformanceHost:
    """The conformance host app (tools/quality/ios_conformance.py) on the
    device: benchmark programs it carries run from its Documents, where
    their inputs are copied (quality/profiles/ios-arm64-device.json)."""

    def __init__(self, host=DEFAULT_HOST):
        root = Path(__file__).resolve().parents[2]
        self.profile = json.loads((root / "quality/profiles/ios-arm64-device.json").read_text())
        self.device = Device(self.profile["bundle_id"], host=host)
        self.pushed = {}

    def push(self, path, directory):
        """Copies a local input to Documents/<directory>/ once; returns the
        Documents-relative path a program opens."""
        if path not in self.pushed:
            remote = "%s/%s" % (directory, Path(path).name)
            self.device.put(path, "Documents/" + remote)
            self.pushed[path] = remote
        return self.pushed[path]

    def run(self, program, args=(), timeout=600):
        """Runs `program` with `args`; the launch result plus `returncode`
        (the exit code, or minus the signal)."""
        run = self.device.launch(self.profile["executable"], ["program:" + program, *args],
                                 timeout=timeout)
        run["returncode"] = run["exit_code"]
        if run["returncode"] is None and run["signal"]:
            run["returncode"] = -run["signal"]
        return run
