#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Load the Portal product profile and verify/fetch its pinned native dependency."""

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import platform
import posixpath
import re
import shutil
import subprocess
import tarfile
import tempfile
import urllib.parse
import urllib.request


DEFAULT_PROFILE = Path(__file__).resolve().parents[2] / "quality/product_profiles/portal-linux-wayland.json"
MAX_MEMBERS = 8192
MAX_EXPANDED_BYTES = 512 * 1024 * 1024


class ProfileError(ValueError):
    pass


def _relative_path(value):
    if not isinstance(value, str) or not value or "\\" in value:
        raise ProfileError("package path must be a nonempty relative POSIX path")
    path = PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts or value in (".", ".."):
        raise ProfileError("package path escapes its root: %r" % value)
    return path


def load_profile(path=DEFAULT_PROFILE):
    try:
        profile = json.loads(Path(path).read_text())
        if profile["schema"] != "source-product-profile/v1":
            raise ProfileError("unsupported product profile schema")
        if not profile["id"] or not profile["target"]["architecture"]:
            raise ProfileError("product profile requires an identity and architecture")
        if profile["toolchain"]["family"] != "gcc":
            raise ProfileError("this profile checker requires the declared GCC toolchain")
        if not profile["dependencies"]["pkg_config"]:
            raise ProfileError("product profile requires pinned pkg-config dependencies")
        for name, version in profile["dependencies"]["pkg_config"].items():
            if not re.fullmatch(r"[A-Za-z0-9_.+-]+", name) or not isinstance(version, str) or not version:
                raise ProfileError("invalid pkg-config dependency pin")
        if "dxvk_native" in profile["dependencies"]:
            dep = profile["dependencies"]["dxvk_native"]
            parsed = urllib.parse.urlparse(dep["url"])
            if parsed.scheme != "https" or not parsed.netloc or parsed.username or parsed.password:
                raise ProfileError("dependency downloads require a public HTTPS URL")
            if not re.fullmatch(r"[0-9a-f]{64}", dep["sha256"]):
                raise ProfileError("dependency requires an exact SHA-256 pin")
            if not isinstance(dep["archive_bytes"], int) or dep["archive_bytes"] <= 0:
                raise ProfileError("dependency requires a positive archive size")
            for field in ("cache_archive", "extracted_directory", "prefix"):
                if len(_relative_path(dep[field]).parts) != 1:
                    raise ProfileError("%s must name one path component" % field)
            if not dep["required_files"]:
                raise ProfileError("dependency must declare required files")
            for required in dep["required_files"]:
                _relative_path(required)
            for field in ("include_directory", "library_directory"):
                _relative_path(dep[field])
        return profile
    except (OSError, KeyError, TypeError, json.JSONDecodeError) as error:
        raise ProfileError("invalid product profile: %s" % error) from error


def _run(command):
    try:
        result = subprocess.run(command, check=True, capture_output=True, text=True, timeout=30)
        return result.stdout.strip()
    except (OSError, subprocess.SubprocessError) as error:
        raise ProfileError("required profile probe failed: %s" % " ".join(command)) from error


def check_environment(profile, cxx=None, pkg_config="pkg-config"):
    """Return only declared observations, never the process environment."""
    target = profile["target"]
    observed = {"os": platform.system().lower(), "architecture": platform.machine(), "packages": {}}
    if observed["os"] != target["os"] or observed["architecture"] != target["architecture"]:
        raise ProfileError("profile requires %(os)s/%(architecture)s" % target)
    compiler = cxx or profile["toolchain"]["cxx"]
    compiler = [compiler] if isinstance(compiler, str) else list(compiler)
    version = _run(compiler + ["-dumpfullversion"])
    banner = _run(compiler + ["--version"]).splitlines()[0]
    if version != profile["toolchain"]["version"] or "clang" in banner.lower():
        raise ProfileError("profile requires GCC %s; observed %s" %
                           (profile["toolchain"]["version"], banner))
    observed["compiler"] = {"command": compiler, "version": version, "banner": banner}
    for name, required in profile["dependencies"]["pkg_config"].items():
        version = _run([pkg_config, "--modversion", name])
        if version != required:
            raise ProfileError("profile requires %s %s; observed %s" % (name, required, version))
        observed["packages"][name] = version
    return observed


def _sha256_stream(stream):
    digest = hashlib.sha256()
    for block in iter(lambda: stream.read(1024 * 1024), b""):
        digest.update(block)
    return digest.hexdigest()


def _sha256_file(path):
    with Path(path).open("rb") as stream:
        return _sha256_stream(stream)


def verify_archive(dependency, archive):
    archive = Path(archive)
    if not archive.is_file() or archive.is_symlink():
        raise ProfileError("pinned archive is missing or is a symlink: %s" % archive)
    if archive.stat().st_size != dependency["archive_bytes"]:
        raise ProfileError("pinned archive size mismatch: %s" % archive)
    if _sha256_file(archive) != dependency["sha256"]:
        raise ProfileError("pinned archive SHA-256 mismatch: %s" % archive)


def _members(archive):
    members = {}
    expanded = 0
    for member in archive:
        path = _relative_path(member.name)
        name = path.as_posix()
        if name in members:
            raise ProfileError("duplicate archive member: %s" % name)
        if not (member.isdir() or member.isfile() or member.issym()):
            raise ProfileError("unsupported archive entry: %s" % name)
        if member.size < 0:
            raise ProfileError("negative archive member size")
        expanded += member.size
        if len(members) >= MAX_MEMBERS or expanded > MAX_EXPANDED_BYTES:
            raise ProfileError("archive exceeds extraction budget")
        members[name] = member
    if not members:
        raise ProfileError("empty dependency archive")
    for name, member in members.items():
        for parent in PurePosixPath(name).parents:
            ancestor = members.get(parent.as_posix())
            if ancestor is not None and not ancestor.isdir():
                raise ProfileError("archive member has a non-directory parent: %s" % name)
        if member.issym():
            current, seen = name, set()
            while members[current].issym():
                if current in seen:
                    raise ProfileError("archive symlink cycle: %s" % name)
                seen.add(current)
                link = members[current].linkname
                if not link or "\\" in link or PurePosixPath(link).is_absolute():
                    raise ProfileError("unsafe archive symlink: %s" % current)
                current = posixpath.normpath(posixpath.join(posixpath.dirname(current), link))
                _relative_path(current)
                if current not in members:
                    raise ProfileError("archive symlink target is missing: %s" % current)
                if not (members[current].issym() or members[current].isfile()):
                    raise ProfileError("archive symlink must resolve to a file: %s" % name)
    return members


def safe_extract(archive_path, destination):
    """Extract into a fresh tree; no tarfile extraction filters are bypassed."""
    destination = Path(destination)
    if destination.exists() or destination.is_symlink():
        raise ProfileError("refusing to replace an existing dependency tree: %s" % destination)
    destination.parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix=".dependency-", dir=destination.parent))
    try:
        with tarfile.open(archive_path, "r:gz") as archive:
            members = _members(archive)
            # No symlink exists while files are written. Soname links are last.
            for name, member in members.items():
                output = temporary / name
                if member.isdir():
                    output.mkdir(parents=True, exist_ok=True)
                elif member.isfile():
                    output.parent.mkdir(parents=True, exist_ok=True)
                    with archive.extractfile(member) as source, output.open("xb") as target:
                        shutil.copyfileobj(source, target)
                    output.chmod(0o755 if member.mode & 0o111 else 0o644)
            for name, member in members.items():
                if member.issym():
                    output = temporary / name
                    output.parent.mkdir(parents=True, exist_ok=True)
                    output.symlink_to(member.linkname)
        if destination.exists() or destination.is_symlink():
            raise ProfileError("dependency destination appeared during extraction")
        temporary.rename(destination)
    except (tarfile.TarError, OSError) as error:
        raise ProfileError("dependency extraction failed: %s" % error) from error
    finally:
        if temporary.exists():
            shutil.rmtree(temporary)


def verify_dependency(profile, name, prefix, archive=None):
    dependency = profile["dependencies"][name]
    prefix = Path(prefix).absolute()
    if prefix.name != dependency["prefix"] or prefix.is_symlink() or not prefix.is_dir():
        raise ProfileError("dependency prefix must be the extracted %s directory" % dependency["prefix"])
    tree = prefix.parent
    archive = Path(archive) if archive else tree.parent / dependency["cache_archive"]
    verify_archive(dependency, archive)
    with tarfile.open(archive, "r:gz") as source:
        members = _members(source)
        expected = set()
        for member_path, member in members.items():
            output = tree / member_path
            expected.add(member_path)
            expected.update(p.as_posix() for p in PurePosixPath(member_path).parents if str(p) != ".")
            if member.issym():
                if not output.is_symlink() or os.readlink(output) != member.linkname:
                    raise ProfileError("dependency symlink differs from archive: %s" % member_path)
            elif member.isdir():
                if output.is_symlink() or not output.is_dir():
                    raise ProfileError("dependency directory differs from archive: %s" % member_path)
            else:
                if output.is_symlink() or not output.is_file():
                    raise ProfileError("dependency file is missing or substituted: %s" % member_path)
                with source.extractfile(member) as stream:
                    digest = _sha256_stream(stream)
                if output.stat().st_size != member.size or _sha256_file(output) != digest:
                    raise ProfileError("dependency file differs from pinned archive: %s" % member_path)
        actual = {p.relative_to(tree).as_posix() for p in tree.rglob("*")}
        if actual != expected:
            raise ProfileError("dependency tree has missing or additional entries")
    for required in dependency["required_files"]:
        if not (prefix / required).is_file():
            raise ProfileError("dependency required file missing: %s" % required)
    return {"prefix": str(prefix), "archive": str(archive), "sha256": dependency["sha256"],
            "members": len(members)}


class _HTTPSRedirects(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, request, fp, code, message, headers, newurl):
        if urllib.parse.urlparse(newurl).scheme != "https":
            raise ProfileError("dependency redirect attempted to leave HTTPS")
        return super().redirect_request(request, fp, code, message, headers, newurl)


def fetch_dependency(profile, cache_dir):
    dependency = profile["dependencies"]["dxvk_native"]
    cache = Path(cache_dir)
    cache.mkdir(parents=True, exist_ok=True)
    archive = cache / dependency["cache_archive"]
    if not archive.exists():
        temporary = None
        try:
            with tempfile.NamedTemporaryFile(dir=cache, prefix=".download-", delete=False) as output:
                temporary = Path(output.name)
                request = urllib.request.Request(dependency["url"], headers={"User-Agent": "Source-profile-fetch/1"})
                with urllib.request.build_opener(_HTTPSRedirects()).open(request, timeout=60) as response:
                    count = 0
                    while block := response.read(1024 * 1024):
                        count += len(block)
                        if count > dependency["archive_bytes"]:
                            raise ProfileError("dependency download exceeded pinned size")
                        output.write(block)
            verify_archive(dependency, temporary)
            temporary.replace(archive)
        finally:
            if temporary is not None and temporary.exists():
                temporary.unlink()
    verify_archive(dependency, archive)
    tree = cache / dependency["extracted_directory"]
    if not tree.exists():
        safe_extract(archive, tree)
    prefix = tree / dependency["prefix"]
    verify_dependency(profile, "dxvk_native", prefix, archive)
    return prefix.resolve()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("fetch", "check"))
    parser.add_argument("--profile", type=Path, default=DEFAULT_PROFILE)
    parser.add_argument("--cache-dir", type=Path, default=Path("build/dependencies"))
    parser.add_argument("--dxvk-root", type=Path)
    parser.add_argument("--cxx")
    args = parser.parse_args(argv)
    try:
        profile = load_profile(args.profile)
        if args.action == "fetch":
            print(fetch_dependency(profile, args.cache_dir))
        else:
            if args.dxvk_root is None:
                parser.error("check requires --dxvk-root")
            result = check_environment(profile, cxx=args.cxx)
            result["dependency"] = verify_dependency(profile, "dxvk_native", args.dxvk_root)
            result["profile"] = profile["id"]
            print(json.dumps(result, indent=2, sort_keys=True))
        return 0
    except (ProfileError, OSError, tarfile.TarError) as error:
        parser.exit(1, "profile: %s\n" % error)


if __name__ == "__main__":
    raise SystemExit(main())
