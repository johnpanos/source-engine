//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.platform.android: the android-native-libs stage
//			(RFC 0027 L7). See public/product/platform_android.h.
//
//=============================================================================//

#include "product/platform_android.h"
#include "product/stage_waf.h"

#include "process.h"

#include <filesystem>
#include <fstream>
#include <map>

namespace product
{

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

ProviderError Fail( std::string code, std::string detail )
{
	return ProviderError{ std::move( code ), std::move( detail ) };
}

std::string ReadBytes( const fs::path &path )
{
	std::ifstream stream( path, std::ios::binary );
	return std::string( std::istreambuf_iterator<char>( stream ), {} );
}

class NativeLibsStage final : public IProductStage
{
public:
	std::string_view Name() const noexcept override { return kAndroidNativeLibsStage; }

	StageDescriptor Describe( const ResolvedProfile & ) const override
	{
		StageDescriptor descriptor;
		descriptor.role = StageRole::kEngine;
		descriptor.consumes = { std::string( kEngineInstallArtifact ) };
		descriptor.produces = { std::string( kAndroidLibsArtifact ) };
		descriptor.determinism = Determinism::kExact;
		return descriptor;
	}

	foundation::Expected<StageResult, ProviderError> Run(
	    StageInputs &inputs, StageOutputs &outputs ) override
	{
		if ( !inputs.Processes() || !inputs.Toolchain() )
			return foundation::MakeUnexpected(
			    Fail( "invalid-request", "the stage needs the toolchain and a process provider" ) );
		const Artifact *install = inputs.Get( kEngineInstallArtifact );
		if ( !install )
			return foundation::MakeUnexpected(
			    Fail( "missing-input", "no engine-install artifact" ) );
		const auto &facts = inputs.Toolchain()->identity.facts;
		const auto fact = [&]( const char *key ) -> foundation::Expected<std::string, ProviderError>
		{
			auto found = facts.find( key );
			if ( found == facts.end() )
				return foundation::MakeUnexpected(
				    Fail( "toolchain-mismatch", std::string( "the toolchain has no fact " ) + key ) );
			return found->second;
		};
		auto strip = fact( "path.strip" );
		auto libcxx = fact( "path.libcxx" );
		auto sdl3 = fact( "path.sdl3-lib" );
		auto buildTools = fact( "path.build-tools" );
		auto androidJar = fact( "path.android-jar" );
		auto sdl3Source = fact( "path.sdl3-src" );
		for ( auto *each : { &strip, &libcxx, &sdl3, &buildTools, &androidJar, &sdl3Source } )
		{
			if ( !*each )
				return foundation::MakeUnexpected( each->Error() );
		}
		const std::string abi = inputs.Flavor();

		const fs::path staging = outputs.StagingDirectory() / "libs";
		const fs::path symbols = staging / "symbols";
		const fs::path stripped = staging / "lib" / abi;
		std::error_code ec;
		fs::create_directories( symbols, ec );
		fs::create_directories( stripped, ec );
		if ( ec )
			return foundation::MakeUnexpected( Fail( "io", "cannot create " + staging.string() ) );

		std::map<std::string, fs::path> sources;
		for ( auto it = fs::recursive_directory_iterator( install->path, ec );
		    !ec && it != fs::recursive_directory_iterator(); it.increment( ec ) )
		{
			if ( it->is_regular_file() && it->path().extension() == ".so" )
				sources[it->path().filename().string()] = it->path();
		}
		sources[fs::path( sdl3.Value() ).filename().string()] = sdl3.Value();
		sources[fs::path( libcxx.Value() ).filename().string()] = libcxx.Value();
		if ( !sources.count( "libmain.so" ) )
			return foundation::MakeUnexpected(
			    Fail( "missing-input", "the engine build produced no libmain.so" ) );

		std::string digest;
		for ( const auto &[name, source] : sources )
		{
			if ( inputs.Cancel() && inputs.Cancel()->IsCancelled() )
				return foundation::MakeUnexpected( Fail( std::string( kCancelled ), name ) );
			fs::copy_file( source, symbols / name, fs::copy_options::overwrite_existing, ec );
			if ( ec )
				return foundation::MakeUnexpected(
				    Fail( "io", "cannot copy " + source.string() + ": " + ec.message() ) );
			auto ran = android::Run( *inputs.Processes(),
			    { strip.Value(), "--strip-unneeded", "-o", ( stripped / name ).string(),
			        ( symbols / name ).string() },
			    inputs.Cancel(), std::chrono::minutes( 5 ) );
			if ( !ran )
				return foundation::MakeUnexpected(
				    Fail( "strip-failed", name + ": " + ran.Error() ) );
			digest += name + ":" + HashHex( ReadBytes( stripped / name ) ) + "\n";
		}

		Artifact libs;
		libs.name = std::string( kAndroidLibsArtifact );
		libs.type = "directory";
		libs.path = staging;
		libs.digest = HashHex( digest );
		libs.facts.Set( "abi", Value::String( abi ) );
		libs.facts.Set( "build_tools", Value::String( buildTools.Value() ) );
		libs.facts.Set( "android_jar", Value::String( androidJar.Value() ) );
		libs.facts.Set( "sdl3_source", Value::String( sdl3Source.Value() ) );
		outputs.Publish( std::move( libs ) );

		StageResult result;
		result.workItems = sources.size();
		result.summary = std::to_string( sources.size() ) + " libraries stripped for " + abi;
		return result;
	}
};

} // namespace

std::unique_ptr<IProductStage> CreateAndroidNativeLibsStage()
{
	return std::make_unique<NativeLibsStage>();
}

} // namespace product
