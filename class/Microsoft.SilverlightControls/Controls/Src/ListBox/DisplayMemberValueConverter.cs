// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Windows.Data;
 
#if !WPF 
namespace System.Windows.Controls
{ 
    /// <summary>
    /// IValueConverter that converts any object to its ToString equivalent
    /// </summary> 
    internal class DisplayMemberValueConverter : IValueConverter
    {
        /// <summary> 
        /// Converts a value. 
        /// </summary>
        /// <param name="value">The value produced by the binding source.</param> 
        /// <param name="targetType">The type of the binding target property.</param>
        /// <param name="parameter">The converter parameter to use.</param>
        /// <param name="culture">The culture to use in the converter.</param> 
        /// <returns>A converted value.</returns>
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        { 
            return (null != value) ? value.ToString() : ""; 
        }
 
        /// <summary>
        /// Converts a value.
        /// </summary> 
        /// <param name="value">The value that is produced by the binding target.</param>
        /// <param name="targetType">The type to convert to.</param>
        /// <param name="parameter">The converter parameter to use.</param> 
        /// <param name="culture">The culture to use in the converter.</param> 
        /// <returns>A converted value.</returns>
        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture) 
        {
            throw new NotImplementedException();
        } 
    }
}
#endif 
