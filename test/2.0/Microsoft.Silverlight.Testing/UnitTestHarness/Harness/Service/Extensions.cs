// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using System.Collections.Generic;
using System.Xml.Linq;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// Extension methods.
    /// </summary>
    public static class Extensions
    {
        /// <summary>
        /// Transform the XElement into a dictionary of key/value pairs.
        /// </summary>
        /// <typeparam name="T">The type of enumeration.</typeparam>
        /// <typeparam name="K">The key type.</typeparam>
        /// <typeparam name="R">The value type.</typeparam>
        /// <param name="that">The root enumerable.</param>
        /// <param name="keySelector">The key selector.</param>
        /// <param name="itemSelector">The item selector.</param>
        /// <returns>Returns a new dictionary.</returns>
        [SuppressMessage("Microsoft.Naming", "CA1715:IdentifiersShouldHaveCorrectPrefix", MessageId = "T", Justification = "Simple LINQ statement.")]
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "R", Justification = "Simple LINQ statement.")]
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "K", Justification = "Simple LINQ statement.")]
        public static Dictionary<K, R> ToTransformedDictionary<T, K, R>(this IEnumerable<T> that, Func<T, K> keySelector, Func<T, R> itemSelector)
        {
            Dictionary<K, R> dictionary = new Dictionary<K, R>();
            foreach (var item in that)
            {
                dictionary[keySelector(item)] = itemSelector(item);
            }
            return dictionary;
        }
    }
}