// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Windows.Input;
 
namespace System.Windows.Controlsb1
{
    public class DataGridEditingTriggerInfo 
    {
        public DataGridEditingTriggerInfo(ModifierKeys modifierKeys,
            KeyEventArgs keyEventArgs, 
            MouseButtonEventArgs mouseButtonEventArgs)
            : this(false, modifierKeys, keyEventArgs, mouseButtonEventArgs)
        { 
        } 

        internal DataGridEditingTriggerInfo(bool containsFocus, 
            ModifierKeys modifierKeys, KeyEventArgs keyEventArgs,
            MouseButtonEventArgs mouseButtonEventArgs)
        { 
            this.ContainsFocus = containsFocus;
            this.ModifierKeys = modifierKeys;
            this.KeyEventArgs = keyEventArgs; 
            this.MouseButtonEventArgs = mouseButtonEventArgs; 
        }
 
        #region Public Properties

        public bool ContainsFocus 
        {
            get;
            internal set; 
        } 

        public KeyEventArgs KeyEventArgs 
        {
            get;
            private set; 
        }

        public ModifierKeys ModifierKeys 
        { 
            get;
            private set; 
        }

        public MouseButtonEventArgs MouseButtonEventArgs 
        {
            get;
            private set; 
        } 

        #endregion Public Properties 
    }
}
