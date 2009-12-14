// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Reflection;
using System.Windows.Controls.Data.Test.DataClasses;
using System.Windows.Controls.Data.Test.DataClassSources;
using System.Windows.Controls.Primitives;
using System.Windows.Controls.Test;
using System.Windows.Data;
using System.Windows.Markup;
using System.Windows.Shapes;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
    /// <summary>
    /// Tests that require more than one item in the sequence.  
    /// For example, tests that involve navigating between rows.
    /// </summary>
    public partial class DataGridUnitTest_RequireGT1<TDataClass, TDataClassSource> : DataGridUnitTest<TDataClass>
        where TDataClass : new()
        where TDataClassSource : DataClassSource<TDataClass>, new()
    {
        #region AllowMultipleSelections Test

        SelectionChangedEventArgs selectionChangedEventArgs;

        [TestMethod]
        [Asynchronous]
        [Description("Test that checks to see if you can have multiple selections.")]
        public virtual void AllowMultipleSelections()
        {
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            TestPanel.Children.Add(dataGrid);

            dataGrid.SelectionChanged += dataGrid_SelectionChanged;
            dataGrid.ItemsSource = boundList;
            dataGrid.SelectionMode = DataGridSelectionMode.Extended;
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.SelectedItems.Remove(boundList.First());
                foreach (TDataClass st in boundList)
                {
                    dataGrid.SelectedItems.Add(st);
                    Assert.AreEqual(0, selectionChangedEventArgs.RemovedItems.Count);
                    Assert.AreEqual(1, selectionChangedEventArgs.AddedItems.Count);
                    Assert.AreEqual(st, selectionChangedEventArgs.AddedItems[0]);
                }
                Assert.AreEqual(dataGrid.SelectedItems.Count, boundList.Count());

                //Remove the an item in the list
                dataGrid.SelectedItems.Remove(boundList.First());
                Assert.AreEqual(boundList.Count() - 1, dataGrid.SelectedItems.Count, "All items in list should be Selected except 1");
                Assert.AreEqual(0, selectionChangedEventArgs.AddedItems.Count);
                Assert.AreEqual(1, selectionChangedEventArgs.RemovedItems.Count);
                Assert.AreEqual(boundList.First(), selectionChangedEventArgs.RemovedItems[0]);

                //Make sure that clear works
                dataGrid.SelectedItems.Add(boundList.Skip(1).First());
                dataGrid.SelectedItems.Clear();
                Assert.AreEqual(0, dataGrid.SelectedItems.Count, "After Clear count should be 0");
                Assert.AreEqual(null, dataGrid.SelectedItem, "After Clear no item should be selected");
                Assert.AreEqual(0, selectionChangedEventArgs.AddedItems.Count);
                Assert.AreEqual(boundList.Count() - 1, selectionChangedEventArgs.RemovedItems.Count);

                dataGrid.SelectionChanged -= dataGrid_SelectionChanged;
            });
            EnqueueTestComplete();
        }

        void dataGrid_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            selectionChangedEventArgs = e;
        }

        #endregion

        #region ChangeRowDetailsTemplate
        [TestMethod]
        [Asynchronous]
        [Description("Changes RowDetailsTemplate and checkes that the dataGrid was updated")]
        public virtual void ChangeRowDetailsTemplate()
        {
            DataGrid dataGrid = new DataGrid();
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.Width = 450;
            dataGrid.Height = 350;
            isLoaded = false;
            dataGrid.RowDetailsTemplate = (DataTemplate)XamlReader.Load(@"<DataTemplate xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" ><TextBlock Text=""Test"" /></DataTemplate>");
            IEnumerable listTestType = new TDataClassSource();
            dataGrid.ItemsSource = listTestType;
            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            DataGridRow row = null;
            DataGridDetailsPresenter detailsPresenter = null;
            FrameworkElement detailsContent = null;
            EnqueueCallback(delegate
            {
                row = dataGrid.GetRowFromItem(listTestType.First());
                Assert.IsNotNull(row);
                detailsPresenter = FindChild(row, "DetailsPresenter") as DataGridDetailsPresenter;
                Assert.IsNotNull(detailsPresenter);
                Assert.IsTrue(detailsPresenter.Children.Count > 0);
                detailsContent = detailsPresenter.Children[0] as FrameworkElement;
                Assert.IsNotNull(detailsContent);
                Assert.IsTrue(detailsContent is TextBlock);
                dataGrid.RowDetailsTemplate = (DataTemplate)XamlReader.Load(@"<DataTemplate xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" ><TextBox Text=""Test"" /></DataTemplate>");
            });
            EnqueueCallback(delegate
            {
                Assert.IsTrue(detailsPresenter.Children.Count > 0);
                detailsContent = detailsPresenter.Children[0] as FrameworkElement;
                Assert.IsNotNull(detailsContent);
                Assert.IsTrue(detailsContent is TextBox);
            });
            EnqueueTestComplete();
        }
        #endregion ChangeRowDetailsTemplate

        [TestMethod]
        [Asynchronous]
        [Timeout(3000)]
        public virtual void NonAutoGeneratedColumns()
        {
            DataGrid dataGrid = new DataGrid();
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.AutoGenerateColumns = false;
            dataGrid.Width = 450;
            dataGrid.Height = 350;
            dataGrid.HorizontalScrollBarVisibility = ScrollBarVisibility.Hidden;
            dataGrid.ItemsSource = null;
            dataGrid.SelectedItems.Clear();
            selectionChanged = false;
            rowLoaded = false;
            isLoaded = false;
            numberOfRowsLoaded = 0;

            IEnumerable listTestType = new TDataClassSource();
            int sizeOfList = listTestType.Count();

            PropertyInfo booleanProperty = properties.Where(p => p.PropertyType == typeof(Boolean)).First();
            //Create a grid with just a check box property
            if (booleanProperty != null)
            {
                booleanProperty.SetValue(listTestType.First(), true, null);
                booleanProperty.SetValue(listTestType.Skip(1).First(), false, null);

                TestPanel.Children.Add(dataGrid);
                DataGridCheckBoxColumn dataGridColumn = new DataGridCheckBoxColumn();
                dataGridColumn.IsThreeState = true;
                dataGridColumn.Binding = new Binding(booleanProperty.Name);
                // Temporary hack.
                dataGridColumn.Header = booleanProperty.Name;
                dataGrid.Columns.Add(dataGridColumn);

                EnqueueConditional(delegate { return isLoaded; });
                EnqueueCallback(delegate
                {
                    dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);
                    dataGrid.ItemsSource = listTestType;
                });
                EnqueueConditional(delegate { return rowLoaded; });

                EnqueueCallback(delegate
                {
                    Assert.IsTrue(numberOfRowsLoaded > 0);

                    int expectedRowsLoaded = dataGrid.DisplayData.LastDisplayedScrollingRow - dataGrid.DisplayData.FirstDisplayedScrollingRow + 1;

                    //Check to make sure right number of rows were created
                    // 


                    Assert.IsTrue(expectedRowsLoaded - numberOfRowsLoaded <= 1, "Wrong number of rows loaded");
                    //Check to see if only one column was created.
                    Assert.AreEqual(1, dataGrid.Columns.Count, "More then 1 column created");

                    //Reset datagrid
                    dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);
                });
            }
            EnqueueTestComplete();
        }

        #region LoadRowUnloadRow Test

        private bool _testInvalidLoadingOperation;
        private bool _testInvalidUnLoadingOperation;
        //
        [TestMethod]
        [Asynchronous]
        public virtual void LoadRowUnloadRow()
        {
            isLoaded = false;
            DataGrid dataGrid = new DataGrid();
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.Width = 350;
            dataGrid.Height = 250;

            loadRowList = new TDataClassSource();

            TestPanel.Children.Add(dataGrid);

            EnqueueConditional(delegate { return isLoaded; });
            EnqueueCallback(delegate
            {
                _testInvalidLoadingOperation = true;
                _testInvalidUnLoadingOperation = true;
                dataGrid.UnloadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_UnloadingRowTest);
                dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowTest);
                dataGrid.ItemsSource = loadRowList;
            });
            EnqueueConditional(delegate
            {
                return rowLoaded;
            });

            EnqueueCallback(delegate
            {
                foreach (TDataClass item in loadRowList)
                {
                    dataGrid.SelectedItems.Add(item);
                }
            });

            this.EnqueueYieldThread();

            int j = 0;

            for (int i = j; i < loadRowList.Count(); i++)
            {
                EnqueueCallback(delegate
                {
                    dataGrid.ScrollIntoView(0, j++, true);
                });

                // We need to free up the thread to allow idle handlers like layout (and therefore load/unload) to execute.
                // The following is an attempt to allow it to happen as soon as possible.
                this.EnqueueYieldThread();
            }

            EnqueueConditional(delegate
            {
                return lastRowLoaded;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.IsTrue(unloadingCalled, "Unloading not called");
                unloadingCalled = false;
                dataGrid.ItemsSource = null;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate 
            {
                Assert.IsTrue(unloadingCalled, "Unloading not called when setting ItemsSource to null");

                //Reset DataGrid
                dataGrid.UnloadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_UnloadingRowTest);
                dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowTest);
                dataGrid.SelectedItems.Clear();
                selectionChanged = false;
                rowLoaded = false;
                isLoaded = false;
                lastRowLoaded = false;
                loadRowList = null;
                unloadingCalled = false;
                _testInvalidLoadingOperation = false;
                _testInvalidUnLoadingOperation = false;
            });
            EnqueueTestComplete();
        }

        bool unloadingCalled = false;

        void dataGrid_UnloadingRowTest(object sender, DataGridRowEventArgs e)
        {
            unloadingCalled = true;
            Assert.IsTrue(e.Row != null);

            if (_testInvalidUnLoadingOperation)
            {
                _testInvalidUnLoadingOperation = false;
                Common.AssertExpectedException(DataGridError.DataGrid.CannotChangeItemsWhenLoadingRows(),
                    delegate
                    {
                        e.Row.OwningGrid.ItemsSource = null;
                    }
                );
            }
        }

        void dataGrid_LoadingRowTest(object sender, DataGridRowEventArgs e)
        {
            if (rowLoaded == false)
            {
                rowLoaded = true;
            }

            if (((TDataClass)e.Row.DataContext).Equals(loadRowList.Last()))
            {
                lastRowLoaded = true;
            }

            if (_testInvalidLoadingOperation)
            {
                _testInvalidLoadingOperation = false;
                Common.AssertExpectedException(DataGridError.DataGrid.CannotChangeItemsWhenLoadingRows(),
                    delegate
                    {
                        e.Row.OwningGrid.ItemsSource = null;
                    }
                );
            }
        }
        #endregion

        #region LoadRowDetailsUnloadRowDetails Test
        [TestMethod]
        [Asynchronous]
        [MoonlightBug ("Test fails because we end up generating the row at index '2' twice, which causes a duplicate key in a dictionary.")]
        public virtual void LoadRowDetailsUnloadRowDetails_VisibleAtDataGrid()
        {
            LoadRowDetailsUnloadRowDetails(true, false);
        }

        [TestMethod]
        [Asynchronous]
        public virtual void LoadRowDetailsUnloadRowDetails_VisibleAtLoadRow()
        {
            LoadRowDetailsUnloadRowDetails(false, true);
        }

        [TestMethod]
        [Asynchronous]
        public virtual void LoadRowDetailsUnloadRowDetails_NotVisible()
        {
            LoadRowDetailsUnloadRowDetails(false, false);
        }

        private void LoadRowDetailsUnloadRowDetails(bool setVisibilityAtDataGrid, bool setVisibilityDuringLoad)
        {
            DataGrid dataGrid = new DataGrid();
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.Width = 350;
            dataGrid.RowDetailsVisibilityMode = DataGridRowDetailsVisibilityMode.Collapsed;
            loadRowList = new TDataClassSource();
            rowDetailsVisibilityChanged = false;
            rowDetailsGuids.Clear();

            dataGrid.RowDetailsTemplate = (DataTemplate)XamlReader.Load(@"<DataTemplate xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" ><TextBlock Text=""Test"" /></DataTemplate>");

            if (setVisibilityAtDataGrid)
            {
                dataGrid.RowDetailsVisibilityMode = DataGridRowDetailsVisibilityMode.Visible;
                dataGrid.Height = 250;
            }

            StackPanel stackPanel = new StackPanel();
            stackPanel.Children.Add(dataGrid);
            TestPanel.Children.Add(stackPanel);

            EnqueueConditional(delegate { return isLoaded; });
            EnqueueCallback(delegate
            {
                dataGrid.UnloadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_UnloadingRowTest);
                dataGrid.UnloadingRowDetails += new EventHandler<DataGridRowDetailsEventArgs>(dataGrid_UnloadingRowDetailsTest);
                dataGrid.LoadingRowDetails += new EventHandler<DataGridRowDetailsEventArgs>(dataGrid_LoadRowDetailsTest);
                dataGrid.RowDetailsVisibilityChanged += new EventHandler<DataGridRowDetailsEventArgs>(dataGrid_RowDetailsVisibilityChanged);

                if (setVisibilityDuringLoad)
                {
                    dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowTest_SetDetailsVisible);
                }
                else
                {
                    dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowTest);
                }

                dataGrid.ItemsSource = loadRowList;
            });
            this.EnqueueYieldThread();

            EnqueueConditional(delegate
            {
                return rowLoaded;
            });

            EnqueueCallback(delegate
            {
                foreach (TDataClass item in loadRowList)
                {
                    dataGrid.ScrollIntoView(item, null);
                }
            });
            EnqueueConditional(delegate
            {
                return lastRowLoaded;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.AreEqual(setVisibilityAtDataGrid || setVisibilityDuringLoad, loadDetailsCalled, "PrepareDetails not called");
                if (setVisibilityAtDataGrid || setVisibilityDuringLoad)
                {
                    Assert.IsTrue(dataGrid.RowDetailsHeightEstimate > 0);
                }

                // Check that Details updates when Contents change size
                if (loadDetailsCalled)
                {
                    Assert.IsNotNull(detailsContent);
                    DataGridDetailsPresenter detailsPresenter = detailsContent.Parent as DataGridDetailsPresenter;
                    Assert.IsNotNull(detailsPresenter);
                    Assert.AreEqual(detailsContent.Height, detailsPresenter.ActualHeight, "Details did update when content changed size");
                }

                Assert.AreEqual(setVisibilityDuringLoad, rowDetailsVisibilityChanged);
                
                Assert.AreEqual(rowDetailsGuids.Count, rowDetailsGuids.Keys.Distinct().Count()); // each row should be unique
                Assert.AreEqual(rowDetailsGuids.Count, rowDetailsGuids.Values.Distinct().Count()); // each guid should be unique

                // Force UnloadingRow and UnloadingRowDetails
                dataGrid.ItemsSource = null;
            });

            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(setVisibilityAtDataGrid || setVisibilityDuringLoad, unloadDetailsCalled, "UnloadDetails not called");

                //Reset DataGrid
                dataGrid.UnloadingRowDetails -= new EventHandler<DataGridRowDetailsEventArgs>(dataGrid_UnloadingRowDetailsTest);
                dataGrid.LoadingRowDetails -= new EventHandler<DataGridRowDetailsEventArgs>(dataGrid_LoadRowDetailsTest);
                dataGrid.RowDetailsVisibilityChanged -= new EventHandler<DataGridRowDetailsEventArgs>(dataGrid_RowDetailsVisibilityChanged);

                if (setVisibilityDuringLoad)
                {
                    dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowTest_SetDetailsVisible);
                }
                else
                {
                    dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowTest);
                }

                dataGrid.SelectedItems.Clear();
                selectionChanged = false;
                isLoaded = false;
                loadRowList = null;
                loadDetailsCalled = false;
                unloadDetailsCalled = false;
                rowDetailsVisibilityChanged = false;
                detailsContent = null;
                rowDetailsGuids.Clear();
            });
            EnqueueTestComplete();
        }

        bool rowDetailsVisibilityChanged = false;

        void dataGrid_RowDetailsVisibilityChanged(object sender, DataGridRowDetailsEventArgs e)
        {
            rowDetailsVisibilityChanged = true;
        }

        bool unloadDetailsCalled = false;

        void dataGrid_UnloadingRowDetailsTest(object sender, DataGridRowDetailsEventArgs e)
        {
            unloadDetailsCalled = true;

            DataGridDetailsPresenter detailsPresenter = (DataGridDetailsPresenter)e.DetailsElement.Parent;

            Assert.AreEqual(2, detailsPresenter.Children.Count);

            Assert.IsInstanceOfType(detailsPresenter.Children[1], typeof(TextBlock));

            TextBlock textBlock = (TextBlock)detailsPresenter.Children[1];

            Assert.AreEqual(rowDetailsGuids[e.DetailsElement.DataContext].ToString(), textBlock.Text);

            detailsPresenter.Children.RemoveAt(1);
        }

        bool loadDetailsCalled = false;
        Dictionary<object, Guid> rowDetailsGuids = new Dictionary<object, Guid>();
        FrameworkElement detailsContent;

        void dataGrid_LoadRowDetailsTest(object sender, DataGridRowDetailsEventArgs e)
        {
            loadDetailsCalled = true;

            DataGridDetailsPresenter detailsPresenter = (DataGridDetailsPresenter)e.DetailsElement.Parent;

            Assert.AreEqual(1, detailsPresenter.Children.Count);

            Guid newGuid = Guid.NewGuid();

            detailsContent = e.DetailsElement;
            detailsContent.Height = 123;

            rowDetailsGuids.Add(e.DetailsElement.DataContext, newGuid);

            detailsPresenter.Children.Add(new TextBlock { Text = newGuid.ToString() });
        }

        void dataGrid_LoadingRowTest_SetDetailsVisible(object sender, DataGridRowEventArgs e)
        {
            if (rowLoaded == false)
            {
                rowLoaded = true;
            }

            e.Row.DetailsVisibility = Visibility.Visible;

            if (((TDataClass)e.Row.DataContext).Equals(loadRowList.Last()))
            {
                lastRowLoaded = true;
            }
        }
        #endregion

        #region RowDetailsVisibilityChanged
        [TestMethod]
        [Asynchronous]
        [Description("Test removing an item to underlying collection and make sure grid gets updated.")]
        public virtual void RowDetailsVisibilityChanged()
        {
            DataGrid dataGrid = new DataGrid();
            TDataClassSource dataSource = new TDataClassSource();
            isLoaded = false;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.RowDetailsVisibilityChanged += new EventHandler<DataGridRowDetailsEventArgs>(RowDetailsVisibilityChanged_RowDetailsVisibilityChanged);
            dataGrid.RowDetailsTemplate = (DataTemplate)XamlReader.Load(@"<DataTemplate xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" ><TextBlock Text=""Test"" /></DataTemplate>");
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.ItemsSource = dataSource;
            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.AreEqual(1, _rowDetailsChangedCount, "RowDetailsChanged was not raised from default selection");
                
                if (System.Windows.Controls.Data.Test.DataClassSources.Extensions.Count (dataSource) > 1)
                {
                    _rowDetailsChangedCount = 0;
                    dataGrid.SelectedIndex = 1;
                    Assert.AreEqual(2, _rowDetailsChangedCount, "RowDetailsChanged was not raised after changing selection");
                }

                _rowDetailsChangedCount = 0;
                dataGrid.RowDetailsVisibilityMode = DataGridRowDetailsVisibilityMode.Collapsed;
                Assert.AreEqual(1, _rowDetailsChangedCount, "RowDetailsChanged was not raised after setting Collapsed");
                
                _rowDetailsChangedCount = 0;
                dataGrid.RowDetailsVisibilityMode = DataGridRowDetailsVisibilityMode.Visible;
                Assert.IsTrue(_rowDetailsChangedCount > 0, "RowDetailsChanged was not raised after setting Visible");

                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
                dataGrid.RowDetailsVisibilityChanged -= new EventHandler<DataGridRowDetailsEventArgs>(RowDetailsVisibilityChanged_RowDetailsVisibilityChanged);
            });

            EnqueueTestComplete();
        }

        private int _rowDetailsChangedCount;

        private void RowDetailsVisibilityChanged_RowDetailsVisibilityChanged(object sender, DataGridRowDetailsEventArgs e)
        {
            _rowDetailsChangedCount++;
        }
        #endregion RowDetailsVisibilityChanged

        [TestMethod]
        [Asynchronous]
        [Description("Test removing an item to underlying collection and make sure grid gets updated.")]
        public virtual void RemoveItemsFromUnderlyingData()
        {
            IEnumerable listSomeType = new TDataClassSource();
            int sizeOfList = listSomeType.Count();
            TDataClass itemToRemove = listSomeType.Skip(10).First<TDataClass>();

            DataGrid dataGrid1 = new DataGrid();
            TestPanel.Children.Add(dataGrid1);
            dataGrid1.ItemsSource = listSomeType;
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                //List should have sizeOfList items in it.
                int countOfItems = 0;
                foreach (TDataClass i in dataGrid1.ItemsSource)
                {
                    countOfItems++;
                    //This will fail if the item isn't in the grid.
                    dataGrid1.SelectedItems.Add(i);
                }
                Assert.AreEqual(sizeOfList, countOfItems);
                Assert.AreEqual(sizeOfList, dataGrid1.SelectedItems.Count,
                    String.Format("Should be {0} items in SelectedItems", sizeOfList));

                //Can only remove items with IList
                if (listSomeType is IList)
                {
                    TDataClass newItem = new TDataClass();
                    ((IList)listSomeType).Remove(itemToRemove);


                    //This should fail
                    Common.AssertExpectedException(DataGridError.DataGrid.ItemIsNotContainedInTheItemsSource("dataItem"),
                            delegate
                            {
                                dataGrid1.SelectedItems.Add(itemToRemove);
                            }
                        );
                        

                    if (listSomeType is INotifyCollectionChanged)
                    {
                        //There should be 1 less item selected
                        Assert.AreEqual(sizeOfList - 1, dataGrid1.SelectedItems.Count, "1 Item should be removed from SelectedItems");

                        //
                    }
                }
            });
            EnqueueTestComplete();
        }

        [TestMethod]
        [Asynchronous]
        [Description("Tests scrolling rows and columns into view.")]
        public virtual void ScrollIntoView()
        {
            // set up datagrid
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ItemsSource = boundList;

            // view tester
            Func<object, DataGridColumn, bool> IsCellInView =
                (item, column) =>
                {
                    bool rowInView = dataGrid.GetRowFromItem(item) != null;
                    bool colInView = dataGrid.IsColumnDisplayed(column.Index);

                    return rowInView && colInView;
                };

            // items
            object firstItem = boundList.First();
            object lastItem = boundList.Last();
            object badItem = new TDataClass();
            object nullItem = null;

            // columns
            DataGridColumn firstColumn = null;
            DataGridColumn lastColumn = null;
            DataGridColumn badColumn = new DataGridTextColumn();
            DataGridColumn nullColumn = null;

            #region asserts
            Action<string> AssertCurrencySelectionUnchanged =
                delegate(string message)
                {
                    Assert.AreEqual(0, dataGrid.CurrentRowIndex, message);
                    Assert.AreEqual(2, dataGrid.SelectedItems.Count, message);
                    Assert.AreSame(lastItem, dataGrid.SelectedItems[1], message);
                };

            Action<string> AssertTopLeft =
                delegate(string message)
                {
                    Assert.IsTrue(IsCellInView(firstItem, firstColumn), message);
                    Assert.IsFalse(IsCellInView(firstItem, lastColumn), message);
                    Assert.IsFalse(IsCellInView(lastItem, firstColumn), message);
                    Assert.IsFalse(IsCellInView(lastItem, lastColumn), message);
                };

            // 
            //
            //    
            //    
            //       
            //       
            //       
            //     
            //    

            Action<string> AssertBottomLeft =
                delegate(string message)
                {
                    Assert.IsFalse(IsCellInView(firstItem, firstColumn), message);
                    Assert.IsFalse(IsCellInView(firstItem, lastColumn), message);
                    Assert.IsTrue(IsCellInView(lastItem, firstColumn), message);
                    Assert.IsFalse(IsCellInView(lastItem, lastColumn), message);
                };

            Action<string> AssertBottomRight =
                delegate(string message)
                {
                    Assert.IsFalse(IsCellInView(firstItem, firstColumn), message);
                    Assert.IsFalse(IsCellInView(firstItem, lastColumn), message);
                    Assert.IsFalse(IsCellInView(lastItem, firstColumn), message);
                    Assert.IsTrue(IsCellInView(lastItem, lastColumn), message);
                };
            #endregion

            TestPanel.Children.Add(dataGrid);

            EnqueueConditional(delegate { return isLoaded; });

            EnqueueConditional(
                () => dataGrid.CurrentRowIndex == 0 && dataGrid.CurrentColumnIndex == 0
                );

            EnqueueCallback(delegate
            {
                firstColumn = dataGrid.Columns.First();
                lastColumn = dataGrid.Columns.Last();

                dataGrid.SelectedItems.Add(lastItem);
            });

            // starting condition: at the top-left cell
            EnqueueCallback(
                delegate
                {
                    AssertCurrencySelectionUnchanged("currency or selection incorrect at starting condition");
                    AssertTopLeft("failed starting condition");
                }
            );

            // both null
            EnqueueCallback(
                delegate
                {
                    dataGrid.ScrollIntoView(nullItem, nullColumn);
                }
            );

            this.EnqueueYieldThread();

            EnqueueCallback(
                delegate
                {
                    AssertCurrencySelectionUnchanged("currency or selection incorrect at both null");
                    AssertTopLeft("failed top-left test for both null");
                }
            );

            // scroll invalid row = no-op
            EnqueueCallback(
                delegate
                {
                    dataGrid.ScrollIntoView(badItem, lastColumn);
                }
            );

            this.EnqueueYieldThread();

            EnqueueCallback(
                delegate
                {
                    AssertCurrencySelectionUnchanged("currency or selection incorrect at invalid row");
                    AssertTopLeft("failed top-left test for invalid row");
                }
            );

            // scroll invalid column = no-op
            EnqueueCallback(
                delegate
                {
                    dataGrid.ScrollIntoView(lastItem, badColumn);
                }
            );

            this.EnqueueYieldThread();

            EnqueueCallback(
                delegate
                {
                    AssertCurrencySelectionUnchanged("currency or selection incorrect at invalid column");
                    AssertTopLeft("failed top-left test for invalid column");
                }
            );

            // scroll row
            EnqueueCallback(
                delegate
                {
                    dataGrid.ScrollIntoView(lastItem, nullColumn);
                }
            );

            this.EnqueueYieldThread();

            EnqueueCallback(
                delegate
                {
                    AssertCurrencySelectionUnchanged("currency or selection incorrect at scroll row");
                    AssertBottomLeft("failed bottom-left test for scroll row");
                }
            );

            // scroll column
            EnqueueCallback(
                delegate
                {
                    dataGrid.ScrollIntoView(lastItem, lastColumn);
                }
            );

            this.EnqueueYieldThread();

            EnqueueCallback(
                delegate
                {
                    AssertCurrencySelectionUnchanged("currency or selection incorrect at scroll column");
                    AssertBottomRight("failed bottom-right test for scroll column");
                }
            );

            // scroll both
            EnqueueCallback(
                delegate
                {
                    dataGrid.ScrollIntoView(firstItem, firstColumn);
                }
            );

            this.EnqueueYieldThread();

            EnqueueCallback(
                delegate
                {
                    AssertCurrencySelectionUnchanged("currency or selection incorrect at scrolling both row and column");
                    AssertTopLeft("failed top-left test for scrolling both row and column");
                }
            );

            // done
            EnqueueTestComplete();
        }

        [TestMethod]
        [Asynchronous]
        [Description("Tests removing rows out of view")]
        public virtual void RemoveRowsOutOfView()
        {
            // set up datagrid
            IList boundList = new TDataClassSource() as IList;
            if (boundList != null && boundList is INotifyCollectionChanged)
            {
                DataGrid dataGrid = new DataGrid();
                Assert.IsNotNull(dataGrid);
                isLoaded = false;
                dataGrid.Width = 350;
                dataGrid.Height = 100;
                dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
                dataGrid.ItemsSource = boundList;
                TestPanel.Children.Add(dataGrid);
                EnqueueConditional(delegate { return isLoaded; });
                this.EnqueueYieldThread();

                double verticalMax;
                EnqueueCallback(delegate
                {
                    verticalMax = dataGrid.VerticalScrollBar.Maximum;
                    boundList.RemoveAt(0);
                    // Viewport should update since we removed a row
                    Assert.IsTrue(verticalMax > dataGrid.VerticalScrollBar.Maximum, "Vertical ScrollBar did not update after deleting row within view");

                    int lastItemIndex = boundList.Count - 1;
                    if (!dataGrid.DisplayData.IsRowDisplayed(lastItemIndex))
                    {
                        verticalMax = dataGrid.VerticalScrollBar.Maximum;
                        boundList.RemoveAt(lastItemIndex);
                        // Viewport should update since we removed a row
                        Assert.IsTrue(verticalMax > dataGrid.VerticalScrollBar.Maximum, "Vertical ScrollBar did not update after deleting row below");

                        dataGrid.ScrollIntoView(boundList.Last(), null);
                    }
                });

                this.EnqueueYieldThread();
                EnqueueCallback(delegate
                {
                    if (!dataGrid.DisplayData.IsRowDisplayed(0))
                    {
                        verticalMax = dataGrid.VerticalScrollBar.Maximum;
                        boundList.RemoveAt(0);

                        // Viewport should update since we removed a row
                        Assert.IsTrue(verticalMax > dataGrid.VerticalScrollBar.Maximum, "Vertical ScrollBar did not update after deleting row above");
                    }
                });
            }
            EnqueueTestComplete();
        }

        [TestMethod]
        [Asynchronous]
        [Description("Test setting the CellStyle of a DataGrid.")]
        public virtual void SetCellStyle()
        {
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            rowLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });

            EnqueueCallback(delegate
            {
                Style style1 = new Style(typeof(DataGridCell));
                Style style2 = new Style(typeof(DataGridCell));
                dataGrid.Columns[0].CellStyle = style1;
                dataGrid.CellStyle = style2;

                for (int i = dataGrid.DisplayData.FirstDisplayedScrollingRow; i <= dataGrid.DisplayData.LastDisplayedScrollingRow; i++)
                {
                    DataGridRow row = dataGrid.DisplayData.GetDisplayedRow(i);
                    foreach (DataGridCell cell in row.Cells)
                    {
                        if (cell.ColumnIndex == 0)
                        {
                            // Column.CellStyle should take precedence over DataGrid.CellStyle
                            Assert.AreEqual(style1, cell.Style);
                        }
                        else
                        {
                            Assert.AreEqual(style2, cell.Style);
                        }
                    }
                }
            });
            EnqueueTestComplete();
        }

        #region Sorting

        // Helps SortingDataTypes find the correct value of a nested property name (i.e. Address.Street)
        private object InvokePath(object item, string[] propertyNames)
        {
            object newItem = item;
            for (int i = 0; i < propertyNames.Length; i++)
            {
                newItem = newItem.GetType().InvokeMember(propertyNames[i], System.Reflection.BindingFlags.GetProperty, null, newItem, null);
            }
            return newItem;
        }

        // Checks if the property type of a nested property implements IComparable
        private bool IsPropertyTypeIComparable(Type parentType, string propertyPath)
        {
            PropertyInfo propertyInfo;
            Type propertyType = parentType;
            string[] propertyNames = propertyPath.Split(ListCollectionView.PropertyNameSeparator);
            for (int i = 0; i < propertyNames.Length; i++)
            {
                propertyInfo = propertyType.GetProperty(propertyNames[i]);
                if (propertyInfo == null)
                {
                    throw CollectionViewError.ListCollectionView.InvalidPropertyName(propertyNames[i]);
                }
                propertyType = propertyInfo.PropertyType;
            }

            // if the type is Nullable, then we want the non-nullable type
            if (propertyType.IsGenericType && propertyType.GetGenericTypeDefinition() == typeof(Nullable<>))
            {
                propertyType = propertyType.GetGenericArguments()[0];
            }

            return typeof(IComparable).IsAssignableFrom(propertyType);
        }

        [TestMethod]
        [Asynchronous]
        [Description("Tests sorting based on a variety of data types")]
        public void SortingDataTypes()
        {
            // Create a list of DataTypes to check sorting with
            List<DataTypes> boundList = new List<DataTypes>();
            for (int i = 0; i < 25; i++)
            {
                boundList.Add(new DataTypes());
            }
            System.ComponentModel.ICollectionView icv = new ListCollectionView(boundList);

            // Create the DataGrid
            DataGrid dataGrid = new DataGrid();
            dataGrid.TestHook.DelayAutogenerateColumns = false;
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ItemsSource = icv;

            // AutoGenerateColumns added columns for each member of DataTypes,
            // here we are adding a new column for a nested property: DataTypes.AString.Length
            string newPropertyName = "AString.Length";
            DataGridBoundColumn newColumn = new DataGridTextColumn();
            newColumn.Binding = new Binding(newPropertyName);
            newColumn.Header = newPropertyName;
            dataGrid.Columns.Add(newColumn);

            // Add the DataGrid to the test surface so it displays correctly, and then start the testing process
            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            // This action will test the sorting of a single column
            Action<DataGridColumn> EnqueueSortTest = (DataGridColumn col) =>
            {
                // Simulate the clicking of a ColumnHeader, to set the new sort description
                EnqueueCallback(delegate
                {
                    col.HeaderCell.ProcessSort();
                });

                this.EnqueueYieldThread();

                // Sort the original boundList and check it against what's in the Datagrid after clicking ColumnHeaders
                EnqueueCallback(delegate
                {
                    string[] propertyNames = col.GetSortPropertyName().Split('.');
                    int index = 0;
                    foreach (var item in boundList.OrderBy(item => InvokePath(item, propertyNames)))
                    {
                        DataGridRow row = dataGrid.GetRowFromItem(item);
                        // It should be sufficient to test just the visible rows
                        if (row != null)
                        {
                            Assert.AreEqual(index, row.Index);
                            index++;
                        }
                        else
                        {
                            break;
                        }
                    }
                });
            };

            Action<DataGridColumn> EnqueueSortFailTest = (DataGridColumn col) =>
            {
                EnqueueCallback(delegate
                {
                    Common.AssertExpectedException(CollectionViewError.ListCollectionView.FailedToCompareElements(),
                        delegate
                        {
                            col.CanUserSort = true;
                            col.HeaderCell.ProcessSort();
                        }
                    );
                });
            };

            Action<DataGridColumn> EnqueueSortNoOpTest = (DataGridColumn col) =>
            {
                // Simulate the clicking of a ColumnHeader, to set the new sort description
                EnqueueCallback(delegate
                {
                    col.HeaderCell.ProcessSort();
                });
            };

            dataGrid.CanUserSortColumns = false;
            foreach (DataGridColumn column in dataGrid.Columns)
            {
                EnqueueSortNoOpTest(column);
            }

            // Loop through all the columns and test a sort based off of their bound property
            EnqueueCallback(delegate
            {
                dataGrid.CanUserSortColumns = true;
            });
            foreach (DataGridColumn column in dataGrid.Columns)
            {
                if (IsPropertyTypeIComparable(typeof(DataTypes), column.GetSortPropertyName()))
                {
                    // we only want to check sorting if the types are comparable
                    EnqueueSortTest(column);
                }
                else
                {
                    // if the types aren't comparable, we should first check that a user can't sort
                    EnqueueSortNoOpTest(column);

                    // then we should check the case where they think they can sort, but they're wrong
                    EnqueueSortFailTest(column);
                }
            }

            // Finish up the test
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
            });
            EnqueueTestComplete();
        }

        [TestMethod]
        [Asynchronous]
        [Description("Tests sorting states when bound to an ICollectionView.  Doesn't test visuals, nor does it test that the items are actually sorted by the ICV.")]
        public void SortingUI()
        {
            // set up datagrid
            IEnumerable boundList = new TDataClassSource();
            System.ComponentModel.ICollectionView icv = new ListCollectionView(boundList.OfType<TDataClass>().ToList());
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.TestHook.DelayAutogenerateColumns = false;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ItemsSource = icv;

            TestPanel.Children.Add(dataGrid);

            #region Helpers

            Action<DataGridColumn, System.ComponentModel.ListSortDirection> EnqueueSingleSortTest =
                (DataGridColumn col, System.ComponentModel.ListSortDirection direction) =>
                {
                    if (!IsPropertyTypeIComparable(typeof(TDataClass), col.GetSortPropertyName()))
                    {
                        return;
                    }
                    EnqueueCallback(
                        delegate
                        {
                            // scroll the column into view so we can see it
                            //dataGrid.ScrollIntoView(null, col);

                            // call the internal sort method
                            col.HeaderCell.ProcessSort();
                        });

                    // give event a chance to fire
                    this.EnqueueYieldThread();

                    EnqueueCallback(
                        delegate
                        {
                            // check sorting state

                            Assert.AreEqual(1, icv.SortDescriptions.Count);
                            Assert.AreEqual(col.GetSortPropertyName(), icv.SortDescriptions[0].PropertyName);
                            Assert.AreEqual(direction, icv.SortDescriptions[0].Direction);

                            Assert.IsTrue(col.HeaderCell.CurrentSortingState.HasValue);
                            Assert.AreEqual(direction, col.HeaderCell.CurrentSortingState.Value);

                            foreach (DataGridColumn testCol in dataGrid.ColumnsItemsInternal.Where(c => c != col))
                            {
                                Assert.IsFalse(testCol.HeaderCell.CurrentSortingState.HasValue);
                            }
                        });
                };

            Action<DataGridColumn, System.ComponentModel.ListSortDirection, DataGridColumn, System.ComponentModel.ListSortDirection>
                EnqueueMultiSortTest =
                (DataGridColumn colA, System.ComponentModel.ListSortDirection directionA, 
                    DataGridColumn colB, System.ComponentModel.ListSortDirection directionB) =>
                {
                    if (!IsPropertyTypeIComparable(typeof(TDataClass), colA.GetSortPropertyName())
                        || !IsPropertyTypeIComparable(typeof(TDataClass), colB.GetSortPropertyName()))
                    {
                        return;
                    }
                    EnqueueCallback(
                        delegate
                        {
                            // scroll the column into view so we can see it
                            dataGrid.ScrollIntoView(null, colA);
                            
                            // set SortDescriptions manually
                            icv.SortDescriptions.Clear();
                            icv.SortDescriptions.Add(new System.ComponentModel.SortDescription(colA.GetSortPropertyName(), directionA));
                            icv.SortDescriptions.Add(new System.ComponentModel.SortDescription(colB.GetSortPropertyName(), directionB));
                        });

                    // give event a chance to fire
                    this.EnqueueYieldThread();

                    EnqueueCallback(
                        delegate
                        {
                            // check sorting state

                            Assert.AreEqual(2, icv.SortDescriptions.Count);
                            Assert.AreEqual(colA.GetSortPropertyName(), icv.SortDescriptions[0].PropertyName);
                            Assert.AreEqual(directionA, icv.SortDescriptions[0].Direction);

                            Assert.AreEqual(colB.GetSortPropertyName(), icv.SortDescriptions[1].PropertyName);
                            Assert.AreEqual(directionB, icv.SortDescriptions[1].Direction);

                            Assert.IsTrue(colA.HeaderCell.CurrentSortingState.HasValue);
                            Assert.AreEqual(directionA, colA.HeaderCell.CurrentSortingState.Value);

                            Assert.IsTrue(colB.HeaderCell.CurrentSortingState.HasValue);
                            Assert.AreEqual(directionB, colB.HeaderCell.CurrentSortingState.Value);

                            foreach (DataGridColumn testCol in dataGrid.ColumnsItemsInternal.Where(c => c != colA && c != colB))
                            {
                                Assert.IsFalse(testCol.HeaderCell.CurrentSortingState.HasValue);
                            }
                        });
                };
            #endregion

            EnqueueConditional(delegate { return isLoaded; });

            EnqueueCallback(
                delegate
                {
                    // Check the sort descriptions collection
                    Assert.AreEqual(0, icv.SortDescriptions.Count, "ICV SortDescriptions isn't empty");

                    Assert.IsNotNull(dataGrid.DataConnection, "DataConnection is null");
                    Assert.IsNotNull(dataGrid.DataConnection.SortDescriptions, "DataConnection's SortDescs are null");
                    Assert.AreEqual(0, dataGrid.DataConnection.SortDescriptions.Count, "DataConnection's SortDescs are not empty");

                    // verify that the columns report the correct property to sort on

                    foreach (DataGridColumn col in dataGrid.ColumnsItemsInternal)
                    {
                        Assert.IsFalse(String.IsNullOrEmpty(col.GetSortPropertyName()), "Cannot get sort property name");
                    }
                });

            // Test sorting behavior

            DataGridColumn[] singleCols = PickTwoColumns(dataGrid);

            EnqueueSingleSortTest(singleCols[0], System.ComponentModel.ListSortDirection.Ascending);
            EnqueueSingleSortTest(singleCols[0], System.ComponentModel.ListSortDirection.Descending);
            EnqueueSingleSortTest(singleCols[1], System.ComponentModel.ListSortDirection.Ascending);
            EnqueueSingleSortTest(singleCols[1], System.ComponentModel.ListSortDirection.Descending);

            // 


            // Set SortDescriptions manually for multi-sort

            for (int i = 0; i < 3; i++)
            {
                DataGridColumn[] multiCols = PickTwoColumns(dataGrid);

                EnqueueMultiSortTest(multiCols[0], System.ComponentModel.ListSortDirection.Ascending, multiCols[1], System.ComponentModel.ListSortDirection.Ascending);
                EnqueueMultiSortTest(multiCols[0], System.ComponentModel.ListSortDirection.Ascending, multiCols[1], System.ComponentModel.ListSortDirection.Descending);
                EnqueueMultiSortTest(multiCols[0], System.ComponentModel.ListSortDirection.Descending, multiCols[1], System.ComponentModel.ListSortDirection.Ascending);
                EnqueueMultiSortTest(multiCols[0], System.ComponentModel.ListSortDirection.Descending, multiCols[1], System.ComponentModel.ListSortDirection.Descending);
            }

            EnqueueTestComplete();
        }

        #endregion

        [TestMethod]
        [Asynchronous]
        [Description("Test the navigation keys")]
        public virtual void UseNavigationKeys()
        {
            IEnumerable boundList = new TDataClassSource();
            int sizeOfList = boundList.Count();

            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 350;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            int expectedSelectionIndex = -1;

            EnqueueCallback(delegate
            {
                dataGrid.ProcessNextKey();
                dataGrid.ProcessNextKey();

                expectedSelectionIndex = dataGrid.DisplayData.LastDisplayedScrollingRow;

                Assert.AreEqual(expectedSelectionIndex, dataGrid.GetRowFromItem(dataGrid.SelectedItem).Index);
                Assert.AreEqual(0, dataGrid.CurrentColumn.Index);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.ProcessUpKey();

                expectedSelectionIndex -= 1;

                Assert.AreEqual(expectedSelectionIndex, dataGrid.GetRowFromItem(dataGrid.SelectedItem).Index);
                Assert.AreEqual(0, dataGrid.CurrentColumn.Index);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.ProcessPriorKey();
                dataGrid.ProcessPriorKey();

                expectedSelectionIndex = 0;

                Assert.AreEqual(expectedSelectionIndex, dataGrid.GetRowFromItem(dataGrid.SelectedItem).Index);
                Assert.AreEqual(0, dataGrid.CurrentColumn.Index);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.ProcessDownKey();

                expectedSelectionIndex = 1;

                Assert.AreEqual(expectedSelectionIndex, dataGrid.GetRowFromItem(dataGrid.SelectedItem).Index);
                Assert.AreEqual(0, dataGrid.CurrentColumn.Index);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.ProcessRightKey();
                Assert.AreEqual(expectedSelectionIndex, dataGrid.GetRowFromItem(dataGrid.SelectedItem).Index);
                Assert.AreEqual(1, dataGrid.CurrentColumn.Index);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.ProcessEnterKey();

                expectedSelectionIndex += 1;

                Assert.AreEqual(expectedSelectionIndex, dataGrid.GetRowFromItem(dataGrid.SelectedItem).Index);
                Assert.AreEqual(1, dataGrid.CurrentColumn.Index);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.ProcessEndKey();

                // expectedSelectionIndex doesn't change

                Assert.AreEqual(expectedSelectionIndex, dataGrid.GetRowFromItem(dataGrid.SelectedItem).Index);
                Assert.AreEqual(numberOfProperties - 1, dataGrid.CurrentColumn.Index);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                if (numberOfProperties > 1)
                {
                    dataGrid.ProcessLeftKey();
                    Assert.AreEqual(expectedSelectionIndex, dataGrid.GetRowFromItem(dataGrid.SelectedItem).Index);
                    Assert.AreEqual(numberOfProperties - 2, dataGrid.CurrentColumn.Index);
                }
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.ProcessHomeKey();

                // expectedSelectionIndex doesn't change

                Assert.AreEqual(expectedSelectionIndex, dataGrid.GetRowFromItem(dataGrid.SelectedItem).Index);
                Assert.AreEqual(0, dataGrid.CurrentColumn.Index);
            });

            EnqueueCallback(delegate
            {
                //Reset datagrid
                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
            });
            EnqueueTestComplete();
        }

        [TestMethod]
        [Asynchronous]
        [Description("Test that exercises the SelectedItems collection")]
        public virtual void UseSelectedItems()
        {
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                object itemToSelect = boundList.Skip(2).First();
                object itemToFail = boundList.Last();

                Assert.AreNotSame(itemToSelect, itemToFail, "itemToSelect is same as itemToFail -- test invalidated");

                dataGrid.SelectedItems.Add(itemToSelect);

                Assert.IsTrue(dataGrid.SelectedItems.Contains(itemToSelect));

                Common.AssertExpectedException<NotSupportedException>(new NotSupportedException(),
                    delegate
                    {
                        dataGrid.SelectedItems.Insert(0, itemToFail);
                    }
                );
                Common.AssertExpectedException<NotSupportedException>(new NotSupportedException(),
                    delegate
                    {
                        dataGrid.SelectedItems[0] = itemToFail;
                    }
                );

                Assert.AreEqual(2, dataGrid.SelectedItems.Count, "Wrong selected item count after expected exceptions");

                Assert.IsFalse(dataGrid.SelectedItems.IsFixedSize);
                Assert.IsFalse(dataGrid.SelectedItems.IsReadOnly);
                Assert.IsFalse(dataGrid.SelectedItems.IsSynchronized);
                Assert.AreEqual(dataGrid.SelectedItems, dataGrid.SelectedItems.SyncRoot);
            });

            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // Test row state
                foreach (DataGridRow row in boundList.OfType<TDataClass>().Select(item => dataGrid.GetRowFromItem(item)).Where(x => x != null && x.RootElement != null))
                {
                    // selected item

                    Rectangle backgroundRectangle = (Rectangle)row.RootElement.FindName("BackgroundRectangle");
                    bool isSelectedItem = dataGrid.SelectedItems.Contains(row.DataContext);

                    if (isSelectedItem)
                    {
                        Assert.AreEqual(backgroundRectangle.Opacity, 1, "Row #" + row.Index + " : isSelected failed");
                    }
                    else
                    {
                        Assert.AreEqual(backgroundRectangle.Opacity, 0, "Row #" + row.Index + " : !isSelected failed");
                    }

                    // 













                }

                dataGrid.SelectedItems.RemoveAt(0);
                Assert.AreEqual(1, dataGrid.SelectedItems.Count);
            });

            EnqueueCallback(delegate
            {
                //Reset datagrid
                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
            });
            EnqueueTestComplete();
        }

        [TestMethod]
        [Asynchronous]
        [Description("Tests to make sure frozen columns update the horizontal scrollbar as expected.  Also tries set set frozen width beyond the width of the DataGrid. No should be thrown in that case.")]
        [MoonlightBug]
        public virtual void FrozenColumnWidths()
        {
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ItemsSource = boundList;
            // Make sure we can set the FrozenColumnCount before the DataGrid loads
            dataGrid.FrozenColumnCount = 2;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // Make sure the FrozenColumnScrollBarSpacer is updated due to the frozen columns
                Rectangle frozenColumnScrollBarSpacer = FindChild(dataGrid, "FrozenColumnScrollBarSpacer") as Rectangle;
                Assert.IsNotNull(frozenColumnScrollBarSpacer);
                double frozenWidth = frozenColumnScrollBarSpacer.Width;
                Assert.IsTrue(frozenWidth > 0);

                Assert.IsNotNull(dataGrid.HorizontalScrollBar);
                double viewPortSize = dataGrid.HorizontalScrollBar.ViewportSize;
                Assert.IsTrue(viewPortSize > 0);

                // FrozenColumnScrollBarSpacer should get smaller, viewPortSize should get bigger
                dataGrid.FrozenColumnCount = 1;
                Assert.IsTrue(frozenWidth > frozenColumnScrollBarSpacer.Width);

                // 


                dataGrid.HorizontalScrollBarVisibility = ScrollBarVisibility.Visible;

                // Change the minimum column width to a value greater than the DataGrid width.
                dataGrid.MinColumnWidth = 400;
            });

            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                //Reset datagrid
                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
            });

            EnqueueTestComplete();
        }
        
        [TestMethod]
        [Asynchronous]
        [Description("Test to make sure the top left corner and the headers behave as expected")]
        public virtual void CheckTopLeftCorner()
        {
            DataGrid dataGrid = new DataGrid();
            dataGrid.HeadersVisibility = DataGridHeadersVisibility.All;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            DataGridTextColumn textColumn = new DataGridTextColumn();
            textColumn.Header = "foo";
            textColumn.Binding = new Binding();
            dataGrid.Columns.Add(textColumn);
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            isLoaded = false;
            TestPanel.Children.Add(dataGrid);
            FrameworkElement topLeftCornerHeader = null;
            DataGridColumnHeadersPresenter headersPresenter = null;
            double initalCornerWidth = double.NaN;

            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                topLeftCornerHeader = FindChild(dataGrid, "TopLeftCornerHeader") as FrameworkElement;
                // Make sure the topLeftCorner and the column header are visible
                Assert.IsNotNull(topLeftCornerHeader);
                initalCornerWidth = topLeftCornerHeader.Width;
                Assert.IsTrue(initalCornerWidth > 0);
                Assert.IsTrue(topLeftCornerHeader.Visibility == Visibility.Visible);
                Assert.IsTrue(textColumn.HeaderCell.Visibility == Visibility.Visible);
                headersPresenter = textColumn.HeaderCell.Parent as DataGridColumnHeadersPresenter;
                Assert.IsNotNull(headersPresenter);
                Assert.IsTrue(headersPresenter.Visibility == Visibility.Visible);

                dataGrid.HeadersVisibility = DataGridHeadersVisibility.Column;
                Assert.IsTrue(topLeftCornerHeader.Visibility == Visibility.Collapsed);

                dataGrid.HeadersVisibility = DataGridHeadersVisibility.Row;
                Assert.IsTrue(topLeftCornerHeader.Visibility == Visibility.Collapsed);

                dataGrid.HeadersVisibility = DataGridHeadersVisibility.All;
                Assert.IsTrue(topLeftCornerHeader.Visibility == Visibility.Visible);
                
                dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(CheckTopLeftCorner_LoadingRow);
                dataGrid.ItemsSource = new TDataClassSource();
            });
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Check that changing the Header on a Row caused the RowHeaders to expand and 
                // the topLeftCorner to grow
                Assert.IsTrue(topLeftCornerHeader.Width > initalCornerWidth);
                dataGrid.ItemsSource = null;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.IsTrue(topLeftCornerHeader.Visibility == Visibility.Visible);
                Assert.IsTrue(headersPresenter.Visibility == Visibility.Visible);
                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
            });
            EnqueueTestComplete();
        }

        private void CheckTopLeftCorner_LoadingRow(object sender, DataGridRowEventArgs e)
        {
            if (e.Row.GetIndex() == 1)
            {
                e.Row.Header = "Hello World";
                isLoaded = true;
            }
        }
    }
}

