// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Collections.Specialized;
using System.Linq;
using System.Reflection;
using System.Windows.Controls.Data.Test.DataClasses;
using System.Windows.Controls.Data.Test.DataClassSources;
using System.Windows.Controls.Test;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
    /// <summary>
    /// Tests that require generics.  These tests are not supported on non-generic 
    /// sequences (IList, IEnumerable, ArrayList), but should be expected to work on 
    /// their generic counterparts (IList<T>, IEnumerable<T>, List<T>, etc).
    /// </summary>
    public partial class DataGridUnitTest_RequireGeneric<TDataClass, TDataClassSource> : DataGridUnitTest<TDataClass>
        where TDataClass : new()
        where TDataClassSource : DataClassSource<TDataClass>, new()
    {
        #region CheckAutoGeneratingColumnEvent

        [TestMethod]
        [Asynchronous]
        [Description("Test AutoGeneratingColumn event")]
        public virtual void CheckAutoGeneratingColumnEvent()
        {
            IEnumerable listTestType = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            numberOfColumnsGenerated = 0;
            dataGrid.AutoGeneratingColumn += new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumn);
            dataGrid.ItemsSource = listTestType;
            TestPanel.Children.Add(dataGrid);
            this.EnqueueYieldThread();
            EnqueueConditional(delegate
            {
                return numberOfColumnsGenerated > 0;
            });
            EnqueueCallback(delegate
            {
                //Make sure the number of columns generated and properties match 
                //the number of times event was called matches
                Assert.AreEqual(numberOfProperties, numberOfColumnsGenerated);
                Assert.AreEqual(dataGrid.Columns.Count, numberOfColumnsGenerated);
                dataGrid.AutoGeneratingColumn -= new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumn);
            });
            EnqueueTestComplete();
        }

        void dataGrid_AutoGeneratingColumn(object sender, DataGridAutoGeneratingColumnEventArgs e)
        {
            numberOfColumnsGenerated++;
        }

        [TestMethod]
        [Asynchronous]
        [Description("Test AutoGeneratingColumn event overrides on column")]
        public virtual void CheckAutoGeneratingColumnEvent_Overrides()
        {
            //Test only makes sense if there are more then 2 properties.
            if (numberOfProperties >= 2)
            {
                IEnumerable listTestType = new TDataClassSource();
                DataGrid dataGrid = new DataGrid();
                Assert.IsNotNull(dataGrid);

                numberOfColumnsGenerated = 0;
                dataGrid.MinColumnWidth = 0;
                columnDisplayIndexChanged = false;
                dataGrid.ItemsSource = listTestType;
                TestPanel.Children.Add(dataGrid);
                this.EnqueueYieldThread();

                EnqueueCallback(delegate
                {
                    //Test changing the Binding DisplayMemberName for all the colums to that of the first property.
                    //reset datagrid
                    dataGrid.ItemsSource = null;
                    numberOfColumnsGenerated = 0;

                    //Test changing the property of the first column header to make sure it works.
                    //reset datagrid
                    dataGrid.ItemsSource = null;
                    numberOfColumnsGenerated = 0;
                    dataGrid.AutoGeneratingColumn += new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnChangeHeader);
                    dataGrid.ItemsSource = listTestType;
                    Assert.AreEqual(dataGrid.Columns.Count, numberOfColumnsGenerated);
                    //Check to see if all the headers have the same name as the first property.
                    var propsWithUpdatedHeader = from p in dataGrid.Columns
                                                 where p.Header.ToString() == properties[0].Name
                                                 select p;
                    Assert.IsTrue(propsWithUpdatedHeader.Count() == numberOfProperties);
                    dataGrid.AutoGeneratingColumn -= new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnChangeHeader);

                    //Check to see if we can reorder the first two columns
                    dataGrid.ItemsSource = null;
                    numberOfColumnsGenerated = 0;
                    dataGrid.AutoGeneratingColumn += new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnChangeIndex);
                    dataGrid.ColumnDisplayIndexChanged += new EventHandler<DataGridColumnEventArgs>(dataGrid_ColumnDisplayIndexChanged);
                    dataGrid.ItemsSource = listTestType;
                    Assert.AreEqual(dataGrid.Columns.Count, numberOfColumnsGenerated);
                    dataGrid.Columns[0].DisplayIndex = 1;
                    //Check to see if display indexes are changed
                    Assert.AreEqual(1, dataGrid.Columns[0].DisplayIndex);
                    Assert.AreEqual(0, dataGrid.Columns[1].DisplayIndex);
                    Assert.AreEqual(1, dataGrid.DisplayData.FirstDisplayedScrollingCol);
                    Assert.IsTrue(columnDisplayIndexChanged, "Column index is incorrect in DataGridColumnEventArgs.");
                    dataGrid.AutoGeneratingColumn -= new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnChangeIndex);
                    dataGrid.ColumnDisplayIndexChanged -= new EventHandler<DataGridColumnEventArgs>(dataGrid_ColumnDisplayIndexChanged);

                    //Check to see if we can make a column invisible in event handler.
                    dataGrid.ItemsSource = null;
                    numberOfColumnsGenerated = 0;
                    dataGrid.AutoGeneratingColumn += new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnChangeIsVisible);
                    dataGrid.ItemsSource = listTestType;
                    Assert.AreEqual(dataGrid.Columns.Count, numberOfColumnsGenerated);
                    //Display indexes stay the same and isVisible = false;
                    Assert.AreEqual(0, dataGrid.Columns[0].DisplayIndex);
                    Assert.AreEqual(Visibility.Collapsed, dataGrid.Columns[0].Visibility);
                    Assert.AreEqual(1, dataGrid.Columns[1].DisplayIndex);
                    dataGrid.AutoGeneratingColumn -= new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnChangeIsVisible);

                    //Check to see if we can make a column invisible in event handler.
                    dataGrid.ItemsSource = null;
                    numberOfColumnsGenerated = 0;
                    dataGrid.AutoGeneratingColumn += new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnChangeWidth);
                    dataGrid.ItemsSource = listTestType;
                    Assert.AreEqual(dataGrid.Columns.Count, numberOfColumnsGenerated);
                    //Check to see if widths are updated.
                    if (dataGrid.Columns.Count >= 1) Assert.AreEqual(20, dataGrid.Columns[0].Width.Value, "First column should have width of 20.");
                    if (dataGrid.Columns.Count >= 2) Assert.AreEqual(originalColumnSize, dataGrid.Columns[1].ActualWidth,
                        String.Format("Second column should have width of {0}.", originalColumnSize));
                    dataGrid.AutoGeneratingColumn -= new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnChangeWidth);
                    dataGrid.ColumnDisplayIndexChanged -= new EventHandler<DataGridColumnEventArgs>(dataGrid_ColumnDisplayIndexChanged);
                });
            }
            EnqueueTestComplete();
        }

        bool columnDisplayIndexChanged = false;
        void dataGrid_ColumnDisplayIndexChanged(object sender, DataGridColumnEventArgs e)
        {
            columnDisplayIndexChanged = true;
        }

        double originalColumnSize = 0;
        void dataGrid_AutoGeneratingColumnChangeHeader(object sender, DataGridAutoGeneratingColumnEventArgs e)
        {
            numberOfColumnsGenerated++;
            e.Column.Header = properties[0].Name;
        }

        void dataGrid_AutoGeneratingColumnChangeIndex(object sender, DataGridAutoGeneratingColumnEventArgs e)
        {
            numberOfColumnsGenerated++;
        }

        void dataGrid_AutoGeneratingColumnChangeWidth(object sender, DataGridAutoGeneratingColumnEventArgs e)
        {
            originalColumnSize = e.Column.ActualWidth;

            if (numberOfColumnsGenerated == 0)
            {
                e.Column.Width = new DataGridLength(20);
            }
            if (numberOfColumnsGenerated == 1)
            {
                e.Column.Width = new DataGridLength(0);

                //Expected no change.
                Assert.AreEqual(originalColumnSize, e.Column.ActualWidth);
            }
            if (numberOfColumnsGenerated == 2)
            {
                try
                {
                    Common.AssertExpectedException(DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "value", 0),
                        delegate
                        {
                            e.Column.Width = new DataGridLength(-1);
                        });
                }
                catch (ArgumentException)
                {
                    //this is what supposed to happen
                }
            }

            numberOfColumnsGenerated++;

        }

        void dataGrid_AutoGeneratingColumnChangeIsVisible(object sender, DataGridAutoGeneratingColumnEventArgs e)
        {
            if (numberOfColumnsGenerated == 0)
            {
                e.Column.Visibility = Visibility.Collapsed;
            }
            numberOfColumnsGenerated++;
        }

        bool columnCancelled = false;
        bool eventHandlerCalled = false;
        [TestMethod]
        [Asynchronous]
        [Description("Test AutoGeneratingColumn event with Cancel=true")]
        public virtual void CheckAutoGeneratingColumnEvent_CancelColumn()
        {
            IEnumerable listTestType = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            TestPanel.Children.Add(dataGrid);
            this.EnqueueYieldThread();

            EnqueueCallback(delegate
            {
                dataGrid.AutoGeneratingColumn += new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnCancelColumn);
                dataGrid.ItemsSource = listTestType;

                //Check to make sure that there are 1 less columns then properties.
                Assert.AreEqual(numberOfProperties - 1, dataGrid.Columns.Count);
                dataGrid.AutoGeneratingColumn -= new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnCancelColumn);

                //Test that all columns can be cancelled.
                dataGrid.AutoGeneratingColumn += new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnCancelAllColumns);
                dataGrid.ItemsSource = null; //This is needed to refresh the grid
                dataGrid.ItemsSource = listTestType;
                Assert.AreEqual(0, dataGrid.Columns.Count);
                dataGrid.AutoGeneratingColumn -= new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnCancelAllColumns);

                dataGrid.AutoGeneratingColumn += new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnNoColumns);
                dataGrid.ItemsSource = null;
                Assert.AreEqual(false, eventHandlerCalled);
                dataGrid.AutoGeneratingColumn -= new EventHandler<DataGridAutoGeneratingColumnEventArgs>(dataGrid_AutoGeneratingColumnNoColumns);
            });
            EnqueueTestComplete();
        }

        void dataGrid_AutoGeneratingColumnCancelColumn(object sender, DataGridAutoGeneratingColumnEventArgs e)
        {
            //Cancel creation of first column
            if (!columnCancelled)
            {
                e.Cancel = true;
                columnCancelled = true;
                return;
            }
        }

        void dataGrid_AutoGeneratingColumnCancelAllColumns(object sender, DataGridAutoGeneratingColumnEventArgs e)
        {
            //Cancel creation of all columns
            e.Cancel = true;
        }

        void dataGrid_AutoGeneratingColumnNoColumns(object sender, DataGridAutoGeneratingColumnEventArgs e)
        {
            eventHandlerCalled = true;
        }

        #endregion

        // 
        [TestMethod]
        [Asynchronous]
        [Description("Test adding items to underlying collection and make sure grid gets updated.")]
        public virtual void InsertItemsToUnderlyingData()
        {
            IEnumerable listSomeType = new TDataClassSource();
            int sizeOfList = listSomeType.Count();
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

                //Can only add items with IList
                if (listSomeType is IList)
                {
                    TDataClass newItem = new TDataClass();
                    ((IList)listSomeType).Insert(2, newItem);

                    if (listSomeType is INotifyCollectionChanged)
                    {
                        //Should be able to select new item
                        dataGrid1.SelectedItems.Add(newItem);
                        Assert.AreEqual(sizeOfList + 1, dataGrid1.SelectedItems.Count,
                            String.Format("Should be {0} items in SelectedItems", sizeOfList + 1));

                        //
                    }
                }
            });
            EnqueueTestComplete();
        }

        [TestMethod]
        [Asynchronous]
        [Description("Test binding same grid to different CLR type.")]
        public virtual void RebindGridToDifferentType()
        {
            IEnumerable listSomeType = new TDataClassSource();

            //Bind to SomeType and make sure that grid columns are correct.
            DataGrid dg = new DataGrid();
            Assert.IsNotNull(dg);
            TestPanel.Children.Add(dg);
            dg.ItemsSource = listSomeType;
            this.EnqueueYieldThread();
            PropertyInfo[] properties = typeof(TDataClass).GetProperties(BindingFlags.Instance | BindingFlags.Public);
            int i = 0;

            EnqueueCallback(delegate
            {
                // 
                Assert.AreEqual(numberOfProperties, dg.Columns.Count);
                foreach (PropertyInfo prop in properties)
                {
                    Assert.AreEqual(prop.Name, dg.Columns[i++].Header);
                }

                //Rebind to Customer List and make sure the grid columns are updated.
                dg.ItemsSource = new CustomerList();
                PropertyInfo[] customerTypeProperties = typeof(Customer).GetProperties(BindingFlags.Instance | BindingFlags.Public);
                i = 0;
                Assert.AreEqual(customerTypeProperties.Length, dg.Columns.Count);
                foreach (PropertyInfo prop in customerTypeProperties)
                {
                    Assert.AreEqual(prop.Name, dg.Columns[i++].Header);
                }
            });
            EnqueueTestComplete();
        }

        // 
        [TestMethod]
        [Asynchronous]
        [Description("Test that exercises the HeadersVisibility property to show/hide the row headers.")]
        public virtual void SetRowHeadersVisibility()
        {
            IEnumerable listSomeType = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 350;
            dataGrid.RowHeaderWidth = 50;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.HeadersVisibility = DataGridHeadersVisibility.Column;
            dataGrid.ItemsSource = listSomeType;
            TestPanel.Children.Add(dataGrid);

            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                IList list = listSomeType as IList;
                if (list != null && list is INotifyCollectionChanged)
                {
                    list.Add(new TDataClass());
                    list.Insert(1, new TDataClass());
                }
                dataGrid.HeadersVisibility = DataGridHeadersVisibility.All;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.AreEqual(50, dataGrid.ActualRowHeaderWidth); 
                Assert.AreEqual(true, dataGrid.AreRowHeadersVisible);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                IList list = listSomeType as IList;
                if (list != null && list is INotifyCollectionChanged)
                {
                    list.Add(new TDataClass());
                    list.Insert(1, new TDataClass());
                }
                dataGrid.HeadersVisibility = DataGridHeadersVisibility.None;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.AreEqual(0, dataGrid.ActualRowHeaderWidth);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                IList list = listSomeType as IList;
                if (list != null && list is INotifyCollectionChanged)
                {
                    list.RemoveAt(3);
                }
                dataGrid.HeadersVisibility = DataGridHeadersVisibility.All;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.AreEqual(50, dataGrid.ActualRowHeaderWidth);
                Assert.AreEqual(true, dataGrid.AreRowHeadersVisible);
            });
            EnqueueCallback(delegate
            {
                //Reset datagrid
                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
            });
            EnqueueTestComplete();
        }

        // 
        [TestMethod]
        [Asynchronous]
        [Description("Test that exercises the HeadersVisibility property to show/hide the column headers.")]
        public virtual void SetColumnHeadersVisibility()
        {
            IEnumerable listSomeType = new TDataClassSource();
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            isLoaded = false;
            dataGrid.Width = 350;
            dataGrid.Height = 350;
            dataGrid.ColumnHeaderHeight = 25;
            dataGrid.Loaded += new RoutedEventHandler(dataGrid_Loaded);
            dataGrid.HeadersVisibility = DataGridHeadersVisibility.Row;
            dataGrid.ItemsSource = listSomeType;
            TestPanel.Children.Add(dataGrid);

            EnqueueConditional(delegate { return isLoaded; });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                IList list = listSomeType as IList;
                if (list != null && list is INotifyCollectionChanged)
                {
                    list.Add(new TDataClass());
                    list.Insert(1, new TDataClass());
                }
                dataGrid.HeadersVisibility = DataGridHeadersVisibility.All;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.AreEqual(25, dataGrid.ColumnHeaders.DesiredSize.Height);
                Assert.AreEqual(Visibility.Visible, dataGrid.ColumnHeaders.Visibility);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                IList list = listSomeType as IList;
                if (list != null && list is INotifyCollectionChanged)
                {
                    list.Add(new TDataClass());
                    list.Insert(1, new TDataClass());
                }
                dataGrid.HeadersVisibility = DataGridHeadersVisibility.None;
                Assert.AreEqual(Visibility.Collapsed, dataGrid.ColumnHeaders.Visibility);
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.AreEqual(Visibility.Collapsed, dataGrid.ColumnHeaders.Visibility);
            });
            EnqueueCallback(delegate
            {
                IList list = listSomeType as IList;
                if (list != null && list is INotifyCollectionChanged)
                {
                    list.RemoveAt(3);
                }
                dataGrid.HeadersVisibility = DataGridHeadersVisibility.Row;
            });
            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.AreEqual(Visibility.Collapsed, dataGrid.ColumnHeaders.Visibility);
            });
            EnqueueCallback(delegate
            {
                //Reset datagrid
                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
            });
            EnqueueTestComplete();
        }

        // 
        [TestMethod]
        [Asynchronous]
        [Description("Test that sets minimum column width")]
        public virtual void SetMinimumColumnWidth()
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
                dataGrid.MinColumnWidth = 10;
                Assert.AreEqual(10, dataGrid.MinColumnWidth);
                dataGrid.Columns[0].MinWidth = 15;
                Assert.AreEqual(15, dataGrid.Columns[0].MinWidth);
                dataGrid.Columns[0].CanUserResize = true;
                Assert.AreEqual(true, dataGrid.Columns[0].CanUserResize);
                dataGrid.Columns[0].CanUserResize = false;
                Assert.AreEqual(false, dataGrid.Columns[0].CanUserResize);

                // DataGrid.MinColumnWidth
                Common.AssertExpectedException(DataGridError.DataGrid.ValueCannotBeSetToNAN("MinColumnWidth"),
                    delegate
                    {
                        dataGrid.MinColumnWidth = double.NaN;
                    }
                );
                Common.AssertExpectedException(DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MinColumnWidth", 0),
                    delegate
                    {
                        dataGrid.MinColumnWidth = -0.5;
                    }
                );
                Common.AssertExpectedException(DataGridError.DataGrid.ValueCannotBeSetToInfinity("MinColumnWidth"),
                    delegate
                    {
                        dataGrid.MinColumnWidth = double.PositiveInfinity;
                    }
                );
                dataGrid.MinColumnWidth = 0;

                // Column.MinWidth
                Common.AssertExpectedException(DataGridError.DataGrid.ValueCannotBeSetToNAN("MinWidth"),
                    delegate
                    {
                        dataGrid.Columns[0].MinWidth = double.NaN;
                    }
                );
                Common.AssertExpectedException(DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MinWidth", 0),
                    delegate
                    {
                        dataGrid.Columns[0].MinWidth = -0.5;
                    }
                );
                Common.AssertExpectedException(DataGridError.DataGrid.ValueCannotBeSetToInfinity("MinWidth"),
                    delegate
                    {
                        dataGrid.Columns[0].MinWidth = double.PositiveInfinity;
                    }
                );
                dataGrid.Columns[0].MinWidth = 0;

                //Reset datagrid
                dataGrid.Loaded -= new RoutedEventHandler(dataGrid_Loaded);
            });
            
            EnqueueTestComplete();
        }

    }
}

