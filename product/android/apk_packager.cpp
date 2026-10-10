//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.platform.android: the android-apk packager (RFC 0027 L7).
//			See public/product/platform_android.h.
//
//=============================================================================//

#include "product/platform_android.h"

#include "process.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

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

std::string Number( const Value *section, const char *key )
{
	const Value *value = section ? section->Find( key ) : nullptr;
	return value && value->IsNumber() ? value->Text() : std::string();
}

std::string Replace( std::string text, const std::string &from, const std::string &to )
{
	for ( size_t at = text.find( from ); at != std::string::npos; at = text.find( from, at + to.size() ) )
		text.replace( at, from.size(), to );
	return text;
}

class ApkPackager final : public IPackager
{
public:
	explicit ApkPackager( platform::IToolProcessProvider &processes ) : m_Processes( processes ) {}

	std::string_view Name() const noexcept override { return kAndroidApkPackager; }

	foundation::Expected<PackageManifest, ProviderError> Package(
	    const PackageRequest &request ) override
	{
		if ( !request.profile )
			return foundation::MakeUnexpected( Fail( "invalid-request", "no profile" ) );
		const Value &document = request.profile->document;
		const Value *android = document.Find( "android" );
		if ( !android )
			return foundation::MakeUnexpected( Fail( "missing-pin", "the android section" ) );
		const auto text = [&]( const char *key ) -> const std::string *
		{
			return android->FindString( key );
		};
		const std::string *appId = text( "application_id" );
		const std::string *label = text( "application_label" );
		const std::string *versionName = text( "version_name" );
		const std::string *javaTemplate = text( "manifest_template" );
		const std::string versionCode = Number( android, "version_code" );
		const std::string minSdk = Number( android, "min_sdk" );
		const std::string targetSdk = Number( android, "target_sdk" );
		const std::string javaRelease = Number( android, "java_release" );
		const std::string pageAlign = Number( android, "page_size_alignment" );
		if ( !appId || !label || !versionName || !javaTemplate || versionCode.empty() ||
		     minSdk.empty() || targetSdk.empty() || javaRelease.empty() || pageAlign.empty() )
			return foundation::MakeUnexpected( Fail( "missing-pin",
			    "android.application_id, application_label, version_code, version_name, min_sdk, "
			    "target_sdk, java_release, page_size_alignment and manifest_template" ) );
		const Value *options = document.Find( "configure_options" );
		const std::string *game = options ? options->FindString( "build_games" ) : nullptr;

		auto found = request.artifacts.find( std::string( kAndroidLibsArtifact ) );
		if ( found == request.artifacts.end() )
			return foundation::MakeUnexpected(
			    Fail( "missing-input", "the package needs the android-libs artifact" ) );
		const Artifact &libs = found->second;
		const std::string *abi = libs.facts.FindString( "abi" );
		const std::string *buildToolsText = libs.facts.FindString( "build_tools" );
		const std::string *androidJarText = libs.facts.FindString( "android_jar" );
		const std::string *sdl3Text = libs.facts.FindString( "sdl3_source" );
		if ( !abi || !buildToolsText || !androidJarText || !sdl3Text )
			return foundation::MakeUnexpected(
			    Fail( "missing-input", "android-libs lacks its tool facts" ) );
		const fs::path buildTools = *buildToolsText;
		const fs::path androidJar = *androidJarText;
		const fs::path sdl3 = *sdl3Text;

		const auto credential = [&]( const char *key ) -> std::string
		{
			auto it = request.credentials.find( key );
			return it == request.credentials.end() ? std::string() : it->second;
		};
		const bool release = !credential( "keystore" ).empty();
		const std::string kind = release ? "release" : "debug";
		if ( release && credential( "keystore" ).rfind( request.sourceRoot.string() + "/", 0 ) == 0 )
			return foundation::MakeUnexpected( Fail( "credential-in-tree",
			    "keep the release keystore outside the repository" ) );

		const fs::path work = request.output.string() + ".work";
		std::error_code ec;
		fs::remove_all( work, ec );
		fs::create_directories( work / "classes", ec );
		fs::create_directories( work / "dex", ec );
		fs::create_directories( work / "manifest", ec );
		if ( ec )
			return foundation::MakeUnexpected( Fail( "io", "cannot create " + work.string() ) );

		const auto run = [&]( std::vector<std::string> argv, const char *what,
		                     std::vector<platform::ToolProcessEnvironmentOverride> env = {},
		                     const std::string &cwd = "/" ) -> foundation::Expected<void, ProviderError>
		{
			if ( request.cancel && request.cancel->IsCancelled() )
				return foundation::MakeUnexpected( Fail( std::string( kCancelled ), what ) );
			auto ran = android::Run( m_Processes, std::move( argv ), request.cancel,
			    std::chrono::minutes( 20 ), std::move( env ), cwd );
			if ( !ran )
				return foundation::MakeUnexpected(
				    Fail( ran.Error() == "cancelled" ? std::string( kCancelled ) : "package-failed",
				        std::string( what ) + ": " + ran.Error() ) );
			return {};
		};
		const auto bail = [&]( ProviderError error ) -> foundation::Expected<PackageManifest, ProviderError>
		{
			fs::remove_all( work, ec );
			return foundation::MakeUnexpected( std::move( error ) );
		};

		// The manifest, from the template.
		const std::string manifest = Replace(
		    Replace( ReadBytes( request.sourceRoot / *javaTemplate ), "@PACKAGE@", *appId ),
		    "@APP_LABEL@", *label );
		if ( manifest.empty() )
			return bail( Fail( "missing-input", "manifest template " + *javaTemplate ) );
		std::ofstream( work / "manifest" / "AndroidManifest.xml", std::ios::binary ) << manifest;

		// The app's own UI art, installed into <game>/custom/ at startup.
		if ( auto r = run( { "python3", ( request.sourceRoot / "tools/android/touch_icons.py" ).string(),
		                       ( work / "assets" / "touch" ).string() },
		         "touch icons" );
		     !r )
			return bail( r.Error() );

		std::vector<std::string> link = { ( buildTools / "aapt2" ).string(), "link", "-o",
		    ( work / "base.apk" ).string(), "-I", androidJar.string() };
		if ( !release )
			link.push_back( "--debug-mode" ); // run-as, native debugging
		link.insert( link.end(), { "-A", ( work / "assets" ).string(), "--manifest",
		                             ( work / "manifest" / "AndroidManifest.xml" ).string(),
		                             "--min-sdk-version", minSdk, "--target-sdk-version", targetSdk,
		                             "--version-code", versionCode, "--version-name", *versionName } );
		if ( auto r = run( link, "aapt2 link" ); !r )
			return bail( r.Error() );

		// SDLActivity and its helpers, from the pinned SDL3 source.
		std::vector<std::string> javac = { "javac", "-nowarn", "-Xlint:-options", "--release",
		    javaRelease, "-classpath", androidJar.string(), "-d", ( work / "classes" ).string() };
		for ( auto it = fs::recursive_directory_iterator(
		          sdl3 / "android-project" / "app" / "src" / "main" / "java", ec );
		    !ec && it != fs::recursive_directory_iterator(); it.increment( ec ) )
		{
			if ( it->path().extension() == ".java" )
				javac.push_back( it->path().string() );
		}
		if ( auto r = run( javac, "javac" ); !r )
			return bail( r.Error() );
		std::vector<std::string> d8 = { ( buildTools / "d8" ).string(), "--release", "--min-api",
		    minSdk, "--lib", androidJar.string(), "--output", ( work / "dex" ).string() };
		for ( auto it = fs::recursive_directory_iterator( work / "classes", ec );
		    !ec && it != fs::recursive_directory_iterator(); it.increment( ec ) )
		{
			if ( it->path().extension() == ".class" )
				d8.push_back( it->path().string() );
		}
		if ( auto r = run( d8, "d8" ); !r )
			return bail( r.Error() );

		fs::copy_file( work / "base.apk", work / "unaligned.apk", ec );
		fs::copy( libs.path / "lib", work / "lib", fs::copy_options::recursive, ec );
		if ( ec )
			return bail( Fail( "io", "cannot stage lib/: " + ec.message() ) );
		if ( auto r = run( { "zip", "-q", "-X", ( work / "unaligned.apk" ).string(), "classes.dex" },
		         "zip dex", {}, ( work / "dex" ).string() );
		     !r )
			return bail( r.Error() );
		if ( auto r = run( { "zip", "-q", "-X", "-r", ( work / "unaligned.apk" ).string(),
		                       "lib/" + *abi },
		         "zip lib", {}, work.string() );
		     !r )
			return bail( r.Error() );
		const std::string pageKib = std::to_string( std::stoul( pageAlign ) / 1024 );
		if ( auto r = run( { ( buildTools / "zipalign" ).string(), "-f", "-P", pageKib, "4",
		                       ( work / "unaligned.apk" ).string(), ( work / "aligned.apk" ).string() },
		         "zipalign" );
		     !r )
			return bail( r.Error() );
		if ( auto r = run( { ( buildTools / "zipalign" ).string(), "-c", "-P", pageKib, "4",
		                       ( work / "aligned.apk" ).string() },
		         "zipalign verification" );
		     !r )
			return bail( r.Error() );

		// Signing. Secrets travel in the signer's environment only.
		const fs::path apkName = ( game ? *game : std::string( "app" ) ) + "-" + *versionName + "-" +
		                         *abi + "-" + kind + ".apk";
		std::vector<std::string> sign = { ( buildTools / "apksigner" ).string(), "sign" };
		std::vector<platform::ToolProcessEnvironmentOverride> signEnv;
		if ( release )
		{
			sign.insert( sign.end(), { "--ks", credential( "keystore" ), "--ks-key-alias",
			                             credential( "key_alias" ).empty() ? "source-engine"
			                                                              : credential( "key_alias" ) } );
			if ( !credential( "keystore_pass" ).empty() )
			{
				sign.insert( sign.end(), { "--ks-pass", "env:KILN_KEYSTORE_PASS" } );
				signEnv.push_back( { "KILN_KEYSTORE_PASS", credential( "keystore_pass" ) } );
			}
			if ( !credential( "key_pass" ).empty() )
			{
				sign.insert( sign.end(), { "--key-pass", "env:KILN_KEY_PASS" } );
				signEnv.push_back( { "KILN_KEY_PASS", credential( "key_pass" ) } );
			}
		}
		else
		{
			auto home = android::Run( m_Processes, { "sh", "-c", "printf %s \"$HOME\"" } );
			if ( !home || home.Value().out.empty() )
				return bail( Fail( "unavailable", "no HOME for the debug keystore" ) );
			const fs::path keystore = fs::path( home.Value().out ) / ".android" / "debug.keystore";
			if ( !fs::exists( keystore, ec ) )
			{
				fs::create_directories( keystore.parent_path(), ec );
				if ( auto r = run( { "keytool", "-genkeypair", "-keystore", keystore.string(),
				                       "-storepass", "android", "-keypass", "android", "-alias",
				                       "androiddebugkey", "-keyalg", "RSA", "-keysize", "2048",
				                       "-validity", "10000", "-dname",
				                       "CN=Android Debug,O=Android,C=US" },
				         "keytool" );
				     !r )
					return bail( r.Error() );
			}
			sign.insert( sign.end(), { "--ks", keystore.string(), "--ks-pass", "pass:android",
			                             "--ks-key-alias", "androiddebugkey" } );
		}
		sign.insert( sign.end(), { "--out", ( work / apkName ).string(),
		                             ( work / "aligned.apk" ).string() } );
		if ( auto r = run( sign, "apksigner", std::move( signEnv ) ); !r )
			return bail( r.Error() );
		fs::remove( work / ( apkName.string() + ".idsig" ), ec );

		// The independent reader judges the package against the profile.
		const fs::path report = work / ( apkName.string() + ".check.json" );
		if ( auto r = run( { "python3", ( request.sourceRoot / "tools/quality/android_apk.py" ).string(),
		                       "check", ( work / apkName ).string(), "--build-tools",
		                       buildTools.string(), "--profile",
		                       ( request.sourceRoot / "quality" / "product_profiles" /
		                           request.profile->file ).string(),
		                       "--variant", kind, "--report", report.string(), "--abi=" + *abi },
		         "APK verification" );
		     !r )
			return bail( r.Error() );

		// Publish: the directory holds the APK and its report, nothing else.
		fs::remove_all( request.output, ec );
		fs::create_directories( request.output, ec );
		for ( const fs::path &name : { apkName, fs::path( report.filename() ) } )
		{
			fs::rename( work / name, request.output / name, ec );
			if ( ec )
				return bail( Fail( "io", "cannot publish " + name.string() ) );
		}
		fs::remove_all( work, ec );

		PackageManifest result;
		result.form = std::string( kAndroidApkPackager );
		for ( const fs::path &name : { apkName, fs::path( report.filename() ) } )
		{
			const std::string bytes = ReadBytes( request.output / name );
			ManifestEntry entry;
			entry.path = name.generic_string();
			entry.hash = HashHex( bytes );
			entry.role = name == apkName ? "package" : "evidence";
			entry.bytes = bytes.size();
			result.entries.push_back( std::move( entry ) );
		}
		std::sort( result.entries.begin(), result.entries.end(),
		    []( const auto &a, const auto &b )
		    {
			    return a.path < b.path;
		    } );
		return result;
	}

private:
	platform::IToolProcessProvider &m_Processes;
};

} // namespace

std::unique_ptr<IPackager> CreateAndroidApkPackager( platform::IToolProcessProvider &processes )
{
	return std::make_unique<ApkPackager>( processes );
}

} // namespace product
