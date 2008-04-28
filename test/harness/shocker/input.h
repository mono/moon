

#ifndef __INPUT_H__
#define __INPUT_H__

#include <glib.h>

#define Visual _XVisual
#include <X11/X.h>
#include <X11/extensions/XTest.h>
#undef Visual


#include "netscape.h"


class InputProvider {

public:
	InputProvider ();
	virtual ~InputProvider ();

	void MoveMouseLogarithmic (int x, int y);
	void MoveMouse (int x, int y);
	void MouseLeftClick ();
	void MouseRightClick ();
	void MouseLeftButtonDown ();
	void MouseLeftButtonUp ();

	bool MouseIsAtPosition (int x, int y);

	void SendKeyInput (uint32 key_code, bool key_down);
private:
	Display *display;
	Window root_window;
	bool xtest_available;

	GSList* down_keys;

	void GetCursorPos (int &x, int &y);
	int MapToKeysym (int key);
};

#endif  // __INPUT_H__

