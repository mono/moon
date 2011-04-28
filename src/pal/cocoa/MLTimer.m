#import "MLTimer.h"

@implementation MLTimer

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
