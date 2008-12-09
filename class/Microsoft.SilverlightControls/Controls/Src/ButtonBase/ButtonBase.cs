// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Diagnostics; 
using System.Windows.Input; 
using System.Windows.Media.Animation;
using System.Windows.Controls;
 
namespace System.Windows.Controls.Primitives
{
    /// <summary> 
    /// Represents the base class for all Button controls.
    /// </summary>
    public partial class ButtonBase : ContentControl 
    { 
        #region ClickMode
        /// <summary> 
        /// Gets or sets when the Click event should occur.
        /// </summary>
        public ClickMode ClickMode 
        {
            get { return (ClickMode) GetValue(ClickModeProperty); }
            set { SetValue(ClickModeProperty, value); } 
        } 

        /// <summary> 
        /// Identifies the ClickMode dependency property.
        /// </summary>
        public static readonly DependencyProperty ClickModeProperty = 
            DependencyProperty.Register(
                "ClickMode",
                typeof(ClickMode), 
                typeof(ButtonBase), 
                new PropertyMetadata(OnClickModePropertyChanged));
 
        /// <summary>
        /// ClickModeProperty property changed handler.
        /// </summary> 
        /// <param name="d">ButtonBase that changed its ClickMode.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Usage", "CA2208:InstantiateArgumentExceptionsCorrectly", Justification = "Name")] 
        private static void OnClickModePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            ButtonBase source = d as ButtonBase; 
            Debug.Assert(source != null,
                "The source is not an instance of ButtonBase!");
 
            Debug.Assert(typeof(ClickMode).IsInstanceOfType(e.NewValue),
                "The value is not an instance of ClickMode!");
            ClickMode value = (ClickMode) e.NewValue; 
 
            if (value != ClickMode.Release && value != ClickMode.Press && value != ClickMode.Hover)
            { 
                throw new ArgumentException(Resource.ButtonBase_OnClickModePropertyChanged_InvalidValue, "value");
            }
        } 
        #endregion ClickMode

        #region IsFocused 
        /// <summary> 
        /// Gets a value that determines whether this element has logical focus.
        /// </summary> 
        /// <remarks>
        /// IsFocused will not be set until OnFocus has been called.  It may not
        /// yet have been set if you check it in your own Focus event handler. 
        /// </remarks>
        public bool IsFocused
        { 
            get { return (bool) GetValue(IsFocusedProperty); } 
            internal set { SetValue(IsFocusedProperty, value); }
        } 

        /// <summary>
        /// Identifies the IsFocused dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsFocusedProperty =
            DependencyProperty.Register( 
                "IsFocused", 
                typeof(bool),
                typeof(ButtonBase), 
                null);
        #endregion IsFocused
 
        #region IsMouseOver
        /// <summary>
        /// Gets a value indicating whether the mouse pointer is located over 
        /// this element. 
        /// </summary>
        /// <remarks> 
        /// IsMouseOver will not be set until MouseEnter has been called.  It
        /// may not yet have been set if you check it in your own MouseEnter
        /// event handler. 
        /// </remarks>
        public bool IsMouseOver
        { 
            get { return (bool) GetValue(IsMouseOverProperty); } 
            internal set { SetValue(IsMouseOverProperty, value); }
        } 

        /// <summary>
        /// Identifies the IsMouseOver dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsMouseOverProperty =
            DependencyProperty.Register( 
                "IsMouseOver", 
                typeof(bool),
                typeof(ButtonBase), 
                null);
        #endregion IsMouseOver
 
        #region IsPressed
        /// <summary>
        /// Gets a value that indicates whether a ButtonBase is currently 
        /// activated. 
        /// </summary>
        public bool IsPressed 
        {
            get { return (bool) GetValue(IsPressedProperty); }
            protected internal set { SetValue(IsPressedProperty, value); } 
        }

        /// <summary> 
        /// Identifies the IsPressed dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsPressedProperty = 
            DependencyProperty.Register(
                "IsPressed",
                typeof(bool), 
                typeof(ButtonBase),
                new PropertyMetadata(OnIsPressedPropertyChanged));
 
        /// <summary> 
        /// IsPressedProperty property changed handler.
        /// </summary> 
        /// <param name="d">ButtonBase that changed its IsPressed.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsPressedPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            ButtonBase source = d as ButtonBase;
            Debug.Assert(source != null, 
                "The source is not an instance of ButtonBase!"); 

            source.OnIsPressedChanged(e); 
        }
        #endregion IsPressed
 
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
        /// Occurs when a Button is clicked.
        /// </summary>
        public event RoutedEventHandler Click; 
 
        /// <summary>
        /// Initializes a new instance of the ButtonBase class. 
        /// </summary>
        protected ButtonBase()
        { 
            // Allow the button to respond to the ENTER key and be focused
            KeyboardNavigation.SetAcceptsReturn(this, true);
            IsTabStop = true; 
 
            // Attach the necessary events to their virtual counterparts
            Loaded += delegate { _isLoaded = true; UpdateVisualState(); }; 
            GotFocus += OnGotFocus;
            LostFocus += OnLostFocus;
            KeyDown += OnKeyDown; 
            KeyUp += OnKeyUp;
            MouseEnter += OnMouseEnter;
            MouseLeave += OnMouseLeave; 
            MouseLeftButtonDown += OnMouseLeftButtonDown; 
            MouseLeftButtonUp += OnMouseLeftButtonUp;
            MouseMove += OnMouseMove; 
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
        /// Invoke OnClick for testing.
        /// </summary> 
        internal void OnClickInternal()
        {
            OnClick(); 
        } 

        /// <summary> 
        /// Raises the Click routed event.
        /// </summary>
        protected virtual void OnClick() 
        {
            RoutedEventHandler handler = Click;
            if (handler != null) 
            { 
                handler(this, new RoutedEventArgs { OriginalSource = this });
            } 
        }

        /// <summary> 
        /// Called when the IsEnabled property changes.
        /// </summary>
        /// <param name="isEnabled">New value of the IsEnabled property.</param> 
        internal override void OnIsEnabledChanged(bool isEnabled) 
        {
            base.OnIsEnabledChanged(isEnabled); 
            _suspendStateChanges = true;
            try
            { 
                if (!isEnabled)
                {
                    IsFocused = false; 
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
        protected virtual void OnIsPressedChanged(DependencyPropertyChangedEventArgs e) 
        {
            UpdateVisualState();
        } 

        /// <summary>
        /// Handle the GotFocus event. 
        /// </summary> 
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">RoutedEventArgs.</param> 
        internal void OnGotFocus(object sender, RoutedEventArgs e)
        {
            IsFocused = true; 
            OnGotFocus(e);
        }
 
        /// <summary> 
        /// Responds to the GotFocus event.
        /// </summary> 
        /// <param name="e">The event data for the GotFocus event.</param>
        protected virtual void OnGotFocus(RoutedEventArgs e)
        { 
            if (e == null)
            {
                throw new ArgumentNullException("e"); 
            } 
            UpdateVisualState();
        } 

        /// <summary>
        /// Handle the LostFocus event. 
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">RoutedEventArgs.</param> 
        internal void OnLostFocus(object sender, RoutedEventArgs e) 
        {
            IsFocused = false; 
            OnLostFocus(e);
        }
 
        /// <summary>
        /// Responds to the LostFocus event.
        /// </summary> 
        /// <param name="e">The event data for the LostFocus event.</param> 
        protected virtual void OnLostFocus(RoutedEventArgs e)
        { 
            if (e == null)
            {
                throw new ArgumentNullException("e"); 
            }

            _suspendStateChanges = true; 
            try 
            {
                if (ClickMode != ClickMode.Hover) 
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
        /// Handle the KeyDown event. 
        /// </summary> 
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">KeyEventArgs.</param> 
        internal void OnKeyDown(object sender, KeyEventArgs e)
        {
            OnKeyDown(e); 
        }

        /// <summary> 
        /// Responds to the KeyDown event. 
        /// </summary>
        /// <param name="e">The event data for the KeyDown event.</param> 
        protected virtual void OnKeyDown(KeyEventArgs e)
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
            if (IsEnabled && (ClickMode != ClickMode.Hover))
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
 
                        if (ClickMode == ClickMode.Press)
                        {
                            OnClick(); 
                        } 

                        handled = true; 
                    }
                }
                // The ENTER key forces a click 
                else if ((key == Key.Enter) && KeyboardNavigation.GetAcceptsReturn(this))
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
        /// Handle the KeyUp event. 
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">KeyEventArgs.</param> 
        internal void OnKeyUp(object sender, KeyEventArgs e)
        {
            OnKeyUp(e); 
        } 

        /// <summary> 
        /// Responds to the KeyUp event.
        /// </summary>
        /// <param name="e">The event data for the KeyUp event.</param> 
        protected virtual void OnKeyUp(KeyEventArgs e)
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
            if (IsEnabled && (ClickMode != ClickMode.Hover) && (key == Key.Space))
            {
                _isSpaceKeyDown = false; 

                if (!_isMouseLeftButtonDown)
                { 
                    // If the mouse isn't in use, raise the Click event if we're 
                    // in the correct click mode
                    ReleaseMouseCaptureInternal(); 
                    if (IsPressed && (ClickMode == ClickMode.Release))
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
        /// Handle the MouseEnter event. 
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">MouseEventArgs.</param> 
        internal void OnMouseEnter(object sender, MouseEventArgs e)
        {
            IsMouseOver = true; 
            OnMouseEnter(e); 
        }
 
        /// <summary>
        /// Responds to the MouseEnter event.
        /// </summary> 
        /// <param name="e">The event data for the MouseEnter event.</param>
        protected virtual void OnMouseEnter(MouseEventArgs e)
        { 
            if (e == null) 
            {
                throw new ArgumentNullException("e"); 
            }
/*            else if (e.Handled)
            { 
                return;
            }*/
 
            _suspendStateChanges = true; 
            try
            { 
                if ((ClickMode == ClickMode.Hover) && IsEnabled)
                {
                    IsPressed = true; 
                    OnClick();
//                    e.Handled = true;
                } 
 
            }
            finally 
            {
                _suspendStateChanges = false;
                UpdateVisualState(); 
            }
        }
 
        /// <summary> 
        /// Handle the MouseLeave event.
        /// </summary> 
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">MouseEventArgs.</param>
        internal void OnMouseLeave(object sender, MouseEventArgs e) 
        {
            IsMouseOver = false;
            OnMouseLeave(e); 
        } 

        /// <summary> 
        /// Responds to the MouseLeave event.
        /// </summary>
        /// <param name="e">The event data for the MouseLeave event.</param> 
        protected virtual void OnMouseLeave(MouseEventArgs e)
        {
            if (e == null) 
            { 
                throw new ArgumentNullException("e");
            } 
/*            else if (e.Handled)
            {
                return; 
            }*/

            _suspendStateChanges = true; 
            try 
            {
                if ((ClickMode == ClickMode.Hover) && IsEnabled) 
                {
                    IsPressed = false;
//                    e.Handled = true; 
                }
            }
            finally 
            { 
                _suspendStateChanges = false;
                UpdateVisualState(); 
            }
        }
 
        /// <summary>
        /// Handle the MouseLeftButtonDown event.
        /// </summary> 
        /// <param name="sender">The source of the event.</param> 
        /// <param name="e">MouseButtonEventArgs.</param>
        internal void OnMouseLeftButtonDown(object sender, MouseButtonEventArgs e) 
        {
            _isMouseLeftButtonDown = true;
            OnMouseLeftButtonDown(e); 
        }

        /// <summary> 
        /// Responds to the MouseLeftButtonDown event. 
        /// </summary>
        /// <param name="e"> 
        /// The event data for the MouseLeftButtonDown event.
        /// </param>
        protected virtual void OnMouseLeftButtonDown(MouseButtonEventArgs e) 
        {
            if (e == null)
            { 
                throw new ArgumentNullException("e"); 
            }
            else if (e.Handled) 
            {
                return;
            } 

            if (!IsEnabled || (ClickMode == ClickMode.Hover))
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
 
            if (ClickMode == ClickMode.Press)
            {
                OnClick(); 
            }
        }
 
        /// <summary> 
        /// Handle the MouseLeftButtonUp event.
        /// </summary> 
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">MouseButtonEventArgs.</param>
        internal void OnMouseLeftButtonUp(object sender, MouseButtonEventArgs e) 
        {
            _isMouseLeftButtonDown = false;
            OnMouseLeftButtonUp(e); 
        } 

        /// <summary> 
        /// Responds to the MouseLeftButtonUp event.
        /// </summary>
        /// <param name="e"> 
        /// The event data for the MouseLeftButtonUp event.
        /// </param>
        protected virtual void OnMouseLeftButtonUp(MouseButtonEventArgs e) 
        { 
            if (e == null)
            { 
                throw new ArgumentNullException("e");
            }
            else if (e.Handled) 
            {
                return;
            } 
 
            if (!IsEnabled || (ClickMode == ClickMode.Hover))
            { 
                return;
            }
 
            e.Handled = true;
            if (!_isSpaceKeyDown && IsPressed && (ClickMode == ClickMode.Release))
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
        /// Handle the MouseMove event. 
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">MouseEventArgs.</param> 
        internal void OnMouseMove(object sender, MouseEventArgs e)
        {
            OnMouseMove(e); 
        } 

        /// <summary> 
        /// Responds to the MouseMove event.
        /// </summary>
        /// <param name="e">The event data for the MouseMove event.</param> 
        protected virtual void OnMouseMove(MouseEventArgs e)
        {
            if (e == null) 
            { 
                throw new ArgumentNullException("e");
            } 

            // Cache the latest mouse position.
            _mousePosition = e.GetPosition(this); 

/*            if (e.Handled)
            { 
                return; 
            }*/
 
            // Determine if the button is still pressed based on the mouse's
            // current position.
            if (_isMouseLeftButtonDown && 
                IsEnabled &&
                (ClickMode != ClickMode.Hover) &&
                _isMouseCaptured && 
                !_isSpaceKeyDown) 
            {
                IsPressed = IsValidMousePosition(); 
//                e.Handled = true;
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
