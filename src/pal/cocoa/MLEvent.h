/* @SkipFile */
#import "MLView.h"
#import <AppKit/AppKit.h>

@interface MLEvent : NSObject {
	NSEvent *event;
	MLView *view;
}

@property(assign) NSEvent *event;
@property(assign) MLView *view;

- (id) initWithEvent: (NSEvent *) event view: (MLView *) view;
@end

