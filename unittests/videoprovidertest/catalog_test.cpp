//========= Copyright Valve Corporation, All rights reserved. ============//
#include "provider_catalog_runtime.h"

#if defined( VIDEO_SERVICE_INTEGRATION )
#include "appframework/linked_systems.h"
#include "filesystem.h"
#include "materialsystem/imaterialsystem.h"
#include "inputsystem/provider_catalog.h"
#endif

#include <cstdio>
#include <cstring>
#include <string>

namespace
{
int checks = 0;
std::string calls;

bool Check( bool result, const char *description )
{
	++checks;
	if ( !result )
		std::fprintf( stderr, "FAIL: %s (calls=%s)\n", description, calls.c_str() );
	return result;
}

class FakeVideo : public CBaseAppSystem<IVideoSubSystem>
{
public:
	FakeVideo( VideoSystem_t system, const char *label ) : id( system ), name( label ) {}
	VideoSystem_t id;
	const char *name;
	int fail = 0;
	void Record( const char *operation ) { calls += std::string( name ) + operation + " "; }
	bool Connect( CreateInterfaceFn ) override
	{
		Record( "C" );
		return fail != 1;
	}
	void Disconnect() override { Record( "D" ); }
	InitReturnVal_t Init() override
	{
		Record( "I" );
		return fail == 3 ? INIT_FAILED : INIT_OK;
	}
	void Shutdown() override { Record( "S" ); }
	VideoSystem_t GetSystemID() override { return id; }
	VideoSystemStatus_t GetSystemStatus() override { return VideoSystemStatus::OK; }
	VideoSystemFeature_t GetSupportedFeatures() override
	{
		return VideoSystemFeature::PLAY_VIDEO_FILE_IN_MATERIAL;
	}
	const char *GetVideoSystemName() override { return name; }
	bool InitializeVideoSystem( IVideoCommonServices * ) override
	{
		Record( "V" );
		return fail != 2;
	}
	bool ShutdownVideoSystem() override
	{
		Record( "X" );
		return true;
	}
	VideoResult_t VideoSoundDeviceCMD( VideoSoundDeviceOperation_t, void *, void * ) override
	{
		return VideoResult::SUCCESS;
	}
	int GetSupportedFileExtensionCount() override { return 1; }
	const char *GetSupportedFileExtension( int ) override { return ".test"; }
	VideoSystemFeature_t GetSupportedFileExtensionFeatures( int ) override
	{
		return VideoSystemFeature::PLAY_VIDEO_FILE_IN_MATERIAL;
	}
	VideoResult_t PlayVideoFileFullScreen(
	    const char *, void *, int, int, int, int, bool, float, VideoPlaybackFlags_t ) override
	{
		return VideoResult::SUCCESS;
	}
	IVideoMaterial *CreateVideoMaterial( const char *, const char *, VideoPlaybackFlags_t ) override
	{
		return nullptr;
	}
	VideoResult_t DestroyVideoMaterial( IVideoMaterial * ) override { return VideoResult::SUCCESS; }
	IVideoRecorder *CreateVideoRecorder() override { return nullptr; }
	VideoResult_t DestroyVideoRecorder( IVideoRecorder * ) override { return VideoResult::SUCCESS; }
	VideoResult_t CheckCodecAvailability( VideoEncodeCodec_t ) override
	{
		return VideoResult::SUCCESS;
	}
	VideoResult_t GetLastResult() override { return VideoResult::SUCCESS; }
};

FakeVideo a( VideoSystem::BINK, "a" );
FakeVideo b( VideoSystem::WEBM, "b" );
IVideoSubSystem *CreateA()
{
	return &a;
}
IVideoSubSystem *CreateB()
{
	return &b;
}
IVideoSubSystem *CreateMissing()
{
	return nullptr;
}
#if defined( VIDEO_SERVICE_INTEGRATION )
// These identities are borrowed but never dereferenced: this fixture tests service
// composition, not filesystem or renderer behavior. Real product runs cover IO.
int dependencyIdentity;
void *Dependencies( const char *name, int * )
{
	if ( std::strcmp( name, FILESYSTEM_INTERFACE_VERSION ) == 0 ||
	     std::strcmp( name, MATERIAL_SYSTEM_INTERFACE_VERSION ) == 0 )
		return &dependencyIdentity;
	return nullptr;
}

bool TestLinkedServices( VideoProviderDescriptor *descriptors )
{
	bool passed = true;
	const InputProviderDescriptor *input = InputSystem_Describe();
#if defined( USE_SDL3 )
	const char *expected = "sdl3";
#elif defined( USE_SDL )
	const char *expected = "sdl2";
#else
	const char *expected = "native";
#endif
	passed &= Check( input && std::strcmp( input->name, expected ) == 0 &&
	                     input->create() == InputSystem_Create(),
	    "native linked input descriptor names and constructs the compiled provider" );
	const VideoProviderCatalog *built = VideoServices_GetBuiltProviders();
	passed &=
	    Check( built && built->count >= 0, "actual service exposes its build-selected catalog" );
	VideoProviderCatalog catalog = { descriptors, 2 };
	for ( int repeat = 0; repeat < 2; ++repeat )
	{
		calls.clear();
		IVideoServices *video = VideoServices_CreateWithProviders( &catalog );
		passed &= Check( video && video == VideoServices_Create(),
		    "legacy entry point shares configured service" );
		if ( !video )
			return false;
		passed &= Check( video->Connect( Dependencies ) && video->Init() == INIT_OK,
		    "actual service starts linked codecs" );
		passed &= Check( video->GetAvailableVideoSystemCount() == 2 &&
		                     video->GetSupportedFileExtensionCount( VideoSystem::BINK ) == 1,
		    "actual service exposes installed capabilities" );
		passed &= Check( !VideoServices_CreateWithProviders( &catalog ),
		    "actual connected service rejects reconfiguration" );
		video->Shutdown();
		video->Disconnect();
		passed &=
		    Check( video->GetAvailableVideoSystemCount() == 0 &&
		               video->GetSupportedFileExtensionCount( VideoSystem::BINK ) == 0 &&
		               video->LocateVideoSystemForPlayingFile( "stale.test" ) == VideoSystem::NONE,
		        "actual service clears counts and cached extensions before reconnect" );
		passed &= Check( calls == "aC aV bC bV aI bI bS aS bX bD aX aD ",
		    "actual service balances the provider lifecycle" );
	}
	for ( int stage = 1; stage <= 3; ++stage )
	{
		b.fail = stage;
		IVideoServices *video = VideoServices_CreateWithProviders( &catalog );
		bool connected = video && video->Connect( Dependencies );
		bool initialized = connected && video->Init() == INIT_OK;
		passed &= Check( !initialized && video && video->GetAvailableVideoSystemCount() == 0,
		    "actual required failure leaves no available providers" );
		video->Shutdown();
		video->Disconnect();
		descriptors[1].required = false;
		video = VideoServices_CreateWithProviders( &catalog );
		passed &= Check( video && video->Connect( Dependencies ) && video->Init() == INIT_OK &&
		                     video->GetAvailableVideoSystemCount() == 1 &&
		                     !video->IsVideoSystemAvailable( VideoSystem::WEBM ),
		    "actual optional failure preserves healthy provider and removes failed one" );
		video->Shutdown();
		video->Disconnect();
		descriptors[1].required = true;
	}
	b.fail = 0;
	VideoProviderDescriptor saved = descriptors[1];
	descriptors[1].system = descriptors[0].system;
	passed &= Check( !VideoServices_CreateWithProviders( &catalog ),
	    "actual service rejects duplicate selection" );
	descriptors[1] = saved;
	VideoProviderCatalog empty = { nullptr, 0 };
	IVideoServices *video = VideoServices_CreateWithProviders( &empty );
	passed &= Check( video && video->Connect( Dependencies ) && video->Init() == INIT_OK &&
	                     video->GetAvailableVideoSystemCount() == 0,
	    "actual explicit no-video product composition succeeds" );
	video->Shutdown();
	video->Disconnect();
	return passed;
}
#endif

}

int main( int argc, char **argv )
{
	bool passed = true;
	CVideoProviderSet providers;
	VideoProviderDescriptor descriptors[] = { { "bink", VideoSystem::BINK, CreateA, true },
	    { "webm", VideoSystem::WEBM, CreateB, true } };
	VideoProviderCatalog catalog = { descriptors, 2 };
	VideoProviderCatalog empty = { nullptr, 0 };
	passed &= Check( !providers.Init(), "Init requires Connect" );
	passed &= Check(
	    providers.Configure( empty ) && providers.Connect( nullptr, nullptr ) && providers.Init(),
	    "explicit optional empty composition starts" );
	passed &= Check( !providers.Connect( nullptr, nullptr ) && !providers.Configure( catalog ),
	    "connected composition cannot be replaced or double connected" );
	providers.Disconnect();
	passed &= Check( providers.Count() == 0, "empty disconnect resets state" );

	VideoProviderCatalog malformed = { nullptr, 1 };
	passed &= Check( !providers.Configure( malformed ), "missing descriptor storage rejected" );
	malformed = { descriptors, -1 };
	passed &= Check( !providers.Configure( malformed ), "negative count rejected" );
	malformed = { descriptors, VideoSystem::VIDEO_SYSTEM_COUNT };
	passed &= Check(
	    !providers.Configure( malformed ), "oversized count rejected before reading storage" );
	VideoProviderDescriptor saved = descriptors[1];
	descriptors[1].system = descriptors[0].system;
	passed &= Check(
	    !providers.Configure( catalog ), "duplicate system rejected before factory invocation" );
	descriptors[1] = saved;
	descriptors[1].name = descriptors[0].name;
	passed &= Check( !providers.Configure( catalog ), "duplicate config name rejected" );
	descriptors[1] = saved;
	descriptors[1].create = nullptr;
	passed &= Check( !providers.Configure( catalog ), "missing factory rejected" );
	descriptors[1] = saved;
	descriptors[1].system = VideoSystem::ALL_VIDEO_SYSTEMS;
	passed &= Check( !providers.Configure( catalog ), "out of range identity rejected" );
	descriptors[1] = saved;
	passed &= Check( calls.empty(), "invalid catalogs perform no provider callbacks" );

	for ( int repeat = 0; repeat < 2; ++repeat )
	{
		calls.clear();
		passed &= Check( providers.Configure( catalog ) && providers.Connect( nullptr, nullptr ),
		    "valid catalog connects" );
		passed &= Check( providers.Count() == 2 && providers.Get( VideoSystem::BINK ) == &a &&
		                     !providers.Get( -1 ),
		    "only connected providers are visible" );
		passed &= Check( providers.Init() && providers.Init(), "Init is repeat safe" );
		providers.Disconnect();
		passed &= Check( calls == "aC aV bC bV aI bI bS aS bX bD aX aD ",
		    "reverse shutdown and disconnect balance startup" );
		const std::string finished = calls;
		providers.Disconnect();
		providers.Shutdown();
		passed &= Check( calls == finished && providers.Count() == 0,
		    "repeated teardown performs no callbacks" );
	}

	for ( int stage = 1; stage <= 3; ++stage )
	{
		calls.clear();
		b.fail = stage;
		passed &= Check( providers.Configure( catalog ), "configure required failure case" );
		bool connected = providers.Connect( nullptr, nullptr );
		bool initialized = connected && providers.Init();
		passed &= Check( !initialized && providers.Count() == 0,
		    "required provider failure rolls back entire composition" );
		passed &= Check( std::strcmp( providers.FailedProvider(), "webm" ) == 0,
		    "failure identifies required provider" );
		const char *expected[] = { "", "aC aV bC bD aX aD ", "aC aV bC bV bX bD aX aD ",
		    "aC aV bC bV aI bI bS bX bD aS aX aD " };
		passed &= Check(
		    calls == expected[stage], "failure cleanup includes failed stage and prior providers" );

		calls.clear();
		descriptors[1].required = false;
		passed &= Check( providers.Configure( catalog ) && providers.Connect( nullptr, nullptr ) &&
		                     providers.Init(),
		    "optional unavailable provider does not fail composition" );
		passed &= Check( providers.Count() == 1 && !providers.Get( VideoSystem::WEBM ),
		    "optional failure is explicitly unavailable" );
		providers.Disconnect();
		descriptors[1].required = true;
	}
	b.fail = 0;
	calls.clear();
	descriptors[1].create = CreateMissing;
	passed &= Check( providers.Configure( catalog ) && !providers.Connect( nullptr, nullptr ) &&
	                     providers.Count() == 0,
	    "required null factory result rolls back" );
	passed &= Check( calls == "aC aV aX aD ", "null factory has no invalid callbacks" );
	descriptors[1].create = CreateA;
	calls.clear();
	passed &= Check( providers.Configure( catalog ) && !providers.Connect( nullptr, nullptr ),
	    "factory identity mismatch rejected" );
	passed &=
	    Check( calls == "aC aV aX aD ", "identity mismatch does not touch the wrong provider" );
	descriptors[1] = saved;

	calls.clear();
	{
		CVideoProviderSet scoped;
		passed &= Check( scoped.Configure( catalog ), "configure scoped provider lifecycle" );
		descriptors[1].create = nullptr;
		passed &= Check(
		    !scoped.Configure( catalog ) && scoped.Connect( nullptr, nullptr ) && scoped.Init(),
		    "invalid replacement preserves the copied valid catalog" );
		descriptors[1] = saved;
	}
	passed &= Check( calls == "aC aV bC bV aI bI bS aS bX bD aX aD ",
	    "destruction drains connected providers in reverse order" );

#if defined( VIDEO_SERVICE_INTEGRATION )
	passed &= TestLinkedServices( descriptors );
#endif

	// Negative fixture proves checks remain active in release builds.
	if ( argc > 1 && std::strcmp( argv[1], "--inject-failure" ) == 0 )
		passed &= Check( false, "seeded negative fixture" );
	std::printf( "Video provider catalog: %d checks, %s\n", checks, passed ? "pass" : "fail" );
	return passed ? 0 : 1;
}
