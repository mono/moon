// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.ComponentModel;
using System.Globalization;
using System.Windows.Controls;
using System.Windows.Media;

namespace System.Windows.Media
{
    /// <summary>
    /// Converts instances of the String type to FontFamily instances. 
    /// </summary>
    public sealed partial class FontFamilyConverter : TypeConverter
    { 
        /// <summary> 
        /// Initializes a new instance of the FontFamilyConverter class.
        /// </summary> 
        public FontFamilyConverter()
        {
        } 

        /// <summary>
        /// Returns a value that indicates whether this converter can convert an 
        /// object of the given type to an instance of FontFamily. 
        /// </summary>
        /// <param name="sourceType"> 
        /// The type of the source that is being evaluated for conversion.
        /// </param>
        /// <returns> 
        /// true if the converter can convert the provided type to an instance
        /// of FontFamily; otherwise, false.
        /// </returns> 
#if NET_2_1
	override
#endif
	public bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType) 
        {
            return TypeConverters.CanConvertFrom<FontFamily>(sourceType); 
        }

        /// <summary> 
        /// Attempts to convert a specified object to an instance of FontFamily.
        /// </summary>
        /// <param name="value">The object being converted.</param> 
        /// <returns> 
        /// The instance of FontFamily created from the converted object.
        /// </returns> 
#if NET_2_1
	override
#endif
	public object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        {
            return TypeConverters.ConvertFrom<FontFamily>(this, value); 
        }
    }
}
