// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections;
using System.ComponentModel;
using System.Globalization;
using System.Reflection;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// Converts a FontUnit to and from a specified data type.
    /// </summary>
    public class FontUnitConverter : TypeConverter 
    {
        /// <summary>
        /// Determines if the specified data type can be converted to a 
        /// FontUnit.
        /// </summary>
        /// <param name="context">Context matches the desktop framework's 
        /// signature for this method.</param>
        /// <param name="sourceType">The source type.</param>
        /// <returns>Returns a value, whether it can convert the object.</returns>
        public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType) 
        {
            if (sourceType == typeof(string)) 
            {
                return true;
            }
            else 
            {
                return base.CanConvertFrom(sourceType);
            }
        }

        /// <summary>
        /// Converts the specified object into a FontUnit.
        /// </summary>
        /// <param name="context">Context matches the desktop framework's 
        /// signature for this method.</param>
        /// <param name="culture">The culture information.</param>
        /// <param name="value">The value to convert.</param>
        /// <returns>Returns the converted object.</returns>
        public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        {
            if (value == null)
            {
                return null;
            }
            string stringValue = value as string;
            if (stringValue != null) 
            {
                string textValue = stringValue.Trim();
                if (textValue.Length == 0) 
                {
                    return FontUnit.Empty;
                }
                return FontUnit.Parse(textValue, CultureInfo.InvariantCulture);
            }
            else 
            {
                return base.ConvertFrom(value);
            }
        }

        /// <summary>
        /// Returns a value indicating whether the converter can
        /// convert to the specified destination type.
        /// </summary>
        /// <param name="context">Context matches the desktop framework's 
        /// signature for this method.</param>
        /// <param name="destinationType">The type to convert to.</param>
        /// <returns>Returns if the type can convert.</returns>
        public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
        {
            if (destinationType == typeof(string))
            {
                return true;
            }
            else 
            {
                return base.CanConvertTo(destinationType);
            }
        }

        /// <summary>
        /// Converts the specified FontUnit into the specified Type.
        /// </summary>
        /// <param name="context">Context matches the desktop framework's 
        /// signature for this method.</param>
        /// <param name="culture">The culture information.</param>
        /// <param name="value">The value to convert.</param>
        /// <param name="destinationType">The destination type.</param>
        /// <returns>Returns the converted object.</returns>
        public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
        {
            if (destinationType == typeof(string)) 
            {
                if ((value == null) || (((FontUnit)value).Type == FontSize.NotSet))
                {
                    return String.Empty;
                }
                else
                {
                    return ((FontUnit)value).ToString();
                }
            }
            else 
            {
                return base.ConvertTo(value, destinationType);
            }
        }
    }
}