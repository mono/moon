// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using Microsoft.Silverlight.Testing.Harness;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A class that does a lazy lookup when needed using reflection.
    /// </summary>
    public class LazyMethodInfo
    {
        /// <summary>
        /// Attribute Type instance.
        /// </summary>
        private Type _attributeType;

        /// <summary>
        /// The Type to search with.
        /// </summary>
        private Type _searchType;

        /// <summary>
        /// Whether the search has happened.
        /// </summary>
        private bool _hasSearched;

        /// <summary>
        /// The method reflection object.
        /// </summary>
        private MethodInfo _methodInfo;

        /// <summary>
        /// Construct a new lazy method wrapper.
        /// </summary>
        /// <param name="attributeType">The attribute type.</param>
        public LazyMethodInfo(Type attributeType)
        {
            _attributeType = attributeType;
        }

        /// <summary>
        /// Construct a new lazy method wrapper.
        /// </summary>
        /// <param name="searchType">Type to search.</param>
        /// <param name="attributeType">Attribute type.</param>
        public LazyMethodInfo(Type searchType, Type attributeType)
            : this(attributeType)
        {
            _searchType = searchType;
        }

        /// <summary>
        /// Gets the type of attribute the lazy method is searching for.
        /// </summary>
        protected Type AttributeType
        {
            get { return _attributeType; }
        }

        /// <summary>
        /// Gets the underlying type that is searched.
        /// </summary>
        protected Type SearchType
        {
            get { return _searchType; }
        }

        /// <summary>
        /// Gets or sets a value indicating whether a lookup has already been attempted.
        /// </summary>
        protected bool HasSearched
        {
            get { return _hasSearched; }
            set { _hasSearched = value; }
        }

        /// <summary>
        /// Gets or sets the underlying MethodInfo from reflection.
        /// </summary>
        protected MethodInfo MethodInfo
        {
            get { return _methodInfo; }
            set { _methodInfo = value; }
        }

        /// <summary>
        /// Does a search and retrieves the method information.
        /// </summary>
        /// <returns>The method reflection object.</returns>
        [SuppressMessage("Microsoft.Design", "CA1024:UsePropertiesWhereAppropriate", Justification = "Helps document the more advanced lazy load functionality to API users")]
        public MethodInfo GetMethodInfo()
        {
            if (!_hasSearched)
            {
                Search();
            }
            return _methodInfo;
        }

        /// <summary>
        /// Whether the type has a method info.
        /// </summary>
        /// <returns>A value indicating whether the method information has 
        /// been found.</returns>
        public bool HasMethodInfo()
        {
            if (!_hasSearched)
            {
                Search();
            }
            return (_methodInfo != null);
        }

        /// <summary>
        /// Perform a search on the type.
        /// </summary>
        protected virtual void Search()
        {
            if (_hasSearched)
            {
                return;
            }
            _methodInfo = ReflectionUtility.GetOneMethodWithAttribute(_searchType, _attributeType);
            _hasSearched = true;
        }
    }
}