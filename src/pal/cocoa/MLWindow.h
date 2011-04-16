/* @SkipFile */
#import "window-cocoa.h"
#import <AppKit/AppKit.h>

@interface MLWindow : NSWindow {
	Moonlight::MoonWindowCocoa *moonwindow;
}

@property Moonlight::MoonWindowCocoa *moonwindow;
@end
