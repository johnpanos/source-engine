#!/usr/bin/env python3
"""Read-only Source style checks, with changed-line adoption for legacy files.

Formatting is delegated to the pinned clang-format, never approximated with
regexes. The lexical include check does not evaluate conditional compilation;
its conservative guarantee is that no include text follows memdbgon.h.
"""

from __future__ import annotations

import argparse
import bisect
import difflib
import os
from pathlib import Path
import re
import subprocess
import sys
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[2]
SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".inl", ".m", ".mm"}
# Vendored code retains its own conventions. No architectural rule is waived.
VENDOR_ROOTS = (
    "thirdparty/", "external/", "ivp/", "lib/", "common/protobuf-2.3.0/",
    "common/lzma/", "utils/lzma/", "utils/jpeglib/", "utils/bzip2/",
)
# Data fixtures whose exact text is part of an oracle (seeded constructs, line
# numbers in expectations, deliberately minimal toolchain inputs). They are
# test inputs, not first-party code, so formatting them would change the test.
DATA_FIXTURE_ROOTS = ("quality/fixtures/corpus/",)
HUNK = re.compile(rb"^@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@", re.MULTILINE)


class LintError(Exception):
    """An invalid invocation or unavailable/unusable lint dependency."""


def run(command: list[str], root: Path, *, data: bytes | None = None) -> bytes:
    try:
        result = subprocess.run(
            command, cwd=root, input=data, capture_output=True, timeout=120, check=False
        )
    except (OSError, subprocess.TimeoutExpired) as error:
        raise LintError(f"cannot run {command[0]}: {error}") from error
    if result.returncode:
        message = result.stderr.decode("utf-8", errors="replace").strip()
        raise LintError(f"{command[0]} failed ({result.returncode}): {message}")
    return result.stdout


def git(root: Path, *args: str) -> bytes:
    return run(["git", "--literal-pathspecs", *args], root)


def required_version(root: Path) -> str:
    requirement = (root / "tools/stylelint/requirements.txt").read_text().strip()
    match = re.fullmatch(r"clang-format==(\d+\.\d+\.\d+)", requirement)
    if not match:
        raise LintError("requirements.txt must pin exactly one clang-format version")
    return match[1]


def check_formatter(root: Path, executable: str) -> None:
    version = required_version(root)
    output = run([executable, "--version"], root).decode("utf-8", errors="replace")
    match = re.search(r"clang-format version (\d+\.\d+\.\d+)\b", output)
    if not match or match[1] != version:
        raise LintError(
            f"expected clang-format {version}, got {output.strip()!r}; "
            "install tools/stylelint/requirements.txt and/or set CLANG_FORMAT"
        )
    # Validate the root policy even when a documentation-only diff selects no C++.
    run([executable, f"--style=file:{root / '.clang-format'}", "--dump-config"], root)


def eligible(name: str) -> bool:
    return Path(name).suffix.lower() in SUFFIXES and not name.startswith(VENDOR_ROOTS + DATA_FIXTURE_ROOTS)


def file_names(root: Path, *args: str) -> set[str]:
    return {os.fsdecode(name) for name in git(root, *args).split(b"\0") if name}


def base_revision(root: Path, base: str | None) -> str:
    ref = base or "HEAD"
    # Resolve to a commit before constructing diff arguments; no raw revision options.
    commit = git(root, "rev-parse", "--verify", "--end-of-options", ref + "^{commit}").strip()
    if base:
        commit = git(root, "merge-base", commit.decode("ascii"), "HEAD").strip()
    return commit.decode("ascii")


def changed_ranges(patch: bytes, line_count: int) -> list[tuple[int, int]]:
    ranges = []
    for match in HUNK.finditer(patch):
        start = int(match[1])
        count = int(match[2]) if match[2] is not None else 1
        # A deletion can change indentation or join adjacent constructs. Inspect
        # its surviving boundary rather than passing no --lines (= the whole file).
        start = max(1, min(start, max(1, line_count)))
        end = max(start, min(start + max(1, count) - 1, max(1, line_count)))
        ranges.append((start, end))
    return ranges


def select_files(
    root: Path, *, changed: bool, all_files: bool, base: str | None, paths: list[str]
) -> tuple[dict[str, list[tuple[int, int]] | None], str | None]:
    """None means whole-file lint; ranges mean edited syntactic regions."""
    revision = base_revision(root, base) if changed else None
    if changed:
        names = file_names(
            root, "diff", "--no-renames", "--name-only", "-z", "--diff-filter=ACMT",
            revision, "--",
        ) | file_names(root, "ls-files", "--others", "--exclude-standard", "-z")
    elif all_files:
        names = file_names(root, "ls-files", "--cached", "--others", "--exclude-standard", "-z")
    else:
        names = set()
        for supplied in paths:
            path = Path(supplied)
            path = path if path.is_absolute() else root / path
            # Do not follow symlinks into vendors/outside the working tree.
            try:
                name = path.absolute().relative_to(root).as_posix()
            except ValueError as error:
                raise LintError(f"path is outside the repository: {supplied}") from error
            if ".." in Path(name).parts or not eligible(name) or not path.is_file():
                raise LintError(f"not a first-party C/C++ file: {supplied}")
            names.add(name)

    selected = {}
    # ls-tree avoids probing missing paths with a command whose failure could
    # otherwise hide a bad revision or Git error as a supposedly new file.
    old_names = file_names(root, "ls-tree", "-r", "--name-only", "-z", revision) if revision else set()
    for name in sorted(names):
        if not eligible(name):
            continue
        path = root / name
        if path.is_symlink() or path.resolve() != path.absolute():
            raise LintError(f"refusing symlinked source path: {name}")
        if not path.is_file():
            # Deleted tracked files can appear in --all's index enumeration.
            continue
        ranges = None
        if revision and name in old_names:
            patch = git(
                root, "diff", "--no-ext-diff", "--no-textconv", "--no-renames",
                "--text", "--unified=0", revision, "--", name,
            )
            ranges = changed_ranges(patch, len(path.read_bytes().splitlines()))
            if not ranges:
                # E.g. only an executable-bit change. Semantic include checks
                # still run, but clang-format must not format the whole file.
                ranges = []
        selected[name] = ranges
    return selected, revision


def splice_lines(text: str) -> tuple[str, list[int]]:
    """Apply escaped-newline splicing before recognizing C/C++ comments."""
    chars, lines = [], []
    line, index = 1, 0
    while index < len(text):
        if text.startswith("\\\r\n", index) or text.startswith("\\\n", index):
            index += 3 if text.startswith("\\\r\n", index) else 2
            line += 1
            continue
        char = text[index]
        chars.append(char)
        lines.append(line)
        line += char == "\n"
        index += 1
    return "".join(chars), lines


def mask_non_code(text: str) -> str:
    """Mask comments and literals (including raw strings), preserving offsets."""
    masked = list(text)
    token = re.compile(r'//[^\n]*|/\*[\s\S]*?(?:\*/|\Z)|(?:u8|[uUL])?R"([^\s()\\]{0,16})\(')
    index = 0
    while index < len(text):
        match = token.match(text, index)
        if match:
            end = match.end()
            if match.group(1) is not None:
                closing = ')' + match.group(1) + '"'
                found = text.find(closing, end)
                end = len(text) if found < 0 else found + len(closing)
        elif text[index] in {'"', "'"}:
            # Apostrophes between numeric token characters are digit separators.
            if text[index] == "'" and index and text[index - 1].isalnum() and index + 1 < len(text) and text[index + 1].isalnum():
                index += 1
                continue
            quote, end = text[index], index + 1
            while end < len(text):
                if text[end] == "\\":
                    end += 2
                elif text[end] == quote:
                    end += 1
                    break
                else:
                    end += 1
        else:
            index += 1
            continue
        for pos in range(index, min(end, len(text))):
            if text[pos] not in "\r\n":
                masked[pos] = " "
        index = end
    return "".join(masked)


def includes(text: str) -> list[tuple[int, str]]:
    text, source_lines = splice_lines(text)
    masked = mask_non_code(text)
    result = []
    offset = 0
    for original, code in zip(text.splitlines(keepends=True), masked.splitlines(keepends=True)):
        # Literal contents are masked; recognize only the directive in code, then
        # recover its operand from the original text. A raw string cannot fake it.
        directive = re.match(r'^[ \t]*#[ \t]*(?:include|include_next|import)\b', code)
        if directive:
            operand_start = offset + directive.end()
            # A block comment between keyword and operand may span lines.
            trivia = re.match(r'(?:[ \t]|/\*[\s\S]*?\*/)*', text[operand_start:])
            operand = text[operand_start + trivia.end():].split('\n', 1)[0].strip()
            match = re.match(r'["<]([^">]+)[">]', operand)
            result.append((source_lines[offset], match[1] if match else operand))
        offset += len(original)
    return result


def memdbgon_violations(text: str) -> list[tuple[int, str]]:
    seen = False
    violations = []
    for line, header in includes(text):
        if seen:
            violations.append((line, header))
        if header.replace("\\", "/").split("/")[-1] == "memdbgon.h":
            seen = True
    return violations


def newly_broken_includes(old: str | None, new: str) -> list[tuple[int, str]]:
    current = memdbgon_violations(new)
    if old is None:
        return current
    # Keep existing identical violating include occurrences, but not added copies.
    # Unchanged-line mapping also catches moving memdbgon before an old include.
    old_bad = dict(memdbgon_violations(old))
    allowed = set()
    matcher = difflib.SequenceMatcher(None, old.splitlines(), new.splitlines(), autojunk=False)
    for block in matcher.get_matching_blocks():
        for offset in range(block.size):
            old_line, new_line = block.a + offset + 1, block.b + offset + 1
            if old_line in old_bad:
                allowed.add((new_line, old_bad[old_line]))
    return [item for item in current if item not in allowed]


def source_text(data: bytes, name: str, old: str | None) -> str:
    """Retain legacy bytes only in unchanged baseline comments or literals.

    Surrogates are a lossless lexical view, never replacement characters or a
    source reencoding. New text and code tokens must still be valid UTF-8.
    """
    text = data.decode("utf-8", errors="surrogateescape")
    invalid = [index for index, char in enumerate(text) if "\udc80" <= char <= "\udcff"]
    if not invalid:
        return text
    if old is None:
        raise LintError(f"{name}: invalid UTF-8 without an unchanged Git baseline")
    masked = mask_non_code(text)
    if any(masked[index] != " " for index in invalid):
        raise LintError(f"{name}: invalid UTF-8 in a code token")
    unchanged = set()
    matcher = difflib.SequenceMatcher(None, old.splitlines(), text.splitlines(), autojunk=False)
    for block in matcher.get_matching_blocks():
        unchanged.update(range(block.b + 1, block.b + block.size + 1))
    starts = [0] + [match.end() for match in re.finditer("\n", text)]
    if any(bisect.bisect_right(starts, index) not in unchanged for index in invalid):
        raise LintError(f"{name}: invalid UTF-8 in new or modified comment/literal text")
    return text


def print_diff(name: str, original: bytes, formatted: bytes) -> None:
    """A redirected suggested patch must preserve historical bytes exactly."""
    difference = b"".join(difflib.diff_bytes(
        difflib.unified_diff, original.splitlines(keepends=True), formatted.splitlines(keepends=True),
        fromfile=os.fsencode(f"a/{name}"), tofile=os.fsencode(f"b/{name}"),
    ))
    if hasattr(sys.stdout, "buffer"):
        sys.stdout.flush()
        sys.stdout.buffer.write(difference)
        sys.stdout.buffer.flush()
    else:
        # StringIO is used by embedding callers and fixtures; round-trips bytes
        # with the same surrogateescape convention as the lexical source view.
        sys.stdout.write(difference.decode("utf-8", errors="surrogateescape"))


def format_bytes(
    root: Path, executable: str, name: str, data: bytes, ranges: list[tuple[int, int]] | None
) -> tuple[bytes, list[int]]:
    # clang-format emits no XML for an empty input. The caller has already
    # validated the formatter and policy; an empty file needs no replacements.
    if not data or ranges == []:
        return data, []
    command = [
        executable, f"--style=file:{root / '.clang-format'}",
        f"--assume-filename={name}", "--output-replacements-xml",
    ]
    if ranges is not None:
        command.extend(f"--lines={start}:{end}" for start, end in ranges)
    output = run(command, root, data=data)
    try:
        tree = ET.fromstring(output)
        if tree.tag != "replacements" or tree.attrib.get("incomplete_format") != "false":
            raise LintError(f"{name}: clang-format could not completely parse the selected source")
        edits = [(int(e.attrib["offset"]), int(e.attrib["length"]), (e.text or "").encode("utf-8")) for e in tree]
    except (ET.ParseError, KeyError, ValueError) as error:
        raise LintError(f"{name}: invalid clang-format replacement output") from error
    starts = [0] + [match.end() for match in re.finditer(b"\n", data)]
    locations = sorted({bisect.bisect_right(starts, offset) for offset, _, _ in edits})
    previous_end = 0
    for offset, length, _ in sorted(edits):
        if offset < previous_end or offset + length > len(data) or offset < 0 or length < 0:
            raise LintError(f"{name}: invalid/overlapping formatter edit")
        previous_end = offset + length
    formatted = data
    for offset, length, replacement in sorted(edits, reverse=True):
        formatted = formatted[:offset] + replacement + formatted[offset + length:]
    return formatted, locations


def main(argv: list[str] | None = None, *, root: Path = ROOT) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    selection = parser.add_mutually_exclusive_group()
    selection.add_argument("--changed", action="store_true", help="edited regions against HEAD or merge-base")
    selection.add_argument("--all", action="store_true", help="audit all first-party source (legacy debt may fail)")
    parser.add_argument("--base", help="compare merge-base(REF, HEAD) to working tree, including staged/untracked files")
    parser.add_argument("--diff", action="store_true", help="show suggested formatting patch; never writes files")
    parser.add_argument("--clang-format", default=os.environ.get("CLANG_FORMAT", "clang-format"))
    parser.add_argument("paths", nargs="*", help="explicit whole-file checks; paths are repository-relative")
    args = parser.parse_args(argv)
    if args.paths and (args.changed or args.all or args.base):
        parser.error("explicit paths cannot be combined with --changed, --all or --base")
    if args.base and not args.changed:
        parser.error("--base requires --changed")
    if not args.paths and not args.changed and not args.all:
        parser.error("select --changed, --all, or one or more explicit files")
    try:
        check_formatter(root, args.clang_format)
        selected, revision = select_files(
            root, changed=args.changed, all_files=args.all, base=args.base, paths=args.paths
        )
        old_names = file_names(root, "ls-tree", "-r", "--name-only", "-z", revision) if revision else set()
        failures = 0
        for name, ranges in selected.items():
            data = (root / name).read_bytes()
            old = git(root, "show", f"{revision}:{name}").decode("utf-8", errors="surrogateescape") if revision and name in old_names else None
            text = source_text(data, name, old)
            for line, header in newly_broken_includes(old, text):
                print(f"{name}:{line}: STYLE002 include {header!r} follows memdbgon.h; keep memdbgon last")
                failures += 1
            formatted, locations = format_bytes(root, args.clang_format, name, data, ranges)
            if formatted != data:
                failures += 1
                print(f"{name}:{locations[0] if locations else 1}: STYLE001 formatting differs from .clang-format")
                if args.diff:
                    print_diff(name, data, formatted)
        if not selected:
            print("stylelint: no eligible C/C++ changes (not applicable); formatter/config validated")
        else:
            print(f"stylelint: checked {len(selected)} file(s); {failures} failure(s)")
        return 1 if failures else 0
    except (LintError, OSError, UnicodeError) as error:
        print(f"stylelint: error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
