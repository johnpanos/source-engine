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
#include <nanobind/stl/vector.h>

#include <cstdio>
#include <memory>
#include <stdexcept>

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

kiln::PlayRequest MakePlay( const std::string &profile, std::optional<std::string> map,
    std::vector<std::string> switches, std::vector<std::string> arguments,
    std::optional<std::string> flavor, std::optional<std::string> display,
    std::vector<std::string> mounts, std::optional<std::string> device )
{
	kiln::PlayRequest request;
	request.profile = profile;
	request.map = std::move( map );
	request.switches = std::move( switches );
	request.arguments = std::move( arguments );
	request.flavor = std::move( flavor );
	request.displaySession = std::move( display );
	request.mountSets = std::move( mounts );
	request.device = std::move( device );
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
	    const std::string &upTo, std::vector<std::string> mounts )
	{
		kiln::PipelineRequest request;
		request.profile = profile;
		request.flavor = std::move( flavor );
		request.upTo = RoleFor( upTo );
		request.mountSets = std::move( mounts );
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
	        nb::arg( "up_to" ) = "engine", nb::arg( "mounts" ) = std::vector<std::string>{} );

	// plan, play and run take the same play request.
	const auto request = []( auto method )
	{
		return [method]( Session &self, const std::string &profile, std::optional<std::string> map,
		           std::vector<std::string> switches, std::vector<std::string> arguments,
		           std::optional<std::string> flavor, std::optional<std::string> display,
		           std::vector<std::string> mounts, std::optional<std::string> device )
		{
			return method(
			    self, MakePlay( profile, std::move( map ), std::move( switches ),
			              std::move( arguments ), std::move( flavor ), std::move( display ),
			              std::move( mounts ), std::move( device ) ) );
		};
	};
	const auto define = [&]( const char *name, auto method, const char *doc )
	{
		session.def( name, request( method ), nb::arg( "profile" ), nb::arg( "map" ) = nb::none(),
		    nb::arg( "switches" ) = std::vector<std::string>{},
		    nb::arg( "arguments" ) = std::vector<std::string>{}, nb::arg( "flavor" ) = nb::none(),
		    nb::arg( "display" ) = nb::none(), nb::arg( "mounts" ) = std::vector<std::string>{},
		    nb::arg( "device" ) = nb::none(), doc );
	};
	define(
	    "plan",
	    []( Session &self, const kiln::PlayRequest &play )
	    {
		    return self.Plan( play );
	    },
	    "The launch plan of a play or run request (kiln play --dry-run)." );
	define(
	    "play",
	    []( Session &self, const kiln::PlayRequest &play )
	    {
		    return self.Launch( play, true );
	    },
	    "Build through the package stage, launch, and return the exit status (kiln play)." );
	define(
	    "run",
	    []( Session &self, const kiln::PlayRequest &play )
	    {
		    return self.Launch( play, false );
	    },
	    "Launch what is packaged and return the exit status (kiln run)." );
}
