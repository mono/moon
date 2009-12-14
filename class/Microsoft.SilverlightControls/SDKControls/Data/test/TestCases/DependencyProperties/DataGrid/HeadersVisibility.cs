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
        [Description("Verify Dependency Property: (DataGridHeaders) DataGrid.HeadersVisibility.")]
        public void HeadersVisibility()
        {
            Type propertyType = typeof(DataGridHeadersVisibility);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGrid control = new DataGrid();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGrid).GetField("HeadersVisibilityProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGrid.HeadersVisibilityProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGrid).GetProperty("HeadersVisibility", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGrid.HeadersVisibility does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGrid.HeadersVisibility not expected type 'DataGridHeaders'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                control.HeadersVisibility = DataGridHeadersVisibility.All;

                Assert.AreEqual(DataGridHeadersVisibility.All, control.HeadersVisibility);

                control.HeadersVisibility = DataGridHeadersVisibility.Column;

                Assert.AreEqual(DataGridHeadersVisibility.Column, control.HeadersVisibility);

                control.HeadersVisibility = DataGridHeadersVisibility.None;

                Assert.AreEqual(DataGridHeadersVisibility.None, control.HeadersVisibility);

                control.HeadersVisibility = DataGridHeadersVisibility.Row;

                Assert.AreEqual(DataGridHeadersVisibility.Row, control.HeadersVisibility);

                control.HeadersVisibility = DataGridHeadersVisibility.All;

                Assert.AreEqual(DataGridHeadersVisibility.All, control.HeadersVisibility);
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnHeadersVisibilityPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGrid.HeadersVisibility to have static, non-public side-effect callback 'OnHeadersVisibilityPropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnHeadersVisibilityPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGrid.HeadersVisibility NOT to have static side-effect callback 'OnHeadersVisibilityPropertyChanged'.");
            }
        }
    }
}
