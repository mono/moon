// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Reflection;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
    public partial class DataGrid_DependencyProperties_TestClass
    {
        [TestMethod]
        [Description("Verify Dependency Property: (DataGridRowDetailsVisibilityMode) DataGrid.RowDetailsVisibilityMode.")]
        public void RowDetailsVisibilityMode()
        {
            Type propertyType = typeof(DataGridRowDetailsVisibilityMode);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGrid control = new DataGrid();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGrid).GetField("RowDetailsVisibilityModeProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGrid.RowDetailsVisibilityModeProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGrid).GetProperty("RowDetailsVisibilityMode", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGrid.RowDetailsVisibilityMode does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGrid.RowDetailsVisibilityMode not expected type 'DataGridRowDetailsVisibilityMode'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                control.RowDetailsVisibilityMode = DataGridRowDetailsVisibilityMode.Collapsed;

                Assert.AreEqual(DataGridRowDetailsVisibilityMode.Collapsed, control.RowDetailsVisibilityMode);

                control.RowDetailsVisibilityMode = DataGridRowDetailsVisibilityMode.Visible;

                Assert.AreEqual(DataGridRowDetailsVisibilityMode.Visible, control.RowDetailsVisibilityMode);

                control.RowDetailsVisibilityMode = DataGridRowDetailsVisibilityMode.VisibleWhenSelected;

                Assert.AreEqual(DataGridRowDetailsVisibilityMode.VisibleWhenSelected, control.RowDetailsVisibilityMode);

                control.RowDetailsVisibilityMode = DataGridRowDetailsVisibilityMode.Collapsed;

                Assert.AreEqual(DataGridRowDetailsVisibilityMode.Collapsed, control.RowDetailsVisibilityMode);
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnRowDetailsVisibilityModePropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGrid.RowDetailsVisibilityMode to have static, non-public side-effect callback 'OnRowDetailsVisibilityModePropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnRowDetailsVisibilityModePropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGrid.RowDetailsVisibilityMode NOT to have static side-effect callback 'OnRowDetailsVisibilityModePropertyChanged'.");
            }
        }
    }
}
