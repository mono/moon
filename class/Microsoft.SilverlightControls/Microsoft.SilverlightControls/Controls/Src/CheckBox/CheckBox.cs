// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Windows.Controls.Primitives; 
using System.Windows.Input; 
using System.Windows.Media.Animation;
using System.Windows.Controls;
 
namespace System.Windows.Controls
{
    /// <summary> 
    /// Represents a control that a user can select and clear.
    /// </summary>
    [TemplatePart(Name = CheckBox.ElementRootName, Type = typeof(FrameworkElement))] 
    [TemplatePart(Name = CheckBox.ElementFocusVisualName, Type = typeof(UIElement))] 
    [TemplatePart(Name = CheckBox.ElementContentFocusVisualName, Type = typeof(UIElement))]
    [TemplatePart(Name = CheckBox.StateCheckedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = CheckBox.StateIndeterminateName, Type = typeof(Storyboard))]
    [TemplatePart(Name = CheckBox.StateNormalName, Type = typeof(Storyboard))]
    [TemplatePart(Name = CheckBox.StateMouseOverCheckedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = CheckBox.StateMouseOverIndeterminateName, Type = typeof(Storyboard))]
    [TemplatePart(Name = CheckBox.StateMouseOverUncheckedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = CheckBox.StatePressedCheckedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = CheckBox.StatePressedIndeterminateName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = CheckBox.StatePressedUncheckedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = CheckBox.StateDisabledCheckedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = CheckBox.StateDisabledIndeterminateName, Type = typeof(Storyboard))]
    [TemplatePart(Name = CheckBox.StateDisabledUncheckedName, Type = typeof(Storyboard))]
    public partial class CheckBox : ToggleButton 
    {
        /// <summary>
        /// Initializes a new instance of the CheckBox class. 
        /// </summary> 
        public CheckBox()
        { 
            // Ignore the ENTER key by default
            KeyboardNavigation.SetAcceptsReturn(this, false);
        } 

        /// <summary>
        /// Handles the KeyDown event for CheckBox. 
        /// </summary> 
        /// <param name="key">
        /// The keyboard key associated with the event. 
        /// </param>
        /// <returns>True if the event was handled, false otherwise.</returns>
        internal override bool OnKeyDownInternal(Key key) 
        {
            bool handled = base.OnKeyDownInternal(key);
 
            if (!IsThreeState && IsEnabled) 
            {
                if (key == Key.Add) 
                {
                    handled = true;
                    IsPressed = false; 
                    IsChecked = true;
                }
                else if (key == Key.Subtract) 
                { 
                    handled = true;
                    IsPressed = false; 
                    IsChecked = false;
                }
            } 

            return handled;
        } 
    } 
}
