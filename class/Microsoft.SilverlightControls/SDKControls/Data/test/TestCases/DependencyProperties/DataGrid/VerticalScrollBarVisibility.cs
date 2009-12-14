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
        [Description("Verify Dependency Property: (ScrollBarVisibility) DataGrid.VerticalScrollBarVisibility.")]
        public void VerticalScrollBarVisibility()
        {
            Type propertyType = typeof(ScrollBarVisibility);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGrid control = new DataGrid();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGrid).GetField("VerticalScrollBarVisibilityProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGrid.VerticalScrollBarVisibilityProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGrid).GetProperty("VerticalScrollBarVisibility", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGrid.VerticalScrollBarVisibility does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGrid.VerticalScrollBarVisibility not expected type 'ScrollBarVisibility'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                control.VerticalScrollBarVisibility = ScrollBarVisibility.Auto;
                Assert.AreEqual(ScrollBarVisibility.Auto, control.VerticalScrollBarVisibility);

                control.VerticalScrollBarVisibility = ScrollBarVisibility.Disabled;
                Assert.AreEqual(ScrollBarVisibility.Disabled, control.VerticalScrollBarVisibility);

                control.VerticalScrollBarVisibility = ScrollBarVisibility.Hidden;
                Assert.AreEqual(ScrollBarVisibility.Hidden, control.VerticalScrollBarVisibility);

                control.VerticalScrollBarVisibility = ScrollBarVisibility.Visible;
                Assert.AreEqual(ScrollBarVisibility.Visible, control.VerticalScrollBarVisibility);
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnVerticalScrollBarVisibilityPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGrid.VerticalScrollBarVisibility to have static, non-public side-effect callback 'OnVerticalScrollBarVisibilityPropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnVerticalScrollBarVisibilityPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGrid.VerticalScrollBarVisibility NOT to have static side-effect callback 'OnVerticalScrollBarVisibilityPropertyChanged'.");
            }
        }
    }
}
