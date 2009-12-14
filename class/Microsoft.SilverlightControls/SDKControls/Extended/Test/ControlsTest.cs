// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Globalization;
using System.Windows.Controls.Primitives;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace System.Windows.Controls.Test
{
    /// <summary>
    /// This class runs the unit tests for the Controls in the System.Windows.Controls assembly
    /// </summary>
    [TestClass]
    public class ControlsTest : SilverlightControlTest
    {
        [TestMethod]
        [Description("Verify Calendar Control Functionality.")]
        [Asynchronous]
        public void CalendarTest()
        {
            _isCalendarLoaded = false;
            Calendar calendar = new Calendar();
            calendar.Loaded += new RoutedEventHandler(Calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsCalendarLoaded);
            EnqueueCallback(delegate
            {
                Assert.IsNull(calendar.SelectedDate);
                Assert.IsTrue(calendar.SelectedDates.Count == 0);
                calendar.SelectedDate = DateTime.Today;
                Assert.IsTrue(calendar.SelectedDates.Count == 1);
                calendar.DisplayMode = CalendarMode.Decade;
                calendar.DisplayMode = CalendarMode.Year;
                calendar.DisplayMode = CalendarMode.Month;
            }
            );
            EnqueueTestComplete();
        }

        [TestMethod]
        [Description("Verify DatePicker Functionality.")]
        [Asynchronous]
        public void DatePickerTest()
        {
            _isDatePickerLoaded = false;
            DatePicker datePicker = new DatePicker();
            datePicker.Loaded += new RoutedEventHandler(DatePicker_Loaded);
            TestPanel.Children.Add(datePicker);
            EnqueueConditional(IsDatePickerLoaded);
            DateTimeFormatInfo dtfi = DateTimeHelper.GetCurrentDateFormat();
            EnqueueCallback(delegate
            {
                datePicker.Text = DateTime.Today.ToString(CultureInfo.CurrentCulture);
                
            });

            EnqueueCallback(delegate
            {
              Assert.IsTrue(DateTime.Compare(datePicker.SelectedDate.Value, DateTime.Today) == 0);
            });

            EnqueueCallback(delegate
            {
                Assert.AreEqual(string.Format(CultureInfo.CurrentCulture, DateTime.Today.ToString(dtfi.ShortDatePattern, dtfi)), datePicker.Text);
                datePicker.IsDropDownOpen = true;
                datePicker.IsDropDownOpen = false;
            });
            EnqueueTestComplete();
        }

        [TestMethod]
        [Description("Verify GridSplitter Functionality.")]
        [Asynchronous]
        public void GridSplitterTest()
        {
            Grid grid = new Grid();
            grid.Height = 100.0;
            grid.Width = 100.0;
            grid.RowDefinitions.Add(new RowDefinition());
            grid.RowDefinitions.Add(new RowDefinition());
            grid.ColumnDefinitions.Add(new ColumnDefinition());
            grid.ColumnDefinitions.Add(new ColumnDefinition());

            GridSplitter splitter = new GridSplitter();
            splitter.SetValue(Grid.ColumnProperty, 1);
            splitter.SetValue(Grid.RowSpanProperty, 2);
            splitter.Width = 5.0;
            splitter.ShowsPreview = true;
            splitter.HorizontalAlignment = HorizontalAlignment.Left;

            grid.Children.Add(splitter);
            this.CreateAsyncTest(grid,
                () => splitter.DragValidator_DragStartedEvent(splitter, new DragStartedEventArgs(0.0, 0.0)),
                () => splitter.DragValidator_DragDeltaEvent(splitter, new DragDeltaEventArgs(5.0, 6.0)),
                () => splitter.DragValidator_DragCompletedEvent(splitter, new DragCompletedEventArgs(5.0, 6.0, false)),
                delegate
                {
                    Assert.IsNull(splitter._resizeData, "The stored ResizeData instance should be gone since the resize operation is over.");
                    Assert.AreEqual(55.0, grid.ColumnDefinitions[0].ActualWidth, "The first column should have had the new width committed.");
                    Assert.AreEqual(45.0, grid.ColumnDefinitions[1].ActualWidth, "The second column should have had the new width committed.");
                });
        }

        [TestMethod]
        [Description("Verify TabControl Functionality.")]
        [Asynchronous]
        public void TabControlTest()
        {
            TabControl tabControl = new TabControl();
            Assert.AreEqual(tabControl.SelectedIndex, -1);

            Button header1 = new Button();
            Button header2 = new Button();
            header1.Content = "Tab 1";
            header2.Content = "Tab 2";

            Button content1 = new Button();
            Button content2 = new Button();
            content1.Content = "Tab 1 Content";
            content2.Content = "Tab 2 Content";

            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();
            tabItem1.Header = header1;
            tabItem2.Header = header2;

            CreateAsyncTask(tabControl,
                delegate
                {
                    // verify default selected index/item
                    Assert.AreEqual(tabControl.SelectedIndex, -1);
                    Assert.IsNull(tabControl.SelectedItem);

                    // verify that adding item updates selected index/item
                    tabControl.Items.Add(tabItem1);
                    Assert.AreEqual(tabControl.SelectedIndex, 0);
                    Assert.AreEqual(tabControl.SelectedItem, tabItem1);

                    // verify selection
                    tabControl.Items.Add(tabItem2);
                    Assert.AreEqual(tabControl.SelectedIndex, 0);
                    tabControl.SelectedIndex = 1;
                    Assert.AreEqual(tabControl.SelectedIndex, 1);
                    Assert.AreEqual(tabControl.SelectedItem, tabItem2);

                    // verify TabStripPlacement
                    tabControl.TabStripPlacement = Dock.Bottom;
                    Assert.AreEqual(tabItem1.TabStripPlacement, Dock.Bottom);
                    tabControl.TabStripPlacement = Dock.Left;
                    Assert.AreEqual(tabItem1.TabStripPlacement, Dock.Left);
                    tabControl.TabStripPlacement = Dock.Right;
                    Assert.AreEqual(tabItem1.TabStripPlacement, Dock.Right);
                    tabControl.TabStripPlacement = Dock.Top;
                    Assert.AreEqual(tabItem1.TabStripPlacement, Dock.Top);
                }
            );

            EnqueueTestComplete();
        }

        #region Private

        private void Calendar_Loaded(object sender, RoutedEventArgs e)
        {
            _isCalendarLoaded = true;
        }

        private void DatePicker_Loaded(object sender, RoutedEventArgs e)
        {
            _isDatePickerLoaded = true;
        }

        private bool IsCalendarLoaded()
        {
            return _isCalendarLoaded;
        }

        private bool IsDatePickerLoaded()
        {
            return _isDatePickerLoaded;
        }

        #endregion

        #region Data

        private bool _isCalendarLoaded;
        private bool _isDatePickerLoaded;

        #endregion
    }
}
