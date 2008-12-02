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
    /// A collection of controls.
    /// </summary>
    [SuppressMessage("Microsoft.Naming", "CA1711:IdentifiersShouldNotHaveIncorrectSuffix", Justification = "This name comes from the desktop framework's collection for HtmlControls.")]
    public class HtmlControlCollection : ICollection<HtmlControlBase>
    {
        /// <summary>
        /// The control's children.  Only children that are actually added to 
        /// the collection show up - actual HtmlElement objects in the real 
        /// DOM of the browser may exist that are not represented here.
        /// </summary>
        private List<HtmlControlBase> _children;

        /// <summary>
        /// Gets the owning Control reference.
        /// </summary>
        public HtmlControlBase Owner
        {
            get; private set;
        }

        /// <summary>
        /// Initializes a new control collection object.
        /// </summary>
        /// <param name="owner">The collection's owner control.</param>
        public HtmlControlCollection(HtmlControlBase owner)
        {
            Owner = owner;
            _children = new List<HtmlControlBase>();
        }

        /// <summary>
        /// Adds a new control to the collection.
        /// </summary>
        /// <param name="item">The control to add.</param>
        public void Add(HtmlControlBase item)
        {
            _children.Add(item);

            // If the owner of this collection has already initialized, any 
            // new child controls must immediately initialize upon append.
            if (Owner.HtmlElement != null && item.HtmlElement == null)
            {
                Owner.HtmlElement.AppendChild(item);
            }
        }

        /// <summary>
        /// Removes all controls from the collection.  Also clears 
        /// the browser elements.
        /// </summary>
        public void Clear()
        {
            ClearBrowserElements();
            _children.Clear();
        }

        /// <summary>
        /// Removes elements from the web browser for all children 
        /// in the collection.
        /// </summary>
        private void ClearBrowserElements()
        {
            foreach (HtmlControlBase child in _children)
            {
                if (child.HtmlElement != null)
                {
                    Owner.HtmlElement.RemoveChild(child.HtmlElement);
                }
            }
        }

        /// <summary>
        /// Check whether the collection contains the control.
        /// </summary>
        /// <param name="item">The Control object.</param>
        /// <returns>Returns a value indicating whether the Control is in the 
        /// collection.</returns>
        public bool Contains(HtmlControlBase item)
        {
            return _children.Contains(item);
        }

        /// <summary>
        /// Returns the index in the collection of the control.
        /// </summary>
        /// <param name="control">The Control object.</param>
        /// <returns>Returns the index of the control.</returns>
        public int IndexOf(HtmlControlBase control)
        {
            return _children.IndexOf(control);
        }

        /// <summary>
        /// Removes the control from the collection.
        /// </summary>
        /// <param name="item">The control object.</param>
        /// <returns>Returns a value indicating whether the control was removed.</returns>
        public bool Remove(HtmlControlBase item)
        {
            return _children.Remove(item);
        }

        /// <summary>
        /// Removes the object at a location in the collection.
        /// </summary>
        /// <param name="index">The index value.</param>
        public void RemoveAt(int index)
        {
            _children.RemoveAt(index);
        }

        /// <summary>
        /// Gets the count of objects in the collection.
        /// </summary>
        public int Count
        {
            get
            {
                return _children.Count;
            }
        }

        /// <summary>
        /// Gets a value indicating whether the collection is read only.
        /// </summary>
        public bool IsReadOnly
        {
            get
            {
                return false;
            }
        }

        /// <summary>
        /// Copies the collection into the array.
        /// </summary>
        /// <param name="array">The array.</param>
        /// <param name="arrayIndex">The starting index.</param>
        public void CopyTo(HtmlControlBase[] array, int arrayIndex)
        {
            _children.CopyTo(array, arrayIndex);
        }

        /// <summary>
        /// Gets the Control at the specific index.
        /// </summary>
        /// <param name="index">The object index.</param>
        /// <returns>Returns the control.</returns>
        public HtmlControlBase this[int index]
        {
            get
            {
                return _children[index];
            }
        }

        /// <summary>
        /// Returns an enumerator that iterates through the collection.
        /// </summary>
        /// <returns>A System.Collections.Generic.IEnumerator&lt;T&gt; that can 
        /// be used to iterate through the collection.</returns>
        public IEnumerator<HtmlControlBase> GetEnumerator()
        {
            return _children.GetEnumerator();
        }

        /// <summary>
        /// Returns an enumerator that iterates through a collection.
        /// </summary>
        /// <returns>An System.Collections.IEnumerator object that can be used 
        /// to iterate through the collection.</returns>
        IEnumerator IEnumerable.GetEnumerator()
        {
            return _children.GetEnumerator();
        }
    }
}