// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls
{
    public partial class TabControl
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
        /// Test hook class that exposes internal and private members of the TabItem
        /// </summary>
        internal class InternalTestHook
        {
            //Reference to the outer 'parent' TabItem
            private TabControl _tabControl;

            internal InternalTestHook(TabControl tabControl)
            {
                _tabControl = tabControl;
            }

            #region Internal Methods

            internal TabItem FindNextTabItem(int startIndex, int direction)
            {
                return _tabControl.FindNextTabItem(startIndex, direction);
            }

            #endregion
        }
    }
}
