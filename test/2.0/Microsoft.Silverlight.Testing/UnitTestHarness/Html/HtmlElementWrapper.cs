// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A wrapper around HtmlElement that stores and retrieves values, 
    /// properties, and other important data.
    /// </summary>
    public class HtmlElementWrapper
    {
        /// <summary>
        /// The real HtmlElement after it has been initialized.
        /// </summary>
        private HtmlElement _realElement;

        /// <summary>
        /// Initializes the HtmlElementWrapper with no real HtmlElement set.
        /// </summary>
        protected HtmlElementWrapper()
        {
        }

        /// <summary>
        /// Initializes a new element wrapper.
        /// </summary>
        /// <param name="realElement">The element whose properties to wrap.</param>
        public HtmlElementWrapper(HtmlElement realElement) : this()
        {
            if (realElement == null)
            {
                throw new ArgumentNullException("realElement");
            }
            _realElement = realElement;
        }

        /// <summary>
        /// Initializes the HtmlElementWrapper by copying the contents of a 
        /// property bag into the real element, and then discarding the bag's 
        /// contents.
        /// </summary>
        /// <param name="realElement">The real HtmlElement.</param>
        /// <param name="bag">The existing wrapper (bag) to copy the information
        /// from.</param>
        [SuppressMessage("Microsoft.Usage", "CA2214:DoNotCallOverridableMethodsInConstructors", Justification = "This is a special copy constructor to move in another object's values.  This constructor is only useful to the base type.")]
        public HtmlElementWrapper(HtmlElement realElement, HtmlElementWrapperBag bag) : this(realElement)
        {
            if (bag == null)
            {
                throw new ArgumentNullException("bag");
            }
            MoveBagContents(bag);
        }

        /// <summary>
        /// Copies the contents of the property, event and attribute bag into 
        /// the actual live control.
        /// </summary>
        /// <param name="bag">The bag containing the values.</param>
        private void MoveBagContents(HtmlElementWrapperBag bag)
        {
            // Attributes
            foreach (string attribute in bag.Attributes.Keys)
            {
                SetAttribute(attribute, bag.Attributes[attribute]);
            }
            bag.Attributes.Clear();

            // Properties
            foreach (string property in bag.Properties.Keys)
            {
                SetProperty(property, bag.Properties[property]);
            }
            bag.Properties.Clear();

            // Style attributes
            foreach (string style in bag.StyleAttributes.Keys)
            {
                SetStyleAttribute(style, bag.StyleAttributes[style]);
            }
            bag.StyleAttributes.Clear();

            // HTML Events
            foreach (EventAttachment<EventHandler<HtmlEventArgs>> htmlEvent in bag.HtmlEvents)
            {
                AttachEvent(htmlEvent.EventName, htmlEvent.Handler);
            }
            bag.HtmlEvents.Clear();

            // Events
            foreach (EventAttachment<EventHandler> attachment in bag.Events)
            {
                AttachEvent(attachment.EventName, attachment.Handler);
            }
            bag.Events.Clear();
        }

        #region Standard HtmlElement Wrapper methods

        /// <summary>
        /// Attach an event handler to the element.
        /// </summary>
        /// <param name="name">The HTML script event name.</param>
        /// <param name="handler">The event handler.</param>
        public virtual void AttachEvent(string name, EventHandler handler)
        {
            _realElement.AttachEvent(name, handler);
        }

        /// <summary>
        /// Attach an event handler to the element.
        /// </summary>
        /// <param name="name">The HTML script event name.</param>
        /// <param name="handler">The event handler.</param>
        public virtual void AttachEvent(string name, EventHandler<HtmlEventArgs> handler)
        {
            _realElement.AttachEvent(name, handler);
        }

        /// <summary>
        /// Get an attribute from the element.
        /// </summary>
        /// <param name="name">The attribute name to lookup.</param>
        /// <returns>Returns the attribute's value.</returns>
        public virtual string GetAttribute(string name)
        {
            return _realElement.GetAttribute(name);
        }

        /// <summary>
        /// Gets a property.
        /// </summary>
        /// <param name="name">The HTML property.</param>
        /// <returns>Returns the value.</returns>
        public virtual object GetProperty(string name)
        {
            return _realElement.GetProperty(name);
        }

        /// <summary>
        /// Gets the style attribute.
        /// </summary>
        /// <param name="name">The attribute to get.</param>
        /// <returns>Returns the string value of the style attribute.</returns>
        public virtual string GetStyleAttribute(string name)
        {
            return _realElement.GetStyleAttribute(name);
        }

        /// <summary>
        /// Remove an attribute from the element.
        /// </summary>
        /// <param name="name">The attribute name.</param>
        public virtual void RemoveAttribute(string name)
        {
            _realElement.RemoveAttribute(name);
        }

        /// <summary>
        /// Remove a style attribute from the element.
        /// </summary>
        /// <param name="name">The style attribute name.</param>
        public virtual void RemoveStyleAttribute(string name)
        {
            _realElement.RemoveStyleAttribute(name);
        }

        /// <summary>
        /// Set an attribute value.
        /// </summary>
        /// <param name="name">The attribute name.</param>
        /// <param name="value">The value to set.</param>
        public virtual void SetAttribute(string name, string value)
        {
            _realElement.SetAttribute(name, value);
        }

        /// <summary>
        /// Sets a property.
        /// </summary>
        /// <param name="name">The HTML property.</param>
        /// <param name="value">The value to set.</param>
        public virtual void SetProperty(string name, object value)
        {
            _realElement.SetProperty(name, value);
        }

        /// <summary>
        /// Sets the style attribute.
        /// </summary>
        /// <param name="name">Attribute name.</param>
        /// <param name="value">The value to set the property to.</param>
        public virtual void SetStyleAttribute(string name, string value)
        {
            _realElement.SetStyleAttribute(name, value);
        }

        #endregion
    }
}