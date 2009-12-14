// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Reflection;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
    public partial class DataGridRow_DependencyProperties_TestClass
    {
        [TestMethod]
        [Description("Verify Dependency Property: (object) DataGridRow.Header.")]
        public void Header()
        {
            Type propertyType = typeof(object);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGridRow control = new DataGridRow();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGridRow).GetField("HeaderProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGridRow.HeaderProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGridRow).GetProperty("Header", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGridRow.Header does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGridRow.Header not expected type 'object'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                Button button1 = new Button();
                Button button2 = new Button();
                Button button3 = new Button();

                control.Header = button1;

                Assert.AreEqual(button1, control.Header);

                control.Header = button2;

                Assert.AreEqual(button2, control.Header);

                control.Header = button3;

                Assert.AreEqual(button3, control.Header);
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGridRow).GetMethod("OnHeaderPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGridRow.Header to have static, non-public side-effect callback 'OnHeaderPropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGridRow).GetMethod("OnHeaderPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGridRow.Header NOT to have static side-effect callback 'OnHeaderPropertyChanged'.");
            }
        }
    }
}
