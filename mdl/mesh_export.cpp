//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Export the portable studio-model reader's LOD 0 mesh to the
//          legacy-map lighting scene builder. This is a host tool, not a
//          second MDL/VVD/VTX parser.
//
//=============================================================================//

#include "mdl/studio_model.h"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

namespace
{

bool Read( const char *path, std::string &out )
{
	std::ifstream file( path, std::ios::binary );
	if ( !file )
		return false;
	out.assign( std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() );
	return file.good() || file.eof();
}

void String( std::ostream &out, const std::string &value )
{
	out << '"';
	for ( unsigned char c : value )
	{
		if ( c == '"' || c == '\\' )
			out << '\\' << c;
		else if ( c < 0x20 )
			out << "\\u00" << "0123456789abcdef"[c >> 4] << "0123456789abcdef"[c & 15];
		else
			out << c;
	}
	out << '"';
}

void Vec3( std::ostream &out, const mdl::Float3 &v )
{
	out << '[' << v.x << ',' << v.y << ',' << v.z << ']';
}

} // namespace

int main( int argc, char **argv )
{
	// --color-groups: the strip groups a static prop's baked vertex lighting
	// lists (mdl::VertexColorGroup), with the model's checksum.
	const bool colorGroups = argc == 5 && std::string( argv[4] ) == "--color-groups";
	if ( argc != 4 && !colorGroups )
	{
		std::fprintf(
		    stderr, "usage: mdl_mesh_export model.mdl model.vvd model.vtx [--color-groups]\n" );
		return 2;
	}
	std::string mdlBytes, vvdBytes, vtxBytes;
	if ( !Read( argv[1], mdlBytes ) || !Read( argv[2], vvdBytes ) || !Read( argv[3], vtxBytes ) )
	{
		std::fprintf( stderr, "mdl_mesh_export: a model input could not be read\n" );
		return 1;
	}
	auto parsed = colorGroups
	                  ? mdl::ParseModelGeometryVariants( { mdlBytes, vvdBytes, vtxBytes, {} } )
	                  : mdl::ParseModel( { mdlBytes, vvdBytes, vtxBytes, {} } );
	if ( !parsed )
	{
		std::fprintf( stderr, "mdl_mesh_export: %s\n", mdl::Describe( parsed.Error() ).c_str() );
		return 1;
	}
	const mdl::Model &model = parsed.Value();
	std::ostringstream out;
	out.precision( 9 );
	if ( colorGroups )
	{
		out << "{\"checksum\":" << model.checksum << ",\"groups\":[";
		for ( std::size_t i = 0; i < model.vertexColorGroups.size(); ++i )
		{
			const mdl::VertexColorGroup &group = model.vertexColorGroups[i];
			out << ( i ? "," : "" ) << "{\"lod\":" << group.lod << ",\"p\":[";
			for ( std::size_t j = 0; j < group.vertices.size(); ++j )
			{
				out << ( j ? "," : "" );
				Vec3( out, group.vertices[j].position );
			}
			out << "],\"n\":[";
			for ( std::size_t j = 0; j < group.vertices.size(); ++j )
			{
				out << ( j ? "," : "" );
				Vec3( out, group.vertices[j].normal );
			}
			out << "]}";
		}
		out << "]}\n";
		const std::string bytes = out.str();
		return std::fwrite( bytes.data(), 1, bytes.size(), stdout ) == bytes.size() ? 0 : 1;
	}
	out << "{\"name\":";
	String( out, model.name );
	out << ",\"textures\":[";
	for ( std::size_t i = 0; i < model.textures.size(); ++i )
	{
		if ( i )
			out << ',';
		String( out, model.textures[i] );
	}
	out << "],\"cdMaterials\":[";
	for ( std::size_t i = 0; i < model.cdMaterials.size(); ++i )
	{
		if ( i )
			out << ',';
		String( out, model.cdMaterials[i] );
	}
	out << "],\"skinFamilies\":[";
	for ( std::size_t i = 0; i < model.skinFamilies.size(); ++i )
	{
		if ( i )
			out << ',';
		out << '[';
		for ( std::size_t j = 0; j < model.skinFamilies[i].size(); ++j )
		{
			if ( j )
				out << ',';
			out << model.skinFamilies[i][j];
		}
		out << ']';
	}
	out << "],\"meshes\":[";
	for ( std::size_t i = 0; i < model.meshes.size(); ++i )
	{
		if ( i )
			out << ',';
		const mdl::Mesh &mesh = model.meshes[i];
		out << "{\"textureRef\":" << mesh.textureRef << ",\"vertices\":[";
		for ( std::size_t j = 0; j < mesh.vertices.size(); ++j )
		{
			if ( j )
				out << ',';
			const mdl::Vertex &v = mesh.vertices[j];
			out << "{\"p\":";
			Vec3( out, v.position );
			out << ",\"n\":";
			Vec3( out, v.normal );
			out << ",\"uv\":[" << v.u << ',' << v.v << "]}";
		}
		out << "],\"indices\":[";
		for ( std::size_t j = 0; j < mesh.indices.size(); ++j )
		{
			if ( j )
				out << ',';
			out << mesh.indices[j];
		}
		out << "]}";
	}
	out << "]}\n";
	const std::string bytes = out.str();
	return std::fwrite( bytes.data(), 1, bytes.size(), stdout ) == bytes.size() ? 0 : 1;
}
