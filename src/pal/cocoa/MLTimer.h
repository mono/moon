/* @SkipFile */
#import "window-cocoa.h"
#import <AppKit/AppKit.h>

@interface MLTimer : NSTimer {
	Moonlight::MoonSourceFunc timeout;
	gpointer userInfo;
}

@property Moonlight::MoonSourceFunc timeout;
@property gpointer userInfo;

- (void) onTick: (NSTimer *) theTimer;

@end

