//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Backend-neutral draw-state fixtures (schema source-draw-state/v1).
//
//          Every shader API backend that claims to render Source materials
//          records, per material render pass, the state it actually applied:
//          the textures it samples, their addressing and filtering, blend,
//          depth and alpha-test state, the render target and viewport, and the
//          material registers of the vertex shader. Written by the D3D9 backend
//          (the reference) and the native Vulkan backend alike, two captures of
//          the same frame can be compared field by field.
//
//          Unlike the D3D9 provider trace (shaderapidx9/render_diagnostics.h),
//          nothing here is backend-specific: textures are named, factors are
//          spelled out, and values are normalized, so a difference in a fixture
//          is a difference in behavior rather than in representation.
//
//          Enabled with "-drawstatefixture <directory>". Each screenshot writes
//          the draws of the frame being captured to
//          <directory>/<backend>-<n>.jsonl: one header line, then one line per
//          render pass in submission order.
//
//===========================================================================//

#ifndef DRAWSTATEFIXTURE_H
#define DRAWSTATEFIXTURE_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/icommandline.h"

#include <cstdio>
#include <string>
#include <vector>

namespace drawstatefixture
{

enum
{
	kMaxSamplers = 16,
	// A frame larger than this is truncated, and the header says so.
	kMaxDrawsPerFrame = 65536
};

struct Sampler
{
	int stage = 0;
	const char *texture = "";
	const char *addressU = ""; // "wrap", "clamp", "border", "mirror"
	const char *addressV = "";
	const char *filter = ""; // minification filter: "point", "linear", "anisotropic"
};

struct Draw
{
	const char *material = "";
	const char *shader = "";
	int pass = 0;
	int passCount = 0;
	const char *target = "backbuffer"; // render-target texture name
	int viewport[4] = { 0, 0, 0, 0 };  // x, y, width, height
	float depthRange[2] = { 0.0f, 1.0f };
	// Only the samplers the backend's shader for this draw actually samples.
	Sampler samplers[kMaxSamplers];
	int samplerCount = 0;
	bool blend = false;
	const char *srcBlend = "one"; // "zero", "one", "src_alpha", "one_minus_src_alpha", ...
	const char *dstBlend = "zero";
	bool depthTest = true;
	bool depthWrite = true;
	float alphaTestRef = -1.0f; // 0..1; negative when alpha test is off
	// cModulationColor (c47) and cBaseTextureTransform (c48-c49), the material
	// registers every textured Source vertex shader shares.
	float modulation[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float baseTextureTransform[8] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
	// Whether the draw reached the device, and why not when it did not.
	bool submitted = true;
	const char *dropReason = "";
};

class Writer
{
public:
	// Idempotent. Reads the command line once; `backend` names the files.
	void Configure( const char *backend )
	{
		if ( m_configured )
			return;
		m_configured = true;
		m_backend = backend ? backend : "unknown";
		const char *dir = CommandLine() ? CommandLine()->ParmValue( "-drawstatefixture", "" ) : "";
		m_directory = dir ? dir : "";
	}

	bool Enabled() const { return !m_directory.empty(); }

	// A new frame starts: the previous frame's records are discarded.
	void BeginFrame()
	{
		m_lines.clear();
		m_truncated = false;
	}

	void Record( const Draw &draw )
	{
		if ( !Enabled() )
			return;
		if ( m_lines.size() >= kMaxDrawsPerFrame )
		{
			m_truncated = true;
			return;
		}
		std::string line = "{\"seq\":" + std::to_string( m_lines.size() );
		AppendField( line, "material", draw.material );
		AppendField( line, "shader", draw.shader );
		line += ",\"pass\":" + std::to_string( draw.pass );
		line += ",\"pass_count\":" + std::to_string( draw.passCount );
		AppendField( line, "target", draw.target );
		line += ",\"viewport\":[" + std::to_string( draw.viewport[0] ) + "," +
		        std::to_string( draw.viewport[1] ) + "," + std::to_string( draw.viewport[2] ) +
		        "," + std::to_string( draw.viewport[3] ) + "]";
		line += ",\"depth_range\":";
		AppendFloats( line, draw.depthRange, 2 );
		line += ",\"samplers\":[";
		for ( int i = 0; i < draw.samplerCount && i < kMaxSamplers; ++i )
		{
			const Sampler &s = draw.samplers[i];
			line += ( i ? ",{\"stage\":" : "{\"stage\":" ) + std::to_string( s.stage );
			AppendField( line, "texture", s.texture );
			AppendField( line, "address_u", s.addressU );
			AppendField( line, "address_v", s.addressV );
			AppendField( line, "filter", s.filter );
			line += "}";
		}
		line += "]";
		line += std::string( ",\"blend\":" ) + ( draw.blend ? "true" : "false" );
		AppendField( line, "src_blend", draw.blend ? draw.srcBlend : "one" );
		AppendField( line, "dst_blend", draw.blend ? draw.dstBlend : "zero" );
		line += std::string( ",\"depth_test\":" ) + ( draw.depthTest ? "true" : "false" );
		line += std::string( ",\"depth_write\":" ) + ( draw.depthWrite ? "true" : "false" );
		line += ",\"alpha_test_ref\":";
		if ( draw.alphaTestRef < 0.0f )
			line += "null";
		else
			AppendFloats( line, &draw.alphaTestRef, 1, false );
		line += ",\"modulation\":";
		AppendFloats( line, draw.modulation, 4 );
		line += ",\"base_texture_transform\":";
		AppendFloats( line, draw.baseTextureTransform, 8 );
		line += std::string( ",\"submitted\":" ) + ( draw.submitted ? "true" : "false" );
		if ( !draw.submitted )
			AppendField( line, "drop_reason", draw.dropReason );
		line += "}";
		m_lines.push_back( line );
	}

	// Writes the current frame's records. Returns the path, or "" on failure.
	std::string WriteFrame( const char *reason )
	{
		if ( !Enabled() )
			return "";
		const std::string path =
		    m_directory + "/" + m_backend + "-" + std::to_string( m_written ) + ".jsonl";
		std::FILE *out = std::fopen( path.c_str(), "w" );
		if ( !out )
		{
			std::fprintf( stderr, "draw-state fixture: cannot write %s\n", path.c_str() );
			return "";
		}
		std::string header = "{\"schema\":\"source-draw-state/v1\"";
		AppendField( header, "backend", m_backend.c_str() );
		AppendField( header, "reason", reason );
		header += ",\"index\":" + std::to_string( m_written );
		header += ",\"draws\":" + std::to_string( m_lines.size() );
		header += std::string( ",\"truncated\":" ) + ( m_truncated ? "true" : "false" ) + "}\n";
		std::fputs( header.c_str(), out );
		for ( const std::string &line : m_lines )
		{
			std::fputs( line.c_str(), out );
			std::fputc( '\n', out );
		}
		std::fclose( out );
		++m_written;
		return path;
	}

private:
	static void AppendField( std::string &line, const char *name, const char *value )
	{
		line += ",\"";
		line += name;
		line += "\":\"";
		for ( const char *p = value ? value : ""; *p; ++p )
		{
			const unsigned char c = static_cast<unsigned char>( *p );
			if ( c == '"' || c == '\\' )
			{
				line += '\\';
				line += static_cast<char>( c );
			}
			else if ( c < 0x20 )
			{
				char escaped[8];
				std::snprintf( escaped, sizeof( escaped ), "\\u%04x", c );
				line += escaped;
			}
			else
				line += static_cast<char>( c );
		}
		line += "\"";
	}

	static void AppendFloats( std::string &line, const float *values, int count, bool array = true )
	{
		if ( array )
			line += "[";
		for ( int i = 0; i < count; ++i )
		{
			// Six decimals: enough to see register differences, stable to diff.
			char text[32];
			std::snprintf( text, sizeof( text ), "%s%.6g", i ? "," : "", values[i] );
			line += text;
		}
		if ( array )
			line += "]";
	}

	bool m_configured = false;
	bool m_truncated = false;
	int m_written = 0;
	std::string m_backend;
	std::string m_directory;
	std::vector<std::string> m_lines;
};

// One writer per backend module.
inline Writer &Instance()
{
	static Writer s_writer;
	return s_writer;
}

} // namespace drawstatefixture

#endif // DRAWSTATEFIXTURE_H
