// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Reflection;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
    public partial class DataGrid_DependencyProperties_TestClass
    {
        [TestMethod]
        [Asynchronous]
        [Description("Verify Dependency Property: (bool) DataGrid.AutoGenerateColumns.")]
        public void AutoGenerateColumns()
        {
            Type propertyType = typeof(bool);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGrid control = new DataGrid();
            Assert.IsNotNull(control);
            TestPanel.Children.Add(control);
            this.EnqueueSleep(0);
            this.EnqueueCallback(delegate
            {
                // Verify Dependency Property Property member
                FieldInfo fieldInfo = typeof(DataGrid).GetField("AutoGenerateColumnsProperty", BindingFlags.Static | BindingFlags.Public);
                Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGrid.AutoGenerateColumnsProperty not expected type 'DependencyProperty'.");

                // Verify Dependency Property Property's value type
                DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

                Assert.IsNotNull(property);

                // 


                // Verify Dependency Property CLR property member
                PropertyInfo propertyInfo = typeof(DataGrid).GetProperty("AutoGenerateColumns", BindingFlags.Instance | BindingFlags.Public);
                Assert.IsNotNull(propertyInfo, "Expected CLR property DataGrid.AutoGenerateColumns does not exist.");
                Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGrid.AutoGenerateColumns not expected type 'bool'.");

                // Verify getter/setter access
                Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
                Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

                // Verify that we set what we get
                if (expectSet) // if expectSet == false, this block can be removed
                {
                    Assert.AreEqual(true, control.AutoGenerateColumns);

                    control.AutoGenerateColumns = false;

                    Assert.AreEqual(false, control.AutoGenerateColumns);

                    control.AutoGenerateColumns = true;

                    Assert.AreEqual(true, control.AutoGenerateColumns);
                }

                // Verify Dependency Property callback
                if (hasSideEffects)
                {
                    MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnAutoGenerateColumnsPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                    Assert.IsNotNull(methodInfo, "Expected DataGrid.AutoGenerateColumns to have static, non-public side-effect callback 'OnAutoGenerateColumnsPropertyChanged'.");

                    // 
                }
                else
                {
                    MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnAutoGenerateColumnsPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                    Assert.IsNull(methodInfo, "Expected DataGrid.AutoGenerateColumns NOT to have static side-effect callback 'OnAutoGenerateColumnsPropertyChanged'.");
                }
            });
            EnqueueTestComplete();
       }
    }
}
