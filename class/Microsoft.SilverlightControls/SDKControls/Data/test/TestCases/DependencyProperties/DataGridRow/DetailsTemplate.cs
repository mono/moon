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
        [Description("Verify Dependency Property: (DataTemplate) DataGridRow.DetailsTemplate.")]
        public void DetailsTemplate()
        {
            Type propertyType = typeof(DataTemplate);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGridRow control = new DataGridRow();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGridRow).GetField("DetailsTemplateProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGridRow.DetailsTemplateProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGridRow).GetProperty("DetailsTemplate", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGridRow.DetailsTemplate does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGridRow.DetailsTemplate not expected type 'DataTemplate'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                DataTemplate template = new DataTemplate();

                control.DetailsTemplate = template;

                Assert.AreEqual(template, control.DetailsTemplate);
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGridRow).GetMethod("OnDetailsTemplatePropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGridRow.DetailsTemplate to have static, non-public side-effect callback 'OnDetailsTemplatePropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGridRow).GetMethod("OnDetailsTemplatePropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGridRow.DetailsTemplate NOT to have static side-effect callback 'OnDetailsTemplatePropertyChanged'.");
            }
        }
    }
}
