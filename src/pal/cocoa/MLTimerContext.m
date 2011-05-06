#include "config.h"
#import "MLTimerContext.h"

@implementation MLTimerContext

- (id) initWithWindowingSystem: (Moonlight::MoonWindowingSystemCocoa*) winsys
{
	windowingSystem = winsys;
	return self;
}

- (void) onTick: (NSTimer *) timer
{
	windowingSystem->OnTick ();
}

@end
