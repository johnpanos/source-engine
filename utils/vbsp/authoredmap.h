//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Typed in-memory map input for USD-authored maps (RFC 0009 U1).
//
// The native USD map compiler (tools/quality/usd_map_compile.py) reads and
// validates the authored stage and writes one `source-authored-brushset/v1`
// JSON document: entities in output order with typed origins, angles and
// entity keys, and each convex solid as its authored face planes, polygons,
// materials and affine st. This loader builds vbsp's entities, brushes and
// sides from it directly; it never produces or parses VMF text.
//
//=============================================================================//

#ifndef VBSP_AUTHOREDMAP_H
#define VBSP_AUTHOREDMAP_H

// True when vbsp compiles an authored brush set (-authored). An authored map
// fails instead of warning when it leaks, drops a static prop, or names a
// material the game content does not have.
extern bool g_bAuthoredInput;

// Builds g_MainMap from the brush set at pszFileName. Any schema, material or
// geometry error is fatal; there is no partial map.
void LoadAuthoredMapFile( const char *pszFileName );

#endif // VBSP_AUTHOREDMAP_H
