//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dynamic range for the SDL3-Vulkan bridge on iOS and tvOS (see
//          sdl3_dynamic_range.h). MoltenVK maps
//          VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT to the Metal layer's
//          extended linear sRGB color space, but it enables EDR only on macOS.
//          Here the layer SDL made for the window's Vulkan surface is set to
//          high dynamic range:
//          - iOS and tvOS 26: CALayer.preferredDynamicRange =
//            CADynamicRangeHigh (tvOS has no other way). It engages EDR only
//            for content tagged with headroom above 1, so the drawables are
//            tagged (contentsHeadroom) with the display's potential headroom
//            and are not tone mapped again (toneMapMode never): the core's
//            output term maps them to the current headroom. Untagged, an
//            iPhone stays at 1.2 of its 8.0;
//          - iOS 16 to 25: CAMetalLayer.wantsExtendedDynamicRangeContent.
//          The headroom is UIScreen's currentEDRHeadroom and
//          potentialEDRHeadroom (iOS 16), read on the main thread as UIKit
//          requires.
//
//          On tvOS the television decides the range, not the layer: an Apple
//          TV driving its display in SDR reports a headroom of about 1.2. An
//          extended-range presentation therefore also asks tvOS to switch the
//          display into HDR, through the window's AVDisplayManager with
//          criteria for HDR10 (BT.2020 primaries, the SMPTE ST 2084 transfer)
//          at the screen's refresh rate; a standard presentation withdraws
//          them. tvOS switches only when the user's "Match Dynamic Range"
//          setting is on (AVDisplayManager.displayCriteriaMatchingEnabled);
//          the switch takes seconds, and the headroom rises once it is done.
//
//===========================================================================//

#if defined( __APPLE__ )

#include "sdl3_dynamic_range.h"

#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#if TARGET_OS_TV
#import <AVFoundation/AVFoundation.h>
#import <AVKit/AVKit.h>
#import <CoreMedia/CoreMedia.h>
#endif

#include <SDL3/SDL.h>

#include <atomic>

namespace render_vulkan
{
namespace
{

// The last headroom read on the main thread, for callers on other threads.
std::atomic<float> g_CurrentHeadroom( 1.0f );
std::atomic<float> g_PotentialHeadroom( 1.0f );

// The headroom extended-range drawables are tagged with: the display's
// potential headroom (1.0 and below are untagged, which the layer reads as
// SDR content).
CGFloat ContentsHeadroom()
{
	const float potential = g_PotentialHeadroom.load( std::memory_order_relaxed );
	return potential > 1.0f ? CGFloat( potential ) : 0.0;
}

// The Metal layer SDL created for the window's Vulkan surface
// (SDL_uikitmetalview, found by its tag).
CAMetalLayer *MetalLayer( SDL_Window *window )
{
	if ( !window )
		return nil;
	const SDL_PropertiesID props = SDL_GetWindowProperties( window );
	UIWindow *uiWindow = (UIWindow *)SDL_GetPointerProperty(
	    props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr );
	const Sint64 tag =
	    SDL_GetNumberProperty( props, SDL_PROP_WINDOW_UIKIT_METAL_VIEW_TAG_NUMBER, 0 );
	if ( !uiWindow || tag == 0 )
		return nil;
	UIView *view = [uiWindow viewWithTag:(NSInteger)tag];
	if ( !view || ![view.layer isKindOfClass:[CAMetalLayer class]] )
		return nil;
	return (CAMetalLayer *)view.layer;
}

UIScreen *WindowScreen( SDL_Window *window )
{
	UIWindow *uiWindow = window ? (UIWindow *)SDL_GetPointerProperty(
	                                  SDL_GetWindowProperties( window ),
	                                  SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr )
	                            : nil;
	return uiWindow ? uiWindow.screen : [UIScreen mainScreen];
}

#if TARGET_OS_TV
// tvOS reports no EDR headroom for an HDR television: measured on an Apple TV
// 4K (tvOS 26.6), UIScreen says 1.20 of 1.20 in SDR and 1.00 of 1.00 once
// the TV is in HDR10, while tagged content above white shows brighter on the
// TV. In HDR10 the headroom is therefore declared: the usual HDR10 mastering
// peak (1000 cd/m^2) over reference white (203 cd/m^2, ITU-R BT.2408), and
// tvOS maps content tagged with it to the television's real peak.
constexpr float kTvHdr10Headroom = 1000.0f / 203.0f;
bool TvInHdrMode( SDL_Window *window );
#endif

// Reads the screen's headroom into the cache. Main thread only.
void RefreshHeadroom( SDL_Window *window )
{
	float current = 1.0f, potential = 1.0f;
	if ( @available( iOS 16.0, tvOS 16.0, * ) )
	{
		UIScreen *screen = WindowScreen( window );
		current = static_cast<float>( screen.currentEDRHeadroom );
		potential = static_cast<float>( screen.potentialEDRHeadroom );
	}
#if TARGET_OS_TV
	if ( TvInHdrMode( window ) )
		current = potential = kTvHdr10Headroom;
#endif
	current = current > 1.0f ? current : 1.0f;
	potential = potential > current ? potential : current;
	g_CurrentHeadroom.store( current, std::memory_order_relaxed );
	g_PotentialHeadroom.store( potential, std::memory_order_relaxed );
}

bool SupportsLayerRange()
{
	if ( @available( iOS 26.0, tvOS 26.0, * ) )
		return true;
#if !TARGET_OS_TV
	if ( @available( iOS 16.0, * ) )
		return true;
#endif
	return false;
}

bool ApplyLayerRange( CAMetalLayer *layer, bool extended )
{
	if ( @available( iOS 26.0, tvOS 26.0, * ) )
	{
		layer.preferredDynamicRange = extended ? CADynamicRangeHigh : CADynamicRangeStandard;
		// preferredDynamicRange engages EDR only for content tagged with
		// headroom above 1: the drawables are tagged with the display's
		// potential headroom, the most the output term writes, and are
		// not tone mapped again (the core's output term already mapped
		// them to the current headroom).
		layer.contentsHeadroom = extended ? ContentsHeadroom() : 0.0;
#if TARGET_OS_TV
		// The declared headroom is mapped to the television by tvOS.
		layer.toneMapMode = CAToneMapModeAutomatic;
#else
		layer.toneMapMode = extended ? CAToneMapModeNever : CAToneMapModeAutomatic;
#endif
		return true;
	}
#if !TARGET_OS_TV
	if ( @available( iOS 16.0, * ) )
	{
		layer.wantsExtendedDynamicRangeContent = extended ? YES : NO;
		return true;
	}
#endif
	return false;
}

// Keeps an extended-range layer's tag at the display's potential headroom,
// which changes with the display (a tvOS mode switch). Main thread only.
void RetagLayer( SDL_Window *window )
{
	if ( @available( iOS 26.0, tvOS 26.0, * ) )
	{
		CAMetalLayer *layer = MetalLayer( window );
		if ( layer && [layer.preferredDynamicRange isEqualToString:CADynamicRangeHigh] &&
		     layer.contentsHeadroom != ContentsHeadroom() )
			layer.contentsHeadroom = ContentsHeadroom();
	}
}

#if TARGET_OS_TV
UIWindow *WindowOf( SDL_Window *window )
{
	return window ? (UIWindow *)SDL_GetPointerProperty( SDL_GetWindowProperties( window ),
	                    SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr )
	              : nil;
}

// Display criteria for HDR10: a 10-bit HEVC format in BT.2020 with the PQ
// transfer, at the screen's refresh rate. Returned retained (no ARC here).
AVDisplayCriteria *CreateHdr10Criteria( UIScreen *screen )
{
	NSDictionary *extensions = @{
		(__bridge NSString *)kCMFormatDescriptionExtension_ColorPrimaries :
		    (__bridge NSString *)kCMFormatDescriptionColorPrimaries_ITU_R_2020,
		(__bridge NSString *)kCMFormatDescriptionExtension_TransferFunction :
		    (__bridge NSString *)kCMFormatDescriptionTransferFunction_SMPTE_ST_2084_PQ,
		(__bridge NSString *)kCMFormatDescriptionExtension_YCbCrMatrix :
		    (__bridge NSString *)kCMFormatDescriptionYCbCrMatrix_ITU_R_2020,
		(__bridge NSString *)kCMFormatDescriptionExtension_BitsPerComponent : @10,
	};
	const CGSize size = screen.nativeBounds.size;
	CMVideoFormatDescriptionRef format = nullptr;
	if ( CMVideoFormatDescriptionCreate( kCFAllocatorDefault, kCMVideoCodecType_HEVC,
	         (int32_t)size.width, (int32_t)size.height, (__bridge CFDictionaryRef)extensions,
	         &format ) != noErr ||
	     !format )
		return nil;
	const float refresh = screen.maximumFramesPerSecond > 0 ? (float)screen.maximumFramesPerSecond
	                                                        : 60.0f;
	AVDisplayCriteria *criteria = [[AVDisplayCriteria alloc] initWithRefreshRate:refresh
	                                                          formatDescription:format];
	CFRelease( format );
	return criteria;
}

// Whether the window's criteria ask for HDR (set by this file).
bool AsksForHdr( AVDisplayManager *manager )
{
	AVDisplayCriteria *criteria = manager.preferredDisplayCriteria;
	return criteria != nil;
}

// Whether the television is in HDR10 at this window's request: the window
// asks for it, tvOS matches the display's range, the display plays HDR10 and
// no switch is under way. Main thread only.
bool TvInHdrMode( SDL_Window *window )
{
	UIWindow *uiWindow = WindowOf( window );
	AVDisplayManager *manager = uiWindow ? uiWindow.avDisplayManager : nil;
	return manager && AsksForHdr( manager ) && manager.displayCriteriaMatchingEnabled &&
	       !manager.displayModeSwitchInProgress &&
	       ( [AVPlayer availableHDRModes] & AVPlayerHDRModeHDR10 ) != 0;
}

// Asks tvOS for the HDR mode, or withdraws the request. Main thread only.
void ApplyDisplayCriteria( UIWindow *uiWindow, bool hdr )
{
	AVDisplayManager *manager = uiWindow ? uiWindow.avDisplayManager : nil;
	if ( !manager || AsksForHdr( manager ) == hdr )
		return;
	if ( !hdr )
	{
		manager.preferredDisplayCriteria = nil;
		return;
	}
	AVDisplayCriteria *criteria = CreateHdr10Criteria( uiWindow.screen );
	manager.preferredDisplayCriteria = criteria;
	[criteria release];
}
#endif

} // namespace

bool Sdl3CanShowExtendedRange( SDL_Window *window )
{
	if ( !window || !SupportsLayerRange() )
		return false;
	if ( [NSThread isMainThread] )
		RefreshHeadroom( window );
#if TARGET_OS_TV
	// A display in SDR may still switch to HDR when asked.
	UIWindow *uiWindow = [NSThread isMainThread] ? WindowOf( window ) : nil;
	if ( uiWindow && uiWindow.avDisplayManager.displayCriteriaMatchingEnabled )
		return true;
#endif
	return g_PotentialHeadroom.load( std::memory_order_relaxed ) > 1.0f;
}

bool Sdl3SetExtendedRange( SDL_Window *window, bool extended )
{
	if ( !SupportsLayerRange() )
		return !extended;
	if ( [NSThread isMainThread] )
	{
		CAMetalLayer *layer = MetalLayer( window );
#if TARGET_OS_TV
		ApplyDisplayCriteria( WindowOf( window ), extended );
#endif
		return layer && ApplyLayerRange( layer, extended );
	}
	// UIKit objects belong to the main thread; the layer changes there, in order
	// with the window's other main-thread work. The window is found again by ID
	// there, since it may be destroyed before the block runs.
	const SDL_WindowID id = SDL_GetWindowID( window );
	dispatch_async( dispatch_get_main_queue(), ^{
		SDL_Window *found = SDL_GetWindowFromID( id );
		CAMetalLayer *layer = MetalLayer( found );
#if TARGET_OS_TV
		ApplyDisplayCriteria( WindowOf( found ), extended );
#endif
		if ( layer )
			ApplyLayerRange( layer, extended );
	} );
	return id != 0;
}

Sdl3DisplayHeadroom Sdl3ReadHeadroom( SDL_Window *window )
{
	if ( [NSThread isMainThread] )
	{
		RefreshHeadroom( window );
		RetagLayer( window );
	}
	else
	{
		const SDL_WindowID id = SDL_GetWindowID( window );
		dispatch_async( dispatch_get_main_queue(), ^{
			SDL_Window *found = SDL_GetWindowFromID( id );
			RefreshHeadroom( found );
			RetagLayer( found );
		} );
	}
	Sdl3DisplayHeadroom headroom;
	headroom.current = g_CurrentHeadroom.load( std::memory_order_relaxed );
	headroom.potential = g_PotentialHeadroom.load( std::memory_order_relaxed );
	return headroom;
}

Sdl3LayerRange Sdl3ReadLayerRange( SDL_Window *window )
{
	Sdl3LayerRange report;
	CAMetalLayer *layer = [NSThread isMainThread] ? MetalLayer( window ) : nil;
	if ( !layer )
		return report;
	report.known = true;
	if ( @available( iOS 26.0, tvOS 26.0, * ) )
		report.extended = [layer.preferredDynamicRange isEqualToString:CADynamicRangeHigh];
#if !TARGET_OS_TV
	else if ( @available( iOS 16.0, * ) )
		report.extended = layer.wantsExtendedDynamicRangeContent == YES;
#endif
	CGColorSpaceRef space = layer.colorspace;
	CFStringRef name = space ? CGColorSpaceCopyName( space ) : nullptr;
	if ( name )
	{
		report.extendedLinearColorspace =
		    CFStringCompare( name, kCGColorSpaceExtendedLinearSRGB, 0 ) == kCFCompareEqualTo;
		CFRelease( name );
	}
	return report;
}

Sdl3DisplayMode Sdl3ReadDisplayMode( SDL_Window *window )
{
	Sdl3DisplayMode mode;
#if TARGET_OS_TV
	UIWindow *uiWindow = [NSThread isMainThread] ? WindowOf( window ) : nil;
	AVDisplayManager *manager = uiWindow ? uiWindow.avDisplayManager : nil;
	if ( !manager )
		return mode;
	mode.known = true;
	mode.matchingEnabled = manager.displayCriteriaMatchingEnabled == YES;
	mode.switching = manager.displayModeSwitchInProgress == YES;
	mode.askedForHdr = AsksForHdr( manager );
	const AVPlayerHDRMode modes = [AVPlayer availableHDRModes];
	mode.hdrModes = ( ( modes & AVPlayerHDRModeHLG ) ? 1u : 0u ) |
	                ( ( modes & AVPlayerHDRModeHDR10 ) ? 2u : 0u ) |
	                ( ( modes & AVPlayerHDRModeDolbyVision ) ? 4u : 0u );
	mode.eligibleForHdr = [AVPlayer eligibleForHDRPlayback] == YES;
#else
	(void)window;
#endif
	return mode;
}

} // namespace render_vulkan

#endif // __APPLE__
