// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Windows.Data;
using System.Windows.Markup;
using System.Windows.Controls;
 
namespace System.Windows.Controlsb1
{ 
    public class DataGridCheckBoxColumn : DataGridBoundColumnBase
    {
        #region Constants 

        private const string DATAGRIDCHECKBOXCOLUMN_checkBoxContentBindingName = "CheckBoxContentBinding";
        private const string DATAGRIDCHECKBOXCOLUMN_isThreeStateName = "IsThreeState"; 
 
        #endregion Constants
 
        #region Data

        private Binding _checkBoxContentBinding; // 
        private CheckBox _editingCheckBox;
        private bool _isThreeState; //
        // Used to set the Style on our ReadOnlyCheckBox since Styles don't inherit in Silverlight 
        private static Style _readOnlyCheckBoxStyle = InitializeCheckBoxStyle(); 

        #endregion Data 

        public DataGridCheckBoxColumn()
        { 
            this.ElementStyle = _readOnlyCheckBoxStyle;
        }
 
        #region Dependency Properties 

        /* 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

*/ 

        #endregion Dependency Properties
 
        #region Public Properties

        public Binding CheckBoxContentBinding 
        { 
            get
            { 
                return this._checkBoxContentBinding;
            }
            set 
            {
                if (this._checkBoxContentBinding != value)
                { 
                    this._checkBoxContentBinding = value; 
                    UpdateElements(DATAGRIDCHECKBOXCOLUMN_checkBoxContentBindingName);
                } 
            }
        }
 
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
                    UpdateElements(DATAGRIDCHECKBOXCOLUMN_isThreeStateName); 
                } 
            }
        } 

        #endregion Public Properties
 
        #region Public Methods

        public override void CancelCellEdit(object uneditedValue) 
        { 
            if (this._editingCheckBox != null)
            { 
                this._editingCheckBox.IsChecked = (bool?)uneditedValue;
            }
        } 

        public override object PrepareCellEdit(DataGridEditingTriggerInfo editingTriggerInfo)
        { 
            if (this._editingCheckBox != null) 
            {
                bool? uneditedValue = this._editingCheckBox.IsChecked; 
                if (editingTriggerInfo != null &&
                    editingTriggerInfo.MouseButtonEventArgs != null)
                { 
                    // Editing was triggered by a mouse click
                    FrameworkElement checkBox = editingTriggerInfo.MouseButtonEventArgs.Source as FrameworkElement;
                    while (checkBox != null && !(checkBox is CheckBox)) 
                    { 
                        checkBox = checkBox.Parent as FrameworkElement;
                    } 
                    if (checkBox != null)
                    {
                        // User clicked the checkbox itself, let's toggle the IsChecked value 
                        if (this._editingCheckBox.IsThreeState)
                        {
                            switch (this._editingCheckBox.IsChecked) 
                            { 
                                case false:
                                    this._editingCheckBox.IsChecked = true; 
                                    break;
                                case true:
                                    this._editingCheckBox.IsChecked = null; 
                                    break;
                                case null:
                                    this._editingCheckBox.IsChecked = false; 
                                    break; 
                            }
                        } 
                        else
                        {
                            this._editingCheckBox.IsChecked = !this._editingCheckBox.IsChecked; 
                        }
                    }
                } 
                return uneditedValue; 
            }
            return false; 
        }

        /// <summary> 
        /// Called by the DataGrid control when this column asks for its elements to be
        /// updated, because its CheckBoxContentBinding or IsThreeState property changed.
        /// </summary> 
        public override void UpdateElement(FrameworkElement element, string propertyName) 
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
            if (propertyName == DATAGRIDCHECKBOXCOLUMN_checkBoxContentBindingName) 
            {
                checkBox.SetBinding(CheckBox.ContentProperty, this.CheckBoxContentBinding);
            } 
            else if (propertyName == DATAGRIDCHECKBOXCOLUMN_isThreeStateName)
            {
                checkBox.IsThreeState = this.IsThreeState; 
            } 
            else
            { 
                checkBox.SetBinding(CheckBox.ContentProperty, this.CheckBoxContentBinding);
                checkBox.IsThreeState = this.IsThreeState;
            } 
        }

        #endregion Public Methods 
 
        #region Protected Methods
 
        protected override FrameworkElement GenerateEditingElement()
        {
            this._editingCheckBox = new CheckBox(); 
            //
            this._editingCheckBox.Margin = new Thickness(0);
            ConfigureCheckBox(this._editingCheckBox); 
            return this._editingCheckBox; 
        }
 
        protected override FrameworkElement GenerateElement()
        {
            ReadOnlyCheckBox checkBoxElement = new ReadOnlyCheckBox(); 
            ConfigureCheckBox(checkBoxElement);
            return checkBoxElement;
        } 
 
        #endregion Protected Methods
 
        #region Private Methods

        private void ConfigureCheckBox(CheckBox checkBox) 
        {
            checkBox.HorizontalAlignment = HorizontalAlignment.Center;
            checkBox.VerticalAlignment   = VerticalAlignment.Center; 
            checkBox.IsThreeState        = this.IsThreeState; 
            checkBox.SetBinding(CheckBox.IsCheckedProperty, this.DisplayMemberBinding);
 
            if (this.CheckBoxContentBinding != null)
            {
                checkBox.SetBinding(CheckBox.ContentProperty, this.CheckBoxContentBinding); 
            }
        }
 
        private static Style InitializeCheckBoxStyle() 
        {
            // Loads our styles for the ReadOnlyCheckBox 
            string styleXaml = null;
            System.IO.Stream stream = typeof(DataGridCheckBoxColumn).Assembly.GetManifestResourceStream("System.Windows.Controls.DataGrid.DataGridCheckBoxColumn.xaml");
            if (stream != null) 
            {
                styleXaml = new System.IO.StreamReader(stream).ReadToEnd();
                stream.Close(); 
            } 
            return XamlReader.Load(styleXaml) as Style;
        } 

        #endregion Private Methods
 
        #region Nested Types

        private class ReadOnlyCheckBox : CheckBox 
        { 
            //
 
            protected override void OnIndeterminate(RoutedEventArgs e)
            {
                DataGridCell dataGridCell = DataGrid.GetOwningCell(this); 
                if (dataGridCell != null && dataGridCell.RowIndex == -1)
                {
                    return; 
                } 
                base.OnIndeterminate(e);
            } 

            protected override void OnKeyUp(System.Windows.Input.KeyEventArgs e)
            { 
                e.Handled = true;
            }
 
            protected override void OnKeyDown(System.Windows.Input.KeyEventArgs e) 
            {
                e.Handled = true; 
            }

            protected override void OnMouseEnter(System.Windows.Input.MouseEventArgs e) 
            {
                e.Handled = true;
            } 
 
            protected override void OnMouseLeave(System.Windows.Input.MouseEventArgs e)
            { 
                e.Handled = true;
            }
 
            protected override void OnMouseLeftButtonDown(System.Windows.Input.MouseButtonEventArgs e)
            {
                e.Handled = false; 
            } 

            protected override void OnMouseLeftButtonUp(System.Windows.Input.MouseButtonEventArgs e) 
            {
                e.Handled = true;
            } 
        }

        #endregion Nested Types 
    } 
}
