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
    /// Well-known HTML script object property names.
    /// </summary>
    public enum HtmlProperty
    {
        /// <summary>
        /// All defined HTML element attributes.
        /// </summary>
        Attributes,

        /// <summary>
        /// The child nodes of an element.
        /// </summary>
        ChildNodes,

        /// <summary>
        /// The CSS class name of an element.
        /// </summary>
        ClassName,

        /// <summary>
        /// The viewable client width.
        /// </summary>
        ClientWidth,

        /// <summary>
        /// The viewable client height.
        /// </summary>
        ClientHeight,

        /// <summary>
        /// The text direction of an element.
        /// </summary>
        Dir,

        /// <summary>
        /// The property that marks an input control as not enabled, 
        /// as defined by common browsers.
        /// </summary>
        Disabled,

        /// <summary>
        /// The first child element.
        /// </summary>
        FirstChild,

        /// <summary>
        /// The ID attribute of the object.
        /// </summary>
        Id,

        /// <summary>
        /// The inner HTML property of an HTML element.  Non-W3C.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "HTML", Justification = "The casing is important.")]
        InnerHTML,

        /// <summary>
        /// The inner text property of an HTML element.
        /// </summary>
        InnerText,
        
        /// <summary>
        /// The language of an attribute's values and an element's content.
        /// </summary>
        Lang,

        /// <summary>
        /// The last child element.
        /// </summary>
        LastChild,

        /// <summary>
        /// The local name of an XML element.
        /// </summary>
        LocalName,

        /// <summary>
        /// The URI string for an xmlns attribute of a XML element.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "URI", Justification = "The casing comes from the HTML W3C spec.")]
        NamespaceURI,

        /// <summary>
        /// The next sibling object.
        /// </summary>
        NextSibling,

        /// <summary>
        /// The name of the node.
        /// </summary>
        NodeName,

        /// <summary>
        /// The type of the node.
        /// </summary>
        NodeType,

        /// <summary>
        /// The value of the node.  Used for text nodes.
        /// </summary>
        NodeValue,

        /// <summary>
        /// The horizontal offset relative to the container.
        /// </summary>
        OffsetLeft,

        /// <summary>
        /// The vertical offset relative to the container.
        /// </summary>
        OffsetTop,

        /// <summary>
        /// The offset container of the current element.
        /// </summary>
        OffsetParent,

        /// <summary>
        /// Returns the width of an element without margins.
        /// </summary>
        OffsetWidth,

        /// <summary>
        /// The height of an element without margins.
        /// </summary>
        OffsetHeight,

        /// <summary>
        /// The DOM object that contains the current object.
        /// </summary>
        OwnerDocument,

        /// <summary>
        /// The parent of the current object.
        /// </summary>
        ParentNode,

        /// <summary>
        /// The namespace prefix, if any, for an XML element.
        /// </summary>
        Prefix,

        /// <summary>
        /// The previous sibling object.
        /// </summary>
        PreviousSibling,

        /// <summary>
        /// Distance between scrollbar and the horizontal location of 
        /// an element.
        /// </summary>
        ScrollLeft,

        /// <summary>
        /// Distance between the scrollbar and the top location of an 
        /// element.
        /// </summary>
        ScrollTop,

        /// <summary>
        /// The height of an element, including that hidden by scrolling 
        /// or out of the screen.
        /// </summary>
        ScrollHeight,

        /// <summary>
        /// The width, including non-visible portions.  Complement to 
        /// ScrollHeight.
        /// </summary>
        ScrollWidth,

        /// <summary>
        /// The style object for an element.
        /// </summary>
        Style,

        /// <summary>
        /// The tab order of the element.
        /// </summary>
        TabIndex,

        /// <summary>
        /// The tag name of the element.
        /// </summary>
        TagName,

        /// <summary>
        /// The title of the attribute.
        /// </summary>
        Title,
    }
}