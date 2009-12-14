// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Collections.ObjectModel;
using System.Reflection;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
    public partial class DataGrid_DependencyProperties_TestClass
    {
        [TestMethod]
        [Description("Verify Dependency Property: (IEnumerable) DataGrid.ItemsSource.")]
        public void ItemsSource()
        {
            Type propertyType = typeof(IEnumerable);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGrid control = new DataGrid();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGrid).GetField("ItemsSourceProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGrid.ItemsSourceProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGrid).GetProperty("ItemsSource", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGrid.ItemsSource does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGrid.ItemsSource not expected type 'IEnumerable'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                ObservableCollection<int> data1 = new ObservableCollection<int> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
                ObservableCollection<string> data2 = new ObservableCollection<string> { "zero", "one", "two" };
                ObservableCollection<double> data3 = new ObservableCollection<double> { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 };

                control.ItemsSource = data1;

                Assert.AreEqual(data1, control.ItemsSource);

                control.ItemsSource = data2;

                Assert.AreEqual(data2, control.ItemsSource);

                control.ItemsSource = data3;

                Assert.AreEqual(data3, control.ItemsSource);
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnItemsSourcePropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGrid.ItemsSource to have static, non-public side-effect callback 'OnItemsSourcePropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnItemsSourcePropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGrid.ItemsSource NOT to have static side-effect callback 'OnItemsSourcePropertyChanged'.");
            }
        }
    }
}
