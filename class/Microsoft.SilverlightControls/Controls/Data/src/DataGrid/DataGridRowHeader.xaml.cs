// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis; 
using System.Windows.Input; 
using System.Windows.Media;
using System.Windows.Media.Animation; 
using System.Windows.Shapes;

namespace System.Windows.Controlsb1
{
    [TemplatePart(Name = DATAGRIDROWHEADER_elementRootName, Type = typeof(FrameworkElement))]
 
    [TemplatePart(Name = DATAGRIDROWHEADER_stateNormal, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDROWHEADER_stateNormalCurrentRow, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROWHEADER_stateNormalEditingRow, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDROWHEADER_stateNormalEditingRowFocused, Type = typeof(Storyboard))]

    [TemplatePart(Name = DATAGRIDROWHEADER_stateMouseOver, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDROWHEADER_stateMouseOverCurrentRow, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROWHEADER_stateMouseOverEditingRow, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROWHEADER_stateMouseOverEditingRowFocused, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDROWHEADER_stateMouseOverSelected, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDROWHEADER_stateMouseOverSelectedFocused, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRow, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRowFocused, Type = typeof(Storyboard))]

    [TemplatePart(Name = DATAGRIDROWHEADER_stateSelected, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDROWHEADER_stateSelectedCurrentRow, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROWHEADER_stateSelectedCurrentRowFocused, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROWHEADER_stateSelectedFocused, Type = typeof(Storyboard))] 
 
    /*
    [TemplatePart(Name = "STATE_ValidRow", Type = typeof(Storyboard))] 
    [TemplatePart(Name = "STATE_InvalidRow", Type = typeof(Storyboard))]
    */
    public partial class DataGridRowHeader : ContentControl 
    {
        #region Constants
 
        private const string DATAGRIDROWHEADER_elementRootName = "RootElement"; 
        private const double DATAGRIDROWHEADER_separatorThickness = 1;
 
        private const string DATAGRIDROWHEADER_stateMouseOver = "MouseOver State";
        private const string DATAGRIDROWHEADER_stateMouseOverCurrentRow = "MouseOver CurrentRow State";
        private const string DATAGRIDROWHEADER_stateMouseOverEditingRow = "MouseOver Unfocused EditingRow State"; 
        private const string DATAGRIDROWHEADER_stateMouseOverEditingRowFocused = "MouseOver EditingRow State";
        private const string DATAGRIDROWHEADER_stateMouseOverSelected = "MouseOver Unfocused Selected State";
        private const string DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRow = "MouseOver Unfocused CurrentRow Selected State"; 
        private const string DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRowFocused = "MouseOver CurrentRow Selected State"; 
        private const string DATAGRIDROWHEADER_stateMouseOverSelectedFocused = "MouseOver Selected State";
        private const string DATAGRIDROWHEADER_stateNormal = "Normal State"; 
        private const string DATAGRIDROWHEADER_stateNormalCurrentRow = "Normal CurrentRow State";
        private const string DATAGRIDROWHEADER_stateNormalEditingRow = "Unfocused EditingRow State";
        private const string DATAGRIDROWHEADER_stateNormalEditingRowFocused = "Normal EditingRow State"; 
        private const string DATAGRIDROWHEADER_stateSelected = "Unfocused Selected State";
        private const string DATAGRIDROWHEADER_stateSelectedCurrentRow = "Unfocused CurrentRow Selected State";
        private const string DATAGRIDROWHEADER_stateSelectedCurrentRowFocused = "Normal CurrentRow Selected State"; 
        private const string DATAGRIDROWHEADER_stateSelectedFocused = "Normal Selected State"; 

        private const byte DATAGRIDROWHEADER_stateMouseOverCode = 0; 
        private const byte DATAGRIDROWHEADER_stateMouseOverCurrentRowCode = 1;
        private const byte DATAGRIDROWHEADER_stateMouseOverEditingRowCode = 2;
        private const byte DATAGRIDROWHEADER_stateMouseOverEditingRowFocusedCode = 3; 
        private const byte DATAGRIDROWHEADER_stateMouseOverSelectedCode = 4;
        private const byte DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRowCode = 5;
        private const byte DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRowFocusedCode = 6; 
        private const byte DATAGRIDROWHEADER_stateMouseOverSelectedFocusedCode = 7; 
        private const byte DATAGRIDROWHEADER_stateNormalCode = 8;
        private const byte DATAGRIDROWHEADER_stateNormalCurrentRowCode = 9; 
        private const byte DATAGRIDROWHEADER_stateNormalEditingRowCode = 10;
        private const byte DATAGRIDROWHEADER_stateNormalEditingRowFocusedCode = 11;
        private const byte DATAGRIDROWHEADER_stateSelectedCode = 12; 
        private const byte DATAGRIDROWHEADER_stateSelectedCurrentRowCode = 13;
        private const byte DATAGRIDROWHEADER_stateSelectedCurrentRowFocusedCode = 14;
        private const byte DATAGRIDROWHEADER_stateSelectedFocusedCode = 15; 
        private const byte DATAGRIDROWHEADER_stateNullCode = 255; 

        private const byte DATAGRIDROWHEADER_stateCount = 16; 

        private static byte[] _fallbackStateMapping = new byte[] {
            DATAGRIDROWHEADER_stateNormalCode, 
            DATAGRIDROWHEADER_stateNormalCurrentRowCode,
            DATAGRIDROWHEADER_stateMouseOverEditingRowFocusedCode,
            DATAGRIDROWHEADER_stateNormalEditingRowFocusedCode, 
            DATAGRIDROWHEADER_stateMouseOverSelectedFocusedCode, 
            DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRowFocusedCode,
            DATAGRIDROWHEADER_stateSelectedFocusedCode, 
            DATAGRIDROWHEADER_stateSelectedFocusedCode,
            DATAGRIDROWHEADER_stateNullCode,
            DATAGRIDROWHEADER_stateNormalCode, 
            DATAGRIDROWHEADER_stateNormalEditingRowFocusedCode,
            DATAGRIDROWHEADER_stateSelectedCurrentRowFocusedCode,
            DATAGRIDROWHEADER_stateSelectedFocusedCode, 
            DATAGRIDROWHEADER_stateSelectedCurrentRowFocusedCode, 
            DATAGRIDROWHEADER_stateNormalCurrentRowCode,
            DATAGRIDROWHEADER_stateNormalCode, 
        };

        private static byte[] _idealStateMapping = new byte[] { 
            DATAGRIDROWHEADER_stateNormalCode,
            DATAGRIDROWHEADER_stateNormalCode,
            DATAGRIDROWHEADER_stateMouseOverCode, 
            DATAGRIDROWHEADER_stateMouseOverCode, 
            DATAGRIDROWHEADER_stateNullCode,
            DATAGRIDROWHEADER_stateNullCode, 
            DATAGRIDROWHEADER_stateNullCode,
            DATAGRIDROWHEADER_stateNullCode,
            DATAGRIDROWHEADER_stateSelectedCode, 
            DATAGRIDROWHEADER_stateSelectedFocusedCode,
            DATAGRIDROWHEADER_stateMouseOverSelectedCode,
            DATAGRIDROWHEADER_stateMouseOverSelectedFocusedCode, 
            DATAGRIDROWHEADER_stateNormalEditingRowCode, 
            DATAGRIDROWHEADER_stateNormalEditingRowFocusedCode,
            DATAGRIDROWHEADER_stateMouseOverEditingRowCode, 
            DATAGRIDROWHEADER_stateMouseOverEditingRowFocusedCode,
            DATAGRIDROWHEADER_stateNormalCurrentRowCode,
            DATAGRIDROWHEADER_stateNormalCurrentRowCode, 
            DATAGRIDROWHEADER_stateMouseOverCurrentRowCode,
            DATAGRIDROWHEADER_stateMouseOverCurrentRowCode,
            DATAGRIDROWHEADER_stateNullCode, 
            DATAGRIDROWHEADER_stateNullCode, 
            DATAGRIDROWHEADER_stateNullCode,
            DATAGRIDROWHEADER_stateNullCode, 
            DATAGRIDROWHEADER_stateSelectedCurrentRowCode,
            DATAGRIDROWHEADER_stateSelectedCurrentRowFocusedCode,
            DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRowCode, 
            DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRowFocusedCode,
            DATAGRIDROWHEADER_stateNormalEditingRowCode,
            DATAGRIDROWHEADER_stateNormalEditingRowFocusedCode, 
            DATAGRIDROWHEADER_stateMouseOverEditingRowCode, 
            DATAGRIDROWHEADER_stateMouseOverEditingRowFocusedCode
        }; 

        private static string[] _stateNames = new string[]
        { 
            DATAGRIDROWHEADER_stateMouseOver,
            DATAGRIDROWHEADER_stateMouseOverCurrentRow,
            DATAGRIDROWHEADER_stateMouseOverEditingRow, 
            DATAGRIDROWHEADER_stateMouseOverEditingRowFocused, 
            DATAGRIDROWHEADER_stateMouseOverSelected,
            DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRow, 
            DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRowFocused,
            DATAGRIDROWHEADER_stateMouseOverSelectedFocused,
            DATAGRIDROWHEADER_stateNormal, 
            DATAGRIDROWHEADER_stateNormalCurrentRow,
            DATAGRIDROWHEADER_stateNormalEditingRow,
            DATAGRIDROWHEADER_stateNormalEditingRowFocused, 
            DATAGRIDROWHEADER_stateSelected, 
            DATAGRIDROWHEADER_stateSelectedCurrentRow,
            DATAGRIDROWHEADER_stateSelectedCurrentRowFocused, 
            DATAGRIDROWHEADER_stateSelectedFocused
        };
 
        #endregion Constants

        #region Data 
 
        private FrameworkElement _rootElement;
        private Duration?[] _storyboardDuration; // 

        #endregion Data
 
        public DataGridRowHeader()
        {
            this._storyboardDuration = new Duration?[DATAGRIDROWHEADER_stateCount]; 
            this.MouseLeftButtonDown += new MouseButtonEventHandler(DataGridRowHeader_MouseLeftButtonDown); 
            this.MouseEnter += new MouseEventHandler(DataGridRowHeader_MouseEnter);
            this.MouseLeave += new MouseEventHandler(DataGridRowHeader_MouseLeave); 
        }

        #region Dependency Properties 

        #region SeparatorBrush
 
        /// <summary> 
        /// Gets or sets a brush used by the header separator line
        /// </summary> 
        public Brush SeparatorBrush
        {
            get { return GetValue(SeparatorBrushProperty) as Brush; } 
            set { SetValue(SeparatorBrushProperty, value); }
        }
 
        public static readonly DependencyProperty SeparatorBrushProperty = 
            DependencyProperty.Register("SeparatorBrush",
                typeof(Brush), 
                typeof(DataGridRowHeader),
                null);
 
        #endregion SeparatorBrush

        #region SeparatorVisibility 
 
        /// <summary>
        /// Gets or sets the visibility of the header separator line 
        /// </summary>
        public Visibility SeparatorVisibility
        { 
            get { return (Visibility)GetValue(SeparatorVisibilityProperty); }
            set { SetValue(SeparatorVisibilityProperty, value); }
        } 
 
        public static readonly DependencyProperty SeparatorVisibilityProperty =
            DependencyProperty.Register("SeparatorVisibility", 
                typeof(Visibility),
                typeof(DataGridRowHeader),
                null); 

        #endregion SeparatorVisibility
 
        #endregion Dependency Properties 

 
        #region Public Properties

        #endregion Public Properties 


        #region Protected Properties 
 
        #endregion Protected Properties
 

        #region Internal Properties
 
        internal DataGrid OwningGrid
        {
            get 
            { 
                if (this.OwningRow != null && this.OwningRow.OwningGrid != null)
                { 
                    return this.OwningRow.OwningGrid;
                }
                return null; 
            }
        }
 
        internal DataGridRow OwningRow 
        {
            get; 
            set;
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

        #endregion Private Properties 


        #region Public Methods 
 
        #endregion Public Methods
 

        #region Protected Methods
 
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate(); 
 
            this._rootElement = GetTemplateChild(DATAGRIDROWHEADER_elementRootName) as FrameworkElement;
            if (this._rootElement != null) 
            {
                ApplyRowStatus(false /*animate*/);
            } 
        }

        #endregion Protected Methods 
 

        #region Internal Methods 

        [SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes")]
        internal void ApplyRowStatus(bool animate) 
        {
            if (this._rootElement != null && this.OwningRow != null && this.OwningGrid != null)
            { 
                byte idealStateMappingIndex = 0; 
                // Current
                if (this.OwningGrid.CurrentRowIndex == this.OwningRow.Index) 
                {
                    idealStateMappingIndex += 16;
                } 
                if (this.OwningRow.IsSelected)
                {
                    idealStateMappingIndex += 8; 
                } 
                if (this.OwningRow.IsEditing)
                { 
                    idealStateMappingIndex += 4;
                }
                if (this.OwningRow.IsMouseOver) 
                {
                    idealStateMappingIndex += 2;
                } 
                if (this.OwningGrid.ContainsFocus) 
                {
                    idealStateMappingIndex += 1; 
                }

                byte stateCode = _idealStateMapping[idealStateMappingIndex]; 
                Debug.Assert(stateCode != DATAGRIDROWHEADER_stateNullCode);

                Storyboard storyboard; 
                do 
                {
                    string storyboardName = _stateNames[stateCode]; 
                    storyboard = this._rootElement.FindName(storyboardName) as Storyboard;
                    if (storyboard == null)
                    { 
                        stateCode = _fallbackStateMapping[stateCode];
                    }
                } 
                while (storyboard == null && stateCode != DATAGRIDROWHEADER_stateNullCode); 

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
        }
 
        #endregion Internal Methods 

 
        #region Private Methods

        private void DataGridRowHeader_MouseEnter(object sender, MouseEventArgs e) 
        {
            if (this.OwningRow != null)
            { 
                this.OwningRow.IsMouseOver = true; 
            }
        } 

        private void DataGridRowHeader_MouseLeave(object sender, MouseEventArgs e)
        { 
            if (this.OwningRow != null)
            {
                this.OwningRow.IsMouseOver = false; 
            } 
        }
 
        private void DataGridRowHeader_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (!e.Handled && this.OwningGrid != null) 
            {
                if (this.OwningRow != null)
                { 
                    Debug.Assert(sender is DataGridRowHeader); 
                    Debug.Assert(sender == this);
                    e.Handled = this.OwningGrid.UpdateStateOnMouseLeftButtonDown(e, -1, this.RowIndex); 
                }
                if (this.OwningGrid.IsTabStop)
                { 
                    bool success = this.OwningGrid.Focus();
                    Debug.Assert(success);
                } 
            } 
        }
 
        #endregion Private Methods
    }
} 
