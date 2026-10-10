//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.platform.android: the adb transport (RFC 0027 L7).
//			See public/product/platform_android.h.
//
//=============================================================================//

#include "product/platform_android.h"

#include "process.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>

namespace product
{

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

constexpr const char *kRecordName = ".kiln-content.json";
constexpr size_t kStatBatch = 100;

ProviderError Fail( std::string code, std::string detail )
{
	return ProviderError{ std::move( code ), std::move( detail ) };
}

// A device path inside single quotes for `adb shell`.
std::string Quote( const std::string &path )
{
	std::string quoted = "'";
	for ( char c : path )
	{
		if ( c == '\'' )
			quoted += "'\\''";
		else
			quoted += c;
	}
	return quoted + "'";
}

std::string ParentOf( const std::string &path )
{
	const size_t at = path.rfind( '/' );
	return at == std::string::npos ? std::string() : path.substr( 0, at );
}

class AdbTransport final : public IDeployTransport,
                           private IInstall,
                           private IContentSync,
                           private ILaunch,
                           private IDeviceFacts
{
public:
	AdbTransport( platform::IToolProcessProvider &processes, std::string adb )
	    : m_Processes( processes ), m_Adb( std::move( adb ) )
	{
	}

	std::string_view Name() const noexcept override { return kAdbTransport; }
	IInstall *Install() noexcept override { return this; }
	IContentSync *ContentSync() noexcept override { return this; }
	ILaunch *Launch() noexcept override { return this; }
	IDeviceFacts *DeviceFacts() noexcept override { return this; }

private:
	foundation::Expected<android::Output, std::string> Adb( const DeviceAddress &device,
	    std::vector<std::string> arguments, const ICancellation *cancel,
	    std::chrono::milliseconds timeout = std::chrono::minutes( 2 ) )
	{
		std::vector<std::string> argv = { m_Adb };
		if ( !device.address.empty() )
			argv.insert( argv.end(), { "-s", device.address } );
		argv.insert( argv.end(), arguments.begin(), arguments.end() );
		return android::Run( m_Processes, std::move( argv ), cancel, timeout );
	}

	// kUnavailable unless exactly the addressed device is attached and ready.
	foundation::Expected<void, ProviderError> Reachable(
	    const DeviceAddress &device, const ICancellation *cancel )
	{
		auto state = Adb( device, { "get-state" }, cancel, std::chrono::seconds( 20 ) );
		if ( !state || android::FirstLine( state.Value().out ) != "device" )
			return foundation::MakeUnexpected( Fail( std::string( kUnavailable ),
			    "device \"" + device.name + "\" is not reachable over adb" +
			        ( state ? "" : ": " + state.Error() ) ) );
		return {};
	}

	foundation::Expected<void, ProviderError> Install( const DeviceAddress &device,
	    const PackageManifest &manifest, const fs::path &package,
	    const ICancellation *cancel ) override
	{
		if ( auto reachable = Reachable( device, cancel ); !reachable )
			return reachable;
		const ManifestEntry *apk = nullptr;
		for ( const ManifestEntry &entry : manifest.entries )
		{
			if ( entry.role == "package" )
				apk = &entry;
		}
		if ( !apk )
			return foundation::MakeUnexpected(
			    Fail( "invalid-request", "the manifest lists no package file" ) );
		auto installed = Adb( device, { "install", "-r", ( package / apk->path ).string() }, cancel,
		    std::chrono::minutes( 30 ) );
		if ( !installed )
			return foundation::MakeUnexpected( Fail(
			    installed.Error() == "cancelled" ? std::string( kCancelled ) : "install-failed",
			    installed.Error() ) );
		return {};
	}

	// The record kiln keeps in the content root: path -> hash of what it
	// pushed and verified. Missing or unreadable means nothing is trusted.
	std::map<std::string, std::string> ReadRecord(
	    const DeviceAddress &device, const std::string &root, const ICancellation *cancel )
	{
		std::map<std::string, std::string> record;
		auto text = Adb( device, { "shell", "cat " + Quote( root + "/" + kRecordName ) }, cancel );
		if ( !text )
			return record;
		auto parsed = foundation::json::Parse( text.Value().out );
		if ( parsed && parsed.Value().IsObject() )
		{
			for ( const auto &[path, hash] : parsed.Value().Members() )
			{
				if ( hash.IsString() )
					record[path] = hash.Text();
			}
		}
		return record;
	}

	foundation::Expected<SyncResult, ProviderError> Sync( const DeviceAddress &device,
	    const std::vector<SyncEntry> &entries, const ICancellation *cancel ) override
	{
		if ( auto reachable = Reachable( device, cancel ); !reachable )
			return foundation::MakeUnexpected( reachable.Error() );
		const std::string root = device.contentRoot.generic_string();
		if ( root.empty() || root.front() != '/' )
			return foundation::MakeUnexpected(
			    Fail( "invalid-request", "the device's content root must be absolute" ) );
		const auto cancelled = [&]
		{
			return cancel && cancel->IsCancelled();
		};
		for ( const SyncEntry &entry : entries )
		{
			if ( entry.path.empty() || entry.path.front() == '/' ||
			     entry.path.find( ".." ) != std::string::npos )
				return foundation::MakeUnexpected(
				    Fail( "invalid-request", "entry \"" + entry.path + "\" leaves the content root" ) );
		}
		if ( auto made = Adb( device, { "shell", "mkdir -p " + Quote( root ) }, cancel ); !made )
			return foundation::MakeUnexpected( Fail( "sync-failed", made.Error() ) );

		std::map<std::string, std::string> record = ReadRecord( device, root, cancel );
		SyncResult result;
		std::vector<const SyncEntry *> todo;
		std::set<std::string> wanted;
		for ( const SyncEntry &entry : entries )
		{
			wanted.insert( entry.path );
			auto have = record.find( entry.path );
			if ( have != record.end() && have->second == entry.hash )
				++result.unchanged;
			else
				todo.push_back( &entry );
		}
		// Only what kiln pushed is ever removed.
		for ( auto it = record.begin(); it != record.end(); )
		{
			if ( wanted.count( it->first ) )
			{
				++it;
				continue;
			}
			if ( cancelled() )
				return foundation::MakeUnexpected( Fail( std::string( kCancelled ), "removing" ) );
			(void)Adb( device, { "shell", "rm -f " + Quote( root + "/" + it->first ) }, cancel );
			++result.removed;
			it = record.erase( it );
		}

		// One push per destination directory.
		std::map<std::string, std::vector<const SyncEntry *>> byDirectory;
		for ( const SyncEntry *entry : todo )
			byDirectory[ParentOf( entry->path )].push_back( entry );
		for ( const auto &[directory, group] : byDirectory )
		{
			if ( cancelled() )
				return foundation::MakeUnexpected( Fail( std::string( kCancelled ), "pushing" ) );
			const std::string destination = directory.empty() ? root : root + "/" + directory;
			if ( auto made = Adb( device, { "shell", "mkdir -p " + Quote( destination ) }, cancel );
			     !made )
				return foundation::MakeUnexpected( Fail( "sync-failed", made.Error() ) );
			std::vector<std::string> push = { "push" };
			std::vector<const SyncEntry *> renamed;
			for ( const SyncEntry *entry : group )
			{
				if ( entry->source.filename().string() ==
				     fs::path( entry->path ).filename().string() )
					push.push_back( entry->source.string() );
				else
					renamed.push_back( entry );
			}
			if ( push.size() > 1 )
			{
				push.push_back( destination + "/" );
				auto pushed = Adb( device, push, cancel, std::chrono::hours( 2 ) );
				if ( !pushed )
					return foundation::MakeUnexpected(
					    Fail( pushed.Error() == "cancelled" ? std::string( kCancelled )
					                                        : "sync-failed",
					        "adb push to " + destination + ": " + pushed.Error() ) );
			}
			for ( const SyncEntry *entry : renamed )
			{
				auto pushed = Adb( device,
				    { "push", entry->source.string(), root + "/" + entry->path }, cancel,
				    std::chrono::hours( 2 ) );
				if ( !pushed )
					return foundation::MakeUnexpected( Fail( "sync-failed",
					    "adb push " + entry->path + ": " + pushed.Error() ) );
			}
		}

		// Verify what was pushed by size, in batches, before recording it.
		for ( size_t at = 0; at < todo.size(); at += kStatBatch )
		{
			std::string command = "cd " + Quote( root ) + " && stat -c '%s %n'";
			const size_t end = std::min( todo.size(), at + kStatBatch );
			for ( size_t i = at; i < end; ++i )
				command += " " + Quote( todo[i]->path );
			auto stat = Adb( device, { "shell", command }, cancel );
			if ( !stat )
				return foundation::MakeUnexpected( Fail( "sync-failed", "verify: " + stat.Error() ) );
			std::map<std::string, std::uint64_t> sizes;
			size_t line = 0;
			const std::string &text = stat.Value().out;
			while ( line < text.size() )
			{
				size_t next = text.find( '\n', line );
				if ( next == std::string::npos )
					next = text.size();
				std::string row = text.substr( line, next - line );
				if ( !row.empty() && row.back() == '\r' )
					row.pop_back();
				const size_t space = row.find( ' ' );
				if ( space != std::string::npos && row.find_first_not_of( "0123456789" ) == space )
					sizes[row.substr( space + 1 )] = std::stoull( row.substr( 0, space ) );
				line = next + 1;
			}
			for ( size_t i = at; i < end; ++i )
			{
				std::error_code ec;
				const std::uint64_t expected = fs::file_size( todo[i]->source, ec );
				auto got = sizes.find( todo[i]->path );
				if ( ec || got == sizes.end() || got->second != expected )
					return foundation::MakeUnexpected(
					    Fail( "sync-failed", "verification failed for " + todo[i]->path ) );
				record[todo[i]->path] = todo[i]->hash;
				++result.transferred;
			}
		}

		// The app's uid must be able to enter what adb created.
		std::set<std::string> tops;
		for ( const SyncEntry *entry : todo )
			tops.insert( entry->path.substr( 0, entry->path.find( '/' ) ) );
		for ( const std::string &top : tops )
		{
			if ( top.empty() || top.find( '/' ) != std::string::npos )
				continue;
			(void)Adb( device,
			    { "shell", "[ ! -d " + Quote( root + "/" + top ) + " ] || find " +
			                   Quote( root + "/" + top ) +
			                   " -type d ! -executable -prune -o -user $(id -u) -exec chmod a+rwX {} +" },
			    cancel, std::chrono::minutes( 10 ) );
		}

		// The record last: an interrupted sync re-pushes, never trusts.
		Value object = Value::Object();
		for ( const auto &[path, hash] : record )
			object.Set( path, Value::String( hash ) );
		const fs::path scratch = fs::temp_directory_path() / ( "kiln-adb-record-" + HashHex( root ) );
		std::ofstream( scratch, std::ios::binary ) << object.WritePretty() << "\n";
		auto written =
		    Adb( device, { "push", scratch.string(), root + "/" + kRecordName }, cancel );
		std::error_code ec;
		fs::remove( scratch, ec );
		if ( !written )
			return foundation::MakeUnexpected( Fail( "sync-failed", "record: " + written.Error() ) );
		return result;
	}

	foundation::Expected<LaunchResult, ProviderError> Launch(
	    const DeviceAddress &device, const LaunchRequest &request ) override
	{
		if ( auto reachable = Reachable( device, request.cancel ); !reachable )
			return foundation::MakeUnexpected( reachable.Error() );
		if ( request.arguments.empty() )
			return foundation::MakeUnexpected( Fail( "invalid-request",
			    "launch needs the component as its first argument (<package>/<activity>)" ) );
		auto started = Adb(
		    device, { "shell", "am", "start", "-n", request.arguments.front() }, request.cancel );
		if ( !started )
			return foundation::MakeUnexpected( Fail( "launch-failed", started.Error() ) );
		LaunchResult result;
		const std::string &text = started.Value().out;
		size_t line = 0;
		while ( line < text.size() && result.log.size() < 50 )
		{
			size_t next = text.find( '\n', line );
			if ( next == std::string::npos )
				next = text.size();
			result.log.push_back( text.substr( line, next - line ) );
			line = next + 1;
		}
		return result;
	}

	foundation::Expected<Value, ProviderError> Facts( const DeviceAddress &device ) override
	{
		if ( auto reachable = Reachable( device, nullptr ); !reachable )
			return foundation::MakeUnexpected( reachable.Error() );
		Value facts = Value::Object();
		for ( const auto &[name, property] : std::vector<std::pair<const char *, const char *>>{
		          { "model", "ro.product.model" }, { "android", "ro.build.version.release" },
		          { "sdk", "ro.build.version.sdk" }, { "abis", "ro.product.cpu.abilist" } } )
		{
			auto got = Adb( device, { "shell", std::string( "getprop " ) + property }, nullptr );
			facts.Set( name, Value::String( got ? android::FirstLine( got.Value().out ) : "" ) );
		}
		return facts;
	}

	platform::IToolProcessProvider &m_Processes;
	std::string m_Adb;
};

} // namespace

std::unique_ptr<IDeployTransport> CreateAdbTransport(
    platform::IToolProcessProvider &processes, std::string adb )
{
	return std::make_unique<AdbTransport>( processes, std::move( adb ) );
}

} // namespace product
