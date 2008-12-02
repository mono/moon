// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// CSS style property names.
    /// </summary>
    [SuppressMessage("Microsoft.Naming", "CA1711:IdentifiersShouldNotHaveIncorrectSuffix", Justification = "These represent actual CSS attributes (as in, XML attributes).")]
    public enum CssAttribute
    {
        /// <summary>
        /// The CSS property BackgroundAttachment.
        /// </summary>
        BackgroundAttachment,

        /// <summary>
        /// The CSS property BackgroundColor.
        /// </summary>
        BackgroundColor,

        /// <summary>
        /// The CSS property BackgroundImage.
        /// </summary>
        BackgroundImage,

        /// <summary>
        /// The CSS property BackgroundPosition.
        /// </summary>
        BackgroundPosition,

        /// <summary>
        /// The CSS property BackgroundRepeat.
        /// </summary>
        BackgroundRepeat,

        /// <summary>
        /// The CSS property Background.
        /// </summary>
        Background,

        /// <summary>
        /// The CSS pBorder property.
        /// </summary>
        Border,

        /// <summary>
        /// The CSS property BorderTop.
        /// </summary>
        BorderTop,

        /// <summary>
        /// The CSS property BorderTopColor.
        /// </summary>
        BorderTopColor,

        /// <summary>
        /// The CSS property BorderTopStyle.
        /// </summary>
        BorderTopStyle,

        /// <summary>
        /// The CSS property BorderTopWidth.
        /// </summary>
        BorderTopWidth,

        /// <summary>
        /// The CSS property BorderRight.
        /// </summary>
        BorderRight,

        /// <summary>
        /// The CSS property BorderRightColor.
        /// </summary>
        BorderRightColor,

        /// <summary>
        /// The CSS property BorderRightStyle.
        /// </summary>
        BorderRightStyle,

        /// <summary>
        /// The CSS property BorderRightWidth.
        /// </summary>
        BorderRightWidth,

        /// <summary>
        /// The CSS property BorderLeft.
        /// </summary>
        BorderLeft,

        /// <summary>
        /// The CSS property BorderLeftColor.
        /// </summary>
        BorderLeftColor,

        /// <summary>
        /// The CSS property BorderLeftStyle.
        /// </summary>
        BorderLeftStyle,

        /// <summary>
        /// The CSS property BorderLeftWidth.
        /// </summary>
        BorderLeftWidth,

        /// <summary>
        /// The CSS property BorderBottom.
        /// </summary>
        BorderBottom,

        /// <summary>
        /// The CSS property BorderBottomColor.
        /// </summary>
        BorderBottomColor,

        /// <summary>
        /// The CSS property BorderBottomStyle.
        /// </summary>
        BorderBottomStyle,

        /// <summary>
        /// The CSS property BorderBottomWidth.
        /// </summary>
        BorderBottomWidth,

        /// <summary>
        /// The CSS property BorderColor.
        /// </summary>
        BorderColor,

        /// <summary>
        /// The CSS property BorderStyle.
        /// </summary>
        BorderStyle,

        /// <summary>
        /// The CSS property BorderWidth.
        /// </summary>
        BorderWidth,

        /// <summary>
        /// The CSS Clear property.
        /// </summary>
        Clear,

        /// <summary>
        /// The CSS Cursor property.
        /// </summary>
        Cursor,

        /// <summary>
        /// The CSS Color property.
        /// </summary>
        Color,

        /// <summary>
        /// The CSS Display property.
        /// </summary>
        Display,

        /// <summary>
        /// The CSS property FontWeight.
        /// </summary>
        FontWeight,

        /// <summary>
        /// The CSS property FontSize.
        /// </summary>
        FontSize,

        /// <summary>
        /// The CSS property FontFamily.
        /// </summary>
        FontFamily,

        /// <summary>
        /// The CSS property FontStyle.
        /// </summary>
        FontStyle,

        /// <summary>
        /// The CSS Height property.
        /// </summary>
        Height,

        /// <summary>
        /// The CSS property MarginLeft.
        /// </summary>
        MarginLeft,

        /// <summary>
        /// The CSS property MarginTop.
        /// </summary>
        MarginTop,

        /// <summary>
        /// The CSS property MarginRight.
        /// </summary>
        MarginRight,

        /// <summary>
        /// The CSS property MarginBottom.
        /// </summary>
        MarginBottom,

        /// <summary>
        /// The CSS Margin property.
        /// </summary>
        Margin,

        /// <summary>
        /// The CSS property PaddingLeft.
        /// </summary>
        PaddingLeft,

        /// <summary>
        /// The CSS property PaddingTop.
        /// </summary>
        PaddingTop,

        /// <summary>
        /// The CSS property PaddingRight.
        /// </summary>
        PaddingRight,

        /// <summary>
        /// The CSS property PaddingBottom.
        /// </summary>
        PaddingBottom,

        /// <summary>
        /// The CSS Padding property.
        /// </summary>
        Padding,

        /// <summary>
        /// The CSS property TextDecoration.
        /// </summary>
        TextDecoration,

        /// <summary>
        /// The CSS Width property.
        /// </summary>
        Width,

        /// <summary>
        /// The CSS Float property.
        /// </summary>
        Float,

        /// <summary>
        /// The CSS property Visibility.
        /// </summary>
        Visibility,

        /// <summary>
        /// The Top CSS property.
        /// </summary>
        Top,

        /// <summary>
        /// The CSS Right property.
        /// </summary>
        Right,

        /// <summary>
        /// The CSS Bottom property.
        /// </summary>
        Bottom,

        /// <summary>
        /// The CSS Left property.
        /// </summary>
        Left,

        /// <summary>
        /// The CSS property Position.
        /// </summary>
        Position,

        /// <summary>
        /// The CSS Clip property.
        /// </summary>
        Clip,

        /// <summary>
        /// The CSS property Overflow.
        /// </summary>
        Overflow,

        /// <summary>
        /// The CSS property VerticalAlign.
        /// </summary>
        VerticalAlign,

        /// <summary>
        /// The CSS Zindex property.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Zindex", Justification = "From the W3C spec.")]
        Zindex,

        /// <summary>
        /// The CSS property MaxHeight.
        /// </summary>
        MaxHeight,

        /// <summary>
        /// The CSS property MinHeight.
        /// </summary>
        MinHeight,

        /// <summary>
        /// The CSS property MaxWidth.
        /// </summary>
        MaxWidth,

        /// <summary>
        /// The CSS property MinWidth.
        /// </summary>
        MinWidth,

        /// <summary>
        /// The CSS property FontSizeAdjust.
        /// </summary>
        FontSizeAdjust,

        /// <summary>
        /// The CSS property FontStretch.
        /// </summary>
        FontStretch,

        /// <summary>
        /// The CSS property FontVariant.
        /// </summary>
        FontVariant,

        /// <summary>
        /// The CSS property Font.
        /// </summary>
        Font,

        /// <summary>
        /// The CSS property Content.
        /// </summary>
        Content,

        /// <summary>
        /// The CSS property CounterIncrement.
        /// </summary>
        CounterIncrement,

        /// <summary>
        /// The CSS property CounterReset.
        /// </summary>
        CounterReset,

        /// <summary>
        /// The CSS property Quotes.
        /// </summary>
        Quotes,

        /// <summary>
        /// The CSS property ListStyleType.
        /// </summary>
        ListStyleType,

        /// <summary>
        /// The CSS property ListStylePosition.
        /// </summary>
        ListStylePosition,

        /// <summary>
        /// The CSS property ListStyleImage.
        /// </summary>
        ListStyleImage,

        /// <summary>
        /// The CSS property ListStyle.
        /// </summary>
        ListStyle,

        /// <summary>
        /// The CSS property MarkerOffset.
        /// </summary>
        MarkerOffset,

        /// <summary>
        /// The CSS property OutlineColor.
        /// </summary>
        OutlineColor,

        /// <summary>
        /// The CSS property OutlineStyle.
        /// </summary>
        OutlineStyle,

        /// <summary>
        /// The CSS property OutlineWidth.
        /// </summary>
        OutlineWidth,

        /// <summary>
        /// The CSS property Outline.
        /// </summary>
        Outline,

        /// <summary>
        /// The CSS property Marks.
        /// </summary>
        Marks,

        /// <summary>
        /// The CSS property Orphans.
        /// </summary>
        Orphans,

        /// <summary>
        /// The CSS property Page.
        /// </summary>
        Page,

        /// <summary>
        /// The CSS property PageBreakAfter.
        /// </summary>
        PageBreakAfter,

        /// <summary>
        /// The CSS property PageBreakBefore.
        /// </summary>
        PageBreakBefore,

        /// <summary>
        /// The CSS property PageBreakInside.
        /// </summary>
        PageBreakInside,

        /// <summary>
        /// The CSS property Size.
        /// </summary>
        Size,

        /// <summary>
        /// The CSS property Windows.
        /// </summary>
        Windows,

        /// <summary>
        /// The CSS property BorderCollapse.
        /// </summary>
        BorderCollapse,

        /// <summary>
        /// The CSS property BorderSpacing.
        /// </summary>
        BorderSpacing,

        /// <summary>
        /// The CSS property CaptionSide.
        /// </summary>
        CaptionSide,

        /// <summary>
        /// The CSS property EmptyCells.
        /// </summary>
        EmptyCells,

        /// <summary>
        /// The CSS property TableLayout.
        /// </summary>
        TableLayout,

        /// <summary>
        /// The CSS property Direction.
        /// </summary>
        Direction,

        /// <summary>
        /// The CSS property LineHeight.
        /// </summary>
        LineHeight,

        /// <summary>
        /// The CSS property LetterSpacing.
        /// </summary>
        LetterSpacing,

        /// <summary>
        /// The CSS property TextAlign.
        /// </summary>
        TextAlign,

        /// <summary>
        /// The CSS property TextIndent.
        /// </summary>
        TextIndent,

        /// <summary>
        /// The CSS property TextTransform.
        /// </summary>
        TextTransform,

        /// <summary>
        /// The CSS property UnicodeBidi.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Bidi", Justification = "From the W3C spec.")]
        UnicodeBidi,

        /// <summary>
        /// The CSS property WhiteSpace.
        /// </summary>
        WhiteSpace,

        /// <summary>
        /// The CSS property WordSpacing.
        /// </summary>
        WordSpacing,

        /// <summary>
        /// The CSS property Azimuth.
        /// </summary>
        Azimuth,

        /// <summary>
        /// The CSS property CueAfter.
        /// </summary>
        CueAfter,

        /// <summary>
        /// The CSS property CueBefore.
        /// </summary>
        CueBefore,

        /// <summary>
        /// The CSS property Cue.
        /// </summary>
        Cue,

        /// <summary>
        /// The CSS property Elevation.
        /// </summary>
        Elevation,

        /// <summary>
        /// The CSS property PauseAfter.
        /// </summary>
        PauseAfter,

        /// <summary>
        /// The CSS property PauseBefore.
        /// </summary>
        PauseBefore,

        /// <summary>
        /// The CSS property Pause.
        /// </summary>
        Pause,

        /// <summary>
        /// The CSS property Pitch.
        /// </summary>
        Pitch,

        /// <summary>
        /// The CSS property PitchRange.
        /// </summary>
        PitchRange,

        /// <summary>
        /// The CSS property PlayDuring.
        /// </summary>
        PlayDuring,

        /// <summary>
        /// The CSS property Richness.
        /// </summary>
        Richness,

        /// <summary>
        /// The CSS property Speak.
        /// </summary>
        Speak,

        /// <summary>
        /// The CSS property SpeakHeader.
        /// </summary>
        SpeakHeader,

        /// <summary>
        /// The CSS property SpeakNumeral.
        /// </summary>
        SpeakNumeral,

        /// <summary>
        /// The CSS property SpeakPunctuation.
        /// </summary>
        SpeakPunctuation,

        /// <summary>
        /// The CSS property SpeechRate.
        /// </summary>
        SpeechRate,

        /// <summary>
        /// The CSS property Stress.
        /// </summary>
        Stress,

        /// <summary>
        /// The CSS property VoiceFamily.
        /// </summary>
        VoiceFamily,

        /// <summary>
        /// The CSS property Volume.
        /// </summary>
        Volume,
    }
}