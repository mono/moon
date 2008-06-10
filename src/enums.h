/*
 * enums.c: various enumerated types + enum -> str helpers
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_ENUMS_H__
#define __MOON_ENUMS_H__

#include <glib.h>

enum ErrorType {
	NoError,
	UnknownError,
	InitializeError,
	ParserError,
	ObjectModelError,
	RuntimeError,
	DownloadError,
	MediaError,
	ImageError
};

enum Stretch {
	StretchNone,
	StretchFill,
	StretchUniform,
	StretchUniformToFill
};

enum PenLineCap {
	PenLineCapFlat,
	PenLineCapSquare,
	PenLineCapRound,
	PenLineCapTriangle
};

enum PenLineJoin {
	PenLineJoinMiter,
	PenLineJoinBevel,
	PenLineJoinRound
};

enum FillRule {
	FillRuleEvenOdd,
	FillRuleNonzero
};

enum SweepDirection {
	SweepDirectionCounterclockwise,
	SweepDirectionClockwise
};

enum Visibility {
	VisibilityVisible,
	VisibilityCollapsed
};

enum CollectionChangeType {
	CollectionChangeTypeItemAdded,
	CollectionChangeTypeItemRemoved,
	CollectionChangeTypeItemChanged,

	CollectionChangeTypeChanging,
	CollectionChangeTypeChanged
};

enum MouseCursor {
	MouseCursorDefault,
	MouseCursorArrow,
	MouseCursorHand,
	MouseCursorWait,
	MouseCursorIBeam,
	MouseCursorStylus,
	MouseCursorEraser,
	MouseCursorNone
};

enum Key {
	KeyKEYNONE = 0,
	KeyBACKSPACE = 1,
	KeyTAB = 2,
	KeyENTER = 3,
	KeySHIFT = 4,
	KeyCTRL = 5,
	KeyALT = 6,
	KeyCAPSLOCK = 7,
	KeyESCAPE = 8,
	KeySPACE = 9,
	KeyPAGEUP = 10,
	KeyPAGEDOWN = 11,
	KeyEND = 12,
	KeyHOME = 13,
	KeyLEFT = 14,
	KeyUP = 15,
	KeyRIGHT = 16,
	KeyDOWN = 17,
	KeyINSERT = 18,
	KeyDELETE = 19,
	KeyDIGIT0 = 20,
	KeyDIGIT1 = 21,
	KeyDIGIT2 = 22,
	KeyDIGIT3 = 23,
	KeyDIGIT4 = 24,
	KeyDIGIT5 = 25,
	KeyDIGIT6 = 26,
	KeyDIGIT7 = 27,
	KeyDIGIT8 = 28,
	KeyDIGIT9 = 29,
	KeyA = 30,
	KeyB = 31,
	KeyC = 32,
	KeyD = 33,
	KeyE = 34,
	KeyF = 35,
	KeyG = 36,
	KeyH = 37,
	KeyI = 38,
	KeyJ = 39,
	KeyK = 40,
	KeyL = 41,
	KeyM = 42,
	KeyN = 43,
	KeyO = 44,
	KeyP = 45,
	KeyQ = 46,
	KeyR = 47,
	KeyS = 48,
	KeyT = 49,
	KeyU = 50,
	KeyV = 51,
	KeyW = 52,
	KeyX = 53,
	KeyY = 54,
	KeyZ = 55,
	KeyF1 = 56,
	KeyF2 = 57,
	KeyF3 = 58,
	KeyF4 = 59,
	KeyF5 = 60,
	KeyF6 = 61,
	KeyF7 = 62,
	KeyF8 = 63,
	KeyF9 = 64,
	KeyF10 = 65,
	KeyF11 = 66,
	KeyF12 = 67,
	KeyNUMPAD0 = 68,
	KeyNUMPAD1 = 69,
	KeyNUMPAD2 = 70,
	KeyNUMPAD3 = 71,
	KeyNUMPAD4 = 72,
	KeyNUMPAD5 = 73,
	KeyNUMPAD6 = 74,
	KeyNUMPAD7 = 75,
	KeyNUMPAD8 = 76,
	KeyNUMPAD9 = 77,
	KeyMULTIPLY = 78,
	KeyADD = 79,
	KeySUBTRACT = 80,
	KeyDECIMAL = 81,
	KeyDIVIDE = 82,
	KeyKEYUNKNOWN = 255
};

G_BEGIN_DECLS

int		enums_str_to_int (const char *prop_name, const char *str);
const char*	enums_int_to_str (const char *prop_name, int e);

G_END_DECLS

#endif
