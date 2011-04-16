#include "runtime.h"
#import "MLView.h"

@implementation MLView

@synthesize moonwindow;

-(void) drawRect: (NSRect) rect
{
	moonwindow->ExposeEvent (Moonlight::Rect (rect.origin.x, rect.origin.y, rect.size.width, rect.size.height));
}

- (void) mouseDown: (NSEvent *) event
{
	moonwindow->ButtonPressEvent (event);
}

- (void) mouseUp: (NSEvent *) event
{
	moonwindow->ButtonReleaseEvent (event);
}

- (void) mouseMoved: (NSEvent *) event
{
	moonwindow->MotionEvent (event);
}

- (void) mouseEntered: (NSEvent *) event
{
	moonwindow->MouseEnteredEvent (event);
}

- (void) mouseExited: (NSEvent *) event
{
	moonwindow->MouseExitedEvent (event);
}

- (void)keyDown: (NSEvent *) event
{
	moonwindow->KeyDownEvent (event);
}

- (void)keyUp: (NSEvent *) event
{
	moonwindow->KeyUpEvent (event);
}

- (void) setFrame: (NSRect) frame {
	[super setFrame: frame];
	[self removeTrackingRect: trackingrect];
	trackingrect = [self addTrackingRect: frame owner: self userData: NULL assumeInside: NO];

	moonwindow->SetWidthInternal (frame.size.width);
	moonwindow->SetHeightInternal (frame.size.height);

	if (moonwindow->GetSurface ())
		moonwindow->GetSurface ()->HandleUIWindowAllocation (true);
}

- (BOOL) acceptsFirstMouse: (NSEvent *) event
{
	return YES;
}

- (BOOL) acceptsFirstResponder
{
	return YES;
}

@end
