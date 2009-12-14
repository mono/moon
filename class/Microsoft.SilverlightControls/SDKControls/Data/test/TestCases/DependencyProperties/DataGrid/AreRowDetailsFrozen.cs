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
        [Description("Verify Dependency Property: (bool) DataGrid.AreRowDetailsFrozen.")]
        public void AreRowDetailsFrozen()
        {
            Type propertyType = typeof(bool);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = false;

            DataGrid control = new DataGrid();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGrid).GetField("AreRowDetailsFrozenProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGrid.AreRowDetailsFrozenProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGrid).GetProperty("AreRowDetailsFrozen", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGrid.AreRowDetailsFrozen does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGrid.AreRowDetailsFrozen not expected type 'bool'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                control.AreRowDetailsFrozen = true;

                Assert.AreEqual(true, control.AreRowDetailsFrozen);

                control.AreRowDetailsFrozen = false;

                Assert.AreEqual(false, control.AreRowDetailsFrozen);

                control.AreRowDetailsFrozen = true;

                Assert.AreEqual(true, control.AreRowDetailsFrozen);
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnAreRowDetailsFrozenPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGrid.AreRowDetailsFrozen to have static, non-public side-effect callback 'OnAreRowDetailsFrozenPropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnAreRowDetailsFrozenPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGrid.AreRowDetailsFrozen NOT to have static side-effect callback 'OnAreRowDetailsFrozenPropertyChanged'.");
            }
        }
    }
}
