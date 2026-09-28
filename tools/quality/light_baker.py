#!/usr/bin/env python3
"""The map lighting back end's baker seam (RFC 0007 R48-BAKER).

Every map is lit by one back end, `pbrt_map_build.Pipeline` (a compiled BSP
plus an authored or derived scene; see its docstring). The back end asks one
baker for each light-transport product the map carries, and nothing else in
the tree runs a map-lighting bake:

    bake          the lightmap atlas: direct and indirect layers, the
                  directional page, the noise halves (LMAP)
    probe         reflection-probe faces with depth and relight bands (RPRB)
    probe-volume  the irradiance and visibility probe volume (PRBV)
    radiosity     the radiosity transfer of the switchable sources (RTRN)
    sdf           the distance volume and its light cells (SDFV)

Its callers are the back end and oracle harnesses that exercise a baker
directly (`gi_probes.py bake`, as R48-BAKER's shared suite will).

`CyclesBaker` is today's provider: the pinned Blender runs one script per
operation (`Pipeline.blender`), on this host or on another host's GPU
(`remote_blender.py`). R48-BAKER's `ILightBaker` contract, its legacy vrad
provider and the shared suite with bad providers replace the provider table
below; the back end's steps and cache keys name operations, not scripts.
"""

# operation -> (Blender script, the scripts it reads besides the scene reader)
OPERATIONS = {
    "bake": ("pbrt_lightmap_bake.py", ["pbrt_blender.py"]),
    "probe": ("pbrt_reflection_probe.py",
              ["reflection_probe_set.py", "reflection_probe.py", "pbrt_blender.py"]),
    "probe-volume": ("probe_volume_bake.py", ["probe_volume.py", "pbrt_blender.py"]),
    "radiosity": ("radiosity_transfer_bake.py",
                  ["radiosity_transfer.py", "probe_volume.py", "pbrt_blender.py"]),
    "sdf": ("sdf_volume_bake.py",
            ["sdf_volume.py", "sdf_light_cells.py", "legacy_bsp.py",
             "radiosity_transfer_bake.py", "pbrt_blender.py"]),
}


class CyclesBaker:
    """Cycles in the pinned Blender: one subprocess per operation, run by
    `run_blender(operation, script, arguments)` (the back end's executor,
    which also sends it to a remote GPU host when one is configured)."""

    name = "cycles"

    def __init__(self, run_blender):
        self.run_blender = run_blender

    @staticmethod
    def scripts(operation):
        """The scripts an operation runs, for the step's cache key."""
        script, reads = OPERATIONS[operation]
        return [script] + reads

    def bake(self, operation, arguments):
        """Run one operation; returns its seconds."""
        return self.run_blender(operation, OPERATIONS[operation][0], arguments)
