//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.stage.video-av1: the video.av1 compiler (RFC 0027 L1), a
//			port of tools/video/transcode_av1.py. See stage_video_av1.h.
//
//=============================================================================//

#include "product/stage_video_av1.h"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <fstream>
#include <sstream>

namespace product
{

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

constexpr const char *kManifest = "manifest.json";
constexpr double kMinSsim = 0.97;
constexpr int kCrfLadder[] = { 24, 18, 12 };
constexpr int kKeyframeSeconds = 2;

ProviderError Fail( std::string code, std::string detail )
{
	return ProviderError{ std::move( code ), std::move( detail ) };
}

std::string ReadFile( const fs::path &path )
{
	std::ifstream stream( path, std::ios::binary );
	std::ostringstream text;
	text << stream.rdbuf();
	return text.str();
}

Value Strings( std::initializer_list<const char *> items )
{
	Value list = Value::Array();
	for ( const char *item : items )
		list.Push( Value::String( item ) );
	return list;
}

// The encoder settings; any change re-encodes every clip (the manifest key).
Value Settings()
{
	Value settings = Value::Object();
	settings.Set( "version", Value::Number( 2 ) );
	settings.Set( "video", Strings( { "-c:v", "libsvtav1", "-preset", "4", "-pix_fmt", "yuv420p",
	                           "-svtav1-params", "tune=0" } ) );
	Value ladder = Value::Array();
	for ( int crf : kCrfLadder )
		ladder.Push( Value::Number( crf ) );
	settings.Set( "crf_ladder", std::move( ladder ) );
	settings.Set( "keyframe_seconds", Value::Number( kKeyframeSeconds ) );
	settings.Set( "audio", Strings( { "-c:a", "libopus", "-b:a", "128k" } ) );
	return settings;
}

long long MtimeNs( const fs::path &path )
{
	std::error_code ec;
	const auto system = std::chrono::file_clock::to_sys( fs::last_write_time( path, ec ) );
	return static_cast<long long>(
	    std::chrono::duration_cast<std::chrono::nanoseconds>( system.time_since_epoch() ).count() );
}

Value SourceKey( const fs::path &clip )
{
	std::error_code ec;
	Value key = Value::Object();
	key.Set( "size", Value::Number( static_cast<long long>( fs::file_size( clip, ec ) ) ) );
	key.Set( "mtime_ns", Value::Number( MtimeNs( clip ) ) );
	key.Set( "settings", Settings() );
	return key;
}

// Python's json.dumps(sort_keys=True): members sorted at every level.
Value SortKeys( const Value &value )
{
	if ( value.IsArray() )
	{
		Value list = Value::Array();
		for ( const Value &item : value.Items() )
			list.Push( SortKeys( item ) );
		return list;
	}
	if ( !value.IsObject() )
		return value;
	std::vector<std::pair<std::string, Value>> members = value.Members();
	std::sort( members.begin(), members.end(),
	    []( const auto &a, const auto &b )
	    {
		    return a.first < b.first;
	    } );
	Value sorted = Value::Object();
	for ( const auto &member : members )
		sorted.Set( member.first, SortKeys( member.second ) );
	return sorted;
}

// Python's repr of a float rounded to 5 places (round(x, 5)).
std::string Round5( double value )
{
	const double rounded = std::nearbyint( value * 100000.0 ) / 100000.0;
	char text[64];
	auto result = std::to_chars( text, text + sizeof( text ), rounded );
	std::string out( text, result.ptr );
	if ( out.find_first_of( ".e" ) == std::string::npos )
		out += ".0";
	return out;
}

class VideoAv1Stage final : public IProductStage
{
public:
	std::string_view Name() const noexcept override { return kVideoAv1Stage; }
	StageDescriptor Describe( const ResolvedProfile & ) const override
	{
		return { StageRole::kContent, {}, { "av1-media" }, Determinism::kStatistical };
	}

	foundation::Expected<StageResult, ProviderError> Run(
	    StageInputs &inputs, StageOutputs &outputs ) override
	{
		if ( inputs.Cancel() && inputs.Cancel()->IsCancelled() )
			return foundation::MakeUnexpected( Fail( std::string( kCancelled ), "" ) );
		auto source = inputs.Locations().find( "steam-portal2" );
		auto out = inputs.Locations().find( "av1-media" );
		if ( source == inputs.Locations().end() || out == inputs.Locations().end() )
			return foundation::MakeUnexpected( Fail( std::string( kUnavailable ),
			    "video-av1 needs the steam-portal2 and av1-media locators" ) );
		std::error_code ec;
		const fs::path root = fs::canonical( source->second, ec );
		const fs::path store = fs::weakly_canonical( out->second, ec );
		std::vector<std::pair<std::string, fs::path>> clips;
		std::vector<fs::path> games;
		for ( auto it = fs::directory_iterator( root, ec ); !ec && it != fs::directory_iterator();
		    it.increment( ec ) )
		{
			std::error_code inner;
			if ( it->is_directory( inner ) )
				games.push_back( it->path() );
		}
		std::sort( games.begin(), games.end() );
		for ( const fs::path &game : games )
		{
			std::vector<fs::path> media;
			for ( auto it = fs::directory_iterator( game / "media", ec );
			    !ec && it != fs::directory_iterator(); it.increment( ec ) )
			{
				std::string extension = it->path().extension().string();
				std::transform( extension.begin(), extension.end(), extension.begin(), ::tolower );
				if ( extension == ".bik" )
					media.push_back( it->path() );
			}
			ec.clear();
			std::sort( media.begin(), media.end() );
			for ( const fs::path &clip : media )
				clips.emplace_back( fs::relative( clip, root ).generic_string(), clip );
		}
		if ( clips.empty() )
			return foundation::MakeUnexpected(
			    Fail( "invalid-input", "no media/*.bik under " + root.string() ) );

		Value manifest =
		    foundation::json::Parse( ReadFile( store / kManifest ) ).ValueOr( Value() );
		if ( !manifest.IsObject() || !manifest.Find( "clips" ) )
		{
			manifest = Value::Object();
			manifest.Set( "clips", Value::Object() );
		}
		std::vector<std::pair<std::string, fs::path>> todo;
		for ( const auto &[name, clip] : clips )
		{
			const Value *record = manifest.Find( "clips" )->Find( name );
			const Value *key = record ? record->Find( "key" ) : nullptr;
			fs::path target = store / name;
			target.replace_extension( ".webm" );
			if ( !key || !( *key == SourceKey( clip ) ) || !fs::is_regular_file( target, ec ) )
				todo.emplace_back( name, clip );
		}
		StageResult result;
		if ( !todo.empty() )
		{
			fs::create_directories( store, ec );
			// One clip at a time: the process provider runs one process at a
			// time, and SVT-AV1 threads each encode itself.
			std::uint64_t failures = 0;
			for ( const auto &[name, clip] : todo )
			{
				if ( inputs.Cancel() && inputs.Cancel()->IsCancelled() )
					break;
				fs::path target = store / name;
				target.replace_extension( ".webm" );
				auto record = Transcode( inputs, clip, target );
				Value *clipsValue = manifest.Find( "clips" );
				if ( !record )
				{
					clipsValue->Remove( name );
					++failures;
					if ( inputs.Diagnostics() )
						inputs.Diagnostics()->Report( Severity::kWarning, Name(),
						    "FAILED " + name + ": " + record.Error().detail );
					continue;
				}
				Value entry = std::move( record ).Value();
				entry.Set( "key", SourceKey( clip ) );
				clipsValue->Set( name, std::move( entry ) );
				++result.workItems;
				// Written after every clip, so an interrupted run keeps its progress.
				std::ofstream stream( store / kManifest, std::ios::binary | std::ios::trunc );
				stream << SortKeys( manifest ).WritePretty( 1 ) << "\n";
			}
			if ( inputs.Cancel() && inputs.Cancel()->IsCancelled() )
				return foundation::MakeUnexpected( Fail( std::string( kCancelled ), "" ) );
			if ( failures )
				return foundation::MakeUnexpected(
				    Fail( "transcode-failed", std::to_string( failures ) + " clips failed" ) );
		}
		Artifact artifact;
		artifact.name = "av1-media";
		artifact.type = "directory";
		artifact.path = store;
		artifact.digest = HashHex( ReadFile( store / kManifest ) );
		outputs.Publish( std::move( artifact ) );
		result.upToDate = todo.empty();
		result.summary = todo.empty() ? std::to_string( clips.size() ) + " clips current"
		                              : std::to_string( result.workItems ) + " clips encoded";
		return result;
	}

private:
	foundation::Expected<std::string, ProviderError> Tool(
	    StageInputs &inputs, std::vector<std::string> argv, bool wantStderr )
	{
		platform::ToolProcessRequest request;
		request.argv = std::move( argv );
		request.workingDirectory = "/";
		request.executionTimeout = std::chrono::hours( 2 );
		request.cancellationTimeout = std::chrono::seconds( 5 );
		const platform::ToolProcessResult result = inputs.Processes()->Run( request );
		if ( !result.Succeeded() )
		{
			if ( result.error.code == platform::ToolProcessErrorCode::kExecutableNotFound )
				return foundation::MakeUnexpected(
				    Fail( std::string( kUnavailable ), request.argv[0] + " is not installed" ) );
			return foundation::MakeUnexpected( Fail(
			    "tool-failed", request.argv[0] + ": " + result.stderrData.substr( 0, 400 ) ) );
		}
		return wantStderr ? result.stderrData : result.stdoutData;
	}

	foundation::Expected<std::vector<Value>, ProviderError> Probe(
	    StageInputs &inputs, const fs::path &path )
	{
		auto out = Tool( inputs,
		    { "ffprobe", "-v", "error", "-count_frames", "-show_entries",
		        "stream=index,codec_type,codec_name,pix_fmt,width,height,avg_frame_rate,r_frame_"
		        "rate,nb_read_frames",
		        "-of", "json", path.string() },
		    false );
		if ( !out )
			return foundation::MakeUnexpected( out.Error() );
		auto parsed = foundation::json::Parse( out.Value() );
		const Value *streams =
		    parsed && parsed.Value().IsObject() ? parsed.Value().Find( "streams" ) : nullptr;
		if ( !streams || !streams->IsArray() )
			return foundation::MakeUnexpected(
			    Fail( "tool-failed", "ffprobe gave no streams for " + path.string() ) );
		return streams->Items();
	}

	static std::string Text( const Value &stream, const char *key )
	{
		const Value *value = stream.Find( key );
		return value ? value->Text() : std::string();
	}

	foundation::Expected<Value, ProviderError> Transcode(
	    StageInputs &inputs, const fs::path &clip, const fs::path &target )
	{
		std::string failures;
		for ( int crf : kCrfLadder )
		{
			bool belowGate = false;
			auto record = Encode( inputs, clip, target, crf, belowGate );
			if ( record )
			{
				Value entry = std::move( record ).Value();
				entry.Set( "crf", Value::Number( crf ) );
				return entry;
			}
			if ( !belowGate )
				return record;
			failures += ( failures.empty() ? "" : "; " ) + std::string( "crf " ) +
			            std::to_string( crf ) + ": " + record.Error().detail;
		}
		return foundation::MakeUnexpected( Fail( "ssim", failures ) );
	}

	foundation::Expected<Value, ProviderError> Encode( StageInputs &inputs, const fs::path &clip,
	    const fs::path &target, int crf, bool &belowGate )
	{
		auto streams = Probe( inputs, clip );
		if ( !streams )
			return foundation::MakeUnexpected( streams.Error() );
		std::vector<Value> video;
		int audioIn = 0;
		for ( const Value &stream : streams.Value() )
		{
			if ( Text( stream, "codec_type" ) == "video" )
				video.push_back( stream );
			audioIn += Text( stream, "codec_type" ) == "audio";
		}
		if ( video.size() != 1 || ( Text( video[0], "pix_fmt" ) != "yuv420p" &&
		                              Text( video[0], "pix_fmt" ) != "yuvj420p" ) )
			return foundation::MakeUnexpected(
			    Fail( "invalid-input", "expected one yuv420p video stream" ) );
		const std::string rate = Text( video[0], "r_frame_rate" );
		const size_t slash = rate.find( '/' );
		const double fps =
		    std::stod( rate.substr( 0, slash ) ) /
		    ( slash == std::string::npos ? 1.0 : std::stod( rate.substr( slash + 1 ) ) );
		const long gop = std::max( 1L, std::lround( fps * kKeyframeSeconds ) );
		std::error_code ec;
		fs::create_directories( target.parent_path(), ec );
		fs::path partial = target;
		partial.replace_filename( target.stem().string() + ".partial.webm" );
		std::vector<std::string> command = {
		    "ffmpeg", "-v", "error", "-y", "-i", clip.string(), "-map", "0:v:0", "-map", "0:a?" };
		const Value settings = Settings(); // the loops below borrow from it
		for ( const Value &item : settings.Find( "video" )->Items() )
			command.push_back( item.Text() );
		for ( const std::string &item :
		    { std::string( "-crf" ), std::to_string( crf ), std::string( "-g" ),
		        std::to_string( gop ), std::string( "-fps_mode" ), std::string( "passthrough" ) } )
			command.push_back( item );
		for ( const Value &item : settings.Find( "audio" )->Items() )
			command.push_back( item.Text() );
		command.push_back( partial.string() );
		struct Cleanup
		{
			fs::path path;
			~Cleanup()
			{
				std::error_code ignored;
				fs::remove( path, ignored );
			}
		} cleanup{ partial };
		auto encoded = Tool( inputs, command, false );
		if ( !encoded )
			return foundation::MakeUnexpected( encoded.Error() );
		auto outStreams = Probe( inputs, partial );
		if ( !outStreams )
			return foundation::MakeUnexpected( outStreams.Error() );
		std::vector<Value> outVideo;
		int audioOut = 0;
		for ( const Value &stream : outStreams.Value() )
		{
			if ( Text( stream, "codec_type" ) == "video" )
				outVideo.push_back( stream );
			audioOut += Text( stream, "codec_type" ) == "audio";
		}
		if ( outVideo.size() != 1 || Text( outVideo[0], "codec_name" ) != "av1" ||
		     Text( outVideo[0], "pix_fmt" ) != "yuv420p" )
			return foundation::MakeUnexpected(
			    Fail( "invalid-output", "output is not one AV1 yuv420p stream" ) );
		const std::string framesIn = Text( video[0], "nb_read_frames" ),
		                  framesOut = Text( outVideo[0], "nb_read_frames" );
		if ( framesIn != framesOut )
			return foundation::MakeUnexpected(
			    Fail( "invalid-output", "frame count " + framesOut + ", source " + framesIn ) );
		if ( Text( outVideo[0], "width" ) != Text( video[0], "width" ) ||
		     Text( outVideo[0], "height" ) != Text( video[0], "height" ) )
			return foundation::MakeUnexpected( Fail( "invalid-output", "size changed" ) );
		if ( audioIn != audioOut )
			return foundation::MakeUnexpected( Fail( "invalid-output", "audio tracks differ" ) );
		// Paired by frame index (WebM rounds timestamps); the reference is the Bink decode.
		auto log = Tool( inputs,
		    { "ffmpeg", "-v", "info", "-nostats", "-i", partial.string(), "-i", clip.string(),
		        "-lavfi",
		        "[0:v]settb=1/1000,setpts=N[a];[1:v]settb=1/1000,setpts=N[b];[a][b]ssim=shortest=1",
		        "-f", "null", "-" },
		    true );
		if ( !log )
			return foundation::MakeUnexpected( log.Error() );
		const size_t at = log.Value().find( "All:",
		    log.Value().find( "SSIM " ) == std::string::npos ? 0 : log.Value().find( "SSIM " ) );
		if ( log.Value().find( "SSIM " ) == std::string::npos || at == std::string::npos )
			return foundation::MakeUnexpected( Fail( "invalid-output", "no SSIM result" ) );
		const double ssim = std::stod( log.Value().substr( at + 4 ) );
		if ( ssim < kMinSsim )
		{
			belowGate = true;
			return foundation::MakeUnexpected(
			    Fail( "ssim", "SSIM " + std::to_string( ssim ) + " below 0.97" ) );
		}
		fs::rename( partial, target, ec );
		if ( ec )
			return foundation::MakeUnexpected( Fail( "io", "cannot publish " + target.string() ) );
		Value record = Value::Object();
		record.Set( "frames", Value::Number( framesOut ) );
		record.Set( "ssim", Value::Number( Round5( ssim ) ) );
		record.Set(
		    "bytes", Value::Number( static_cast<long long>( fs::file_size( target, ec ) ) ) );
		record.Set(
		    "source_bytes", Value::Number( static_cast<long long>( fs::file_size( clip, ec ) ) ) );
		record.Set( "audio_tracks", Value::Number( audioOut ) );
		return record;
	}
};

} // namespace

std::unique_ptr<IProductStage> CreateVideoAv1Stage()
{
	return std::make_unique<VideoAv1Stage>();
}

} // namespace product
