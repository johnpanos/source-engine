//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Game Center achievement platform; see game_center_achievements.h.
//
//=============================================================================//

#include "game_center_achievements.h"

#import <GameKit/GameKit.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <utility>

namespace platform
{

namespace
{

// How often, and how many times, to look for a window to present Game
// Center's sign-in screen on while the engine has none yet.
constexpr int64_t kPresentRetryNanoseconds = 500 * NSEC_PER_MSEC;
constexpr int kPresentRetries = 120;

// The root view controller of the foreground key window, if there is one.
UIViewController *KeyRootViewController()
{
	for ( UIScene *scene in UIApplication.sharedApplication.connectedScenes )
	{
		if ( scene.activationState != UISceneActivationStateForegroundActive ||
		     ![scene isKindOfClass:[UIWindowScene class]] )
			continue;
		for ( UIWindow *window in static_cast<UIWindowScene *>( scene ).windows )
		{
			if ( window.isKeyWindow && window.rootViewController )
				return window.rootViewController;
		}
	}
	return nil;
}

// Presents `viewController` over whatever the root view controller already
// shows, retrying until the engine has created its window.
void PresentWhenPossible( UIViewController *viewController, int retriesLeft )
{
	UIViewController *host = KeyRootViewController();
	while ( host.presentedViewController )
		host = host.presentedViewController;
	if ( host )
	{
		[host presentViewController:viewController animated:YES completion:nil];
		return;
	}
	if ( retriesLeft == 0 )
		return;
	dispatch_after( dispatch_time( DISPATCH_TIME_NOW, kPresentRetryNanoseconds ),
	    dispatch_get_main_queue(), ^{
		  PresentWhenPossible( viewController, retriesLeft - 1 );
	    } );
}

} // namespace

void GameCenterAchievements::Authenticate( SignInChanged signInChanged )
{
	// The block copies the shared function; GameKit keeps the handler and
	// calls it on the main thread whenever the sign-in state changes.
	auto notify = std::make_shared<SignInChanged>( std::move( signInChanged ) );
	GKLocalPlayer.localPlayer.authenticateHandler =
	    ^( UIViewController *viewController, NSError *error ) {
		  if ( viewController )
		  {
			  PresentWhenPossible( viewController, kPresentRetries );
			  return;
		  }
		  if ( error )
			  NSLog( @"Source: Game Center sign-in unavailable: %@", error.localizedDescription );
		  ( *notify )( GKLocalPlayer.localPlayer.isAuthenticated );
	    };
}

void GameCenterAchievements::Submit( std::vector<std::string> ids, Completion done )
{
	@autoreleasepool
	{
		NSMutableArray<GKAchievement *> *achievements =
		    [NSMutableArray arrayWithCapacity:ids.size()];
		for ( const std::string &id : ids )
		{
			// Objective-C++ here is built without ARC.
			GKAchievement *achievement = [[[GKAchievement alloc]
			    initWithIdentifier:[NSString stringWithUTF8String:id.c_str()]] autorelease];
			achievement.percentComplete = 100.0;
			achievement.showsCompletionBanner = YES;
			[achievements addObject:achievement];
		}
		auto completion = std::make_shared<Completion>( std::move( done ) );
		[GKAchievement reportAchievements:achievements
		            withCompletionHandler:^( NSError *error ) {
			          if ( error )
				          NSLog( @"Source: Game Center refused achievements: %@",
				              error.localizedDescription );
			          ( *completion )( error == nil );
		            }];
	}
}

bool GameCenterAchievements::Present()
{
	if ( !GKLocalPlayer.localPlayer.isAuthenticated )
		return false;
	[GKAccessPoint.shared triggerAccessPointWithState:GKGameCenterViewControllerStateAchievements
	                                          handler:^{
	                                          }];
	return true;
}

bool GameCenterAchievements::AnnouncesCompletions() const
{
	return true;
}

} // namespace platform
