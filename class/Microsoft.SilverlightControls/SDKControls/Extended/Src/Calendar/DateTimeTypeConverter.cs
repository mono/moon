// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.ComponentModel;
using System.Globalization;

namespace System.Windows.Controls
{
    /// <summary>
    /// Represents the Converter class for the DateTime type.
    /// </summary>
    public class DateTimeTypeConverter : TypeConverter
    {
        /// <summary>
        /// Overrides the CanConvertFrom method of TypeConverter class.
        /// </summary>
        /// <param name="context">
        /// An ITypeDescriptorContext that provides a format context. 
        /// </param>
        /// <param name="sourceType"></param>
        /// <returns></returns>
        public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
        {
            return sourceType == typeof(string);
        }

        /// <summary>
        /// Checks whether or not this class can convert to a given type.
        /// </summary>
        /// <param name="context"></param>
        /// <param name="destinationType"></param>
        /// <returns></returns>
        public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
        {
            return destinationType == typeof(string);
        }

        /// <summary>
        /// Overrides the ConvertFrom method of TypeConverter class.
        /// </summary>
        /// <param name="context">
        /// An ITypeDescriptorContext that provides a format context. 
        /// </param>
        /// <param name="culture">
        /// The CultureInfo to use for the conversion. 
        /// </param>
        /// <param name="value"></param>
        /// <returns></returns>
        public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        {
            if (culture == null)
            {
                throw new ArgumentNullException("culture");
            }

            if (value == null)
            {
                throw new ArgumentNullException("value");
            }

            DateTimeFormatInfo dateTimeFormatInfo = (DateTimeFormatInfo)culture.GetFormat(typeof(DateTimeFormatInfo));
            DateTime d = DateTime.ParseExact(value.ToString(), dateTimeFormatInfo.ShortDatePattern, culture);
            return d;
        }

        /// <summary>
        /// Attempts to convert a DateTime instance to the given type.
        /// </summary>
        /// <param name="context"></param>
        /// <param name="culture"></param>
        /// <param name="value"></param>
        /// <param name="destinationType"></param>
        /// <returns></returns>
        public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
        {
            if (destinationType == null)
            {
                throw new ArgumentNullException("destinationType");
            }

            if (culture == null)
            {
                throw new ArgumentNullException("culture");
            }

            DateTime? d = value as DateTime?;

            if (!d.HasValue || destinationType != typeof(string))
            {
                throw new NotSupportedException();
            }
            else
            {
                DateTimeFormatInfo dateTimeFormatInfo = (DateTimeFormatInfo)culture.GetFormat(typeof(DateTimeFormatInfo));
                return d.Value.ToString(dateTimeFormatInfo.ShortDatePattern, culture);
            }
        }

    }
}

