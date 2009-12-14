// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.ComponentModel; 
using System.Windows.Controls; 
using System.Globalization;
 
namespace System.Windows
{
    /// <summary> 
    /// Converts a String type to a nullable boolean type.
    /// </summary>
    public sealed class NullableBoolConverter : TypeConverter 
    { 
        /// <summary>
        /// Initializes a new instance of the NullableBoolConverter class. 
        /// </summary>
        public NullableBoolConverter()
        { 
        }

        /// <summary> 
        /// Returns whether this converter can convert an object of the given 
        /// type to the type of this converter.
        /// </summary> 
        /// <param name="sourceType">
        /// A type that represents the type that you want to convert from.
        /// </param> 
        /// <returns>
        /// true if sourceType is a String type or a Boolean? type that can be
        /// assigned from sourceType; otherwise, false. 
        /// </returns> 
#if NET_2_1
	override
#else
	new
#endif
        public bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
        { 
            return TypeConverters.CanConvertFrom<bool?>(sourceType) ||
                (sourceType == typeof(bool));
        } 

        /// <summary>
        /// Converts the specified object to a bool?. 
        /// </summary> 
        /// <param name="value">Object to convert into a bool?.</param>
        /// <returns>A bool? that represents the converted text.</returns> 
#if NET_2_1
	override
#else
	new
#endif
        public object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        {
            if (value is bool) 
            {
                return (bool?) value;
            } 
            if (value is string)
            {
                string text = (string) value;

                return (!string.IsNullOrEmpty(text)) ?
                    (bool?) bool.Parse(text) : 
                    null; 
            }
 
            return TypeConverters.ConvertFrom<bool?>(this, value);
        } 
    } 
}
