//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The Nintendo 3DS launch shell: hardware setup and the command line before
// main.cpp's statically composed launch.
//
//  * New 3DS: 804 MHz and the L2 cache (osSetSpeedupEnable); the extra
//    application core is opened to the engine's thread pool
//    (APT_SetAppCpuTimeLimit).
//  * Heaps: the GPU-visible linear heap holds the PICA backend's textures,
//    meshes and per-frame rings; the rest is the application heap.
//  * Files: the game lives in sdmc:/source-engine (the working directory);
//    stdout and stderr go to sdmc:/source-engine/console.log, the only log
//    an emulator or a console run leaves, and, once the PICA renderer has
//    the screens, to a text console on the bottom screen
//    (N3ds_StartDebugConsole).
//  * Command line: sdmc:/source-engine/args.txt (whitespace separated, one
//    or more lines), after the 3DS defaults.
//
//=============================================================================//

#include <3ds.h>

#include <cstdint>
#include <cstdio>
#include <sys/stat.h>
#include <cstdlib>
#include <malloc.h>
#include <cstring>
#include <unistd.h>
#include <new>
#include <sys/iosupport.h>

// libctru reads these when it sizes the heaps at startup.
extern "C" u32 __ctru_linear_heap_size;
// GPU-visible memory: textures, the meshes the legacy path draws (resident on
// demand), the transient rings and the render core's buffers (model geometry
// submitted in 1.5 MB steps). sp_a1_intro4 with the core world keeps ~12 MB of
// 22 free, while the main heap is the tighter side: 18 MB (2026-10-07).
u32 __ctru_linear_heap_size = 18u * 1024u * 1024u;
// The main thread's stack, which libctru's startup (stack_adjust.s) moves to
// the heap: Source's frames (map loading, the material system) need far more
// than the kernel's default stack.
extern "C" u32 __stacksize__;
u32 __stacksize__ = 2u * 1024u * 1024u;

namespace
{

const char *const kGameDir = "sdmc:/source-engine";

const char *const kDefaults[] = {
	"hl2_launcher",
	"-game", "portal2",
	"-novid",
	// The buttons, Circle Pad and C-Stick are SDL3's "Nintendo 3DS" gamepad;
	// Portal 2's joy_configuration.cfg binds its Xbox layout.
	"-console",
	"-window", "-w", "400", "-h", "240",
	"-physics", "vphysics_box3d",
	"+mat_queue_mode", "0",
	"+mat_picmip", "2",
	// Render and feature settings: engine/n3ds_platform_defaults.cpp. The
	// render core composes the one PICA200 device (render.device.pica) and
	// hands it to the shader API (RFC 0026).
};

char *g_Args[256];
int g_ArgCount = 0;

void Add( const char *arg )
{
	if ( g_ArgCount < int( sizeof( g_Args ) / sizeof( g_Args[0] ) ) - 1 )
		g_Args[g_ArgCount++] = strdup( arg );
}

void ReadArgsFile( const char *path )
{
	FILE *file = fopen( path, "r" );
	if ( !file )
		return;
	char token[512];
	while ( fscanf( file, "%511s", token ) == 1 )
		Add( token );
	fclose( file );
}

FILE *g_EarlyLog = nullptr;

// A fault anywhere: write the registers, so a crash shows where it happened
// (addr2line -e build-3ds/launcher_main/hl2_launcher <pc> <lr>).
void OnException( ERRF_ExceptionInfo *info, CpuRegisters *regs )
{
	FILE *file = fopen( "sdmc:/source-engine/crash.txt", "w" );
	if ( file )
	{
		fprintf( file, "exception type %d far %08lx fsr %08lx\npc %08lx lr %08lx sp %08lx\n",
			int( info->type ), (unsigned long)info->far, (unsigned long)info->fsr,
			(unsigned long)regs->pc, (unsigned long)regs->lr, (unsigned long)regs->sp );
		for ( int i = 0; i < 13; ++i )
			fprintf( file, "r%d %08lx\n", i, (unsigned long)regs->r[i] );
		// Return addresses on the stack: candidates for the call chain.
		const u32 *sp = reinterpret_cast<const u32 *>( regs->sp );
		for ( int i = 0; i < 256; ++i )
			if ( sp[i] >= 0x00100000 && sp[i] < 0x03000000 )
				fprintf( file, "stack %08lx\n", (unsigned long)sp[i] );
		fclose( file );
	}
	for ( ;; )
		svcSleepThread( 1000000000LL );
}

alignas( 8 ) std::uint8_t g_ExceptionStack[16384];

// Before every other static constructor (init_priority 101): the engine's
// constructors run before main, so a fault there needs the handler already.
__attribute__(( constructor( 101 ) )) void N3ds_EarlyInit()
{
	threadOnException( OnException, g_ExceptionStack + sizeof( g_ExceptionStack ), WRITE_DATA_TO_FAULTING_STACK );
	mkdir( "sdmc:/source-engine", 0777 );
	g_EarlyLog = fopen( "sdmc:/source-engine/early.txt", "w" );
	if ( g_EarlyLog )
	{
		fprintf( g_EarlyLog, "constructors start\n" );
		fflush( g_EarlyLog );
	}
}

} // namespace

// stdout and stderr (descriptors 1 and 2) go through one device that writes
// each chunk to console.log and, once N3ds_StartDebugConsole has run, to
// libctru's text console on the bottom screen.
static FILE *g_LogFile;
static ssize_t ( *g_ScreenWrite )( struct _reent *, void *, const char *, size_t );
static devoptab_t g_TeeDevice;

static ssize_t TeeWrite( struct _reent *r, void *fd, const char *ptr, size_t len )
{
	if ( g_LogFile )
	{
		fwrite( ptr, 1, len, g_LogFile );
		if ( memchr( ptr, '\n', len ) )
			fflush( g_LogFile );
	}
	if ( g_ScreenWrite )
		g_ScreenWrite( r, fd, ptr, len );
	return (ssize_t)len;
}

static void InstallTee()
{
	devoptab_list[STD_OUT] = &g_TeeDevice;
	devoptab_list[STD_ERR] = &g_TeeDevice;
}

static void StartLogTee()
{
	g_LogFile = fopen( "sdmc:/source-engine/console.log", "w" );
	if ( g_LogFile )
		setvbuf( g_LogFile, nullptr, _IOFBF, 16 * 1024 );
	g_TeeDevice.name = "tee";
	g_TeeDevice.write_r = TeeWrite;
	InstallTee();
	setvbuf( stdout, nullptr, _IOLBF, 0 );
	setvbuf( stderr, nullptr, _IONBF, 0 );
}

// Called by the PICA renderer after SDL's gfxInit (which owns both screens'
// framebuffers): the bottom screen becomes a scrolling text console.
extern "C" void N3ds_StartDebugConsole()
{
	static PrintConsole s_Console;
	if ( g_ScreenWrite )
		return;
	consoleInit( GFX_BOTTOM, &s_Console );
	g_ScreenWrite = devoptab_list[STD_OUT]->write_r;
	InstallTee();
}

// operator new's failures name their memory's owners too: tier0's
// large-allocation ledger (MemLedger_Print) before the abort, as tier0's own
// allocator failures do (tools/n3ds/n3ds.py ledger symbolizes it).
extern "C" void MemLedger_Print();

static void OnNewFailure()
{
	std::set_new_handler( nullptr );
	fprintf( stderr, "OOM: operator new failed\n" );
	MemLedger_Print();
	abort();
}

extern "C" void N3ds_PrepareLaunch( int *argc, char ***argv )
{
	std::set_new_handler( OnNewFailure );
	bool isNew3ds = false;
	APT_CheckNew3DS( &isNew3ds );
	if ( isNew3ds )
		osSetSpeedupEnable( true );
	// Let threads run on the system core too (up to 80 % of its time).
	APT_SetAppCpuTimeLimit( 80 );
	// BSD sockets (soc:U): the engine opens its local client/server sockets
	// at startup. The service needs a page-aligned buffer it owns.
	constexpr u32 kSocketBufferBytes = 0x40000;
	static void *s_SocketBuffer = memalign( 0x1000, kSocketBufferBytes );
	if ( !s_SocketBuffer || R_FAILED( socInit( (u32 *)s_SocketBuffer, kSocketBufferBytes ) ) )
		printf( "n3ds: socInit failed; networking unavailable\n" );

	if ( g_EarlyLog )
	{
		fprintf( g_EarlyLog, "main\n" );
		fclose( g_EarlyLog );
		g_EarlyLog = nullptr;
	}
	chdir( kGameDir );
	StartLogTee();
	printf( "n3ds: %s 3DS, linear heap %lu KB free, heap %lu KB\n", isNew3ds ? "New" : "Old",
		(unsigned long)( linearSpaceFree() / 1024 ), (unsigned long)( osGetMemRegionFree( MEMREGION_APPLICATION ) / 1024 ) );

	for ( const char *arg : kDefaults )
		Add( arg );
	ReadArgsFile( "sdmc:/source-engine/args.txt" );
	for ( int i = 1; i < *argc; ++i )
		Add( ( *argv )[i] );
	g_Args[g_ArgCount] = nullptr;
	*argc = g_ArgCount;
	*argv = g_Args;
	for ( int i = 0; i < g_ArgCount; ++i )
		printf( "%s%s", i ? " " : "n3ds: argv ", g_Args[i] );
	printf( "\n" );
}
