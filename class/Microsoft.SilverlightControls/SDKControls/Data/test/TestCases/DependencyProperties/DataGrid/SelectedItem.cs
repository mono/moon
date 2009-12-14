// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Reflection;
using System.Windows.Controls.Data.Test.DataClasses;
using System.Windows.Controls.Test;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
    public partial class DataGrid_DependencyProperties_TestClass
    {
        [TestMethod]
        [Description("Verify Dependency Property: (object) DataGrid.SelectedItem.")]
        public void SelectedItem()
        {
            Type propertyType = typeof(object);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGrid control = new DataGrid();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGrid).GetField("SelectedItemProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGrid.SelectedItemProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGrid).GetProperty("SelectedItem", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGrid.SelectedItem does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGrid.SelectedItem not expected type 'object'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                CustomerList list = new CustomerList();

                Assert.IsTrue(list.Count > 3, "CustomerList has too few items for this test.");

                Common.AssertExpectedException(DataGridError.DataGrid.ItemIsNotContainedInTheItemsSource("dataItem"),
                    delegate
                    {
                        control.SelectedItems.Add(list[2]);
                    }
                );

                control.ItemsSource = list;

                // 







                control.SelectedItem = list[0];

                Assert.AreEqual(list[0], control.SelectedItem);
                Assert.AreNotEqual(list[1], control.SelectedItem);

                control.SelectedItem = list[3];

                Assert.AreEqual(list[3], control.SelectedItem);

                control.SelectedItem = list[2];

                Assert.AreEqual(list[2], control.SelectedItem);
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnSelectedItemPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGrid.SelectedItem to have static, non-public side-effect callback 'OnSelectedItemPropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnSelectedItemPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGrid.SelectedItem NOT to have static side-effect callback 'OnSelectedItemPropertyChanged'.");
            }
        }
    }
}
