//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.platform.android: the android-ndk toolchain (RFC 0027 L7).
//			See public/product/platform_android.h.
//
//=============================================================================//

#include "product/platform_android.h"

#include "process.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>

namespace product
{

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

// Bump when a dependency recipe below changes, to force those rebuilds.
constexpr const char *kDependenciesRecipe = "4";
constexpr const char *kFamily = "android-ndk";

ProviderError Error( std::string code, std::string detail )
{
	return ProviderError{ std::move( code ), std::move( detail ) };
}

// One pinned archive of the profile's dependencies section.
struct Pin
{
	std::string name;
	std::string url;
	std::string sha256;
	std::uint64_t bytes = 0;
	fs::path archive;   // under the provider's home
	fs::path directory; // the extracted tree under the home
	std::string inner;  // the archive's top directory
};

bool IsHex64( const std::string &text )
{
	return text.size() == 64 && text.find_first_not_of( "0123456789abcdef" ) == std::string::npos;
}

class AndroidNdkToolchain final : public ITargetToolchain
{
public:
	explicit AndroidNdkToolchain( platform::IToolProcessProvider &processes )
	    : m_Processes( processes )
	{
	}

	std::string_view Name() const noexcept override { return kAndroidNdkToolchain; }

	foundation::Expected<ToolchainEnvironment, ProviderError> Prepare(
	    const ToolchainRequest &request ) override
	{
		if ( request.cancel && request.cancel->IsCancelled() )
			return foundation::MakeUnexpected( Error( std::string( kCancelled ), "before prepare" ) );
		if ( !request.profile )
			return foundation::MakeUnexpected( Error( "invalid-request", "no profile" ) );
		const Value &document = request.profile->document;
		const Value *toolchain = document.Find( "toolchain" );
		const std::string *family = toolchain ? toolchain->FindString( "family" ) : nullptr;
		const std::string *version = toolchain ? toolchain->FindString( "version" ) : nullptr;
		if ( !family || !version )
			return foundation::MakeUnexpected( Error(
			    "missing-pin", "toolchain.family and toolchain.version are required" ) );
		if ( *family != kFamily )
			return foundation::MakeUnexpected( Error( "toolchain-mismatch",
			    std::string( kAndroidNdkToolchain ) + " builds toolchain.family \"" + kFamily +
			        "\", the profile pins \"" + *family + "\"" ) );
		const Value *android = document.Find( "android" );
		const Value *dependencies = document.Find( "dependencies" );
		if ( !android || !dependencies )
			return foundation::MakeUnexpected(
			    Error( "missing-pin", "the android and dependencies sections are required" ) );

		// The flavor is the ABI.
		std::string abi = request.flavor;
		const Value *abis = android->Find( "abis" );
		if ( !abis || !abis->IsArray() || abis->Items().empty() )
			return foundation::MakeUnexpected( Error( "missing-pin", "android.abis" ) );
		if ( abi.empty() )
			abi = abis->Items().front().Text();
		bool declared = false;
		for ( const Value &item : abis->Items() )
			declared = declared || item.Text() == abi;
		if ( !declared )
			return foundation::MakeUnexpected(
			    Error( "invalid-flavor", "ABI \"" + abi + "\" is not in android.abis" ) );
		const Value *wafArchs = android->Find( "waf_arch" );
		const std::string *wafArch = wafArchs ? wafArchs->FindString( abi ) : nullptr;
		const Value *minSdk = android->Find( "min_sdk" );
		const Value *triples = android->Find( "ndk_triple" );
		const std::string *triple = triples ? triples->FindString( abi ) : nullptr;
		const std::string *jarSha = android->FindString( "compile_platform_jar_sha256" );
		if ( !wafArch || !minSdk || !minSdk->IsNumber() || !triple || !jarSha )
			return foundation::MakeUnexpected( Error( "missing-pin",
			    "android.waf_arch, ndk_triple, min_sdk and compile_platform_jar_sha256 are "
			    "required for " + abi ) );

		const fs::path home = request.dependencyRoot / std::string( kAndroidNdkToolchain );
		std::error_code ec;
		fs::create_directories( home, ec );
		if ( ec )
			return foundation::MakeUnexpected(
			    Error( "io", "cannot create " + home.string() + ": " + ec.message() ) );

		std::map<std::string, Pin> pins;
		for ( const char *name :
		    { "ndk", "sdl3", "ktx_software", "sdk_platform", "sdk_build_tools" } )
		{
			auto pin = ReadPin( *dependencies, name, home );
			if ( !pin )
				return foundation::MakeUnexpected( pin.Error() );
			if ( auto fetched = Fetch( pin.Value(), request.cancel ); !fetched )
				return foundation::MakeUnexpected( fetched.Error() );
			pins[name] = std::move( pin ).Value();
		}

		const Value *ndk = dependencies->Find( "ndk" );
		const std::string *ndkVersion = ndk->FindString( "version" );
		const std::string *ndkRevision = ndk->FindString( "revision" );
		if ( !ndkVersion || !ndkRevision )
			return foundation::MakeUnexpected(
			    Error( "missing-pin", "dependencies.ndk.version and revision" ) );
		if ( *ndkVersion != *version )
			return foundation::MakeUnexpected( Error( "pin-mismatch",
			    "toolchain.version pins " + *version + "; dependencies.ndk is " + *ndkVersion ) );
		const fs::path ndkDir = pins["ndk"].directory;
		if ( ReadFile( ndkDir / "source.properties" ).find( "Pkg.Revision = " + *ndkRevision ) ==
		     std::string::npos )
			return foundation::MakeUnexpected( Error( "pin-mismatch",
			    "the NDK at " + ndkDir.string() + " is not revision " + *ndkRevision ) );

		const fs::path buildTools = pins["sdk_build_tools"].directory;
		for ( const char *tool : { "aapt2", "d8", "zipalign", "apksigner" } )
		{
			if ( !fs::exists( buildTools / tool, ec ) )
				return foundation::MakeUnexpected(
				    Error( "missing-tool", ( buildTools / tool ).string() ) );
		}
		const fs::path androidJar = pins["sdk_platform"].directory / "android.jar";
		auto jar = Sha256( androidJar, request.cancel );
		if ( !jar )
			return foundation::MakeUnexpected(
			    Error( "missing-archive", androidJar.string() + ": " + jar.Error() ) );
		if ( jar.Value() != *jarSha )
			return foundation::MakeUnexpected( Error( "pin-mismatch",
			    androidJar.string() + " does not match the profile's pinned digest" ) );

		const fs::path llvm = ndkDir / "toolchains" / "llvm" / "prebuilt" / "linux-x86_64";
		const fs::path clangxx = llvm / "bin" / "clang++";
		auto clangVersion = android::Run( m_Processes, { clangxx.string(), "--version" } );
		if ( !clangVersion )
			return foundation::MakeUnexpected(
			    Error( "missing-compiler", clangxx.string() + ": " + clangVersion.Error() ) );

		// The thirdparty submodule's revision keys the vendored recipes.
		auto thirdparty = android::Run( m_Processes,
		    { "git", "-C", ( request.sourceRoot / "thirdparty" ).string(), "rev-parse", "HEAD" } );
		if ( !thirdparty )
			return foundation::MakeUnexpected(
			    Error( "missing-input", "thirdparty revision: " + thirdparty.Error() ) );

		const std::string minSdkText = minSdk->Text();
		const std::string key =
		    HashHex( std::string( "recipe=" ) + kDependenciesRecipe + " ndk=" + pins["ndk"].sha256 +
		             " sdl3=" + pins["sdl3"].sha256 + " ktx=" + pins["ktx_software"].sha256 +
		             " thirdparty=" + android::FirstLine( thirdparty.Value().out ) + " api=" + minSdkText +
		             " abi=" + abi )
		        .substr( 0, 12 );
		const fs::path abiHome = home / "abi" / ( abi + "-" + key );
		auto built = BuildDependencies( request, abiHome, abi, minSdkText, pins, *dependencies,
		    request.sourceRoot, ndkDir, llvm, *triple );
		if ( !built )
			return foundation::MakeUnexpected( built.Error() );
		const fs::path prefix = abiHome / "deps" / "prefix";

		ToolchainEnvironment environment;
		Identity &identity = environment.identity;
		identity.provider = std::string( kAndroidNdkToolchain );
		identity.facts["cxx"] = clangxx.string();
		identity.facts["cxx.version"] = android::FirstLine( clangVersion.Value().out );
		identity.facts["pin.version"] = *version;
		identity.facts["ndk.revision"] = *ndkRevision;
		identity.facts["sdk"] = "android-api-" + minSdkText;
		identity.facts["target"] = *triple;
		identity.facts["abi"] = abi;
		identity.facts["deps.key"] = key;
		identity.facts["path.ndk"] = ndkDir.string();
		identity.facts["path.strip"] = ( llvm / "bin" / "llvm-strip" ).string();
		identity.facts["path.libcxx"] =
		    ( llvm / "sysroot" / "usr" / "lib" / *triple / "libc++_shared.so" ).string();
		identity.facts["path.sdl3-lib"] = ( prefix / "lib" / "libSDL3.so" ).string();
		identity.facts["path.sdl3-src"] = pins["sdl3"].directory.string();
		identity.facts["path.build-tools"] = buildTools.string();
		identity.facts["path.android-jar"] = androidJar.string();

		environment.environment = {
		    { "ANDROID_NDK_HOME", ndkDir.string() },
		    { "PKG_CONFIG_LIBDIR",
		        ( prefix / "lib" / "pkgconfig" ).string() + ":" +
		            ( prefix / "share" / "pkgconfig" ).string() },
		    { "PKG_CONFIG_PATH", std::string() },
		    { "PKG_CONFIG_SYSROOT_DIR", std::string() } };
		environment.wafOptions = { "--android=" + *wafArch + ",clang," + minSdkText,
		    "--ktx-source-root=" + pins["ktx_software"].directory.string(),
		    "--ktx-build-root=" + ( abiHome / "deps" / "build" / "ktx" ).string() };
		environment.cmakeToolchainFile = ndkDir / "build" / "cmake" / "android.toolchain.cmake";
		environment.compilerProbe = { clangxx.string(), "--version" };
		return environment;
	}

	void CheckHost( const ResolvedProfile *, DoctorReport &report ) override
	{
		for ( const char *tool : { "curl", "sha256sum", "unzip", "tar", "cmake", "ninja", "git" } )
		{
			auto found = android::Run( m_Processes, { "sh", "-c", std::string( "command -v " ) + tool } );
			report.Add( std::string( kAndroidNdkToolchain ) + ": " + tool, found.HasValue(),
			    found ? android::FirstLine( found.Value().out ) : "not on PATH" );
		}
		auto javac = android::Run( m_Processes, { "sh", "-c", "command -v javac" } );
		report.Add( std::string( kAndroidNdkToolchain ) + ": javac", javac.HasValue(),
		    javac ? android::FirstLine( javac.Value().out )
		          : "a JDK is needed to compile SDLActivity" );
	}

private:
	static std::string ReadFile( const fs::path &path )
	{
		std::ifstream stream( path, std::ios::binary );
		return std::string( std::istreambuf_iterator<char>( stream ), {} );
	}

	foundation::Expected<std::string, std::string> Sha256(
	    const fs::path &path, const ICancellation *cancel )
	{
		auto sum = android::Run(
		    m_Processes, { "sha256sum", path.string() }, cancel, std::chrono::minutes( 10 ) );
		if ( !sum )
			return foundation::MakeUnexpected( sum.Error() );
		return sum.Value().out.substr( 0, 64 );
	}

	static foundation::Expected<Pin, ProviderError> ReadPin(
	    const Value &dependencies, const char *name, const fs::path &home )
	{
		const Value *entry = dependencies.Find( name );
		const std::string field = std::string( "dependencies." ) + name;
		if ( !entry || !entry->IsObject() )
			return foundation::MakeUnexpected( Error( "missing-pin", field ) );
		const std::string *url = entry->FindString( "url" );
		const std::string *sha = entry->FindString( "sha256" );
		const std::string *cache = entry->FindString( "cache_archive" );
		const std::string *extracted = entry->FindString( "extracted_directory" );
		const Value *bytes = entry->Find( "archive_bytes" );
		if ( !url || !sha || !IsHex64( *sha ) || !cache || !extracted || !bytes ||
		     !bytes->IsNumber() )
			return foundation::MakeUnexpected( Error( "missing-pin",
			    field + " needs url, sha256 (64 hex), archive_bytes, cache_archive and "
			            "extracted_directory" ) );
		Pin pin;
		pin.name = name;
		pin.url = *url;
		pin.sha256 = *sha;
		pin.bytes = std::stoull( bytes->Text() );
		pin.archive = home / *cache;
		pin.directory = home / *extracted;
		const std::string *inner = entry->FindString( "archive_directory" );
		pin.inner = inner ? *inner : *extracted;
		return pin;
	}

	// The archive, fetched when absent, verified once per digest, extracted
	// through staging and one rename.
	foundation::Expected<void, ProviderError> Fetch( const Pin &pin, const ICancellation *cancel )
	{
		std::error_code ec;
		const fs::path stamp = pin.archive.string() + ".verified";
		if ( fs::exists( pin.directory, ec ) )
			return {};
		if ( !fs::exists( pin.archive, ec ) )
		{
			const fs::path part = pin.archive.string() + ".part";
			auto got = android::Run( m_Processes,
			    { "curl", "-fL", "--retry", "3", "-o", part.string(), pin.url }, cancel,
			    std::chrono::hours( 1 ) );
			if ( !got )
			{
				fs::remove( part, ec );
				return foundation::MakeUnexpected(
				    Error( "fetch-failed", pin.name + ": " + got.Error() ) );
			}
			fs::rename( part, pin.archive, ec );
			if ( ec )
				return foundation::MakeUnexpected(
				    Error( "io", "cannot publish " + pin.archive.string() ) );
		}
		if ( fs::file_size( pin.archive, ec ) != pin.bytes )
			return foundation::MakeUnexpected(
			    Error( "pin-mismatch", pin.name + " archive size mismatch: " + pin.archive.string() ) );
		if ( ReadFile( stamp ) != pin.sha256 )
		{
			auto sum = Sha256( pin.archive, cancel );
			if ( !sum || sum.Value() != pin.sha256 )
				return foundation::MakeUnexpected( Error( "pin-mismatch",
				    pin.name + " SHA-256 mismatch: " + pin.archive.string() ) );
			std::ofstream( stamp, std::ios::binary ) << pin.sha256;
		}
		const fs::path tmp = pin.directory.string() + ".extract";
		fs::remove_all( tmp, ec );
		fs::create_directories( tmp, ec );
		const std::string name = pin.archive.string();
		std::vector<std::string> argv =
		    name.size() > 4 && name.substr( name.size() - 4 ) == ".zip"
		        ? std::vector<std::string>{ "unzip", "-q", name, "-d", tmp.string() }
		        : std::vector<std::string>{ "tar", "-xzf", name, "-C", tmp.string() };
		auto extracted = android::Run( m_Processes, argv, cancel, std::chrono::minutes( 30 ) );
		if ( !extracted || !fs::exists( tmp / pin.inner, ec ) )
		{
			fs::remove_all( tmp, ec );
			return foundation::MakeUnexpected( Error( "extract-failed",
			    pin.name + ": " +
			        ( extracted ? "the archive lacks " + pin.inner + "/" : extracted.Error() ) ) );
		}
		fs::rename( tmp / pin.inner, pin.directory, ec );
		const std::error_code published = ec;
		fs::remove_all( tmp, ec );
		if ( published )
			return foundation::MakeUnexpected(
			    Error( "io", "cannot publish " + pin.directory.string() ) );
		return {};
	}

	foundation::Expected<void, ProviderError> Cmake( const std::string &name,
	    const fs::path &source, const fs::path &deps, const std::string &abi,
	    const std::string &minSdk, const fs::path &ndk, const std::string &target,
	    std::vector<std::string> extra, const ICancellation *cancel )
	{
		const fs::path build = deps / "build" / name;
		const fs::path prefix = deps / "prefix";
		std::vector<std::string> configure = { "cmake", "-S", source.string(), "-B", build.string(),
		    "-G", "Ninja",
		    "-DCMAKE_TOOLCHAIN_FILE=" + ( ndk / "build" / "cmake" / "android.toolchain.cmake" ).string(),
		    "-DANDROID_ABI=" + abi, "-DANDROID_PLATFORM=android-" + minSdk,
		    "-DANDROID_STL=c++_shared", "-DCMAKE_BUILD_TYPE=Release",
		    "-DCMAKE_INSTALL_PREFIX=" + prefix.string(),
		    "-DCMAKE_FIND_ROOT_PATH=" + prefix.string(), "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
		    "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" };
		configure.insert( configure.end(), extra.begin(), extra.end() );
		const auto hour = std::chrono::hours( 1 );
		if ( auto ran = android::Run( m_Processes, configure, cancel, hour ); !ran )
			return foundation::MakeUnexpected(
			    Error( "dependency-build-failed", name + " configure: " + ran.Error() ) );
		std::vector<std::string> build_ = { "cmake", "--build", build.string() };
		if ( !target.empty() )
			build_.insert( build_.end(), { "--target", target } );
		if ( auto ran = android::Run( m_Processes, build_, cancel, 3 * hour ); !ran )
			return foundation::MakeUnexpected(
			    Error( "dependency-build-failed", name + " build: " + ran.Error() ) );
		if ( target.empty() )
		{
			if ( auto ran =
			         android::Run( m_Processes, { "cmake", "--install", build.string() }, cancel, hour );
			     !ran )
				return foundation::MakeUnexpected(
				    Error( "dependency-build-failed", name + " install: " + ran.Error() ) );
		}
		return {};
	}

	foundation::Expected<void, ProviderError> BuildDependencies( const ToolchainRequest &request,
	    const fs::path &abiHome, const std::string &abi, const std::string &minSdk,
	    std::map<std::string, Pin> &pins, const Value &dependencies, const fs::path &sourceRoot,
	    const fs::path &ndk, const fs::path &llvm, const std::string &triple )
	{
		std::error_code ec;
		if ( fs::exists( abiHome / "complete", ec ) )
			return {};
		// A directory without its `complete` file is never used: a failed or
		// interrupted build is redone from nothing.
		fs::remove_all( abiHome, ec );
		const fs::path deps = abiHome / "deps";
		const fs::path prefix = deps / "prefix";
		fs::create_directories( deps / "build", ec );
		fs::create_directories( prefix / "lib" / "pkgconfig", ec );
		if ( ec )
			return foundation::MakeUnexpected( Error( "io", "cannot create " + deps.string() ) );

		const fs::path third = sourceRoot / "thirdparty";
		const ICancellation *cancel = request.cancel;
		const auto step = [&]( const std::string &name, const fs::path &source,
		                       const std::string &target, std::vector<std::string> extra )
		{
			return Cmake( name, source, deps, abi, minSdk, ndk, target, std::move( extra ), cancel );
		};
		if ( auto r = step( "sdl3", pins["sdl3"].directory, "",
		         { "-DSDL_SHARED=ON", "-DSDL_STATIC=OFF", "-DSDL_TEST_LIBRARY=OFF", "-DSDL_TESTS=OFF",
		             "-DSDL_EXAMPLES=OFF", "-DSDL_VULKAN=ON", "-DSDL_OPENGLES=ON" } );
		     !r )
			return r;
		// The static read-only KTX library for BSP2 lightmaps; Waf links it
		// from this build tree and checks it against the pinned source.
		const Value *ktx = dependencies.Find( "ktx_software" );
		std::vector<std::string> ktxOptions;
		if ( const Value *options = ktx ? ktx->Find( "cmake_options" ) : nullptr )
		{
			for ( const auto &[option, value] : options->Members() )
				ktxOptions.push_back( "-D" + option + "=" + value.Text() );
		}
		if ( const std::string *ktxVersion = ktx ? ktx->FindString( "version" ) : nullptr )
			ktxOptions.push_back( "-DKTX_GIT_VERSION_FULL=" + *ktxVersion );
		if ( auto r = step( "ktx", pins["ktx_software"].directory, "ktx_read", ktxOptions ); !r )
			return r;
		if ( auto r = step( "freetype", third / "freetype", "",
		         { "-DBUILD_SHARED_LIBS=OFF", "-DFT_DISABLE_ZLIB=ON", "-DFT_DISABLE_BZIP2=ON",
		             "-DFT_DISABLE_PNG=ON", "-DFT_DISABLE_HARFBUZZ=ON", "-DFT_DISABLE_BROTLI=ON" } );
		     !r )
			return r;
		if ( auto r = step( "libpng", third / "libpng", "",
		         { "-DPNG_SHARED=OFF", "-DPNG_STATIC=ON", "-DPNG_EXECUTABLES=OFF",
		             "-DPNG_TESTS=OFF", "-DPNG_ARM_NEON=off" } );
		     !r )
			return r;
		if ( auto r = step( "libjpeg", third / "libjpeg", "",
		         { "-DBUILD_STATIC=ON", "-DBUILD_EXECUTABLES=OFF", "-DBUILD_TESTS=OFF" } );
		     !r )
			return r;
		if ( auto r = step( "curl", third / "curl", "",
		         { "-DBUILD_SHARED_LIBS=OFF", "-DBUILD_CURL_EXE=OFF", "-DBUILD_TESTING=OFF",
		             "-DHTTP_ONLY=ON", "-DCMAKE_USE_OPENSSL=OFF", "-DCURL_USE_OPENSSL=OFF",
		             "-DCURL_ENABLE_SSL=OFF", "-DUSE_LIBIDN2=OFF", "-DCURL_USE_LIBSSH2=OFF",
		             "-DCURL_DISABLE_LDAP=ON", "-DENABLE_MANUAL=OFF", "-DPICKY_COMPILER=OFF" } );
		     !r )
			return r;

		// The vendored libjpeg build installs no pkg-config file.
		if ( !fs::exists( prefix / "lib" / "libjpeg.a", ec ) )
			return foundation::MakeUnexpected(
			    Error( "dependency-build-failed", "libjpeg did not install lib/libjpeg.a" ) );
		std::ofstream( prefix / "lib" / "pkgconfig" / "libjpeg.pc" )
		    << "prefix=" << prefix.string() << "\nlibdir=${prefix}/lib\nincludedir=${prefix}/include\n\n"
		    << "Name: libjpeg\nDescription: vendored IJG libjpeg (thirdparty/libjpeg)\n"
		    << "Version: 9a\nLibs: -L${libdir} -ljpeg\nCflags: -I${includedir}\n";
		// zlib is the NDK's system library, which ships no pkg-config file;
		// libpng's .pc requires one.
		std::string zlibVersion = "1";
		{
			const std::string header = ReadFile( llvm / "sysroot" / "usr" / "include" / "zlib.h" );
			const std::string tag = "#define ZLIB_VERSION \"";
			const size_t at = header.find( tag );
			if ( at != std::string::npos )
				zlibVersion = header.substr( at + tag.size(),
				    header.find( '"', at + tag.size() ) - at - tag.size() );
		}
		std::ofstream( prefix / "lib" / "pkgconfig" / "zlib.pc" )
		    << "Name: zlib\nDescription: Android NDK system zlib\nVersion: " << zlibVersion
		    << "\nLibs: -lz\nCflags:\n";
		(void)triple;
		std::ofstream( abiHome / "complete" ) << "ok\n";
		return {};
	}

	platform::IToolProcessProvider &m_Processes;
};

} // namespace

std::unique_ptr<ITargetToolchain> CreateAndroidNdkToolchain( platform::IToolProcessProvider &processes )
{
	return std::make_unique<AndroidNdkToolchain>( processes );
}

} // namespace product
