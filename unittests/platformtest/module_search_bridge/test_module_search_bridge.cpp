//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Equivalence oracle for Sys_LoadModule's POSIX module search (R11):
//			the bridge over platform.module-resolver.v1 against the legacy
//			search, frozen verbatim from tier1/interface.cpp at 6a8a60d63
//			(foundLibraryWithPrefix, V_SetExtension/V_StripExtension, stat) on
//			seeded random directory trees.
//
//			Recorded deviations: a found path is the same file but no longer
//			contains "//" when the root ends in '/'; a module name with a ".."
//			segment is refused instead of searched.
//
//			Build/run: tools/quality/conformance.py check --suite platform.module_search_bridge
//
//=============================================================================//

#include "../../../tier1/module_search_bridge.h"
#include "testing/conformance_result.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace
{

// ---------------------------------------------------------------------------
// Frozen legacy search (6a8a60d63). PATHSEPARATOR is '/' or '\\'.
// ---------------------------------------------------------------------------

bool LegacyPathSeparator( char c )
{
	return c == '/' || c == '\\';
}

void LegacyStripExtension( const char *in, char *out, int outSize )
{
	int end = static_cast<int>( std::strlen( in ) ) - 1;
	while ( end > 0 && in[end] != '.' && !LegacyPathSeparator( in[end] ) )
	{
		--end;
	}
	if ( end > 0 && !LegacyPathSeparator( in[end] ) && end < outSize )
	{
		int nChars = end < outSize - 1 ? end : outSize - 1;
		if ( out != in )
		{
			std::memcpy( out, in, nChars );
		}
		out[nChars] = 0;
	}
	else if ( out != in )
	{
		std::snprintf( out, outSize, "%s", in );
	}
}

void LegacySetExtension( char *path, const char *extension, int pathStringLength )
{
	LegacyStripExtension( path, path, pathStringLength );
	if ( extension[0] != '.' )
	{
		std::strncat( path, ".", pathStringLength - std::strlen( path ) - 1 );
	}
	std::strncat( path, extension, pathStringLength - std::strlen( path ) - 1 );
}

bool LegacyFoundLibraryWithPrefix( char *pModuleAbsolutePath, size_t AbsolutePathSize,
    const char *pPath, const char *pModuleName, const char *libDir, const char *ext )
{
	char str[1024];
	std::snprintf( str, sizeof( str ), "%s", pModuleName );
	LegacySetExtension( str, ext, sizeof( str ) );
	bool bFound = false;
	struct stat statBuf;
	std::snprintf( pModuleAbsolutePath, AbsolutePathSize, "%s/%slib%s", pPath, libDir, str );
	bFound |= stat( pModuleAbsolutePath, &statBuf ) == 0;
	if ( !bFound )
	{
		std::snprintf( pModuleAbsolutePath, AbsolutePathSize, "%s/%s%s", pPath, libDir, str );
		bFound |= stat( pModuleAbsolutePath, &statBuf ) == 0;
	}
	if ( !bFound )
	{
		std::snprintf( pModuleAbsolutePath, AbsolutePathSize, "%s/lib%s", pPath, str );
		bFound |= stat( pModuleAbsolutePath, &statBuf ) == 0;
	}
	if ( !bFound )
	{
		std::snprintf( pModuleAbsolutePath, AbsolutePathSize, "%s/%s", pPath, str );
		bFound |= stat( pModuleAbsolutePath, &statBuf ) == 0;
	}
	return bFound;
}

// ---------------------------------------------------------------------------

int g_checks = 0;
int g_failures = 0;
int g_reported = 0;

void Check( bool ok, const std::string &what )
{
	++g_checks;
	if ( !ok )
	{
		++g_failures;
		if ( g_reported++ < 10 )
		{
			std::printf( "FAIL %s\n", what.c_str() );
		}
	}
}

std::string Collapse( std::string s )
{
	std::string out;
	for ( char c : s )
	{
		if ( c == '/' && !out.empty() && out.back() == '/' )
		{
			continue;
		}
		out += c;
	}
	return out;
}

std::string RealPath( const std::string &path )
{
	char buffer[PATH_MAX];
	return realpath( path.c_str(), buffer ) != nullptr ? std::string( buffer ) : std::string();
}

void Touch( const std::string &path )
{
	const int fd = open( path.c_str(), O_CREAT | O_WRONLY, 0644 );
	if ( fd >= 0 )
	{
		close( fd );
	}
}

void RemoveTree( const std::string &path )
{
	const std::string command = "rm -rf '" + path + "'";
	if ( std::system( command.c_str() ) != 0 )
	{
		std::printf( "note: could not remove %s\n", path.c_str() );
	}
}

} // namespace

int main()
{
	std::mt19937 rng( 20261008 );
	const char *bases[] = { "engine", "server", "client", "a.b/c", "sub/mod", "x" };
	const char *suffixes[] = { "", ".dll", ".so", ".", ".tar.gz" };
	const char *libDirs[] = { "bin/", "" };
	const char *exts[] = { ".so", "so", ".dylib" };

	for ( int trial = 0; trial < 400; ++trial )
	{
		char tmpl[] = "/tmp/r11-bridge-XXXXXX";
		const char *made = mkdtemp( tmpl );
		if ( made == nullptr )
		{
			Check( false, "mkdtemp" );
			break;
		}
		const std::string top = made;
		// The root: plain, with a trailing '/', or a directory that is not UTF-8.
		const int rootKind = static_cast<int>( rng() % 3 );
		std::string root = top;
		if ( rootKind == 2 )
		{
			root = top + "/\xff\xfe-root";
			mkdir( root.c_str(), 0755 );
		}
		const std::string rootArg = rootKind == 1 ? root + "/" : root;
		mkdir( ( root + "/bin" ).c_str(), 0755 );
		mkdir( ( root + "/a.b" ).c_str(), 0755 );
		mkdir( ( root + "/sub" ).c_str(), 0755 );
		mkdir( ( root + "/bin/a.b" ).c_str(), 0755 );
		mkdir( ( root + "/bin/sub" ).c_str(), 0755 );

		const char *libDir = libDirs[rng() % 2];
		const char *ext = exts[rng() % 3];
		const std::string extDot = ext[0] == '.' ? ext : std::string( "." ) + ext;
		const char *base = bases[rng() % 6];
		const std::string name = std::string( base ) + suffixes[rng() % 5];

		// Populate some candidates with files, directories, links and dangling links.
		const std::string stem = std::string( base );
		const std::string candidates[] = { root + "/" + libDir + "lib" + stem + extDot,
		    root + "/" + libDir + stem + extDot, root + "/lib" + stem + extDot,
		    root + "/" + stem + extDot };
		for ( const std::string &c : candidates )
		{
			switch ( rng() % 6 )
			{
			case 0:
				Touch( c );
				break;
			case 1:
				mkdir( c.c_str(), 0755 );
				break;
			case 2:
				Touch( c + ".target" );
				symlink( ( c + ".target" ).c_str(), c.c_str() );
				break;
			case 3:
				symlink( ( c + ".nowhere" ).c_str(), c.c_str() );
				break;
			default:
				break; // absent
			}
		}

		char legacy[2048] = {};
		char bridge[2048] = {};
		const bool legacyFound = LegacyFoundLibraryWithPrefix(
		    legacy, sizeof( legacy ), rootArg.c_str(), name.c_str(), libDir, ext );
		const bool bridgeFound = Tier1_FindModuleWithPrefix(
		    bridge, sizeof( bridge ), rootArg.c_str(), name.c_str(), libDir, ext );
		const std::string label = "trial " + std::to_string( trial ) + " name=" + name +
		                          " libDir=" + libDir + " ext=" + ext +
		                          " root=" + std::to_string( rootKind );
		Check( legacyFound == bridgeFound, label + ": found differs" );
		if ( legacyFound && bridgeFound )
		{
			Check( RealPath( legacy ) == RealPath( bridge ) && !RealPath( bridge ).empty(),
			    label + ": different file (" + legacy + " vs " + bridge + ")" );
		}
		Check( Collapse( legacy ) == Collapse( bridge ),
		    label + ": path text differs (" + legacy + " vs " + bridge + ")" );
		Check( Tier1_ModulePathExists( legacy ) == ( access( legacy, F_OK ) == 0 ||
		                                               [&]
		                                               {
			                                               struct stat st;
			                                               return stat( legacy, &st ) == 0;
		                                               }() ),
		    label + ": absolute-path check differs" );
		RemoveTree( top );
	}

	// Recorded deviation: ".." in a module name is refused, not searched.
	{
		char out[2048] = {};
		Check( !Tier1_FindModuleWithPrefix(
		           out, sizeof( out ), "/tmp", "../etc/engine", "bin/", ".so" ),
		    "a '..' segment is refused" );
	}
	std::printf( "%s module search bridge: %d checks, %d failures\n", g_failures ? "FAIL" : "ok",
	    g_checks, g_failures );
	return testing::ReportConformance( g_checks, g_failures );
}
