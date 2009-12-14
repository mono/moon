// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace System.Windows.Controls.Extended.Test
{
    using System.Windows.Controls.Primitives;
    using System.Windows.Controls.Test;
    using System.Windows.Markup;
    using System.Windows.Media;
    using Microsoft.Silverlight.Testing;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Mono.Moonlight.UnitTesting;

    [TestClass]
    public class PreviewControlTest : SilverlightControlTest
    {
        private const int StyleApplicationTimeout = 2000;

        [TestMethod]
        [Description("Create a new PreviewControl instance through code")]
        public void InstantiatePreviewControl()
        {
            PreviewControl preview = new PreviewControl();
            Assert.IsNotNull(preview, "PreviewControl construction should succeed.");
        }

        [TestMethod]
        [Description("Change the OffsetX property")]
        public void ChangeXOffset()
        {
            PreviewControl preview = new PreviewControl();
            preview.OffsetX = 2.0;

            Assert.AreEqual(2.0, (double)preview.GetValue(Canvas.LeftProperty), "OffsetX should have set the canvas's Left property.");
        }

        [TestMethod]
        [Description("Change the OffsetY property")]
        public void ChangeYOffset()
        {
            PreviewControl preview = new PreviewControl();
            preview.OffsetY = 3.0;

            Assert.AreEqual(3.0, (double)preview.GetValue(Canvas.TopProperty), "OffsetY should have set the canvas's Top property.");
        }

        [TestMethod]
        [Description("Apply a style to the preview")]
        [Asynchronous]
        public void ApplyPreviewStyle()
        {
            Grid g = GridSplitterTest.CreateGrid(2, 2, 100, 100);
            GridSplitter splitter = GridSplitterTest.CreateGridSplitterThroughCode(1, 1, 1, null, null, 10.0, null, HorizontalAlignment.Left, new SolidColorBrush(Colors.Cyan), true);
            splitter.PreviewStyle = XamlReader.Load(Resource.GridSplitter_PreviewStyle) as Style;
            g.Children.Add(splitter);

            TestPanel.Children.Add(g);

            // Wait until the splitter style got applied
            DateTime timeout = DateTime.Now.AddMilliseconds(StyleApplicationTimeout);
            EnqueueConditional(() => (splitter.Template != null) || (DateTime.Now > timeout));
            
            EnqueueCallback(() => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)));

            // Give time for the preview style to get applied (if required)
            timeout = DateTime.Now.AddMilliseconds(StyleApplicationTimeout);
            EnqueueConditional(() => (splitter._resizeData.PreviewControl.Template != null) || (DateTime.Now > timeout));

            EnqueueCallback(delegate
            {
                Assert.IsNotNull(splitter._resizeData.PreviewControl.Template, "The template should have been set if the style was applied.");
            });

            EnqueueCallback(() => TestPanel.Children.Remove(g));
            EnqueueTestComplete();
        }

        [TestMethod]
        [Description("Bind to a valid GridSplitter")]
        [Asynchronous]
        [MoonlightBug ()]
        public void BindToValidGridSplitter()
        {
            Grid g = GridSplitterTest.CreateGrid(2, 2, 100, 100);
            GridSplitter splitter = GridSplitterTest.CreateGridSplitterThroughCode(1, 1, 1, null, null, 10.0, null, HorizontalAlignment.Left, new SolidColorBrush(Colors.Cyan), true);
            g.Children.Add(splitter);
            PreviewControl preview = new PreviewControl();
            this.CreateAsyncTest(g,
                () => preview.Bind(splitter),
                delegate
                {
                    Assert.AreEqual(50.0, preview.Height, "Height should be half the height of the grid");
                    Assert.AreEqual(10.0, preview.Width, "Width should be the width of the GridSplitter");
                    Assert.AreEqual(50.0, (double)preview.GetValue(Canvas.LeftProperty), "Left position should be on left edge of the second column");
                    Assert.AreEqual(50.0, (double)preview.GetValue(Canvas.TopProperty), "Top position should be on top edge of the second row");
                });
        }

        [TestMethod]
        [Description("Switch between vertical and horizontal templates")]
        [Asynchronous]
        public void TemplateOrientation()
        {
            Grid g = GridSplitterTest.CreateGrid(2, 2, 100.0, 100.0);
            GridSplitter splitter = GridSplitterTest.CreateGridSplitterThroughCode(1, 1, 1, 1, 10.0, 10.0, VerticalAlignment.Stretch, HorizontalAlignment.Left, new SolidColorBrush(Colors.Green), true);
            g.Children.Add(splitter);

            this.CreateAsyncTest(g,
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                delegate
                {
                    Assert.AreEqual(Visibility.Collapsed, splitter._resizeData.PreviewControl._elementHorizontalTemplateFrameworkElement.Visibility);
                    Assert.AreEqual(Visibility.Visible, splitter._resizeData.PreviewControl._elementVerticalTemplateFrameworkElement.Visibility);
                },
                () => splitter.DragValidator_DragDeltaEvent(splitter, new DragDeltaEventArgs(5.0, 6.0)),
                () => splitter.DragValidator_DragCompletedEvent(splitter, new DragCompletedEventArgs(5.0, 6.0, false)),
                () => splitter.HorizontalAlignment = HorizontalAlignment.Stretch,
                () => splitter.VerticalAlignment = VerticalAlignment.Top,
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                delegate
                {
                    Assert.AreEqual(Visibility.Visible, splitter._resizeData.PreviewControl._elementHorizontalTemplateFrameworkElement.Visibility);
                    Assert.AreEqual(Visibility.Collapsed, splitter._resizeData.PreviewControl._elementVerticalTemplateFrameworkElement.Visibility);
                });
        }
    }
}
