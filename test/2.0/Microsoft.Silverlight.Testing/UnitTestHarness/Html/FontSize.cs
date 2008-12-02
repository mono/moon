// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.
using System;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// Specifies the font size.
    /// </summary>
    public enum FontSize 
    {
        /// <summary>
        /// The font size is not set.
        /// </summary>
        NotSet = 0,
        
        /// <summary>
        /// The font size is specified as point values.
        /// </summary>
        AsUnit = 1,
        
        /// <summary>
        /// The font size is smaller.
        /// </summary>
        Smaller = 2,

        /// <summary>
        /// The font size is larger.
        /// </summary>
        Larger = 3,

        /// <summary>
        /// The font size is extra extra small.
        /// </summary>
        XXSmall = 4,

        /// <summary>
        /// The font size is extra small.
        /// </summary>
        XSmall = 5,

        /// <summary>
        /// The font size is small.
        /// </summary>
        Small = 6,

        /// <summary>
        /// The font size is medium.
        /// </summary>
        Medium = 7,

        /// <summary>
        /// The font size is large.
        /// </summary>
        Large = 8,

        /// <summary>
        /// The font size is extra large.
        /// </summary>
        XLarge = 9,

        /// <summary>
        /// The font size is extra extra large.
        /// </summary>
        XXLarge = 10
    }
}