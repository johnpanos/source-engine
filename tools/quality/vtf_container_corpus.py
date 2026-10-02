#!/usr/bin/env python3
"""Pipe external VPK textures into the shared VTF/native-adapter conformance test.

Reads installed assets in place. Writes only logs and metadata under --out.
The VPK reader is the existing source_content reader; no texture decoder lives here.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import sys

from conformance_result import Checks
from source_content import VpkDirectory


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--vpk', type=Path, action='append', required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    counts = Checks()
    command = [str(args.binary.resolve()), '--corpus']
    records, total = [], 0
    log = args.out / 'native.log'
    with log.open('wb') as output:
        process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=output,
                                   stderr=subprocess.STDOUT)
        try:
            for path in args.vpk:
                archive = VpkDirectory(str(path.resolve()))
                names = sorted(name for name in archive.entries if name.endswith('.vtf'))
                counts.check(bool(names), 'archive contains textures', str(path))
                digest = hashlib.sha256()
                for name in names:
                    data = archive.read(name)
                    label = (str(path.resolve()) + ':' + name).encode()
                    digest.update(name.encode() + b'\0' + data)
                    process.stdin.write(struct.pack('<II', len(label), len(data)))
                    process.stdin.write(label)
                    process.stdin.write(data)
                    total += 1
                records.append({'archive': str(path.resolve()), 'textures': len(names),
                                'texture_digest_sha256': digest.hexdigest()})
            process.stdin.close()
            code = process.wait(timeout=120)
        except Exception:
            process.kill()
            process.wait()
            raise
    text = log.read_text(errors='replace')
    result = re.findall(r'^CONFORMANCE (\d+) (\d+)$', text, re.M)
    observed = re.findall(r'^VTF_CORPUS (\d+)$', text, re.M)
    counts.check(code == 0, 'native process completed', str(log))
    counts.check(len(result) == 1 and int(result[0][0]) > 0 and result[0][1] == '0',
                 'nonempty native checks pass', str(log))
    counts.equal(observed, [str(total)], 'every texture processed')
    (args.out / 'result.json').write_text(json.dumps({
        'schema': 'vtf-container-corpus/v1', 'command': command,
        'binary_sha256': hashlib.sha256(args.binary.read_bytes()).hexdigest(),
        'archives': records, 'textures': total, 'native_result': result,
        'exit_code': code, 'log': str(log), 'failures': counts.failures,
    }, indent=2) + '\n')
    print(f'{total} external VTFs; evidence: {args.out / "result.json"}')
    return counts.report()


if __name__ == '__main__':
    sys.exit(main())
