// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Windows.Input; 
using System.Windows.Media; 
using System.Windows.Shapes;
 
namespace System.Windows.Controlsb1
{
    // 
    [TemplatePart(Name = DATAGRIDCOLUMNHEADER_elementRoot, Type = typeof(FrameworkElement))]

    /* 
 

*/ 
    public partial class DataGridColumnHeader : ContentControl
    {
        #region Constants 

        private const string DATAGRIDCOLUMNHEADER_elementRoot = "RootElement";
 
        private const int DATAGRIDCOLUMNHEADER_resizeRegionWidth = 5; 
        private const double DATAGRIDCOLUMNHEADER_separatorThickness = 1;
 
        #endregion Constants

        #region Data 

        private static double _lastResizeWidth;
        private static double _originalHorizontalOffset; 
        private static double _originalWidth; 
        private static Point? _resizeStart;
        private static DataGridColumnBase _resizingColumn; 
        private bool _internalSeparatorVisibilityChange;
        private Visibility _desiredSeparatorVisibility;
 
        #endregion Data

        public DataGridColumnHeader() 
        { 
            this.MouseLeftButtonDown += new MouseButtonEventHandler(DataGridColumnHeader_MouseLeftButtonDown);
            this.MouseLeftButtonUp += new MouseButtonEventHandler(DataGridColumnHeader_MouseLeftButtonUp); 
            this.MouseMove += new MouseEventHandler(DataGridColumnHeader_MouseMove);
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
                typeof(DataGridColumnHeader),
                null); 

        #endregion SeparatorBrush
 
        #region SeparatorVisibility 

        /// <summary> 
        /// Gets or sets the visibility of the header separator line
        /// </summary>
        public Visibility SeparatorVisibility 
        {
            get { return (Visibility) GetValue(SeparatorVisibilityProperty); }
            set { SetValue(SeparatorVisibilityProperty, value); } 
        } 

        public static readonly DependencyProperty SeparatorVisibilityProperty = 
            DependencyProperty.Register("SeparatorVisibility",
                typeof(Visibility),
                typeof(DataGridColumnHeader), 
                new PropertyMetadata(OnSeparatorVisibilityPropertyChanged));

        private static void OnSeparatorVisibilityPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        { 
            DataGridColumnHeader source = d as DataGridColumnHeader;
            Debug.Assert(source != null, 
                "The source is not an instance of DataGridColumnHeader!");
            Debug.Assert(typeof(Visibility).IsInstanceOfType(e.NewValue),
                "The value is not an instance of Visibility!"); 

            if (!source.IsHandlerSuspended(e.Property) && !source._internalSeparatorVisibilityChange)
            { 
                source._desiredSeparatorVisibility = (Visibility)e.NewValue; 
            }
        } 

        #endregion SeparatorVisibility
 
        #endregion Dependency Properties

 
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
 
        internal DataGrid OwningGrid
        {
            get 
            { 
                if (this.OwningColumn != null && this.OwningColumn.OwningGrid != null)
                { 
                    return this.OwningColumn.OwningGrid;
                }
                return null; 
            }
        }
 
        internal DataGridColumnBase OwningColumn 
        {
            get; 
            set;
        }
 
        #endregion Internal Properties

 
        #region Private Properties 

        #endregion Private Properties 


        #region Public Methods 

        #endregion Public Methods
 
 
        #region Protected Methods
 
        #endregion Protected Methods

 
        #region Internal Methods

        internal void UpdateSeparatorVisibility() 
        { 
            Debug.Assert(this.OwningGrid != null);
            try 
            {
                this._internalSeparatorVisibilityChange = true;
                if (this._desiredSeparatorVisibility == Visibility.Visible) 
                {
                    if (this.OwningColumn != this.OwningGrid.ColumnsInternal.LastVisibleColumn ||
                        this.OwningGrid.ColumnsInternal.FillerColumn.IsActive) 
                    { 
                        this.SeparatorVisibility = Visibility.Visible;
                    } 
                    else
                    {
                        this.SeparatorVisibility = Visibility.Collapsed; 
                    }
                }
            } 
            finally 
            {
                this._internalSeparatorVisibilityChange = false; 
            }
        }
 
        #endregion Internal Methods

 
        #region Private Methods 
        private bool CanResizeColumn(DataGridColumnBase column)
        { 
            return this.OwningGrid.CanUserResizeColumns &&
                !(column is DataGridFillerColumn) &&
                (column.CanUserResize.HasValue && column.CanUserResize.Value || !column.CanUserResize.HasValue); 
        }

        private void DataGridColumnHeader_MouseLeftButtonDown(object sender, MouseButtonEventArgs e) 
        { 
            if (!e.Handled && this.OwningGrid != null)
            { 
                Point mousePosition = e.GetPosition(this);

                double distanceFromLeft = mousePosition.X; 
                double distanceFromRight = this.Width - distanceFromLeft;
                DataGridColumnBase currentColumn = this.OwningColumn;
                DataGridColumnBase previousColumn = null; 
                if (!(this.OwningColumn is DataGridFillerColumn)) 
                {
                    previousColumn = this.OwningGrid.ColumnsInternal.GetPreviousVisibleColumn(currentColumn); 
                }

                if (_resizingColumn == null && (distanceFromRight <= DATAGRIDCOLUMNHEADER_resizeRegionWidth)) 
                {
                    e.Handled = TrySetResizeColumn(currentColumn);
                } 
                else if (_resizingColumn == null && distanceFromLeft <= DATAGRIDCOLUMNHEADER_resizeRegionWidth && previousColumn != null) 
                {
                    e.Handled = TrySetResizeColumn(previousColumn); 
                }

                if (_resizingColumn != null) 
                {
                    this.CaptureMouse();
                    _resizeStart = e.GetPosition(this.OwningGrid); 
                    _originalWidth = _resizingColumn.Width; 
                    _originalHorizontalOffset = this.OwningGrid.HorizontalOffset;
 
                    _lastResizeWidth = _originalWidth;
                    e.Handled = true;
                } 
            }
        }
 
        private void DataGridColumnHeader_MouseLeftButtonUp(object sender, MouseButtonEventArgs e) 
        {
            _resizeStart = null; 
            ReleaseMouseCapture();

            if (_resizingColumn != null) 
            {
                _resizingColumn = null;
                // 
                SetResizeCursor(e.GetPosition(this)); 
                e.Handled = true;
            } 
        }

        private void DataGridColumnHeader_MouseMove(object sender, MouseEventArgs e) 
        {
            if (this.OwningColumn == null)
            { 
                return; 
            }
 
            if (_resizingColumn != null && _resizeStart.HasValue)
            {
                // resize column 
                Point mousePositionGrid = e.GetPosition(this.OwningGrid);

                double mouseDelta = mousePositionGrid.X - _resizeStart.Value.X; 
                double newWidth = _originalWidth + mouseDelta; 

                if (_lastResizeWidth >= _resizingColumn.MinWidth && newWidth != _lastResizeWidth) 
                {
                    newWidth = Math.Max(newWidth, _resizingColumn.MinWidth);
 
                    _resizingColumn.Width = newWidth;
                    _lastResizeWidth = newWidth;
                    this.OwningGrid.UpdateHorizontalOffset(_originalHorizontalOffset); 
                    // 
                }
 
                e.Handled = true;

                return; 
            }
            else
            { 
                SetResizeCursor(e.GetPosition(this)); 
            }
        } 

        private void SetResizeCursor(Point mousePosition)
        { 
            if (_resizingColumn != null)
            {
                return; 
            } 

            // set mouse if we can resize column 

            double distanceFromLeft = mousePosition.X;
            double distanceFromRight = this.Width - distanceFromLeft; 
            DataGridColumnBase currentColumn = this.OwningColumn;
            DataGridColumnBase previousColumn = null;
            if (!(this.OwningColumn is DataGridFillerColumn)) 
            { 
                previousColumn = this.OwningGrid.ColumnsInternal.GetPreviousVisibleColumn(currentColumn);
            } 

            if (distanceFromRight <= DATAGRIDCOLUMNHEADER_resizeRegionWidth && currentColumn != null && CanResizeColumn(currentColumn))
            { 
                this.Cursor = Cursors.SizeWE;
            }
            else if (distanceFromLeft <= DATAGRIDCOLUMNHEADER_resizeRegionWidth && previousColumn != null && CanResizeColumn(previousColumn)) 
            { 
                this.Cursor = Cursors.SizeWE;
            } 
            else
            {
                this.Cursor = Cursors.Arrow; 
            }
        }
 
        private bool TrySetResizeColumn(DataGridColumnBase column) 
        {
            // If datagrid.CanUserResizeColumns == false, then the column can still override it 
            if (CanResizeColumn(column))
            {
                _resizingColumn = column; 
                //
                return true;
            } 
            return false; 
        }
 
        #endregion Private Methods
    }
} 
