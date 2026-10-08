#!/usr/bin/env python3
"""Reads and reports a guest-time profile of the 3DS build.

  guest_profile.py [profile.bin] [--elf ELF] [--top N] [--callers F] [--lines F]
                   [--window START END]

The patched Azahar's harness samples core 0 every N microseconds of emulated
time ("profile start <us>", "profile stop <path>"; probe.py --guest-profile
drives it). Each sample is one record of 68 little-endian u32: the running
thread's id (0: core 0 idle), pc, lr, the low 32 bits of core 0's system
tick and 160 stack words. --window START END keeps one slow frame's samples
(the ticks the game's "pica: slow frame" line prints). Because samples are
taken on the emulated clock, shares are shares of emulated time on core 0:
host costs (the emulator's renderer, the JIT) do not distort them, and idle
time is visible.

Inclusive time counts the pc's function and every lr or stack word that is a
real return address (the instruction before it in the ELF is BL, BLX imm or
BLX reg), once per sample. Callers: for each of the hottest self functions,
the nearest caller outside the C library, which names who drives libc time
(memcpy, printf, malloc).
"""

import argparse
import collections
import struct
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import n3ds_tree  # noqa: E402

RECORD_WORDS = 164
TEXT_START, TEXT_END = 0x00100000, 0x04000000
ADDR2LINE = n3ds_tree.ROOT / "dependencies/3ds/devkitpro/devkitARM/bin/arm-none-eabi-addr2line"

# Functions whose time belongs to their caller (the C library, libctru locks).
LIBRARY = {
    "memcpy", "memset", "memmove", "memcmp", "strlen", "strcmp", "strcpy", "strncpy", "strchr",
    "_svfprintf_r", "_vfprintf_r", "__ssprint_r", "__ssputs_r", "__sprint_r", "__sfvwrite_r",
    "_dtoa_r", "__d2b", "_localeconv_r", "__utf8_mbtowc", "__locale_mb_cur_max", "_vsnprintf_r",
    "vsnprintf", "snprintf", "_snprintf_r", "sprintf", "_sprintf_r", "vsprintf", "_vsprintf_r",
    "printf", "_printf_r", "vfprintf", "fwrite", "_fwrite_r", "_fflush_r", "__swrite", "_write_r",
    "fputs", "_fputs_r", "puts", "_puts_r", "putchar", "fputc", "_fputc_r", "__swbuf_r",
    "_malloc_r", "_free_r", "_memalign_r", "_realloc_r", "_calloc_r", "malloc", "free", "realloc",
    "calloc", "memalign", "operator new(unsigned int)", "operator delete(void*)",
    "operator delete(void*, unsigned int)", "__malloc_lock", "__malloc_unlock",
    "__malloc_update_mallinfo", "mallinfo", "_mallinfo_r",
    "__libc_lock_acquire_recursive", "__libc_lock_release_recursive", "RecursiveLock_Lock",
    "RecursiveLock_Unlock", "LightLock_Lock", "LightLock_Unlock", "__getreent",
    "__syscall_getreent", "pthread_mutex_lock", "pthread_mutex_unlock", "svcArbitrateAddressNoTimeout",
    "consolePrintChar", "newRow", "con_write", "__udivsi3", "__divsi3", "__aeabi_uidivmod",
    "__aeabi_idivmod",
    # The C and libctru file layers: a wait in them belongs to the engine
    # code that read or opened the file.
    "fread", "_fread_r", "__srefill_r", "__sread", "_read_r", "fopen", "_fopen_r", "fseek",
    "_fseek_r", "_fseeko_r", "ftell", "_ftell_r", "fclose", "_fclose_r", "archive_read",
    "archive_open", "archive_seek", "archive_close", "FSFILE_Read", "FSFILE_Write",
    "FSUSER_OpenFile", "FSFILE_Close", "FSFILE_GetSize",
}


def read_samples(path, window=None):
    """(thread, pc, lr, stack) per sample; window = (start, end) system ticks
    (low 32 bits, as the game's "pica: slow frame" line prints them) keeps
    the samples taken inside it, modulo 2^32 (the low bits wrap every 16 s)."""
    data = Path(path).read_bytes()
    size = RECORD_WORDS * 4
    span = (window[1] - window[0]) & 0xFFFFFFFF if window else 0
    for offset in range(0, len(data) - size + 1, size):
        words = struct.unpack_from("<%dI" % RECORD_WORDS, data, offset)
        if window and ((words[3] - window[0]) & 0xFFFFFFFF) > span:
            continue
        yield words[0], words[1], words[2], words[4:]


def text_reader(elf_path):
    """word(address) -> the ELF's code word at a virtual address, or None."""
    data = Path(elf_path).read_bytes()
    shoff, = struct.unpack_from("<I", data, 0x20)
    shentsize, shnum = struct.unpack_from("<HH", data, 0x2E)
    sections = []
    for i in range(shnum):
        _, kind, flags, addr, offset, size = struct.unpack_from("<IIIIII", data, shoff + i * shentsize)
        if kind == 1 and flags & 0x4:  # PROGBITS, executable
            sections.append((addr, offset, size))

    def word(address):
        for addr, offset, size in sections:
            if addr <= address < addr + size - 3:
                return struct.unpack_from("<I", data, offset + address - addr)[0]
        return None
    return word


def is_return_address(word, address):
    """The ARM instruction before `address` is a call: BL, BLX imm or BLX reg."""
    if address & 3 or not TEXT_START <= address < TEXT_END:
        return False
    insn = word(address - 4)
    if insn is None:
        return False
    return ((insn & 0x0F000000) == 0x0B000000 and insn >> 28 != 0xF) \
        or (insn & 0xFE000000) == 0xFA000000 or (insn & 0x0FFFFFF0) == 0x012FFF30


def symbolize(addresses, elf):
    addresses = sorted({a for a in addresses if TEXT_START <= a < TEXT_END})
    names = {}
    for start in range(0, len(addresses), 20000):
        chunk = addresses[start:start + 20000]
        out = subprocess.run([str(ADDR2LINE), "-f", "-C", "-e", str(elf)] + ["%x" % a for a in chunk],
                             capture_output=True, text=True).stdout.splitlines()
        for i, address in enumerate(chunk):
            function = out[2 * i] if 2 * i < len(out) else "?"
            names[address] = function if function != "??" else "?"
    return names


def report_lines(samples, function, names, elf, total, top, out):
    """Where inside the functions matching `function` the samples fall: the
    source line of each sampled pc (addr2line, with inlined frames)."""
    pcs = collections.Counter(pc for thread, pc, _, _ in samples if thread and function in names.get(pc, ""))
    if not pcs:
        out("-- lines of %s: no samples" % function)
        return
    ordered = sorted(pcs)
    lines = subprocess.run([str(ADDR2LINE), "-i", "-e", str(elf), "-a"] + ["%x" % a for a in ordered],
                           capture_output=True, text=True).stdout.splitlines()
    # "-a" prints each address, then its line and the lines it is inlined into.
    where, current = {}, None
    for line in lines:
        if line.startswith("0x"):
            current = int(line, 16)
            where[current] = []
        elif current is not None:
            where[current].append(line.split("/")[-1].split(" (")[0])
    by_line = collections.Counter()
    for address, hits in pcs.items():
        chain = where.get(address) or ["?"]
        by_line[" <- ".join(chain[:3])] += hits
    out("-- lines of %s (%.1f%% of core-0 time)" % (function, 100.0 * sum(pcs.values()) / total))
    if all(line.startswith("?") or ":?" in line for line in by_line):
        # No line tables (the build has no -g): the hottest instructions,
        # each with the two before it, from the ELF.
        objdump = ADDR2LINE.with_name("arm-none-eabi-objdump")
        for address, hits in pcs.most_common(min(top, 8)):
            text = subprocess.run([str(objdump), "-d", "--no-show-raw-insn",
                                   "--start-address=0x%x" % (address - 8), "--stop-address=0x%x" % (address + 4),
                                   str(elf)], capture_output=True, text=True).stdout.splitlines()
            body = [l.strip() for l in text if ":\t" in l]
            out("  %5.1f%%  %x  %s" % (100.0 * hits / total, address, " | ".join(body)[:160]))
        return
    for line, hits in by_line.most_common(top):
        out("  %5.1f%%  %s" % (100.0 * hits / total, line[:150]))


def report(path, elf=None, top=30, callers_of=None, out=print, lines_of=None, window=None):
    elf = Path(elf or n3ds_tree.ELF)
    samples = list(read_samples(path, window))
    if not samples:
        out("-- guest profile: no samples in %s" % path)
        return
    total = len(samples)
    WAITING = 0x80000000
    busy = [s for s in samples if s[0] and not s[0] & WAITING]
    waiting = [s for s in samples if s[0] & WAITING]
    idle = total - len(busy)
    word = text_reader(elf)
    addresses = set()
    for _, pc, lr, stack in busy + waiting:
        addresses.add(pc)
        addresses.add(lr)
        addresses.update(stack)
    calls = {a for a in addresses if is_return_address(word, a)}
    names = symbolize({pc for _, pc, _, _ in busy + waiting} | calls, elf)

    own, inclusive, threads = collections.Counter(), collections.Counter(), collections.Counter()
    callers = collections.defaultdict(collections.Counter)
    for thread, pc, lr, stack in busy:
        threads[thread] += 1
        self_name = names.get(pc, "?")
        own[self_name] += 1
        chain = [self_name] + [names[a] for a in [lr] + list(stack) if a in calls and a in names]
        inclusive.update(set(chain))
        caller = next((f for f in chain[1:] if f not in LIBRARY and f != self_name), "?")
        callers[self_name][caller] += 1

    share = lambda hits: 100.0 * hits / total
    out("-- guest profile: %d samples of core 0 (emulated time); idle %.1f%%" % (total, share(idle)))
    for thread, hits in threads.most_common(6):
        out("  thread %d: %.1f%%" % (thread, share(hits)))
    out("-- self (and its nearest caller outside the C library)")
    for name, hits in own.most_common(top):
        caller, caller_hits = callers[name].most_common(1)[0]
        out("  %5.1f%%  %s%s" % (share(hits), name[:90],
                                 "  <- %s (%.0f%%)" % (caller[:60], 100.0 * caller_hits / hits)
                                 if name in LIBRARY else ""))
    out("-- inclusive")
    for name, hits in inclusive.most_common(top):
        out("  %5.1f%%  %s" % (share(hits), name[:120]))
    if waiting:
        # Core 0 idle while the main thread is blocked: what it waits in,
        # named by its nearest engine caller (the libctru/syscall frames are
        # the same for every wait).
        blocked = collections.Counter()
        blocked_chain = collections.Counter()
        for thread, pc, lr, stack in waiting:
            chain = [names.get(pc, "?")] + [names[a] for a in [lr] + list(stack) if a in calls and a in names]
            # The engine's file system is plumbing too: name who reads.
            engine = [f for f in chain if f not in LIBRARY and not f.startswith(
                ("svc", "_", "?", "CFileSystem_Stdio::", "CFileHandle::", "CBaseFileSystem::",
                 "non-virtual thunk to CBaseFileSystem", "CPackedStore", "CStdioFile::",
                 "CFileSystem_Stdio", "CBaseFileSystem"))]
            blocked[engine[0] if engine else chain[0]] += 1
            blocked_chain[" <- ".join(engine[:3]) if engine else chain[0]] += 1
        out("-- waiting: core 0 idle while the main thread is blocked (%.1f%% of core-0 time)"
            % share(len(waiting)))
        for name, hits in blocked_chain.most_common(min(top, 15)):
            out("  %5.1f%%  %s" % (share(hits), name[:150]))
    if lines_of:
        report_lines(samples, lines_of, names, elf, total, top, out)
    if callers_of:
        out("-- callers of %s" % callers_of)
        merged = collections.Counter()
        for name, counter in callers.items():
            if callers_of in name:
                merged.update(counter)
        for name, hits in merged.most_common(15):
            out("  %5.1f%%  %s" % (share(hits), name[:120]))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("profile", nargs="?", type=Path)
    parser.add_argument("--elf", type=Path)
    parser.add_argument("--top", type=int, default=30)
    parser.add_argument("--callers", help="list the nearest non-library callers of functions matching this")
    parser.add_argument("--lines", help="where inside the functions matching this the time goes (source lines)")
    parser.add_argument("--window", nargs=2, metavar=("START", "END"),
                        help="only samples between two system ticks (hex, from a 'pica: slow frame' line)")
    options = parser.parse_args()
    import azahar_ns
    report(options.profile or azahar_ns.HOME / "guest_profile.bin", options.elf, options.top, options.callers,
           lines_of=options.lines,
           window=tuple(int(t, 16) for t in options.window) if options.window else None)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
