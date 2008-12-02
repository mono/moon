// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A bag that stores HtmlElement information before the actual element is 
    /// initialized.
    /// </summary>
    public class HtmlElementWrapperBag : HtmlElementWrapper
    {
        /// <summary>
        /// The attributes.
        /// </summary>
        private Dictionary<string, string> _attributes;

        /// <summary>
        /// The properties.
        /// </summary>
        private Dictionary<string, object> _properties;

        /// <summary>
        /// The styles.
        /// </summary>
        private Dictionary<string, string> _styles;

        /// <summary>
        /// The event attachments.
        /// </summary>
        private List<EventAttachment<EventHandler>> _events;

        /// <summary>
        /// The event attachments that use HtmlEventArgs.
        /// </summary>
        private List<EventAttachment<EventHandler<HtmlEventArgs>>> _htmlEvents;

        /// <summary>
        /// Initializes a new bag that stores HtmlElement properties and settings before 
        /// the actual HtmlElement exists.
        /// </summary>
        public HtmlElementWrapperBag()
        {
            _attributes = new Dictionary<string, string>();
            _properties = new Dictionary<string, object>();
            _styles = new Dictionary<string, string>();
            _events = new List<EventAttachment<EventHandler>>();
            _htmlEvents = new List<EventAttachment<EventHandler<HtmlEventArgs>>>();
        }

        /// <summary>
        /// Attach an event handler.
        /// </summary>
        /// <param name="name">The event name.</param>
        /// <param name="handler">The event handler.</param>
        public override void AttachEvent(string name, EventHandler handler)
        {
            EventAttachment<EventHandler> attachment = new EventAttachment<EventHandler>
            {
                EventName = name,
                Handler = handler
            };
            _events.Add(attachment);
        }

        /// <summary>
        /// Attach an event handler to the element.
        /// </summary>
        /// <param name="name">The HTML script event name.</param>
        /// <param name="handler">The event handler.</param>
        public override void AttachEvent(string name, EventHandler<HtmlEventArgs> handler)
        {
            EventAttachment<EventHandler<HtmlEventArgs>> attachment = new EventAttachment<EventHandler<HtmlEventArgs>>
            {
                EventName = name,
                Handler = handler
            };
            _htmlEvents.Add(attachment);
        }

        /// <summary>
        /// Gets the stored attributes.
        /// </summary>
        public Dictionary<string, string> Attributes
        {
            get
            {
                return _attributes;
            }
        }

        /// <summary>
        /// Gets the stored properties.
        /// </summary>
        public Dictionary<string, object> Properties
        {
            get
            {
                return _properties;
            }
        }

        /// <summary>
        /// Gets the stored style attributes.
        /// </summary>
        public Dictionary<string, string> StyleAttributes
        {
            get
            {
                return _styles;
            }
        }

        /// <summary>
        /// Gets the standard event attachments.
        /// </summary>
        [SuppressMessage("Microsoft.Design", "CA1006:DoNotNestGenericTypesInMemberSignatures", Justification = "This is intended for simplicity sake of not creating too many types."), System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1002:DoNotExposeGenericLists", Justification = "Intended for use only in this framework, but leaving open as a public type since there is no reason to prematurely mark this as internal.")]
        public List<EventAttachment<EventHandler>> Events
        {
            get
            {
                return _events;
            }
        }

        /// <summary>
        /// Gets the HTML events.
        /// </summary>
        [SuppressMessage("Microsoft.Design", "CA1006:DoNotNestGenericTypesInMemberSignatures", Justification = "This is intended for simplicity sake."), System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1002:DoNotExposeGenericLists", Justification = "Intended for use only in this framework, but leaving open as a public type since there is no reason to prematurely mark this as internal.")]
        public List<EventAttachment<EventHandler<HtmlEventArgs>>> HtmlEvents
        {
            get
            {
                return _htmlEvents;
            }
        }

        /// <summary>
        /// Get an attribute from the element.
        /// </summary>
        /// <param name="name">The attribute name to lookup.</param>
        /// <returns>Returns the attribute's value.</returns>
        public override string GetAttribute(string name)
        {
            if (_attributes.ContainsKey(name))
            {
                return _attributes[name];
            }
            return null;
        }

        /// <summary>
        /// Gets a property.
        /// </summary>
        /// <param name="name">The HTML property.</param>
        /// <returns>Returns the value.</returns>
        public override object GetProperty(string name)
        {
            if (_properties.ContainsKey(name))
            {
                return _properties[name];
            }
            return null;
        }

        /// <summary>
        /// Gets the style attribute.
        /// </summary>
        /// <param name="name">The attribute to get.</param>
        /// <returns>Returns the string value of the style attribute.</returns>
        public override string GetStyleAttribute(string name)
        {
            if (_styles.ContainsKey(name))
            {
                return _styles[name];
            }
            return null;
        }

        /// <summary>
        /// Remove an attribute from the element.
        /// </summary>
        /// <param name="name">The attribute name.</param>
        public override void RemoveAttribute(string name)
        {
            _attributes.Remove(name);
        }

        /// <summary>
        /// Remove a style attribute from the element.
        /// </summary>
        /// <param name="name">The style attribute name.</param>
        public override void RemoveStyleAttribute(string name)
        {
            _styles.Remove(name);
        }

        /// <summary>
        /// Set an attribute value.
        /// </summary>
        /// <param name="name">The attribute name.</param>
        /// <param name="value">The value to set.</param>
        public override void SetAttribute(string name, string value)
        {
            _attributes[name] = value;
        }

        /// <summary>
        /// Sets a property.
        /// </summary>
        /// <param name="name">The HTML property.</param>
        /// <param name="value">The value to set.</param>
        public override void SetProperty(string name, object value)
        {
            _properties[name] = value;
        }

        /// <summary>
        /// Sets the style attribute.
        /// </summary>
        /// <param name="name">Attribute name.</param>
        /// <param name="value">The value to set the property to.</param>
        public override void SetStyleAttribute(string name, string value)
        {
            _styles[name] = value;
        }
    }
}