// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 
using System.Windows.Controls;

namespace System.Windows.Controls
{ 
    /// <summary> 
    /// Specifies the visibility of a ScrollBar for scrollable content.
    /// </summary> 
    public enum ScrollBarVisibility
    {
        /// <summary> 
        /// A ScrollBar does not appear even when the viewport cannot display all of the content. The dimension of the content is set to the corresponding dimension of the ScrollViewer parent. For a horizontal ScrollBar, the width of the content is set to the ViewportWidth of the ScrollViewer. For a vertical ScrollBar, the height of the content is set to the ViewportHeight of the ScrollViewer.
        /// </summary>
        Disabled, 
 
        /// <summary>
        /// A ScrollBar appears and the dimension of the ScrollViewer is applied to the content when the viewport cannot display all of the content. For a horizontal ScrollBar, the width of the content is set to the ViewportWidth of the ScrollViewer. For a vertical ScrollBar, the height of the content is set to the ViewportHeight of the ScrollViewer. 
        /// </summary>
        Auto,
 
        /// <summary>
        /// A ScrollBar does not appear even when the viewport cannot display all of the content. The dimension of the ScrollViewer is not applied to the content.
        /// </summary> 
        Hidden, 

        /// <summary> 
        /// A ScrollBar always appears. The dimension of the ScrollViewer is applied to the content. For a horizontal ScrollBar, the width of the content is set to the ViewportWidth of the ScrollViewer. For a vertical ScrollBar, the height of the content is set to the ViewportHeight of the ScrollViewer.
        /// </summary>
        Visible, 
    }
}
