// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;

using System.Windows.Controls.Data.Test.DataClasses;
using System.Windows.Controls.Data.Test.DataClassSources;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Controls.Data.Test;

namespace System.Windows.Controls.Test
{
    #region DataGrid_P0
    /// <summary>
    /// This class runs the DRT checkin tests
    /// </summary>    
    [TestClass]
    public class DataDrtTest_P0 : SilverlightControlTest
    {
        private bool _loaded;
        private bool _preparedCell;

        /// <summary>
        /// P0 Test for the DataGrid
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("P0 test for the DataGrid")]
        public void DataGrid_P0()
        {
            List<Customer> boundList = CreateCustomerList(10);
            DataGrid dataGrid = new DataGrid();
            Assert.IsNotNull(dataGrid);
            dataGrid.Width = 350;
            dataGrid.Height = 250;
            dataGrid.Loaded += new RoutedEventHandler(DataGrid_P0_Loaded);
            dataGrid.ItemsSource = boundList;
            TestPanel.Children.Add(dataGrid);

            EnqueueConditional(delegate { return _loaded; });

            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                Assert.IsTrue(dataGrid.RowCount > 0);
                Assert.IsTrue(dataGrid.Columns.Count > 0);
                Assert.IsTrue(dataGrid.Columns[0].DesiredWidth > 0);
                Assert.IsTrue(dataGrid.Columns[0].ActualWidth > 0);
                Assert.IsNotNull(dataGrid.SelectedItem);
                Assert.IsTrue(dataGrid.SelectedIndex != -1);
                Assert.IsTrue(dataGrid.CellsWidth > 0);
                Assert.IsTrue(dataGrid.CellsHeight > 0);
                Assert.IsTrue(dataGrid.DisplayData.FirstDisplayedScrollingRow != -1);
                Assert.IsTrue(dataGrid.DisplayData.LastDisplayedScrollingRow != -1);
                Assert.IsTrue(dataGrid.DisplayData.FirstDisplayedScrollingCol != -1);
                dataGrid.PreparingCellForEdit += new EventHandler<DataGridPreparingCellForEditEventArgs>(DataGrid_P0_PreparingCellForEdit);
                dataGrid.BeginEdit();
            });

            EnqueueConditional(delegate { return _preparedCell; });

            this.EnqueueYieldThread();
            EnqueueCallback(delegate
            {
                dataGrid.CommitEdit(DataGridEditingUnit.Row, true);
            });

            EnqueueTestComplete();
        }

        private void DataGrid_P0_PreparingCellForEdit(object sender, DataGridPreparingCellForEditEventArgs e)
        {
            _preparedCell = true;
            TextBox textBox = e.EditingElement as TextBox;
            Assert.IsNotNull(textBox);
            Assert.IsTrue(!String.IsNullOrEmpty(textBox.Text), "Editing TextBox has no Text");
        }

        private void DataGrid_P0_Loaded(object sender, RoutedEventArgs e)
        {
            _loaded = true;
        }

        #region Helpers
        private List<Customer> CreateCustomerList(int size)
        {
            List<Customer> list = new List<Customer>();
            for (int i = 0; i < size; i++)
            {
                list.Add(new Customer());
            }
            return list;
        }
        #endregion
    }
    #endregion P0

    #region P1
        
    [TestClass]
    public partial class DataDrtTest_P1_DataTypesINPC_GenericList_5_RequireGT1 : DataGridUnitTest_RequireGT1<DataTypes, GenericList_5<DataTypes>>
    {
        public override void LoadRowDetailsUnloadRowDetails_VisibleAtLoadRow()
        {
        }

        public override void LoadRowUnloadRow()
        {
        }

        public override void RemoveItemsFromUnderlyingData()
        {
        }

        public override void ScrollIntoView()
        {
        }
    }
        
    [TestClass]
    public partial class DataDrtTest_P1_DataTypesINPC_GenericList_1_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypes, GenericList_1<DataTypes>>
    {
    }
        
    [TestClass]
    public partial class DataDrtTest_P1_DataTypesINPC_GenericList_1_Unrestricted : DataGridUnitTest_Unrestricted<DataTypes, GenericList_1<DataTypes>>
    {
    }

    #endregion P1
}
