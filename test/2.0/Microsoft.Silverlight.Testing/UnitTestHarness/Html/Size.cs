// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A simple type with both a Width and Height value.
    /// </summary>
    /// <typeparam name="T">The type of width and height property.</typeparam>
    public class Size<T>
    {
        /// <summary>
        /// Initializes a new Size instance.
        /// </summary>
        public Size() { }

        /// <summary>
        /// Initializes a new Size instance.
        /// </summary>
        /// <param name="width">The width.</param>
        /// <param name="height">The height.</param>
        public Size(T width, T height) 
        { 
            Width = width;
            Height = height;
        }

        /// <summary>
        /// Gets or sets the width.
        /// </summary>
        public virtual T Width
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the height.
        /// </summary>
        public virtual T Height
        {
            get;
            set;
        }
   }
}