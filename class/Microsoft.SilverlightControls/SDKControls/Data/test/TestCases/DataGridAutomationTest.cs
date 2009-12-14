// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Collections.Generic;
using System.Windows.Controls.Primitives;
using System.Windows.Controls.Data.Test.DataClasses;
using System.Windows.Data;
using System.Windows.Automation.Peers;
using System.Reflection;
using System.Windows.Automation.Provider;
using System.Windows.Automation;
using System.Windows.Markup;

namespace System.Windows.Controls.Test
{
    /// <summary>
    /// This class runs the unit tests for DataGrid automation peers
    /// </summary>
    [TestClass]
    [Ignore("Not implemented")]
    public class DataGridAutomationTest : SilverlightControlTest
    {
        #region DataGridAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for the DataGrid
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for the DataGrid")]
        public void DataGrid_AutomationPeer()
        {
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);

            DataGridAutomationPeer peer = ((DataGridAutomationPeer)DataGridAutomationPeer.CreatePeerForElement(dataGrid));
            Assert.IsNotNull(peer);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(dataGrid.Height, peer.GetBoundingRectangle().Height, "Incorrect BoundingRectangle.Height value");
                Assert.AreEqual(dataGrid.Width, peer.GetBoundingRectangle().Width, "Incorrect BoundingRectangle.Width value");
                Assert.AreEqual(dataGrid.GetType().Name, peer.GetClassName(), "Incorrect ClassName value");
                Assert.AreEqual(Automation.Peers.AutomationControlType.DataGrid, peer.GetAutomationControlType(), "Incorrect ControlType value");
                Assert.AreEqual(true, peer.IsContentElement(), "Incorrect IsContentElement value");
                Assert.AreEqual(true, peer.IsControlElement(), "Incorrect IsControlElement value");
                Assert.AreEqual(true, peer.IsKeyboardFocusable(), "Incorrect IsKeyBoardFocusable value");
                Assert.IsNotNull(peer.GetPattern(PatternInterface.Grid), "Incorrect GetPattern result for PatternInterface.Grid");
                Assert.IsNull(peer.GetPattern(PatternInterface.Scroll), "Incorrect GetPattern result for PatternInterface.Scroll");
                Assert.IsNotNull(peer.GetPattern(PatternInterface.Selection), "Incorrect GetPattern result for PatternInterface.Selection");
                Assert.IsNotNull(peer.GetPattern(PatternInterface.Table), "Incorrect GetPattern result for PatternInterface.Table");
                Assert.IsNotNull(peer.GetChildren(), "GetChildren should never return null");
            });

            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGrid's grid control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGrid's grid control pattern")]
        public void DataGrid_IGridProvider()
        {
            List<Customer> boundList = CreateCustomerList(25);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;

            DataGridAutomationPeer peer = ((DataGridAutomationPeer)DataGridAutomationPeer.CreatePeerForElement(dataGrid));
            Assert.IsNotNull(peer);
            IGridProvider gridProvider = ((IGridProvider)peer.GetPattern(PatternInterface.Grid));
            Assert.IsNotNull(gridProvider);
            TestPeer testPeer = new TestPeer(dataGrid);
            Assert.IsNotNull(testPeer);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(dataGrid.RowCount, gridProvider.RowCount, "Incorrect RowCount value");
                Assert.AreEqual(dataGrid.Columns.Count, gridProvider.ColumnCount, "Incorrect ColumnCount value");
                
                IRawElementProviderSimple item = gridProvider.GetItem(21, 2);
                Assert.IsNotNull(item, "GetItem returned null for valid cell");
                AutomationPeer itemPeer = testPeer.GetPeerFromProvider(item);
                Assert.AreEqual(typeof(DataGridCell).Name, itemPeer.GetClassName(), "GetItem did not return DataGridCell");

                item = gridProvider.GetItem(100, 100);
                Assert.IsNull(item, "GetItem returned object for invalid cell");
            });
            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGrid's scroll control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGrid's scroll control pattern")]
        public void DataGrid_IScrollProvider()
        {
            int size = 25;
            List<Customer> boundList = CreateCustomerList(size);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = false;
            dataGrid.ItemsSource = boundList;

            DataGridAutomationPeer peer = ((DataGridAutomationPeer)DataGridAutomationPeer.CreatePeerForElement(dataGrid));
            Assert.IsNotNull(peer);
            IScrollProvider scrollProvider;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // Ensure that we can't scroll yet
                scrollProvider = ((IScrollProvider)peer.GetPattern(PatternInterface.Scroll));
                Assert.IsNull(scrollProvider, "Scroll control pattern should not exist yet");
                AddTextBoxColumn(dataGrid, GetRandomCustomerPropertyName());
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // We should only be able to scroll vertically now
                scrollProvider = ((IScrollProvider)peer.GetPattern(PatternInterface.Scroll));
                Assert.IsNotNull(scrollProvider, "Scroll control pattern should exist now");

                // Check horizontal scrollbar values
                Assert.AreEqual(false, scrollProvider.HorizontallyScrollable, "Incorrect HorizontallyScrollable value");
                Assert.AreEqual(-1, scrollProvider.HorizontalScrollPercent, "Incorrect HorizontalScrollPercent value");
                Assert.AreEqual(100.0, scrollProvider.HorizontalViewSize, "Incorrect HorizontalViewSize value");

                // Check vertical scrollbar values
                Assert.AreEqual(true, scrollProvider.VerticallyScrollable, "Incorrect VerticallyScrollable value");
                Assert.AreEqual(0.0, scrollProvider.VerticalScrollPercent, "Incorrect VerticalScrollPercent value");
                Assert.AreEqual(ScrollViewSize(dataGrid.VerticalScrollBar), scrollProvider.VerticalViewSize, "Incorrect VerticalViewSize value");

                dataGrid.Columns.Clear();
                dataGrid.AutoGenerateColumns = true;
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // Now we should be able to scroll in either direction
                scrollProvider = ((IScrollProvider)peer.GetPattern(PatternInterface.Scroll));
                Assert.IsNotNull(scrollProvider, "Scroll control pattern should exist now");

                // Check horizontal scrollbar values again because they've changed
                Assert.AreEqual(true, scrollProvider.HorizontallyScrollable, "Incorrect HorizontallyScrollable value");
                Assert.AreEqual(0.0, scrollProvider.HorizontalScrollPercent, "Incorrect HorizontalScrollPercent value");
                Assert.AreEqual(ScrollViewSize(dataGrid.HorizontalScrollBar), scrollProvider.HorizontalViewSize, "Incorrect HorizontalViewSize value");

                scrollProvider.Scroll(ScrollAmount.SmallIncrement, ScrollAmount.SmallIncrement);
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // Check that a scroll worked correctly
                scrollProvider = ((IScrollProvider)peer.GetPattern(PatternInterface.Scroll));
                Assert.IsNotNull(scrollProvider, "Scroll control pattern should exist now");

                Assert.AreEqual(1, dataGrid.DisplayData.FirstDisplayedScrollingCol);
                Assert.AreEqual(1, dataGrid.DisplayData.FirstDisplayedScrollingRow);

                scrollProvider.Scroll(ScrollAmount.LargeIncrement, ScrollAmount.LargeIncrement);
                scrollProvider.Scroll(ScrollAmount.NoAmount, ScrollAmount.NoAmount);
                scrollProvider.Scroll(ScrollAmount.LargeDecrement, ScrollAmount.LargeDecrement);
                scrollProvider.Scroll(ScrollAmount.SmallDecrement, ScrollAmount.SmallDecrement);
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // Check that subsequent scrolls worked correctly
                scrollProvider = ((IScrollProvider)peer.GetPattern(PatternInterface.Scroll));
                Assert.IsNotNull(scrollProvider, "Scroll control pattern should exist now");

                Assert.AreEqual(0, dataGrid.DisplayData.FirstDisplayedScrollingCol);
                Assert.AreEqual(0, dataGrid.DisplayData.FirstDisplayedScrollingRow);

                scrollProvider.SetScrollPercent(100.0, 100.0);
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // Check that a percent scroll worked correctly
                Assert.IsTrue(dataGrid.DisplayData.FirstDisplayedScrollingCol > 0);
                Assert.AreEqual(size - 1, dataGrid.DisplayData.LastDisplayedScrollingRow);
            });
            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGrid's selection control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGrid's selection control pattern")]
        public void DataGrid_ISelectionProvider()
        {
            List<Customer> boundList = CreateCustomerList(25);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;

            DataGridAutomationPeer peer = ((DataGridAutomationPeer)DataGridAutomationPeer.CreatePeerForElement(dataGrid));
            Assert.IsNotNull(peer);
            ISelectionProvider selectionProvider = ((ISelectionProvider)peer.GetPattern(PatternInterface.Selection));
            Assert.IsNotNull(selectionProvider);
            TestPeer testPeer = new TestPeer(dataGrid);
            Assert.IsNotNull(testPeer);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                IRawElementProviderSimple[] selection = selectionProvider.GetSelection();
                Assert.IsNotNull(selection, "GetItem returned null for valid cell");
                AutomationPeer selectionPeer = testPeer.GetPeerFromProvider(selection[0]);
                Assert.AreEqual(typeof(DataGridRow).Name, selectionPeer.GetClassName(), "GetSelection did not return DataGridRow");

                Assert.AreEqual(dataGrid.SelectionMode == DataGridSelectionMode.Extended, selectionProvider.CanSelectMultiple, "Incorrect CanSelectMultiple value");
                Assert.AreEqual(false, selectionProvider.IsSelectionRequired, "Incorrect IsSelectionRequired value");
            });
            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGrid's table control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGrid's table control pattern")]
        public void DataGrid_ITableProvider()
        {
            List<Customer> boundList = CreateCustomerList(25);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.HeadersVisibility = DataGridHeadersVisibility.All;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;

            DataGridAutomationPeer peer = ((DataGridAutomationPeer)DataGridAutomationPeer.CreatePeerForElement(dataGrid));
            Assert.IsNotNull(peer);
            ITableProvider tableProvider = ((ITableProvider)peer.GetPattern(PatternInterface.Table));
            Assert.IsNotNull(tableProvider);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(RowOrColumnMajor.RowMajor, tableProvider.RowOrColumnMajor, "Incorrect RowOrColumnMajor value");

                // Check the column headers
                IRawElementProviderSimple[] headers = tableProvider.GetColumnHeaders();
                Assert.IsNotNull(headers, "GetColumnHeaders returns null");
                if (headers != null)
                {
                    Assert.AreEqual(dataGrid.Columns.Count, headers.Length, "Incorrect number of column headers");
                }

                // Check the row headers
                headers = tableProvider.GetRowHeaders();
                Assert.IsNotNull(headers, "GetRowHeaders returns null");
                if (headers != null)
                {
                    Assert.AreEqual(dataGrid.DisplayData.NumDisplayedScrollingRows, headers.Length, "Incorrect number of column headers");
                }
            });
            EnqueueTestComplete();
        }

        #endregion

        #region DataGridCellAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for the DataGridCellAutomationPeer
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for the DataGridCellAutomationPeer")]
        public void DataGridCell_AutomationPeer()
        {
            List<Customer> boundList = CreateCustomerList(1);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                DataGridCell cell = dataGrid.GetRowFromItem(boundList[0]).Cells[0];
                AutomationPeer peer = DataGridAutomationPeer.CreatePeerForElement(cell);
                Assert.IsNotNull(peer);
                Assert.AreEqual(cell.GetType().Name, peer.GetClassName(), "Incorrect ClassName value");
                Assert.AreEqual(Automation.Peers.AutomationControlType.Custom, peer.GetAutomationControlType(), "Incorrect ControlType value");
                Assert.AreEqual(true, peer.IsContentElement(), "Incorrect IsContentElement value");
                Assert.AreEqual(true, peer.IsControlElement(), "Incorrect IsControlElement value");
                Assert.AreEqual(true, peer.IsKeyboardFocusable(), "Incorrect IsKeyBoardFocusable value");
                Assert.AreEqual(((TextBlock)cell.Content).Text, peer.GetName(), "Incorrect Name value");
                Assert.IsNotNull(peer.GetChildren(), "GetChildren should never return null");
            });

            EnqueueTestComplete();
        }
        
        /// <summary>
        /// Tests the DataGridCell's grid item control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridCell's grid item control pattern")]
        public void DataGridCell_IGridItemProvider()
        {
            List<Customer> boundList = CreateCustomerList(5);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 200;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;

            DataGridAutomationPeer gridPeer = ((DataGridAutomationPeer)DataGridAutomationPeer.CreatePeerForElement(dataGrid));
            Assert.IsNotNull(gridPeer);
            TestPeer testPeer = new TestPeer(dataGrid);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                AutomationPeer peer;
                IGridItemProvider gridItem;
                foreach (DataGridRow row in dataGrid.DisplayData.GetScrollingRows())
                {
                    foreach (DataGridCell cell in row.Cells)
                    {
                        peer = DataGridAutomationPeer.CreatePeerForElement(cell);
                        Assert.IsNotNull(peer);
                        gridItem = (IGridItemProvider)peer.GetPattern(PatternInterface.GridItem);
                        Assert.IsNotNull(gridItem);

                        Assert.AreEqual(cell.ColumnIndex, gridItem.Column);
                        Assert.AreEqual(cell.RowIndex, gridItem.Row);
                        Assert.AreEqual(1, gridItem.ColumnSpan);
                        Assert.AreEqual(1, gridItem.RowSpan);
                        Assert.AreEqual(gridPeer, testPeer.GetPeerFromProvider(gridItem.ContainingGrid));
                    }
                }
            });

            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGridCell's invoke control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridCell's invoke control pattern")]
        public void DataGridCell_IInvokeProvider()
        {
            List<Customer> boundList = CreateCustomerList(2);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 200;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            DataGridCell cell;
            AutomationPeer peer;
            IInvokeProvider invoker;

            EnqueueCallback(delegate
            {
                cell = dataGrid.DisplayData.GetDisplayedRow(1).Cells[2];
                peer = DataGridAutomationPeer.CreatePeerForElement(cell);
                Assert.IsNotNull(peer);
                invoker = (IInvokeProvider)peer.GetPattern(PatternInterface.Invoke);
                Assert.IsNotNull(invoker);

                Assert.AreEqual(0, dataGrid.CurrentColumnIndex);
                Assert.AreEqual(0, dataGrid.CurrentRowIndex);
                Assert.IsFalse(dataGrid.DisplayData.GetDisplayedRow(1).IsSelected);
                Assert.AreEqual(-1, dataGrid.EditingColumnIndex);
                Assert.IsNull(dataGrid.EditingRowIndex);
                invoker.Invoke();
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(2, dataGrid.CurrentColumnIndex);
                Assert.AreEqual(1, dataGrid.CurrentRowIndex);
                Assert.IsTrue(dataGrid.DisplayData.GetDisplayedRow(1).IsSelected);
                Assert.AreEqual(2, dataGrid.EditingColumnIndex);
                Assert.AreEqual(1, dataGrid.EditingRowIndex);

                cell = dataGrid.DisplayData.GetDisplayedRow(1).Cells[2];
                peer = DataGridAutomationPeer.CreatePeerForElement(cell);
                Assert.IsNotNull(peer);
                invoker = (IInvokeProvider)peer.GetPattern(PatternInterface.Invoke);
                Assert.IsNotNull(invoker);
                invoker.Invoke();
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(2, dataGrid.CurrentColumnIndex);
                Assert.AreEqual(1, dataGrid.CurrentRowIndex);
                Assert.IsTrue(dataGrid.DisplayData.GetDisplayedRow(1).IsSelected);
                Assert.AreEqual(2, dataGrid.EditingColumnIndex);
                Assert.AreEqual(1, dataGrid.EditingRowIndex);
            });

            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGridCell's scroll item control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridCell's scroll item control pattern")]
        public void DataGridCell_IScrollItemProvider()
        {
            List<Customer> boundList = CreateCustomerList(25);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;
            dataGrid.ColumnWidth = new DataGridLength(100.0);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                DataGridCell cell = dataGrid.DisplayData.GetDisplayedRow(5).Cells[dataGrid.Columns.Count - 1];
                AutomationPeer peer = DataGridAutomationPeer.CreatePeerForElement(cell);
                Assert.IsNotNull(peer);
                IScrollItemProvider scrollItem = peer.GetPattern(PatternInterface.ScrollItem) as IScrollItemProvider;
                Assert.IsNotNull(scrollItem);
                scrollItem.ScrollIntoView();
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(dataGrid.Columns.Count - 1, dataGrid.DisplayData.LastTotallyDisplayedScrollingCol);
                Assert.IsTrue(dataGrid.DisplayData.IsRowDisplayed(5));
            });
            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGridCell's selection item control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridCell's selection item control pattern")]
        public void DataGridCell_ISelectionItemProvider()
        {
            List<Customer> boundList = CreateCustomerList(10);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;
            dataGrid.ColumnWidth = new DataGridLength(100.0);

            TestPeer testPeer = new TestPeer(dataGrid);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // Get automation peers and the selection item provider for (0,0)
                DataGridRow row = dataGrid.DisplayData.GetDisplayedRow(0);
                AutomationPeer rowPeer = DataGridAutomationPeer.CreatePeerForElement(row);
                DataGridCell cell = row.Cells[0];
                AutomationPeer peer = DataGridAutomationPeer.CreatePeerForElement(cell);
                Assert.IsNotNull(peer);
                ISelectionItemProvider selectionItem = peer.GetPattern(PatternInterface.SelectionItem) as ISelectionItemProvider;
                Assert.IsNotNull(selectionItem);

                // Test the properties
                Assert.IsTrue(selectionItem.IsSelected);
                Assert.AreEqual(rowPeer, testPeer.GetPeerFromProvider(selectionItem.SelectionContainer));
                // 







                // Get a new selection item provider for a new cell (3,2)
                row = dataGrid.DisplayData.GetDisplayedRow(3);
                rowPeer = DataGridAutomationPeer.CreatePeerForElement(row);
                cell = row.Cells[2];
                peer = DataGridAutomationPeer.CreatePeerForElement(cell);
                Assert.IsNotNull(peer);
                selectionItem = peer.GetPattern(PatternInterface.SelectionItem) as ISelectionItemProvider;
                Assert.IsNotNull(selectionItem);

                // Test the new cell
                Assert.IsFalse(selectionItem.IsSelected);
                Assert.AreEqual(rowPeer, testPeer.GetPeerFromProvider(selectionItem.SelectionContainer));
                // 






                selectionItem.Select();
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(2, dataGrid.CurrentColumnIndex);
                Assert.AreEqual(3, dataGrid.CurrentRowIndex);
                Assert.AreEqual(3, dataGrid.SelectedIndex);
            });
            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGridCell's table item control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridCell's table item control pattern")]
        public void DataGridCell_ITableItemProvider()
        {
            List<Customer> boundList = CreateCustomerList(10);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.HeadersVisibility = DataGridHeadersVisibility.All;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;
            dataGrid.ColumnWidth = new DataGridLength(100.0);

            TestPeer testPeer = new TestPeer(dataGrid);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // Get automation peers and the selection item provider for (0,0)
                DataGridRow row = dataGrid.DisplayData.GetDisplayedRow(1);
                AutomationPeer rowPeer = DataGridAutomationPeer.CreatePeerForElement(row);
                DataGridCell cell = row.Cells[2];
                AutomationPeer cellPeer = DataGridAutomationPeer.CreatePeerForElement(cell);
                Assert.IsNotNull(cellPeer);
                ITableItemProvider tableItem = cellPeer.GetPattern(PatternInterface.TableItem) as ITableItemProvider;
                Assert.IsNotNull(tableItem);

                // Test GetColumnHeaders
                IRawElementProviderSimple[] headers = tableItem.GetColumnHeaderItems();
                Assert.AreEqual(1, headers.Length);
                AutomationPeer headerPeer = DataGridAutomationPeer.CreatePeerForElement(dataGrid.Columns[2].HeaderCell);
                Assert.AreEqual(headerPeer, testPeer.GetPeerFromProvider(headers[0]));

                // Test GetRowHeaders
                headers = tableItem.GetRowHeaderItems();
                Assert.AreEqual(1, headers.Length);
                headerPeer = DataGridAutomationPeer.CreatePeerForElement(row.HeaderCell);
                Assert.AreEqual(headerPeer, testPeer.GetPeerFromProvider(headers[0]));
            });

            EnqueueTestComplete();
        }

        #endregion

        #region DataGridColumnHeadersPresenterAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for the DataGridColumnHeadersPresenterAutomationPeer
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for the DataGridColumnHeadersPresenterAutomationPeer")]
        public void DataGridColumnHeadersPresenter_AutomationPeer()
        {
            List<Customer> boundList = CreateCustomerList(25);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                DataGridColumnHeadersPresenterAutomationPeer peer =
                    ((DataGridColumnHeadersPresenterAutomationPeer)DataGridAutomationPeer.CreatePeerForElement(dataGrid.ColumnHeaders));
                Assert.IsNotNull(peer);
                Assert.AreEqual(dataGrid.ColumnHeaders.GetType().Name, peer.GetClassName(), "Incorrect ClassName value");
                Assert.AreEqual(Automation.Peers.AutomationControlType.Header, peer.GetAutomationControlType(), "Incorrect ControlType value");
                Assert.AreEqual(false, peer.IsContentElement(), "Incorrect IsContentElement value");
                Assert.AreEqual(true, peer.IsControlElement(), "Incorrect IsControlElement value");
                Assert.AreEqual(false, peer.IsKeyboardFocusable(), "Incorrect IsKeyBoardFocusable value");
                Assert.IsNotNull(peer.GetChildren(), "GetChildren should never return null");
            });

            EnqueueTestComplete();
        }

        #endregion

        #region DataGridColumnHeaderAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for the DataGridColumnHeaderAutomationPeer
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for the DataGridColumnHeaderAutomationPeer")]
        public void DataGridColumnHeader_AutomationPeer()
        {
            List<Customer> boundList = CreateCustomerList(25);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                DataGridColumnHeaderAutomationPeer peer =
                    ((DataGridColumnHeaderAutomationPeer)DataGridAutomationPeer.CreatePeerForElement(dataGrid.Columns[0].HeaderCell));
                Assert.IsNotNull(peer);
                Assert.AreEqual(typeof(DataGridColumnHeader).Name, peer.GetClassName(), "Incorrect ClassName value");
                Assert.AreEqual(Automation.Peers.AutomationControlType.HeaderItem, peer.GetAutomationControlType(), "Incorrect ControlType value");
                Assert.AreEqual(false, peer.IsContentElement(), "Incorrect IsContentElement value");
                Assert.AreEqual(true, peer.IsControlElement(), "Incorrect IsControlElement value");
                Assert.AreEqual(false, peer.IsKeyboardFocusable(), "Incorrect IsKeyBoardFocusable value");
                Assert.IsNotNull(peer.GetChildren(), "GetChildren should never return null");
            });

            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGridColumnHeader's invoke control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridColumnHeader's invoke control pattern")]
        public void DataGridColumnHeader_IInvokeProvider()
        {
            List<Customer> boundList = CreateCustomerList(4);
            boundList[0].FirstName = "frank";
            boundList[1].FirstName = "bill";
            boundList[2].FirstName = "adam";
            boundList[3].FirstName = "mark";
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 200;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = false;
            AddTextBoxColumn(dataGrid, "FirstName");
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                DataGridColumnHeaderAutomationPeer peer =
                    ((DataGridColumnHeaderAutomationPeer)DataGridAutomationPeer.CreatePeerForElement(dataGrid.Columns[0].HeaderCell));
                Assert.IsNotNull(peer);
                IInvokeProvider invoker = peer.GetPattern(PatternInterface.Invoke) as IInvokeProvider;
                Assert.IsNotNull(invoker);
                invoker.Invoke();
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // The customers should have been sorted
                Assert.AreEqual("adam", ((TextBlock)dataGrid.Columns[0].GetCellContent(dataGrid.DisplayData.GetDisplayedRow(0))).Text);
                Assert.AreEqual("bill", ((TextBlock)dataGrid.Columns[0].GetCellContent(dataGrid.DisplayData.GetDisplayedRow(1))).Text);
                Assert.AreEqual("frank", ((TextBlock)dataGrid.Columns[0].GetCellContent(dataGrid.DisplayData.GetDisplayedRow(2))).Text);
                Assert.AreEqual("mark", ((TextBlock)dataGrid.Columns[0].GetCellContent(dataGrid.DisplayData.GetDisplayedRow(3))).Text);
            });
            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGridColumnHeader's scroll item control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridColumnHeader's scroll item control pattern")]
        public void DataGridColumnHeader_IScrollItemProvider()
        {
            List<Customer> boundList = CreateCustomerList(25);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;
            dataGrid.ColumnWidth = new DataGridLength(100.0);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                AutomationPeer peer =
                    DataGridAutomationPeer.CreatePeerForElement(dataGrid.Columns[dataGrid.Columns.Count - 1].HeaderCell);
                Assert.IsNotNull(peer);
                IScrollItemProvider scrollItem = peer.GetPattern(PatternInterface.ScrollItem) as IScrollItemProvider;
                Assert.IsNotNull(scrollItem);
                scrollItem.ScrollIntoView();
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(dataGrid.Columns[dataGrid.DisplayData.LastTotallyDisplayedScrollingCol], dataGrid.Columns[dataGrid.Columns.Count - 1]);
            });
            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGridColumnHeader's transform control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridColumnHeader's transform control pattern")]
        public void DataGridColumnHeader_ITransformProvider()
        {
            List<Customer> boundList = CreateCustomerList(25);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;
            dataGrid.ColumnWidth = new DataGridLength(100.0);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                AutomationPeer peer = DataGridAutomationPeer.CreatePeerForElement(dataGrid.Columns[0].HeaderCell);
                Assert.IsNotNull(peer);
                ITransformProvider transformer = peer.GetPattern(PatternInterface.Transform) as ITransformProvider;
                Assert.IsNotNull(transformer);

                Assert.AreEqual(false, transformer.CanMove);
                // 







                Assert.AreEqual(false, transformer.CanRotate);
                // 







                Assert.AreEqual(dataGrid.CanUserResizeColumns, transformer.CanResize);
                transformer.Resize(75.0, 0);

            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(75.0, dataGrid.Columns[0].Width.Value, "ITransformProvider.Resize did not work");
            });
            EnqueueTestComplete();
        }

        #endregion

        #region DataGridDetailsPresenterAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for the DataGridDetailsPresenterAutomationPeer
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for the DataGridDetailsPresenterAutomationPeer")]
        public void DataGridDetailsPresenter_AutomationPeer()
        {
            List<Customer> boundList = CreateCustomerList(15);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;
            dataGrid.RowDetailsTemplate = (DataTemplate)XamlReader.Load(@"<DataTemplate xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" ><TextBlock Text=""Test"" /></DataTemplate>");

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                DataGridDetailsPresenter details = dataGrid.GetRowFromItem(boundList[0]).TestHook.DetailsPresenter;
                AutomationPeer peer = DataGridAutomationPeer.CreatePeerForElement(details);
                Assert.IsNotNull(peer);
                Assert.AreEqual(typeof(DataGridDetailsPresenter).Name, peer.GetClassName(), "Incorrect ClassName value");
                Assert.AreEqual(Automation.Peers.AutomationControlType.Custom, peer.GetAutomationControlType(), "Incorrect ControlType value");
                Assert.AreEqual(false, peer.IsContentElement(), "Incorrect IsContentElement value");
                Assert.AreEqual(true, peer.IsControlElement(), "Incorrect IsControlElement value");
                Assert.AreEqual(false, peer.IsKeyboardFocusable(), "Incorrect IsKeyBoardFocusable value");
                Assert.IsNotNull(peer.GetChildren(), "GetChildren should never return null");
            });

            EnqueueTestComplete();
        }

        #endregion

        #region DataGridItemAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for a DataGrid item
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for a DataGrid item")]
        public void DataGridItem_AutomationPeer()
        {
            List<Customer> boundList = CreateCustomerList(1);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                AutomationPeer peer = new DataGridItemAutomationPeer(boundList[0], dataGrid);
                Assert.IsNotNull(peer);
                Assert.AreEqual(Automation.Peers.AutomationControlType.DataItem, peer.GetAutomationControlType(), "Incorrect ControlType value");
                Assert.AreEqual(true, peer.IsContentElement(), "Incorrect IsContentElement value");
                Assert.AreEqual(true, peer.IsControlElement(), "Incorrect IsControlElement value");
                Assert.AreEqual(false, peer.IsKeyboardFocusable(), "Incorrect IsKeyBoardFocusable value");
                Assert.AreEqual(boundList[0].ToString(), peer.GetName(), "Incorrect Name value");
                Assert.IsNotNull(peer.GetChildren(), "GetChildren should never return null");
            });

            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGridItem's invoke control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridItem's invoke control pattern")]
        public void DataGridItem_IInvokeProvider()
        {
            List<Customer> boundList = CreateCustomerList(5);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 200;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            AutomationPeer peer;
            IInvokeProvider invoker;

            int index = 0;
            for (int i = 0; i < boundList.Count; i++)
            {
                EnqueueCallback(delegate
                {
                    Assert.IsNull(dataGrid.EditingRowIndex);
                    Assert.AreEqual(-1, dataGrid.EditingColumnIndex);

                    peer = new DataGridItemAutomationPeer(boundList[index], dataGrid);
                    Assert.IsNotNull(peer);
                    invoker = (IInvokeProvider)peer.GetPattern(PatternInterface.Invoke);
                    Assert.IsNotNull(invoker);

                    invoker.Invoke();
                });
                this.EnqueueYieldThread();

                EnqueueCallback(delegate
                {
                    Assert.AreEqual(0, dataGrid.EditingColumnIndex);
                    Assert.AreEqual(index, dataGrid.EditingRowIndex);

                    peer = new DataGridItemAutomationPeer(boundList[index], dataGrid);
                    Assert.IsNotNull(peer);
                    invoker = (IInvokeProvider)peer.GetPattern(PatternInterface.Invoke);
                    Assert.IsNotNull(invoker);

                    invoker.Invoke();
                    index++;
                });
                this.EnqueueYieldThread();

                EnqueueCallback(delegate
                {
                    Assert.IsNull(dataGrid.EditingRowIndex);
                    Assert.AreEqual(-1, dataGrid.EditingColumnIndex);
                });
            }
            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGridItem's scroll item control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridItem's scroll item control pattern")]
        public void DataGridItem_IScrollItemProvider()
        {
            List<Customer> boundList = CreateCustomerList(25);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;
            dataGrid.ColumnWidth = new DataGridLength(100.0);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            AutomationPeer peer;
            IScrollItemProvider scrollItem;

            int index = boundList.Count;
            for (int i = 0; i < boundList.Count; i++)
            {
                EnqueueCallback(delegate
                {
                    index--;
                    peer = new DataGridItemAutomationPeer(boundList[index], dataGrid);
                    Assert.IsNotNull(peer);
                    scrollItem = peer.GetPattern(PatternInterface.ScrollItem) as IScrollItemProvider;
                    Assert.IsNotNull(scrollItem);
                    scrollItem.ScrollIntoView();
                });
                this.EnqueueYieldThread();

                EnqueueCallback(delegate
                {
                    Assert.IsTrue(dataGrid.DisplayData.IsRowDisplayed(index));
                });
            }
            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGridItem's selection item control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridItem's selection item control pattern")]
        public void DataGridItem_ISelectionItemProvider()
        {
            List<Customer> boundList = CreateCustomerList(5);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;
            dataGrid.ColumnWidth = new DataGridLength(100.0);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            List<DataGridRow> rows = new List<DataGridRow>();
            AutomationPeer peer;
            List<ISelectionItemProvider> selectionItems = new List<ISelectionItemProvider>();

            EnqueueCallback(delegate
            {
                for (int i = 0; i < boundList.Count; i++)
                {
                    rows.Add(dataGrid.GetRowFromItem(boundList[i]));
                    peer = new DataGridItemAutomationPeer(boundList[i], dataGrid);
                    Assert.IsNotNull(peer);
                    selectionItems.Add(peer.GetPattern(PatternInterface.SelectionItem) as ISelectionItemProvider);
                    Assert.IsNotNull(selectionItems[i]);
                    selectionItems[i].AddToSelection();
                }
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                foreach (DataGridRow row in rows)
                {
                    Assert.IsTrue(row.IsSelected);
                }

                foreach (ISelectionItemProvider select in selectionItems)
                {
                    select.RemoveFromSelection();
                }
            });
            this.EnqueueYieldThread();
            
            EnqueueCallback(delegate
            {
                foreach (DataGridRow row in rows)
                {
                    Assert.IsFalse(row.IsSelected);
                }

                selectionItems[0].Select();
                selectionItems[3].Select();
                selectionItems[3].Select();
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.IsFalse(rows[0].IsSelected);
                Assert.IsTrue(rows[3].IsSelected);

                dataGrid.SelectionMode = DataGridSelectionMode.Single;
                selectionItems[4].AddToSelection();
                // 






            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.IsFalse(rows[2].IsSelected);
                Assert.IsTrue(rows[4].IsSelected);
            });

            EnqueueTestComplete();
        }

        /// <summary>
        /// Tests the DataGridItem's selection control pattern
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the DataGridItem's selection control pattern")]
        public void DataGridItem_ISelectionProvider()
        {
            List<Customer> boundList = CreateCustomerList(5);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;
            dataGrid.ColumnWidth = new DataGridLength(100.0);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            AutomationPeer peer;
            ISelectionProvider selector;
            TestPeer testPeer = new TestPeer(dataGrid);

            EnqueueCallback(delegate
            {
                peer = new DataGridItemAutomationPeer(boundList[0], dataGrid);
                Assert.IsNotNull(peer);
                selector = peer.GetPattern(PatternInterface.SelectionItem) as ISelectionProvider;
                Assert.IsNotNull(selector);
                Assert.AreEqual(1, selector.GetSelection().Length);
                peer = testPeer.GetPeerFromProvider(selector.GetSelection()[0]);
                Assert.AreEqual(typeof(DataGridCell).Name, peer.GetClassName(), "GetSelection did not return DataGridCell");
            });
            this.EnqueueYieldThread();

            EnqueueTestComplete();
        }

        #endregion

        #region DataGridRowAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for a DataGridRow
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for a DataGridRow")]
        public void DataGridRow_AutomationPeer()
        {
            List<Customer> boundList = CreateCustomerList(5);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // Row's automation peer is only a wrapper for a DataGridItemAutomationPeer, so there isn't much to test
                AutomationPeer peer = DataGridAutomationPeer.CreatePeerForElement(dataGrid.DisplayData.GetDisplayedRow(0));
                Assert.IsNotNull(peer);
                Assert.AreEqual(typeof(DataGridRow).Name, peer.GetClassName(), "Incorrect ClassName value");
                Assert.AreEqual(Automation.Peers.AutomationControlType.DataItem, peer.GetAutomationControlType(), "Incorrect ControlType value");
                Assert.IsNotNull(peer.GetChildren(), "GetChildren should never return null");
            });

            EnqueueTestComplete();
        }

        #endregion

        #region DataGridRowsPresenterAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for the DataGridRowsPresenterAutomationPeer
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for the DataGridRowsPresenterAutomationPeer")]
        public void DataGridRowsPresenter_AutomationPeer()
        {
            List<Customer> boundList = CreateCustomerList(15);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = true;

            DataGridRowsPresenter rowsPresenter = new DataGridRowsPresenter();

            TestPanel.Children.Add(dataGrid);
            TestPanel.Children.Add(rowsPresenter);

            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            AutomationPeer peer = null;
            EnqueueCallback(delegate
            {
                peer = DataGridAutomationPeer.CreatePeerForElement(dataGrid.TestHook.RowsPresenter);
                Assert.IsNotNull(peer);
                Assert.AreEqual(typeof(DataGridRowsPresenter).Name, peer.GetClassName(), "Incorrect ClassName value");
                Assert.AreEqual(Automation.Peers.AutomationControlType.Custom, peer.GetAutomationControlType(), "Incorrect ControlType value");
                Assert.AreEqual(false, peer.IsContentElement(), "Incorrect IsContentElement value");
                Assert.AreEqual(true, peer.IsControlElement(), "Incorrect IsControlElement value");
                Assert.AreEqual(false, peer.IsKeyboardFocusable(), "Incorrect IsKeyBoardFocusable value");
                Assert.IsNotNull(peer.GetChildren(), "GetChildren should never return null");
                Assert.AreEqual(0, peer.GetChildren().Count, "Incorrect number of children");
                dataGrid.ItemsSource = boundList;
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // After setting the ItemsSource, GetChildren should return boundList.Count number of elements
                Assert.AreEqual(boundList.Count, peer.GetChildren().Count, "Incorrect number of children");

                // Check that an unattached RowsPresenter does not return null for GetChildren.
                peer = DataGridAutomationPeer.CreatePeerForElement(rowsPresenter);
                Assert.IsNotNull(peer);
                Assert.IsNotNull(peer.GetChildren(), "GetChildren should never return null");
                Assert.AreEqual(0, peer.GetChildren().Count, "Empty RowsPresenter should have no children");
            });

            EnqueueTestComplete();
        }

        #endregion

        #region Helper Class

        private class TestPeer : FrameworkElementAutomationPeer
        {
            public TestPeer(FrameworkElement element)
                : base(element)
            {
            }

            public AutomationPeer GetPeerFromProvider(IRawElementProviderSimple provider)
            {
                return PeerFromProvider(provider);
            }
        }

        #endregion

        #region Helper Methods

        private void AddTextBoxColumn(DataGrid dataGrid, string dataField)
        {
            Customer customer = new Customer();
            DataGridTextColumn dataGridColumn = new DataGridTextColumn();
            dataGridColumn.Binding = new Binding(dataField);
            dataGridColumn.Header = dataField;
            dataGridColumn.Width = new DataGridLength(100);
            dataGrid.Columns.Add(dataGridColumn);
        }

        private List<Customer> CreateCustomerList(int size)
        {
            List<Customer> list = new List<Customer>();
            for (int i = 0; i < size; i++)
            {
                list.Add(new Customer());
            }
            return list;
        }

        private void dataGrid_Loaded(object sender, RoutedEventArgs e)
        {
            isLoaded = true;
        }

        private string GetRandomCustomerPropertyName()
        {
            int randomIndex = random.Next();
            PropertyInfo[] properties = typeof(Customer).GetProperties();
            randomIndex %= properties.Length;
            return properties[randomIndex].Name;
        }

        private double ScrollViewSize(ScrollBar scrollBar)
        {
            return scrollBar.ViewportSize * 100 / (scrollBar.Maximum + scrollBar.ViewportSize);
        }

        #endregion

        #region Data

        private bool isLoaded = false;
        private Random random = new Random(DateTime.Now.Millisecond);

        #endregion
    }
}
