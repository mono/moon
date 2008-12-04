// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Diagnostics; 
using System.Windows.Controls.Primitives; 
using System.Windows.Media.Animation;
 
namespace System.Windows.Controls
{
    /// <summary> 
    /// Represents a button control, which reacts to the Click event.
    /// </summary>
    [TemplatePart(Name = Button.ElementRootName, Type = typeof(FrameworkElement))] 
    [TemplatePart(Name = Button.ElementFocusVisualName, Type = typeof(UIElement))] 
    [TemplatePart(Name = Button.StateNormalName, Type = typeof(Storyboard))]
    [TemplatePart(Name = Button.StateMouseOverName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = Button.StatePressedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = Button.StateDisabledName, Type = typeof(Storyboard))]
    public partial class Button : ButtonBase 
    {
        #region Template Parts
        /// <summary> 
        /// Root template part of the Button. 
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal FrameworkElement _elementRoot;
        internal const string ElementRootName = "RootElement";
 
        /// <summary>
        /// Focus visual template part of the Button.
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal UIElement _elementFocusVisual;
        internal const string ElementFocusVisualName = "FocusVisualElement"; 

        /// <summary>
        /// Normal state of the Button. 
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _stateNormal; 
        internal const string StateNormalName = "Normal State"; 

        /// <summary> 
        /// MouseOver state of the Button.
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal Storyboard _stateMouseOver;
        internal const string StateMouseOverName = "MouseOver State";
 
        /// <summary> 
        /// Pressed state of the Button.
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _statePressed;
        internal const string StatePressedName = "Pressed State"; 

        /// <summary>
        /// Disabled state of the Button. 
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _stateDisabled; 
        internal const string StateDisabledName = "Disabled State";
        #endregion Template Parts
 
        /// <summary>
        /// Initializes a new instance of the Button class.
        /// </summary> 
        public Button() 
        {
            // IsTabStop should always match IsEnabled (which is false by 
            // default) to prevent tabbing into disabled buttons.
            IsTabStop = false;
        } 

        /// <summary>
        /// Apply a template to the Button. 
        /// </summary> 
        public override void OnApplyTemplate()
        { 
            base.OnApplyTemplate();

            // Get the elements 
            object root = GetTemplateChild(ElementRootName);
            Debug.Assert(typeof(FrameworkElement).IsInstanceOfType(root) || (root == null),
                "The template part RootElement is not an instance of FrameworkElement!"); 
            _elementRoot = root as FrameworkElement; 

            object focusVisual = GetTemplateChild(ElementFocusVisualName); 
            Debug.Assert(typeof(UIElement).IsInstanceOfType(focusVisual) || (focusVisual == null),
                "The template part FocusVisualElement is not an instance of UIElement!");
            _elementFocusVisual = focusVisual as UIElement; 

            // Get the states
            if (_elementRoot != null) 
            { 
                object normal = _elementRoot.Resources[StateNormalName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(normal) || (normal == null), 
                    "The template part Normal State is not an instance of Storyboard!");
                _stateNormal = normal as Storyboard;
 
                object mouseOver = _elementRoot.Resources[StateMouseOverName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(mouseOver) || (mouseOver == null),
                    "The template part MouseOver State is not an instance of Storyboard!"); 
                _stateMouseOver = mouseOver as Storyboard; 

                object pressed = _elementRoot.Resources[StatePressedName]; 
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(pressed) || (pressed == null),
                    "The template part Pressed State is not an instance of Storyboard!");
                _statePressed = pressed as Storyboard; 

                object disabled = _elementRoot.Resources[StateDisabledName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(disabled) || (disabled == null), 
                    "The template part Disabled State is not an instance of Storyboard!"); 
                _stateDisabled = disabled as Storyboard;
            } 

            // Sync the logical and visual states of the control
            UpdateVisualState(); 
        }

        /// <summary> 
        /// Change to the correct visual state for the button. 
        /// </summary>
        internal override void ChangeVisualState() 
        {
            if (!IsEnabled)
            { 
                ChangeVisualState(_stateDisabled ??
                    _stateNormal);
            } 
            else if (IsPressed) 
            {
                ChangeVisualState(_statePressed ?? 
                    _stateMouseOver ??
                    _stateNormal);
            } 
            else if (IsMouseOver)
            {
                ChangeVisualState(_stateMouseOver ?? 
                    _stateNormal); 
            }
            else 
            {
                ChangeVisualState(_stateNormal);
            } 

            if (_elementFocusVisual != null)
            { 
                _elementFocusVisual.Visibility = (IsFocused && IsEnabled) ? 
                    Visibility.Visible :
                    Visibility.Collapsed; 
            }
        }
 
        /// <summary>
        /// Called when the IsEnabled property changes.
        /// </summary> 
        /// <param name="isEnabled">New value of the IsEnabled property.</param> 
        internal override void OnIsEnabledChanged(bool isEnabled)
        { 
            base.OnIsEnabledChanged(isEnabled);

            // Sync IsTabStop with IsEnabled so that disabled buttons won't 
            // receive focus.  Unfortunately because we can't receive change
            // notifications on IsTabStop, we can't preserve a user supplied
            // value when IsEnabled = true and have to clobber it. 
            IsTabStop = isEnabled; 
        }
    } 
}
