// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Reflection;
using System.Windows.Controls.Data.Test.DataClasses;
using System.Windows.Controls.Test;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
    public partial class DataGrid_DependencyProperties_TestClass
    {
        [TestMethod]
        [Asynchronous]
        [Description("Verify Dependency Property: (object) DataGrid.SelectedIndex.")]
        public void SelectedIndex()
        {
            Type propertyType = typeof(int);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGrid control = new DataGrid();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGrid).GetField("SelectedIndexProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGrid.SelectedIndexProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGrid).GetProperty("SelectedIndex", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGrid.SelectedIndex does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGrid.SelectedIndex not expected type 'object'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                CustomerList list = new CustomerList();

                Assert.IsTrue(list.Count > 3, "CustomerList has too few items for this test.");

                control.ItemsSource = list;

                // 







                TestPanel.Children.Add(control);
                this.EnqueueYieldThread();

                EnqueueCallback(delegate
                {
                    Assert.AreEqual(list.Count, control.RowCount);

                    control.SelectedIndex = 0;

                    Assert.AreEqual(0, control.SelectedIndex);
                    Assert.AreNotEqual(1, control.SelectedIndex);

                    Assert.AreEqual(list[0], control.SelectedItem);
                    Assert.AreNotEqual(list[1], control.SelectedItem);

                    control.SelectedIndex = 3;

                    Assert.AreEqual(3, control.SelectedIndex);
                    Assert.AreEqual(list[3], control.SelectedItem);

                    control.SelectedIndex = 2;

                    Assert.AreEqual(2, control.SelectedIndex);
                    Assert.AreEqual(list[2], control.SelectedItem);

                    control.SelectedItem = list[3];

                    Assert.AreEqual(list[3], control.SelectedItem);
                    Assert.AreEqual(3, control.SelectedIndex);

                    control.SelectedIndex = -2;

                    Assert.IsNull(control.SelectedItem);
                    Assert.AreEqual(-1, control.SelectedIndex);

                    control.SelectedIndex = list.Count - 1;

                    Assert.AreEqual(list.Count - 1, control.SelectedIndex);
                    Assert.AreEqual(list[list.Count - 1], control.SelectedItem);

                    control.SelectedIndex = list.Count;

                    Assert.IsNull(control.SelectedItem);
                    Assert.AreEqual(-1, control.SelectedIndex);
                });
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnSelectedIndexPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGrid.SelectedIndex to have static, non-public side-effect callback 'OnSelectedIndexPropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnSelectedIndexPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGrid.SelectedIndex NOT to have static side-effect callback 'OnSelectedIndexPropertyChanged'.");
            }
            EnqueueTestComplete();
        }
    }
}
