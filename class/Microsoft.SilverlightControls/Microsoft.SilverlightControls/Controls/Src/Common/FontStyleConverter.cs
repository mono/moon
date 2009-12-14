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
    /// Converts instances of the String type to FontStyle instances.
    /// </summary>
    public sealed partial class FontStyleConverter : TypeConverter 
    { 
        /// <summary>
        /// FontStyle known values. 
        /// </summary>
        private static Dictionary<string, FontStyle> KnownValues =
            new Dictionary<string, FontStyle>(2, StringComparer.OrdinalIgnoreCase) 
            {
                { "Normal", FontStyles.Normal },
                { "Italic", FontStyles.Italic } 
            }; 

        /// <summary> 
        /// Initializes a new instance of the FontStyleConverter class.
        /// </summary>
        public FontStyleConverter() 
        {
        }
 
        /// <summary> 
        /// Returns a value that indicates whether this converter can convert an
        /// object of the given type to an instance of FontStyle. 
        /// </summary>
        /// <param name="sourceType">
        /// The type of the source that is being evaluated for conversion. 
        /// </param>
        /// <returns>
        /// true if the converter can convert the provided type to an instance 
        /// of FontStyle; otherwise, false. 
        /// </returns>
#if NET_2_1
	override
#endif
        public bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType) 
        {
            return TypeConverters.CanConvertFrom<FontStyle>(sourceType);
        } 

        /// <summary>
        /// Attempts to convert a specified object to an instance of FontStyle. 
        /// </summary> 
        /// <param name="value">The object being converted.</param>
        /// <returns> 
        /// The instance of FontStyle created from the converted object.
        /// </returns>
#if NET_2_1
	override
#endif
        public object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value) 
        {
            return TypeConverters.ConvertFrom<FontStyle>(this, value);
        }
    } 
}
