// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Globalization;
using System.Windows.Data;

namespace System.Windows.Controls
{
    internal class DataGridValueConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (targetType != null && targetType.IsNullableType())
            {
                String strValue = value as String;
                if (strValue == String.Empty)
                {
                    return null;
                }
            }
            return value;
        }
    }
}
