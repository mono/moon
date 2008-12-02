// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// A dictionary that can store just about any kind of object, keyed off any
    /// object.  As a Hashtable, it permits using keys of 
    /// - enums
    /// - objects
    /// - types
    /// 
    /// This makes it ideal for use in decorating and adorning special types in
    /// the system, such as log messages.
    /// 
    /// A strongly-typed decorator key could come from an enum value, while a
    /// prototype extension to the test system could simply use a string key for
    /// organizing.  
    /// 
    /// In all instances, the resulting object needs to be cast appropriately.
    /// </summary>
    public class DecoratorDictionary : Dictionary<object, object>
    {
        /// <summary>
        /// Check if a decorator exists.
        /// </summary>
        /// <param name="decoratorKey">The decorator key object.</param>
        /// <returns>
        /// Returns a value indicating whether the decorator key exists.  Even
        /// if the key exists, the instance value for the key could be set to
        /// null, yielding a null instance.
        /// </returns>
        public bool HasDecorator(object decoratorKey)
        {
            return ContainsKey(decoratorKey);
        }

        /// <summary>
        /// Check if a set of decorators exists.
        /// </summary>
        /// <param name="decorators">The set of decorator(s) of interest.</param>
        /// <returns>
        /// Returns a value indicating whether the decorators of interest were
        /// present.
        /// </returns>
        public bool HasDecorators(params object[] decorators)
        {
            if (decorators != null)
            {
                foreach (object decorator in decorators)
                {
                    if (!ContainsKey(decorator))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        /// <summary>
        /// Retrieves the decorator instance if it exists.  If not, this method
        /// returns null.
        /// </summary>
        /// <param name="decoratorKey">The decorator key object.</param>
        /// <returns>
        /// Returns the instance or null if it does not exist.  No exceptions
        /// are thrown in this method.
        /// </returns>
        public object GetDecorator(object decoratorKey)
        {
            object decorator = null;
            TryGetValue(decoratorKey, out decorator);
            return decorator;
        }
    }
}