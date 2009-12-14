// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Collections; 
using System.Windows; 

namespace System.Windows.Controls 
{
    /// <summary>
    /// Provides data for the SelectionChanged event. 
    /// </summary>
    public class SelectionChangedEventArgs : RoutedEventArgs
    { 
        static readonly object[] EmptyArgs = new object [0];
        /// <summary> 
        /// Gets a list that contains the items that were selected during this event.
        /// </summary> 
        public IList AddedItems
        {
            get { return this._addedItems; } 
        }
        private object[] _addedItems;
 
        /// <summary> 
        /// Gets a list that contains the items that were unselected during this event.
        /// </summary> 
        public IList RemovedItems
        {
            get { return this._removedItems; } 
        }
        private object[] _removedItems;
 
        internal SelectionChangedEventArgs (object removedItem, object addedItem)
            : this (removedItem == null ? EmptyArgs : new object[] { removedItem }, addedItem == null ? EmptyArgs : new object [] { addedItem })
        {
            
        }
        
        /// <summary> 
        /// Initializes a new instance of a SelectionChangedEventArgs class.
        /// </summary> 
        /// <param name="removedItems">The items that were unselected during this event.</param>
        /// <param name="addedItems">The items that were selected during this event.</param>
        public SelectionChangedEventArgs(IList removedItems, IList addedItems) 
        {
            if (null == removedItems)
            { 
                throw new ArgumentNullException("removedItems"); 
            }
            if (null == addedItems) 
            {
                throw new ArgumentNullException("addedItems");
            } 
            _removedItems = new object[removedItems.Count];
            removedItems.CopyTo(_removedItems, 0);
            _addedItems = new object[addedItems.Count]; 
            addedItems.CopyTo(_addedItems, 0); 
        }
    } 
}
