#import "MLTimer.h"

@implementation MLTimer

@synthesize timeout;
@synthesize userInfo;

- (void) onTick: (NSTimer *) timer
{
	if (!timeout (userInfo))
		[timer invalidate];
}

@end
