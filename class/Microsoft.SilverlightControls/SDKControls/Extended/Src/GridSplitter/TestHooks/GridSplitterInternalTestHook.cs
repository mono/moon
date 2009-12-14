// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Controls.Primitives;

namespace System.Windows.Controls
{
    public partial class GridSplitter
    {
         private InternalTestHook _testHook;

        //Internal property to expose the testhook object
        internal InternalTestHook TestHook
        {
            get
            {
                if (_testHook == null)
                {
                    _testHook = new InternalTestHook(this);
                }
                return _testHook;
            }
        }
        /// <summary>
        /// Test hook class that exposes internal and private members of the GridSplitter
        /// </summary>
        internal class InternalTestHook
        {
            //Reference to the outer 'parent' GridSplitter
            private GridSplitter _gridSplitter;

            internal InternalTestHook(GridSplitter gridSplitter)
            {
                _gridSplitter = gridSplitter;
            }



            #region Internal Properties          

            internal GridResizeDirection GridResizeDirection
            {
                get { return _gridSplitter._currentGridResizeDirection; }
            }

            internal Canvas PreviewLayer
            {
                get { return _gridSplitter._previewLayer; }
            }

            internal ResizeData ResizeData
            {
                get { return _gridSplitter._resizeData; }
            }
            #endregion

            #region Internal Methods

            internal void DragValidator_DragCompletedEvent(object sender, DragCompletedEventArgs e)
            {
                _gridSplitter.DragValidator_DragCompletedEvent(sender, e);
            }

            internal void DragValidator_DragDeltaEvent(object sender, DragDeltaEventArgs e)
            {
                _gridSplitter.DragValidator_DragDeltaEvent(sender, e);
            }

            internal void DragValidator_DragStartedEvent(object sender, DragStartedEventArgs e)
            {
                _gridSplitter.DragValidator_DragStartedEvent(sender, e);
            }           

            internal bool KeyboardMoveSplitter(double horizontalChange, double verticalChange)
            {
                return _gridSplitter.KeyboardMoveSplitter(horizontalChange, verticalChange);
            }

            #endregion
        }



    }
}
