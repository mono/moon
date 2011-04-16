/* @SkipFile */
#import "MLView.h"
#import <AppKit/AppKit.h>

@interface MLEvent : NSObject {
	NSEvent *event;
	MLView *view;
}

@property(retain) NSEvent *event;
@property(retain) MLView *view;

- (id) initWithEvent: (NSEvent *) event view: (MLView *) view;
@end

