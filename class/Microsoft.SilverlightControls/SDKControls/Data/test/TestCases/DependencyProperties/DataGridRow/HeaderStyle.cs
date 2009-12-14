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
        [Description("Verify Dependency Property: (Style) DataGridRow.HeaderStyle.")]
        public void HeaderStyle()
        {
            Type propertyType = typeof(Style);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGridRow control = new DataGridRow();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGridRow).GetField("HeaderStyleProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGridRow.HeaderStyleProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGridRow).GetProperty("HeaderStyle", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGridRow.HeaderStyle does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGridRow.HeaderStyle not expected type 'Style'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                Style style1 = new Style();
                //Style style2 = new Style();

                control.HeaderStyle = style1;

                Assert.AreEqual(style1, control.HeaderStyle);

                // 

                //control.HeaderStyle = style2;

                //Assert.AreEqual(style2, control.HeaderStyle);
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGridRow).GetMethod("OnHeaderStylePropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGridRow.HeaderStyle to have static, non-public side-effect callback 'OnHeaderStylePropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGridRow).GetMethod("OnHeaderStylePropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGridRow.HeaderStyle NOT to have static side-effect callback 'OnHeaderStylePropertyChanged'.");
            }
        }
    }
}
