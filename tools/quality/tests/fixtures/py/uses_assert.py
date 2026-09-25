#!/usr/bin/env python3
# Command-suite fixture: an assert statement vanishes under python -O.
import sys
sys.path.insert(0, __file__.rsplit("/", 4)[0])
assert 1 + 1 == 2
print("CONFORMANCE 1 0")
