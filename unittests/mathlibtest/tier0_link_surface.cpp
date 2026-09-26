//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The tier0 link surface mathlib needs, for the standalone mathlib
//          conformance suite and benchmark (mathlib.core, mathlibbench).
//
// Those programs link mathlib's objects and no engine library, so the same
// build runs under the headless conformance runner, on the Linux host and
// on an Android device from adb. This unit defines what mathlib references:
//
//   GetCPUInformation  the facts MathLib_Init selects on, reported the way
//                      tier0/cpu.cpp reports them: SSE/SSE2 from cpuid on x86,
//                      none on ARM. Only m_bSSE/m_bSSE2/m_bMMX/m_b3DNow are read
//   Warning, Msg       forwarded to stderr
//   HushAsserts        never hushed
//   g_pMemAlloc        null: the programs build with NO_MALLOC_OVERRIDE
//
//=============================================================================//

#include "tier0/platform.h"
#include "tier0/dbg.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#if defined( __i386__ ) || defined( __x86_64__ )
#include <cpuid.h>
#endif

const CPUInformation *GetCPUInformation()
{
	static CPUInformation s_info;
	static bool s_bInit = false;
	if ( !s_bInit )
	{
		std::memset( (void *)&s_info, 0, sizeof( s_info ) );
		s_info.m_Size = sizeof( s_info );
#if defined( __i386__ ) || defined( __x86_64__ )
		unsigned int eax, ebx, ecx, edx;
		if ( __get_cpuid( 1, &eax, &ebx, &ecx, &edx ) )
		{
			s_info.m_bMMX = ( edx & ( 1u << 23 ) ) != 0;
			s_info.m_bSSE = ( edx & ( 1u << 25 ) ) != 0;
			s_info.m_bSSE2 = ( edx & ( 1u << 26 ) ) != 0;
			s_info.m_bSSE3 = ( ecx & 1u ) != 0;
			s_info.m_bSSSE3 = ( ecx & ( 1u << 9 ) ) != 0;
			s_info.m_bSSE41 = ( ecx & ( 1u << 19 ) ) != 0;
			s_info.m_bSSE42 = ( ecx & ( 1u << 20 ) ) != 0;
		}
#endif
		s_info.m_nLogicalProcessors = 1;
		s_info.m_nPhysicalProcessors = 1;
		s_bInit = true;
	}
	return &s_info;
}

void Warning( const tchar *pMsg, ... )
{
	va_list args;
	va_start( args, pMsg );
	std::vfprintf( stderr, pMsg, args );
	va_end( args );
}

void Msg( const tchar *pMsg, ... )
{
	va_list args;
	va_start( args, pMsg );
	std::vfprintf( stderr, pMsg, args );
	va_end( args );
}

bool HushAsserts()
{
	return false;
}
