// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Specialized;
using System.Windows.Input;

namespace System.Windows.Controls
{
    public class DataGridCheckBoxColumn : DataGridBoundColumn
    {
        #region Constants

        private const string DATAGRIDCHECKBOXCOLUMN_isThreeStateName = "IsThreeState";
        
        #endregion Constants

        #region Data

        private bool _beganEditWithKeyboard;
        private bool _isThreeState; //
        private CheckBox _currentCheckBox;
        private DataGrid _owningGrid;

        #endregion Data

        public DataGridCheckBoxColumn()
        {
        }

        #region Public Properties

        public bool IsThreeState
        {
            get
            {
                return this._isThreeState;
            }
            set
            {
                if (this._isThreeState != value)
                {
                    this._isThreeState = value;
                    NotifyPropertyChanged(DATAGRIDCHECKBOXCOLUMN_isThreeStateName);
                }
            }
        }

        #endregion Public Properties

        #region Protected Methods

        protected override void CancelCellEdit(FrameworkElement editingElement, object uneditedValue)
        {
            CheckBox editingCheckBox = editingElement as CheckBox;
            if (editingCheckBox != null)
            {
                editingCheckBox.IsChecked = (bool?)uneditedValue;
            }
        }

        protected override FrameworkElement GenerateEditingElement(DataGridCell cell, object dataItem)
        {
            CheckBox checkBox = new CheckBox();
            // 
            checkBox.Margin = new Thickness(0);
            ConfigureCheckBox(checkBox);
            return checkBox;
        }

        protected override FrameworkElement GenerateElement(DataGridCell cell, object dataItem)
        {
            bool isEnabled = false;
            CheckBox checkBoxElement = new CheckBox();
            if (EnsureOwningGrid())
            {
                if (cell.RowIndex != -1 && cell.ColumnIndex != -1 &&
                    cell.RowIndex == this.OwningGrid.CurrentRowIndex &&
                    cell.ColumnIndex == this.OwningGrid.CurrentColumnIndex)
                {
                    isEnabled = true;
                    _currentCheckBox = checkBoxElement;
                }
            }
            checkBoxElement.IsEnabled = isEnabled;
            checkBoxElement.IsHitTestVisible = false;
            ConfigureCheckBox(checkBoxElement);
            return checkBoxElement;
        }

        protected override object PrepareCellForEdit(FrameworkElement editingElement, RoutedEventArgs editingEventArgs)
        {
            CheckBox editingCheckBox = editingElement as CheckBox;
            if (editingCheckBox != null)
            {
                bool? uneditedValue = editingCheckBox.IsChecked;
                MouseButtonEventArgs mouseButtonEventArgs = editingEventArgs as MouseButtonEventArgs;
                bool editValue = false;
                if (mouseButtonEventArgs != null)
                {
                    // Editing was triggered by a mouse click
                    Point position = mouseButtonEventArgs.GetPosition(editingCheckBox);
                    Rect rect = new Rect(0, 0, editingCheckBox.ActualWidth, editingCheckBox.ActualHeight);
                    editValue = rect.Contains(position);
                }
                else if (_beganEditWithKeyboard)
                {
                    // Editing began by a user pressing spacebar
                    editValue = true;
                    _beganEditWithKeyboard = false;
                }
                if (editValue)
                {
                    // User clicked the checkbox itself or pressed space, let's toggle the IsChecked value
                    if (editingCheckBox.IsThreeState)
                    {
                        switch (editingCheckBox.IsChecked)
                        {
                            case false:
                                editingCheckBox.IsChecked = true;
                                break;
                            case true:
                                editingCheckBox.IsChecked = null;
                                break;
                            case null:
                                editingCheckBox.IsChecked = false;
                                break;
                        }
                    }
                    else
                    {
                        editingCheckBox.IsChecked = !editingCheckBox.IsChecked;
                    }
                }
                return uneditedValue;
            }
            return false;
        }
        
        /// <summary>
        /// Called by the DataGrid control when this column asks for its elements to be
        /// updated, because its CheckBoxContent or IsThreeState property changed.
        /// </summary>
        protected internal override void RefreshCellContent(FrameworkElement element, string propertyName)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            CheckBox checkBox = element as CheckBox;
            if (checkBox == null)
            {
                throw DataGridError.DataGrid.ValueIsNotAnInstanceOf("element", typeof(CheckBox));
            }
            if (propertyName == DATAGRIDCHECKBOXCOLUMN_isThreeStateName)
            {
                checkBox.IsThreeState = this.IsThreeState;
            }
            else
            {
                checkBox.IsThreeState = this.IsThreeState;
            }
        }

        #endregion Protected Methods

        #region Private Methods

        private void Columns_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (e.Action == NotifyCollectionChangedAction.Remove && e.OldItems.Contains(this) && _owningGrid != null)
            {
                _owningGrid.Columns.CollectionChanged -= new NotifyCollectionChangedEventHandler(Columns_CollectionChanged);
                _owningGrid.CurrentCellChanged -= new EventHandler<EventArgs>(OwningGrid_CurrentCellChanged);
                _owningGrid.KeyDown -= new KeyEventHandler(OwningGrid_KeyDown);
                _owningGrid.LoadingRow -= new EventHandler<DataGridRowEventArgs>(OwningGrid_LoadingRow);
                _owningGrid = null;
            }
        }

        private void ConfigureCheckBox(CheckBox checkBox)
        {
            checkBox.HorizontalAlignment = HorizontalAlignment.Center;
            checkBox.VerticalAlignment   = VerticalAlignment.Center;
            checkBox.IsThreeState        = this.IsThreeState;
            checkBox.SetBinding(CheckBox.IsCheckedProperty, this.Binding);
        }

        private bool EnsureOwningGrid()
        {
            if (this.OwningGrid != null)
            {
                if (this.OwningGrid != _owningGrid)
                {
                    _owningGrid = this.OwningGrid;
                    _owningGrid.Columns.CollectionChanged += new NotifyCollectionChangedEventHandler(Columns_CollectionChanged);
                    _owningGrid.CurrentCellChanged += new EventHandler<EventArgs>(OwningGrid_CurrentCellChanged);
                    _owningGrid.KeyDown += new KeyEventHandler(OwningGrid_KeyDown);
                    _owningGrid.LoadingRow += new EventHandler<DataGridRowEventArgs>(OwningGrid_LoadingRow);
                }
                return true;
            }
            return false;
        }

        private void OwningGrid_CurrentCellChanged(object sender, EventArgs e)
        {
            if (_currentCheckBox != null)
            {
                _currentCheckBox.IsEnabled = false;
            }
            if (this.OwningGrid != null && this.OwningGrid.CurrentColumn == this)
            {
                DataGridRow row = this.OwningGrid.DisplayData.GetDisplayedRow(this.OwningGrid.CurrentRowIndex);
                CheckBox checkBox = this.GetCellContent(row) as CheckBox;
                if (checkBox != null && !this.OwningGrid.CurrentColumn.IsReadOnly)
                {
                    checkBox.IsEnabled = true;
                }
                _currentCheckBox = checkBox;
            }
        }

        private void OwningGrid_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Space && this.OwningGrid != null &&
                this.OwningGrid.CurrentColumn == this)
            {
                CheckBox checkBox = this.GetCellContent(this.OwningGrid.DisplayData.GetDisplayedRow(this.OwningGrid.CurrentRowIndex)) as CheckBox;
                if (checkBox == _currentCheckBox)
                {
                    _beganEditWithKeyboard = true;
                    this.OwningGrid.BeginEdit();
                    return;
                }
            }
            _beganEditWithKeyboard = false;
        }

        private void OwningGrid_LoadingRow(object sender, DataGridRowEventArgs e)
        {
            if (this.OwningGrid != null)
            {
                CheckBox checkBox = this.GetCellContent(e.Row) as CheckBox;
                if (checkBox != null && !checkBox.IsEnabled)
                {
                    if (this.OwningGrid.CurrentColumnIndex == this.Index && this.OwningGrid.CurrentRowIndex == e.Row.Index)
                    {
                        checkBox.IsEnabled = true;
                        _currentCheckBox = checkBox;
                    }
                    else
                    {
                        checkBox.IsEnabled = false;
                    }
                }
            }
        }

        #endregion Private Methods
    }
}
