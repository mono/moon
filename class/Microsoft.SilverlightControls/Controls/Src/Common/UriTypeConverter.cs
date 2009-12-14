// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.ComponentModel; 
using System.Globalization; 
using System.Windows.Controls;
 
namespace System
{
    /// <summary> 
    /// Converts a String type to a Uri type.
    /// </summary>
    public sealed class UriTypeConverter : TypeConverter 
    { 
        /// <summary>
        /// Initializes a new instance of the UriTypeConverter class. 
        /// </summary>
        public UriTypeConverter()
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
        /// true if sourceType is a String type or a Uri type that can be
        /// assigned from sourceType; otherwise, false. 
        /// </returns> 
#if NET_2_1
	override
#endif
        public bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
        { 
            return TypeConverters.CanConvertFrom<Uri>(sourceType);
        }
 
        /// <summary>
        /// Converts the specified object to a Uri.
        /// </summary> 
        /// <param name="value">Object to convert into a Uri.</param> 
        /// <returns>A Uri that represents the converted text.</returns>
#if NET_2_1
	override
#endif
        public object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value) 
        {
            return TypeConverters.ConvertFrom<Uri>(this, value);
        } 
    }
}
