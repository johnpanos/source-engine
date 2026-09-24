# render.dxsupport-policy.v1

`public/render/dxsupport_policy.h` owns how `dxsupport.cfg` groups combine into
a recommended configuration. The rules are the ones
`CShaderDeviceMgrBase::GetRecommendedConfigurationInfo` has always applied.
Callers describe each group (name, vendor/device range, CPU, RAM and
video-memory ranges, picmip). They then copy the returned groups' values in
order, later groups overriding earlier ones, apply the picmip floor, and force
`ConVar.mat_dxlevel`.

Each lookup takes the first matching group in file order:
- **Dx level:** the name, read as an integer, equals the level.
- **Dx level and vendor:** as above, plus the vendor ID matches.
- **Card:** the vendor ID matches and the device ID is within
  `[MinDeviceID, MaxDeviceID]`.
- **CPU:** the name contains AMD or Intel, and the speed falls in the half-open
  `[min, max)` megahertz range.
- **System RAM:** the half-open `[min, max)` megabytes range.
- **Video memory:** the half-open `[min, max)` megatexels range, compared with
  memory in MB.

Application order and picmip:
- **Vendor catch-all card group (0..0xffff):** the dx-level group, then the card
  group, then the dx-level-and-vendor group.
- **Narrower card group:** the dx-level group, then the card group last. The
  dx-level-and-vendor group is skipped.
- CPU and system-RAM groups follow.
- The video-memory group contributes only a picmip floor, and only at the
  maximum level or under 100 MB of video memory.

`ClosestActualDxLevel` maps requested levels to the levels a manager runs.

The suite checks the order for catch-all, narrow, catch-all-fallback and
unlisted cards, the half-open ranges, each picmip-floor case, and the level
mapping. Its sensitivity build swaps the catch-all order, which the oracle must
reject. Reading the file, KeyValues conversion and override merging belong to
the shader APIs' shared KeyValues adapter. Proof that D3D9 output is unchanged
is recorded with that adapter.
