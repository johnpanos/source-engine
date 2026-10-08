//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.stage.fstop: the F-Stop content stage (RFC 0027 L1). A
//			port of tools/quality/stage_fstop_runtime.py's content assembly;
//			the text rules below follow it byte for byte, so the staged
//			runtime is the one a fresh Python staging produced.
//
//=============================================================================//

#include "product/stage_fstop.h"

#include "content/vpk_archive.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>

namespace product
{

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

constexpr const char *kGame = "fstop";

struct ValveContent
{
	const char *root; // in the depot
	const char *link; // the runtime directory it is mirrored into
	std::vector<const char *> directories;
};

const std::array<ValveContent, 2> kValveContent = { {
    { "portal2", "fstop_valve",
        { "materials", "models", "sound", "scenes", "expressions", "particles" } },
    { "portal2_tempcontent", "fstop_valve_tempcontent", { "materials", "models", "sound" } },
} };

constexpr const char *kValveSoundScripts[] = { "game_sounds_props_aperture.txt",
    "game_sounds_spheres_auto_generated.txt", "npc_sounds_android.txt", "npc_sounds_chicken.txt",
    "npc_sounds_hover_turret.txt", "npc_sounds_mannequin.txt", "npc_sounds_zombie_aperture.txt" };
constexpr const char *kValveParticles[] = {
    "airvents.pcf", "chicken.pcf", "fizzler.pcf", "geyser.pcf", "zombie.pcf" };
constexpr const char *kHl2SoundScripts[] = {
    "npc_sounds_zombie.txt", "npc_sounds_headcrab.txt", "npc_sounds_strider.txt" };
constexpr const char *kAuthoredSoundScripts[] = { "game_sounds_fstop.txt" };

const std::vector<std::pair<const char *, const char *>> kAuthoredScripts = {
    { "weapon_camera.txt", R"(WeaponData
{
	"printname"		"#FSTOP_Camera"
	"viewmodel"		"models/weapons/v_cam.mdl"
	"playermodel"		"models/weapons/w_cam.mdl"
	"anim_prefix"		"cam"
	"bucket"		"0"
	"bucket_position"	"0"
	"clip_size"		"1"
	"primary_ammo"		"None"
	"secondary_ammo"	"None"
	"weight"		"4"
	"item_flags"		"0"
	"autoswitchto"		"1"
	SoundData
	{
		"single_shot"		"Weapon_Camera.Capture"
		"single_shot_npc"	"Weapon_Camera.Capture"
	}
}
)" },
    { "weapon_placement.txt", R"(WeaponData
{
	"printname"		"#FSTOP_Placement"
	"viewmodel"		"models/weapons/v_photo.mdl"
	"playermodel"		"models/weapons/w_cam.mdl"
	"anim_prefix"		"cam"
	"bucket"		"1"
	"bucket_position"	"0"
	"clip_size"		"1"
	"primary_ammo"		"None"
	"secondary_ammo"	"None"
	"weight"		"4"
	"item_flags"		"0"
	"autoswitchto"		"1"
	SoundData
	{
		"single_shot"		"Weapon_Portalgun.fire_blue"
		"double_shot"		"Weapon_Portalgun.fire_red"
	}
}
)" },
    { "game_sounds_fstop.txt", R"("Weapon_Camera.Capture"
{
	"channel"		"CHAN_WEAPON"
	"volume"		"0.9"
	"soundlevel"	"SNDLVL_NORM"
	"wave"		"camera/snapshot.wav"
}

"Weapon_Camera.Release"
{
	"channel"		"CHAN_WEAPON"
	"volume"		"0.9"
	"soundlevel"	"SNDLVL_NORM"
	"wave"		"camera/release.wav"
}

"PhotoInventory.Erased"
{
	"channel"		"CHAN_ITEM"
	"volume"		"0.9"
	"soundlevel"	"SNDLVL_NORM"
	"rndwave"
	{
		"wave"	"camera/photo_erase1.wav"
		"wave"	"camera/photo_erase2.wav"
		"wave"	"camera/photo_erase3.wav"
	}
}
)" },
};

const std::vector<std::pair<const char *, const char *>> kAuthoredTokens = {
    { "FSTOP_Camera", "CAMERA" }, { "FSTOP_Placement", "PHOTOS" } };
constexpr const char *kValveTokenPrefix = "fstop_";
constexpr const char *kResourceFiles[] = {
    "photoinventory.res", "controlhelper.res", "indicator.res" };
constexpr const char *kHudElements[] = {
    "HudControlHelper", "HudPhotoInventory", "HudViewfinder", "HudIndicator" };
constexpr const char *kPortal2Models[] = { "models/props/portal_door_combined" };
constexpr const char *kPortal2ModelFiles[] = { ".mdl", ".vvd", ".dx90.vtx", ".vtx", ".phy" };
constexpr const char *kPortal2TextureKeys[] = { "$basetexture", "$bumpmap", "$normalmap",
    "$envmapmask", "$selfillummask", "$phongexponenttexture", "$detail" };
constexpr const char *kShaderOverlay = "source-engine-shaders";
constexpr const char *kBlueBlobMaterial = R"("VertexLitGeneric"
{
    "$basetexture" "vgui/white"
    "$envmap" "env_cubemap"
    "$envmaptint" "[0.08 0.2 0.3]"
    "$color2" "[0.1 0.6 0.9]"
}
)";

ProviderError Fail( std::string code, std::string detail )
{
	return ProviderError{ std::move( code ), std::move( detail ) };
}

std::string Lower( std::string text )
{
	for ( char &c : text )
		c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
	return text;
}

std::optional<std::string> ReadFile( const fs::path &path )
{
	std::error_code ec;
	if ( !fs::is_regular_file( path, ec ) )
		return std::nullopt;
	std::ifstream stream( path, std::ios::binary );
	std::ostringstream text;
	text << stream.rdbuf();
	return text.str();
}

bool WriteFile( const fs::path &path, const std::string &bytes )
{
	std::error_code ec;
	fs::create_directories( path.parent_path(), ec );
	std::ofstream stream( path, std::ios::binary | std::ios::trunc );
	return static_cast<bool>(
	    stream.write( bytes.data(), static_cast<std::streamsize>( bytes.size() ) ) );
}

// Python's str.replace over a whole string.
std::string Replace( std::string text, const std::string &from, const std::string &to )
{
	for ( size_t at = text.find( from ); at != std::string::npos;
	    at = text.find( from, at + to.size() ) )
		text.replace( at, from.size(), to );
	return text;
}

// Text read with Python's universal newlines.
std::string UniversalNewlines( const std::string &text )
{
	return Replace( Replace( text, "\r\n", "\n" ), "\r", "\n" );
}

// The entry of `directory` named `name`, ignoring case (find_child).
std::optional<fs::path> FindChild( const fs::path &directory, const std::string &name )
{
	std::error_code ec;
	const std::string wanted = Lower( name );
	for ( auto it = fs::directory_iterator( directory, ec ); !ec && it != fs::directory_iterator();
	    it.increment( ec ) )
	{
		if ( Lower( it->path().filename().string() ) == wanted )
			return it->path();
	}
	return std::nullopt;
}

bool IsPythonSpace( char c )
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

std::string Strip( const std::string &text )
{
	size_t begin = 0, end = text.size();
	while ( begin < end && IsPythonSpace( text[begin] ) )
		++begin;
	while ( end > begin && IsPythonSpace( text[end - 1] ) )
		--end;
	return text.substr( begin, end - begin );
}

// str.split() (whitespace, no empties).
std::vector<std::string> SplitWhitespace( const std::string &text )
{
	std::vector<std::string> out;
	std::string current;
	for ( char c : text )
	{
		if ( IsPythonSpace( c ) )
		{
			if ( !current.empty() )
				out.push_back( current );
			current.clear();
		}
		else
			current += c;
	}
	if ( !current.empty() )
		out.push_back( current );
	return out;
}

// str.splitlines() for the separators the content uses.
std::vector<std::string> SplitLines( const std::string &text )
{
	std::vector<std::string> lines;
	std::string current;
	for ( size_t i = 0; i < text.size(); ++i )
	{
		const char c = text[i];
		if ( c == '\n' || c == '\r' || c == '\v' || c == '\f' || c == '\x1c' || c == '\x1d' ||
		     c == '\x1e' )
		{
			lines.push_back( current );
			current.clear();
			if ( c == '\r' && i + 1 < text.size() && text[i + 1] == '\n' )
				++i;
		}
		else
			current += c;
	}
	if ( !current.empty() )
		lines.push_back( current );
	return lines;
}

// Hand-written equivalents of the Python stager's patterns (leftmost
// match, the same captures); strict modules take no regular-expression header.

// re.sub(r'(\n\s*KEY\s+)"[^"]*"', r'\1"VALUE"', text, count=1)
std::string ReplaceFirstQuotedValue(
    const std::string &text, const std::string &key, const std::string &value )
{
	for ( size_t at = text.find( '\n' ); at != std::string::npos; at = text.find( '\n', at + 1 ) )
	{
		size_t i = at + 1;
		while ( i < text.size() && IsPythonSpace( text[i] ) )
			++i;
		if ( text.compare( i, key.size(), key ) != 0 )
			continue;
		i += key.size();
		const size_t spaces = i;
		while ( i < text.size() && IsPythonSpace( text[i] ) )
			++i;
		if ( i == spaces || i >= text.size() || text[i] != '"' )
			continue;
		const size_t close = text.find( '"', i + 1 );
		if ( close == std::string::npos )
			continue;
		return text.substr( 0, i ) + "\"" + value + "\"" + text.substr( close + 1 );
	}
	return text;
}

// re.findall(r'"file"\s+"!?particles/([^"]+)"', text)
std::vector<std::string> FindParticleFiles( const std::string &text )
{
	std::vector<std::string> found;
	const std::string marker = "\"file\"";
	size_t at = text.find( marker );
	while ( at != std::string::npos )
	{
		size_t i = at + marker.size();
		const size_t spaces = i;
		while ( i < text.size() && IsPythonSpace( text[i] ) )
			++i;
		bool matched = false;
		if ( i > spaces && i < text.size() && text[i] == '"' )
		{
			++i;
			if ( i < text.size() && text[i] == '!' )
				++i;
			if ( text.compare( i, 10, "particles/" ) == 0 )
			{
				i += 10;
				const size_t close = text.find( '"', i );
				if ( close != std::string::npos && close > i )
				{
					found.push_back( text.substr( i, close - i ) );
					at = text.find( marker, close + 1 );
					matched = true;
				}
			}
		}
		if ( !matched )
			at = text.find( marker, at + 1 );
	}
	return found;
}

// re.match(r'^\s*"([^"]+)"\s+"((?:[^"\\]|\\.)*)"\s*(\[[^\]]*\])?\s*$', line)
struct Token
{
	std::string name;
	std::string value;
	bool conditional = false;
};

std::optional<Token> MatchToken( const std::string &line )
{
	size_t i = 0;
	const auto spaces = [&]
	{
		while ( i < line.size() && IsPythonSpace( line[i] ) )
			++i;
	};
	spaces();
	if ( i >= line.size() || line[i] != '"' )
		return std::nullopt;
	const size_t nameEnd = line.find( '"', i + 1 );
	if ( nameEnd == std::string::npos || nameEnd == i + 1 )
		return std::nullopt;
	Token token;
	token.name = line.substr( i + 1, nameEnd - i - 1 );
	i = nameEnd + 1;
	const size_t gap = i;
	spaces();
	if ( i == gap || i >= line.size() || line[i] != '"' )
		return std::nullopt;
	++i;
	const size_t valueStart = i;
	while ( i < line.size() && line[i] != '"' )
	{
		if ( line[i] == '\\' )
		{
			if ( i + 1 >= line.size() || line[i + 1] == '\n' )
				return std::nullopt;
			i += 2;
			continue;
		}
		++i;
	}
	if ( i >= line.size() )
		return std::nullopt;
	token.value = line.substr( valueStart, i - valueStart );
	++i;
	spaces();
	if ( i < line.size() && line[i] == '[' )
	{
		const size_t close = line.find( ']', i );
		if ( close == std::string::npos )
			return std::nullopt;
		token.conditional = true;
		i = close + 1;
		spaces();
	}
	if ( i != line.size() )
		return std::nullopt;
	return token;
}

// re.search(r'"?KEY"?\s+"?([^"\s]+)', text, re.IGNORECASE).group(1)
std::optional<std::string> FindKeyValue( const std::string &text, const std::string &key )
{
	const std::string lowerText = Lower( text );
	const std::string lowerKey = Lower( key );
	for ( size_t start = 0; start < text.size(); ++start )
	{
		size_t i = start;
		if ( text[i] == '"' )
			++i;
		if ( lowerText.compare( i, lowerKey.size(), lowerKey ) != 0 )
			continue;
		i += lowerKey.size();
		if ( i < text.size() && text[i] == '"' )
			++i;
		const size_t gap = i;
		while ( i < text.size() && IsPythonSpace( text[i] ) )
			++i;
		if ( i == gap )
			continue;
		if ( i < text.size() && text[i] == '"' )
			++i;
		const size_t valueStart = i;
		while ( i < text.size() && text[i] != '"' && !IsPythonSpace( text[i] ) )
			++i;
		if ( i > valueStart )
			return text.substr( valueStart, i - valueStart );
	}
	return std::nullopt;
}

// Ordered game content, as source_content.ContentResolver searches it: VPKs
// (case-folded names, the last duplicate winning) and loose directories (the
// exact name, then the lower-cased one).
class Resolver
{
public:
	void AddVpk( const fs::path &path )
	{
		std::error_code ec;
		if ( !fs::is_regular_file( path, ec ) )
			return;
		std::string error;
		auto archive = content::VpkArchive::Open( m_Source, path.string(), error );
		if ( !archive )
			return;
		Layer layer;
		for ( const content::VpkEntry &entry : archive->Entries() )
			layer.folded[Lower( entry.path )] = entry.path;
		layer.vpk = std::move( archive );
		m_Layers.push_back( std::move( layer ) );
	}
	void AddDirectory( const fs::path &path )
	{
		std::error_code ec;
		if ( !fs::is_directory( path, ec ) )
			return;
		Layer layer;
		layer.directory = path;
		m_Layers.push_back( std::move( layer ) );
	}
	std::optional<std::string> Read( const std::string &relative ) const
	{
		for ( const Layer &layer : m_Layers )
		{
			if ( layer.vpk )
			{
				auto it = layer.folded.find( Lower( relative ) );
				std::string bytes;
				if ( it != layer.folded.end() && layer.vpk->Read( it->second, bytes ) )
					return bytes;
				continue;
			}
			if ( auto bytes = ReadFile( layer.directory / relative ) )
				return bytes;
			if ( auto bytes = ReadFile( layer.directory / Lower( relative ) ) )
				return bytes;
		}
		return std::nullopt;
	}

private:
	struct Layer
	{
		std::unique_ptr<content::VpkArchive> vpk;
		std::map<std::string, std::string> folded;
		fs::path directory;
	};
	content::FileByteSource m_Source;
	std::vector<Layer> m_Layers;
};

// source_content.SEARCH_PATHS over a Portal runtime.
void AddPortalOrder( Resolver &resolver, const fs::path &runtime )
{
	resolver.AddVpk( runtime / "portal/portal_pak_dir.vpk" );
	resolver.AddDirectory( runtime / "portal" );
	resolver.AddVpk( runtime / "hl2/hl2_textures_dir.vpk" );
	resolver.AddVpk( runtime / "hl2/hl2_misc_dir.vpk" );
	resolver.AddDirectory( runtime / "hl2" );
}

// Python's Path ordering: by path components, then case-sensitive.
bool ComponentLess( const fs::path &a, const fs::path &b )
{
	return std::lexicographical_compare( a.begin(), a.end(), b.begin(), b.end(),
	    []( const fs::path &x, const fs::path &y )
	    {
		    return x.string() < y.string();
	    } );
}

class FstopContentStage final : public IProductStage
{
public:
	std::string_view Name() const noexcept override { return kFstopContentStage; }
	StageDescriptor Describe( const ResolvedProfile & ) const override
	{
		return { StageRole::kContent, {}, { "fstop-content" }, Determinism::kExact };
	}

	foundation::Expected<StageResult, ProviderError> Run(
	    StageInputs &inputs, StageOutputs &outputs ) override
	{
		if ( inputs.Cancel() && inputs.Cancel()->IsCancelled() )
			return foundation::MakeUnexpected( Fail( std::string( kCancelled ), "" ) );
		const auto location = [&]( const char *name ) -> fs::path
		{
			auto it = inputs.Locations().find( name );
			return it == inputs.Locations().end() ? fs::path() : it->second;
		};
		const fs::path base = location( "portal-base" );
		const fs::path depot = location( "fstop-depot" );
		const fs::path portal2Runtime = location( "portal2-runtime" );
		std::error_code ec;
		if ( base.empty() || !fs::is_regular_file( base / "portal/gameinfo.txt", ec ) )
			return foundation::MakeUnexpected( Fail( std::string( kUnavailable ),
			    "content locator \"portal-base\" has no portal/gameinfo.txt" ) );
		std::map<std::string, fs::path> roots;
		for ( const ValveContent &valve : kValveContent )
		{
			auto root = depot.empty() ? std::nullopt : FindChild( depot, valve.root );
			if ( !root || !fs::is_directory( *root, ec ) )
				return foundation::MakeUnexpected(
				    Fail( std::string( kUnavailable ), "content locator \"fstop-depot\" is not "
				                                       "Valve's depot 852 tree (needs portal2 and "
				                                       "portal2_tempcontent)" ) );
			roots[valve.root] = fs::canonical( *root, ec );
		}
		m_Out = outputs.StagingDirectory() / "fstop-content";
		fs::remove_all( m_Out, ec );
		fs::create_directories( m_Out / kGame, ec );
		m_Work = 0;

		std::uint64_t shadowed = 0;
		for ( const ValveContent &valve : kValveContent )
		{
			auto mirrored = LinkAssets( roots[valve.root], m_Out / valve.link, valve.directories );
			if ( !mirrored )
				return foundation::MakeUnexpected( mirrored.Error() );
			shadowed += mirrored.Value();
		}
		// Reads go through the staged fstop directory first, then Portal,
		// then the mirrors (source_content's F-Stop order).
		Resolver runtime;
		runtime.AddDirectory( m_Out / kGame );
		AddPortalOrder( runtime, base );
		runtime.AddDirectory( m_Out / "fstop_valve" );
		runtime.AddDirectory( m_Out / "fstop_valve_tempcontent" );

		if ( auto e = WriteGameinfo( base ) )
			return foundation::MakeUnexpected( *e );
		WriteFile( m_Out / kGame / "materials/fstop/blob_surface_bounce.vmt", kBlueBlobMaterial );
		const fs::path valve = roots["portal2"];
		const fs::path scripts = FindChild( valve, "scripts" ).value_or( valve / "scripts" );
		if ( auto e = WriteScripts( runtime, scripts ) )
			return foundation::MakeUnexpected( *e );
		if ( auto e = WriteParticles( runtime ) )
			return foundation::MakeUnexpected( *e );
		if ( auto e = WriteHudLayout( runtime, scripts ) )
			return foundation::MakeUnexpected( *e );
		const fs::path resource = FindChild( valve, "resource" ).value_or( valve / "resource" );
		if ( auto e = WriteLocalization( runtime, resource ) )
			return foundation::MakeUnexpected( *e );
		std::string models = "-";
		if ( !portal2Runtime.empty() && fs::is_directory( portal2Runtime / "portal2", ec ) )
		{
			if ( auto e = WritePortal2Assets( portal2Runtime ) )
				return foundation::MakeUnexpected( *e );
			models = kPortal2Models[0];
		}

		Artifact artifact;
		artifact.name = "fstop-content";
		artifact.type = "directory";
		artifact.path = m_Out;
		artifact.digest = HashHex( std::to_string( m_Work ) + models );
		outputs.Publish( std::move( artifact ) );
		StageResult result;
		result.workItems = m_Work;
		result.summary = std::to_string( m_Work ) + " entries (" + std::to_string( shadowed ) +
		                 " case duplicates skipped); Portal 2 models " + models;
		return result;
	}

private:
	// link_assets: a lower-case mirror of `source`'s asset directories; of names
	// that differ only in case, the first in Python's path order is kept.
	foundation::Expected<std::uint64_t, ProviderError> LinkAssets( const fs::path &source,
	    const fs::path &destination, const std::vector<const char *> &names )
	{
		std::error_code ec;
		std::uint64_t shadowed = 0;
		fs::create_directories( destination, ec );
		for ( const char *name : names )
		{
			auto directory = FindChild( source, name );
			if ( !directory || !fs::is_directory( *directory, ec ) )
				continue;
			std::vector<fs::path> files;
			for ( auto it = fs::recursive_directory_iterator( *directory, ec );
			    !ec && it != fs::recursive_directory_iterator(); it.increment( ec ) )
			{
				std::error_code inner;
				if ( it->is_regular_file( inner ) )
					files.push_back( it->path() );
			}
			std::sort( files.begin(), files.end(), ComponentLess );
			for ( const fs::path &file : files )
			{
				const fs::path target =
				    destination / name /
				    Lower( fs::relative( file, *directory, ec ).generic_string() );
				if ( fs::exists( fs::symlink_status( target, ec ) ) )
				{
					++shadowed;
					continue;
				}
				fs::create_directories( target.parent_path(), ec );
				fs::create_symlink( fs::canonical( file, ec ), target, ec );
				if ( ec )
					return foundation::MakeUnexpected(
					    Fail( "io", "cannot link " + target.string() ) );
				++m_Work;
			}
		}
		return shadowed;
	}

	std::optional<ProviderError> Write( const std::string &relative, const std::string &bytes )
	{
		if ( !WriteFile( m_Out / kGame / relative, bytes ) )
			return Fail( "io", "cannot write fstop/" + relative );
		++m_Work;
		return std::nullopt;
	}

	std::optional<ProviderError> WriteGameinfo( const fs::path &base )
	{
		auto bytes = ReadFile( base / "portal/gameinfo.txt" );
		const std::string portal = UniversalNewlines( bytes.value_or( "" ) );
		const size_t start = portal.find( "SearchPaths" );
		const size_t open = start == std::string::npos ? start : portal.find( '{', start );
		const size_t close = open == std::string::npos ? open : portal.find( '}', open );
		if ( close == std::string::npos )
			return Fail( "invalid-input", "portal/gameinfo.txt has no SearchPaths block" );
		std::vector<std::string> first;
		std::vector<std::string> lines = {
		    "\t\t\tgame+mod+mod_write+game_write+default_write_path\t|gameinfo_path|.",
		    "\t\t\tgamebin\t\t\t\t|gameinfo_path|bin" };
		const std::vector<std::string> owned = { "|gameinfo_path|.", "portal/bin" };
		const std::vector<std::string> writeKinds = {
		    "mod", "mod_write", "game_write", "default_write_path" };
		for ( const std::string &line : SplitLines( portal.substr( open + 1, close - open - 1 ) ) )
		{
			const std::vector<std::string> entry =
			    SplitWhitespace( line.substr( 0, line.find( "//" ) ) );
			if ( entry.size() != 2 ||
			     std::find( owned.begin(), owned.end(), entry[1] ) != owned.end() )
				continue;
			std::string kinds;
			std::istringstream parts( entry[0] );
			std::string kind;
			while ( std::getline( parts, kind, '+' ) )
			{
				if ( std::find( writeKinds.begin(), writeKinds.end(), kind ) == writeKinds.end() )
					kinds += ( kinds.empty() ? "" : "+" ) + kind;
			}
			if ( kinds.empty() )
				continue;
			const std::string text = "\t\t\t" + kinds + "\t\t\t" + entry[1];
			( entry[1].find( kShaderOverlay ) != std::string::npos ? first : lines )
			    .push_back( text );
		}
		for ( const ValveContent &valve : kValveContent )
			lines.push_back( std::string( "\t\t\tgame\t\t\t\t" ) + valve.link );
		first.insert( first.end(), lines.begin(), lines.end() );
		std::string head = portal.substr( 0, start );
		head = ReplaceFirstQuotedValue( head, "game", "F-Stop" );
		head = ReplaceFirstQuotedValue( head, "title", "F-STOP" );
		std::string joined;
		for ( size_t i = 0; i < first.size(); ++i )
			joined += ( i ? "\n" : "" ) + first[i];
		return Write( "gameinfo.txt",
		    head + "SearchPaths\n\t\t{\n" + joined + "\n\t\t}" + portal.substr( close + 1 ) );
	}

	std::optional<ProviderError> WriteScripts(
	    const Resolver &runtime, const fs::path &contentScripts )
	{
		std::error_code ec;
		std::vector<std::string> copied;
		for ( const char *name : kValveSoundScripts )
		{
			auto source = fs::is_directory( contentScripts, ec ) ? FindChild( contentScripts, name )
			                                                     : std::nullopt;
			if ( source && fs::is_regular_file( *source, ec ) )
			{
				if ( auto e = Write(
				         std::string( "scripts/" ) + name, ReadFile( *source ).value_or( "" ) ) )
					return e;
				copied.push_back( name );
			}
		}
		for ( const auto &[name, text] : kAuthoredScripts )
		{
			if ( auto e = Write( std::string( "scripts/" ) + name, text ) )
				return e;
		}
		auto manifest = runtime.Read( "scripts/game_sounds_manifest.txt" );
		if ( !manifest )
			return Fail(
			    "invalid-input", "the Portal runtime has no scripts/game_sounds_manifest.txt" );
		std::vector<std::string> names = copied;
		names.insert(
		    names.end(), std::begin( kAuthoredSoundScripts ), std::end( kAuthoredSoundScripts ) );
		names.insert( names.end(), std::begin( kHl2SoundScripts ), std::end( kHl2SoundScripts ) );
		std::string added;
		for ( const std::string &name : names )
			added += "\t\"precache_file\"\t\t\"scripts/" + name + "\"\n";
		const size_t close = manifest->rfind( '}' );
		return Write( "scripts/game_sounds_manifest.txt",
		    manifest->substr( 0, close ) + "\n\t// F-Stop\n" + added + manifest->substr( close ) );
	}

	std::optional<ProviderError> WriteParticles( const Resolver &runtime )
	{
		auto manifest = runtime.Read( "particles/particles_manifest.txt" );
		if ( !manifest )
			return Fail(
			    "invalid-input", "the Portal runtime has no particles/particles_manifest.txt" );
		std::vector<std::string> listed;
		for ( const std::string &name : FindParticleFiles( *manifest ) )
			listed.push_back( Lower( name ) );
		std::string added;
		for ( const char *name : kValveParticles )
		{
			if ( std::find( listed.begin(), listed.end(), name ) == listed.end() )
				added += std::string( "\t\"file\"\t\t\"particles/" ) + name + "\"\n";
		}
		const size_t close = manifest->rfind( '}' );
		return Write( "particles/particles_manifest.txt",
		    manifest->substr( 0, close ) + "\n\t// F-Stop (Valve's portal2 particles)\n" + added +
		        manifest->substr( close ) );
	}

	// The end of the first line that is NAME, optionally quoted (Python's
	// (?m)^\s*"?NAME"?\s*$), or npos.
	static size_t FindLayoutName( const std::string &text, const std::string &name )
	{
		size_t start = 0;
		while ( start <= text.size() )
		{
			size_t end = text.find( '\n', start );
			if ( end == std::string::npos )
				end = text.size();
			std::string line = Strip( text.substr( start, end - start ) );
			if ( !line.empty() && line.front() == '"' )
				line.erase( 0, 1 );
			if ( !line.empty() && line.back() == '"' )
				line.pop_back();
			if ( line == name )
				return end;
			start = end + 1;
		}
		return std::string::npos;
	}

	static std::optional<std::string> LayoutBlock(
	    const std::string &text, const std::string &name )
	{
		const size_t at = FindLayoutName( text, name );
		if ( at == std::string::npos )
			return std::nullopt;
		const size_t open = text.find( '{', at );
		if ( open == std::string::npos )
			return std::nullopt;
		int depth = 0;
		for ( size_t index = open; index < text.size(); ++index )
		{
			depth += text[index] == '{' ? 1 : text[index] == '}' ? -1 : 0;
			if ( depth == 0 )
				return "\t" + name + "\n\t" + Strip( text.substr( open, index + 1 - open ) );
		}
		return std::nullopt;
	}

	std::optional<ProviderError> WriteHudLayout(
	    const Resolver &runtime, const fs::path &contentScripts )
	{
		auto portal = runtime.Read( "scripts/hudlayout.res" );
		if ( !portal )
			return Fail( "invalid-input", "the Portal runtime has no scripts/hudlayout.res" );
		std::error_code ec;
		auto source = fs::is_directory( contentScripts, ec )
		                  ? FindChild( contentScripts, "hudlayout.res" )
		                  : std::nullopt;
		const std::string fstop =
		    source ? Replace( UniversalNewlines( ReadFile( *source ).value_or( "" ) ), "\r", "" )
		           : "";
		std::vector<std::string> blocks;
		for ( const char *name : kHudElements )
		{
			if ( FindLayoutName( *portal, name ) != std::string::npos )
				continue;
			blocks.push_back( LayoutBlock( fstop, name )
			        .value_or( std::string( "\t" ) + name + "\n\t{\n\t\t\"fieldName\" \"" + name +
			                   "\"\n\t\t\"visible\" \"1\"\n\t\t\"enabled\" \"1\"\n\t\t\"wide\"\t "
			                   "\"640\"\n\t\t\"tall\"\t \"480\"\n\t}" ) );
		}
		std::string joined;
		for ( size_t i = 0; i < blocks.size(); ++i )
			joined += ( i ? "\n" : "" ) + blocks[i];
		const size_t close = portal->rfind( '}' );
		return Write( "scripts/hudlayout.res", portal->substr( 0, close ) + "\n\t// F-Stop\n" +
		                                           joined + "\n" + portal->substr( close ) );
	}

	// UTF-16 (with its byte-order mark) or UTF-8 (with an optional BOM) to UTF-8.
	static std::string DecodeLocalization( const std::string &data )
	{
		const auto byte = [&]( size_t i )
		{
			return static_cast<unsigned char>( data[i] );
		};
		if ( data.size() >= 2 && ( ( byte( 0 ) == 0xff && byte( 1 ) == 0xfe ) ||
		                             ( byte( 0 ) == 0xfe && byte( 1 ) == 0xff ) ) )
		{
			const bool little = byte( 0 ) == 0xff;
			std::string out;
			for ( size_t i = 2; i + 1 < data.size(); i += 2 )
			{
				std::uint32_t unit = little ? ( byte( i ) | ( byte( i + 1 ) << 8 ) )
				                            : ( ( byte( i ) << 8 ) | byte( i + 1 ) );
				if ( unit >= 0xd800 && unit <= 0xdbff && i + 3 < data.size() )
				{
					const std::uint32_t low = little ? ( byte( i + 2 ) | ( byte( i + 3 ) << 8 ) )
					                                 : ( ( byte( i + 2 ) << 8 ) | byte( i + 3 ) );
					unit = 0x10000 + ( ( unit - 0xd800 ) << 10 ) + ( low - 0xdc00 );
					i += 2;
				}
				AppendUtf8( out, unit );
			}
			return out;
		}
		if ( data.size() >= 3 && byte( 0 ) == 0xef && byte( 1 ) == 0xbb && byte( 2 ) == 0xbf )
			return data.substr( 3 );
		return data;
	}

	static void AppendUtf8( std::string &out, std::uint32_t cp )
	{
		if ( cp < 0x80 )
			out += static_cast<char>( cp );
		else if ( cp < 0x800 )
		{
			out += static_cast<char>( 0xc0 | ( cp >> 6 ) );
			out += static_cast<char>( 0x80 | ( cp & 0x3f ) );
		}
		else if ( cp < 0x10000 )
		{
			out += static_cast<char>( 0xe0 | ( cp >> 12 ) );
			out += static_cast<char>( 0x80 | ( ( cp >> 6 ) & 0x3f ) );
			out += static_cast<char>( 0x80 | ( cp & 0x3f ) );
		}
		else
		{
			out += static_cast<char>( 0xf0 | ( cp >> 18 ) );
			out += static_cast<char>( 0x80 | ( ( cp >> 12 ) & 0x3f ) );
			out += static_cast<char>( 0x80 | ( ( cp >> 6 ) & 0x3f ) );
			out += static_cast<char>( 0x80 | ( cp & 0x3f ) );
		}
	}

	// UTF-8 to UTF-16LE with a byte-order mark.
	static std::string EncodeUtf16le( const std::string &text )
	{
		std::string out = "\xff\xfe";
		const auto put = [&]( std::uint32_t unit )
		{
			out += static_cast<char>( unit & 0xff );
			out += static_cast<char>( ( unit >> 8 ) & 0xff );
		};
		for ( size_t i = 0; i < text.size(); )
		{
			const unsigned char c = static_cast<unsigned char>( text[i] );
			std::uint32_t cp = c;
			size_t length = 1;
			if ( c >= 0xf0 && i + 3 < text.size() )
			{
				cp = ( ( c & 0x07u ) << 18 ) | ( ( text[i + 1] & 0x3fu ) << 12 ) |
				     ( ( text[i + 2] & 0x3fu ) << 6 ) | ( text[i + 3] & 0x3fu );
				length = 4;
			}
			else if ( c >= 0xe0 && i + 2 < text.size() )
			{
				cp = ( ( c & 0x0fu ) << 12 ) | ( ( text[i + 1] & 0x3fu ) << 6 ) |
				     ( text[i + 2] & 0x3fu );
				length = 3;
			}
			else if ( c >= 0xc0 && i + 1 < text.size() )
			{
				cp = ( ( c & 0x1fu ) << 6 ) | ( text[i + 1] & 0x3fu );
				length = 2;
			}
			if ( cp >= 0x10000 )
			{
				cp -= 0x10000;
				put( 0xd800 + ( cp >> 10 ) );
				put( 0xdc00 + ( cp & 0x3ff ) );
			}
			else
				put( cp );
			i += length;
		}
		return out;
	}

	std::optional<ProviderError> WriteLocalization(
	    const Resolver &runtime, const fs::path &contentResource )
	{
		auto found = runtime.Read( "resource/portal_english.txt" );
		if ( !found )
			return Fail( "invalid-input", "the Portal runtime has no resource/portal_english.txt" );
		const std::string portal = DecodeLocalization( *found );
		std::vector<std::string> present;
		for ( const std::string &line : SplitLines( portal ) )
		{
			if ( auto match = MatchToken( line ) )
				present.push_back( Lower( match->name ) );
		}
		std::vector<std::pair<std::string, std::string>> candidates(
		    kAuthoredTokens.begin(), kAuthoredTokens.end() );
		std::error_code ec;
		auto source = fs::is_directory( contentResource, ec )
		                  ? FindChild( contentResource, "portal2_english.txt" )
		                  : std::nullopt;
		if ( source )
		{
			for ( const std::string &line :
			    SplitLines( DecodeLocalization( ReadFile( *source ).value_or( "" ) ) ) )
			{
				auto match = MatchToken( line );
				if ( match && !match->conditional &&
				     Lower( match->name ).rfind( kValveTokenPrefix, 0 ) == 0 )
					candidates.emplace_back( match->name, match->value );
			}
		}
		std::vector<std::string> extra;
		for ( const auto &[name, value] : candidates )
		{
			if ( std::find( present.begin(), present.end(), Lower( name ) ) == present.end() )
			{
				present.push_back( Lower( name ) );
				extra.push_back( "\t\t\"" + name + "\"\t\t\"" + value + "\"" );
			}
		}
		const size_t last = portal.rfind( '}' );
		const size_t close = last == std::string::npos || last == 0 ? std::string::npos
		                                                            : portal.rfind( '}', last - 1 );
		if ( portal.find( "\"Tokens\"" ) == std::string::npos || close == std::string::npos )
			return Fail( "invalid-input", "resource/portal_english.txt has no Tokens block" );
		std::string joined;
		for ( size_t i = 0; i < extra.size(); ++i )
			joined += ( i ? "\n" : "" ) + extra[i];
		if ( auto e = Write( "resource/fstop_english.txt",
		         EncodeUtf16le( portal.substr( 0, close ) + "\n\t\t// F-Stop\n" + joined + "\n\t" +
		                        portal.substr( close ) ) ) )
			return e;
		for ( const char *name : kResourceFiles )
		{
			auto control = fs::is_directory( contentResource, ec )
			                   ? FindChild( contentResource, name )
			                   : std::nullopt;
			if ( control && fs::is_regular_file( *control, ec ) )
			{
				if ( auto e = Write(
				         std::string( "resource/" ) + name, ReadFile( *control ).value_or( "" ) ) )
					return e;
			}
		}
		return std::nullopt;
	}

	// The material names a studio model uses: its cdmaterials x texture names.
	static std::vector<std::string> ModelMaterials( const std::string &mdl )
	{
		const auto i32 = [&]( size_t at ) -> std::int32_t
		{
			if ( at + 4 > mdl.size() )
				return 0;
			std::uint32_t v = 0;
			std::memcpy( &v, mdl.data() + at, 4 );
			return static_cast<std::int32_t>( v );
		};
		const auto text = [&]( size_t at ) -> std::string
		{
			if ( at >= mdl.size() )
				return {};
			const size_t end = mdl.find( '\0', at );
			return mdl.substr( at, end == std::string::npos ? std::string::npos : end - at );
		};
		const std::int32_t count = i32( 0xcc ), index = i32( 0xd0 );
		std::vector<std::string> names;
		for ( std::int32_t i = 0; i < count; ++i )
		{
			const size_t record = static_cast<size_t>( index ) + 64u * static_cast<size_t>( i );
			names.push_back( text( record + static_cast<size_t>( i32( record ) ) ) );
		}
		const std::int32_t cdCount = i32( 0xd4 ), cdIndex = i32( 0xd8 );
		std::vector<std::string> folders;
		for ( std::int32_t i = 0; i < cdCount; ++i )
			folders.push_back( text( static_cast<size_t>(
			    i32( static_cast<size_t>( cdIndex ) + 4u * static_cast<size_t>( i ) ) ) ) );
		if ( folders.empty() )
			folders.push_back( "" );
		std::vector<std::string> materials;
		for ( const std::string &folder : folders )
		{
			for ( const std::string &name : names )
				materials.push_back( Lower( Replace( folder + name, "\\", "/" ) ) );
		}
		return materials;
	}

	std::optional<ProviderError> WritePortal2Assets( const fs::path &portal2Runtime )
	{
		Resolver resolver;
		for ( const char *name : { "update", "portal2_dlc2", "portal2_dlc1", "portal2" } )
		{
			resolver.AddVpk( portal2Runtime / name / "pak01_dir.vpk" );
			resolver.AddDirectory( portal2Runtime / name );
		}
		const auto copy = [&]( const std::string &relative ) -> std::optional<std::string>
		{
			auto data = resolver.Read( relative );
			if ( data && Write( relative, *data ) )
				return std::nullopt;
			return data;
		};
		for ( const char *model : kPortal2Models )
		{
			auto mdl = copy( std::string( model ) + ".mdl" );
			if ( !mdl )
				return Fail(
				    "invalid-input", portal2Runtime.string() + " has no " + model + ".mdl" );
			for ( size_t i = 1; i < std::size( kPortal2ModelFiles ); ++i )
				copy( std::string( model ) + kPortal2ModelFiles[i] );
			for ( const std::string &material : ModelMaterials( *mdl ) )
			{
				auto vmt = copy( "materials/" + material + ".vmt" );
				if ( !vmt )
					continue;
				for ( const char *key : kPortal2TextureKeys )
				{
					if ( auto texture = FindKeyValue( *vmt, key ) )
						copy( "materials/" + Lower( Replace( *texture, "\\", "/" ) ) + ".vtf" );
				}
			}
		}
		return std::nullopt;
	}

	fs::path m_Out;
	std::uint64_t m_Work = 0;
};

} // namespace

std::unique_ptr<IProductStage> CreateFstopContentStage()
{
	return std::make_unique<FstopContentStage>();
}

} // namespace product
