// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Diagnostics; 
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Inputb1;
using System.Windows.Media.Animation;
using System.Windows.Controls;

namespace System.Windows.Controlsb1
{ 
    /// <summary>
    /// Represents the base class for all Button controls in a Calendar
    /// </summary> 
    public abstract partial class CalendarButtonBase : ButtonBase 
    {
        /// <summary> 
        /// True if the control has been loaded; false otherwise.
        /// </summary>
        internal bool _isLoaded; 

        /// <summary>
        /// True if the mouse has been captured by this button, false otherwise. 
        /// </summary> 
        internal bool _isMouseCaptured;
 
        /// <summary>
        /// True if the SPACE key is currently pressed, false otherwise.
        /// </summary> 
        internal bool _isSpaceKeyDown;

        /// <summary> 
        /// True if the mouse's left button is currently down, false otherwise. 
        /// </summary>
        internal bool _isMouseLeftButtonDown; 

        /// <summary>
        /// Last known position of the mouse with respect to this Button. 
        /// </summary>
        internal Point _mousePosition;
 
        /// <summary> 
        /// Current visual state of the button.
        /// </summary> 
        internal Storyboard _currentState;

        /// <summary> 
        /// True if visual state changes are suspended; false otherwise.
        /// </summary>
        internal bool _suspendStateChanges; 
 
        /// <summary>
        /// Initializes a new instance of the ButtonBase class. 
        /// </summary>
        internal CalendarButtonBase()
        { 
            // Attach the necessary events to their virtual counterparts
            Loaded += delegate { _isLoaded = true; UpdateVisualState(); };
        } 
 
        /// <summary>
        /// Update the current visual state of the button. 
        /// </summary>
        internal void UpdateVisualState()
        { 
            if (!_suspendStateChanges)
            {
                ChangeVisualState(); 
            } 
        }
 
        /// <summary>
        /// Change to the correct visual state for the button.
        /// </summary> 
        internal virtual void ChangeVisualState()
        {
        } 
 
        /// <summary>
        /// Change the visual state of the button. 
        /// </summary>
        /// <param name="state">Next visual state of the button.</param>
        /// <remarks> 
        /// This method should not be called by controls to force a change to
        /// the current visual state.  UpdateVisualState is preferred because
        /// it properly handles suspension of state changes. 
        /// </remarks> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes", Justification = "An invalid Storyboard or playing a Storyboard at the wrong time throws System.Exception.")]
        internal void ChangeVisualState(Storyboard state) 
        {
            // Do nothing if we're already in the right state
            Storyboard previousState = _currentState; 
            if (state == previousState)
            {
                return; 
            } 

            // Try to prevent transitioning to a new state before the control 
            // has been loaded by waiting for the Loaded event and checking if
            // it has a Parent because it was added to the visual tree (since
            // playing a Storyboard before a control is loaded will raise an 
            // Exception as per Jolt Bug 13025).
            if (state != null && _isLoaded && Parent != null)
            { 
                try 
                {
                    // Transition into the state 
                    state.Begin();

                    // Don't make the new state the current state until after we 
                    // we know it didn't fail to play the transition (so the
                    // next call to UpdateVisualState will have a chance to try
                    // again) 
                    _currentState = state; 

                    // Back out of the previous state after moving to the next 
                    // state to restore any values not used by the new state.
                    if (previousState != null)
                    { 
                        previousState.Stop();
                    }
                } 
                catch (Exception) 
                {
                } 
            }
        }
 
        /// <summary>
        /// Capture the mouse.
        /// </summary> 
        internal void CaptureMouseInternal() 
        {
            if (!_isMouseCaptured) 
            {
                _isMouseCaptured = CaptureMouse();
            } 
        }

        /// <summary> 
        /// Release mouse capture if we already had it. 
        /// </summary>
        internal void ReleaseMouseCaptureInternal() 
        {
            ReleaseMouseCapture();
           _isMouseCaptured = false; 
        }

        /// <summary> 
        /// Called when the IsEnabled property changes. 
        /// </summary>
        /// <param name="isEnabled">New value of the IsEnabled property.</param> 
        protected override void OnIsEnabledChanged(bool isEnabled)
        {
            _suspendStateChanges = true; 
            try
            {
                if (!isEnabled) 
                { 
                    SetValue(IsFocusedProperty, false);
                    IsPressed = false; 
                    _isMouseCaptured = false;
                    _isSpaceKeyDown = false;
                    _isMouseLeftButtonDown = false; 
                }
            }
            finally 
            { 
                _suspendStateChanges = false;
                UpdateVisualState(); 
            }
        }
 
        /// <summary>
        /// Called when the IsPressed property changes.
        /// </summary> 
        /// <param name="e"> 
        /// The data for DependencyPropertyChangedEventArgs.
        /// </param> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        protected override void OnIsPressedChanged(DependencyPropertyChangedEventArgs e)
        { 
            UpdateVisualState();
        }
 
        /// <summary> 
        /// Responds to the GotFocus event.
        /// </summary> 
        /// <param name="e">The event data for the GotFocus event.</param>
        protected override void OnGotFocus(RoutedEventArgs e)
        { 
            if (e == null)
            {
                throw new ArgumentNullException("e"); 
            } 
            UpdateVisualState();
        } 

        /// <summary>
        /// Responds to the LostFocus event. 
        /// </summary>
        /// <param name="e">The event data for the LostFocus event.</param>
        protected override void OnLostFocus(RoutedEventArgs e) 
        { 
            if (e == null)
            { 
                throw new ArgumentNullException("e");
            }
 
            _suspendStateChanges = true;
            try
            { 
                if (ClickMode != System.Windows.Controls.ClickMode.Hover) 
                {
                    IsPressed = false; 
                    ReleaseMouseCaptureInternal();
                    _isSpaceKeyDown = false;
                } 
            }
            finally
            { 
                _suspendStateChanges = false; 
                UpdateVisualState();
            } 
        }

        /// <summary> 
        /// Responds to the KeyDown event.
        /// </summary>
        /// <param name="e">The event data for the KeyDown event.</param> 
        protected override void OnKeyDown(KeyEventArgs e) 
        {
            if (e == null) 
            {
                throw new ArgumentNullException("e");
            } 
            else if (e.Handled)
            {
                return; 
            } 

            if (OnKeyDownInternal(e.Key)) 
            {
                e.Handled = true;
            } 
        }

        /// <summary> 
        /// Handles the KeyDown event for ButtonBase. 
        /// </summary>
        /// <param name="key"> 
        /// The keyboard key associated with the event.
        /// </param>
        /// <returns>True if the event was handled, false otherwise.</returns> 
        /// <remarks>
        /// This method exists for the purpose of unit testing since we can't
        /// set KeyEventArgs.Key to simulate key press events. 
        /// </remarks> 
        internal virtual bool OnKeyDownInternal(Key key)
        { 
            // True if the button will handle the event, false otherwise.
            bool handled = false;
 
            // Key presses can be ignored when disabled or in ClickMode.Hover
            if (IsEnabled && (ClickMode != System.Windows.Controls.ClickMode.Hover))
            { 
                // Hitting the SPACE key is equivalent to pressing the mouse 
                // button
                if (key == Key.Space) 
                {
                    // Ignore the SPACE key if we already have the mouse
                    // captured 
                    if (!_isMouseCaptured)
                    {
                        _isSpaceKeyDown = true; 
                        IsPressed = true; 
                        CaptureMouseInternal();

                        if (ClickMode == System.Windows.Controls.ClickMode.Press)
                        {
                            OnClick(); 
                        }

                        handled = true; 
                    } 
                }
                // The ENTER key forces a click 
                else if ((key == Key.Enter) && System.Windows.Inputb1.KeyboardNavigation.GetAcceptsReturn(this))
                {
                    _isSpaceKeyDown = false; 
                    IsPressed = false;
                    ReleaseMouseCaptureInternal();
 
                    OnClick(); 

                    handled = true; 
                }
                // Any other keys pressed are irrelevant
                else if (_isSpaceKeyDown) 
                {
                    IsPressed = false;
                    _isSpaceKeyDown = false; 
                    ReleaseMouseCaptureInternal(); 
                }
            } 

            return handled;
        } 

        /// <summary>
        /// Responds to the KeyUp event. 
        /// </summary> 
        /// <param name="e">The event data for the KeyUp event.</param>
        protected override void OnKeyUp(KeyEventArgs e) 
        {
            if (e == null)
            { 
                throw new ArgumentNullException("e");
            }
            else if (e.Handled) 
            { 
                return;
            } 

            if (OnKeyUpInternal(e.Key))
            { 
                e.Handled = true;
            }
        } 
 
        /// <summary>
        /// Handles the KeyUp event for ButtonBase. 
        /// </summary>
        /// <param name="key">The keyboard key associated with the event.</param>
        /// <returns>True if the event was handled, false otherwise.</returns> 
        /// <remarks>
        /// This method exists for the purpose of unit testing since we can't
        /// set KeyEventArgs.Key to simulate key press events. 
        /// </remarks> 
        internal bool OnKeyUpInternal(Key key)
        { 
            // True if the button will handle the event, false otherwise.
            bool handled = false;
 
            // Key presses can be ignored when disabled or in ClickMode.Hover
            // or if any other key than SPACE was released.
            if (IsEnabled && (ClickMode != System.Windows.Controls.ClickMode.Hover) && (key == Key.Space)) 
            { 
                _isSpaceKeyDown = false;
 
                if (!_isMouseLeftButtonDown)
                {
                    // If the mouse isn't in use, raise the Click event if we're 
                    // in the correct click mode
                    ReleaseMouseCaptureInternal();
                    if (IsPressed && (ClickMode == System.Windows.Controls.ClickMode.Release)) 
                    { 
                        OnClick();
                    } 

                    IsPressed = false;
                } 
                else if (_isMouseCaptured)
                {
                    // Determine if the button should still be pressed based on 
                    // the position of the mouse. 
                    bool isValid = IsValidMousePosition();
                    IsPressed = isValid; 
                    if (!isValid)
                    {
                        ReleaseMouseCaptureInternal(); 
                    }
                }
 
                handled = true; 
            }
 
            return handled;
        }
 
        /// <summary>
        /// Responds to the MouseEnter event.
        /// </summary> 
        /// <param name="e">The event data for the MouseEnter event.</param> 
        protected override void OnMouseEnter(MouseEventArgs e)
        { 
            if (e == null)
            {
                throw new ArgumentNullException("e"); 
            }
            else if (e.Handled)
            { 
                return; 
            }
 
            _suspendStateChanges = true;
            try
            {
                if ((ClickMode == System.Windows.Controls.ClickMode.Hover) && IsEnabled)
                {
                    IsPressed = true; 
                    OnClick(); 
                    e.Handled = true;
                } 

            }
            finally 
            {
                _suspendStateChanges = false;
                UpdateVisualState(); 
            } 
        }
 
        /// <summary>
        /// Responds to the MouseLeave event.
        /// </summary> 
        /// <param name="e">The event data for the MouseLeave event.</param>
        protected override void OnMouseLeave(MouseEventArgs e)
        { 
            if (e == null) 
            {
                throw new ArgumentNullException("e"); 
            }
            else if (e.Handled)
            { 
                return;
            }
 
            _suspendStateChanges = true; 
            try
            {
                if ((ClickMode == System.Windows.Controls.ClickMode.Hover) && IsEnabled)
                {
                    IsPressed = false; 
                    e.Handled = true;
                }
            } 
            finally 
            {
                _suspendStateChanges = false; 
                UpdateVisualState();
            }
        } 

        /// <summary>
        /// Responds to the MouseLeftButtonDown event. 
        /// </summary> 
        /// <param name="e">
        /// The event data for the MouseLeftButtonDown event. 
        /// </param>
        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
        { 
            if (e == null)
            {
                throw new ArgumentNullException("e"); 
            } 
            else if (e.Handled)
            { 
                return;
            }

            if (!IsEnabled || (ClickMode == System.Windows.Controls.ClickMode.Hover))
            {
                return; 
            } 

            e.Handled = true; 
            _suspendStateChanges = true;
            try
            { 
                Focus();

                CaptureMouseInternal(); 
                if (_isMouseCaptured) 
                {
                    IsPressed = true; 
                }
            }
            finally 
            {
                _suspendStateChanges = false;
                UpdateVisualState(); 
            }

            if (ClickMode == System.Windows.Controls.ClickMode.Press) 
            {
                OnClick();
            } 
        }

        /// <summary> 
        /// Responds to the MouseLeftButtonUp event. 
        /// </summary>
        /// <param name="e"> 
        /// The event data for the MouseLeftButtonUp event.
        /// </param>
        protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e) 
        {
            if (e == null)
            { 
                throw new ArgumentNullException("e"); 
            }
            else if (e.Handled) 
            {
                return;
            }

            if (!IsEnabled || (ClickMode == System.Windows.Controls.ClickMode.Hover))
            { 
                return; 
            }
 
            e.Handled = true;
            if (!_isSpaceKeyDown && IsPressed && (ClickMode == System.Windows.Controls.ClickMode.Release))
            { 
                OnClick();
            }
 
            if (!_isSpaceKeyDown) 
            {
                ReleaseMouseCaptureInternal(); 
                IsPressed = false;
            }
        } 

        /// <summary>
        /// Responds to the MouseMove event. 
        /// </summary> 
        /// <param name="e">The event data for the MouseMove event.</param>
        protected override void OnMouseMove(MouseEventArgs e) 
        {
            if (e == null)
            { 
                throw new ArgumentNullException("e");
            }
 
            // Cache the latest mouse position. 
            _mousePosition = e.GetPosition(this);
 
            if (e.Handled)
            {
                return; 
            }

            // Determine if the button is still pressed based on the mouse's 
            // current position. 
            if (_isMouseLeftButtonDown &&
                IsEnabled &&
                (ClickMode != System.Windows.Controls.ClickMode.Hover) &&
                _isMouseCaptured &&
                !_isSpaceKeyDown) 
            {
                IsPressed = IsValidMousePosition();
                e.Handled = true; 
            } 
        }
 
        /// <summary>
        /// Determine if the mouse is above the button based on its last known
        /// position. 
        /// </summary>
        /// <returns>
        /// True if the mouse is considered above the button, false otherwise. 
        /// </returns> 
        internal bool IsValidMousePosition()
        { 
            return (_mousePosition.X >= 0.0) &&
                (_mousePosition.X <= ActualWidth) &&
                (_mousePosition.Y >= 0.0) && 
                (_mousePosition.Y <= ActualHeight);
        }
    } 
} 
