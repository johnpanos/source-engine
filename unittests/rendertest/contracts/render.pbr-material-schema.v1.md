# render.pbr-material-schema.v1

RFC 0007 Phase D gives `PBRMetalRough` one versioned VMT parameter owner in
`public/render/pbr_material_schema.h`. Base and emission textures are sRGB;
MRAO and bump maps are linear. A material requires `$basetexture`,
`$mraotexture`, and `$fallbackmaterial`. The fallback is a separate VMT path,
not a shader name. Existing material families retain their current parameters.

Glass is three optional scalars on the same family: `$transmission` (0..1, the
refracted fraction, default 0), `$ior` (1..3, default 1.5) and `$thickness`
(world units through the glass, default 0 for a thin sheet). `$basetexture`
tints transmitted light. `IsValidTransmission` owns the accepted ranges; NaN is
rejected. A material with `$transmission` above 0 is translucent: the native
shader sets the flag, so it sorts and draws after the opaque scene it refracts.

The editor catalog validates that the PBR definition and fallback VMT parse,
that patch includes on both sides are bounded to the legacy loader's ten levels
and acyclic, and that the supplied compatibility
provider claims the resolved shader. Unknown provider support is a failure.
The catalog never substitutes a fallback on its own. A valid content result
does not prove the material system uses the fallback at runtime, or that either
provider renders correct pixels; those are separate R47 gates.

The suite includes missing fields, missing/malformed fallback, self and patch
cycles, path traversal, a chain beyond the legacy bound, and a provider that
rejects the fallback shader. For scalar parameters, nested patch `insert` and
`replace` follow `CMaterial::ApplyPatchKeyValues`: inner patches win conflicts;
an insert writes its value, then a replace changes an existing key.
