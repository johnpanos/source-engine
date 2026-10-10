//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: determine CPU speed under linux
//
// $NoKeywords: $
//=============================================================================//
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#if defined( LINUX )
#include <linux/sysctl.h>
#else
#include <sys/sysctl.h>
# ifdef __APPLE__
#  define CPUFREQ_SYSCTL "hw.cpufrequency_max"
#else
#  define CPUFREQ_SYSCTL "dev.cpu.0.freq"
# endif
#endif
#include <sys/time.h>
#include <unistd.h>
#include <tier0/platform.h>
#include "foundation_facade.h"
#include <errno.h>

#define rdtsc(x) \
	__asm__ __volatile__ ("rdtsc" : "=A" (x))

// Compute the positive difference between two 64 bit numbers.
static inline uint64 diff(uint64 v1, uint64 v2)
{
	int64 d = v1 - v2;
	if (d >= 0)
		return d;
	else
		return -d;
}

#if defined(OSX) || defined(PLATFORM_BSD)

// Mac or BSD
uint64 GetCPUFreqFromPROC()
{
	uint64 freq_hz = 0;
	size_t freq_size = sizeof(freq_hz);
	int retval = sysctlbyname(CPUFREQ_SYSCTL, &freq_hz, &freq_size, NULL, 0);
	return freq_hz;
}

#else

// Linux
uint64 GetCPUFreqFromPROC()
{
	double mhz = 0;
	char line[1024], *s, search_str[] = "cpu MHz";

	/* open proc/cpuinfo */
	FILE *fp = fopen( "/proc/cpuinfo", "r" );
	if (fp == NULL)
	{
		return 0;
	}

	/* ignore all lines until we reach MHz information */
	while (fgets(line, 1024, fp) != NULL) 
	{ 
		if (strstr(line, search_str) != NULL) 
		{
			/* ignore all characters in line up to : */
			for (s = line; *s && (*s != ':'); ++s)
				;

			/* get MHz number */
			if ( *s && ( sscanf( s + 1, "%lf", &mhz) == 1 ) )
				break;
		}
    }

    fclose(fp);

    return ( uint64 )( mhz * 1000000 );
}

#endif

uint64 CalculateCPUFreq()
{
#if defined(__APPLE__) || defined(PLATFORM_BSD)
	return GetCPUFreqFromPROC();
#else
	// Try to open cpuinfo_max_freq. If the kernel was built with cpu scaling support disabled, this will fail.
	FILE *fp = fopen( "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r" );
	if ( fp )
	{
		char buf[ 256 ];
		uint64 retVal = 0;

		buf[ 0 ] = 0;
		if( fread( buf, 1, ARRAYSIZE( buf ), fp ) )
		{
			retVal = ( uint64 )atoll( buf );
		}
		fclose(fp);

		if( retVal )
		{
			return retVal * 1000;
		}
	}

#if !defined(__arm__) && !defined(__aarch64__)
	// fallback mechanism to calculate when failed
	// Compute the period. Loop until we get 3 consecutive periods that
	// are the same to within a small error. The error is chosen
	// to be +/- 0.02% on a P-200.
	const uint64 error = 40000;
	const int max_iterations = 600;
	int count;
	uint64 period, period1 = error * 2, period2 = 0,  period3 = 0;

	for (count = 0; count < max_iterations; count++)
	{
		uint64 start_tsc, end_tsc;

		// R103: the interval comes from Tier 0's monotonic clock, the sleep
		// from its thread provider.
		const platform::IMonotonicClock &clock = tier0_facade::MonotonicClock();
		const platform::MonotonicTimestamp start_time = clock.Now();
		rdtsc( start_tsc );
		tier0_facade::Threads().SleepFor( 5000000 ); // 5 msec
		const platform::MonotonicTimestamp end_time = clock.Now();
		rdtsc( end_tsc );

		const double seconds = clock.ElapsedNanoseconds( start_time, end_time ) * 1e-9;
		period3 = ( end_tsc - start_tsc ) / seconds;

		if (diff ( period1, period2 ) <= error &&
			diff ( period2, period3 ) <= error &&
			diff ( period1, period3 ) <= error )
		{
			break;
		}

		period1 = period2;
		period2 = period3;
	}

	if ( count == max_iterations )
	{
		return GetCPUFreqFromPROC(); // fall back to /proc
	}

	// Set the period to the average period measured.
	period = ( period1 + period2 + period3 ) / 3;

	// Some Pentiums have broken TSCs that increment very
	// slowly or unevenly. 
	if (period < 10000000)
	{
		return GetCPUFreqFromPROC(); // fall back to /proc
	}

	return period;
#else
	// ARM hard-coded frequency
	return (uint64)2000000000;
#endif // if !ARM
#endif // if APPLE
}

