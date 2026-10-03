//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0015 identity, index, graph, cache and publication controls.
//
//=============================================================================//

#include "content/asset_index.h"
#include "content/asset_resolver.h"
#include "content/build_graph.h"
#include "content/material_compiler.h"

#include <array>
#include <barrier>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

namespace
{
int failures = 0;
void Check( bool condition, const char *message )
{
	if ( !condition )
	{
		std::cerr << "FAIL: " << message << '\n';
		++failures;
	}
}
std::vector<std::uint8_t> Read( const fs::path &path )
{
	std::ifstream stream( path, std::ios::binary );
	return { std::istreambuf_iterator<char>{ stream }, {} };
}
void Write( const fs::path &path, const std::vector<std::uint8_t> &bytes )
{
	fs::create_directories( path.parent_path() );
	std::ofstream stream( path, std::ios::binary | std::ios::trunc );
	stream.write( reinterpret_cast<const char *>( bytes.data() ), bytes.size() );
}
class RacingTextureCompiler final : public content::IAssetCompiler
{
public:
	explicit RacingTextureCompiler( std::barrier<> &gate ) : m_Gate( gate ) {}
	content::AssetKind Kind() const noexcept override { return m_Texture.Kind(); }
	std::string_view Id() const noexcept override { return m_Texture.Id(); }
	std::uint32_t Version() const noexcept override { return m_Texture.Version(); }
	std::optional<std::vector<content::AssetEdge>> Plan( const content::AssetRef &ref,
	    content::BuildInputs &inputs, std::string &error ) const override
	{
		return m_Texture.Plan( ref, inputs, error );
	}
	std::optional<std::vector<std::uint8_t>> Compile( const content::AssetRef &ref,
	    content::BuildInputs &inputs, std::string &error ) const override
	{
		m_Gate.arrive_and_wait();
		return m_Texture.Compile( ref, inputs, error );
	}

private:
	content::TexturePassthroughCompiler m_Texture;
	std::barrier<> &m_Gate;
};
} // namespace

int main()
{
	using namespace content;
	const fs::path root =
	    fs::temp_directory_path() /
	    ( "contenttest-" +
	        std::to_string( std::chrono::steady_clock::now().time_since_epoch().count() ) );
	const fs::path source = root / "source";
	const fs::path output = root / "output";
	const auto ref = AssetRef::Create( AssetKind::Texture, "MATERIALS\\Test\\Wall", true );
	Check( ref && ref->name == "materials/test/wall", "identity normalization" );
	Check( !AssetRef::Create( AssetKind::Texture, "materials/../wall" ), "parent path rejected" );
	Check( content::FoldAssetName( "Brick\\Wall/" ) == "brick/wall/",
	    "fold applies the identity's character rule without component checks" );
	Check( NormalizeAssetName( "Brick\\Wall" ) == content::FoldAssetName( "Brick\\Wall" ),
	    "a valid name normalizes to its fold" );
	Check( !AssetRef::Create( AssetKind::Texture, "materials/con/wall", true ),
	    "reserved new name rejected" );
	Check( !AssetRef::Create( AssetKind( 42 ), "materials/test/wall" ), "unknown kind rejected" );
	BuildInputs guarded( source );
	guarded.Freeze();
	std::string denied;
	Check( !guarded.Read( "materials/test/wall.vtf", &denied ) &&
	           denied.find( "undeclared" ) != std::string::npos,
	    "compile cannot read an input after its plan is frozen" );
	std::vector<std::uint8_t> vtf = {
	    'V', 'T', 'F', 0, 7, 0, 0, 0, 5, 0, 0, 0, 16, 0, 0, 0, 1, 2, 3, 4 };
	Write( source / ref->RuntimePath(), vtf );
	TexturePassthroughCompiler compiler;
	BuildGraph graph( source, output, "test", {} );
	graph.Register( compiler );
	std::string error;
	const auto first = graph.Build( std::span( &*ref, 1 ), "textures", error );
	Check( first && !first->trace[0].hit, "first build misses" );
	if ( first )
	{
		Check( Read( first->package / ref->RuntimePath() ) == vtf, "legacy bytes preserved" );
		const auto index = AssetIndex::Read( first->package / "assets.index", &error );
		Check( index && index->Find( *ref ) && index->Find( *ref )->location == ref->RuntimePath(),
		    "runtime index finds texture" );
		const AssetResolver resolver( first->package );
		Check( resolver.Find( *ref ).status == AssetLookupStatus::Indexed,
		    "mounted resolver finds indexed texture" );
		Check( !resolver.CheckClosure( std::span( &*ref, 1 ) ), "indexed root closes" );
		const auto other = AssetRef::Create( AssetKind::Texture, "materials/test/other" );
		Check( resolver.Find( *other ).status == AssetLookupStatus::Unindexed,
		    "unindexed content remains a legacy-search candidate" );
		if ( index )
		{
			auto bytes = Read( first->package / "assets.index" );
			bytes[48] ^= 1;
			Check( !AssetIndex::Open( bytes ), "directory hash detects corruption" );
			bytes.resize( 24 );
			Check( !AssetIndex::Open( bytes ), "truncated index rejected" );
		}
		const auto second = graph.Build( std::span( &*ref, 1 ), "textures", error );
		Check( second && second->trace[0].hit && second->version == first->version,
		    "incremental build hits exact action" );
		vtf.push_back( 5 );
		Write( source / ref->RuntimePath(), vtf );
		const auto third = graph.Build( std::span( &*ref, 1 ), "textures", error );
		Check( third && !third->trace[0].hit && third->version != first->version,
		    "changed input rebuilds" );
		if ( third )
		{
			Check( fs::read_symlink( output / "test/packages/textures/current" ) == third->version,
			    "current points to new complete version" );
			Check( fs::exists( first->package ), "previous package retained" );
		}
		Write( source / ref->RuntimePath(), { 'b', 'a', 'd' } );
		Check( !graph.Build( std::span( &*ref, 1 ), "textures", error ),
		    "invalid compiler input fails" );
		if ( third )
			Check( fs::read_symlink( output / "test/packages/textures/current" ) == third->version,
			    "failure preserves published package" );
		const fs::path broken = root / "broken";
		const auto material = AssetRef::Create( AssetKind::Material, "materials/test/mat" );
		const auto missing = AssetRef::Create( AssetKind::Texture, "materials/test/missing" );
		const std::vector<std::uint8_t> vmt = { 'T', 'e', 's', 't' };
		Write( broken / material->RuntimePath(), vmt );
		std::string indexError;
		const auto brokenIndex =
		    AssetIndex::Write( { AssetEntry{ *material, "legacy", material->RuntimePath(), "test",
		                           "key", {}, vmt.size() } },
		        { AssetEdge{ *material, *missing, false } }, &indexError );
		Check( brokenIndex.has_value(), "reference fixture index builds" );
		if ( brokenIndex )
		{
			Write( broken / "assets.index", *brokenIndex );
			const AssetResolver brokenResolver( broken );
			const auto why = brokenResolver.CheckClosure( std::span( &*material, 1 ) );
			Check(
			    why && why->find( "referred by material:materials/test/mat" ) != std::string::npos,
			    "missing reference names its referrer" );
		}
		Write( source / ref->RuntimePath(), vtf );
		std::barrier gate( 2 );
		RacingTextureCompiler racing( gate );
		BuildGraph left( source, root / "race", "test", {} );
		BuildGraph right( source, root / "race", "test", {} );
		left.Register( racing );
		right.Register( racing );
		std::optional<BuildResult> leftResult, rightResult;
		std::string leftError, rightError;
		std::thread firstBuild(
		    [&]
		    {
			    leftResult = left.Build( std::span( &*ref, 1 ), "textures", leftError );
		    } );
		std::thread secondBuild(
		    [&]
		    {
			    rightResult = right.Build( std::span( &*ref, 1 ), "textures", rightError );
		    } );
		firstBuild.join();
		secondBuild.join();
		Check( leftResult && rightResult && leftResult->version == rightResult->version,
		    "concurrent identical actions publish one complete package" );
		if ( leftResult )
			Check( Read( leftResult->package / ref->RuntimePath() ) == vtf,
			    "concurrent package retains exact legacy bytes" );
		const auto materialRoot = AssetRef::Create( AssetKind::Material, "materials/test/mat" );
		const std::string materialText =
		    "\"LightmappedGeneric\" { \"$basetexture\" \"test/wall\" }\n";
		Write( source / materialRoot->RuntimePath(),
		    std::vector<std::uint8_t>( materialText.begin(), materialText.end() ) );
		MaterialVmtCompiler materialCompiler;
		graph.Register( materialCompiler );
		const auto materialFirst =
		    graph.Build( std::span( &*materialRoot, 1 ), "materials", error );
		Check( materialFirst && materialFirst->trace.size() == 2 && !materialFirst->trace[0].hit,
		    "VMT compiler discovers texture closure" );
		if ( materialFirst )
		{
			Check( Read( materialFirst->package / materialRoot->RuntimePath() ) ==
			           std::vector<std::uint8_t>( materialText.begin(), materialText.end() ),
			    "legacy VMT bytes preserved" );
			const auto materialIndex = AssetIndex::Read( materialFirst->package / "assets.index" );
			Check( materialIndex && materialIndex->References( *materialRoot ).size() == 1,
			    "VMT texture reference recorded" );
			const auto same = graph.Build( std::span( &*materialRoot, 1 ), "materials", error );
			Check( same && same->trace[0].hit && same->trace[1].hit,
			    "unchanged VMT and VTF both hit" );
			vtf.push_back( 6 );
			Write( source / ref->RuntimePath(), vtf );
			const auto textureEdit =
			    graph.Build( std::span( &*materialRoot, 1 ), "materials", error );
			Check( textureEdit && textureEdit->trace[0].hit && !textureEdit->trace[1].hit,
			    "texture edit rebuilds texture and package only" );
			const std::string editedMaterial = materialText + "// changed material\n";
			Write( source / materialRoot->RuntimePath(),
			    std::vector<std::uint8_t>( editedMaterial.begin(), editedMaterial.end() ) );
			const auto materialEdit =
			    graph.Build( std::span( &*materialRoot, 1 ), "materials", error );
			Check( materialEdit && !materialEdit->trace[0].hit && materialEdit->trace[1].hit,
			    "VMT edit rebuilds material and package only" );
			const auto baseMaterial =
			    AssetRef::Create( AssetKind::Material, "materials/test/base" );
			const auto patchMaterial =
			    AssetRef::Create( AssetKind::Material, "materials/test/patch" );
			const std::string baseText =
			    "\"LightmappedGeneric\" { \"$basetexture\" \"test/wall\" }\n";
			const std::string patchText = "\"Patch\" { \"include\" \"materials/test/base.vmt\" }\n";
			Write( source / baseMaterial->RuntimePath(),
			    std::vector<std::uint8_t>( baseText.begin(), baseText.end() ) );
			Write( source / patchMaterial->RuntimePath(),
			    std::vector<std::uint8_t>( patchText.begin(), patchText.end() ) );
			const auto patchFirst = graph.Build( std::span( &*patchMaterial, 1 ), "patch", error );
			Check( patchFirst && patchFirst->trace.size() == 3 &&
			           patchFirst->trace[0].inputs.size() == 2,
			    "patch include is a discovered input and reference" );
			const std::string changedBase = baseText + "// base edit\n";
			Write( source / baseMaterial->RuntimePath(),
			    std::vector<std::uint8_t>( changedBase.begin(), changedBase.end() ) );
			const auto patchEdit = graph.Build( std::span( &*patchMaterial, 1 ), "patch", error );
			Check( patchEdit && !patchEdit->trace[0].hit && patchEdit->trace[2].hit,
			    "discovered include change invalidates patch without rebuilding texture" );
			const std::string danglingMaterial =
			    "\"LightmappedGeneric\" { \"$basetexture\" \"test/missing\" }\n";
			Write( source / materialRoot->RuntimePath(),
			    std::vector<std::uint8_t>( danglingMaterial.begin(), danglingMaterial.end() ) );
			Check( !graph.Build( std::span( &*materialRoot, 1 ), "materials", error ),
			    "required VMT texture missing from source fails closure" );
			Check( error.find( "required by material:materials/test/mat" ) != std::string::npos,
			    "build failure names the referrer" );
			if ( materialEdit )
				Check( fs::read_symlink( output / "test/packages/materials/current" ) ==
				           materialEdit->version,
				    "dangling reference preserves last package" );
			Write( source / materialRoot->RuntimePath(), { 'b', 'a', 'd' } );
			Check( !graph.Build( std::span( &*materialRoot, 1 ), "materials", error ),
			    "malformed VMT fails before publication" );
			if ( materialEdit )
				Check( fs::read_symlink( output / "test/packages/materials/current" ) ==
				           materialEdit->version,
				    "malformed VMT preserves last package" );
		}
	}
	fs::remove_all( root );
	std::cout << "contenttest: " << ( failures ? "fail" : "pass" ) << '\n';
	return failures ? 1 : 0;
}
