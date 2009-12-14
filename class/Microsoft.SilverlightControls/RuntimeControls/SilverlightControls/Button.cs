// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 


using System; 
using System.Diagnostics; 
using System.Windows;
using System.Windows.Controls.Primitives; 
using System.Windows.Media.Animation;
using System.Windows.Automation.Peers;
 
namespace System.Windows.Controls
{
    /// <summary> 
    /// Represents a button control, which reacts to the Click event. 
    /// </summary>
    [TemplateVisualState(Name = Button.StateNormal, GroupName = Button.GroupCommon)] 
    [TemplateVisualState(Name = Button.StateMouseOver, GroupName = Button.GroupCommon)]
    [TemplateVisualState(Name = Button.StatePressed, GroupName = Button.GroupCommon)]
    [TemplateVisualState(Name = Button.StateDisabled, GroupName = Button.GroupCommon)] 
    [TemplateVisualState(Name = Button.StateUnfocused, GroupName = Button.GroupFocus)]
    [TemplateVisualState(Name = Button.StateFocused, GroupName = Button.GroupFocus)]
    public partial class Button : ButtonBase 
    { 
        /// <summary>
        /// Initializes a new instance of the Button class. 
        /// </summary>
        public Button()
        { 
            DefaultStyleKey = typeof(Button);
        }
 
        /// <summary> 
        /// Apply a template to the Button.
        /// </summary> 
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate(); 

            // Sync the logical and visual states of the control
            UpdateVisualState(false); 
        } 

		protected override AutomationPeer OnCreateAutomationPeer ()
		{
			return new ButtonAutomationPeer (this);
		}

        /// <summary> 
        /// Change to the correct visual state for the button.
        /// </summary>
        /// <param name="useTransitions"> 
        /// true to use transitions when updating the visual state, false to
        /// snap directly to the new visual state.
        /// </param> 
        internal override void ChangeVisualState(bool useTransitions) 
        {
            if (!IsEnabled) 
            {
                GoToState(useTransitions, StateDisabled);
            } 
            else if (IsPressed)
            {
                GoToState(useTransitions, StatePressed); 
            } 
            else if (IsMouseOver)
            { 
                GoToState(useTransitions, StateMouseOver);
            }
            else 
            {
                GoToState(useTransitions, StateNormal);
            } 
 
            if (IsFocused && IsEnabled)
            { 
                GoToState(useTransitions, StateFocused);
            }
            else 
            {
                GoToState(useTransitions, StateUnfocused);
            } 
        } 

        /// <summary>
        /// Raises the Click routed event.
        /// </summary>
        protected override void OnClick()
        {
			base.OnClick ();
        }
    }
} 
