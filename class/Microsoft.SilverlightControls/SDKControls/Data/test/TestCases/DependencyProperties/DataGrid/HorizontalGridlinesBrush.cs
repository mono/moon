// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Reflection;
using System.Windows.Media;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
    public partial class DataGrid_DependencyProperties_TestClass
    {
        [TestMethod]
        [Description("Verify Dependency Property: (Brush) DataGrid.HorizontalGridLinesBrush.")]
        public void HorizontalGridLinesBrush()
        {
            Type propertyType = typeof(Brush);
            bool expectGet = true;
            bool expectSet = true;
            bool hasSideEffects = true;

            DataGrid control = new DataGrid();
            Assert.IsNotNull(control);

            // Verify Dependency Property Property member
            FieldInfo fieldInfo = typeof(DataGrid).GetField("HorizontalGridLinesBrushProperty", BindingFlags.Static | BindingFlags.Public);
            Assert.AreEqual(typeof(DependencyProperty), fieldInfo.FieldType, "DataGrid.HorizontalGridLinesBrushProperty not expected type 'DependencyProperty'.");

            // Verify Dependency Property Property's value type
            DependencyProperty property = fieldInfo.GetValue(null) as DependencyProperty;

            Assert.IsNotNull(property);

            // 


            // Verify Dependency Property CLR property member
            PropertyInfo propertyInfo = typeof(DataGrid).GetProperty("HorizontalGridLinesBrush", BindingFlags.Instance | BindingFlags.Public);
            Assert.IsNotNull(propertyInfo, "Expected CLR property DataGrid.HorizontalGridLinesBrush does not exist.");
            Assert.AreEqual(propertyType, propertyInfo.PropertyType, "DataGrid.HorizontalGridLinesBrush not expected type 'Brush'.");

            // Verify getter/setter access
            Assert.AreEqual(expectGet, propertyInfo.CanRead, "Unexpected value for propertyInfo.CanRead.");
            Assert.AreEqual(expectSet, propertyInfo.CanWrite, "Unexpected value for propertyInfo.CanWrite.");

            // Verify that we set what we get
            if (expectSet) // if expectSet == false, this block can be removed
            {
                Brush brush1 = new LinearGradientBrush { GradientStops = { new GradientStop { Offset = 0.3, Color = Colors.Red } } };
                Brush brush2 = new LinearGradientBrush { GradientStops = { new GradientStop { Offset = 0.6, Color = Colors.Green } } };

                control.HorizontalGridLinesBrush = brush1;

                Assert.AreEqual(brush1, control.HorizontalGridLinesBrush);

                control.HorizontalGridLinesBrush = brush2;

                Assert.AreEqual(brush2, control.HorizontalGridLinesBrush);

                control.HorizontalGridLinesBrush = brush1;

                Assert.AreEqual(brush1, control.HorizontalGridLinesBrush);
            }

            // Verify Dependency Property callback
            if (hasSideEffects)
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnHorizontalGridLinesBrushPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNotNull(methodInfo, "Expected DataGrid.HorizontalGridLinesBrush to have static, non-public side-effect callback 'OnHorizontalGridLinesBrushPropertyChanged'.");

                // 
            }
            else
            {
                MethodInfo methodInfo = typeof(DataGrid).GetMethod("OnHorizontalGridLinesBrushPropertyChanged", BindingFlags.Static | BindingFlags.NonPublic);
                Assert.IsNull(methodInfo, "Expected DataGrid.HorizontalGridLinesBrush NOT to have static side-effect callback 'OnHorizontalGridLinesBrushPropertyChanged'.");
            }
        }
    }
}
