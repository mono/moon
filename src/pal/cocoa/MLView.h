/* @SkipFile */
#import "window-cocoa.h"
#import <AppKit/AppKit.h>

@interface MLView : NSView {
	Moonlight::MoonWindowCocoa *moonwindow;
	NSTrackingRectTag trackingrect;
	NSOpenGLContext *openGLContext;
}

@property Moonlight::MoonWindowCocoa *moonwindow;

-(void) setOpenGLContext: (NSOpenGLContext *)context;
@end

