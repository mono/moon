// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis; 
using System.Windows.Input; 
using System.Windows.Media.Animation;
using System.Windows.Shapes; 

namespace System.Windows.Controlsb1
{ 
    [TemplatePart(Name = DATAGRIDCELL_elementRoot, Type = typeof(FrameworkElement))]

    [TemplatePart(Name = DATAGRIDCELL_stateMouseOver, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDCELL_stateMouseOverCurrent, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDCELL_stateMouseOverCurrentFocused, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDCELL_stateMouseOverEditing, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDCELL_stateMouseOverEditingFocused, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDCELL_stateNormal, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDCELL_stateNormalCurrent, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDCELL_stateNormalCurrentFocused, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDCELL_stateNormalEditing, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDCELL_stateNormalEditingFocused, Type = typeof(Storyboard))] 
    public sealed class DataGridCell : ContentControl 
    {
        #region Constants 

        private const string DATAGRIDCELL_elementRoot = "RootElement";
 
        private const string DATAGRIDCELL_stateMouseOver = "MouseOver State";
        private const string DATAGRIDCELL_stateMouseOverCurrent = "MouseOver Unfocused Current State";
        private const string DATAGRIDCELL_stateMouseOverCurrentFocused = "MouseOver Current State"; 
        private const string DATAGRIDCELL_stateMouseOverEditing = "MouseOver Unfocused Editing State"; 
        private const string DATAGRIDCELL_stateMouseOverEditingFocused = "MouseOver Editing State";
        private const string DATAGRIDCELL_stateNormal = "Normal State"; 
        private const string DATAGRIDCELL_stateNormalCurrent = "Unfocused Current State";
        private const string DATAGRIDCELL_stateNormalCurrentFocused = "Normal Current State";
        private const string DATAGRIDCELL_stateNormalEditing = "Unfocused Editing State"; 
        private const string DATAGRIDCELL_stateNormalEditingFocused = "Normal Editing State";

        private const byte DATAGRIDCELL_stateMouseOverCode = 0; 
        private const byte DATAGRIDCELL_stateMouseOverCurrentCode = 1; 
        private const byte DATAGRIDCELL_stateMouseOverCurrentFocusedCode = 2;
        private const byte DATAGRIDCELL_stateMouseOverEditingCode = 3; 
        private const byte DATAGRIDCELL_stateMouseOverEditingFocusedCode = 4;
        private const byte DATAGRIDCELL_stateNormalCode = 5;
        private const byte DATAGRIDCELL_stateNormalCurrentCode = 6; 
        private const byte DATAGRIDCELL_stateNormalCurrentFocusedCode = 7;
        private const byte DATAGRIDCELL_stateNormalEditingCode = 8;
        private const byte DATAGRIDCELL_stateNormalEditingFocusedCode = 9; 
        private const byte DATAGRIDCELL_stateNullCode = 255; 

        private const byte DATAGRIDCELL_stateCount = 10; 

        #endregion Constants
 
        #region Data

        private Line _rightGridline; 
        private FrameworkElement _rootElement; 
        private Duration?[] _storyboardDuration; //
 
        // Static arrays to handle state transitions:
        private static byte[] _idealStateMapping = new byte[] {
            DATAGRIDCELL_stateNormalCode, 
            DATAGRIDCELL_stateNormalCode,
            DATAGRIDCELL_stateMouseOverCode,
            DATAGRIDCELL_stateMouseOverCode, 
            DATAGRIDCELL_stateNormalCurrentCode, 
            DATAGRIDCELL_stateNormalCurrentFocusedCode,
            DATAGRIDCELL_stateMouseOverCurrentCode, 
            DATAGRIDCELL_stateMouseOverCurrentFocusedCode,
            DATAGRIDCELL_stateNullCode,
            DATAGRIDCELL_stateNullCode, 
            DATAGRIDCELL_stateNullCode,
            DATAGRIDCELL_stateNullCode,
            DATAGRIDCELL_stateNormalEditingCode, 
            DATAGRIDCELL_stateNormalEditingFocusedCode, 
            DATAGRIDCELL_stateMouseOverEditingCode,
            DATAGRIDCELL_stateMouseOverEditingFocusedCode 
        };

        private static byte[] _fallbackStateMapping = new byte[] { 
            DATAGRIDCELL_stateNormalCode, //DATAGRIDCELL_stateMouseOverCode's fallback
            DATAGRIDCELL_stateMouseOverCurrentFocusedCode, //DATAGRIDCELL_stateMouseOverCurrentCode's fallback
            DATAGRIDCELL_stateNormalCurrentFocusedCode, //DATAGRIDCELL_stateMouseOverCurrentFocusedCode's fallback 
            DATAGRIDCELL_stateMouseOverEditingFocusedCode, //DATAGRIDCELL_stateMouseOverEditingCode's fallback 
            DATAGRIDCELL_stateNormalEditingFocusedCode, //DATAGRIDCELL_stateMouseOverEditingFocusedCode's fallback
            DATAGRIDCELL_stateNullCode, //DATAGRIDCELL_stateNormalCode's fallback 
            DATAGRIDCELL_stateNormalCurrentFocusedCode, //DATAGRIDCELL_stateNormalCurrentCode's fallback
            DATAGRIDCELL_stateNormalCode, //DATAGRIDCELL_stateNormalCurrentFocusedCode's fallback
            DATAGRIDCELL_stateNormalEditingFocusedCode, //DATAGRIDCELL_stateNormalEditingCode's fallback 
            DATAGRIDCELL_stateNormalCurrentFocusedCode //DATAGRIDCELL_stateNormalEditingFocusedCode's fallback
        };
 
        private static string[] _stateNames = new string[] { 
            DATAGRIDCELL_stateMouseOver,
            DATAGRIDCELL_stateMouseOverCurrent, 
            DATAGRIDCELL_stateMouseOverCurrentFocused,
            DATAGRIDCELL_stateMouseOverEditing,
            DATAGRIDCELL_stateMouseOverEditingFocused, 
            DATAGRIDCELL_stateNormal,
            DATAGRIDCELL_stateNormalCurrent,
            DATAGRIDCELL_stateNormalCurrentFocused, 
            DATAGRIDCELL_stateNormalEditing, 
            DATAGRIDCELL_stateNormalEditingFocused
        }; 

        #endregion Data
 
        public DataGridCell()
        {
            // 
            this.Background = new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Transparent); 
            this._storyboardDuration = new Duration?[DATAGRIDCELL_stateCount];
            this.MouseLeftButtonDown += new MouseButtonEventHandler(DataGridCell_MouseLeftButtonDown); 
            this.MouseEnter += new MouseEventHandler(DataGridCell_MouseEnter);
            this.MouseLeave += new MouseEventHandler(DataGridCell_MouseLeave);
        } 

        #region Public Properties
 
        #endregion Public Properties 

 
        #region Protected Properties

        #endregion Protected Properties 


        #region Internal Properties 
 
        internal int ColumnIndex
        { 
            get
            {
                if (this.OwningColumn == null) 
                {
                    return -1;
                } 
                return this.OwningColumn.Index; 
            }
        } 

        internal bool HasRightGridline
        { 
            get
            {
                return this._rightGridline != null; 
            } 
        }
 
        internal DataGridColumnBase OwningColumn
        {
            get; 
            set;
        }
 
        internal DataGrid OwningGrid 
        {
            get 
            {
                if (this.OwningRow != null && this.OwningRow.OwningGrid != null)
                { 
                    return this.OwningRow.OwningGrid;
                }
                if (this.OwningColumn != null) 
                { 
                    return this.OwningColumn.OwningGrid;
                } 
                return null;
            }
        } 

        internal DataGridRow OwningRow
        { 
            get; 
            set;
        } 

        internal Line RightGridline
        { 
            get
            {
                if (this._rightGridline == null) 
                { 
                    EnsureGridline();
                } 
                return this._rightGridline;
            }
            set 
            {
                this._rightGridline = value;
            } 
        } 

        internal int RowIndex 
        {
            get
            { 
                if (this.OwningRow == null)
                {
                    return -1; 
                } 
                return this.OwningRow.Index;
            } 
        }

        #endregion Internal Properties 


        #region Private Properties 
 
        private bool IsCurrent
        { 
            get
            {
                Debug.Assert(this.OwningGrid != null && this.OwningColumn != null && this.OwningRow != null); 
                return this.OwningGrid.CurrentColumnIndex == this.OwningColumn.Index &&
                       this.OwningGrid.CurrentRowIndex == this.OwningRow.Index;
            } 
        } 

        private bool IsEdited 
        {
            get
            { 
                Debug.Assert(this.OwningGrid != null);
                return this.OwningGrid.EditingRowIndex == this.RowIndex &&
                       this.OwningGrid.EditingColumnIndex == this.ColumnIndex; 
            } 
        }
 
        private bool IsMouseOver
        {
            get 
            {
                return this.OwningRow != null && this.OwningRow.MouseOverColumnIndex == this.ColumnIndex;
            } 
            set 
            {
                Debug.Assert(this.OwningRow != null); 
                if (value != this.IsMouseOver)
                {
                    if (value) 
                    {
                        this.OwningRow.MouseOverColumnIndex = this.ColumnIndex;
                    } 
                    else 
                    {
                        this.OwningRow.MouseOverColumnIndex = null; 
                    }
                }
            } 
        }

        #endregion Private Properties 
 

        #region Public Methods 

        #endregion Public Methods
 

        #region Protected Methods
 
        public override void OnApplyTemplate() 
        {
            base.OnApplyTemplate(); 

            Debug.Assert(this._rootElement == null);
            this._rootElement = GetTemplateChild(DATAGRIDCELL_elementRoot) as FrameworkElement; 
            if (this._rootElement != null)
            {
                ApplyCellState(false /*animate*/); 
            } 
        }
 
        #endregion Protected Methods

 
        #region Internal Methods

        [SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes")] 
        internal void ApplyCellState(bool animate) 
        {
            if (this.OwningGrid == null || this._rootElement == null) 
            {
                return;
            } 

            byte idealStateMappingIndex = 0;
            if (this.IsEdited) 
            { 
                idealStateMappingIndex += 8;
            } 
            if (this.IsCurrent)
            {
                idealStateMappingIndex += 4; 
            }
            if (this.IsMouseOver)
            { 
                idealStateMappingIndex += 2; 
            }
            if (this.OwningGrid.ContainsFocus) 
            {
                idealStateMappingIndex += 1;
            } 

            byte stateCode = _idealStateMapping[idealStateMappingIndex];
            Debug.Assert(stateCode != DATAGRIDCELL_stateNullCode); 
 
            Storyboard storyboard;
            do 
            {
                storyboard = this._rootElement.FindName(_stateNames[stateCode]) as Storyboard;
                if (storyboard == null) 
                {
                    stateCode = _fallbackStateMapping[stateCode];
                } 
            } 
            while (storyboard == null && stateCode != DATAGRIDCELL_stateNullCode);
 
            if (storyboard != null)
            {
                // 
                if (!animate)
                {
                    this._storyboardDuration[stateCode] = DataGrid.ResetStoryboardDuration(storyboard); 
                } 
                else if (this._storyboardDuration[stateCode].HasValue)
                { 
                    DataGrid.RestoreStoryboardDuration(storyboard, this._storyboardDuration[stateCode].Value);
                }
                // 


                try 
                { 
                    storyboard.Begin();
                } 
                catch (Exception)
                {
                } 
            }
        }
 
        internal void EnsureGridline() 
        {
            if (this._rightGridline != null) 
            {
                return;
            } 

            Debug.Assert(this.OwningGrid != null);
            Debug.Assert(this.OwningGrid.GridlinesVisibility == DataGridGridlines.Vertical || this.OwningGrid.GridlinesVisibility == DataGridGridlines.All); 
            Debug.Assert(this.OwningGrid.VerticalGridlinesBrush != null); 
            Debug.Assert(DataGrid.VerticalGridlinesThickness > 0);
            this._rightGridline = new Line(); 
            this._rightGridline.Stroke = this.OwningGrid.VerticalGridlinesBrush;
            this._rightGridline.StrokeThickness = DataGrid.VerticalGridlinesThickness;
            this._rightGridline.SetValue(System.Windows.Controls.Canvas.ZIndexProperty, 1); 
            this.OwningGrid.AddDisplayedVerticalGridline(this._rightGridline);
        }
 
        internal void ResetGridline() 
        {
            this._rightGridline = null; 
        }

        #endregion Internal Methods 


        #region Private Methods 
 
        private void DataGridCell_MouseEnter(object sender, MouseEventArgs e)
        { 
            if (this.OwningRow != null)
            {
                this.IsMouseOver = true; 
            }
        }
 
        private void DataGridCell_MouseLeave(object sender, MouseEventArgs e) 
        {
            if (this.OwningRow != null) 
            {
                this.IsMouseOver = false;
            } 
        }

        private void DataGridCell_MouseLeftButtonDown(object sender, MouseButtonEventArgs e) 
        { 
            if (!e.Handled)
            { 
                // OwningGrid is null for TopLeftHeaderCell and TopRightHeaderCell because they have no OwningRow
                if (this.OwningGrid != null)
                { 
                    if (this.OwningRow != null)
                    {
                        Debug.Assert(sender is DataGridCell); 
                        Debug.Assert(sender == this); 
                        e.Handled = this.OwningGrid.UpdateStateOnMouseLeftButtonDown(e, this.ColumnIndex, this.RowIndex);
                    } 
                    if (this.OwningGrid.IsTabStop)
                    {
                        bool success = this.OwningGrid.Focus(); 
                        Debug.Assert(success);
                    }
                } 
            } 
        }
 
        #endregion Private Methods
    }
} 
