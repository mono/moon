// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// The HtmlControl manager.
    /// </summary>
    public class HtmlControlManager
    {
        /// <summary>
        /// A set of managed controls to associate known HtmlElements that 
        /// have attached managed Controls.
        /// </summary>
        private Dictionary<WeakReference, HtmlControlBase> _managedControls;

        /// <summary>
        /// Creates a new HTML control manager that associates with a HARD reference 
        /// the element and control.
        /// </summary>
        public HtmlControlManager()
        {
            _managedControls = new Dictionary<WeakReference, HtmlControlBase>();
        }

        /// <summary>
        /// Check if the HtmlElement has a known managed control.
        /// </summary>
        /// <param name="element">The element object.</param>
        /// <returns>Returns whether the element has an attached control.</returns>
        public bool HasControl(HtmlElement element)
        {
            HtmlControlBase hcb = GetControl(element);
            return (hcb != null);
        }

        /// <summary>
        /// Retrieves a managed control instance, if any, for the HTML element.
        /// </summary>
        /// <param name="element">The HTML element.</param>
        /// <returns>Returns the managed control for an HTML element.</returns>
        public HtmlControlBase GetControl(HtmlElement element)
        {
            WeakReference key = GetKey(element);
            HtmlControlBase value = null;
            if (key != null && _managedControls.ContainsKey(key))
            {
                value = _managedControls[key];
            }
            return value;
        }

        /// <summary>
        /// Retrieves the key, if any. Also attempts to be proactive in 
        /// clearing out no longer-alive weak references, though once the key 
        /// is found the remaining keys are not checked.
        /// </summary>
        /// <param name="element">The HtmlElement object.</param>
        /// <returns>Returns the WeakReference that corresponds to the element 
        /// key in the weak reference dictionary.</returns>
        private WeakReference GetKey(HtmlElement element)
        {
            List<WeakReference> deadReferences = new List<WeakReference>();
            WeakReference key = null;
            foreach (WeakReference reference in _managedControls.Keys)
            {
                if (!reference.IsAlive)
                {
                    deadReferences.Add(reference);
                    continue;
                }

                HtmlElement he = reference.Target as HtmlElement;
                if (he != null && he == element)
                {
                    key = reference;
                    break;
                }
            }

            if (deadReferences.Count > 0)
            {
                foreach (WeakReference wr in deadReferences)
                {
                    _managedControls.Remove(wr);
                }
            }

            return key;
        }

        /// <summary>
        /// Register a managed HTML element control with the collection.
        /// </summary>
        /// <param name="control">The Control object.</param>
        public void RegisterControl(HtmlControlBase control)
        {
            WeakReference wr = new WeakReference(control.HtmlElement);
            _managedControls[wr] = control;
        }

        /// <summary>
        /// Removes an HTML element from the set of known control.
        /// </summary>
        /// <param name="element">The HTML element object.</param>
        public void UnregisterElement(HtmlElement element)
        {
            WeakReference key = GetKey(element);
            if (key != null)
            {
                _managedControls.Remove(key);
            }
        }
    }
}