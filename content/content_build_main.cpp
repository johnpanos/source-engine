//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Thin CLI over the canonical C++ content graph (RFC 0015 C1).
//
//=============================================================================//

#include "content/build_graph.h"
#include "content/material_compiler.h"

#include <iostream>
#include <string>
#include <vector>

int main( int argc, char **argv )
{
	std::string source, output, profile, package;
	std::vector<content::AssetRef> roots;
	for ( int i = 1; i < argc; ++i )
	{
		const std::string option = argv[i];
		if ( option == "--texture" && i + 1 < argc )
		{
			auto ref = content::AssetRef::Create( content::AssetKind::Texture, argv[++i], true );
			if ( !ref )
			{
				std::cerr << "invalid texture identity\n";
				return 2;
			}
			roots.push_back( std::move( *ref ) );
		}
		else if ( option == "--material" && i + 1 < argc )
		{
			auto ref = content::AssetRef::Create( content::AssetKind::Material, argv[++i], true );
			if ( !ref )
			{
				std::cerr << "invalid material identity\n";
				return 2;
			}
			roots.push_back( std::move( *ref ) );
		}
		else if ( i + 1 < argc && option == "--source" )
			source = argv[++i];
		else if ( i + 1 < argc && option == "--out" )
			output = argv[++i];
		else if ( i + 1 < argc && option == "--profile" )
			profile = argv[++i];
		else if ( i + 1 < argc && option == "--package" )
			package = argv[++i];
		else
		{
			std::cerr
			    << "usage: content_build --source DIR --out DIR --profile ID "
			       "--package NAME [--material materials/NAME | --texture materials/NAME]...\n";
			return 2;
		}
	}
	if ( source.empty() || output.empty() || profile.empty() || package.empty() || roots.empty() ||
	     !content::NormalizeAssetName( profile, true ) )
	{
		std::cerr << "incomplete content build request\n";
		return 2;
	}
	std::string error;
	auto digest = content::FileDigest( argv[0], error );
	if ( !digest )
	{
		std::cerr << error << '\n';
		return 1;
	}
	content::TexturePassthroughCompiler texture;
	content::MaterialVmtCompiler material;
	content::BuildGraph graph( source, output, profile, *digest );
	graph.Register( texture );
	graph.Register( material );
	auto built = graph.Build( roots, package, error );
	if ( !built )
	{
		std::cerr << error << '\n';
		return 1;
	}
	for ( const content::BuildTraceNode &node : built->trace )
		std::cout << ( node.hit ? "hit " : "miss " ) << content::AssetKindName( node.ref.kind )
		          << ':' << node.ref.name << ' ' << node.key << '\n';
	std::cout << "package " << built->package << '\n';
	return 0;
}
