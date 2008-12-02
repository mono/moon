// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.ComponentModel;
using System.Globalization;
using System.Reflection;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A type converter for Unit.
    /// </summary>
    public class UnitConverter : TypeConverter
    {
        /// <summary>
        ///   Returns a value indicating whether the unit converter can 
        ///   convert from the specified source type.
        /// </summary>
        /// <param name="context">Context matches the desktop framework's 
        /// signature for this method.</param>
        /// <param name="sourceType">The source type for the conversion.</param>
        /// <returns>Returns the conversion information.</returns>
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
        ///   Returns a value indicating whether the converter can
        ///   convert to the specified destination type.
        /// </summary>
        /// <param name="context">Context matches the desktop framework's 
        /// signature for this method.</param>
        /// <param name="destinationType">The destination type for the 
        /// conversion.</param>
        /// <returns>Returns whether the conversion will work.</returns>
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
        /// Performs type conversion from the given value into a Unit.
        /// </summary>
        /// <param name="context">Context matches the desktop framework's 
        /// signature for this method.</param>
        /// <param name="culture">The culture information.</param>
        /// <param name="value">The value to convert from.</param>
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
                    return Unit.Empty;
                }
                else
                {
                    return Unit.Parse(textValue, CultureInfo.InvariantCulture);
                }
            }
            else
            {
                return base.ConvertFrom(value);
            }
        }

        /// <summary>
        /// Performs type conversion to the specified destination type.
        /// </summary>
        /// <param name="context">Context matches the desktop framework's 
        /// signature for this method.</param>
        /// <param name="culture">The culture information.</param>
        /// <param name="value">The value to convert.</param>
        /// <param name="destinationType">The destination Type.</param>
        /// <returns>Returns the converted object.</returns>
        public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
        {
            if (destinationType == typeof(string))
            {
                if ((value == null) || ((Unit)value).IsEmpty)
                {
                    return String.Empty;
                }
                else
                {
                    return ((Unit)value).ToString();
                }
            }
            else
            {
                return base.ConvertTo(value, destinationType);
            }
        }
    }
}