// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.ComponentModel;
using System.Collections;    
using System.Globalization;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// Converts a string with font names separated by commas to and from an 
    /// array of strings containing individual names.
    /// </summary>
    public class FontNamesConverter : TypeConverter 
    {
        /// <summary>
        /// Determines if the specified data type can be converted to an array 
        /// of strings containing individual font names.
        /// </summary>
        /// <param name="context">Context matches the desktop framework's 
        /// signature for this method.</param>
        /// <param name="sourceType">The source type.</param>
        /// <returns>Returns whether the object can be converted.</returns>
        public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType) 
        {
            if (sourceType == typeof(string)) 
            {
                return true;
            }
            return false;
        }

        /// <summary>
        /// Parses a string that represents a list of font names separated by 
        /// commas into an array of strings containing individual font names.
        /// </summary>
        /// <param name="context">Context matches the desktop framework's 
        /// signature for this method.</param>
        /// <param name="culture">The culture information.</param>
        /// <param name="value">The object value.</param>
        /// <returns>The names array from the object.</returns>
        public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        {
            string sv = value as string;
            if (sv != null) 
            {
                if (((string)sv).Length == 0)
                {
                    return new string[0];
                }
                
                char comma = CultureInfo.InvariantCulture.TextInfo.ListSeparator[0];
                string[] names = sv.Split(new char[] { comma });
                for (int i = 0; i < names.Length; i++) 
                {
                    names[i] = names[i].Trim();
                }
                return names;
            }
            throw new InvalidOperationException();
        }

        /// <summary>
        /// Creates a string that represents a list of font names separated 
        /// by commas from an array of strings containing individual font names.
        /// </summary>
        /// <param name="context">Context matches the desktop framework's 
        /// signature for this method.</param>
        /// <param name="culture">The culture information.</param>
        /// <param name="value">The value object.</param>
        /// <param name="destinationType">The destination type.</param>
        /// <returns>Returns the converted object.</returns>
        public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
        {
            if (destinationType == typeof(string)) 
            {
                if (value == null) 
                {
                    return "";
                }
                return string.Join(culture.TextInfo.ListSeparator, ((string[])value));
            }
            throw new InvalidOperationException();
        }
    }
}