//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dynamic range for the SDL3-Vulkan bridge on Android (see
//          sdl3_dynamic_range.h). SDL reports no HDR there, so the display is
//          read over JNI: Display.isHdr and its HDR capabilities (the panel's
//          peak, API 24) and, where the system offers it, the current HDR/SDR
//          ratio (API 34). The activity asks for the HDR color mode in its
//          manifest; the swapchain's color space carries the rest.
//
//===========================================================================//

#if defined( __ANDROID__ )

#include "sdl3_dynamic_range.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_system.h>
#include <jni.h>

#include <chrono>
#include <cstdio>
#include <mutex>

namespace render_vulkan
{
namespace
{

// SDR reference white (ITU-R BT.2408) for a panel that states only its peak.
constexpr float kReferenceWhiteNits = 203.0f;

struct AndroidDisplayHdr
{
	bool read = false;
	bool hdr = false;      // Display.isHdr
	float maxNits = 0.0f;  // HdrCapabilities.getDesiredMaxLuminance
	float sdrRatio = 0.0f; // Display.getHdrSdrRatio where available, else 0
};

// A Java call's exception (an older API level lacks the method) is cleared.
bool Cleared( JNIEnv *env )
{
	if ( !env->ExceptionCheck() )
		return false;
	env->ExceptionClear();
	return true;
}

AndroidDisplayHdr QueryDisplay()
{
	AndroidDisplayHdr out;
	JNIEnv *env = static_cast<JNIEnv *>( SDL_GetAndroidJNIEnv() );
	jobject activity = static_cast<jobject>( SDL_GetAndroidActivity() );
	if ( !env || !activity )
		return out;
	if ( env->PushLocalFrame( 16 ) != 0 )
	{
		env->DeleteLocalRef( activity );
		return out;
	}
	out.read = true;
	jclass activityClass = env->GetObjectClass( activity );
	jmethodID getWindowManager =
	    env->GetMethodID( activityClass, "getWindowManager", "()Landroid/view/WindowManager;" );
	jobject manager =
	    getWindowManager ? env->CallObjectMethod( activity, getWindowManager ) : nullptr;
	jobject display = nullptr;
	if ( !Cleared( env ) && manager )
	{
		jmethodID getDisplay = env->GetMethodID(
		    env->GetObjectClass( manager ), "getDefaultDisplay", "()Landroid/view/Display;" );
		display = getDisplay ? env->CallObjectMethod( manager, getDisplay ) : nullptr;
		Cleared( env );
	}
	if ( display )
	{
		jclass displayClass = env->GetObjectClass( display );
		if ( jmethodID isHdr = env->GetMethodID( displayClass, "isHdr", "()Z" ) )
			out.hdr = env->CallBooleanMethod( display, isHdr ) && !Cleared( env );
		Cleared( env );
		jmethodID getCaps = env->GetMethodID(
		    displayClass, "getHdrCapabilities", "()Landroid/view/Display$HdrCapabilities;" );
		jobject caps = getCaps ? env->CallObjectMethod( display, getCaps ) : nullptr;
		Cleared( env );
		if ( caps )
		{
			if ( jmethodID maxLuminance = env->GetMethodID(
			         env->GetObjectClass( caps ), "getDesiredMaxLuminance", "()F" ) )
			{
				const jfloat nits = env->CallFloatMethod( caps, maxLuminance );
				if ( !Cleared( env ) && nits > 0.0f )
					out.maxNits = nits;
			}
			Cleared( env );
		}
		// API 34: the headroom the system grants now.
		jmethodID available = env->GetMethodID( displayClass, "isHdrSdrRatioAvailable", "()Z" );
		Cleared( env );
		if ( available && env->CallBooleanMethod( display, available ) && !Cleared( env ) )
		{
			if ( jmethodID ratio = env->GetMethodID( displayClass, "getHdrSdrRatio", "()F" ) )
			{
				const jfloat value = env->CallFloatMethod( display, ratio );
				if ( !Cleared( env ) && value >= 1.0f )
					out.sdrRatio = value;
			}
			Cleared( env );
		}
		Cleared( env );
	}
	env->PopLocalFrame( nullptr );
	env->DeleteLocalRef( activity );
	return out;
}

// The display's state, read at most once a second (JNI on the render path).
AndroidDisplayHdr Display()
{
	static std::mutex s_Lock;
	static AndroidDisplayHdr s_Cached;
	static std::chrono::steady_clock::time_point s_ReadAt;
	std::lock_guard<std::mutex> lock( s_Lock );
	const auto now = std::chrono::steady_clock::now();
	if ( !s_Cached.read || now - s_ReadAt > std::chrono::seconds( 1 ) )
	{
		const bool first = !s_Cached.read;
		s_Cached = QueryDisplay();
		s_ReadAt = now;
		if ( first )
			std::fprintf( stderr,
			    "[NativeVulkan] Android display: HDR %s, peak %.0f cd/m^2, HDR/SDR ratio %s%.2f\n",
			    s_Cached.hdr ? "yes" : "no", s_Cached.maxNits,
			    s_Cached.sdrRatio > 0.0f ? "" : "unavailable ", s_Cached.sdrRatio );
	}
	return s_Cached;
}

} // namespace

bool Sdl3CanShowExtendedRange( SDL_Window *window )
{
	return window != nullptr && Display().hdr;
}

bool Sdl3SetExtendedRange( SDL_Window *window, bool extended )
{
	(void)extended;
	return window != nullptr;
}

Sdl3DisplayHeadroom Sdl3ReadHeadroom( SDL_Window *window )
{
	Sdl3DisplayHeadroom headroom;
	if ( !window )
		return headroom;
	const AndroidDisplayHdr display = Display();
	if ( !display.hdr )
		return headroom;
	// The panel's peak over reference white is what HDR content may reach;
	// the system's ratio, where it reports one, is what it grants now.
	const float potential =
	    display.maxNits > kReferenceWhiteNits ? display.maxNits / kReferenceWhiteNits : 1.0f;
	headroom.potential = potential;
	headroom.current = display.sdrRatio > 1.0f ? display.sdrRatio : potential;
	return headroom;
}

} // namespace render_vulkan

#endif // __ANDROID__
