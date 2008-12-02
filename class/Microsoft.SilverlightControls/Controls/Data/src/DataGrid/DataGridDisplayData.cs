// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controlsb1
{ 
    internal class DataGridDisplayData 
    {
        #region Data 

        private int _firstDisplayedFrozenCol;
        // 
        private int _firstDisplayedScrollingCol;
        private int _firstDisplayedScrollingRow;
        private int _lastDisplayedFrozenCol; 
        // 
        private int _lastDisplayedScrollingRow;
        private int _lastTotallyDisplayedScrollingCol; 
        private int _numDisplayedFrozenCols;
        //
        private int _numDisplayedScrollingCols; 
        private int _numDisplayedScrollingRows;
        //
        private int _numTotallyDisplayedScrollingRows; 
 
        #endregion
 
        public DataGridDisplayData()
        {
            // 
            _firstDisplayedFrozenCol = -1;
            _firstDisplayedScrollingRow = -1;
            _firstDisplayedScrollingCol = -1; 
            _lastTotallyDisplayedScrollingCol = -1; 
            _lastDisplayedScrollingRow = -1;
            _lastDisplayedFrozenCol = -1; 
            //
            OldFirstDisplayedScrollingRow = -1;
            OldFirstDisplayedScrollingCol = -1; 
        }

        #region Public Properties 
 
        public bool ColumnInsertionOccurred
        { 
            get;
            private set;
        } 

        public bool Dirty
        { 
            get; 
            set;
        } 

        public int FirstDisplayedFrozenCol
        { 
            get
            {
                return this._firstDisplayedFrozenCol; 
            } 
            set
            { 
                if (value != _firstDisplayedFrozenCol)
                {
                    EnsureDirtyState(); 
                    _firstDisplayedFrozenCol = value;
                }
            } 
        } 

        // 


 


 
 

 


        public int FirstDisplayedScrollingCol 
        {
            get
            { 
                return _firstDisplayedScrollingCol; 
            }
            set 
            {
                if (value != _firstDisplayedScrollingCol)
                { 
                    EnsureDirtyState();
                    _firstDisplayedScrollingCol = value;
                } 
            } 
        }
 
        public int FirstDisplayedScrollingRow
        {
            get 
            {
                return _firstDisplayedScrollingRow;
            } 
            set 
            {
                if (value != _firstDisplayedScrollingRow) 
                {
                    EnsureDirtyState();
                    _firstDisplayedScrollingRow = value; 
                }
            }
        } 
 
        public int LastDisplayedFrozenCol
        { 
            get
            {
                return _lastDisplayedFrozenCol; 
            }
            set
            { 
                if (value != _lastDisplayedFrozenCol) 
                {
                    EnsureDirtyState(); 
                    _lastDisplayedFrozenCol = value;
                }
            } 
        }

        // 
 

 


 


 
 

        public int LastDisplayedScrollingRow 
        {
            get
            { 
                return _lastDisplayedScrollingRow;
            }
            set 
            { 
                if (value != _lastDisplayedScrollingRow)
                { 
                    EnsureDirtyState();
                    _lastDisplayedScrollingRow = value;
                } 
            }
        }
 
        public int LastTotallyDisplayedScrollingCol 
        {
            get 
            {
                return _lastTotallyDisplayedScrollingCol;
            } 
            set
            {
                if (value != _lastTotallyDisplayedScrollingCol) 
                { 
                    EnsureDirtyState();
                    _lastTotallyDisplayedScrollingCol = value; 
                }
            }
        } 

        public int NumDisplayedFrozenCols
        { 
            get 
            {
                return _numDisplayedFrozenCols; 
            }
            set
            { 
                if (value != _numDisplayedFrozenCols)
                {
                    EnsureDirtyState(); 
                    _numDisplayedFrozenCols = value; 
                }
            } 
        }

        // 


 
 

 


 


 
 

 
        public int NumDisplayedScrollingCols
        {
            get 
            {
                return _numDisplayedScrollingCols;
            } 
            set 
            {
                if (value != _numDisplayedScrollingCols) 
                {
                    EnsureDirtyState();
                    _numDisplayedScrollingCols = value; 
                }
            }
        } 
 
        public int NumDisplayedScrollingRows
        { 
            get
            {
                return _numDisplayedScrollingRows; 
            }
            set
            { 
                if (value != _numDisplayedScrollingRows) 
                {
                    EnsureDirtyState(); 
                    _numDisplayedScrollingRows = value;
                }
            } 
        }

        // 
 

 


 


 
 

 


 
        public int NumTotallyDisplayedScrollingRows
        {
            get 
            { 
                return _numTotallyDisplayedScrollingRows;
            } 
            set
            {
                if (value != _numTotallyDisplayedScrollingRows) 
                {
                    EnsureDirtyState();
                    _numTotallyDisplayedScrollingRows = value; 
                } 
            }
        } 

        public int OldFirstDisplayedScrollingCol
        { 
            get;
            private set;
        } 
 
        public int OldFirstDisplayedScrollingRow
        { 
            get;
            private set;
        } 

        //
 
 

 

        public int OldNumDisplayedScrollingRows
        { 
            get;
            private set;
        } 
 
        public bool RowInsertionOccurred
        { 
            get;
            private set;
        } 

        #endregion
 
        #region Public Methods 

        public void CorrectColumnIndexAfterInsertion(int columnIndex, int insertionCount) 
        {
            EnsureDirtyState();
            if (OldFirstDisplayedScrollingCol != -1 && columnIndex <= OldFirstDisplayedScrollingCol) 
            {
                OldFirstDisplayedScrollingCol += insertionCount;
            } 
            ColumnInsertionOccurred = true; 
        }
 
        public void CorrectRowIndexAfterDeletion(int rowIndex)
        {
            EnsureDirtyState(); 
            if (OldFirstDisplayedScrollingRow != -1 && rowIndex <= OldFirstDisplayedScrollingRow)
            {
                OldFirstDisplayedScrollingRow--; 
            } 
        }
 
        public void CorrectRowIndexAfterInsertion(int rowIndex, int insertionCount)
        {
            EnsureDirtyState(); 
            if (OldFirstDisplayedScrollingRow != -1 && rowIndex <= OldFirstDisplayedScrollingRow)
            {
                OldFirstDisplayedScrollingRow += insertionCount; 
            } 
            RowInsertionOccurred = true;
            OldNumDisplayedScrollingRows += insertionCount; 
            //
        }
 
        public void EnsureDirtyState()
        {
            if (!Dirty) 
            { 
                Dirty = true;
                RowInsertionOccurred = false; 
                ColumnInsertionOccurred = false;
                SetOldValues();
            } 
        }

        #endregion 
 
        #region Private Methods
 
        private void SetOldValues()
        {
            OldFirstDisplayedScrollingRow = _firstDisplayedScrollingRow; 
            OldFirstDisplayedScrollingCol = _firstDisplayedScrollingCol;
            //
            OldNumDisplayedScrollingRows = _numDisplayedScrollingRows; 
        } 

        #endregion 
    }
}
