// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// Set of extension methods used by the harness.
    /// </summary>
    public static class HarnessExtensions
    {
        /// <summary>
        /// An AddRange implementation for the generic IList interface.
        /// </summary>
        /// <typeparam name="TListType">The list type.</typeparam>
        /// <param name="list">The list object.</param>
        /// <param name="collection">The collection to copy into the list.</param>
        public static void AddRange<TListType>(this IList<TListType> list, IEnumerable<TListType> collection)
        {
            if (list == null || collection == null)
            {
                return;
            }
            foreach (TListType value in collection)
            {
                list.Add(value);
            }
        }

        /// <summary>
        /// Replace a list's contents with the items in the IEnumerable.
        /// </summary>
        /// <typeparam name="TListType">The list type.</typeparam>
        /// <param name="list">The list object.</param>
        /// <param name="sequence">The sequence to copy into the list.</param>
        public static void Replace<TListType>(this IList<TListType> list, IEnumerable<TListType> sequence)
        {
            if (list == null)
            {
                return;
            }
            if (sequence == null)
            {
                throw new ArgumentNullException("sequence");
            }

            List<TListType> copy = sequence.ToList();
            list.Clear();
            list.AddRange(copy);
        }
    }
}