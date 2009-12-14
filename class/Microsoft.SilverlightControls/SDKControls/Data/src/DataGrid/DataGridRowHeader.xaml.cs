// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Windows.Automation.Peers;
using System.Windows.Input;
using System.Windows.Media;

namespace System.Windows.Controls.Primitives
{
    [TemplatePart(Name = DataGridRowHeader.DATAGRIDROWHEADER_elementRootName, Type = typeof(FrameworkElement))]

    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateNormal, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateNormalCurrentRow, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateNormalEditingRow, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateNormalEditingRowFocused, GroupName = VisualStates.GroupCommon)]

    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateMouseOver, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateMouseOverCurrentRow, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateMouseOverEditingRow, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateMouseOverEditingRowFocused, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateMouseOverSelected, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateMouseOverSelectedFocused, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRow, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRowFocused, GroupName = VisualStates.GroupCommon)]

    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateSelected, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateSelectedCurrentRow, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateSelectedCurrentRowFocused, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRowHeader.DATAGRIDROWHEADER_stateSelectedFocused, GroupName = VisualStates.GroupCommon)]
    
    public partial class DataGridRowHeader : ContentControl
    {
        #region Constants

        private const string DATAGRIDROWHEADER_elementRootName = "Root";
        private const double DATAGRIDROWHEADER_separatorThickness = 1;

        private const string DATAGRIDROWHEADER_stateMouseOver = "MouseOver";
        private const string DATAGRIDROWHEADER_stateMouseOverCurrentRow = "MouseOver CurrentRow";
        private const string DATAGRIDROWHEADER_stateMouseOverEditingRow = "MouseOver Unfocused EditingRow";
        private const string DATAGRIDROWHEADER_stateMouseOverEditingRowFocused = "MouseOver EditingRow";
        private const string DATAGRIDROWHEADER_stateMouseOverSelected = "MouseOver Unfocused Selected";
        private const string DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRow = "MouseOver Unfocused CurrentRow Selected";
        private const string DATAGRIDROWHEADER_stateMouseOverSelectedCurrentRowFocused = "MouseOver CurrentRow Selected";
        private const string DATAGRIDROWHEADER_stateMouseOverSelectedFocused = "MouseOver Selected";
        private const string DATAGRIDROWHEADER_stateNormal = "Normal";
        private const string DATAGRIDROWHEADER_stateNormalCurrentRow = "Normal CurrentRow";
        private const string DATAGRIDROWHEADER_stateNormalEditingRow = "Unfocused EditingRow";
        private const string DATAGRIDROWHEADER_stateNormalEditingRowFocused = "Normal EditingRow";
        private const string DATAGRIDROWHEADER_stateSelected = "Unfocused Selected";
        private const string DATAGRIDROWHEADER_stateSelectedCurrentRow = "Unfocused CurrentRow Selected";
        private const string DATAGRIDROWHEADER_stateSelectedCurrentRowFocused = "Normal CurrentRow Selected";
        private const string DATAGRIDROWHEADER_stateSelectedFocused = "Normal Selected";

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

        #endregion Data

        public DataGridRowHeader()
        {
            this.MouseLeftButtonDown += new MouseButtonEventHandler(DataGridRowHeader_MouseLeftButtonDown);
            this.MouseEnter += new MouseEventHandler(DataGridRowHeader_MouseEnter);
            this.MouseLeave += new MouseEventHandler(DataGridRowHeader_MouseLeave);

            DefaultStyleKey = typeof(DataGridRowHeader);
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

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            this._rootElement = GetTemplateChild(DATAGRIDROWHEADER_elementRootName) as FrameworkElement;
            if (this._rootElement != null)
            {
                ApplyRowStatus(false /*animate*/);
            }
        }

        #endregion Public Methods


        #region Protected Methods

        protected override Size MeasureOverride(Size availableSize)
        {
            if (this.OwningRow == null || this.OwningGrid == null)
            {
                return base.MeasureOverride(availableSize);
            }
            double measureHeight = double.IsNaN(this.OwningGrid.RowHeight) ? availableSize.Height : this.OwningGrid.RowHeight;
            double measureWidth = double.IsNaN(this.OwningGrid.RowHeaderWidth) ? availableSize.Width : this.OwningGrid.RowHeaderWidth;
            Size measuredSize = base.MeasureOverride(new Size(measureWidth, measureHeight));

            // Auto grow the row header or force it to a fixed width based on the DataGrid's setting
            if (!double.IsNaN(this.OwningGrid.RowHeaderWidth) || measuredSize.Width < this.OwningGrid.ActualRowHeaderWidth)
            {
                return new Size(this.OwningGrid.ActualRowHeaderWidth, measuredSize.Height);
            }

            return measuredSize;
        }

        /// <summary>
        /// Creates AutomationPeer (<see cref="UIElement.OnCreateAutomationPeer"/>)
        /// </summary>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new DataGridRowHeaderAutomationPeer(this);
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

                string storyboardName;
                while (stateCode != DATAGRIDROWHEADER_stateNullCode)
                {
                    storyboardName = _stateNames[stateCode];
                    if (VisualStateManager.GoToState(this, storyboardName, animate))
                    {
                        break;
                    }
                    else
                    {
                        // The state wasn't implemented so fall back to the next one
                        stateCode = _fallbackStateMapping[stateCode];
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
