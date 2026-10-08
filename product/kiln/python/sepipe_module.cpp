//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: sepipe (RFC 0027): Python bindings over kiln.api. A sepipe.Session
//			  is the default composition and one kiln::Session over a checkout;
//			  its methods are kiln.api requests, and their results are the
//			  same documents `kiln --json` prints (kiln-output/v1), as Python
//			  values. Failures raise sepipe.KilnError("<code>: <detail>").
//			  Python owns no pipeline policy here: every decision is kiln.api's.
//
//=============================================================================//

#include "foundation/json.h"
#include "kiln/api.h"
#include "kiln/composition.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/vector.h>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <tuple>

namespace nb = nanobind;
using foundation::json::Value;

namespace
{

class KilnError : public std::runtime_error
{
public:
	explicit KilnError( const kiln::Error &error ) : std::runtime_error( error.Describe() ) {}
};

template <typename T> T Take( foundation::Expected<T, kiln::Error> result )
{
	if ( !result )
		throw KilnError( result.Error() );
	return std::move( result ).Value();
}

nb::object ToPython( const Value &value )
{
	return nb::module_::import_( "json" ).attr( "loads" )( value.Write() );
}

product::StageRole RoleFor( const std::string &name )
{
	for ( product::StageRole role : { product::StageRole::kEngine, product::StageRole::kContent,
	          product::StageRole::kPackage } )
	{
		if ( product::StageRoleName( role ) == name )
			return role;
	}
	throw KilnError( kiln::Error{ "request", "up_to is engine, content or package, not " + name } );
}

// Diagnostics go to a Python callable (severity, source, message) when one is
// given, else to stderr as the kiln application prints them. Stages may
// report from executor threads, so the callable runs under the GIL.
class Sink final : public product::IDiagnosticSink
{
public:
	explicit Sink( nb::object callback ) : m_Callback( std::move( callback ) ) {}
	void Report(
	    product::Severity severity, std::string_view source, std::string_view message ) override
	{
		const char *level = severity == product::Severity::kError     ? "error"
		                    : severity == product::Severity::kWarning ? "warning"
		                                                              : "info";
		if ( m_Callback.is_none() )
		{
			std::fprintf( stderr, "kiln: [%.*s] %.*s\n", static_cast<int>( source.size() ),
			    source.data(), static_cast<int>( message.size() ), message.data() );
			return;
		}
		nb::gil_scoped_acquire gil;
		m_Callback( level, std::string( source ), std::string( message ) );
	}

private:
	nb::object m_Callback;
};

// A play request from keyword options: map, switches, arguments, flavor,
// display, mounts, device, runtime, log, display_mode=(w, h, hz), cancel,
// exact_arguments, environment={name: value | None}, wrapper,
// started=callable(name, process_id).
// An unknown option is refused by name.
kiln::PlayRequest MakePlay( const std::string &profile, const nb::kwargs &options )
{
	kiln::PlayRequest request;
	request.profile = profile;
	for ( const auto &[key, value] : options )
	{
		const std::string name = nb::cast<std::string>( key );
		if ( value.is_none() )
			continue;
		if ( name == "map" )
			request.map = nb::cast<std::string>( value );
		else if ( name == "switches" )
			request.switches = nb::cast<std::vector<std::string>>( value );
		else if ( name == "arguments" )
			request.arguments = nb::cast<std::vector<std::string>>( value );
		else if ( name == "flavor" )
			request.flavor = nb::cast<std::string>( value );
		else if ( name == "display" )
			request.displaySession = nb::cast<std::string>( value );
		else if ( name == "mounts" )
			request.mountSets = nb::cast<std::vector<std::string>>( value );
		else if ( name == "device" )
			request.device = nb::cast<std::string>( value );
		else if ( name == "runtime" )
			request.runtime =
			    std::filesystem::absolute( nb::cast<std::string>( nb::str( value ) ) );
		else if ( name == "log" )
			request.log = std::filesystem::absolute( nb::cast<std::string>( nb::str( value ) ) );
		else if ( name == "display_mode" )
		{
			const auto mode = nb::cast<std::tuple<int, int, double>>( value );
			request.displayMode = kiln::PlayRequest::DisplayMode{
			    std::get<0>( mode ), std::get<1>( mode ), std::get<2>( mode ) };
		}
		else if ( name == "exact_arguments" )
			request.exactArguments = nb::cast<std::vector<std::string>>( value );
		else if ( name == "wrapper" )
			request.wrapper = nb::cast<std::vector<std::string>>( value );
		else if ( name == "environment" )
		{
			for ( const auto &[variable, setting] : nb::cast<nb::dict>( value ) )
			{
				request.environment.push_back( { nb::cast<std::string>( variable ),
				    setting.is_none() ? std::optional<std::string>()
				                      : std::optional<std::string>(
				                            nb::cast<std::string>( nb::str( setting ) ) ) } );
			}
		}
		else if ( name == "started" )
		{
			// Run threads call back under the GIL.
			nb::object callback = nb::borrow( value );
			request.started = [callback]( const std::string &launch, std::int64_t process )
			{
				nb::gil_scoped_acquire gil;
				callback( launch, process );
			};
		}
		else if ( name == "cancel" )
			request.cancel = nb::cast<product::CancellationFlag *>( value );
		else
			throw KilnError( kiln::Error{ "request", "unknown option " + name } );
	}
	return request;
}

class Session
{
public:
	Session( const std::string &root, nb::object diagnostics )
	    : m_Composition( Take( kiln::ComposeDefault() ) ), m_Sink( std::move( diagnostics ) ),
	      m_Session( m_Composition.catalog, *m_Composition.processes, *m_Composition.executor,
	          m_Sink, kiln::DefaultSessionConfig( root, m_Composition.hostTag ) )
	{
	}

	nb::object Profiles() const { return ToPython( kiln::ToJson( m_Session.ListProfiles() ) ); }

	nb::object Resolve( const std::string &profile ) const
	{
		return ToPython( Take( m_Session.ResolveProfile( profile ) ).document );
	}

	nb::object Switches( const std::string &profile ) const
	{
		Value list = Value::Array();
		for ( const auto &entry : Take( m_Session.Switches( profile ) ) )
		{
			Value item = Value::Object();
			item.Set( "name", Value::String( entry.name ) );
			item.Set( "description", Value::String( entry.description ) );
			list.Push( std::move( item ) );
		}
		return ToPython( list );
	}

	nb::object Doctor( const std::string &profile )
	{
		return ToPython( kiln::ToJson( Take( m_Session.Doctor( profile ) ) ) );
	}

	nb::object Build( const std::string &profile, std::optional<std::string> flavor,
	    const std::string &upTo, std::vector<std::string> mounts,
	    std::optional<std::string> runtime )
	{
		kiln::PipelineRequest request;
		request.profile = profile;
		request.flavor = std::move( flavor );
		request.upTo = RoleFor( upTo );
		request.mountSets = std::move( mounts );
		if ( runtime )
			request.runtime = std::filesystem::absolute( *runtime );
		foundation::Expected<kiln::PipelineResult, kiln::Error> result =
		    foundation::MakeUnexpected( kiln::Error{} );
		{
			nb::gil_scoped_release release;
			result = m_Session.Run( request );
		}
		return ToPython( kiln::ToJson( Take( std::move( result ) ) ) );
	}

	nb::object Plan( const kiln::PlayRequest &request ) const
	{
		return ToPython( kiln::ToJson( Take( m_Session.PlanLaunch( request ) ) ) );
	}

	// `play` builds through the package stage first (Session::BuildForPlay);
	// `run` launches what is already packaged. Both wait for the program
	// under its display session and run provider and return its status.
	int Launch( kiln::PlayRequest request, bool build )
	{
		nb::gil_scoped_release release;
		if ( build )
			Take( m_Session.BuildForPlay( request ) );
		return Take( m_Session.Launch( request, *m_Composition.spawner ) );
	}

private:
	kiln::DefaultComposition m_Composition;
	Sink m_Sink;
	kiln::Session m_Session;
};

} // namespace

NB_MODULE( sepipe, m )
{
	m.doc() = "Python bindings over kiln.api (RFC 0027). Results are kiln-output/v1 documents.";
	m.attr( "CONTRACT" ) = std::string( kiln::kContractName );
	m.attr( "SCHEMA" ) = std::string( kiln::kJsonSchema );
	nb::exception<KilnError>( m, "KilnError" );

	nb::class_<Session> session( m, "Session" );
	session
	    .def( nb::init<const std::string &, nb::object>(), nb::arg( "root" ),
	        nb::arg( "diagnostics" ) = nb::none(),
	        "A session over the checkout at root; diagnostics(severity, source, message) "
	        "receives stage reports (stderr when None)." )
	    .def( "profiles", &Session::Profiles )
	    .def( "resolve", &Session::Resolve, nb::arg( "profile" ) )
	    .def( "switches", &Session::Switches, nb::arg( "profile" ) )
	    .def( "doctor", &Session::Doctor, nb::arg( "profile" ) )
	    .def( "build", &Session::Build, nb::arg( "profile" ), nb::arg( "flavor" ) = nb::none(),
	        nb::arg( "up_to" ) = "engine", nb::arg( "mounts" ) = std::vector<std::string>{},
	        nb::arg( "runtime" ) = nb::none() );

	// plan, play and run take the same play request (MakePlay's options).
	nb::class_<product::CancellationFlag>( m, "Cancellation",
	    "A cancellation token for play and run; cancel() from any thread stops the run's "
	    "programs. Keep it alive until the run returns." )
	    .def( nb::init<>() )
	    .def( "cancel", &product::CancellationFlag::Cancel )
	    .def_prop_ro( "cancelled", &product::CancellationFlag::IsCancelled );
	session
	    .def(
	        "plan",
	        []( Session &self, const std::string &profile, const nb::kwargs &options )
	        {
		        return self.Plan( MakePlay( profile, options ) );
	        },
	        nb::arg( "profile" ), nb::arg( "options" ),
	        "The launch plan of a play or run request (kiln play --dry-run)." )
	    .def(
	        "play",
	        []( Session &self, const std::string &profile, const nb::kwargs &options )
	        {
		        return self.Launch( MakePlay( profile, options ), true );
	        },
	        nb::arg( "profile" ), nb::arg( "options" ),
	        "Build through the package stage, launch, and return the exit status (kiln play)." )
	    .def(
	        "run",
	        []( Session &self, const std::string &profile, const nb::kwargs &options )
	        {
		        return self.Launch( MakePlay( profile, options ), false );
	        },
	        nb::arg( "profile" ), nb::arg( "options" ),
	        "Launch what is packaged and return the exit status (kiln run)." );
}
