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
        [Description("Verify Dependency Property: (DataGridGridLinesVisibility) DataGrid.GridLinesVisibility.")]
        public void GridLinesVisibility()
        {
            Type propertyType = typeof(DataGridGridLinesVisibility);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGrid control = new DataGrid();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGrid).GetField("GridLinesVisibilityProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGrid.GridLinesVisibilityProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGrid).GetProperty("GridLinesVisibility", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGrid.GridLinesVisibility does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGrid.GridLinesVisibility not expected type 'DataGridGridLinesVisibility'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                control.GridLinesVisibility = DataGridGridLinesVisibility.All;

                Assert.AreEqual(DataGridGridLinesVisibility.All, control.GridLinesVisibility);

                control.GridLinesVisibility = DataGridGridLinesVisibility.Horizontal;

                Assert.AreEqual(DataGridGridLinesVisibility.Horizontal, control.GridLinesVisibility);

                control.GridLinesVisibility = DataGridGridLinesVisibility.Vertical;

                Assert.AreEqual(DataGridGridLinesVisibility.Vertical, control.GridLinesVisibility);

                control.GridLinesVisibility = DataGridGridLinesVisibility.None;

                Assert.AreEqual(DataGridGridLinesVisibility.None, control.GridLinesVisibility);

                control.GridLinesVisibility = DataGridGridLinesVisibility.All;

                Assert.AreEqual(DataGridGridLinesVisibility.All, control.GridLinesVisibility);
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnGridLinesVisibilityPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGrid.GridLinesVisibility to have static, non-public side-effect callback 'OnGridLinesVisibilityPropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnGridLinesVisibilityPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGrid.GridLinesVisibility NOT to have static side-effect callback 'OnGridLinesVisibilityPropertyChanged'.");
            }
        }
    }
}
