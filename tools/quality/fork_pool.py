"""Forked worker pools for the Blender bake scripts that cannot hang.

The bakes fan pure-Python/numpy work (BVH traces, fits) out to forked copies
of the Blender process. A forked Blender can die in its own runtime (seen on
bazzite, 2026-10-06: every worker of probe_volume_bake crashed in
pthread_once, each writing /tmp/blender.crash.txt), and multiprocessing.Pool
then waits forever for results that will never come. ProcessPoolExecutor
notices a dead worker (BrokenProcessPool); this pool then reports it and runs
the remaining work serially in the parent, so a bake finishes - correctly,
since every job's result is independent of where it ran - or fails with the
job's own error, never a silent hang.
"""

import concurrent.futures
import multiprocessing
import sys
from concurrent.futures.process import BrokenProcessPool


class ForkPool:
    """A context manager with `map(function, jobs, chunksize=1)`.

    With one worker (or after the workers broke) jobs run in the parent.
    `function` and the state it reads must be inherited by fork (module
    globals set before the pool opens), as with multiprocessing.Pool."""

    def __init__(self, workers):
        self.workers = workers
        self.executor = None
        self.broken = False

    def __enter__(self):
        if self.workers > 1:
            self.executor = concurrent.futures.ProcessPoolExecutor(
                self.workers, mp_context=multiprocessing.get_context("fork"))
        return self

    def __exit__(self, *exc):
        if self.executor is not None:
            self.executor.shutdown(wait=not self.broken, cancel_futures=True)
            self.executor = None
        return False

    def map(self, function, jobs, chunksize=1):
        jobs = list(jobs)
        if self.executor is None or self.broken or len(jobs) <= 1:
            return [function(job) for job in jobs]
        try:
            return list(self.executor.map(function, jobs, chunksize=max(1, chunksize)))
        except BrokenProcessPool as error:
            self.broken = True
            print("fork_pool: a forked worker died (%s); running %d job(s) serially"
                  % (error, len(jobs)), file=sys.stderr, flush=True)
            return [function(job) for job in jobs]
