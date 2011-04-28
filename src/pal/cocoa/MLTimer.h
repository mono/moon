/* @SkipFile */
#import "pal-cocoa.h"
#import <AppKit/AppKit.h>

@interface MLTimer : NSTimer {
	Moonlight::MoonWindowingSystemCocoa *windowingSystem;
}

- (id) initWithWindowingSystem: (Moonlight::MoonWindowingSystemCocoa*) windowingSystem;
- (void) onTick: (NSTimer *) theTimer;

@end

