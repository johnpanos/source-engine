//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The device's own vibrator (Android), when it has amplitude control.
//
// Every vibration is tagged as media (VibrationAttributes.USAGE_MEDIA on
// Android 13+, AudioAttributes.USAGE_GAME before), which is the usage Android
// documents for games, so the user's media-vibration intensity applies.
//
//===========================================================================//

#include "vibrator_device.h"
#include "tier0/dbg.h"

#if defined( ANDROID ) && defined( USE_SDL3 )
#include "SDL.h"
#include <jni.h>
#include <atomic>
#endif

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

#if defined( ANDROID ) && defined( USE_SDL3 )

namespace
{
// Android 13 (API 33): VibrationAttributes.createForUsage and
// Vibrator.vibrate(VibrationEffect, VibrationAttributes).
const int kVibrationAttributesSdk = 33;

// Clears a pending Java exception. Returns true if there was one.
bool TakeException( JNIEnv *env, const char *pWhat )
{
	if ( !env->ExceptionCheck() )
		return false;
	env->ExceptionClear();
	Warning( "Device vibrator: %s threw a Java exception.\n", pWhat );
	return true;
}

// A global reference to the class, or NULL.
jclass FindGlobalClass( JNIEnv *env, const char *pName )
{
	jclass localClass = env->FindClass( pName );
	if ( TakeException( env, pName ) || !localClass )
		return NULL;
	jclass globalClass = static_cast<jclass>( env->NewGlobalRef( localClass ) );
	env->DeleteLocalRef( localClass );
	return globalClass;
}
} // namespace

struct CDeviceVibrator::State
{
	jobject m_Vibrator = NULL;   // android.os.Vibrator
	jobject m_Attributes = NULL; // VibrationAttributes or AudioAttributes
	jclass m_EffectClass = NULL; // android.os.VibrationEffect
	jmethodID m_CreateOneShot = NULL;
	jmethodID m_Vibrate = NULL;
	jmethodID m_Cancel = NULL;
	// Set by a failed Play; the references stay valid for Cancel until Shutdown.
	bool m_bFailed = false;
	std::atomic<bool> m_bSuspended;

	State() { m_bSuspended.store( false ); }

	void Release( JNIEnv *env )
	{
		if ( m_Vibrator )
			env->DeleteGlobalRef( m_Vibrator );
		if ( m_Attributes )
			env->DeleteGlobalRef( m_Attributes );
		if ( m_EffectClass )
			env->DeleteGlobalRef( m_EffectClass );
		m_Vibrator = m_Attributes = NULL;
		m_EffectClass = NULL;
	}
};

// Media-usage attributes as a local reference, with the matching
// Vibrator.vibrate overload; NULL on failure.
static jobject CreateAttributes( JNIEnv *env, jclass vibratorClass, jmethodID *pVibrate )
{
	jobject attributes = NULL;
	if ( SDL_GetAndroidSDKVersion() >= kVibrationAttributesSdk )
	{
		jclass attributesClass = env->FindClass( "android/os/VibrationAttributes" );
		if ( TakeException( env, "VibrationAttributes" ) || !attributesClass )
			return NULL;
		jfieldID usageMedia = env->GetStaticFieldID( attributesClass, "USAGE_MEDIA", "I" );
		jmethodID createForUsage = env->GetStaticMethodID(
		    attributesClass, "createForUsage", "(I)Landroid/os/VibrationAttributes;" );
		if ( !TakeException( env, "VibrationAttributes lookup" ) && usageMedia && createForUsage )
		{
			attributes = env->CallStaticObjectMethod( attributesClass, createForUsage,
			    env->GetStaticIntField( attributesClass, usageMedia ) );
			if ( TakeException( env, "VibrationAttributes.createForUsage" ) )
				attributes = NULL;
		}
		env->DeleteLocalRef( attributesClass );
		*pVibrate = env->GetMethodID( vibratorClass, "vibrate",
		    "(Landroid/os/VibrationEffect;Landroid/os/VibrationAttributes;)V" );
	}
	else
	{
		jclass audioClass = env->FindClass( "android/media/AudioAttributes" );
		jclass builderClass = env->FindClass( "android/media/AudioAttributes$Builder" );
		if ( !TakeException( env, "AudioAttributes" ) && audioClass && builderClass )
		{
			jfieldID usageGame = env->GetStaticFieldID( audioClass, "USAGE_GAME", "I" );
			jmethodID init = env->GetMethodID( builderClass, "<init>", "()V" );
			jmethodID setUsage = env->GetMethodID(
			    builderClass, "setUsage", "(I)Landroid/media/AudioAttributes$Builder;" );
			jmethodID build =
			    env->GetMethodID( builderClass, "build", "()Landroid/media/AudioAttributes;" );
			if ( !TakeException( env, "AudioAttributes lookup" ) && usageGame && init && setUsage &&
			     build )
			{
				jobject builder = env->NewObject( builderClass, init );
				if ( !TakeException( env, "AudioAttributes.Builder" ) && builder )
				{
					jobject sameBuilder = env->CallObjectMethod(
					    builder, setUsage, env->GetStaticIntField( audioClass, usageGame ) );
					if ( sameBuilder )
						env->DeleteLocalRef( sameBuilder );
					if ( !TakeException( env, "AudioAttributes.Builder.setUsage" ) )
					{
						attributes = env->CallObjectMethod( builder, build );
						if ( TakeException( env, "AudioAttributes.Builder.build" ) )
							attributes = NULL;
					}
					env->DeleteLocalRef( builder );
				}
			}
		}
		if ( audioClass )
			env->DeleteLocalRef( audioClass );
		if ( builderClass )
			env->DeleteLocalRef( builderClass );
		*pVibrate = env->GetMethodID( vibratorClass, "vibrate",
		    "(Landroid/os/VibrationEffect;Landroid/media/AudioAttributes;)V" );
	}

	if ( TakeException( env, "Vibrator.vibrate lookup" ) || !*pVibrate )
	{
		if ( attributes )
			env->DeleteLocalRef( attributes );
		return NULL;
	}
	return attributes;
}

// The system vibrator as a local reference, or NULL.
static jobject GetSystemVibrator( JNIEnv *env )
{
	jobject activity = static_cast<jobject>( SDL_GetAndroidActivity() );
	if ( !activity )
		return NULL;

	jobject vibrator = NULL;
	jclass contextClass = env->FindClass( "android/content/Context" );
	if ( !TakeException( env, "Context" ) && contextClass )
	{
		jfieldID serviceName =
		    env->GetStaticFieldID( contextClass, "VIBRATOR_SERVICE", "Ljava/lang/String;" );
		jmethodID getSystemService = env->GetMethodID(
		    contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;" );
		if ( !TakeException( env, "Context lookup" ) && serviceName && getSystemService )
		{
			jobject name = env->GetStaticObjectField( contextClass, serviceName );
			vibrator = env->CallObjectMethod( activity, getSystemService, name );
			if ( TakeException( env, "Context.getSystemService" ) )
				vibrator = NULL;
			if ( name )
				env->DeleteLocalRef( name );
		}
		env->DeleteLocalRef( contextClass );
	}
	env->DeleteLocalRef( activity );
	return vibrator;
}

CDeviceVibrator::CDeviceVibrator() : m_pState( NULL )
{
}

CDeviceVibrator::~CDeviceVibrator()
{
	Shutdown();
}

bool CDeviceVibrator::Init()
{
	Shutdown();

	JNIEnv *env = static_cast<JNIEnv *>( SDL_GetAndroidJNIEnv() );
	if ( !env )
		return false;

	jobject vibrator = GetSystemVibrator( env );
	if ( !vibrator )
		return false;

	State *pState = new State;
	jclass vibratorClass = env->FindClass( "android/os/Vibrator" );
	bool bUsable = false;
	if ( !TakeException( env, "Vibrator" ) && vibratorClass )
	{
		jmethodID hasVibrator = env->GetMethodID( vibratorClass, "hasVibrator", "()Z" );
		jmethodID hasAmplitudeControl =
		    env->GetMethodID( vibratorClass, "hasAmplitudeControl", "()Z" );
		pState->m_Cancel = env->GetMethodID( vibratorClass, "cancel", "()V" );
		if ( !TakeException( env, "Vibrator lookup" ) && hasVibrator && hasAmplitudeControl &&
		     pState->m_Cancel )
		{
			bool bHasVibrator = env->CallBooleanMethod( vibrator, hasVibrator );
			bool bAmplitude =
			    bHasVibrator && env->CallBooleanMethod( vibrator, hasAmplitudeControl );
			if ( TakeException( env, "Vibrator capabilities" ) )
				bHasVibrator = bAmplitude = false;

			if ( !bHasVibrator )
				Msg( "Device vibrator: none.\n" );
			else if ( !bAmplitude )
				Msg(
				    "Device vibrator: on/off only (no amplitude control); not used for rumble.\n" );
			else
				bUsable = true;
		}

		jobject attributes =
		    bUsable ? CreateAttributes( env, vibratorClass, &pState->m_Vibrate ) : NULL;
		pState->m_EffectClass =
		    bUsable ? FindGlobalClass( env, "android/os/VibrationEffect" ) : NULL;
		if ( pState->m_EffectClass )
		{
			pState->m_CreateOneShot = env->GetStaticMethodID(
			    pState->m_EffectClass, "createOneShot", "(JI)Landroid/os/VibrationEffect;" );
			TakeException( env, "VibrationEffect.createOneShot lookup" );
		}
		bUsable = bUsable && attributes && pState->m_CreateOneShot;
		if ( bUsable )
		{
			pState->m_Vibrator = env->NewGlobalRef( vibrator );
			pState->m_Attributes = env->NewGlobalRef( attributes );
		}
		if ( attributes )
			env->DeleteLocalRef( attributes );
		env->DeleteLocalRef( vibratorClass );
	}
	env->DeleteLocalRef( vibrator );

	if ( !bUsable || !pState->m_Vibrator || !pState->m_Attributes )
	{
		pState->Release( env );
		delete pState;
		return false;
	}

	m_pState = pState;
	Msg( "Device vibrator: amplitude control; rumble plays on the device (API %d).\n",
	    SDL_GetAndroidSDKVersion() );
	return true;
}

void CDeviceVibrator::Shutdown()
{
	if ( !m_pState )
		return;

	JNIEnv *env = static_cast<JNIEnv *>( SDL_GetAndroidJNIEnv() );
	if ( env )
	{
		Cancel();
		m_pState->Release( env );
	}
	delete m_pState;
	m_pState = NULL;
}

bool CDeviceVibrator::IsAvailable() const
{
	return m_pState != NULL && !m_pState->m_bFailed;
}

void CDeviceVibrator::Play( int nAmplitude, int nDurationMs )
{
	if ( !IsAvailable() || IsSuspended() || nAmplitude <= 0 || nDurationMs <= 0 )
		return;
	if ( nAmplitude > 255 )
		nAmplitude = 255;

	JNIEnv *env = static_cast<JNIEnv *>( SDL_GetAndroidJNIEnv() );
	if ( !env )
		return;

	jobject effect =
	    env->CallStaticObjectMethod( m_pState->m_EffectClass, m_pState->m_CreateOneShot,
	        static_cast<jlong>( nDurationMs ), static_cast<jint>( nAmplitude ) );
	bool bFailed = TakeException( env, "VibrationEffect.createOneShot" ) || !effect;
	if ( !bFailed )
	{
		env->CallVoidMethod(
		    m_pState->m_Vibrator, m_pState->m_Vibrate, effect, m_pState->m_Attributes );
		bFailed = TakeException( env, "Vibrator.vibrate" );
	}
	if ( effect )
		env->DeleteLocalRef( effect );

	// A missing VIBRATE permission or a broken service fails every call; stop
	// trying rather than throwing an exception per frame.
	if ( bFailed )
	{
		Warning( "Device vibrator: disabled after a failed vibration.\n" );
		m_pState->m_bFailed = true;
	}
}

void CDeviceVibrator::Cancel()
{
	if ( !m_pState )
		return;
	JNIEnv *env = static_cast<JNIEnv *>( SDL_GetAndroidJNIEnv() );
	if ( !env )
		return;
	env->CallVoidMethod( m_pState->m_Vibrator, m_pState->m_Cancel );
	TakeException( env, "Vibrator.cancel" );
}

// Runs on Android's UI thread (SDL lifecycle events). Vibrator is thread-safe.
// A Play already past its IsSuspended check can still start after this cancel;
// it ends with its one-shot, and Android does not start vibrations for
// background applications anyway.
void CDeviceVibrator::SetSuspended( bool bSuspended )
{
	if ( !m_pState )
		return;
	m_pState->m_bSuspended.store( bSuspended );
	if ( bSuspended )
		Cancel();
}

bool CDeviceVibrator::IsSuspended() const
{
	return m_pState && m_pState->m_bSuspended.load();
}

#else // !( ANDROID && USE_SDL3 )

// No device vibrator backend on this platform.
struct CDeviceVibrator::State
{
};

CDeviceVibrator::CDeviceVibrator() : m_pState( NULL )
{
}

CDeviceVibrator::~CDeviceVibrator()
{
}

bool CDeviceVibrator::Init()
{
	return false;
}

void CDeviceVibrator::Shutdown()
{
}

bool CDeviceVibrator::IsAvailable() const
{
	return false;
}

void CDeviceVibrator::Play( int, int )
{
}

void CDeviceVibrator::Cancel()
{
}

void CDeviceVibrator::SetSuspended( bool )
{
}

bool CDeviceVibrator::IsSuspended() const
{
	return false;
}

#endif // ANDROID && USE_SDL3
