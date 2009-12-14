// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Collections.Generic; 
using System.ComponentModel;
using System.Globalization;
using System.Windows.Controls;
using System.Windows;
 
namespace System.Windows
{
    /// <summary> 
    /// Converts instances of the String type to FontWeight instances.
    /// </summary>
    public sealed partial class FontWeightConverter : TypeConverter 
    { 
        /// <summary>
        /// FontWeight known values. 
        /// </summary>
        private static Dictionary<string, FontWeight> KnownValues =
            new Dictionary<string, FontWeight>(10, StringComparer.OrdinalIgnoreCase) 
            {
                { "Black", FontWeights.Black },
                { "Bold", FontWeights.Bold }, 
                { "ExtraBlack", FontWeights.ExtraBlack }, 
                { "ExtraBold", FontWeights.ExtraBold },
                { "ExtraLight", FontWeights.ExtraLight }, 
                { "Light", FontWeights.Light },
                { "Medium", FontWeights.Medium },
                { "Normal", FontWeights.Normal }, 
                { "SemiBold", FontWeights.SemiBold },
                { "Thin", FontWeights.Thin }
            }; 
 
        /// <summary>
        /// Initializes a new instance of the FontWeightConverter class. 
        /// </summary>
        public FontWeightConverter()
        { 
        }

        /// <summary> 
        /// Returns a value that indicates whether this converter can convert an 
        /// object of the given type to an instance of FontWeight.
        /// </summary> 
        /// <param name="sourceType">
        /// The type of the source that is being evaluated for conversion.
        /// </param> 
        /// <returns>
        /// true if the converter can convert the provided type to an instance
        /// of FontWeight; otherwise, false. 
        /// </returns> 
#if NET_2_1
	override
#endif
        public bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
        { 
            return TypeConverters.CanConvertFrom<FontWeight>(sourceType);
        }
 
        /// <summary>
        /// Attempts to convert a specified object to an instance of FontWeight.
        /// </summary> 
        /// <param name="value">The object being converted.</param> 
        /// <returns>
        /// The instance of FontWeight created from the converted object. 
        /// </returns>
#if NET_2_1
	override
#endif
        public object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        { 
            return TypeConverters.ConvertFrom<FontWeight>(this, value);
        }
    }
} 
