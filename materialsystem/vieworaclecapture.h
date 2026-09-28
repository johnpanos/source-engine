//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: View oracle captures (schema source-view-oracle/v1, RFC 0016 K0).
//
//          The frozen record of which views a real product frame renders and
//          which draws each view makes, in submission order: the oracle the
//          render core's later gates (K1-K3) compare byte for byte.
//
//          A shader API backend records, in the order it receives them:
//          - label_begin / label_end / marker: the render context's PIX
//            events. The engine brackets every CRender::Push3DView/Push2DView
//            ... PopView span with a PIX event whose name starts with
//            "vieworacle " followed by a JSON object describing the view
//            (engine/gl_rmain.cpp, only under -vieworacle). Other PIX events
//            (mat_pix_events) are recorded too; the comparator
//            (tools/render/view_oracle.py) reads only the view events.
//          - clear: a ClearBuffers call.
//          - draw: one material render pass, with the geometry it drew.
//
//          Enabled with "-vieworacle <directory>". Each screenshot writes the
//          frame being captured to <directory>/<backend>-<n>.jsonl: one header
//          line, then one line per event. Off, every entry point returns after
//          one test of a bool.
//
//===========================================================================//

#ifndef VIEWORACLECAPTURE_H
#define VIEWORACLECAPTURE_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/icommandline.h"

#include <cstdio>
#include <string>
#include <vector>

namespace vieworacle
{

enum
{
	// A frame larger than this is truncated, and the header says so.
	kMaxEventsPerFrame = 262144
};

struct Draw
{
	const char *material = "";
	const char *shader = "";
	int pass = 0;
	int passCount = 0;
	int primitive = 0; // MaterialPrimitiveType_t
	int firstIndex = 0;
	int indexCount = 0;
	int vertexCount = 0;
	bool worldBatch = false;
	const char *target = "backbuffer"; // render-target texture name
	int viewport[4] = { 0, 0, 0, 0 };  // x, y, width, height
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
		const char *dir = CommandLine() ? CommandLine()->ParmValue( "-vieworacle", "" ) : "";
		m_directory = dir ? dir : "";
		m_enabled = !m_directory.empty();
	}

	bool Enabled() const { return m_enabled; }

	// A new frame starts: the previous frame's records are discarded.
	void BeginFrame()
	{
		if ( !m_enabled )
			return;
		m_lines.clear();
		m_truncated = false;
	}

	void LabelBegin( const char *name )
	{
		if ( !Admit() )
			return;
		std::string line = Start( "label_begin" );
		AppendField( line, "name", name );
		Finish( line );
	}

	void LabelEnd()
	{
		if ( !Admit() )
			return;
		std::string line = Start( "label_end" );
		Finish( line );
	}

	void Marker( const char *name )
	{
		if ( !Admit() )
			return;
		std::string line = Start( "marker" );
		AppendField( line, "name", name );
		Finish( line );
	}

	void Clear( bool color, bool depth, bool stencil, const char *target )
	{
		if ( !Admit() )
			return;
		std::string line = Start( "clear" );
		line += std::string( ",\"color\":" ) + ( color ? "true" : "false" );
		line += std::string( ",\"depth\":" ) + ( depth ? "true" : "false" );
		line += std::string( ",\"stencil\":" ) + ( stencil ? "true" : "false" );
		AppendField( line, "target", target );
		Finish( line );
	}

	void RecordDraw( const Draw &draw )
	{
		if ( !Admit() )
			return;
		std::string line = Start( "draw" );
		AppendField( line, "material", draw.material );
		AppendField( line, "shader", draw.shader );
		line += ",\"pass\":" + std::to_string( draw.pass );
		line += ",\"pass_count\":" + std::to_string( draw.passCount );
		line += ",\"primitive\":" + std::to_string( draw.primitive );
		line += ",\"first_index\":" + std::to_string( draw.firstIndex );
		line += ",\"index_count\":" + std::to_string( draw.indexCount );
		line += ",\"vertex_count\":" + std::to_string( draw.vertexCount );
		line += std::string( ",\"world_batch\":" ) + ( draw.worldBatch ? "true" : "false" );
		AppendField( line, "target", draw.target );
		line += ",\"viewport\":[" + std::to_string( draw.viewport[0] ) + "," +
		        std::to_string( draw.viewport[1] ) + "," + std::to_string( draw.viewport[2] ) +
		        "," + std::to_string( draw.viewport[3] ) + "]";
		line += std::string( ",\"submitted\":" ) + ( draw.submitted ? "true" : "false" );
		if ( !draw.submitted )
			AppendField( line, "drop_reason", draw.dropReason );
		Finish( line );
	}

	// Writes the current frame's records. Returns the path, or "" on failure.
	std::string WriteFrame( const char *reason )
	{
		if ( !m_enabled )
			return "";
		const std::string path =
		    m_directory + "/" + m_backend + "-" + std::to_string( m_written ) + ".jsonl";
		std::FILE *out = std::fopen( path.c_str(), "w" );
		if ( !out )
		{
			std::fprintf( stderr, "view oracle: cannot write %s\n", path.c_str() );
			return "";
		}
		std::string header = "{\"schema\":\"source-view-oracle/v1\"";
		AppendField( header, "backend", m_backend.c_str() );
		AppendField( header, "reason", reason );
		header += ",\"index\":" + std::to_string( m_written );
		header += ",\"events\":" + std::to_string( m_lines.size() );
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
	bool Admit()
	{
		if ( !m_enabled )
			return false;
		if ( m_lines.size() >= kMaxEventsPerFrame )
		{
			m_truncated = true;
			return false;
		}
		return true;
	}

	std::string Start( const char *event ) const
	{
		std::string line = "{\"seq\":" + std::to_string( m_lines.size() );
		line += ",\"event\":\"";
		line += event;
		line += "\"";
		return line;
	}

	void Finish( std::string &line )
	{
		line += "}";
		m_lines.push_back( std::move( line ) );
	}

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

	bool m_configured = false;
	bool m_enabled = false;
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

} // namespace vieworacle

#endif // VIEWORACLECAPTURE_H
