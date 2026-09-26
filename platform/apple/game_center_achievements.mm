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

// One console line per Game Center result, "Source: game-center: <event>
// key=value ...", which tools/quality/game_center_e2e.py reads. Errors carry
// GameKit's domain and code (GKErrorDomain 6: no signed-in player; 15: the app
// is not registered with Game Center).
NSString *ErrorText( NSError *error )
{
	return error ? [NSString stringWithFormat:@"error=%@:%ld \"%@\"", error.domain,
	                                          static_cast<long>( error.code ),
	                                          error.localizedDescription]
	             : @"error=none";
}

NSString *JoinIds( const std::vector<std::string> &ids )
{
	std::string joined;
	for ( const std::string &id : ids )
		joined += ( joined.empty() ? "" : "," ) + id;
	return [NSString stringWithUTF8String:joined.c_str()];
}

// Runs the main run loop, where GameKit completes, until `done` or timeout.
bool PumpUntil( bool ( ^done )( void ), double seconds )
{
	const CFAbsoluteTime deadline = CFAbsoluteTimeGetCurrent() + seconds;
	while ( !done() )
	{
		if ( CFAbsoluteTimeGetCurrent() > deadline )
			return false;
		CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0.05, true );
	}
	return true;
}

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
		  const bool signedIn = GKLocalPlayer.localPlayer.isAuthenticated;
		  NSLog( @"Source: game-center: sign-in authenticated=%d %@", signedIn ? 1 : 0,
		      ErrorText( error ) );
		  ( *notify )( signedIn );
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
		// The block retains what it captures when GameKit copies it.
		NSString *list = JoinIds( ids );
		[GKAchievement reportAchievements:achievements
		            withCompletionHandler:^( NSError *error ) {
			          NSLog( @"Source: game-center: submit ids=%@ result=%@ %@", list,
			              error ? @"refused" : @"accepted", ErrorText( error ) );
			          ( *completion )( error == nil );
		            }];
	}
}

bool GameCenterAchievements::Present()
{
	if ( !GKLocalPlayer.localPlayer.isAuthenticated )
	{
		NSLog( @"Source: game-center: present result=unavailable" );
		return false;
	}
	[GKAccessPoint.shared triggerAccessPointWithState:GKGameCenterViewControllerStateAchievements
	                                          handler:^{
	                                          }];
	NSLog( @"Source: game-center: present result=requested" );
	// Whether the screen actually came up, for the end-to-end test.
	dispatch_after( dispatch_time( DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC ), dispatch_get_main_queue(), ^{
		NSLog( @"Source: game-center: presenting=%d",
		    GKAccessPoint.shared.isPresentingGameCenter ? 1 : 0 );
	} );
	return true;
}

bool GameCenterAchievements::AnnouncesCompletions() const
{
	return true;
}

void AuditGameCenter( double timeoutSeconds )
{
	@autoreleasepool
	{
		const bool signedIn =
		    PumpUntil( ^bool { return GKLocalPlayer.localPlayer.isAuthenticated; }, timeoutSeconds );
		NSLog( @"Source: game-center-audit: authenticated=%d", signedIn ? 1 : 0 );

		__block bool descriptionsDone = false;
		[GKAchievementDescription loadAchievementDescriptionsWithCompletionHandler:^(
		    NSArray<GKAchievementDescription *> *descriptions, NSError *error ) {
			for ( GKAchievementDescription *description in descriptions )
				NSLog( @"Source: game-center-audit: description id=%@", description.identifier );
			NSLog( @"Source: game-center-audit: descriptions count=%lu %@",
			    static_cast<unsigned long>( descriptions.count ), ErrorText( error ) );
			descriptionsDone = true;
		}];
		if ( !PumpUntil( ^bool { return descriptionsDone; }, timeoutSeconds ) )
			NSLog( @"Source: game-center-audit: descriptions timeout=1" );

		__block bool achievementsDone = false;
		[GKAchievement loadAchievementsWithCompletionHandler:^(
		    NSArray<GKAchievement *> *achievements, NSError *error ) {
			for ( GKAchievement *achievement in achievements )
				NSLog( @"Source: game-center-audit: achievement id=%@ percent=%.1f completed=%d",
				    achievement.identifier, achievement.percentComplete,
				    achievement.isCompleted ? 1 : 0 );
			NSLog( @"Source: game-center-audit: achievements count=%lu %@",
			    static_cast<unsigned long>( achievements.count ), ErrorText( error ) );
			achievementsDone = true;
		}];
		if ( !PumpUntil( ^bool { return achievementsDone; }, timeoutSeconds ) )
			NSLog( @"Source: game-center-audit: achievements timeout=1" );
		NSLog( @"Source: game-center-audit: done" );
	}
}

} // namespace platform
