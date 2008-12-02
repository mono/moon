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
    /// Set of HTML tags.
    /// </summary>
    public enum HtmlTag
    {
        /// <summary>
        /// An Anchor element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "A", Justification = "Name comes from the HTML spec.")]
        A,

        /// <summary>
        /// An abbreviation element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Abbr")]
        Abbr,

        /// <summary>
        /// An acronym element.
        /// </summary>
        Acronym,

        /// <summary>
        /// An address element.
        /// </summary>
        Address,

        /// <summary>
        /// An image map's area element.
        /// </summary>
        Area,

        /// <summary>
        /// A bold text element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "B")]
        B,

        /// <summary>
        /// A page's base URI definition element.
        /// </summary>
        Base,

        /// <summary>
        /// A text direction definition element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Bdo")]
        Bdo,

        /// <summary>
        /// A large text element.
        /// </summary>
        Big,

        /// <summary>
        /// A large text quote element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Blockquote")]
        Blockquote,

        /// <summary>
        /// The main body element.
        /// </summary>
        Body,

        /// <summary>
        /// A line break element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Br", Justification = "This name comes from the W3C spec for HTML tags.")]
        Br,

        /// <summary>
        /// An input button element.
        /// </summary>
        Button,

        /// <summary>
        /// A table's caption element.
        /// </summary>
        Caption,

        /// <summary>
        /// A citation element.
        /// </summary>
        Cite,

        /// <summary>
        /// A code or fixed text element.
        /// </summary>
        Code,

        /// <summary>
        /// A table column definition element.
        /// </summary>
        Col,

        /// <summary>
        /// A column grouping definition element for table use.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Colgroup")]
        Colgroup,

        /// <summary>
        /// A description or definition-description element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Dd", Justification = "This name comes from the W3C spec for HTML tags.")]
        Dd,

        /// <summary>
        /// A removed text element.
        /// </summary>
        Del,

        /// <summary>
        /// A division or section element.
        /// </summary>
        Div,

        /// <summary>
        /// A definition list element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Dl", Justification = "This name comes from the W3C spec for HTML tags.")]
        Dl,

        /// <summary>
        /// A definition term element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Dt", Justification = "This name comes from the W3C spec for HTML tags.")]
        Dt,

        /// <summary>
        /// An emphasized text element.
        /// </summary>
        Em,

        /// <summary>
        /// A fieldset element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Fieldset")]
        Fieldset,

        /// <summary>
        /// A form element.
        /// </summary>
        Form,

        /// <summary>
        /// A sub frame element.
        /// </summary>
        Frame,

        /// <summary>
        /// A frameset definition element.
        /// </summary>
        Frameset,

        /// <summary>
        /// A header 1 element.
        /// </summary>
        H1,

        /// <summary>
        /// A header 2 element.
        /// </summary>
        H2,

        /// <summary>
        /// A header 3element.
        /// </summary>
        H3,

        /// <summary>
        /// A header 4 element.
        /// </summary>
        H4,

        /// <summary>
        /// A header 5 element.
        /// </summary>
        H5,

        /// <summary>
        /// A header 6 element.
        /// </summary>
        H6,

        /// <summary>
        /// The document's informational header element.
        /// </summary>
        Head,

        /// <summary>
        /// A horizontal rule element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Hr", Justification = "This name comes from the W3C spec for HTML tags.")]
        Hr,

        /// <summary>
        /// An HTML document's root element.
        /// </summary>
        Html,

        /// <summary>
        /// Italic text element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "I")]
        I,

        /// <summary>
        /// A inline sub-frame element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Iframe")]
        Iframe,

        /// <summary>
        /// An image element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Img")]
        Img,

        /// <summary>
        /// A input element.
        /// </summary>
        Input,

        /// <summary>
        /// A inserted/modified text element.
        /// </summary>
        Ins,

        /// <summary>
        /// A keyboard text element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Kbd")]
        Kbd,

        /// <summary>
        /// An input control label element.
        /// </summary>
        Label,

        /// <summary>
        /// A fieldset's title element.
        /// </summary>
        Legend,

        /// <summary>
        /// A list item element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Li", Justification = "Name from the spec.")]
        Li,

        /// <summary>
        /// A linked resource definition element.
        /// </summary>
        Link,

        /// <summary>
        /// An image map element.
        /// </summary>
        Map,

        /// <summary>
        /// A metadata definition element.
        /// </summary>
        Meta,

        /// <summary>
        /// A noframe section's element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Noframes")]
        Noframes,

        /// <summary>
        /// A noscript section's element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Noscript")]
        Noscript,

        /// <summary>
        /// An embedded object element.
        /// </summary>
        Object,

        /// <summary>
        /// A ordered list element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Ol")]
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Ol", Justification = "Matches the spec.")]
        Ol,

        /// <summary>
        /// A option group element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Optgroup")]
        Optgroup,

        /// <summary>
        /// A dropdown list's item definition element.
        /// </summary>
        Option,

        /// <summary>
        /// A Paragraph element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "P")]
        P,

        /// <summary>
        /// An object parameter element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Param")]
        Param,

        /// <summary>
        /// A preformatted text element.
        /// </summary>
        Pre,

        /// <summary>
        /// A short quote element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Q")]
        Q,

        /// <summary>
        /// A sample code element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Samp")]
        Samp,

        /// <summary>
        /// A script element.
        /// </summary>
        Script,

        /// <summary>
        /// A dropdown/selectable list element.
        /// </summary>
        Select,

        /// <summary>
        /// A small text element.
        /// </summary>
        Small,

        /// <summary>
        /// A text section/span element.
        /// </summary>
        Span,

        /// <summary>
        /// A strong text element.
        /// </summary>
        Strong,

        /// <summary>
        /// A style definition element.
        /// </summary>
        Style,

        /// <summary>
        /// A subscript element.
        /// </summary>
        Sub,

        /// <summary>
        /// A superscript element.
        /// </summary>
        Sup,

        /// <summary>
        /// A table definition element.
        /// </summary>
        Table,

        /// <summary>
        /// A table body element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Tbody")]
        Tbody,

        /// <summary>
        /// A table cell element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Td", Justification = "Name from the spec.")]
        Td,

        /// <summary>
        /// A multiline text area element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Textarea")]
        Textarea,

        /// <summary>
        /// A table footer element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Tfoot")]
        Tfoot,

        /// <summary>
        /// A table header element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Th", Justification = "Name from the spec.")]
        Th,

        /// <summary>
        /// A table header element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Thead")]
        Thead,

        /// <summary>
        /// A document title element.
        /// </summary>
        Title,

        /// <summary>
        /// A table row element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Tr")]
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Tr", Justification = "Matches the spec.")]
        Tr,

        /// <summary>
        /// A teletype text element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Justification = "These identifiers are from the W3C specification and need to be stored as-is.", MessageId = "Tt")]
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Tt", Justification = "Name from the spec.")]
        Tt,

        /// <summary>
        /// An unordered list element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Ul", Justification = "Name from the spec.")]
        Ul,

        /// <summary>
        /// A variable element.
        /// </summary>
        Var,
    }
}