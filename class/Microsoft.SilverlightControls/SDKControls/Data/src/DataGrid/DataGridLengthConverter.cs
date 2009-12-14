// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.ComponentModel;
using System.Globalization;

namespace System.Windows.Controls
{
    /// <summary>
    /// DataGridLengthConverter - Converter class for converting instances of other types to and from DataGridLength instances.
    /// </summary> 
    public class DataGridLengthConverter : TypeConverter
    {
        #region Data

        private static string[] _valueInvariantUnitStrings = { "auto", "sizetocells", "sizetoheader" };
        private static DataGridLength[] _valueInvariantDataGridLengths = { DataGridLength.Auto, DataGridLength.SizeToCells, DataGridLength.SizeToHeader };
        
        #endregion Data

        #region Methods

        /// <summary>
        /// Checks whether or not this class can convert from a given type.
        /// </summary>
        /// <param name="context">
        /// An ITypeDescriptorContext that provides a format context. 
        /// </param>
        /// <param name="sourceType">The Type being queried for support.</param>
        /// <returns>
        /// <c>true</c> if thie converter can convert from the provided type, 
        /// <c>false</c> otherwise.
        /// </returns>
        public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
        {
            // We can only handle strings, integral and floating types
            TypeCode tc = Type.GetTypeCode(sourceType);
            switch (tc)
            {
                case TypeCode.String:
                case TypeCode.Decimal:
                case TypeCode.Single:
                case TypeCode.Double:
                case TypeCode.Int16:
                case TypeCode.Int32:
                case TypeCode.Int64:
                case TypeCode.UInt16:
                case TypeCode.UInt32:
                case TypeCode.UInt64:
                    return true;
                default:
                    return false;
            }
        }

        /// <summary>
        /// Checks whether or not this class can convert to a given type.
        /// </summary>
        /// <param name="context">
        /// An ITypeDescriptorContext that provides a format context. 
        /// </param>
        /// <param name="destinationType">The Type being queried for support.</param>
        /// <returns>
        /// <c>true</c> if this converter can convert to the provided type, 
        /// <c>false</c> otherwise.
        /// </returns>
        public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
        {
            return destinationType == typeof(string);
        }

        /// <summary>
        /// Attempts to convert to a DataGridLength from the given object.
        /// </summary>
        /// <param name="context">
        /// An ITypeDescriptorContext that provides a format context. 
        /// </param>
        /// <param name="culture">
        /// The CultureInfo to use for the conversion. 
        /// </param>
        /// <param name="value">The object to convert to a GridLength.</param>
        /// <returns>
        /// The GridLength instance which was constructed.
        /// </returns>
        /// <exception cref="NotSupportedException">
        /// A NotSupportedException is thrown if the example object is null.
        /// </exception>
        public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        {
            // GridLengthConverter in WPF throws a NotSupportedException on a null value as well.
            if (value == null)
            {
                throw DataGridError.DataGridLengthConverter.CannotConvertFrom("(null)");
            }

            string stringValue = value as string;
            if (stringValue != null)
            {
                stringValue = stringValue.Trim();
                int index = 0;
                while (index < _valueInvariantUnitStrings.Length && !stringValue.EndsWith(_valueInvariantUnitStrings[index], StringComparison.OrdinalIgnoreCase))
                {
                    index++;
                }
                if (index < _valueInvariantUnitStrings.Length)
                {
                    return _valueInvariantDataGridLengths[index];
                }
            }

            // Conversion from numeric type, WPF lets Convert exceptions bubble out here as well
            double doubleValue = Convert.ToDouble(value, culture ?? CultureInfo.CurrentCulture);
            if (double.IsNaN(doubleValue))
            {
                // WPF returns Auto in this case as well
                return DataGridLength.Auto;
            }
            else
            {
                return new DataGridLength(doubleValue);
            }
        }

        /// <summary>
        /// Attempts to convert a DataGridLength instance to the given type.
        /// </summary>
        /// <param name="context">
        /// An ITypeDescriptorContext that provides a format context. 
        /// </param>
        /// <param name="culture">
        /// The CultureInfo to use for the conversion. 
        /// </param>
        /// <param name="value">The DataGridLength to convert.</param>
        /// <param name="destinationType">The type to which to convert the DataGridLength instance.</param>
        /// <returns>
        /// The object which was constructed.
        /// </returns>
        /// <exception cref="ArgumentNullException">
        /// An ArgumentNullException is thrown if the example object is null.
        /// </exception>
        /// <exception cref="NotSupportedException">
        /// A NotSupportedException is thrown if the object is not null and is not a DataGridLength,
        /// or if the destinationType isn't one of the valid destination types.
        /// </exception>
        ///<SecurityNote>
        ///     Critical: calls InstanceDescriptor ctor which LinkDemands
        ///     PublicOK: can only make an InstanceDescriptor for DataGridLength, not an arbitrary class
        ///</SecurityNote> 
        public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
        {
            if (destinationType == null)
            {
                throw new ArgumentNullException("destinationType");
            }
            if (destinationType != typeof(string))
            {
                throw DataGridError.DataGridLengthConverter.CannotConvertTo(destinationType.ToString());
            }
            DataGridLength? dataGridLength = value as DataGridLength?;
            if (!dataGridLength.HasValue)
            {
                throw DataGridError.DataGridLengthConverter.InvalidDataGridLength("value");
            }
            else
            {
                // Convert dataGridLength to a string
                switch (dataGridLength.Value.UnitType)
                {
                    //  for Auto print out "Auto". value is always "1.0"
                    case DataGridLengthUnitType.Auto:
                        return "Auto";

                    case DataGridLengthUnitType.SizeToHeader:
                        return "SizeToHeader";

                    case DataGridLengthUnitType.SizeToCells:
                        return "SizeToCells";

                    // 







                    
                    default:
                        // WPF lets Convert Exceptions bubble out as well
                        return (Convert.ToString(dataGridLength.Value.Value, culture ?? CultureInfo.CurrentCulture));
                }
            }
        }

        #endregion Methods
    }
}
