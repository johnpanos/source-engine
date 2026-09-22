//========= Copyright Valve Corporation, All rights reserved. ============//
#include "platform/composition.h"
#include "platform/legacy_binding.h"
#include "testing/runner.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

namespace
{
using namespace platform;
using foundation::Expected;
using foundation::MakeUnexpected;

int checks = 0;
int failures = 0;
void Check( bool value, const char *message )
{
	++checks;
	if ( !value )
	{
		++failures;
		std::fprintf( stderr, "FAIL composition: %s\n", message );
	}
}

struct ISource
{
	static constexpr std::string_view kCapabilityName = "source";
	virtual ~ISource() = default;
	virtual void Borrow( int delta ) = 0;
};
struct IConsumer
{
	static constexpr std::string_view kCapabilityName = "consumer";
};
struct IOptional
{
	static constexpr std::string_view kCapabilityName = "optional";
};
struct IUndeclared
{
	static constexpr std::string_view kCapabilityName = "undeclared";
};

enum class Fault
{
	None,
	Borrow,
	Task,
	Subscription,
	Bridge
};
struct Fixture
{
	std::vector<std::string> trace;
	std::string fail;
	int live = 0, borrowers = 0, tasks = 0, subscriptions = 0, violations = 0;
	ISource *legacy = nullptr;
	bool sourceAlive = false;
	Fault fault = Fault::None;
	ApplicationComposition *root = nullptr;
	void Event( const std::string &event ) { trace.push_back( event ); }
	bool Clean() const
	{
		return live == 0 && borrowers == 0 && tasks == 0 && subscriptions == 0 && violations == 0 &&
		       legacy == nullptr && !sourceAlive;
	}
};

class Source final : public IProviderLifecycle, public ISource
{
public:
	explicit Source( Fixture &f ) : f( f )
	{
		++f.live;
		f.sourceAlive = true;
		f.Event( "source.construct" );
	}
	~Source() override
	{
		if ( f.borrowers || f.tasks || f.subscriptions || f.legacy )
			++f.violations;
		--f.live;
		f.sourceAlive = false;
		f.Event( "source.destroy" );
	}
	void Borrow( int delta ) override { f.borrowers += delta; }
	Expected<void, ProviderError> Connect() override
	{
		f.Event( "source.connect" );
		if ( f.fail == "source.connect" )
			return MakeUnexpected( ProviderError{ ProviderErrorCode::ConnectFailed, "injected" } );
		return {};
	}
	Expected<void, ProviderError> Initialize() override
	{
		f.Event( "source.init" );
		if ( f.fail == "source.init" )
			return MakeUnexpected(
			    ProviderError{ ProviderErrorCode::InitializationFailed, "injected" } );
		return {};
	}
	void Shutdown() noexcept override { f.Event( "source.shutdown" ); }
	void Disconnect() noexcept override { f.Event( "source.disconnect" ); }

private:
	Fixture &f;
};

class Consumer final : public IProviderLifecycle, public IConsumer
{
public:
	Consumer( Fixture &f, ISource &source ) : f( f ), source( source )
	{
		++f.live;
		source.Borrow( 1 );
		f.Event( "consumer.construct" );
	}
	~Consumer() override
	{
		if ( !f.sourceAlive )
			++f.violations;
		if ( f.fault != Fault::Borrow )
			source.Borrow( -1 );
		--f.live;
		f.Event( "consumer.destroy" );
	}
	Expected<void, ProviderError> Connect() override
	{
		f.Event( "consumer.connect" );
		++f.subscriptions;
		if ( f.root )
		{
			Check( !f.root->Find<ISource>(), "capabilities hidden until start completes" );
			Check( !f.root->Start(), "reentrant start rejected" );
			Check( !f.root->Stop(), "reentrant stop rejected" );
		}
		auto bound = binding.Bind( f.legacy, source );
		if ( !bound )
			return MakeUnexpected(
			    ProviderError{ ProviderErrorCode::ConnectFailed, "occupied bridge" } );
		if ( f.fail == "consumer.connect" )
			return MakeUnexpected( ProviderError{ ProviderErrorCode::ConnectFailed, "injected" } );
		return {};
	}
	Expected<void, ProviderError> Initialize() override
	{
		f.Event( "consumer.init" );
		++f.tasks;
		if ( f.legacy != &source )
			++f.violations;
		if ( f.fail == "consumer.init" )
			return MakeUnexpected(
			    ProviderError{ ProviderErrorCode::InitializationFailed, "injected" } );
		return {};
	}
	void Shutdown() noexcept override
	{
		f.Event( "consumer.shutdown" );
		if ( f.legacy != &source )
			++f.violations;
		if ( f.fault != Fault::Task )
			--f.tasks;
	}
	void Disconnect() noexcept override
	{
		f.Event( "consumer.disconnect" );
		if ( f.fault != Fault::Subscription )
			--f.subscriptions;
		binding.Reset();
		if ( f.fault == Fault::Bridge )
			f.legacy = &source;
	}

private:
	Fixture &f;
	ISource &source;
	ScopedLegacyBinding<ISource> binding;
};

ProviderDescriptor SourceDescriptor( Fixture &f, std::vector<DependencyDescriptor> deps = {} )
{
	return ProviderDescriptor::Define<Source, ISource>( "source-provider", std::move( deps ),
	    [&f]( const DependencyView & ) -> Expected<std::unique_ptr<Source>, ProviderError>
	    {
		    if ( f.fail == "source.construct" )
			    return MakeUnexpected(
			        ProviderError{ ProviderErrorCode::ConstructionFailed, "injected" } );
		    return std::make_unique<Source>( f );
	    } );
}

ProviderDescriptor ConsumerDescriptor( Fixture &f )
{
	return ProviderDescriptor::Define<Consumer, IConsumer>( "consumer-provider",
	    { DependencyDescriptor::Required<ISource>(), DependencyDescriptor::Optional<IOptional>() },
	    [&f]( const DependencyView &view ) -> Expected<std::unique_ptr<Consumer>, ProviderError>
	    {
		    Check( view.Get<ISource>() != nullptr, "required capability injected" );
		    Check( view.Get<IOptional>() == nullptr, "optional absence is explicit" );
		    Check( view.Get<IUndeclared>() == nullptr, "undeclared capabilities inaccessible" );
		    if ( f.fail == "consumer.construct" )
			    return MakeUnexpected(
			        ProviderError{ ProviderErrorCode::ConstructionFailed, "injected" } );
		    return std::make_unique<Consumer>( f, *view.Get<ISource>() );
	    } );
}

void Populate( ApplicationComposition &root, Fixture &f )
{
	Check( !!root.AddProvider( ConsumerDescriptor( f ) ), "register consumer first" );
	Check( !!root.AddProvider( SourceDescriptor( f ) ), "register dependency second" );
}

// The oracle observes effects and resources, not the kernel's internal graph.
bool LifecycleOracle( Fault fault, const std::string &fail, bool reentrant = false )
{
	Fixture f;
	f.fail = fail;
	f.fault = fault;
	std::vector<std::string> expected;
	const bool sourceMade = fail != "source.construct";
	const bool consumerMade = sourceMade && fail != "consumer.construct";
	const bool sourceConnected = consumerMade;
	const bool consumerConnected = sourceConnected && fail != "source.connect";
	const bool sourceInitialized = consumerConnected && fail != "consumer.connect";
	const bool consumerInitialized = sourceInitialized && fail != "source.init";
	if ( sourceMade )
		expected.push_back( "source.construct" );
	if ( consumerMade )
		expected.push_back( "consumer.construct" );
	if ( sourceConnected )
		expected.push_back( "source.connect" );
	if ( consumerConnected )
		expected.push_back( "consumer.connect" );
	if ( sourceInitialized )
		expected.push_back( "source.init" );
	if ( consumerInitialized )
		expected.push_back( "consumer.init" );
	if ( consumerInitialized )
		expected.push_back( "consumer.shutdown" );
	if ( sourceInitialized )
		expected.push_back( "source.shutdown" );
	if ( consumerConnected )
		expected.push_back( "consumer.disconnect" );
	if ( sourceConnected )
		expected.push_back( "source.disconnect" );
	if ( consumerMade )
		expected.push_back( "consumer.destroy" );
	if ( sourceMade )
		expected.push_back( "source.destroy" );
	{
		ApplicationComposition root;
		if ( reentrant )
			f.root = &root;
		Populate( root, f );
		auto result = root.Start();
		if ( !!result != fail.empty() )
			return false;
		if ( result )
		{
			Check( root.Find<ISource>() && root.Find<IConsumer>(),
			    "root borrows running capabilities" );
			Check( !root.Start(), "double start rejected" );
			Check( !root.AddProvider( SourceDescriptor( f ) ), "running topology frozen" );
		}
		else
		{
			Check( !root.IsRunning() && !root.Find<ISource>(), "failed startup is unpublished" );
			Check( result.Error().detail == "injected", "failure preserves provider diagnostic" );
		}
	}
	return f.Clean() && f.trace == expected;
}

void ValidateBeforeEffects()
{
	Fixture f;
	ApplicationComposition missing;
	Check( !!missing.AddProvider( ConsumerDescriptor( f ) ), "register missing consumer" );
	auto absent = missing.Start();
	Check( !absent && absent.Error().code == CompositionErrorCode::MissingRequiredCapability &&
	           absent.Error().provider == "consumer-provider" &&
	           absent.Error().capability == "source",
	    "missing required error names consumer and capability" );
	Check( f.trace.empty(), "missing requirement causes no construction" );
	ApplicationComposition cycle;
	Check( !!cycle.AddProvider( ConsumerDescriptor( f ) ), "register cycle consumer" );
	Check( !!cycle.AddProvider(
	           SourceDescriptor( f, { DependencyDescriptor::Required<IConsumer>() } ) ),
	    "register cycle source" );
	auto cyclic = cycle.Start();
	Check(
	    !cyclic && cyclic.Error().code == CompositionErrorCode::DependencyCycle, "cycle rejected" );
	Check( f.trace.empty(), "cycle causes no construction" );
	ApplicationComposition duplicate;
	Check( !!duplicate.AddProvider( SourceDescriptor( f ) ), "register source" );
	Check( !!duplicate.AddProvider( SourceDescriptor( f ) ), "register duplicate" );
	Check( !duplicate.Start() && f.trace.empty(), "duplicate rejected before effects" );
}

void RetryAndIsolation()
{
	Fixture f;
	ApplicationComposition root;
	Populate( root, f );
	for ( const char *failure : { "source.construct", "consumer.construct", "source.connect",
	          "consumer.connect", "source.init", "consumer.init" } )
	{
		f.fail = failure;
		Check( !root.Start() && f.Clean(), "failure releases every resource before returning" );
		f.fail.clear();
		Check( !!root.Start(), "retry after failed startup" );
		Fixture second;
		{
			ApplicationComposition independent;
			Populate( independent, second );
			Check( !!independent.Start(), "second independent composition" );
			Check( independent.Find<ISource>() != root.Find<ISource>(), "separate authority" );
		}
		Check( second.Clean() && root.IsRunning(), "second teardown leaves first running" );
		Check( !!root.Stop() && !!root.Stop() && f.Clean(), "stop is idempotent and drained" );
	}
}

class Probe final : public IProviderLifecycle
{
public:
	Expected<void, ProviderError> Connect() override { return {}; }
	Expected<void, ProviderError> Initialize() override { return {}; }
	void Shutdown() noexcept override {}
	void Disconnect() noexcept override {}
};

void EdgeCases()
{
	Fixture f;
	{
		ApplicationComposition root;
		auto optional = ProviderDescriptor::Define<Probe>( "optional-consumer",
		    { DependencyDescriptor::Optional<ISource>() },
		    [&f]( const DependencyView &deps ) -> Expected<std::unique_ptr<Probe>, ProviderError>
		    {
			    Check( deps.Get<ISource>() != nullptr && f.sourceAlive,
			        "present optional creates dependency edge" );
			    return std::make_unique<Probe>();
		    } );
		Check( !!root.AddProvider( std::move( optional ) ), "register optional consumer" );
		Check( !!root.AddProvider( SourceDescriptor( f ) ), "register optional provider" );
		Check( !!root.Start(), "present optional provider starts first" );
	}
	Check( f.Clean(), "optional lifecycle clean" );
	for ( const auto &deps :
	    { std::vector<DependencyDescriptor>{ DependencyDescriptor::Required<ISource>() },
	        std::vector<DependencyDescriptor>{ DependencyDescriptor::Optional<ISource>() },
	        std::vector<DependencyDescriptor>{ DependencyDescriptor::Required<IOptional>(),
	            DependencyDescriptor::Optional<IOptional>() } } )
	{
		ApplicationComposition root;
		Check( !!root.AddProvider( SourceDescriptor( f, deps ) ), "register invalid dependencies" );
		Check( !root.Start() && f.Clean(), "self-cycle and duplicate requirements rejected" );
	}
	{
		ApplicationComposition root;
		Check(
		    !!root.AddProvider( ProviderDescriptor::Define<Source, ISource, ISource>( "twice", {},
		        [&f]( const DependencyView & ) -> Expected<std::unique_ptr<Source>, ProviderError>
		        {
			        return std::make_unique<Source>( f );
		        } ) ),
		    "register duplicate export" );
		auto result = root.Start();
		Check( !result && result.Error().code == CompositionErrorCode::DuplicateCapability &&
		           f.Clean(),
		    "one provider cannot export same capability twice" );
	}
	CompositionError durable;
	{
		ApplicationComposition root;
		Check( !!root.AddProvider( ProviderDescriptor::Define<Probe>( std::string( 100, 'p' ), {},
		           []( const DependencyView & ) -> Expected<std::unique_ptr<Probe>, ProviderError>
		           {
			           return MakeUnexpected( ProviderError{
			               ProviderErrorCode::ConstructionFailed, std::string( 100, 'e' ) } );
		           } ) ),
		    "register owning diagnostic" );
		auto result = root.Start();
		Check( !result, "factory error returned" );
		durable = result.Error();
	}
	Check( durable.provider == std::string( 100, 'p' ) && durable.detail == std::string( 100, 'e' ),
	    "diagnostics survive root destruction" );
	{
		ApplicationComposition root;
		Check( !!root.AddProvider( ProviderDescriptor::Define<Probe>( "null", {},
		           []( const DependencyView & ) -> Expected<std::unique_ptr<Probe>, ProviderError>
		           {
			           return std::unique_ptr<Probe>();
		           } ) ),
		    "register null factory" );
		auto result = root.Start();
		Check( !result && result.Error().code == CompositionErrorCode::ConstructionFailed,
		    "null success is failure" );
	}
	{
		Source source( f );
		ScopedLegacyBinding<ISource> first, second;
		Check( !!first.Bind( f.legacy, source ), "domain bridge publishes capability" );
		Check( !second.Bind( f.legacy, source ) && f.legacy == &source,
		    "occupied slot fails without mutation" );
		Check( !first.Bind( f.legacy, source ), "same binding cannot reacquire" );
		first.Reset();
		Check( !f.legacy && !!second.Bind( f.legacy, source ),
		    "bridge can be reacquired after drain" );
	}
	Check( f.Clean(), "scoped bridge clears before source destruction" );
	// Independent roots with ready providers must preserve registration order.
	for ( int reverse = 0; reverse != 2; ++reverse )
	{
		std::vector<int> constructed;
		ApplicationComposition root;
		for ( int index : { reverse, 1 - reverse } )
			Check(
			    !!root.AddProvider( ProviderDescriptor::Define<Probe>( std::to_string( index ), {},
			        [&, index](
			            const DependencyView & ) -> Expected<std::unique_ptr<Probe>, ProviderError>
			        {
				        constructed.push_back( index );
				        return std::make_unique<Probe>();
			        } ) ),
			    "register independent provider" );
		Check( !!root.Start() && constructed == std::vector<int>{ reverse, 1 - reverse },
		    "stable registration-order tie break" );
	}
}

class TaskBorrower final : public IProviderLifecycle
{
public:
	TaskBorrower( ISource &source, std::atomic<int> &completed )
	    : source( source ), completed( completed )
	{
	}
	~TaskBorrower() override
	{
		if ( worker.joinable() )
			std::terminate();
	}
	Expected<void, ProviderError> Connect() override { return {}; }
	Expected<void, ProviderError> Initialize() override
	{
		source.Borrow( 1 );
		worker = std::thread(
		    [this]
		    {
			    // Release/acquire publishes the cancellation request. Joining below
			    // acknowledges completion before the provider can release this borrow.
			    while ( !cancel.load( std::memory_order_acquire ) )
				    std::this_thread::yield();
			    source.Borrow( -1 );
			    completed.fetch_add( 1 );
		    } );
		return {};
	}
	void Shutdown() noexcept override
	{
		cancel.store( true, std::memory_order_release );
		worker.join();
	}
	void Disconnect() noexcept override {}

private:
	ISource &source;
	std::atomic<int> &completed;
	std::atomic<bool> cancel{ false };
	std::thread worker;
};

void NativeDrain()
{
	Fixture f;
	std::atomic<int> completed{ 0 };
	{
		ApplicationComposition root;
		Check( !!root.AddProvider( ProviderDescriptor::Define<TaskBorrower>( "pending-task",
		           { DependencyDescriptor::Required<ISource>() },
		           [&]( const DependencyView &deps )
		               -> Expected<std::unique_ptr<TaskBorrower>, ProviderError>
		           {
			           return std::make_unique<TaskBorrower>( *deps.Get<ISource>(), completed );
		           } ) ),
		    "register pending work provider" );
		Check( !!root.AddProvider( SourceDescriptor( f ) ) && !!root.Start(),
		    "start real outstanding work" );
		Check( !!root.Stop(), "stop waits for cancellation acknowledgment" );
	}
	Check( completed.load() == 1 && f.Clean(),
	    "native task releases borrow before provider destruction" );
}
}

testing::TestResult RunCompositionConformance()
{
	checks = 0;
	failures = 0;
	ValidateBeforeEffects();
	EdgeCases();
	NativeDrain();
	RetryAndIsolation();
	Check(
	    LifecycleOracle( Fault::None, "", true ), "ordered successful lifecycle and reentrancy" );
	for ( const char *failure : { "source.construct", "consumer.construct", "source.connect",
	          "consumer.connect", "source.init", "consumer.init" } )
		Check( LifecycleOracle( Fault::None, failure ), "ordered failure rollback" );
	for ( Fault fault : { Fault::Borrow, Fault::Task, Fault::Subscription, Fault::Bridge } )
		Check( !LifecycleOracle( fault, "" ), "same oracle rejects broken provider" );
	return { static_cast<std::size_t>( checks ), static_cast<std::size_t>( failures ), 0 };
}

#ifndef SOURCE_CONFORMANCE_LINKED
int main()
{
	const auto result = RunCompositionConformance();
	std::printf( "CONFORMANCE %zu %zu\n", result.checks, result.failures );
	return result.Passed() ? 0 : 1;
}
#endif
