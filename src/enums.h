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


/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum AlignmentX {
	AlignmentXLeft,
	AlignmentXCenter,
	AlignmentXRight
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum AlignmentY {
	AlignmentYTop,
	AlignmentYCenter,
	AlignmentYBottom
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum BrushMappingMode {
	BrushMappingModeAbsolute,
	BrushMappingModeRelativeToBoundingBox
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum ColorInterpolationMode {
	ColorInterpolationModeScRgbLinearInterpolation,
	ColorInterpolationModeSRgbLinearInterpolation
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum GradientSpreadMethod {
	GradientSpreadMethodPad,
	GradientSpreadMethodReflect,
	GradientSpreadMethodRepeat
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum ElevatedPermissions {
	ElevatedPermissionsNone,
	ElevatedPermissionsRequired
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum SystemColor {
	ActiveBorderColor,
	ActiveCaptionColor,
	ActiveCaptionTextColor,
	AppWorkspaceColor,
	ControlColor,
	ControlDarkColor,
	ControlDarkDarkColor,
	ControlLightColor,
	ControlLightLightColor,
	ControlTextColor,
	DesktopColor,
	GrayTextColor,
	HighlightColor,
	HighlightTextColor,
	InactiveBorderColor,
	InactiveCaptionColor,
	InactiveCaptionTextColor,
	InfoColor,
	InfoTextColor,
	MenuColor,
	MenuTextColor,
	ScrollBarColor,
	WindowColor,
	WindowFrameColor,
	WindowTextColor,
	NumSystemColors
};

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

/* @IncludeInKinds */
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

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum LineStackingStrategy {
	LineStackingStrategyMaxHeight,
	LineStackingStrategyBlockLineHeight
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum InstallState {
	InstallStateNotInstalled,
	InstallStateInstalling,
	InstallStateInstalled,
	InstallStateInstallFailed,
	InstallStateUnknown  // not a valid value in managed land
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum MediaElementState {
	MediaElementStateClosed,
	MediaElementStateOpening,
	MediaElementStateBuffering,
	MediaElementStatePlaying,
	MediaElementStatePaused,
	MediaElementStateStopped,
	MediaElementStateIndividualizing,
	MediaElementStateAcquiringLicense,
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum StyleSimulations {
	StyleSimulationsNone       = 0,
	StyleSimulationsBold       = (1 << 0),
	StyleSimulationsItalic     = (1 << 1),
	StyleSimulationsBoldItalic = (StyleSimulationsBold | StyleSimulationsItalic),
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum TextAlignment {
	TextAlignmentCenter,
	TextAlignmentLeft,
	TextAlignmentRight
};

// TextDecorations would appear to be a collection of bit flags rather
// than a normal enumeration of values
/* @IncludeInKinds */
enum TextDecorations {
	TextDecorationsNone      = 0,
	TextDecorationsUnderline = (1 << 0)
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum TextWrapping {
	TextWrappingWrapWithOverflow,
	TextWrappingNoWrap,
	TextWrappingWrap,
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum Stretch {
	StretchNone,
	StretchFill,
	StretchUniform,
	StretchUniformToFill
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum PenLineCap {
	PenLineCapFlat,
	PenLineCapSquare,
	PenLineCapRound,
	PenLineCapTriangle
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum PenLineJoin {
	PenLineJoinMiter,
	PenLineJoinBevel,
	PenLineJoinRound
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum FillRule {
	FillRuleEvenOdd,
	FillRuleNonzero
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum SweepDirection {
	SweepDirectionCounterclockwise,
	SweepDirectionClockwise
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum Visibility {
	VisibilityVisible,
	VisibilityCollapsed
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Controls */
enum Orientation {
	OrientationVertical,
	OrientationHorizontal
};

// FIXME: Rename this to match the managed enum. Then add the 'Namespace' attribute to the enum
// make sure this stays in sync with System.Windows/System.Windows.Input/Cursor.cs (CursorType enum)
/* @IncludeInKinds */
/* @Namespace=System.Windows.Input */
enum CursorType {
	CursorTypeDefault,
	CursorTypeArrow,
	CursorTypeHand,
	CursorTypeWait,
	CursorTypeIBeam,
	CursorTypeStylus,
	CursorTypeEraser,
	CursorTypeSizeNS,
	CursorTypeSizeWE,
	CursorTypeNone	
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Input */
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

/* @IncludeInKinds */
/* @Namespace=System.Windows.Input */
enum TabletDeviceType {
	TabletDeviceTypeMouse,
	TabletDeviceTypeStylus,
	TabletDeviceTypeTouch
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Input */
enum TouchAction {
	TouchActionDown = 1,
	TouchActionMove = 2,
	TouchActionUp   = 3
};

// Silverlight 2.0 Enums:

/* @IncludeInKinds */
/* @Namespace=System.Windows.Data */
enum BindingMode {
	BindingModeOneWay  = 1,
	BindingModeOneTime = 2,
	BindingModeTwoWay  = 3
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum GridUnitType {
       GridUnitTypeAuto,
       GridUnitTypePixel,
       GridUnitTypeStar
};


/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum HorizontalAlignment {
	HorizontalAlignmentLeft,
	HorizontalAlignmentCenter,
	HorizontalAlignmentRight,
	HorizontalAlignmentStretch
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Input */
enum KeyboardNavigationMode {
	KeyboardNavigationModeLocal,
	KeyboardNavigationModeCycle,
	KeyboardNavigationModeOnce
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Input */
enum ModifierKeys {
	ModifierKeyNone     = 0,
	ModifierKeyAlt      = (1 << 0),
	ModifierKeyControl  = (1 << 1),
	ModifierKeyShift    = (1 << 2),
	ModifierKeyWindows  = (1 << 3),
	ModifierKeyApple    = (1 << 3)
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Controls */
enum ScrollBarVisibility {
	ScrollBarVisibilityDisabled,
	ScrollBarVisibilityAuto,
	ScrollBarVisibilityHidden,
	ScrollBarVisibilityVisible
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum VerticalAlignment {
	VerticalAlignmentTop,
	VerticalAlignmentCenter,
	VerticalAlignmentBottom,
	VerticalAlignmentStretch
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum CrossDomainAccess {
	CrossDomainAccessNoAccess = 0,
	// CrossDomainAccessFullAccess (1) was removed before final SL2 release
	CrossDomainAccessScriptableOnly = 2,
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media.Animation */
enum FillBehavior {
	FillBehaviorHoldEnd,
	FillBehaviorStop
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media.Animation */
enum EasingMode {
	EasingModeOut,
	EasingModeIn,
	EasingModeInOut
};

// Silverlight 3.0 enums

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
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

// Silverlight 4.0 enums

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum FlowDirection {
	FlowDirectionLeftToRight,
	FlowDirectionRightToLeft
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Controls */
enum StretchDirection {
	StretchDirectionUpOnly,
	StretchDirectionDownOnly,
	StretchDirectionBoth,
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Input */
enum ImeConversionModeValues {
	ImeConversionModeValuesNative = 1,
	ImeConversionModeValuesKatakana = 2,
	ImeConversionModeValuesFullShape = 4,
	ImeConversionModeValuesRoman = 8,
	ImeConversionModeValuesCharCode = 16,
	ImeConversionModeValuesNoConversion = 32,
	ImeConversionModeValuesEudc = 64,
	ImeConversionModeValuesSymbol = 128,
	ImeConversionModeValuesFixed = 256,
	ImeConversionModeValuesAlphanumeric = 512,
	ImeConversionModeValuesDoNotCare = INT_MIN,
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Input */
enum InputMethodState {
	InputMethodStateOff,
	InputMethodStateOn,
	InputMethodStateDoNotCare,
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum WindowState {
	WindowStateNormal,
	WindowStateMinimized,
	WindowStateMaximized,
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum WindowStartupLocation {
	WindowStartupLocationCenterScreen,
	WindowStartupLocationManual,
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum WindowStyle {
	WindowStyleSingleBorderWindow,
	WindowStyleNone,
	WindowStyleBorderlessRoundCornersWindow,
};

/* @IncludeInKinds */
/* @Namespace=System.Windows */
enum TextTrimming {
	TextTrimmingNone = 0,
	TextTrimmingWordEllipsis = 2,
};

// enums used in the pipeline
// there is no string <-> enum conversions for these

enum MediaStreamSourceDiagnosticKind {
    BufferLevelInMilliseconds = 1,
    BufferLevelInBytes = 2
};

enum MoonPixelFormat {
	MoonPixelFormatNone = 0,
	MoonPixelFormatRGBA32 = 8,
	MoonPixelFormatRGB32,
	MoonPixelFormatYUV420P
};

enum MoonWaveFormatType {
	MoonWaveFormatTypePCM = 1
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

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
enum TextHintingMode {
	TextHintingModeFixed,
	TextHintingModeAnimated
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media.Imaging */
enum BitmapCreateOptions {
	BitmapCreateOptionsNone = 0,
	BitmapCreateOptionsDelayCreation = 2,
	BitmapCreateOptionsIgnoreImageCache = 8
};

enum ReceiverNameScope {
	ReceiverNameScopeDomain,
	ReceiverNameScopeGlobal
};

G_BEGIN_DECLS

int enums_str_to_int (const char *prop_name, const char *str);
const char *enums_int_to_str (const char *prop_name, int e);
bool enums_is_enum_name (const char *enum_name);

G_END_DECLS

#endif
