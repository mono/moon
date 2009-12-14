// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;

namespace System.Windows.Controls
{
    /// <summary>
    /// IComparer class to sort by class property value (using reflection).
    /// </summary>
    internal class SortFieldComparer<T> : IComparer<T>, IComparer
    {
        #region Constructors

        internal SortFieldComparer() { }
        /// <summary>
        /// Create a comparer, using the SortDescription and a Type;
        /// tries to find a reflection PropertyInfo for each property name
        /// </summary>
        /// <param name="sortFields">list of property names and direction to sort by</param>
        public SortFieldComparer(SortDescriptionCollection sortFields)
        {
            _sortFields = sortFields;
            _fields = CreatePropertyInfo(_sortFields);
        }

        #endregion

        #region Public Methods

        /// <summary>
        /// Compares two objects and returns a value indicating whether one is less than, equal to or greater than the other.
        /// </summary>
        /// <param name="x">first item to compare</param>
        /// <param name="y">second item to compare</param>
        /// <returns>; &lt;0: x &lt; y; =0: x == y; &gt; 0: x &gt; y</returns>
        /// <remarks>
        /// Compares the 2 items using the list of property names and directions.
        /// </remarks>
        public int Compare(T x, T y)
        {
            int result = 0;

            // compare both objects by each of the properties until property values don't match
            for (int k = 0; k < _fields.Length; ++k)
            {
                object v1 = _fields[k].GetValue(x);
                object v2 = _fields[k].GetValue(y);

                result = _fields[k].comparer.Compare(v1, v2);
                if (_fields[k].descending)
                    result = -result;

                if (result != 0)
                    break;
            }

            return result;
        }

        #endregion

        #region Private Methods

        private static SortPropertyInfo[] CreatePropertyInfo(SortDescriptionCollection sortFields)
        {
            SortPropertyInfo[] fields = new SortPropertyInfo[sortFields.Count];
            for (int k = 0; k < sortFields.Count; ++k)
            {
                IComparer propertyComparer;
                if (String.IsNullOrEmpty(sortFields[k].PropertyName))
                {
                    // sort by the object itself (as opposed to a property) with a default comparer
                    propertyComparer = Comparer<T>.Default;
                }
                else
                {
                    // sort by the value of a property path, to be applied to
                    // the items in the list
                    Type propertyType = typeof(T).GetNestedPropertyType(sortFields[k].PropertyName);
                    if (propertyType == null)
                    {
                        throw CollectionViewError.ListCollectionView.InvalidPropertyName(sortFields[k].PropertyName);
                    }

                    //dynamically create comparer for property type
                    Type comparerType = typeof(Comparer<>).MakeGenericType(new Type[] { propertyType });
                    propertyComparer = (IComparer)comparerType.GetProperty("Default").GetValue(null, null);
                }

                // remember PropertyPath and direction, used when actually sorting
                fields[k].propertyPath = sortFields[k].PropertyName;
                fields[k].descending = (sortFields[k].Direction == ListSortDirection.Descending);
                fields[k].comparer = propertyComparer;
            }
            return fields;
        }

        #endregion

        #region Private Fields

        struct SortPropertyInfo
        {
            internal IComparer comparer;
            internal bool descending;
            internal string propertyPath;

            internal object GetValue(object o)
            {
                object value;
                if (String.IsNullOrEmpty(propertyPath))
                {
                    value = o;
                }
                else
                {
                    value = ListCollectionView.InvokePath(o, propertyPath);
                }

                return value;
            }
        }

        private SortPropertyInfo[] _fields;
        private SortDescriptionCollection _sortFields;

        #endregion

        #region IComparer Members

        /// <summary>
        /// Compares two objects and returns a value indicating whether one is less than, equal to or greater than the other.
        /// </summary>
        /// <param name="x">first item to compare</param>
        /// <param name="y">second item to compare</param>
        /// <returns>; &lt;0: x &lt; y; =0: x == y; &gt; 0: x &gt; y</returns>
        /// <remarks>
        /// Compares the 2 items using the list of property names and directions.
        /// </remarks>
        public int Compare(object x, object y)
        {
            return Compare((T)x, (T)y);
        }

        #endregion
    }
}
