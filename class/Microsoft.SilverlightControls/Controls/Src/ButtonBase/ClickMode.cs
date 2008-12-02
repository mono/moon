// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
 
namespace System.Windows.Controls
{
    /// <summary> 
    /// Specifies when the ButtonBase.Click event should fire.
    /// </summary>
    public enum ClickMode 
    {
        /// <summary>
        /// Specifies that the Click event will occur when a button is pressed 
        /// and released. 
        /// </summary>
        Release = 0, 

        /// <summary>
        /// Specifies that the Click event will occur as soon as a button is 
        /// pressed.
        /// </summary>
        Press = 1, 
 
        /// <summary>
        /// Specifies that the Click event should fire when the mouse hovers 
        /// over a control.
        /// </summary>
        Hover = 2 
    }
}
