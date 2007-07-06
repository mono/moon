/*
 * enums.h: various enumerated types
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_ENUMS_H__
#define __MOON_ENUMS_H__

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
	VisibilityCollapsed, /* Not handled, treated like Hidden */
	VisibilityHidden
};

enum CollectionChangeType {
	CollectionChangeTypeItemAdded,
	CollectionChangeTypeItemRemoved,
	CollectionChangeTypeItemChanged,

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

#endif
