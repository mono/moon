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
    /// Set of HTML attribute names, as defined by the W3C's 
    /// HTML 4 spec.
    /// </summary>
    [SuppressMessage("Microsoft.Naming", "CA1711:IdentifiersShouldNotHaveIncorrectSuffix", Justification = "These enum values represent HTML attributes.  The name provides the clarification needed to HTML page developers.")]
    public enum HtmlAttribute
    {
        /// <summary>
        /// Abbreviation for a cell.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Abbr", Justification = "This is part of the W3C spec and needs to be present in its current abbreviation.")]
        Abbr,

        /// <summary>
        /// MIME types accepted for the upload input control.
        /// </summary>
        Accept,

        /// <summary>
        /// The accessibility key.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Accesskey", Justification = "This is part of the W3C spec and needs to be present in its current form.")]
        Accesskey,

        /// <summary>
        /// The handler location on the server for forms.
        /// </summary>
        Action,

        /// <summary>
        /// Alignment attribute.
        /// </summary>
        Align,

        /// <summary>
        /// Alternative text.
        /// </summary>
        Alt,

        /// <summary>
        /// List of archive addresses.
        /// </summary>
        Archive,

        /// <summary>
        /// List of related headers.
        /// </summary>
        Axis,

        /// <summary>
        /// Controls the border around a table.
        /// </summary>
        Border,

        /// <summary>
        /// Padding for table cells.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Cellpadding")]
        Cellpadding,

        /// <summary>
        /// Spacing for table cells.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Cellspacing")]
        Cellspacing,

        /// <summary>
        /// The alignment character.
        /// </summary>
        Char,

        /// <summary>
        /// Offset for alignment char.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Charoff")]
        Charoff,

        /// <summary>
        /// Char encoding of linked resource.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Charset")]
        Charset,

        /// <summary>
        /// For radio buttons and check boxes.
        /// </summary>
        Checked,

        /// <summary>
        /// URI for source document or message.
        /// </summary>
        Cite,

        /// <summary>
        /// Space-separated list of classes.
        /// </summary>
        Class,

        /// <summary>
        /// Identifies an implementation.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Classid")]
        Classid,

        /// <summary>
        /// Base URI for classid, data, archive.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Codebase")]
        Codebase,

        /// <summary>
        /// Content type for code.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Codetype")]
        Codetype,

        /// <summary>
        /// List of lengths for columns.
        /// </summary>
        Cols,

        /// <summary>
        /// Number of cols spanned by cell.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Colspan")]
        Colspan,

        /// <summary>
        /// Associated information for a Meta tag.
        /// </summary>
        Content,

        /// <summary>
        /// Comma-separated list of lengths.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Coords")]
        Coords,

        /// <summary>
        /// Reference to object's data.
        /// </summary>
        Data,

        /// <summary>
        /// Date and time of change.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Datetime")]
        Datetime,

        /// <summary>
        /// Declare but don't instantiate flag.
        /// </summary>
        Declare,

        /// <summary>
        /// UA may defer execution of script.
        /// </summary>
        Defer,

        /// <summary>
        /// Direction for text.
        /// </summary>
        Dir,

        /// <summary>
        /// Unavailable in this context.
        /// </summary>
        Disabled,

        /// <summary>
        /// Encoding type for a form.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Enctype")]
        Enctype,

        /// <summary>
        /// Matches field ID value in a label.
        /// </summary>
        For,

        /// <summary>
        /// Which parts of frame to render.
        /// </summary>
        Frame,

        /// <summary>
        /// Request frame borders.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Frameborder")]
        Frameborder,

        /// <summary>
        /// List of IDs for header cells.
        /// </summary>
        Headers,

        /// <summary>
        /// Height of elements and other items.
        /// </summary>
        Height,

        /// <summary>
        /// A hypertext reference.
        /// </summary>
        Href,

        /// <summary>
        /// The language code.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Hreflang")]
        Hreflang,

        /// <summary>
        /// Unique ID.
        /// </summary>
        Id,

        /// <summary>
        /// Use a server-side image map.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Ismap")]
        Ismap,

        /// <summary>
        /// For use in hierarchical menus.
        /// </summary>
        Label,

        /// <summary>
        /// Language code.
        /// </summary>
        Lang,

        /// <summary>
        /// Link to longer description.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Longdesc")]
        Longdesc,

        /// <summary>
        /// Margin height in pixels.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Marginheight")]
        Marginheight,

        /// <summary>
        /// Margin width in pixels.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Marginwidth")]
        Marginwidth,

        /// <summary>
        /// Max characters for text fields.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Maxlength")]
        Maxlength,

        /// <summary>
        /// Media value for style links.
        /// </summary>
        Media,

        /// <summary>
        /// HTTP method used to submit the form.
        /// </summary>
        Method,

        /// <summary>
        /// Default is single selection.
        /// </summary>
        Multiple,

        /// <summary>
        /// The name of an element.
        /// </summary>
        Name,

        /// <summary>
        /// This region has no action.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Nohref")]
        Nohref,

        /// <summary>
        /// Allow users to resize frames.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Noresize")]
        Noresize,

        /// <summary>
        /// On blur event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onblur")]
        Onblur,

        /// <summary>
        /// On change event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onchange")]
        Onchange,

        /// <summary>
        /// On click event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onclick")]
        Onclick,

        /// <summary>
        /// On double click event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Ondblclick")]
        Ondblclick,

        /// <summary>
        /// On focus event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onfocus")]
        Onfocus,

        /// <summary>
        /// On key down event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onkeydown")]
        Onkeydown,

        /// <summary>
        /// On key press event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onkeypress")]
        Onkeypress,

        /// <summary>
        /// On key up event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onkeyup")]
        Onkeyup,

        /// <summary>
        /// On load event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onload")]
        Onload,

        /// <summary>
        /// On mouse down event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onmousedown")]
        Onmousedown,

        /// <summary>
        /// On mouse move event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onmousemove")]
        Onmousemove,

        /// <summary>
        /// On mouse out event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onmouseout")]
        Onmouseout,

        /// <summary>
        /// On mouse over event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onmouseover")]
        Onmouseover,

        /// <summary>
        /// On mouse up event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onmouseup")]
        Onmouseup,

        /// <summary>
        /// On reset event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onreset")]
        Onreset,

        /// <summary>
        /// On select event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onselect")]
        Onselect,

        /// <summary>
        /// On submit event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onsubmit")]
        Onsubmit,

        /// <summary>
        /// On unload event.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Onunload")]
        Onunload,

        /// <summary>
        /// Named dictionary of meta info.
        /// </summary>
        Profile,

        /// <summary>
        /// Read only value.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1702:CompoundWordsShouldBeCasedCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Readonly")]
        Readonly,

        /// <summary>
        /// Forward link types.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Rel")]
        Rel,

        /// <summary>
        /// Reverse link types.
        /// </summary>
        Rev,

        /// <summary>
        /// List of lengths.
        /// </summary>
        Rows,

        /// <summary>
        /// Number of rows spanned by cell.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Rowspan")]
        Rowspan,

        /// <summary>
        /// Ruling between rows and cols.
        /// </summary>
        Rules,

        /// <summary>
        /// Select form of content.
        /// </summary>
        Scheme,

        /// <summary>
        /// Scope covered by header cells.
        /// </summary>
        Scope,

        /// <summary>
        /// Scrollbar or none.
        /// </summary>
        Scrolling,

        /// <summary>
        /// Selected value.
        /// </summary>
        Selected,

        /// <summary>
        /// Controls interpretation of coords.
        /// </summary>
        Shape,

        /// <summary>
        /// For select tag, number of visible rows.
        /// </summary>
        Size,

        /// <summary>
        /// Col attributes or default number of columns 
        /// in group.
        /// </summary>
        Span,

        /// <summary>
        /// URI for various resources or links.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Src")]
        Src,

        /// <summary>
        /// Message to show while loading.
        /// </summary>
        Standby,

        /// <summary>
        /// Associated style information.
        /// </summary>
        Style,

        /// <summary>
        /// Purpose or structure for speech output.
        /// </summary>
        Summary,

        /// <summary>
        /// Position in tabbing order.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Tabindex")]
        Tabindex,

        /// <summary>
        /// Render in a specific frame/window.
        /// </summary>
        Target,

        /// <summary>
        /// Advisory title.
        /// </summary>
        Title,

        /// <summary>
        /// Content type.
        /// </summary>
        Type,

        /// <summary>
        /// Use client-side image map.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Usemap")]
        Usemap,

        /// <summary>
        /// Vertical alignment in cells.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Valign")]
        Valign,

        /// <summary>
        /// Property value.
        /// </summary>
        Value,

        /// <summary>
        /// How to interpret value.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "This is part of the W3C spec and needs to be present in its current form.", MessageId = "Valuetype")]
        Valuetype,

        /// <summary>
        /// Width value.
        /// </summary>
        Width,
    }
}