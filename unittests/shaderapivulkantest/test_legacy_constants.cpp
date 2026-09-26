//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The legacy shader ports' constants block and program registry
//          (materialsystem/shaderapivulkan/shaderapivulkan_legacy.h,
//          vulkan_legacy_programs.h), without a device.
//
//          The matrix registers are checked by their contract, the way the
//          compiled vertex shaders use them: mul( float4( p, 1 ), cViewProj ) is
//          dot( p, c8 + i ) per component, so for sample points the dot products
//          with the derived registers must equal the row-vector products of the
//          transforms, computed here independently. Deliberately wrong blocks
//          (rows for columns, the fast-clip projection in the z rows, a dropped
//          high register) must be rejected by the same checks.
//
//=============================================================================//

#include "materialsystem/shaderapivulkan/shaderapivulkan_legacy.h"
#include "materialsystem/shaderapivulkan/vulkan_legacy_programs.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace
{

using render_vulkan::LegacyConstants;

unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool ok, const char *what )
{
	++g_checks;
	if ( !ok )
	{
		++g_failures;
		std::printf( "FAIL: %s\n", what );
	}
}

// p * M for a row vector p (the shader API's storage: element [row * 4 + col]).
void RowTransform( const float p[4], const float *m, float out[4] )
{
	for ( int col = 0; col < 4; ++col )
		out[col] = p[0] * m[col] + p[1] * m[4 + col] + p[2] * m[8 + col] + p[3] * m[12 + col];
}

void Compose( const float *a, const float *b, float *out )
{
	for ( int row = 0; row < 4; ++row )
	{
		const float p[4] = { a[row * 4], a[row * 4 + 1], a[row * 4 + 2], a[row * 4 + 3] };
		RowTransform( p, b, out + row * 4 );
	}
}

float Dot( const float a[4], const float b[4] )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

bool Near( float a, float b )
{
	return std::fabs( a - b ) <= 1e-4f * ( 1.0f + std::fabs( a ) + std::fabs( b ) );
}

struct Transforms
{
	float model[16];
	float view[16];
	float projection[16];
	float drawProjection[16];
};

Transforms MakeTransforms()
{
	Transforms t;
	// Model: a rotation about z, a scale and a translation.
	const float c = std::cos( 0.7f ), s = std::sin( 0.7f );
	const float model[16] = { 2 * c, 2 * s, 0, 0, -2 * s, 2 * c, 0, 0, 0, 0, 2, 0, 10, -4, 3, 1 };
	// View: a rotation about x and a translation.
	const float cx = std::cos( -0.4f ), sx = std::sin( -0.4f );
	const float view[16] = { 1, 0, 0, 0, 0, cx, sx, 0, 0, -sx, cx, 0, -5, 2, -30, 1 };
	// A D3D-style perspective (row vectors, z in 0..1) and an oblique copy
	// standing in for the fast-clip projection.
	const float n = 4.0f, f = 4000.0f;
	const float projection[16] = {
	    1.2f, 0, 0, 0, 0, 1.6f, 0, 0, 0, 0, f / ( f - n ), 1, 0, 0, -n * f / ( f - n ), 0 };
	std::memcpy( t.model, model, sizeof( model ) );
	std::memcpy( t.view, view, sizeof( view ) );
	std::memcpy( t.projection, projection, sizeof( projection ) );
	std::memcpy( t.drawProjection, projection, sizeof( projection ) );
	t.drawProjection[2] += 0.2f;
	t.drawProjection[6] -= 0.1f;
	t.drawProjection[14] *= 0.5f;
	return t;
}

LegacyConstants Build( const Transforms &t, float ( *vs )[4], float ( *ps )[4] )
{
	render_vulkan::LegacyPassInputs in;
	in.ps = ps;
	in.vs = vs;
	in.model = t.model;
	in.view = t.view;
	in.projection = t.projection;
	in.drawProjection = t.drawProjection;
	LegacyConstants c;
	render_vulkan::BuildLegacyConstants( in, &c );
	return c;
}

// The matrix register contract; returns the number of violations (and reports
// them when `report`).
int CheckMatrixRegisters( const LegacyConstants &c, const Transforms &t, bool report )
{
	float modelView[16], mvp[16], vp[16], mvpZ[16], vpZ[16];
	Compose( t.model, t.view, modelView );
	Compose( modelView, t.drawProjection, mvp );
	Compose( t.view, t.drawProjection, vp );
	Compose( modelView, t.projection, mvpZ );
	Compose( t.view, t.projection, vpZ );
	const float points[3][4] = { { 1, 2, 3, 1 }, { -7, 0.5f, 11, 1 }, { 0.25f, -3, -2, 1 } };
	int violations = 0;
	const auto expect = [&]( bool ok, const char *what )
	{
		if ( !ok )
		{
			++violations;
			if ( report )
				std::printf( "  violation: %s\n", what );
		}
	};
	for ( const float *p : points )
	{
		float want[4];
		RowTransform( p, mvp, want );
		for ( int i = 0; i < 4; ++i )
			expect( Near( Dot( p, c.vs[4 + i] ), want[i] ), "c4..c7 are mul( p, cModelViewProj )" );
		RowTransform( p, vp, want );
		for ( int i = 0; i < 4; ++i )
			expect( Near( Dot( p, c.vs[8 + i] ), want[i] ), "c8..c11 are mul( p, cViewProj )" );
		RowTransform( p, mvpZ, want );
		expect( Near( Dot( p, c.vs[12] ), want[2] ),
		    "c12 is the model-view-projection z without fast clip" );
		RowTransform( p, vpZ, want );
		expect(
		    Near( Dot( p, c.vs[13] ), want[2] ), "c13 is the view-projection z without fast clip" );
		RowTransform( p, t.model, want );
		for ( int i = 0; i < 3; ++i )
			expect( Near( Dot( p, c.vs[58 + i] ), want[i] ), "c58..c60 are mul( p, cModel[0] )" );
	}
	return violations;
}

void TestMatrixRegisters()
{
	static float vs[256][4], ps[32][4];
	const Transforms t = MakeTransforms();
	const LegacyConstants c = Build( t, vs, ps );
	Check(
	    CheckMatrixRegisters( c, t, true ) == 0, "derived matrix registers follow the contract" );

	// Negative controls: each corruption must be caught by the same checks.
	LegacyConstants rows = c;
	for ( int i = 0; i < 4; ++i )
		for ( int k = 0; k < 4; ++k )
			rows.vs[8 + i][k] = c.vs[8 + k][i];
	Check( CheckMatrixRegisters( rows, t, false ) > 0, "rows stored for columns are rejected" );
	LegacyConstants fastClipZ = c;
	std::memcpy( fastClipZ.vs[13], c.vs[10], sizeof( fastClipZ.vs[13] ) );
	Check( CheckMatrixRegisters( fastClipZ, t, false ) > 0,
	    "the fast-clip projection in cViewProjZ is rejected" );
	LegacyConstants noModel = c;
	std::memset( noModel.vs[58], 0, sizeof( noModel.vs[58] ) );
	Check( CheckMatrixRegisters( noModel, t, false ) > 0, "a missing cModel row is rejected" );
}

void TestRegisterCopies()
{
	static float vs[256][4], ps[32][4];
	for ( int r = 0; r < 256; ++r )
		for ( int k = 0; k < 4; ++k )
			vs[r][k] = static_cast<float>( r * 4 + k );
	for ( int r = 0; r < 32; ++r )
		for ( int k = 0; k < 4; ++k )
			ps[r][k] = -static_cast<float>( r * 4 + k );
	const LegacyConstants c = Build( MakeTransforms(), vs, ps );
	Check( std::memcmp( c.ps, ps, sizeof( c.ps ) ) == 0, "pixel shader c0..c31 are copied" );
	bool low = true;
	for ( int r = 0; r < render_vulkan::kLegacyVsLowRegisters; ++r )
	{
		const bool derived = ( r >= 4 && r <= 13 ) || ( r >= 58 && r <= 60 );
		if ( !derived && std::memcmp( c.vs[r], vs[r], sizeof( vs[r] ) ) != 0 )
			low = false;
	}
	Check( low, "vertex shader c0..c63 are copied (the derived registers aside)" );
	bool high = true;
	for ( int r = 0; r < render_vulkan::kLegacyVsHighRegisters; ++r )
	{
		if ( std::memcmp( c.vs[render_vulkan::kLegacyVsLowRegisters + r],
		         vs[render_vulkan::kLegacyVsHighFirst + r], sizeof( vs[0] ) ) != 0 )
			high = false;
	}
	Check( high, "vertex shader c217..c224 follow c63" );
}

void TestCombosAndBools()
{
	static float vs[256][4], ps[32][4];
	const Transforms t = MakeTransforms();
	render_vulkan::LegacyPassInputs in;
	in.ps = ps;
	in.vs = vs;
	in.model = t.model;
	in.view = t.view;
	in.projection = t.projection;
	in.drawProjection = t.drawProjection;
	in.psStatic = 96;
	in.psDynamic = 5;
	in.vsStatic = 8;
	in.vsDynamic = 3;
	const int psBools[16] = { 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
	const int vsBools[16] = { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	in.psBools = psBools;
	in.vsBools = vsBools;
	in.srgbSamplers = 0x10001u | 0x80000u; // samplers 0 and 16..19: only 0..15 exist
	LegacyConstants c;
	render_vulkan::BuildLegacyConstants( in, &c );
	Check( c.combos[0] == 96 && c.combos[1] == 5 && c.combos[2] == 8 && c.combos[3] == 3,
	    "combo indices are the pass's static and dynamic indices" );
	Check( c.bools[0] == ( 1 | 4 | 0x8000 ), "pixel shader booleans are packed b0..b15" );
	Check( c.bools[1] == ( 2 | 4 | 8 ), "vertex shader booleans are packed b0..b15" );
	Check( c.bools[2] == 1, "the sRGB sampler mask keeps samplers 0..15 only" );
}

void TestRegistry()
{
	using namespace render_vulkan;
	Check( LegacyProgramCount() > 0, "the registry holds ports" );
	const int modulate = FindLegacyProgram( "modulate_ps20b", "unlitgeneric_vs20" );
	Check( modulate >= 0, "modulate_ps20b/unlitgeneric_vs20 is registered" );
	Check( FindLegacyProgram( "MODULATE_PS20B", "UnlitGeneric_VS20" ) == modulate,
	    "shader names match case-insensitively (the shader DLL's spelling)" );
	Check( FindLegacyProgram( "modulate_ps20b", "vertexlit_and_unlit_generic_vs20" ) < 0,
	    "a pixel shader with another vertex shader is not a port" );
	Check( FindLegacyProgram( "modulate_ps20", "unlitgeneric_vs20" ) < 0,
	    "a name prefix is not a match" );
	Check( FindLegacyProgram( nullptr, "unlitgeneric_vs20" ) < 0 &&
	           FindLegacyProgram( "modulate_ps20b", nullptr ) < 0,
	    "a pass without both shaders has no port" );
	bool samplersValid = true, stagesPresent = true, pairsUnique = true;
	for ( int i = 0; i < LegacyProgramCount(); ++i )
	{
		const LegacyProgram &p = GetLegacyProgram( i );
		stagesPresent = stagesPresent && p.vertSpv && p.vertBytes > 20 && p.fragSpv &&
		                p.fragBytes > 20 && p.vertSpv[0] == 0x07230203u &&
		                p.fragSpv[0] == 0x07230203u;
		for ( const LegacySamplerSlot &slot : p.samplers )
			samplersValid = samplersValid && slot.sampler >= -1 && slot.sampler < 16 &&
			                slot.dim <= kLegacySamplerVolume;
		pairsUnique = pairsUnique && FindLegacyProgram( p.pixelShader, p.vertexShader ) == i;
	}
	Check( stagesPresent, "every port has SPIR-V stages" );
	Check( samplersValid, "every port's samplers name D3D9 samplers 0..15" );
	Check( pairsUnique, "every shader pair names one port" );
	const LegacyProgram &port = GetLegacyProgram( modulate );
	Check( LegacyProgramReadsSampler( port, 0 ) && !LegacyProgramReadsSampler( port, 1 ),
	    "a port reads exactly its registered samplers" );
}

// D3D9's commit (LegacyTransformCommit): a material's write to a derived
// register after the transforms were committed survives until they change.
void TestTransformCommit()
{
	static float vs[256][4], ps[32][4];
	static uint64_t serial[256];
	Transforms t = MakeTransforms();
	uint64_t lastSerial = 0;
	uint64_t viewGeneration = 0;
	render_vulkan::LegacyTransformCommit commit;
	const auto build = [&]( const Transforms &at )
	{
		render_vulkan::LegacyPassInputs in;
		in.ps = ps;
		in.vs = vs;
		in.model = at.model;
		in.view = at.view;
		in.projection = at.projection;
		in.drawProjection = at.drawProjection;
		in.vsWriteSerial = serial;
		in.writeSerial = lastSerial;
		in.viewGeneration = viewGeneration;
		LegacyConstants c;
		render_vulkan::BuildLegacyConstants( in, &c, &commit );
		return c;
	};
	const auto write = [&]( int reg, float value )
	{
		++lastSerial;
		for ( int i = 0; i < 4; ++i )
			vs[reg][i] = value + static_cast<float>( i );
		serial[reg] = lastSerial;
	};

	// A write before the first commit is overwritten by it.
	write( 4, 100.0f );
	LegacyConstants c = build( t );
	Check( CheckMatrixRegisters( c, t, true ) == 0, "the first commit derives every group" );

	// Compositor's case: the material writes c4..c5 after the commit and the
	// transforms stay: its values reach the port, the other groups stay derived.
	write( 4, 200.0f );
	write( 5, 300.0f );
	write( 58, 400.0f );
	c = build( t );
	Check( c.vs[4][0] == 200.0f && c.vs[4][3] == 203.0f && c.vs[5][0] == 300.0f,
	    "a write after the commit keeps the material's value" );
	Check( c.vs[58][0] == 400.0f, "a write over cModel[0] after the commit is kept" );
	const LegacyConstants derived = Build( t, vs, ps );
	Check( std::memcmp( c.vs[6], derived.vs[6], sizeof( float[4] ) * 6 ) == 0 &&
	           std::memcmp( c.vs[59], derived.vs[59], sizeof( float[4] ) * 2 ) == 0,
	    "registers the material did not write stay derived" );

	// The model changes: cModelViewProj and cModel are derived again over the
	// writes; cViewProj (not written) stays derived.
	t.model[12] += 5.0f;
	c = build( t );
	Check( CheckMatrixRegisters( c, t, true ) == 0,
	    "a model change derives cModelViewProj and cModel again" );

	// A write to cViewProj survives a model change but not a view change.
	write( 8, 500.0f );
	t.model[13] -= 1.0f;
	c = build( t );
	Check( c.vs[8][0] == 500.0f, "a model change leaves a write over cViewProj" );
	Check( CheckMatrixRegisters( c, t, false ) > 0, "(the kept cViewProj write is detected)" );
	t.view[12] += 1.0f;
	c = build( t );
	Check( CheckMatrixRegisters( c, t, true ) == 0, "a view change derives cViewProj again" );

	// The view loaded again with the same value still counts as a change (D3D9's
	// flags), so a write to cViewProj is derived over.
	write( 9, 700.0f );
	c = build( t );
	Check( c.vs[9][0] == 700.0f, "unchanged transforms keep a write over cViewProj" );
	++viewGeneration;
	c = build( t );
	Check( CheckMatrixRegisters( c, t, true ) == 0,
	    "a view loaded again with the same value derives cViewProj again" );

	// Without a commit every draw derives all groups (the unit oracle's mode).
	write( 4, 600.0f );
	c = Build( t, vs, ps );
	Check( CheckMatrixRegisters( c, t, true ) == 0, "without a commit every group is derived" );
}

} // namespace

int main()
{
	TestMatrixRegisters();
	TestRegisterCopies();
	TestCombosAndBools();
	TestRegistry();
	TestTransformCommit();
	std::printf( "legacy constants: %lu checks, %lu failures\n", g_checks, g_failures );
	return testing::ReportConformance( g_checks, g_failures );
}
