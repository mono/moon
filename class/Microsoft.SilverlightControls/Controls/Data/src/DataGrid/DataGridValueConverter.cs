// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Globalization; 
using System.Windows.Data; 

namespace System.Windows.Controlsb1
{
    /// <summary>
    /// Default type converter that knows how to convert primitive types. 
    /// This class raises the DataConversionError event when a conversion fails.
    /// It tries to use the FallbackValue value when the provided value parameter
    /// causes a FormatException. 
    /// </summary> 
    internal class DataGridValueConverter : IValueConverter
    { 
        #region Events

        public event EventHandler<DataGridDataErrorEventArgs> DataConversionError; 

        #endregion Events
 
        public DataGridValueConverter(DataGridColumnBase dataGridColumn) 
        {
            this.Column = dataGridColumn; 
        }

        #region Public Properties 

        public DataGridColumnBase Column
        { 
            get; 
            private set;
        } 

        public object FallbackValue
        { 
            get;
            set;
        } 
 
        public DataGridRow Row
        { 
            get;
            set;
        } 

        #endregion Public Properties
 
        #region Public Methods 

        public static bool CanEdit(Type type) 
        {
            Debug.Assert(type != null);
 
            if (type.IsGenericType &&
                type.GetGenericTypeDefinition() == typeof(Nullable<>))
            { 
                type = type.GetGenericArguments()[0]; 
            }
 
            return
                type.IsEnum
                || type == typeof(System.String) 
                || type == typeof(System.Char)
                || type == typeof(System.DateTime)
                || type == typeof(System.Boolean) 
                || type == typeof(System.Byte) 
                || type == typeof(System.Single)
                || type == typeof(System.Double) 
                || type == typeof(System.Decimal)
                || type == typeof(System.Int16)
                || type == typeof(System.Int32) 
                || type == typeof(System.Int64)
                || type == typeof(System.UInt16)
                || type == typeof(System.UInt32) 
                || type == typeof(System.UInt64); 
        }
 
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return ConvertWithFallback(value, targetType, culture); 
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) 
        { 
            return ConvertWithFallback(value, targetType, culture);
        } 

        #endregion Public Methods
 
        #region Private Methods

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1800:DoNotCastUnnecessarily")] 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")] 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1820:TestForEmptyStringsUsingStringLength")]
        private static object Convert(object value, Type targetType, CultureInfo culture) 
        {
            Debug.Assert(targetType != null);
 
            if (targetType.IsGenericType &&
                targetType.GetGenericTypeDefinition() == typeof(Nullable<>))
            { 
                String strValue = value as String; 
                if (value == null || strValue == String.Empty)
                { 
                    // Special-case the String.Empty or null values to map to null for nullable types.
                    return null;
                } 
            }

            if (targetType == typeof(System.String)) 
            { 
                return System.Convert.ToString(value, culture);
            } 
            if (targetType == typeof(System.Int32?) || targetType == typeof(System.Int32))
            {
                return System.Convert.ToInt32(value, culture); 
            }
            if (targetType == typeof(System.Boolean?) || targetType == typeof(System.Boolean))
            { 
                return System.Convert.ToBoolean(value, culture); 
            }
            if (targetType == typeof(System.DateTime?) || targetType == typeof(System.DateTime)) 
            {
                return System.Convert.ToDateTime(value, culture);
            } 
            if (targetType == typeof(System.Decimal?) || targetType == typeof(System.Decimal))
            {
                return System.Convert.ToDecimal(value, culture); 
            } 
            if (targetType == typeof(System.Double?) || targetType == typeof(System.Double))
            { 
                return System.Convert.ToDouble(value, culture);
            }
            if (targetType == typeof(System.Single?) || targetType == typeof(System.Single)) 
            {
                return System.Convert.ToSingle(value, culture);
            } 
            if (targetType == typeof(System.Int16?) || targetType == typeof(System.Int16)) 
            {
                return System.Convert.ToInt16(value, culture); 
            }
            if (targetType == typeof(System.Byte?) || targetType == typeof(System.Byte))
            { 
                return System.Convert.ToByte(value, culture);
            }
            if (targetType == typeof(System.Char?) || targetType == typeof(System.Char)) 
            { 
                return System.Convert.ToChar(value, culture);
            } 
            if (targetType == typeof(System.Int64?) || targetType == typeof(System.Int64))
            {
                return System.Convert.ToInt64(value, culture); 
            }
            if (targetType == typeof(System.UInt16?) || targetType == typeof(System.UInt16))
            { 
                return System.Convert.ToUInt16(value, culture); 
            }
            if (targetType == typeof(System.UInt32?) || targetType == typeof(System.UInt32)) 
            {
                return System.Convert.ToUInt32(value, culture);
            } 
            if (targetType == typeof(System.UInt64?) || targetType == typeof(System.UInt64))
            {
                return System.Convert.ToUInt64(value, culture); 
            } 
            if (targetType.IsEnum ||
                (targetType.IsGenericType && 
                 targetType.GetGenericTypeDefinition() == typeof(Nullable<>) &&
                 targetType.GetGenericArguments().Length == 1 &&
                 targetType.GetGenericArguments()[0].IsEnum)) 
            {
                Type enumType = targetType.IsEnum ? targetType : targetType.GetGenericArguments()[0];
                String strValue = value as String; 
                if (String.IsNullOrEmpty(strValue)) 
                {
                    strValue = Enum.GetName(enumType, value); 
                }
                if (!String.IsNullOrEmpty(strValue))
                { 
                    Object enumValue = Enum.Parse(enumType, strValue, true /*ignoreCase*/);
                    if (Enum.IsDefined(enumType, enumValue))
                    { 
                        return enumValue; 
                    }
                    else 
                    {
                        throw DataGridError.DataGrid.InvalidEnumValue(value == null ? String.Empty : value.ToString(), enumType.FullName, "value");
                    } 
                }
                else
                { 
                    throw DataGridError.DataGrid.InvalidEnumValue(value == null ? String.Empty : value.ToString(), enumType.FullName, "value"); 
                }
            } 
            return value;
        }
 
        private object ConvertWithFallback(object value, Type targetType, CultureInfo culture)
        {
            if (targetType == null) 
            { 
                return value;
            } 

            try
            { 
                return Convert(value, targetType, culture);
            }
            catch (OverflowException ex) 
            { 
                return OnDataConversionError(ex, targetType, culture);
            } 
            catch (FormatException ex)
            {
                return OnDataConversionError(ex, targetType, culture); 
            }
            catch (ArgumentException ex)
            { 
                return OnDataConversionError(ex, targetType, culture); 
            }
        } 

        private object OnDataConversionError(Exception ex, Type targetType, CultureInfo culture)
        { 
            // this.Row and this.FallbackValue are null when a column cannot handle its field's data type.
            OnDataConversionError(new DataGridDataErrorEventArgs(ex, this.Column, this.Row));
            return Convert(this.FallbackValue, targetType, culture); 
        } 

        private void OnDataConversionError(DataGridDataErrorEventArgs dataError) 
        {
            EventHandler<DataGridDataErrorEventArgs> handler = this.DataConversionError;
            if (handler != null) 
            {
                handler(this, dataError);
            } 
        } 

        #endregion Private Methods 
    }
}
