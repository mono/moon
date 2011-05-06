#include "config.h"
#include "runtime.h"
#import "MLWindow.h"

@implementation MLWindow

@synthesize moonwindow;

-(void) makeKeyAndOrderFront: (id) sender
{
	[super makeKeyAndOrderFront: sender];

	if (moonwindow->GetSurface ()) {
		moonwindow->GetSurface ()->HandleUIWindowAvailable ();
		moonwindow->GetSurface ()->HandleUIWindowAllocation (true);
	}
}
@end
