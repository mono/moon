// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Windows.Media.Animation; 
 
namespace System.Windows.Controlsb1
{ 
    /// <summary>
    /// Represents a button control used in Calendar Control, which reacts to the Click event.
    /// </summary> 
    [TemplatePart(Name = MONTHBUTTON_elementRootName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = MONTHBUTTON_elementFocusVisualName, Type = typeof(UIElement))]
    [TemplatePart(Name = MONTHBUTTON_stateNormalName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = MONTHBUTTON_statePressName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = MONTHBUTTON_statePressSelectedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = MONTHBUTTON_stateSelectedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = MONTHBUTTON_stateMouseOverName, Type = typeof(Storyboard))]
    [TemplatePart(Name = MONTHBUTTON_MouseOverSelectedName, Type = typeof(Storyboard))]
 
    public sealed class MonthButton : CalendarButtonBase
    {
        #region Constants 
 
        private const string MONTHBUTTON_elementFocusVisualName = "FocusVisualElement";
        private const string MONTHBUTTON_elementRootName = "RootElement"; 
        private const string MONTHBUTTON_stateMouseOverName = "MouseOver State";
        private const string MONTHBUTTON_MouseOverSelectedName = "MouseOver Selected State";
        private const string MONTHBUTTON_stateNormalName = "Normal State"; 
        private const string MONTHBUTTON_statePressName = "Pressed State";
        private const string MONTHBUTTON_statePressSelectedName = "Pressed Selected State";
        private const string MONTHBUTTON_stateSelectedName = "Normal Selected State"; 
 
        #endregion Constants
 
        #region Data

        private UIElement _elementFocusVisual; 
        private FrameworkElement _elementRoot;
        private bool _isFocusedOverride;
        private bool _isSelected; 
        private Storyboard _stateNormal; 
        private Storyboard _stateSelected;
        private Storyboard _stateMouseOver; 
        private Storyboard _stateMouseOverSelected;
        private Storyboard _statePressed;
        private Storyboard _statePressedSelected; 

        #endregion Data
 
        internal MonthButton(): base() 
        {
            this.IsTabStop = true; 
        }

        #region Internal Properties 

        internal bool IsFocusedOverride
        { 
            get 
            {
                return this._isFocusedOverride; 
            }
            set
            { 
                this._isFocusedOverride = value;
                ChangeVisualState();
            } 
        } 

        internal bool IsSelected 
        {
            get
            { 
                return this._isSelected;
            }
            set 
            { 
                this._isSelected = value;
                ChangeVisualState(); 
            }
        }
 
        #endregion Internal Properties

        #region Protected methods 
        /// <summary> 
        /// Invoked whenever application code or an internal process,
        /// such as a rebuilding layout pass, calls the ApplyTemplate method. 
        /// </summary>
        public override void OnApplyTemplate()
        { 
            base.OnApplyTemplate();

            // Get the elements 
            object root = GetTemplateChild(MONTHBUTTON_elementRootName); 
            Debug.Assert(typeof(FrameworkElement).IsInstanceOfType(root) || (root == null));
            _elementRoot = root as FrameworkElement; 

            object focusVisual = GetTemplateChild(MONTHBUTTON_elementFocusVisualName);
            Debug.Assert(typeof(UIElement).IsInstanceOfType(focusVisual) || (focusVisual == null)); 
            _elementFocusVisual = focusVisual as UIElement;

            // Get the states 
            if (_elementRoot != null) 
            {
                DependencyObject normal = _elementRoot.Resources[MONTHBUTTON_stateNormalName] as DependencyObject; 
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(normal) || (normal == null));
                _stateNormal = normal as Storyboard;
 
                DependencyObject selected = _elementRoot.Resources[MONTHBUTTON_stateSelectedName] as DependencyObject;
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(selected) || (selected == null));
                _stateSelected = selected as Storyboard; 
 
                DependencyObject mouseOver = _elementRoot.Resources[MONTHBUTTON_stateMouseOverName] as DependencyObject;
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(mouseOver) || (mouseOver == null)); 
                _stateMouseOver = mouseOver as Storyboard;

                DependencyObject mouseOverSelected = _elementRoot.Resources[MONTHBUTTON_MouseOverSelectedName] as DependencyObject; 
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(mouseOverSelected) || (mouseOverSelected == null));
                _stateMouseOverSelected = mouseOverSelected as Storyboard;
 
                DependencyObject pressed = _elementRoot.Resources[MONTHBUTTON_statePressName] as DependencyObject; 
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(pressed) || (pressed == null));
                _statePressed = pressed as Storyboard; 

                DependencyObject pressedSelected = _elementRoot.Resources[MONTHBUTTON_statePressSelectedName] as DependencyObject;
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(pressedSelected) || (pressedSelected == null)); 
                _statePressedSelected = pressedSelected as Storyboard;
            }
 
            // Sync the logical and visual states of the control 
            ChangeVisualState();
        } 

        #endregion Protected Methods
 
        #region Internal Methods

        internal override void ChangeVisualState() 
        { 
            if (IsPressed)
            { 
                if (_isSelected)
                {
                    ChangeVisualState(_statePressedSelected); 
                }
                else
                { 
                    ChangeVisualState(_statePressed); 
                }
            } 
            else
            {
                if (IsMouseOver) 
                {
                    if (_isSelected)
                    { 
                        ChangeVisualState(_stateMouseOverSelected); 
                    }
                    else 
                    {
                        ChangeVisualState(_stateMouseOver);
                    } 
                }
                else
                { 
                    if (_isSelected) 
                    {
                        ChangeVisualState(_stateSelected); 
                    }
                    else
                    { 
                        ChangeVisualState(_stateNormal);
                    }
                } 
            } 

            if (_elementFocusVisual != null) 
            {
                _elementFocusVisual.Visibility = (_isFocusedOverride) ?
                    Visibility.Visible : 
                    Visibility.Collapsed;
            }
        } 
 
        #endregion Internal Methods
    } 
}
