//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for platform.window.v1 (RFC 0001 rank 7,
//			roadmap R14, Q-FOUNDATION). Runs the shared window/input suite
//			against the headless provider, composed through the R06 kernel:
//			required and optional capabilities, optional absence, startup
//			failure rollback, repeat instances and shutdown with live windows.
//
//			Certifies contract semantics via a fake; NOT evidence of native
//			window behavior (the SDL2 provider runs the same suite natively).
//
//			Build/run: tools/quality/conformance.py check --suite platform.window
//
//=============================================================================//

#include "fake_window_system.h"
#include "platform/composition.h"
#include "testing/conformance_result.h"
#include "window_conformance.h"

#include <cstdio>
#include <memory>

namespace
{

using namespace platformtest;
using foundation::Expected;
using platform::ApplicationComposition;
using platform::DependencyDescriptor;
using platform::DependencyView;
using platform::ProviderDescriptor;
using platform::ProviderError;

int g_Checks = 0;
int g_Failures = 0;

void Check( const char *name, bool ok )
{
	++g_Checks;
	if ( !ok )
	{
		++g_Failures;
		std::printf( "FAIL %s\n", name );
	}
}

void Tally( const char *variant, const WindowReport &report, int allowedSkips )
{
	g_Checks += report.checks;
	g_Failures += report.failures;
	if ( report.skipped > allowedSkips )
	{
		++g_Failures;
		std::printf( "FAIL %s: %d unexpected skip(s)\n", variant, report.skipped - allowedSkips );
	}
	std::printf( "%s %s: %d checks, %d failures, %d skips\n", report.failures ? "FAIL" : "ok",
	    variant, report.checks, report.failures, report.skipped );
}

// The consumer a product root would compose: required window system and event
// source, every other capability optional. It keeps the references its factory
// received and nothing else.
class WindowConsumer final : public platform::IProviderLifecycle
{
public:
	struct IRole
	{
		static constexpr std::string_view kCapabilityName = "test.window-consumer";
		virtual ~IRole() = default;
		virtual const WindowCapabilities &Capabilities() const = 0;
	};

	class Provider final : public platform::IProviderLifecycle, public IRole
	{
	public:
		explicit Provider( const DependencyView &deps )
		{
			m_Caps.windows = deps.Get<IWindowSystem>();
			m_Caps.events = deps.Get<IEventSource>();
			m_Caps.cursor = deps.Get<ICursor>();
			m_Caps.clipboard = deps.Get<IClipboard>();
			m_Caps.messageBox = deps.Get<IMessageBox>();
			m_Caps.gamepads = deps.Get<IGamepads>();
		}
		const WindowCapabilities &Capabilities() const override { return m_Caps; }
		Expected<void, ProviderError> Connect() override { return {}; }
		Expected<void, ProviderError> Initialize() override { return {}; }
		void Shutdown() noexcept override {}
		void Disconnect() noexcept override {}

	private:
		WindowCapabilities m_Caps;
	};

	static ProviderDescriptor Describe()
	{
		return ProviderDescriptor::Define<Provider, IRole>( "window-consumer",
		    { DependencyDescriptor::Required<IWindowSystem>(),
		        DependencyDescriptor::Required<IEventSource>(),
		        DependencyDescriptor::Optional<ICursor>(), DependencyDescriptor::Optional<IClipboard>(),
		        DependencyDescriptor::Optional<IMessageBox>(),
		        DependencyDescriptor::Optional<IGamepads>() },
		    []( const DependencyView &deps ) -> Expected<std::unique_ptr<Provider>, ProviderError>
		    { return std::make_unique<Provider>( deps ); } );
	}

	Expected<void, ProviderError> Connect() override { return {}; }
	Expected<void, ProviderError> Initialize() override { return {}; }
	void Shutdown() noexcept override {}
	void Disconnect() noexcept override {}
};

// Records the provider instance each start constructs, so the test can observe
// lifecycle effects without the composition exposing its internals.
struct Observed
{
	FakeWindowSystem *system = nullptr;
	int constructed = 0;
};

template <typename... Exports>
ProviderDescriptor DescribeFake( FakeWindowConfig config, Observed &observed )
{
	return ProviderDescriptor::Define<FakeWindowSystem, Exports...>( "headless-window-system", {},
	    [config, &observed](
	        const DependencyView & ) -> Expected<std::unique_ptr<FakeWindowSystem>, ProviderError>
	    {
		    auto system = std::make_unique<FakeWindowSystem>( config );
		    observed.system = system.get();
		    ++observed.constructed;
		    return system;
	    } );
}

ProviderDescriptor DescribeFull( FakeWindowConfig config, Observed &observed )
{
	return DescribeFake<IWindowSystem, IEventSource, ICursor, IClipboard, IMessageBox, IGamepads>(
	    config, observed );
}

// Runs the suite with every capability exported, composed through the kernel.
void RunComposed( const char *variant, FakeWindowConfig config )
{
	Observed observed;
	ApplicationComposition root;
	Check( "compose.add_consumer", !!root.AddProvider( WindowConsumer::Describe() ) );
	Check( "compose.add_provider", !!root.AddProvider( DescribeFull( config, observed ) ) );
	Check( "compose.start", !!root.Start() );
	const WindowConsumer::IRole *consumer = root.Find<WindowConsumer::IRole>();
	Check( "compose.consumer_published", consumer != nullptr );
	if ( !consumer || !observed.system )
		return;
	const WindowCapabilities &caps = consumer->Capabilities();
	Check( "compose.optional_present", caps.cursor && caps.clipboard && caps.messageBox && caps.gamepads );
	FakeWindowDriver driver( *observed.system );
	WindowSuiteOptions options;
	options.doubleClickMs = config.policy.doubleClickMs;
	options.doubleClickDistance = config.policy.doubleClickDistance;
	Tally( variant, RunWindowConformance( caps, driver, options ), 0 );
	Check( "compose.stop", !!root.Stop() );
	Check( "compose.lifecycle_balanced", observed.constructed == 1 );
}

void OptionalAbsence()
{
	Observed observed;
	ApplicationComposition root;
	Check( "optional.add_consumer", !!root.AddProvider( WindowConsumer::Describe() ) );
	Check( "optional.add_provider",
	    !!root.AddProvider( DescribeFake<IWindowSystem, IEventSource>(
	        FakeWindowConfig{}, observed ) ) );
	Check( "optional.start", !!root.Start() );
	const WindowConsumer::IRole *consumer = root.Find<WindowConsumer::IRole>();
	if ( !consumer )
	{
		Check( "optional.consumer_published", false );
		return;
	}
	const WindowCapabilities &caps = consumer->Capabilities();
	Check( "optional.absent_is_null",
	    !caps.cursor && !caps.clipboard && !caps.messageBox && !caps.gamepads );
	Check( "optional.not_published", root.Find<IClipboard>() == nullptr && root.Find<ICursor>() == nullptr );
	FakeWindowDriver driver( *observed.system );
	// Cursor, clipboard and message-box checks are skipped because the
	// capabilities are genuinely absent, not stubbed.
	Tally( "platform.window[headless, required only]", RunWindowConformance( caps, driver ), 3 );
	Check( "optional.stop", !!root.Stop() );
}

void MissingRequired()
{
	ApplicationComposition root;
	Check( "required.add_consumer", !!root.AddProvider( WindowConsumer::Describe() ) );
	auto started = root.Start();
	Check( "required.missing_fails",
	    !started && started.Error().code == platform::CompositionErrorCode::MissingRequiredCapability );
}

void StartupFailures()
{
	for ( int stage = 0; stage < 2; ++stage )
	{
		Observed observed;
		FakeLifecycleProbe probe;
		FakeWindowConfig config;
		config.defects.failConnect = stage == 0;
		config.defects.failInitialize = stage == 1;
		config.probe = &probe;
		{
			ApplicationComposition root;
			Check( "failure.add_consumer", !!root.AddProvider( WindowConsumer::Describe() ) );
			Check( "failure.add_provider", !!root.AddProvider( DescribeFull( config, observed ) ) );
			auto started = root.Start();
			const platform::CompositionErrorCode expected =
			    stage == 0 ? platform::CompositionErrorCode::ConnectFailed
			               : platform::CompositionErrorCode::InitializationFailed;
			Check( stage == 0 ? "failure.connect_reported" : "failure.initialize_reported",
			    !started && started.Error().code == expected );
			Check( "failure.unpublished", !root.IsRunning() && root.Find<IWindowSystem>() == nullptr &&
			                                  root.Find<WindowConsumer::IRole>() == nullptr );
		}
		// Rollback runs Shutdown for the failed initializer and Disconnect for the
		// failed connector, then destroys the provider.
		if ( stage == 0 )
			Check( "failure.connect_rollback", probe.connects == 1 && probe.initializes == 0 &&
			                                       probe.shutdowns == 0 && probe.disconnects == 1 );
		else
			Check( "failure.initialize_rollback", probe.connects == 1 && probe.initializes == 1 &&
			                                          probe.shutdowns == 1 && probe.disconnects == 1 );
		Check( "failure.constructed_once", observed.constructed == 1 );
	}
}

// A second start after stop constructs a fresh provider; a window left open at
// Stop is destroyed and its presentation told before the provider goes away.
void RepeatAndShutdown()
{
	Observed observed;
	ApplicationComposition root;
	Check( "repeat.add_consumer", !!root.AddProvider( WindowConsumer::Describe() ) );
	Check( "repeat.add_provider", !!root.AddProvider( DescribeFull( FakeWindowConfig{}, observed ) ) );
	FakeWindowSystem *first = nullptr;
	for ( int run = 0; run < 2; ++run )
	{
		Check( "repeat.start", !!root.Start() );
		IWindowSystem *windows = root.Find<IWindowSystem>();
		IEventSource *events = root.Find<IEventSource>();
		Check( "repeat.published", windows && events );
		if ( !windows || !events )
			return;
		if ( run == 0 )
			first = observed.system;
		else
			Check( "repeat.fresh_provider", observed.constructed == 2 );

		WindowDesc desc;
		desc.title = "left open";
		desc.width = 16;
		desc.height = 16;
		auto window = windows->Create( desc );
		Check( "repeat.ids_restart", window && window.Value().value == 1 );
		render::IRenderSurface *surface = window ? windows->Surface( window.Value() ) : nullptr;
		struct Listener final : render::IRenderSurfaceListener
		{
			void OnNativeSurfaceReleasing() override { ++calls; }
			int calls = 0;
		} listener;
		Check( "shutdown.attach", surface && surface->AttachListener( listener ) );
		Check( "shutdown.connected", observed.system->IsConnected() );
		Check( "repeat.stop", !!root.Stop() );
		Check( "shutdown.presentation_told", listener.calls == 1 );
		Check( "shutdown.unpublished", root.Find<IWindowSystem>() == nullptr );
	}
	Check( "repeat.two_instances", observed.constructed == 2 && first != nullptr );
}

} // namespace

int main()
{
	RunComposed( "platform.window[headless]", FakeWindowConfig{} );

	FakeWindowConfig hidpi;
	hidpi.pixelScale = 2;
	RunComposed( "platform.window[headless, 2x pixels]", hidpi );

	FakeWindowConfig policy;
	policy.policy.doubleClickMs = 150;
	policy.policy.doubleClickDistance = 6;
	RunComposed( "platform.window[headless, 150 ms / 6 px double click]", policy );

	OptionalAbsence();
	MissingRequired();
	StartupFailures();
	RepeatAndShutdown();

	std::printf( "%s test_window: %d checks, %d failures\n", g_Failures ? "FAIL" : "ok", g_Checks,
	    g_Failures );
	return testing::ReportConformance( g_Checks, g_Failures );
}
