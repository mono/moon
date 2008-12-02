// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Collections.Generic; 
using System.ComponentModel;
using System.Globalization;
using System.Windows.Controls;
 
namespace System.Windows
{
    /// <summary> 
    /// Converts instances of the String type to FontStretch instances.
    /// </summary>
    public sealed partial class FontStretchConverter : TypeConverter 
    { 
        /// <summary>
        /// FontStretch known values. 
        /// </summary>
        private static Dictionary<string, FontStretch> KnownValues =
            new Dictionary<string, FontStretch>(9, StringComparer.OrdinalIgnoreCase) 
            {
                { "Condensed", FontStretches.Condensed },
                { "Expanded", FontStretches.Expanded }, 
                { "ExtraCondensed", FontStretches.ExtraCondensed }, 
                { "ExtraExpanded", FontStretches.ExtraExpanded },
                { "Normal", FontStretches.Normal }, 
                { "SemiCondensed", FontStretches.SemiCondensed },
                { "SemiExpanded", FontStretches.SemiExpanded },
                { "UltraCondensed", FontStretches.UltraCondensed }, 
                { "UltraExpanded", FontStretches.UltraExpanded }
            };
 
        /// <summary> 
        /// Initializes a new instance of the FontStretchConverter class.
        /// </summary> 
        public FontStretchConverter()
        {
        } 

        /// <summary>
        /// Returns a value that indicates whether this converter can convert an 
        /// object of the given type to an instance of FontStretch. 
        /// </summary>
        /// <param name="sourceType"> 
        /// The type of the source that is being evaluated for conversion.
        /// </param>
        /// <returns> 
        /// true if the converter can convert the provided type to an instance
        /// of FontStretch; otherwise, false.
        /// </returns> 
#if NET_2_1
	override
#endif
        public bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType) 
        {
            return TypeConverters.CanConvertFrom<FontStretch>(sourceType); 
        }

        /// <summary> 
        /// Attempts to convert a specified object to an instance of
        /// FontStretch.
        /// </summary> 
        /// <param name="value">The object being converted.</param> 
        /// <returns>
        /// The instance of FontStretch created from the converted object. 
        /// </returns>
#if NET_2_1
	override
#endif
        public object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        { 
            return TypeConverters.ConvertFrom<FontStretch>(this, value);
        }
    } 
}
