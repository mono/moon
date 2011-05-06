#include "config.h"
#include "runtime.h"
#import "MLEvent.h"

@implementation MLEvent

@synthesize view;
@synthesize event;

- (id) initWithEvent: (NSEvent *) evt view: (MLView *) v
{
	self.view = v;
	self.event = evt;

	return self;
}

@end
