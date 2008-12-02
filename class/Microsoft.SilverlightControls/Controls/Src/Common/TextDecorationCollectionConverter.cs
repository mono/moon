// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.ComponentModel; 
using System.Collections.Generic;
using System.Globalization;
using System.Windows.Controls;
 
namespace System.Windows
{
    /// <summary> 
    /// Converts instances of the String type to TextDecorationCollection
    /// instances.
    /// </summary> 
    public sealed partial class TextDecorationCollectionConverter : TypeConverter 
    {
        /// <summary> 
        /// TextDecorationCollection known values.
        /// </summary>
        private static Dictionary<string, TextDecorationCollection> KnownValues = 
            new Dictionary<string, TextDecorationCollection>(2, StringComparer.OrdinalIgnoreCase)
            {
                { string.Empty, null }, 
                { "Underline", TextDecorations.Underline } 
            };
 
        /// <summary>
        /// Initializes a new instance of the TextDecorationCollectionConverter
        /// class. 
        /// </summary>
        public TextDecorationCollectionConverter()
        { 
        } 

        /// <summary> 
        /// Returns a value that indicates whether this converter can convert an
        /// object of the given type to an instance of TextDecorationCollection.
        /// </summary> 
        /// <param name="sourceType">
        /// The type of the source that is being evaluated for conversion.
        /// </param> 
        /// <returns> 
        /// true if the converter can convert the provided type to an instance
        /// of TextDecorationCollection; otherwise, false. 
        /// </returns>
#if NET_2_1
	override
#endif
        public bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
        { 
            return TypeConverters.CanConvertFrom<TextDecorationCollection>(sourceType);
        }
 
        /// <summary> 
        /// Attempts to convert a specified object to an instance of
        /// TextDecorationCollection. 
        /// </summary>
        /// <param name="value">The object being converted.</param>
        /// <returns> 
        /// The instance of TextDecorationCollection created from the converted
        /// object.
        /// </returns> 
#if NET_2_1
	override
#endif
        public object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value) 
        {
            return TypeConverters.ConvertFrom<TextDecorationCollection>(this, value); 
        }
    }
}
