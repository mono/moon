#import "window-cocoa.h"
#import <AppKit/AppKit.h>

@interface MLView : NSView {
	Moonlight::MoonWindowCocoa *moonwindow;
	NSTrackingRectTag trackingrect;
}

@property Moonlight::MoonWindowCocoa *moonwindow;
@end

