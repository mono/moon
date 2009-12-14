// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Windows.Controls.Data.Test.DataClasses;
using System.Windows.Controls.Data.Test.DataClassSources;
using System.Windows.Controls.Test;
using System.Windows.Data;
using System.Windows.Markup;
using System.Windows.Media;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
    /// <summary>
    /// Tests that require more than zero items in the sequence.  These tests 
    /// are not supported for empty sequences.  For example, editting tests that 
    /// do not (as part of the test) add the item to be editted before editting it.
    /// </summary>
    public partial class DataGridUnitTest_RequireGT0<TDataClass, TDataClassSource> : DataGridUnitTest<TDataClass>
        where TDataClass : new()
        where TDataClassSource : DataClassSource<TDataClass>, new()
    {
        #region CellEditEvents

        public virtual void EditFirstCell(DataGridDelegate subscribeToEvent,
                                          DataGridCellValidateDelegate validateEvent,
                                          DataGridDelegate unsubscribeToEvent)
        {
            //Only works if first column is text which it is for our tests done to simplify test.
            if (properties[0].PropertyType == typeof(string))
            {
                DataGrid dataGrid = new DataGrid();
                string originalValue;
                string updatedValue;
                dataGrid.ItemsSource = null;
                dataGrid.SelectedItems.Clear();
                rowLoaded = false;
                dataGridRow = null;
                isLoaded = false;
                dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
                dataGrid.ColumnWidth = new DataGridLength(50);
                dataGrid.Width = 650;
                dataGrid.Height = 250;
                IEnumerable listTestType = new TDataClassSource();

                TestPanel.Children.Add(dataGrid);
                EnqueueConditional(delegate { return isLoaded; });
                this.EnqueueYieldThread();
                EnqueueCallback(delegate
                {
                    dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);
                    subscribeToEvent(dataGrid);

                    dataGrid.ItemsSource = listTestType;
                    dataGrid.SelectedItem = listTestType.First();
                });
                EnqueueConditional(delegate { return rowLoaded; });

                this.EnqueueYieldThread();
                EnqueueCallback(delegate
                {
                    dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);

                    bool success = dataGrid.BeginEdit();
                    Assert.IsTrue(success, "BeginEdit was not successful");
                });
                this.EnqueueYieldThread();
                //}
                EnqueueCallback(delegate
                {
                    //Set column to valid value
                    Assert.IsTrue(dataGrid.Columns[0].GetCellContent(listTestType.First()) is TextBox, "Not a TextBox");
                    TextBox cell = ((TextBox)dataGrid.CurrentColumn.GetCellContent(listTestType.First()));
                    originalValue = cell.Text;
                    ((TextBox)dataGrid.CurrentColumn.GetCellContent(listTestType.First())).Text = Common.RandomString(10);
                    updatedValue = cell.Text;

                    bool endEditStatus;
                    endEditStatus = dataGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/);
                    Assert.IsTrue(endEditStatus, "EndEdit was not successful");

                    validateEvent(dataGrid, originalValue, updatedValue);

                    unsubscribeToEvent(dataGrid);
                });
                EnqueueTestComplete();
            }
        }

        [TestMethod]
        [Asynchronous]
        [Description("Check for BeginningCellEdit event.")]
        public virtual void BeginningCellEditEvent()
        {
            beginningCellEdit = false;
            EditFirstCell(
                new DataGridDelegate(
                    delegate(DataGrid dataGrid)
                    {
                        dataGrid.BeginningEdit += new EventHandler<DataGridBeginningEditEventArgs>(dataGrid_BeginningCellEdit);
                    }),
                new DataGridCellValidateDelegate(
                    delegate(DataGrid dataGrid, object beforeValue, object afterValue)
                    {
                        Assert.IsTrue(beginningCellEdit, "BeginningCellEdit event was never raised.");
                    }),
                new DataGridDelegate(
                    delegate(DataGrid dataGrid)
                    {
                        dataGrid.BeginningEdit -= new EventHandler<DataGridBeginningEditEventArgs>(dataGrid_BeginningCellEdit);
                    }));
        }
        //[TestMethod]
        //[Asynchronous]
        //[Description("Check for CommittingRowEdit event.")]
        //public virtual void CommittingRowEditEvent()
        //{
        //    committingRowEdit = false;
        //    EditFirstCell(
        //        new DataGridDelegate(
        //            delegate(DataGrid dataGrid)
        //            {
        //                dataGrid.CommittingEdit += new EventHandler<DataGridEndingEditEventArgs>(dataGrid_CommittingEdit);
        //            }),
        //        new DataGridCellValidateDelegate(
        //            delegate(DataGrid dataGrid, object beforeValue, object afterValue)
        //            {
        //                Assert.IsTrue(committingRowEdit, "CommittingRowEdit event was never raised.");
        //            }),
        //        new DataGridDelegate(
        //            delegate(DataGrid dataGrid)
        //            {
        //                dataGrid.CommittingEdit -= new EventHandler<DataGridEndingEditEventArgs>(dataGrid_CommittingEdit);
        //            });
        //}

        //[TestMethod]
        //[Asynchronous]
        //[Description("Check for CommittingCellEdit event.")]
        //public virtual void CommittingCellEditEvent()
        //{
        //    committingCellEdit = false;
        //    EditFirstCell(
        //        new DataGridDelegate(
        //            delegate(DataGrid dataGrid)
        //            {
        //                dataGrid.CommittingEdit += new EventHandler<DataGridEndingEditEventArgs>(dataGrid_CommittingEdit);
        //            }),
        //        new DataGridCellValidateDelegate(
        //            delegate(DataGrid dataGrid, object beforeValue, object afterValue)
        //            {
        //                Assert.IsTrue(committingCellEdit, "CommittingCellEdit event was never raised.");
        //            }),
        //        new DataGridDelegate(
        //            delegate(DataGrid dataGrid)
        //            {
        //                dataGrid.CommittingEdit -= new EventHandler<DataGridEndingEditEventArgs>(dataGrid_CommittingEdit);
        //            });
        //}

        // 













        //    }
        //}

        bool beginningCellEdit = false;
        void dataGrid_BeginningCellEdit(object sender, DataGridBeginningEditEventArgs e)
        {
 	        beginningCellEdit = true;
        }

        #endregion
        
        #region ColumnWidths Test

        [TestMethod]
        [Asynchronous]
        [Description("Test that checks column widths")]
        public virtual void CheckColumnWidths()
        {
            if (properties.Length > 0)
            {
                IEnumerable boundList = new TDataClassSource();
                DataGrid dataGrid = new DataGrid();
                Assert.IsNotNull(dataGrid);
                isLoaded = false;
                rowLoaded = false;
                dataGrid.Width = 350;
                dataGrid.Height = 250;
                dataGrid.ColumnWidth = DataGridLength.SizeToHeader;
                dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);

                // Column1 Bound to properties[0]
                DataGridTextColumn column1 = new DataGridTextColumn();
                column1.Binding = new Binding(properties[0].Name);
                column1.Header = "foo";
                Assert.Equals(0, column1.ActualWidth);
                dataGrid.Columns.Add(column1);
                dataGrid.ItemsSource = boundList;

                TestPanel.Children.Add(dataGrid);
                EnqueueConditional(delegate { return isLoaded; });
                this.EnqueueYieldThread();

                double autoSizedWidth = 0;
                EnqueueCallback(delegate
                {
                    autoSizedWidth = dataGrid.Columns[0].ActualWidth;
                    Assert.IsTrue(autoSizedWidth > 0);
                    dataGrid.ColumnWidth = DataGridLength.SizeToHeader;
                    column1.Header = "the quick brown fox jumped over the lazy dog";
                });
                this.EnqueueYieldThread();
                EnqueueCallback(delegate
                {
                    // Width autosizes to wider header content
                    Assert.IsTrue(dataGrid.Columns[0].ActualWidth > autoSizedWidth);

                    Common.AssertExpectedException(DataGridError.DataGrid.ValueCannotBeSetToNAN("MinColumnWidth"),
                        delegate
                        {
                            dataGrid.MinColumnWidth = double.NaN;
                        }
                    );
                    dataGrid.MinColumnWidth = 4;

                    Common.AssertExpectedException(DataGridError.DataGrid.ValueCannotBeSetToNAN("MaxColumnWidth"),
                        delegate
                        {
                            dataGrid.MaxColumnWidth = double.NaN;
                        }
                    );
                    dataGrid.MaxColumnWidth = 20;
                });
                this.EnqueueYieldThread();
                EnqueueCallback(delegate
                {
                    // Width is coerced by MaxColumnWidth
                    Assert.IsTrue(dataGrid.Columns[0].ActualWidth <= 20);
                    // MinWidth at column level respects MaxColumnWidth at DataGridLevel
                    Common.AssertExpectedException(DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "MinWidth", "MaxWidth"),
                        delegate
                        {
                            dataGrid.Columns[0].MinWidth = 100;
                        }
                    );

                    dataGrid.Columns[0].MinWidth = 15;
                    // MaxWidth at column level respects MinColumnWidth at DataGridLevel
                    Common.AssertExpectedException(DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MaxWidth", "MinWidth"),
                        delegate
                        {
                            dataGrid.Columns[0].MaxWidth = 10;
                        }
                    );
                    dataGrid.MaxColumnWidth = double.PositiveInfinity;
                    dataGrid.ColumnWidth = new DataGridLength(25);
                });
                this.EnqueueYieldThread();
                EnqueueCallback(delegate
                {
                    // Setting Width at DataGrid level
                    Assert.Equals(25, dataGrid.Columns[0].ActualWidth);
                    dataGrid.Columns[0].Width = new DataGridLength(50);
                });
                this.EnqueueYieldThread();
                EnqueueCallback(delegate
                {
                    // Local Width overrides dataGrid.ColumnWidth
                    Assert.Equals(50, dataGrid.Columns[0].ActualWidth);
                });
            }
            EnqueueTestComplete();
        }

        #endregion ColumnWidths Test

        #region Binding Test

        [TestMethod]
        [Asynchronous]
        [Description("Test that exercises the Binding property")]
        public virtual void CheckBinding()
        {
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            rowLoaded = false;
            dataGrid.Height = 150;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);

            // Column1 Bound to properties[0]
            DataGridTextColumn column1 = new DataGridTextColumn();
            column1.Binding = new Binding(properties[0].Name);
            column1.Header = properties[0].Name;
            dataGrid.Columns.Add(column1);

            // Column2 Bound to properties[1]
            DataGridTextColumn column2 = new DataGridTextColumn();
            column2.Binding = new Binding(properties[1].Name);
            column2.Header = properties[1].Name;
            dataGrid.Columns.Add(column2);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });

            EnqueueCallback(delegate
            {
                dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);
                dataGrid.ItemsSource = boundList;
            });
            EnqueueConditional(delegate { return rowLoaded; });

            this.EnqueueYieldThread();

            // Checks the initial Binding
            EnqueueCallback(delegate
            {
                ValidateGridRow(dataGrid, dataGridRow, boundList.First<TDataClass>());
            });

            EnqueueCallback(delegate
            {
                // Change Column2 to bind to properties[0]
                column2.Header = properties[0].Name;
                column2.Binding = new Binding(properties[0].Name);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                ValidateGridRow(dataGrid, dataGridRow, boundList.First<TDataClass>());
            });

            EnqueueCallback(delegate
            {
                // Set Binding to null (shouldn't throw)
                column1.Binding = null;
            });

            EnqueueTestComplete();
        }

        #endregion Binding Test

        #region CurrentColumn Test
        [TestMethod]
        [Asynchronous]
        public virtual void CurrentColumnTest()
        {
            selectionChanged = false;
            rowLoaded = false;
            isLoaded = false;

            TDataClass originalValue = new TDataClass();
            DataGrid dataGrid = new DataGrid();

            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);

            dataGrid.ColumnWidth = new DataGridLength(50);
            dataGrid.Width = 650;
            dataGrid.Height = 250;

            IEnumerable listTestType = new TDataClassSource();

            TestPanel.Children.Add(dataGrid);

            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);
                dataGrid.ItemsSource = listTestType;
            });
            EnqueueConditional(delegate { return rowLoaded; });

            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                // Test 1: set columns collapsed from the first

                dataGrid.CurrentColumn = dataGrid.Columns.First();

                for (int i = 0; i < dataGrid.Columns.Count; i++)
                {
                    Assert.AreEqual(dataGrid.Columns[i], dataGrid.CurrentColumn, "1a: Unexpected CurrentColumn");

                    dataGrid.Columns[i].Visibility = Visibility.Collapsed;

                    if (i < dataGrid.Columns.Count - 1)
                    {
                        Assert.AreEqual(dataGrid.Columns[i + 1], dataGrid.CurrentColumn, "1b: Unexpected CurrentColumn");
                    }
                    else
                    {
                        Assert.IsNull(dataGrid.CurrentColumn);
                    }
                }

                foreach (DataGridBoundColumn column in dataGrid.Columns)
                {
                    column.Visibility = Visibility.Visible;
                }


                // Test 2: set columns collapsed from the last

                dataGrid.CurrentColumn = dataGrid.Columns.Last();

                for (int i = dataGrid.Columns.Count - 1; i >= 0; i--)
                {
                    Assert.AreEqual(dataGrid.Columns[i], dataGrid.CurrentColumn, "2a: Unexpected CurrentColumn");

                    dataGrid.Columns[i].Visibility = Visibility.Collapsed;

                    if (i > 0)
                    {
                        Assert.AreEqual(dataGrid.Columns[i - 1], dataGrid.CurrentColumn, "2b: Unexpected CurrentColumn");
                    }
                    else
                    {
                        Assert.IsNull(dataGrid.CurrentColumn);
                    }
                }

                foreach (DataGridBoundColumn column in dataGrid.Columns)
                {
                    column.Visibility = Visibility.Visible;
                }


                // Test 3: set even columns collapsed

                for (int i = 0; i < dataGrid.Columns.Count; i += 2)
                {
                    if (i == 0)
                    {
                        Assert.AreEqual(dataGrid.Columns[0], dataGrid.CurrentColumn, "3a: Unexpected CurrentColumn");
                    }
                    else
                    {
                        Assert.AreEqual(dataGrid.Columns[1], dataGrid.CurrentColumn, "3b: Unexpected CurrentColumn");
                    }

                    dataGrid.Columns[i].Visibility = Visibility.Collapsed;

                    Assert.AreEqual(dataGrid.Columns[1], dataGrid.CurrentColumn, "3c: Unexpected CurrentColumn");
                }


                // Test 4: cannot set current column to null

                Common.AssertExpectedException(DataGridError.DataGrid.ValueCannotBeSetToNull("value", "CurrentColumn"),
                    delegate
                    {
                        dataGrid.CurrentColumn = null;
                    }
                );


                // Test 5: cannot set current column to a collapsed column

                foreach (DataGridBoundColumn column in dataGrid.Columns)
                {
                    column.Visibility = Visibility.Collapsed;

                    Common.AssertExpectedException(DataGridError.DataGrid.ColumnCannotBeCollapsed(),
                        delegate
                        {
                            dataGrid.CurrentColumn = column;
                        }
                    );
                }


                // Test 6: cannot set current column to a column in another data grid

                Common.AssertExpectedException(DataGridError.DataGrid.ColumnNotInThisDataGrid(),
                    delegate
                    {
                        DataGridTextColumn column = new DataGridTextColumn();
                        dataGrid.CurrentColumn = column;
                    }
                );

            });

            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                //Reset datagrid
                dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);

                rowLoaded = false;

                foreach (DataGridBoundColumn column in dataGrid.Columns)
                {
                    column.Visibility = Visibility.Visible;
                }
            });

            EnqueueTestComplete();
        }
        #endregion

        #region CheckGridDataItem Test
        [TestMethod]
        [Asynchronous]
        public virtual void CheckGridDataItem()
        {
            DataGrid dataGrid = new DataGrid();
            dataGrid.ItemsSource = null;
            dataGrid.SelectedItems.Clear();
            selectionChanged = false;
            rowLoaded = false;
            isLoaded = false;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ColumnWidth = new DataGridLength(50);
            dataGrid.Width = 650;
            dataGrid.Height = 250;
            IEnumerable listTestType = new TDataClassSource();
            TestPanel.Children.Add(dataGrid);

            EnqueueConditional(delegate { return isLoaded; });
            EnqueueCallback(delegate
            {
                dataGrid.SelectionChanged += dataGrid_SelectionChangedFlag;
                dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);
                dataGrid.ItemsSource = listTestType;
            });
            EnqueueConditional(delegate { return rowLoaded; });

            EnqueueConditional(delegate
            {
                dataGrid.SelectedItems.Add(listTestType.First());
                return selectionChanged;
            });

            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                ValidateGridRow(dataGrid, dataGridRow, listTestType.First<TDataClass>());

            });
            EnqueueCallback(delegate
            {
                //Reset datagrid
                dataGrid.SelectionChanged -= dataGrid_SelectionChangedFlag;
                dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);

            });
            EnqueueTestComplete();
        }

        void dataGrid_SelectionChangedFlag(object sender, EventArgs e)
        {
            selectionChanged = true;
        }

        #endregion

        #region HeaderThickness Test

        [TestMethod]
        [Asynchronous]
        [Description("Test that exercises the Row and Column header thickness")]
        [MoonlightBug ("If we call InvalidateArrange/InvalidateMeasure, we don't actually re-arrange or re-measure")]
        public virtual void CheckHeaderThickness()
        {
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.HeadersVisibility = DataGridHeadersVisibility.All;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ItemsSource = boundList;
            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });

            Common.AssertExpectedException(DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "RowHeaderWidth", 4),
                delegate
                {
                    dataGrid.RowHeaderWidth = 3.5;
                }
            );
            Common.AssertExpectedException(DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "RowHeaderWidth", 32768),
                delegate
                {
                    dataGrid.RowHeaderWidth = 32769;
                }
            );
            Common.AssertExpectedException(DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "ColumnHeaderHeight", 4),
                delegate
                {
                    dataGrid.ColumnHeaderHeight = 3.5;
                }
            );
            Common.AssertExpectedException(DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "ColumnHeaderHeight", 32768),
                delegate
                {
                    dataGrid.ColumnHeaderHeight = 32769;
                }
            );

            EnqueueCallback(delegate
            {
                dataGrid.ColumnHeaderHeight = 50;
            });

            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(50, dataGrid.Columns[0].HeaderCell.ActualHeight);
            });

            EnqueueCallback(delegate
            {
                dataGrid.ColumnHeaderHeight = 32767;
            });

            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                Assert.AreEqual(32767, dataGrid.Columns[0].HeaderCell.ActualHeight);
            });

            // Set the ColumnHeaderHeight back otherwise, the RowHeaders will have no available room and no reason to measure
            EnqueueCallback(delegate
            {
                dataGrid.ColumnHeaderHeight = 30;
            });

            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                dataGrid.RowHeaderWidth = 32767;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                DataGridRow row = dataGrid.GetRowFromItem(boundList.First());
                Assert.AreEqual(32767, dataGrid.ActualRowHeaderWidth);
            });

            EnqueueTestComplete();
        }

        #endregion

        #region CheckSelectionChangedEvent

        TDataClass currentlySelectedInstance;
        SelectionChangedEventArgs selectionChangedEventArgs;

        [TestMethod]
        [Asynchronous]
        [Description("Test SelectionChanged event")]
        public virtual void CheckSelectionChangedEvent()
        {
            IEnumerable listTestType = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            Assert.AreEqual(null, currentlySelectedInstance);
            TestPanel.Children.Add(dataGrid);
            numberOfColumnsGenerated = 0;
            dataGrid.SelectionChanged += dataGrid_SelectionChanged;
            dataGrid.ItemsSource = listTestType;
            dataGrid.SelectedItem = listTestType.Last();
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // first item is selected by default
                Assert.AreSame(listTestType.Last(), currentlySelectedInstance);

                //remove all items from selection is selected item null
                dataGrid.SelectedItems.Clear();
                Assert.AreEqual(null, currentlySelectedInstance);

                // AddedItems should be 0, but there should be 1 RemovedItem
                Assert.AreEqual(0, selectionChangedEventArgs.AddedItems.Count);
                Assert.AreEqual(1, selectionChangedEventArgs.RemovedItems.Count);
                Assert.AreEqual(listTestType.Last(), selectionChangedEventArgs.RemovedItems[0]);

                //Try and add something that isn't in the grid, should get argument exception
                // 
                try
                {
                    dataGrid.SelectedItems.Add(new TDataClass());
                    //Since an exception is expected need to assert that it doesn't get here.
                    Assert.Fail();
                }
                catch (ArgumentException)
                {
                    //This is expected
                }
            });
            EnqueueTestComplete();
        }

        void dataGrid_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            currentlySelectedInstance = (TDataClass)(((DataGrid)sender).SelectedItem);
            selectionChangedEventArgs = e;
        }

        #endregion

        #region EditThroughGrid
        public virtual void EditGridCell(bool commitChanges, IEnumerable listTestType)
        {
            TDataClass original = new TDataClass();
            TDataClass updated = new TDataClass();
            DataGrid dataGrid = new DataGrid();
            dataGrid.ItemsSource = null;
            dataGrid.SelectedItems.Clear();
            selectionChanged = false;
            // 

            rowLoaded = false;
            isLoaded = false;
            currentCellChanged = false;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ColumnWidth = new DataGridLength(50);
            dataGrid.Width = 760;
            dataGrid.Height = 250;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                //Save off original values and set starting point for new values
                foreach (PropertyInfo prop in properties)
                {
                    if (prop.CanWrite)
                    {
                        prop.SetValue(original,
                                        prop.GetValue(listTestType.First(), null),
                                        null);
                        prop.SetValue(updated,
                                        prop.GetValue(listTestType.First(), null),
                                        null);
                    }

                }

                dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);
                dataGrid.CurrentCellChanged += new EventHandler<EventArgs>(dataGrid_CurrentCellChanged);
                // 





                dataGrid.ItemsSource = listTestType;
            });
            EnqueueConditional(delegate { return rowLoaded; });

            EnqueueCallback(delegate
            {
                dataGrid.SelectedItem = listTestType.First();
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                foreach (DataGridBoundColumn column in dataGrid.Columns)
                {
                    //Currently we only support updating a few types.
                    PropertyInfo prop = properties.Where(p => column.Header.ToString() == p.Name).First();
                    Type propType = prop.PropertyType;

                    // 
                    // 
                    // 
                    // 
                    // 
                    if (propType.IsNullableType() && !commitChanges)
                    {
                        break;
                    }

                    if (propType.IsValueType ||
                        propType == typeof(String) ||
                        propType == typeof(Decimal) ||
                        propType == typeof(Double) ||
                        propType == typeof(DateTime))
                    {
                        EnqueueColumnEdit(dataGrid, listTestType.Cast<TDataClass>(), column, updated);
                        if (!commitChanges)
                        {
                            EnqueueCancelEdit(dataGrid, DataGridEditingUnit.Cell);
                        }
                    }
                }
                this.EnqueueYieldThread();
                EnqueueCallback(delegate
                {
                    bool endEditStatus;

                    if (commitChanges)
                    {
                        endEditStatus = dataGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/);
                    }
                    else
                    {
                        endEditStatus = dataGrid.CancelEdit(DataGridEditingUnit.Row);
                    }
                    Assert.IsTrue(endEditStatus, "EndEdit was not successful");

                    //Check to see if CurrentCellChangedEvent was raised.
                    Assert.IsTrue(currentCellChanged, "CurrentCellChanged event not raised.");
                });
                this.EnqueueYieldThread();
                EnqueueCallback(delegate
                {
                    //Grid is only reverted if item is an IEditableObject
                    if (commitChanges)
                    {
                        ValidateGridRow(dataGrid, dataGridRow, updated);
                        ValidateGridRow(dataGrid, dataGridRow, listTestType.First<TDataClass>());
                    }
                    else
                    {
                        //

                        if (original is System.ComponentModel.IEditableObject)
                        {
                            ValidateGridRow(dataGrid, dataGridRow, original);
                            ValidateGridRow(dataGrid, dataGridRow, original);
                        }
                        else
                        {
                            // All of the values should have been reverted if we canceled them
                            ValidateGridRow(dataGrid, dataGridRow, original);
                            ValidateGridRow(dataGrid, dataGridRow, listTestType.First<TDataClass>());
                        }
                    }
                    //Reset datagrid
                    dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);
                    dataGrid.CurrentCellChanged -= new EventHandler<EventArgs>(dataGrid_CurrentCellChanged);
                    // 

                });
                EnqueueTestComplete();
            });
        }

        // 






        bool currentCellChanged = false;
        void dataGrid_CurrentCellChanged(object sender, EventArgs e)
        {
            currentCellChanged = true;
        }

        private void EnqueueCancelEdit(DataGrid dataGrid, DataGridEditingUnit editingUnit)
        {
            EnqueueCallback(delegate
            {
                bool success = dataGrid.CancelEdit(editingUnit);
                Assert.IsTrue(success, "CancelEdit was not successful");
            });
            this.EnqueueYieldThread();
        }

        private void EnqueueColumnEdit(DataGrid dataGrid, IEnumerable<TDataClass> listTestType, DataGridBoundColumn column, TDataClass updatedValues)
        {
            EnqueueCallback(delegate
            {
                dataGrid.CurrentColumn = column;
                bool success = dataGrid.Focus();
                Assert.IsTrue(success, "Could not set focus to DataGrid control");
                success = dataGrid.BeginEdit();
                //Begin Edit should be unsuccessful for read only columns and this tests that.
                if (column.IsReadOnly)
                {
                    Assert.IsFalse(success, "BeginEdit should not be successful on IsReadOnly columns");
                }
                else
                {
                    Assert.IsTrue(success, "BeginEdit was not successful");
                }
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                //Gets the cell and property that is being changed
                FrameworkElement cell = column.GetCellContent(listTestType.First());
                PropertyInfo prop = properties.Where(p => column.Header.ToString() == p.Name).First();

                //Checks for the various types of automatic columns
                if (cell is TextBox)
                {
                    //Call RandomTypeValue to get a randomly generated value.
                    MethodInfo mi = (typeof(Common)).GetMethod("RandomTypeValue");
                    Type propType = prop.PropertyType;
                    mi = mi.MakeGenericMethod(propType);
                    object newValue = mi.Invoke(null, null);

                    //Update updatedValues to contain the new value
                    prop.SetValue(updatedValues, newValue, null);

                    ((TextBox)cell).Text = newValue == null ? "" : newValue.ToString() ?? "";
                }
                else if (cell is CheckBox)
                {
                    if (prop.PropertyType == typeof(bool) || prop.PropertyType == typeof(bool?))
                    {
                        bool? newValue = !((CheckBox)cell).IsChecked;
                        ((CheckBox)cell).IsChecked = newValue;

                        //Update updatedValues to contain the new value
                        prop.SetValue(updatedValues, newValue, null);

                    }
                    else
                    {
                        Assert.Fail("Shouldn't bind non-boolean to a check-box.");
                    }
                }
            });
            this.EnqueueYieldThread();
        }

        private IEnumerable GetNonNullListTestType()
        {
            TDataClassSource listTestType = new TDataClassSource();
            TDataClass data = listTestType.First<TDataClass>();
            foreach (PropertyInfo property in properties)
            {
                Type propertyType = property.PropertyType;
                if (propertyType.IsNullableType())
                {
                    //Call RandomTypeValue to get a randomly generated value of the non-nullable type.
                    MethodInfo mi = (typeof(Common)).GetMethod("RandomTypeValue");
                    mi = mi.MakeGenericMethod(propertyType.GetNonNullableType());
                    object newValue = mi.Invoke(null, null);
                    property.SetValue(data, newValue, null);
                }
            }
            return listTestType;
        }

        [TestMethod]
        [Asynchronous]
        public virtual void EditThroughGrid()
        {
            EditGridCell(true, new TDataClassSource());
        }

        [TestMethod]
        [Asynchronous]
        public virtual void EditThroughGrid_CancelEdit()
        {
            EditGridCell(false, new TDataClassSource());
        }

        [TestMethod]
        [Asynchronous]
        public virtual void EditThroughGrid_NonNullEdit()
        {
            EditGridCell(true, GetNonNullListTestType());
        }
        #endregion

        #region EditUnderlyingData Test
        [TestMethod]
        [Asynchronous]
        public virtual void EditUnderlyingData()
        {
            DataGrid dataGrid = new DataGrid();
            dataGrid.ItemsSource = null;
            dataGrid.SelectedItems.Clear();
            selectionChanged = false;
            rowLoaded = false;
            isLoaded = false;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ColumnWidth = new DataGridLength(50);
            dataGrid.Height = 150;
            IEnumerable listTestType = new TDataClassSource();
            
            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            EnqueueCallback(delegate
            {
                dataGrid.SelectionChanged += dataGrid_SelectionChangedFlag;
                dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);
                dataGrid.ItemsSource = listTestType;
            });
            this.EnqueueYieldThread();
            EnqueueConditional(delegate { return rowLoaded; });

            this.EnqueueYieldThread();

            EnqueueConditional(delegate
            {
                dataGrid.SelectedItems.Add(listTestType.First());
                return selectionChanged;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                ValidateGridRow(dataGrid, dataGridRow, listTestType.First<TDataClass>());
                TDataClass oldValues = new TDataClass();
                TDataClass newValues = new TDataClass();

                //Update underlying properties of first data grid row
                foreach (PropertyInfo prop in properties)
                {
                    if (prop.CanWrite)
                    {
                        prop.SetValue(oldValues,
                                        prop.GetValue(listTestType.First(), null),
                                        null);
                        prop.SetValue(listTestType.First(),
                                        prop.GetValue(newValues, null),
                                        null);
                    }
                }


                //The grid will only be updated if INotifyPropertyChanged is implemented on the type.
                if (oldValues is System.ComponentModel.INotifyPropertyChanged)
                {
                    ValidateGridRow(dataGrid, dataGridRow, newValues);
                }
                else
                {
                    //This should work because the grid should be not be updated
                    ValidateGridRow(dataGrid, dataGridRow, oldValues);

                    //This should fail.
                    try
                    {
                        ValidateGridRow(dataGrid, dataGridRow, newValues);
                        Assert.Fail("Grid should not be updated when type doesn't implement INotifyPropertyChanged");
                    }
                    catch (AssertFailedException)
                    {
                        //Do nothing this should fail.
                    }
                }
            });
            EnqueueCallback(delegate
            {
                //Reset DataGrid
                dataGrid.SelectionChanged -= dataGrid_SelectionChangedFlag;
                dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);

            });
            EnqueueTestComplete();
        }
        #endregion

        #region InvalidEdit

        [TestMethod]
        [Asynchronous]
        [Description("Tries to edit integer field in grid invalidly and verifies that the invalid value was reverted")]
        public virtual void InvalidEditIntegerCell()
        {
            PropertyInfo intProperty = null;
            int original = 0;
            DataGrid dataGrid = new DataGrid();
            dataGrid.ItemsSource = null;
            dataGrid.SelectedItems.Clear();
            selectionChanged = false;
            rowLoaded = false;
            dataGridRow = null;
            isLoaded = false;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);
            dataGrid.ColumnWidth = new DataGridLength(50);
            dataGrid.Width = 650;
            dataGrid.Height = 250;
            IEnumerable listTestType = new TDataClassSource();
            dataGrid.ItemsSource = listTestType;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.SelectedItem = listTestType.First();
            });
            this.EnqueueYieldThread();
            EnqueueConditional(delegate { return rowLoaded; });

            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);

                //Find first integer bound column
                var result =
                        (from c in dataGrid.Columns
                         from p in properties
                         where c.Header.ToString() == p.Name &&
                         p.PropertyType == typeof(int)
                         select new { Customer = c, Prop = p }).First();

                dataGrid.CurrentColumn = result.Customer;
                intProperty = result.Prop;
                bool success = dataGrid.BeginEdit();
                Assert.IsTrue(success, "BeginEdit was not successful");
            });
            this.EnqueueYieldThread();
            
            EnqueueCallback(delegate
            {
                //Set column to valid value
                original = (int)(intProperty.GetValue(listTestType.First(), null));
                Assert.IsTrue(dataGrid.CurrentColumn.GetCellContent(listTestType.First()) is TextBox, "Not a TextBox");
               ((TextBox)dataGrid.CurrentColumn.GetCellContent(listTestType.First())).Text = Common.RandomString(10);
                bool endEditStatus = dataGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/);
                Assert.IsTrue(endEditStatus, "EndEdit was not successful");
                Assert.AreEqual(original, intProperty.GetValue(listTestType.First(), null), "Integer was not updated.");
            });
            EnqueueTestComplete();
        }
        
        #endregion

        [TestMethod]
        [Asynchronous]
        [Description("Test that removes columns.")]
        public virtual void RemoveColumns()
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
                // Remove last column in grid
                dataGrid.Columns.RemoveAt(numberOfProperties - 1);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.CurrentColumn = dataGrid.Columns[1];
                dataGrid.BeginEdit();
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Remove edited column
                dataGrid.Columns.RemoveAt(1);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.CurrentColumn = dataGrid.Columns[1];
                dataGrid.BeginEdit();
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Remove column on left of edited column
                dataGrid.Columns.RemoveAt(0);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Remove collapsed column
                dataGrid.Columns[2].Visibility = Visibility.Collapsed;
                dataGrid.Columns.RemoveAt(2);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Remove all columns
                dataGrid.Columns.Clear();
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                //Reset datagrid
                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
            });
            EnqueueTestComplete();
        }

        #region RowBackground Test
        LinearGradientBrush rowBackgroundBrush = new LinearGradientBrush(
                new GradientStopCollection
                {
                    new GradientStop { Offset = 0, Color = Colors.Red },
                    new GradientStop { Offset = 0.3, Color = Colors.Yellow },
                    new GradientStop { Offset = 1.0, Color = Colors.Transparent }
                },
                0.0
                );

        SolidColorBrush rowBackgroundInLoadingBrush = new SolidColorBrush(Colors.Purple);

        LinearGradientBrush alternatingRowBackgroundBrush = new LinearGradientBrush(
                new GradientStopCollection
                {
                    new GradientStop { Offset = 0, Color = Colors.Green },
                    new GradientStop { Offset = 0.6, Color = Colors.Transparent },
                    new GradientStop { Offset = 1.0, Color = Colors.Magenta }
                },
                0.0
                );

        SolidColorBrush defaultAlternateRowBackgroundBrush = new SolidColorBrush(Colors.Transparent);

        [TestMethod]
        [Asynchronous]
        public virtual void RowBackgroundAlternateNull()
        {
            RowBackground(false, rowBackgroundBrush, rowBackgroundBrush, true, null, rowBackgroundBrush);
        }


        [TestMethod]
        [Asynchronous]
        public virtual void RowBackgroundInLoadingAlternateNull()
        {
            RowBackground(true, rowBackgroundBrush, rowBackgroundBrush, true, null, rowBackgroundBrush);
        }

        [TestMethod]
        [Asynchronous]
        public virtual void RowBackgroundAlternateDefault()
        {
            RowBackground(false, rowBackgroundBrush, rowBackgroundBrush, false, null, defaultAlternateRowBackgroundBrush);
        }

        [TestMethod]
        [Asynchronous]
        public virtual void RowBackgroundInLoadingAlternateDefault()
        {
            RowBackground(true, rowBackgroundBrush, rowBackgroundBrush, false, null, rowBackgroundBrush);
        }

        [TestMethod]
        [Asynchronous]
        public virtual void RowBackgroundAlternateSet()
        {
            RowBackground(false, rowBackgroundBrush, rowBackgroundBrush, true, alternatingRowBackgroundBrush, alternatingRowBackgroundBrush);
        }

        [TestMethod]
        [Asynchronous]
        public virtual void RowBackgroundInLoadingAlternateSet()
        {
            RowBackground(true, rowBackgroundBrush, rowBackgroundBrush, true, alternatingRowBackgroundBrush, rowBackgroundBrush);
        }

        void RowBackground(bool setBackgroundInLoading, Brush appliedBackgroundBrush, Brush expectedBackgroundBrush,
            bool setAlternate, Brush appliedAlternatingBackgroundBrush, Brush expectedAlternatingBackgroundBrush)
        {
            DataGrid dataGrid = new DataGrid();
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.Width = 350;
            dataGrid.Height = 250;

            loadRowList = new TDataClassSource();

            if (!setBackgroundInLoading)
            {
                dataGrid.RowBackground = appliedBackgroundBrush;
            }
            else
            {
                this.setBackgroundInLoadingBrush = appliedBackgroundBrush;
                this.setBackgroundInLoading = setBackgroundInLoading;
            }

            if (setAlternate)
            {
                dataGrid.AlternatingRowBackground = appliedAlternatingBackgroundBrush;
            }

            TestPanel.Children.Add(dataGrid);

            EnqueueConditional(delegate { return isLoaded; });
            EnqueueCallback(delegate
            {
                dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowBackgroundTest);
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
                foreach (var item in dataGrid.SelectedItems)
                {
                    var row = dataGrid.GetRowFromItem(item);
                    if (row != null)
                    {
                        var comparisonBrush = row.Index % 2 == 0 ? expectedBackgroundBrush : expectedAlternatingBackgroundBrush;

                        if (row.RootElement != null)
                        {
                            if (row.RootElement.Background is LinearGradientBrush)
                            {
                                Assert.IsInstanceOfType(comparisonBrush, typeof(LinearGradientBrush));

                                LinearGradientBrush brush = (LinearGradientBrush)row.RootElement.Background;
                                LinearGradientBrush compare = comparisonBrush as LinearGradientBrush;

                                Assert.AreEqual(compare.GradientStops.Count, brush.GradientStops.Count);

                                for (int i = 0; i < compare.GradientStops.Count; i++)
                                {
                                    Assert.AreEqual(compare.GradientStops[i].Color, brush.GradientStops[i].Color);
                                    Assert.AreEqual(compare.GradientStops[i].Offset, brush.GradientStops[i].Offset);
                                }
                            }
                            else if (row.RootElement.Background is SolidColorBrush)
                            {
                                Assert.IsInstanceOfType(comparisonBrush, typeof(SolidColorBrush));

                                SolidColorBrush brush = (SolidColorBrush)row.RootElement.Background;
                                SolidColorBrush compare = comparisonBrush as SolidColorBrush;

                                Assert.AreEqual(compare.Color, brush.Color);
                            }
                        }
                    }
                }

                Assert.IsTrue(loadingBackgroundCalled, "LoadingBackground not called");

                //Reset DataGrid
                dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowBackgroundTest);
                dataGrid.ItemsSource = null;
                dataGrid.SelectedItems.Clear();
                selectionChanged = false;
                rowLoaded = false;
                isLoaded = false;
                lastRowLoaded = false;
                loadRowList = null;
                loadingBackgroundCalled = false;
                this.setBackgroundInLoading = false;
                this.setBackgroundInLoadingBrush = null;

            });
            EnqueueTestComplete();
        }

        bool loadingBackgroundCalled = false;
        bool setBackgroundInLoading = false;
        Brush setBackgroundInLoadingBrush = null;

        void dataGrid_LoadingRowBackgroundTest(object sender, DataGridRowEventArgs e)
        {
            loadingBackgroundCalled = true;

            Assert.IsTrue(e.Row != null);

            if (rowLoaded == false)
            {
                rowLoaded = true;
            }

            if (setBackgroundInLoading && setBackgroundInLoadingBrush != null)
            {
                e.Row.Background = setBackgroundInLoadingBrush;
            }

            if (((TDataClass)e.Row.DataContext).Equals(loadRowList.Last()))
            {
                lastRowLoaded = true;
            }
        }
        #endregion RowBackground Test

        #region RowHeight Test
        private DataGridRow _firstRow;
        [TestMethod]
        [Asynchronous]
        [Description("Test that exercises the row height tests.")]
        [MoonlightBug ("This hits a corner case with the Loaded event and so times out waiting for Loaded to fire")]
        public virtual void CheckRowHeights()
        {
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            dataGrid.HeadersVisibility = DataGridHeadersVisibility.All;
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            Common.AssertExpectedException(DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "RowHeight", DataGridRow.DATAGRIDROW_maximumHeight),
                delegate
                {
                    dataGrid.RowHeight = double.PositiveInfinity;
                }
            );
            dataGrid.RowHeight = 50;
            EnqueueCallback(delegate
            {
                dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
                dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_CheckRowHeightsLoadingRow);
                dataGrid.ItemsSource = boundList;
            });

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });

            this.EnqueueYieldThread();

            DataGridCell cell = null;
            double cellHeight = 0;
            double rowHeaderHeight = 0;
            EnqueueCallback(delegate
            {
                Assert.IsNotNull(_firstRow);

                // Check that the DataGrid AutoGrew
                Assert.IsTrue(dataGrid.ActualHeight > 0);

                FrameworkElement content = dataGrid.Columns[0].GetCellContent(_firstRow) as FrameworkElement;
                Assert.IsNotNull(content);
                cell = content.Parent as DataGridCell;
                Assert.IsNotNull(cell);

                cellHeight = cell.ActualHeight;
                rowHeaderHeight = _firstRow.HeaderCell.ActualHeight;
                dataGrid.RowHeight = double.NaN;
                _firstRow.Height = double.NaN;
                _firstRow.HeaderCell.Height = _firstRow.HeaderCell.ActualHeight + 100;

                dataGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(dataGrid_CheckRowHeightsLoadingRow);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.IsTrue(_firstRow.HeaderCell.ActualHeight - rowHeaderHeight == 100);
                // Check that the cell's content grew in response to the RowHeader growing
                Assert.IsTrue(cell.ActualHeight - cellHeight == 100);
                rowHeaderHeight = _firstRow.HeaderCell.ActualHeight;
                cellHeight = cell.ActualHeight;
                // Change the RowHeader's heigth back to NAN so it'll autosize again
                _firstRow.HeaderCell.Height = double.NaN;
                cell.Height = cell.ActualHeight + 100;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.IsTrue(cell.ActualHeight - cellHeight == 100);
                // Check that RowHeader grew in response to the cell growing
                Assert.IsTrue(_firstRow.HeaderCell.ActualHeight - rowHeaderHeight == 100);

                _firstRow = null;
            });
            EnqueueTestComplete();
        }

        private void dataGrid_CheckRowHeightsLoadingRow(object sender, DataGridRowEventArgs e)
        {
            if (e.Row.Index == 0)
            {
                _firstRow = e.Row;
                _firstRow.Height = 100;
                Assert.AreEqual(_firstRow.Height, 100);
            }
        }
        #endregion RowHeight Test

        // 
        [TestMethod]
        [Asynchronous]
        [Description("Test that gives focus to the DataGrid and surrounding controls successively.")]
        public virtual void SetFocusToDataGrid()
        {
            bool success;
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ItemsSource = boundList;

            Button button = new Button();
            button.Width = 50;
            button.Height = 22;
            button.Content = "Btn";

            TextBox textBox = new TextBox();
            textBox.Width = 50;
            textBox.Height = 22;
            textBox.Text = "Txt";

            Grid grid = TestPanel as Grid;
            if (grid != null)
            {
                ColumnDefinition column = new ColumnDefinition();
                column.Width = new GridLength(60);
                grid.ColumnDefinitions.Add(column);

                column = new ColumnDefinition();
                column.Width = new GridLength(360);
                grid.ColumnDefinitions.Add(column);

                column = new ColumnDefinition();
                column.Width = new GridLength(60);
                grid.ColumnDefinitions.Add(column);

                dataGrid.SetValue(Grid.ColumnProperty, 1);
                textBox.SetValue(Grid.ColumnProperty, 2);
            }

            TestPanel.Children.Add(button);
            TestPanel.Children.Add(dataGrid);
            TestPanel.Children.Add(textBox);

            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                success = button.Focus();
                Assert.IsTrue(success);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                success = dataGrid.Focus();
                Assert.IsTrue(success);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                success = textBox.Focus();
                Assert.IsTrue(success);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                success = dataGrid.Focus();
                Assert.IsTrue(success);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                success = dataGrid.BeginEdit();
                Assert.IsTrue(success);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                success = button.Focus();
                Assert.IsTrue(success);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                //Reset datagrid
                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
                TestPanel.Children.Clear();
                if (grid != null)
                {
                    grid.ColumnDefinitions.Clear();
                }
            });
            EnqueueTestComplete();
        }

        [TestMethod]
        [Asynchronous]
        [Description("Test that exercises the GridLinesVisibility property after the DataGrid was populated with rows.")]
        public virtual void SetGridLinesVisibility()
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
                // Make sure top row got selected
                Assert.AreEqual(1, dataGrid.SelectedItems.Count);

                // Make sure default value is DataGridGridLinesVisibility.Vertical
                Assert.AreEqual(DataGridGridLinesVisibility.Vertical, dataGrid.GridLinesVisibility);

                // Change value to DataGridGridLinesVisibility.Horizontal
                dataGrid.GridLinesVisibility = DataGridGridLinesVisibility.Horizontal;
                Assert.AreEqual(DataGridGridLinesVisibility.Horizontal, dataGrid.GridLinesVisibility);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Change value to DataGridGridLinesVisibility.None
                dataGrid.GridLinesVisibility = DataGridGridLinesVisibility.None;
                Assert.AreEqual(DataGridGridLinesVisibility.None, dataGrid.GridLinesVisibility);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Change value to DataGridGridLinesVisibility.Vertical
                dataGrid.GridLinesVisibility = DataGridGridLinesVisibility.Vertical;
                Assert.AreEqual(DataGridGridLinesVisibility.Vertical, dataGrid.GridLinesVisibility);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Change value to DataGridGridLinesVisibility.All
                dataGrid.GridLinesVisibility = DataGridGridLinesVisibility.All;
                Assert.AreEqual(DataGridGridLinesVisibility.All, dataGrid.GridLinesVisibility);
            });
            EnqueueTestComplete();
        }

        [TestMethod]
        [Asynchronous]
        [Description("Test that exercises the DataGridTextColumn's font properties.")]
        public virtual void SetTextBoxColumnFontProperties()
        {
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            rowLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);

            // Column1 Bound to properties[0]
            DataGridTextColumn column1 = new DataGridTextColumn();
            column1.Binding = new Binding(properties[0].Name);
            column1.Header = properties[0].Name;
            dataGrid.Columns.Add(column1);

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });

            EnqueueCallback(delegate
            {
                dataGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(dataGrid_LoadingRowGetRow);
                dataGrid.ItemsSource = boundList;
            });
            EnqueueConditional(delegate { return rowLoaded; });

            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                //Change FontFamily
                column1.FontFamily = new FontFamily("Portable User Interface");
                column1.FontFamily = new FontFamily("Trebuchet MS");

                //Change FontSize
                column1.FontSize = 14.0;
                column1.FontSize = 11.0;

                //Change FontStyle
                column1.FontStyle = FontStyles.Italic;
                column1.FontStyle = FontStyles.Normal;

                //Change FontWeight
                column1.FontWeight = FontWeights.Bold;
                column1.FontWeight = FontWeights.Thin;

                //Change Foreground
                column1.Foreground = new SolidColorBrush(Colors.Magenta);
                column1.Foreground = new SolidColorBrush(Colors.Orange);
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
        [Description("Test that exercises the VerticalScrollBarVisibility property after the DataGrid was populated with rows.")]
        public virtual void SetVerticalScrollBarVisibility()
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
                // Make sure top row got selected
                Assert.AreEqual(1, dataGrid.SelectedItems.Count);

                // Make sure default value is ScrollBarVisibility.Auto
                Assert.AreEqual(ScrollBarVisibility.Auto, dataGrid.VerticalScrollBarVisibility);

                // Change value to ScrollBarVisibility.Disabled
                dataGrid.VerticalScrollBarVisibility = ScrollBarVisibility.Disabled;
                Assert.AreEqual(ScrollBarVisibility.Disabled, dataGrid.VerticalScrollBarVisibility);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Change value to ScrollBarVisibility.Visible
                dataGrid.VerticalScrollBarVisibility = ScrollBarVisibility.Visible;
                Assert.AreEqual(ScrollBarVisibility.Visible, dataGrid.VerticalScrollBarVisibility);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Change value to ScrollBarVisibility.Hidden
                dataGrid.VerticalScrollBarVisibility = ScrollBarVisibility.Hidden;
                Assert.AreEqual(ScrollBarVisibility.Hidden, dataGrid.VerticalScrollBarVisibility);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Change value to ScrollBarVisibility.Auto
                dataGrid.VerticalScrollBarVisibility = ScrollBarVisibility.Auto;
                Assert.AreEqual(ScrollBarVisibility.Auto, dataGrid.VerticalScrollBarVisibility);
            });
            EnqueueCallback(delegate
            {
                // Make the vertical scrollbar appear too
                boundList = new TDataClassSource();
                dataGrid.ItemsSource = boundList;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Change value to ScrollBarVisibility.Disabled
                dataGrid.VerticalScrollBarVisibility = ScrollBarVisibility.Disabled;
                Assert.AreEqual(ScrollBarVisibility.Disabled, dataGrid.VerticalScrollBarVisibility);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Change value to ScrollBarVisibility.Visible
                dataGrid.VerticalScrollBarVisibility = ScrollBarVisibility.Visible;
                Assert.AreEqual(ScrollBarVisibility.Visible, dataGrid.VerticalScrollBarVisibility);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Change value to ScrollBarVisibility.Hidden
                dataGrid.VerticalScrollBarVisibility = ScrollBarVisibility.Hidden;
                Assert.AreEqual(ScrollBarVisibility.Hidden, dataGrid.VerticalScrollBarVisibility);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                // Change value to ScrollBarVisibility.Auto
                dataGrid.VerticalScrollBarVisibility = ScrollBarVisibility.Auto;
                Assert.AreEqual(ScrollBarVisibility.Auto, dataGrid.VerticalScrollBarVisibility);
            });
            EnqueueTestComplete();
        }

        [TestMethod]
        [Asynchronous]
        [Description("Check to see that only a single selection is allowed.")]
        public virtual void SingleSelection()
        {
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            TestPanel.Children.Add(dataGrid);

            dataGrid.ItemsSource = boundList;
            dataGrid.SelectionMode = DataGridSelectionMode.Single;
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {

                dataGrid.SelectedItem = boundList.First();
                Assert.AreEqual(1, dataGrid.SelectedItems.Count, "Should be 1 item in SelectedItems");
                Assert.AreEqual(boundList.First(), dataGrid.SelectedItems[0], "SelectedItem and first item in SelectedItems doesn't match");

                dataGrid.SelectedItem = null;
                Assert.AreEqual(0, dataGrid.SelectedItems.Count, "Should be 0 items in SelectedItems");

                //Test adding a single selection to SelectedItems
                Common.AssertExpectedException(
                    DataGridError.DataGridSelectedItemsCollection.CannotChangeSelectedItemsCollectionInSingleMode(),
                    () =>
                    {
                        dataGrid.SelectedItems.Add(boundList.First());
                    }
                );

                //Expected behavior check to make sure 1 item is selected.
                Assert.AreEqual(0, dataGrid.SelectedItems.Count);
                Assert.AreEqual(null, dataGrid.SelectedItem);
            });
            EnqueueTestComplete();
        }

        [TestMethod]
        [Asynchronous]
        [Description("Test that uses a simple template column.")]
        public virtual void UseTemplateColumn()
        {
            DataGridTemplateColumn templateColumn = new DataGridTemplateColumn();
            IEnumerable boundList = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.RowHeight = 54;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.ItemsSource = boundList;

            TestPanel.Children.Add(dataGrid);
            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                IEnumerator enumerator = boundList.GetEnumerator();
                if (enumerator != null && enumerator.MoveNext() && enumerator.Current is Customer)
                {
                    templateColumn.Header = "Full Name";
                    templateColumn.CellTemplate = (DataTemplate)XamlReader.Load(@"<DataTemplate xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" ><StackPanel Orientation=""Vertical"" Margin=""5, 5, 5, 5""><TextBlock x:Name=""firstName"" FontSize=""11"" Text=""{Binding FirstName}""/><TextBlock x:Name=""lastName"" FontSize=""11"" Text=""{Binding LastName}""/></StackPanel></DataTemplate>");
                    templateColumn.CellEditingTemplate = (DataTemplate)XamlReader.Load(@"<DataTemplate xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" ><StackPanel Orientation=""Vertical"" Margin=""5, 5, 5, 5""><TextBox x:Name=""editedFirstName"" FontSize=""11"" Text=""{Binding FirstName}""/><TextBox x:Name=""editedLastName"" FontSize=""11"" Text=""{Binding LastName}""/></StackPanel></DataTemplate>");
                }
                else
                {
                    templateColumn.Header = "A String And A Boolean";
                    templateColumn.CellTemplate = (DataTemplate)XamlReader.Load(@"<DataTemplate xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" ><StackPanel Orientation=""Vertical"" Margin=""5, 5, 5, 5""><TextBlock x:Name=""aString"" FontSize=""11"" Text=""{Binding AString}""/><TextBlock x:Name=""aBoolean"" FontSize=""11"" Text=""{Binding ABoolean}""/></StackPanel></DataTemplate>");
                    templateColumn.CellEditingTemplate = (DataTemplate)XamlReader.Load(@"<DataTemplate xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" ><StackPanel Orientation=""Vertical"" Margin=""5, 5, 5, 5""><TextBox x:Name=""anEditedString"" FontSize=""11"" Text=""{Binding AString}""/><TextBox x:Name=""anEditedBoolean"" FontSize=""11"" Text=""{Binding ABoolean}""/></StackPanel></DataTemplate>");
                }
                dataGrid.Columns.Insert(0, templateColumn);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.PreparingCellForEdit += new EventHandler<DataGridPreparingCellForEditEventArgs>(DataGrid_PreparingCellForEdit);
                //dataGrid.CommittingEdit += new EventHandler<DataGridEndingEditEventArgs>(DataGrid_CommittingEdit);
                dataGrid.CurrentColumn = dataGrid.Columns[0];
                dataGrid.Focus();
                dataGrid.BeginEdit();
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/);
            });
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                //Reset datagrid
                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
                dataGrid.PreparingCellForEdit -= new EventHandler<DataGridPreparingCellForEditEventArgs>(DataGrid_PreparingCellForEdit);
                //dataGrid.CommittingEdit -= new EventHandler<DataGridEndingEditEventArgs>(DataGrid_CommittingEdit);
            });
            EnqueueTestComplete();
        }
    }
}

