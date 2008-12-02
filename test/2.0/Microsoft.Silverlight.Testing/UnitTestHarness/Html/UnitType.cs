// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// Specifies the unit of measure.
    /// </summary>
    public enum UnitType
    {
        /// <summary>
        /// The unknown type.
        /// </summary>
        Unknown = 0,

        /// <summary>
        /// Measurement is in pixels.
        /// </summary>
        Pixel = 1,

        /// <summary>
        /// Measurement is in points.  A point represents 1/72 of an inch.
        /// </summary>
        Point = 2,
        
        /// <summary>
        /// Measurement is in picas.  A pica represents 12 points.
        /// </summary>
        Pica = 3,
        
        /// <summary>
        /// Measurement is in inches.
        /// </summary>
        Inch = 4,
        
        /// <summary>
        /// Measurement is in millimeters.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Mm", Justification = "This matches the W3C spec as is.")]
        Mm = 5,
        
        /// <summary>
        /// Measurement is in centimeters.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "Cm", Justification = "The casing as is matches the W3C spec.")]
        Cm = 6,
        
        /// <summary>
        /// Measurement is a percentage relative to the parent.
        /// </summary>
        Percentage = 7,
        
        /// <summary>
        /// Measurement is relative to the height of the parent element's font.
        /// </summary>
        Em = 8,
        
        /// <summary>
        /// Measurement is relative to the height of the lowercase letter x of 
        /// the parent element's font.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1711:IdentifiersShouldNotHaveIncorrectSuffix", Justification = "This value matches the W3C spec.")]
        Ex = 9,
    }
}