//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's suites (RFC 0016 K11, RFC 0014): each draws analytic
//			fixtures through the core alone and prints one checks-v1 record.
//
//			render_lab suite <name> [--validate] [--seeded <defect>]
//			    [--sensitivity]
//
//			debug-views     RFC 0014 D0: every installed view of the catalog
//			                against its formula; the hatch; the program
//			                filter; the controls' validation; default
//			                identity. --seeded runs one seeded program
//			                (swapped-normal, tone-maps, misses-nan,
//			                no-hatch); --sensitivity runs the control and
//			                every seeded program and passes when the control
//			                passes and each seeded program fails the checks
//			                it breaks.
//			lighting-controls
//			                RFC 0014 D1: cl_render_debug_term (each term off
//			                is the frame without its input, bitwise), the
//			                BRDF modes against pbr_brdf.h, the furnace
//			                (white spheres read 1; with compensation off a
//			                rough one reads below), the forced roughness and
//			                metalness. --seeded term-ignored ignores
//			                `baked`.
//			area-lights     RFC 0016 K11: the area-light term (LTC) against
//			                the contract's exact irradiance and a GGX
//			                quadrature; neutral, one- and two-sided cases.
//			                Seeded: no-horizon-clip, ltc-transposed,
//			                no-magnitude.
//			lightmap-basis  RFC 0016 K11: the lightmap basis (flat,
//			                directional, RNM) against its oracle, the
//			                directional page split and the baked layer rule.
//			                Seeded: no-smooth-normal, no-gain-clamp,
//			                rnm-unsquared, rnm-offset-from-zero, whole-page.
//			probe-volume    RFC 0016 K11: the probe volume (probe_volume.glsl)
//			                against mapcontainer::ProbeVolumeView over the
//			                PRBV fixtures. Seeded: no-normal-bias,
//			                state-ignored, visibility-ignored, no-crush.
//			reflection-probes
//			                RFC 0016 K11: the RPRB probes
//			                (reflection_probes.glsl) against
//			                mapcontainer::ReflectionProbesView, blended,
//			                nearest, direction-only, weights and relit.
//			                Seeded: no-distance-roughness, no-facing,
//			                relight-added-only.
//			clustered-lights
//			                RFC 0016 K11: the runtime point and spot lights
//			                through the view's cluster lists against every
//			                light of the set; neutral bitwise. Seeded:
//			                slice-off-by-one, skips-first, falloff-unwindowed,
//			                spot-no-cosine.
//			shadowed-lights
//			                RFC 0016 K11: the clustered spots shadowed through
//			                the view's atlas (render.shadows.v1) against a ray
//			                test to box casters. Seeded: shadow-ignored,
//			                tile-next, depth-reversed.
//			selfillum       RFC 0016 surface model, emission term:
//			                VertexLitGeneric's $selfillum region, tint and
//			                $selfillumfresnel through render.pass.world
//			                against the analytic emission; neutral bitwise.
//			                Seeded: fresnel-ignored, brightness-ignored.
//
//=============================================================================//

#ifndef RENDER_LAB_SUITES_H
#define RENDER_LAB_SUITES_H

namespace render::lab
{
int RunTreeSwaySuite( int argc, char **argv );

int RunTemporalSuite( int argc, char **argv );

int RunSuite( int argc, char **argv );
int RunPosedModelSuite( int argc, char **argv );
int RunSelfIllumSuite( int argc, char **argv );
int RunViewStateSuite( int argc, char **argv );
int RunModelSelectionSuite( int argc, char **argv );

int RunCostOverlaySuite( int argc, char **argv );
int RunDebugViewsSuite( int argc, char **argv );
int RunLightingControlsSuite( int argc, char **argv );
int RunEnergyFieldSuite( int argc, char **argv );
int RunAreaLightsSuite( int argc, char **argv );
int RunLightmapBasisSuite( int argc, char **argv );
int RunProbeVolumeSuite( int argc, char **argv );
int RunReflectionProbesSuite( int argc, char **argv );
int RunReflectionCandidatesSuite( int argc, char **argv );
int RunVolumetricSuite( int argc, char **argv );
int RunClusteredLightsSuite( int argc, char **argv );
int RunMapTermsSuite( int argc, char **argv );
int RunSsrSuite( int argc, char **argv );
int RunShadowedLightsSuite( int argc, char **argv );
int RunShadowReceiverPerfSuite( int argc, char **argv );
int RunBounceSuite( int argc, char **argv );
int RunGtaoSuite( int argc, char **argv );
int RunPanelSuite( int argc, char **argv );

} // namespace render::lab

#endif // RENDER_LAB_SUITES_H
