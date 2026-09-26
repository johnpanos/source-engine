//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The legacy shader ports' constants block (shaderapivulkan_legacy.h).
//
//===========================================================================//

#include "shaderapivulkan_legacy.h"

#include <cstring>

namespace render_vulkan
{
namespace
{

void Multiply( const float *a, const float *b, float *out )
{
	for ( int row = 0; row < 4; ++row )
	{
		for ( int col = 0; col < 4; ++col )
		{
			float sum = 0.0f;
			for ( int k = 0; k < 4; ++k )
				sum += a[row * 4 + k] * b[k * 4 + col];
			out[row * 4 + col] = sum;
		}
	}
}

// Column `col` of a row-vector matrix: the register D3D9 stores after
// D3DXMatrixTranspose, which mul( float4( p, 1 ), M ) dots with p.
void Column( const float *m, int col, float out[4] )
{
	for ( int row = 0; row < 4; ++row )
		out[row] = m[row * 4 + col];
}

bool Changed( const float *committed, const float *current )
{
	return std::memcmp( committed, current, sizeof( float[16] ) ) != 0;
}

// Registers [first, first + count) of a group derived at `derivedAt`: those the
// material wrote since keep its value.
void KeepLaterWrites(
    const LegacyPassInputs &in, uint64_t derivedAt, int first, int count, LegacyConstants *out )
{
	for ( int reg = first; reg < first + count; ++reg )
	{
		if ( in.vsWriteSerial[reg] > derivedAt )
			std::memcpy( out->vs[reg], in.vs[reg], sizeof( out->vs[reg] ) );
	}
}

} // namespace

void BuildLegacyConstants(
    const LegacyPassInputs &in, LegacyConstants *out, LegacyTransformCommit *commit )
{
	std::memset( out, 0, sizeof( *out ) );
	std::memcpy( out->ps, in.ps, sizeof( out->ps ) );
	std::memcpy( out->vs, in.vs, sizeof( float[4] ) * kLegacyVsLowRegisters );
	std::memcpy( out->vs[kLegacyVsLowRegisters], in.vs[kLegacyVsHighFirst],
	    sizeof( float[4] ) * kLegacyVsHighRegisters );

	float modelView[16], modelViewProj[16], viewProj[16], modelViewProjZ[16], viewProjZ[16];
	Multiply( in.model, in.view, modelView );
	Multiply( modelView, in.drawProjection, modelViewProj );
	Multiply( in.view, in.drawProjection, viewProj );
	Multiply( modelView, in.projection, modelViewProjZ );
	Multiply( in.view, in.projection, viewProjZ );
	for ( int i = 0; i < 4; ++i )
	{
		Column( modelViewProj, i, out->vs[4 + i] );
		Column( viewProj, i, out->vs[8 + i] );
	}
	Column( modelViewProjZ, 2, out->vs[12] );
	Column( viewProjZ, 2, out->vs[13] );
	for ( int i = 0; i < 3; ++i )
		Column( in.model, i, out->vs[58 + i] );
	if ( commit && in.vsWriteSerial )
	{
		const bool first = !commit->valid;
		const bool model = first || commit->modelGeneration != in.modelGeneration ||
		                   Changed( commit->model, in.model );
		const bool view = first || commit->viewGeneration != in.viewGeneration ||
		                  Changed( commit->view, in.view );
		const bool projection = first || commit->projectionGeneration != in.projectionGeneration ||
		                        Changed( commit->projection, in.projection ) ||
		                        Changed( commit->drawProjection, in.drawProjection );
		if ( view || projection )
			commit->viewProjAt = in.writeSerial;
		if ( model || view || projection )
			commit->modelViewProjAt = in.writeSerial;
		if ( model )
			commit->modelAt = in.writeSerial;
		std::memcpy( commit->model, in.model, sizeof( commit->model ) );
		std::memcpy( commit->view, in.view, sizeof( commit->view ) );
		std::memcpy( commit->projection, in.projection, sizeof( commit->projection ) );
		std::memcpy( commit->drawProjection, in.drawProjection, sizeof( commit->drawProjection ) );
		commit->modelGeneration = in.modelGeneration;
		commit->viewGeneration = in.viewGeneration;
		commit->projectionGeneration = in.projectionGeneration;
		commit->valid = true;
		KeepLaterWrites( in, commit->modelViewProjAt, 4, 4, out );
		KeepLaterWrites( in, commit->modelViewProjAt, 12, 1, out );
		KeepLaterWrites( in, commit->viewProjAt, 8, 4, out );
		KeepLaterWrites( in, commit->viewProjAt, 13, 1, out );
		KeepLaterWrites( in, commit->modelAt, 58, 3, out );
	}

	out->combos[0] = in.psStatic;
	out->combos[1] = in.psDynamic;
	out->combos[2] = in.vsStatic;
	out->combos[3] = in.vsDynamic;
	for ( int i = 0; i < 16; ++i )
	{
		if ( in.psBools && in.psBools[i] )
			out->bools[0] |= 1 << i;
		if ( in.vsBools && in.vsBools[i] )
			out->bools[1] |= 1 << i;
	}
	out->bools[2] = static_cast<int>( in.srgbSamplers & 0xFFFFu );
}

void WriteLegacyCapture( FILE *out, const LegacyCaptureRecord &r )
{
	const auto writeRegisters = [out]( const char *name, const float ( *regs )[4], int count )
	{
		fprintf( out, ",\"%s\":[", name );
		for ( int i = 0; i < count; ++i )
			fprintf( out, "%s[%.9g,%.9g,%.9g,%.9g]", i ? "," : "", regs[i][0], regs[i][1],
			    regs[i][2], regs[i][3] );
		fprintf( out, "]" );
	};
	const LegacyConstants &c = *r.constants;
	fprintf( out,
	    "{\"schema\":\"source-legacy-pass/v1\",\"material\":\"%s\",\"pixel_shader\":\"%s\","
	    "\"vertex_shader\":\"%s\",\"ps_static\":%d,\"ps_dynamic\":%d,\"vs_static\":%d,"
	    "\"vs_dynamic\":%d,\"ps_bools\":%d,\"vs_bools\":%d,\"srgb_samplers\":%d",
	    r.material, r.pixelShader, r.vertexShader, c.combos[0], c.combos[1], c.combos[2],
	    c.combos[3], c.bools[0], c.bools[1], c.bools[2] & 0xFFFF );
	writeRegisters( "ps_constants", c.ps, 32 );
	// The registers the port reads (c4..c13 and c58..c60 as D3D9 derives them)
	// over the shader API's file.
	float vs[256][4];
	std::memcpy( vs, r.vs, sizeof( vs ) );
	std::memcpy( vs, c.vs, sizeof( float[4] ) * kLegacyVsLowRegisters );
	writeRegisters( "vs_constants", vs, 256 );
	const auto writeInts = [out]( const char *name, const int ( *regs )[4] )
	{
		fprintf( out, ",\"%s\":[", name );
		for ( int i = 0; i < 16; ++i )
			fprintf( out, "%s[%d,%d,%d,%d]", i ? "," : "", regs ? regs[i][0] : 0,
			    regs ? regs[i][1] : 0, regs ? regs[i][2] : 0, regs ? regs[i][3] : 0 );
		fprintf( out, "]" );
	};
	// i0 is the light loop the shader API derives (LegacyConstants::vsLoop).
	int vsInts[16][4] = {};
	if ( r.vsInts )
		std::memcpy( vsInts, r.vsInts, sizeof( vsInts ) );
	std::memcpy( vsInts[0], c.vsLoop, sizeof( vsInts[0] ) );
	writeInts( "vs_ints", vsInts );
	writeInts( "ps_ints", r.psInts );
	fprintf( out, ",\"vertex_format\":%llu", r.vertexFormat );
	fprintf( out, ",\"textures\":{" );
	bool first = true;
	for ( int i = 0; i < 16; ++i )
	{
		if ( !r.textures[i] )
			continue;
		fprintf( out, "%s\"%d\":\"%s\"", first ? "" : ",", i, r.textures[i] );
		first = false;
	}
	const auto writeMatrix = [out]( const char *name, const float *m )
	{
		fprintf( out, ",\"%s\":[", name );
		for ( int i = 0; i < 16; ++i )
			fprintf( out, "%s%.9g", i ? "," : "", m ? m[i] : ( i % 5 == 0 ? 1.0f : 0.0f ) );
		fprintf( out, "]" );
	};
	fprintf( out, "}" );
	writeMatrix( "model", r.model );
	writeMatrix( "view", r.view );
	writeMatrix( "projection", r.projection );
	writeMatrix( "draw_projection", r.drawProjection );
	if ( r.geometry && r.geometry[0] )
		fprintf( out, ",%s", r.geometry );
	fprintf( out,
	    ",\"alpha_ref\":%.9g,\"alpha_greater\":%s,\"srgb_write\":%s,\"blend\":%s,"
	    "\"src_blend\":%d,\"dst_blend\":%d,\"color_write\":%s,\"alpha_write\":%s,"
	    "\"cull\":%d}\n",
	    r.alphaRef, r.alphaGreater ? "true" : "false", r.srgbWrite ? "true" : "false",
	    r.blend ? "true" : "false", r.srcBlend, r.dstBlend, r.colorWrite ? "true" : "false",
	    r.alphaWrite ? "true" : "false", r.cullMode );
	fflush( out );
}

} // namespace render_vulkan
