// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Input;

namespace System.Windows.Controls
{
    public partial class TabItem
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
            private TabItem _tabItem;

            internal InternalTestHook(TabItem tabItem)
            {
                _tabItem = tabItem;
            }

            internal bool IsMouseOver 
            { 
                get { return _tabItem._isMouseOver; } 
                set { _tabItem._isMouseOver = value; } 
            }

            internal TabControl TabControlParent { get { return _tabItem.TabControlParent; } }

            #region Internal Methods
            internal void OnMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
            {
                _tabItem.OnMouseLeftButtonDown(sender, e);
            }

            internal void OnMouseLeave(object sender, MouseEventArgs e)
            {
                _tabItem.OnMouseLeave(sender, e);
            }

            internal void OnMouseEnter(object sender, MouseEventArgs e)
            {
                _tabItem.OnMouseEnter(sender, e);
            }

            #endregion
        }
    }
}
