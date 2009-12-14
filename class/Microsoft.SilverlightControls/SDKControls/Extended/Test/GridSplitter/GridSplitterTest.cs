// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace System.Windows.Controls.Extended.Test
{
    using System.Globalization;
    using System.Text;
    using System.Windows.Controls.Test;
    using System.Windows.Markup;
    using System.Windows.Media;
    using Microsoft.Silverlight.Testing;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using System.Windows.Controls.Primitives;
    using System.Windows.Automation.Provider;
    using System.Windows.Automation.Peers;
    using Mono.Moonlight.UnitTesting;
    
    [TestClass]
    public class GridSplitterTest : SilverlightControlTest
    {
        [TestMethod]
        [Description("Create a new GridSplitter instance through code")]
        public void InstantiateGridSplitterCode()
        {
            GridSplitter splitter = new GridSplitter();
            Assert.IsNotNull(splitter, "GridSplitter construction should succeed.");
        }

        [TestMethod]
        [Description("Create a new GridSplitter instance through xaml")]
        public void InstantiateGridSplitterXaml()
        {
            object splitter = XamlReader.Load(Resource.GridSplitter_DefaultXaml);
            Assert.IsInstanceOfType(splitter, typeof(GridSplitter), "Loading the Xaml should have created a GridSplitter.");
        }

        [TestMethod]
        [Description("Check that the default value for HorizontalAlignment has been overridden")]
        public void DefaultValueForHorizontalAlignment()
        {
            GridSplitter splitter = XamlReader.Load(Resource.GridSplitter_DefaultXaml) as GridSplitter;
            Assert.AreEqual(HorizontalAlignment.Right, splitter.HorizontalAlignment, "HorizontalAlignment should be 'Right' by default.");
        }

        [TestMethod]
        [Description("Set the Background property through code")]
        public void BackgroundSetThroughCode()
        {
            GridSplitter splitter = new GridSplitter();
            Brush b = new SolidColorBrush(Colors.Black);
            splitter.Background = b;
            Assert.AreEqual(b, splitter.GetValue(GridSplitter.BackgroundProperty), "Setting Background property should set backing dependency property.");
        }

        [TestMethod]
        [Description("Set the Background property through xaml")]
        public void BackgroundSetThroughXaml()
        {
            GridSplitter splitter = XamlReader.Load(string.Format(CultureInfo.InvariantCulture, Resource.GridSplitter_CustomXaml, "Background=\"Orange\"", "")) as GridSplitter;
            Assert.AreEqual(Colors.Orange, ((SolidColorBrush)splitter.Background).Color, "Setting Background property through Xaml should persist.");
        }

        [TestMethod]
        [Description("Get the Background property through code")]
        public void BackgroundGetThroughCode()
        {
            GridSplitter splitter = new GridSplitter();
            Brush b = new SolidColorBrush(Colors.Black);
            splitter.SetValue(GridSplitter.BackgroundProperty, b);
            Assert.AreEqual(b, splitter.Background, "Getting Background property should pull from the backing dependency property.");
        }

        [TestMethod]
        [Description("Set the IsEnabled property through code")]
        public void IsEnabledSetThroughCode()
        {
            GridSplitter splitter = new GridSplitter();
            splitter.IsEnabled = false;
            Assert.AreEqual(false, splitter.GetValue(GridSplitter.IsEnabledProperty), "Setting IsEnabled property should set backing dependency property.");
        }

        [TestMethod]
        [Description("Set the IsEnabled property through xaml")]
        public void IsEnabledSetThroughXaml()
        {
            GridSplitter splitter = XamlReader.Load(string.Format(CultureInfo.InvariantCulture, Resource.GridSplitter_CustomXaml, "IsEnabled=\"False\"", "")) as GridSplitter;
            Assert.AreEqual(false, splitter.IsEnabled, "Setting IsEnabled property through Xaml should persist.");
        }

        [TestMethod]
        [Description("Get the IsEnabled property through code")]
        public void IsEnabledGetThroughCode()
        {
            GridSplitter splitter = new GridSplitter();
            splitter.SetValue(GridSplitter.IsEnabledProperty, false);
            Assert.AreEqual(false, splitter.IsEnabled, "Getting IsEnabled property should pull from the backing dependency property.");
        }

        [TestMethod]
        [Description("Set the ShowsPreview property through code")]
        public void ShowsPreviewSetThroughCode()
        {
            GridSplitter splitter = new GridSplitter();
            splitter.ShowsPreview = false;
            Assert.AreEqual(false, splitter.GetValue(GridSplitter.ShowsPreviewProperty), "Setting ShowsPreview property should set backing dependency property.");
        }

        [TestMethod]
        [Description("Set the ShowsPreview property through xaml")]
        public void ShowsPreviewSetThroughXaml()
        {
            GridSplitter splitter = XamlReader.Load(string.Format(CultureInfo.InvariantCulture, Resource.GridSplitter_CustomXaml, "ShowsPreview=\"False\"", "")) as GridSplitter;
            Assert.AreEqual(false, splitter.ShowsPreview, "Setting ShowsPreview property through Xaml should persist.");
        }

        [TestMethod]
        [Description("Get the ShowsPreview property through code")]
        public void ShowsPreviewGetThroughCode()
        {
            GridSplitter splitter = new GridSplitter();
            splitter.SetValue(GridSplitter.ShowsPreviewProperty, false);
            Assert.AreEqual(false, splitter.ShowsPreview, "Getting ShowsPreview property should pull from the backing dependency property.");
        }

        [TestMethod]
        [Description("Set the PreviewStyle property through code")]
        public void PreviewStyleSetThroughCode()
        {
            GridSplitter splitter = new GridSplitter();
            Style s = new Style(typeof(PreviewControl));
            splitter.PreviewStyle = s;
            Assert.AreEqual(s, splitter.GetValue(GridSplitter.PreviewStyleProperty), "Setting PreviewStyle property should set backing dependency property.");
        }

        [TestMethod]
        [Description("Set the PreviewStyle property through xaml")]
        public void PreviewStyleSetThroughXaml()
        {
            StringBuilder styleInsertionXaml = new StringBuilder();
            styleInsertionXaml.AppendLine("<GridSplitter.PreviewStyle>");
            styleInsertionXaml.Append(Resource.GridSplitter_PreviewStyle);
            styleInsertionXaml.Append("</GridSplitter.PreviewStyle>");
            GridSplitter splitter = XamlReader.Load(string.Format(CultureInfo.InvariantCulture, Resource.GridSplitter_CustomXaml, "", styleInsertionXaml.ToString())) as GridSplitter;
            bool pass = false;
            foreach (Setter s in splitter.PreviewStyle.Setters)
            {
                // 
                pass = ((string)s.Value == "TestStyle");
                if (pass) break;
            }
            Assert.IsTrue(pass, "Setting PreviewStyle in xaml should persist.");
        }

        [TestMethod]
        [Description("Get the PreviewStyle property through code")]
        public void PreviewStyleGetThroughCode()
        {
            GridSplitter splitter = new GridSplitter();
            Style s = new Style(typeof(PreviewControl));
            splitter.SetValue(GridSplitter.PreviewStyleProperty, s);
            Assert.AreEqual(s, splitter.PreviewStyle, "Getting PreviewStyle property should pull from the backing dependency property.");
        }

        [TestMethod]
        [Description("Start a column resize operation and ensure the resize data is set up right.")]
        [Asynchronous]
        public void MouseStartResizeColumns()
        {
            Grid g = CreateGrid(2, 2, 100.0, 100.0);
            GridSplitter splitter = CreateGridSplitterThroughCode(1, 2, 5.0, HorizontalAlignment.Left, null, false);
            g.Children.Add(splitter);
            this.CreateAsyncTest(g,
            () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
            delegate
            {

                Assert.IsNotNull(splitter._resizeData.Definition1.AsColumnDefinition, "Definition1 should be a column.");
                Assert.AreEqual(g.ColumnDefinitions[0], splitter._resizeData.Definition1.AsColumnDefinition, "Definition1 should be the first column.");
                Assert.AreEqual(0, splitter._resizeData.Definition1Index, "Definition1 should be the first column.");
                Assert.IsNotNull(splitter._resizeData.Definition2.AsColumnDefinition, "Definition2 should be a column.");
                Assert.AreEqual(g.ColumnDefinitions[1], splitter._resizeData.Definition2.AsColumnDefinition, "Definition1 should be the second column.");
                Assert.AreEqual(1, splitter._resizeData.Definition2Index, "Definition2 should be the second column.");
                Assert.AreEqual(g, splitter._resizeData.Grid, "Grid should be the parent grid.");
                Assert.AreEqual(0.0, splitter._resizeData.MaxChange, "MaxChange should be zero since ShowsPreview is false");
                Assert.AreEqual(0.0, splitter._resizeData.MinChange, "MinChange should be zero since ShowsPreivew is false.");
                Assert.AreEqual(new GridLength(1.0, GridUnitType.Star), splitter._resizeData.OriginalDefinition1Length, "OriginalDefinition1Length should be the original default width.");
                Assert.AreEqual(new GridLength(1.0, GridUnitType.Star), splitter._resizeData.OriginalDefinition2Length, "OriginalDefinition2Length should be the original default width.");
                Assert.IsNull(splitter._resizeData.PreviewControl, "No PreviewControl should be set.");
                Assert.AreEqual(GridSplitter.GridResizeBehavior.PreviousAndCurrent, splitter._resizeData.ResizeBehavior, "ResizeBehavior should be previous and current.");
                Assert.AreEqual(GridSplitter.GridResizeDirection.Columns, splitter._resizeData.ResizeDirection, "ResizeDirection should be Columns.");
                Assert.IsFalse(splitter._resizeData.ShowsPreview, "ShowsPreview should be false.");
                Assert.AreEqual(GridSplitter.SplitBehavior.Split, splitter._resizeData.SplitBehavior, "SplitBehavior should be Split since both definitions have * grid lengths.");
                Assert.AreEqual(1, splitter._resizeData.SplitterIndex, "Splitter should be in the second column.");
                Assert.AreEqual(5.0, splitter._resizeData.SplitterLength, "SplitterLength should be the width of the splitter.");
            });
        }

        [TestMethod]
        [Description("Continue a column resize operation and ensure the definitions were updated.")]
        [Asynchronous]
        public void MouseResizeColumns()
        {
            Grid g = CreateGrid(2, 2, 100.0, 100.0);
            GridSplitter splitter = CreateGridSplitterThroughCode(1, 2, 5.0, HorizontalAlignment.Left, null, false);
            g.Children.Add(splitter);
            this.CreateAsyncTest(g,
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                () => splitter.DragValidator_DragDeltaEvent(splitter, new DragDeltaEventArgs(5.0, 6.0)),
                delegate
                {

                    Assert.IsNotNull(splitter._resizeData.Definition1.AsColumnDefinition, "Definition1 should be a column.");
                    Assert.AreEqual(55.0, splitter._resizeData.Definition1.AsColumnDefinition.ActualWidth, "Definition1 should have increased by the drag delta amount.");
                    Assert.IsNotNull(splitter._resizeData.Definition2.AsColumnDefinition, "Definition2 should be a column.");
                    Assert.AreEqual(45.0, splitter._resizeData.Definition2.AsColumnDefinition.ActualWidth, "Definition2 should have decreased by the drag delta amount.");
                });
        }

        [TestMethod]
        [Description("Continue a column resize operation and ensure the column widths were reverted.")]
        [Asynchronous]
        public void MouseCompleteResizeColumnsCanceled()
        {
            Grid g = CreateGrid(2, 2, 100.0, 100.0);
            GridSplitter splitter = CreateGridSplitterThroughCode(1, 2, 5.0, HorizontalAlignment.Left, null, false);
            g.Children.Add(splitter);
            this.CreateAsyncTest(g,
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                () => splitter.DragValidator_DragDeltaEvent(splitter, new DragDeltaEventArgs(5.0, 6.0)),
                () => splitter.DragValidator_DragCompletedEvent(splitter, new DragCompletedEventArgs(5.0, 6.0, true)),
                delegate
                {

                    Assert.IsNull(splitter._resizeData, "The stored ResizeData instance should be gone since the resize operation is over.");
                    Assert.AreEqual(50.0, g.ColumnDefinitions[0].ActualWidth, "Definition1 should have been reset back to the original width.");
                    Assert.AreEqual(50.0, g.ColumnDefinitions[1].ActualWidth, "Definition2 should have been reset back to the original width.");
                });
        }

        [TestMethod]
        [Description("Complete a column resize operation and ensure the new column widths were committed.")]
        [Asynchronous]
        public void MouseCompleteResizeColumns()
        {
            Grid g = CreateGrid(2, 2, 100.0, 100.0);
            GridSplitter splitter = CreateGridSplitterThroughCode(1, 2, 5.0, HorizontalAlignment.Left, null, false);
            g.Children.Add(splitter);
            this.CreateAsyncTest(g,
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                () => splitter.DragValidator_DragDeltaEvent(splitter, new DragDeltaEventArgs(5.0, 6.0)),
                () => splitter.DragValidator_DragCompletedEvent(splitter, new DragCompletedEventArgs(5.0, 6.0, false)),
                delegate
                {
                    Assert.IsNull(splitter._resizeData, "The stored ResizeData instance should be gone since the resize operation is over.");
                    Assert.AreEqual(55.0, g.ColumnDefinitions[0].ActualWidth, "The first column should have had the new width committed.");
                    Assert.AreEqual(45.0, g.ColumnDefinitions[1].ActualWidth, "The second column should have had the new width committed.");
                });
        }

        [TestMethod]
        [Description("Pass equal double values in to AreClose")]
        public void AreCloseEqualValues()
        {
            bool result = GridSplitter.DoubleUtil.AreClose(1.0, 1.0);
            Assert.IsTrue(result, "Equal values should return true.");
        }

        [TestMethod]
        [Description("Pass unequal double values in to AreClose")]
        public void AreCloseUnequalValues()
        {
            bool result = GridSplitter.DoubleUtil.AreClose(1.1, 1.0);
            Assert.IsFalse(result, "Unequal values should return false.");
        }

        [TestMethod]
        [Description("Pass max double values in to AreClose")]
        public void AreCloseMaxMax()
        {
            bool result = GridSplitter.DoubleUtil.AreClose(double.MaxValue, double.MaxValue);
            Assert.IsTrue(result, "Equal max values should return true.");
        }

        [TestMethod]
        [Description("Pass min double values in to AreClose")]
        public void AreCloseMinMin()
        {
            bool result = GridSplitter.DoubleUtil.AreClose(double.MinValue, double.MinValue);
            Assert.IsTrue(result, "Equal min values should return true.");
        }

        [TestMethod]
        [Description("Pass one min, one max double values in to AreClose")]
        public void AreCloseMinMax()
        {
            bool result = GridSplitter.DoubleUtil.AreClose(double.MinValue, double.MaxValue);
            Assert.IsFalse(result, "Unequal min/max values should return false.");
        }

        [TestMethod]
        [Description("Pass one max, one min double values in to AreClose")]
        public void AreCloseMaxMin()
        {
            bool result = GridSplitter.DoubleUtil.AreClose(double.MaxValue, double.MinValue);
            Assert.IsFalse(result, "Unequal max/min values should return false.");
        }

        [TestMethod]
        [Description("Pass zero and double.Epsilon values in to AreClose")]
        public void AreCloseZeroEpsilon()
        {
            bool result = GridSplitter.DoubleUtil.AreClose(0.0, double.Epsilon);
            Assert.IsTrue(result, "Zero and epsilon values should return true.");
        }

        [TestMethod]
        [Description("Pass effectively equal values in to AreClose that mimic grid imprecision ")]
        public void AreCloseGridPrecisionTrue()
        {
            double EffectiveEpsilon = 1.192093E-07; 

            bool result = GridSplitter.DoubleUtil.AreClose(1.0 + EffectiveEpsilon, 1.0);
            Assert.IsTrue(result, "Two values differing by the effective grid epsilon should return true.");
        }

        [TestMethod]
        [Description("Pass effectively unequal double values in to AreClose that mimic grid imprecision")]
        public void AreCloseGridPrecisionFalse()
        {
            double OverEffectiveEpsilon = 1.192093E-04;
            bool result = GridSplitter.DoubleUtil.AreClose(1.0 + OverEffectiveEpsilon, 1.0);
            Assert.IsFalse(result, "Two values differing by more than the effective grid epsilon should return false.");
        }

        [TestMethod]
        [Description("Change the alignment in order to update the template orientation")]
        [Asynchronous]
        public void TemplateOrientation()
        {
            Grid g = CreateGrid(2, 2, 100.0, 100.0);
            GridSplitter splitter = CreateGridSplitterThroughCode(1, 1, 5.0, HorizontalAlignment.Left, null, false);
            g.Children.Add(splitter);
            this.CreateAsyncTest(g,
                () => splitter.HorizontalAlignment = HorizontalAlignment.Stretch,
                () => splitter.VerticalAlignment = VerticalAlignment.Bottom,
                delegate
                {
                    Assert.AreEqual(Visibility.Visible, splitter._elementHorizontalTemplateFrameworkElement.Visibility);
                    Assert.AreEqual(Visibility.Collapsed, splitter._elementVerticalTemplateFrameworkElement.Visibility);
                },
                () => splitter.HorizontalAlignment = HorizontalAlignment.Right,
                () => splitter.VerticalAlignment = VerticalAlignment.Stretch,
                delegate
                {
                    Assert.AreEqual(Visibility.Collapsed, splitter._elementHorizontalTemplateFrameworkElement.Visibility);
                    Assert.AreEqual(Visibility.Visible, splitter._elementVerticalTemplateFrameworkElement.Visibility);
                });
        }

        [TestMethod]
        [Description("Use a GridSplitter in a Grid that does not necessarily have its ColumnDefinitions or RowDefinitions initialized")]
        [Asynchronous]
        public void VariableNumbersOfColumnsAndRows()
        {
            // Check the cases where a vertical GridSplitter is used in a Grid with 0,1, and n columns
            // and when a horizontal GridSplitter is used in a Grid with 0,1, and n rows.
            bool result;
            Grid g = CreateGrid(2, 0, 100.0, 100.0);
            GridSplitter splitter = CreateGridSplitterThroughCode(1, 0, null, null, null, null, VerticalAlignment.Top, HorizontalAlignment.Stretch, null, true);
            g.Children.Add(splitter);
            this.CreateAsyncTest(g,
                delegate
                {
                    result = GridSplitter.DoubleUtil.AreClose(g.RowDefinitions[0].ActualHeight, 50.0);
                    Assert.IsTrue(result, "ActualHeight should be same as the desired height.");
                },
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                () => splitter.DragValidator_DragDeltaEvent(splitter, new DragDeltaEventArgs(0.0, 10.0)),
                () => splitter.DragValidator_DragCompletedEvent(splitter, new DragCompletedEventArgs(0.0, 0.0, false)),
                delegate
                {
                    result = GridSplitter.DoubleUtil.AreClose(g.RowDefinitions[0].ActualHeight, 60.0);
                    Assert.IsTrue(result, "ActualHeight should be same as the desired height.");
                },
                () => g.ColumnDefinitions.Add(new ColumnDefinition()),
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                () => splitter.DragValidator_DragDeltaEvent(splitter, new DragDeltaEventArgs(0.0, -15.0)),
                () => splitter.DragValidator_DragCompletedEvent(splitter, new DragCompletedEventArgs(0.0, 0.0, false)),
                delegate
                {
                    result = GridSplitter.DoubleUtil.AreClose(g.RowDefinitions[0].ActualHeight, 45.0);
                    Assert.IsTrue(result, "ActualHeight should be same as the desired height.");
                },
                () => g.ColumnDefinitions.Add(new ColumnDefinition()),
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                () => splitter.DragValidator_DragDeltaEvent(splitter, new DragDeltaEventArgs(0.0, 25.0)),
                () => splitter.DragValidator_DragCompletedEvent(splitter, new DragCompletedEventArgs(0.0, 0.0, false)),
                delegate
                {
                    result = GridSplitter.DoubleUtil.AreClose(g.RowDefinitions[0].ActualHeight, 70.0);
                    Assert.IsTrue(result, "ActualHeight should be same as the desired height.");
                },
                () => g.RowDefinitions.Clear(),
                () => splitter.VerticalAlignment = VerticalAlignment.Stretch,
                () => splitter.HorizontalAlignment = HorizontalAlignment.Left,
                () => splitter.SetValue(Grid.RowProperty, 0),
                () => splitter.SetValue(Grid.ColumnProperty, 1),
                delegate
                {
                    result = GridSplitter.DoubleUtil.AreClose(g.ColumnDefinitions[0].ActualWidth, 50.0);
                    Assert.IsTrue(result, "ActualWidth should be same as the desired width.");
                },
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                () => splitter.DragValidator_DragDeltaEvent(splitter, new DragDeltaEventArgs(10.0, 0.0)),
                () => splitter.DragValidator_DragCompletedEvent(splitter, new DragCompletedEventArgs(0.0, 0.0, false)),
                delegate
                {
                    result = GridSplitter.DoubleUtil.AreClose(g.ColumnDefinitions[0].ActualWidth, 60.0);
                    Assert.IsTrue(result, "ActualWidth should be same as the desired width.");
                },
                () => g.RowDefinitions.Add(new RowDefinition()),
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                () => splitter.DragValidator_DragDeltaEvent(splitter, new DragDeltaEventArgs(-15.0, 0.0)),
                () => splitter.DragValidator_DragCompletedEvent(splitter, new DragCompletedEventArgs(0.0, 0.0, false)),
                delegate
                {
                    result = GridSplitter.DoubleUtil.AreClose(g.ColumnDefinitions[0].ActualWidth, 45.0);
                    Assert.IsTrue(result, "ActualWidth should be same as the desired width.");
                },
                () => g.RowDefinitions.Add(new RowDefinition()),
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                () => splitter.DragValidator_DragDeltaEvent(splitter, new DragDeltaEventArgs(25.0, 0.0)),
                () => splitter.DragValidator_DragCompletedEvent(splitter, new DragCompletedEventArgs(0.0, 0.0, false)),
                delegate
                {
                    result = GridSplitter.DoubleUtil.AreClose(g.ColumnDefinitions[0].ActualWidth, 70.0);
                    Assert.IsTrue(result, "ActualWidth should be same as the desired width.");
                });
        }

        [TestMethod]
        [Description("Use GridSplitter's AutomationPeer")]
        [Asynchronous]
        public void UseAutomationPeer()
        {
            Grid g = CreateGrid(2, 2, 100.0, 100.0);
            GridSplitter splitter = CreateGridSplitterThroughCode(1, 1, 5.0, HorizontalAlignment.Left, null, false);
            GridSplitterAutomationPeer peer = ((GridSplitterAutomationPeer)GridSplitterAutomationPeer.CreatePeerForElement(splitter));
            ITransformProvider transformer = (ITransformProvider)peer.GetPattern(PatternInterface.Transform);
            g.Children.Add(splitter);
            this.CreateAsyncTest(g,
                delegate
                {
                    Assert.AreEqual(50.0, g.ColumnDefinitions[0].ActualWidth, "Definition1's default width.");
                    Assert.AreEqual(50.0, g.ColumnDefinitions[1].ActualWidth, "Definition2's default width.");
                },
                () => transformer.Move(10,0),
                delegate
                {
                    Assert.AreEqual(AutomationControlType.Thumb, peer.GetAutomationControlType(), "GridSplitter should be a thumb control type.");
                    Assert.AreEqual(splitter.GetType().Name, peer.GetClassName(), "AutomationPeer's ClassName should be 'GridSplitter'");
                    Assert.AreEqual(transformer.CanMove, true, "GridSplitter can be moved");
                    Assert.AreEqual(transformer.CanResize, false, "GridSplitter cannot be resized");
                    Assert.AreEqual(transformer.CanRotate, false, "GridSplitter cannot be rotated");
                    Assert.AreEqual(60.0, g.ColumnDefinitions[0].ActualWidth, "Definition1's width should have been changed by the peer's move.");
                    Assert.AreEqual(40.0, g.ColumnDefinitions[1].ActualWidth, "Definition2's width should have been changed by the peer's move.");
                });
        }

        #region Helper Methods

        internal static Grid CreateGrid(int rows, int columns, double height, double width)
        {
            Grid g = new Grid();
            g.Height = height;
            g.Width = width;

            for (int r = 0; r < rows; r++)
            {
                g.RowDefinitions.Add(new RowDefinition());
            }
            for (int c = 0; c < columns; c++)
            {
                g.ColumnDefinitions.Add(new ColumnDefinition());
            }
            return g;
        }

        internal static GridSplitter CreateGridSplitterThroughCode(int column, int rowSpan, double width, HorizontalAlignment hAlign, Brush background, bool? showsPreview)
        {
            return CreateGridSplitterThroughCode(null, column, rowSpan, null, null, width, null, hAlign, background, showsPreview);
        }

        internal static GridSplitter CreateGridSplitterThroughCode(int? row, int? column, int? rowSpan, int? columnSpan, double? height, double? width, VerticalAlignment? vAlign, HorizontalAlignment? hAlign, Brush background, bool? showsPreview)
        {
            GridSplitter splitter = new GridSplitter();
            if (row != null)
            {
                splitter.SetValue(Grid.RowProperty, (int)row);
            }
            if (column != null)
            {
                splitter.SetValue(Grid.ColumnProperty, (int)column);
            }
            if (rowSpan != null)
            {
                splitter.SetValue(Grid.RowSpanProperty, (int)rowSpan);
            }
            if (columnSpan != null)
            {
                splitter.SetValue(Grid.ColumnSpanProperty, (int)columnSpan);
            }
            if (height != null)
            {
                splitter.Height = (double)height;
            }
            if (width != null)
            {
                splitter.Width = (double)width;
            }
            if (background != null)
            {
                splitter.Background = background;
            }
            if (showsPreview != null)
            {
                splitter.ShowsPreview = (bool)showsPreview;
            }
            if (hAlign != null)
            {
                splitter.HorizontalAlignment = (HorizontalAlignment)hAlign;
            }
            if (vAlign != null)
            {
                splitter.VerticalAlignment = (VerticalAlignment)vAlign;
            }

            return splitter;
        }

        #endregion
    }
}
