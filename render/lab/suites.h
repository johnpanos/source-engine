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
//
//=============================================================================//

#ifndef RENDER_LAB_SUITES_H
#define RENDER_LAB_SUITES_H

namespace render::lab
{

int RunSuite( int argc, char **argv );

int RunDebugViewsSuite( int argc, char **argv );
int RunLightingControlsSuite( int argc, char **argv );
int RunAreaLightsSuite( int argc, char **argv );

} // namespace render::lab

#endif // RENDER_LAB_SUITES_H
