/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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

#include "moonbuild.h"

enum ErrorEventArgsType {
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

enum FontStretches {
	FontStretchesUltraCondensed = 1,
	FontStretchesExtraCondensed = 2,
	FontStretchesCondensed      = 3,
	FontStretchesSemiCondensed  = 4,
	FontStretchesNormal         = 5,
	FontStretchesMedium         = 5,
	FontStretchesSemiExpanded   = 6,
	FontStretchesExpanded       = 7,
	FontStretchesExtraExpanded  = 8,
	FontStretchesUltraExpanded  = 9
};

enum FontStyles {
	FontStylesNormal,
	FontStylesOblique,
	FontStylesItalic
};

enum FontWeights {
	FontWeightsThin       = 100,
	FontWeightsExtraLight = 200,
	FontWeightsLight      = 300,
	FontWeightsNormal     = 400,
	FontWeightsMedium     = 500,
	FontWeightsSemiBold   = 600,
	FontWeightsBold       = 700,
	FontWeightsExtraBold  = 800,
	FontWeightsBlack      = 900,
	FontWeightsExtraBlack = 950,
};

enum LineStackingStrategy {
	LineStackingStrategyMaxHeight,
	LineStackingStrategyBlockLineHeight
};

enum MediaState {
	MediaStateClosed,
	MediaStateOpening,
	MediaStateBuffering,
	MediaStatePlaying,
	MediaStatePaused,
	MediaStateStopped,
	MediaStateIndividualizing,
	MediaStateAcquiringLicense,
};

enum PixelFormats {
	PixelFormatBgr32 = 1,
	PixelFormatPbgra32 = 2
};

enum StyleSimulations {
	StyleSimulationsNone       = 0,
	StyleSimulationsBold       = (1 << 0),
	StyleSimulationsItalic     = (1 << 1),
	StyleSimulationsBoldItalic = (StyleSimulationsBold | StyleSimulationsItalic),
};

enum TextAlignment {
	TextAlignmentCenter,
	TextAlignmentLeft,
	TextAlignmentRight
};

// TextDecorations would appear to be a collection of bit flags rather
// than a normal enumeration of values
enum TextDecorations {
	TextDecorationsNone      = 0,
	TextDecorationsUnderline = (1 << 0)
};

enum TextWrapping {
	TextWrappingWrapWithOverflow,
	TextWrappingNoWrap,
	TextWrappingWrap,
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

enum Orientation {
	OrientationVertical,
	OrientationHorizontal
};

// make sure this stays in sync with System.Windows/System.Windows.Input/Cursor.cs (CursorType enum)
enum MouseCursor {
	MouseCursorDefault,
	MouseCursorArrow,
	MouseCursorHand,
	MouseCursorWait,
	MouseCursorIBeam,
	MouseCursorStylus,
	MouseCursorEraser,
	MouseCursorSizeNS,
	MouseCursorSizeWE,
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
	KeyUNKNOWN = 255
};


// Silverlight 2.0 Enums:

enum BindingMode {
	BindingModeOneWay  = 1,
	BindingModeOneTime = 2,
	BindingModeTwoWay  = 3
};

enum GridUnitType {
       GridUnitTypeAuto,
       GridUnitTypePixel,
       GridUnitTypeStar
};


enum HorizontalAlignment {
	HorizontalAlignmentLeft,
	HorizontalAlignmentCenter,
	HorizontalAlignmentRight,
	HorizontalAlignmentStretch
};

enum KeyboardNavigationMode {
	KeyboardNavigationModeLocal,
	KeyboardNavigationModeCycle,
	KeyboardNavigationModeOnce
};

enum ModifierKeys {
	ModifierKeyNone     = 0,
	ModifierKeyAlt      = (1 << 0),
	ModifierKeyControl  = (1 << 1),
	ModifierKeyShift    = (1 << 2),
	ModifierKeyWindows  = (1 << 3),
	ModifierKeyApple    = (1 << 3)
};

enum ScrollBarVisibility {
	ScrollBarVisibilityDisabled,
	ScrollBarVisibilityAuto,
	ScrollBarVisibilityHidden,
	ScrollBarVisibilityVisible
};

enum VerticalAlignment {
	VerticalAlignmentTop,
	VerticalAlignmentCenter,
	VerticalAlignmentBottom,
	VerticalAlignmentStretch
};

enum CrossDomainAccess {
	CrossDomainAccessNoAccess = 0,
	// CrossDomainAccessFullAccess (1) was removed before final SL2 release
	CrossDomainAccessScriptableOnly = 2,
};

enum FillBehavior {
	FillBehaviorHoldEnd,
	FillBehaviorStop
};

enum EasingMode {
	EasingModeIn,
	EasingModeOut,
	EasingModeInOut
};

// Silverlight 3.0 enums

enum LogSource {
	LogSourceRequestLog,
	LogSourceStop,
	LogSourceSeek,
	LogSourcePause,
	LogSourceSourceChanged,
	LogSourceEndOfStream,
	LogSourceMediaElementShutdown,
	LogSourceRuntimeShutdown
};


// enums used in the pipeline
// there is no string <-> enum conversions for these

enum MediaSourceType {
	MediaSourceTypeFile = 1,
	MediaSourceTypeLive = 2,
	MediaSourceTypeProgressive = 3,
	MediaSourceTypeMemory = 4,
	MediaSourceTypeMms = 5,
	MediaSourceTypeMmsEntry = 6,
	MediaSourceTypeManagedStream = 7,
};

enum MediaStreamSourceDiagnosticKind {
    BufferLevelInMilliseconds = 1,
    BufferLevelInBytes = 2
};

enum MoonPixelFormat {
	MoonPixelFormatNone = 0,
	MoonPixelFormatRGB32,
	MoonPixelFormatRGBA32,
	MoonPixelFormatYUV420P
};

enum MediaStreamType {
	MediaTypeAudio = 0,
	MediaTypeVideo = 1,
	MediaTypeMarker = 2
};

enum MediaFrameState {
	MediaFramePlanar    = 1 << 0,
	MediaFrameDecoded   = 1 << 1,
	MediaFrameDemuxed   = 1 << 2,
	MediaFrameConverted = 1 << 3,
	MediaFrameKeyFrame  = 1 << 4,
	MediaFrameMarker    = 1 << 5,
};

enum TextHintingMode {
	TextHintingModeFixed,
	TextHintingModeAnimated
};

enum BitmapCreateOptions {
	BitmapCreateOptionsNone = 0,
	BitmapCreateOptionsDelayCreation = 2,
	BitmapCreateOptionsIgnoreImageCache = 8
};

G_BEGIN_DECLS

MOON_API int enums_str_to_int (const char *prop_name, const char *str);
MOON_API const char *enums_int_to_str (const char *prop_name, int e);
bool enums_is_enum_name (const char *enum_name);

G_END_DECLS

#endif
