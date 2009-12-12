// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 


using System; 
using System.Windows; 
using System.Diagnostics;
using System.Windows.Input; 
using System.Windows.Media.Animation;
using System.Windows.Controls;

namespace System.Windows.Controls.Primitives
{ 
    /// <summary> 
    /// Represents the base class for all Button controls.
    /// </summary> 
    public abstract class ButtonBase : ContentControl
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
            DependencyProperty.RegisterCore(
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
                throw new ArgumentException();
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
            internal set { SetValueImpl(IsFocusedProperty, value); } 
        }

        /// <summary> 
        /// Identifies the IsFocused dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsFocusedProperty = 
            DependencyProperty.RegisterReadOnlyCore(
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
            internal set { SetValueImpl(IsMouseOverProperty, value); } 
        }

        /// <summary> 
        /// Identifies the IsMouseOver dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsMouseOverProperty = 
            DependencyProperty.RegisterReadOnlyCore(
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
            protected internal set { SetValueImpl(IsPressedProperty, value); } 
        }
 
        /// <summary>
        /// Identifies the IsPressed dependency property.
        /// </summary> 
        public static readonly DependencyProperty IsPressedProperty =
            DependencyProperty.RegisterReadOnlyCore(
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
 
        #region Template Parts
        /// <summary> 
        /// Normal state of the Button.
        /// </summary>
        internal const string StateNormal = "Normal"; 

        /// <summary>
        /// MouseOver state of the Button. 
        /// </summary> 
        internal const string StateMouseOver = "MouseOver";
 
        /// <summary>
        /// Pressed state of the Button.
        /// </summary> 
        internal const string StatePressed = "Pressed";

        /// <summary> 
        /// Disabled state of the Button. 
        /// </summary>
        internal const string StateDisabled = "Disabled"; 

        /// <summary>
        /// Common state group of the Button. 
        /// </summary>
        internal const string GroupCommon = "CommonStates";
 
        /// <summary> 
        /// Focused state of the Button.
        /// </summary> 
        internal const string StateFocused = "Focused";

        /// <summary> 
        /// Unfocused state of the Button.
        /// </summary>
        internal const string StateUnfocused = "Unfocused"; 
 
        /// <summary>
        /// Focus state group of the Button. 
        /// </summary>
        internal const string GroupFocus = "FocusStates";
        #endregion Template Parts 

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
            // Attach the necessary events to their virtual counterparts
            Loaded += delegate { _isLoaded = true; UpdateVisualState(false); }; 
            IsEnabledChanged += OnIsEnabledChanged; 
        }
 
        /// <summary>
        /// Used by ButtonAutomationPeer to invoke the click event
        /// </summary> 
        internal void AutomationButtonBaseClick()
        {
            this.OnClick(); 
        } 
        /// <summary>
        /// Update the current visual state of the button. 
        /// </summary>
        internal void UpdateVisualState()
        { 
            UpdateVisualState(true);
        }
 
        /// <summary> 
        /// Update the current visual state of the button.
        /// </summary> 
        /// <param name="useTransitions">
        /// true to use transitions when updating the visual state, false to
        /// snap directly to the new visual state. 
        /// </param>
        internal void UpdateVisualState(bool useTransitions)
        { 
            if (!_suspendStateChanges) 
            {
                ChangeVisualState(useTransitions); 
            }
        }
 
        /// <summary>
        /// Change to the correct visual state for the button.
        /// </summary> 
        /// <param name="useTransitions"> 
        /// true to use transitions when updating the visual state, false to
        /// snap directly to the new visual state. 
        /// </param>
        internal virtual void ChangeVisualState(bool useTransitions)
        { 
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
// OriginalSource needs to be set, but that's not possible outside System.Windows.dll, so SL2 controls don't do it
//                handler(this, new RoutedEventArgs()); 
                handler(this, new RoutedEventArgs { OriginalSource = this });
            } 
        }
 
        /// <summary>
        /// Called when the IsEnabled property changes.
        /// </summary> 
        /// <param name="e">Property changed args</param>
        private void OnIsEnabledChanged(object sender, DependencyPropertyChangedEventArgs e)
        { 
            _suspendStateChanges = true; 

            try 
            {
                if (!IsEnabled)
                { 
                    IsPressed = false;
                    IsMouseOver = false;
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
        /// Responds to the GotFocus event.
        /// </summary>
        /// <param name="e">The event data for the GotFocus event.</param> 
        protected override void OnGotFocus(RoutedEventArgs e) 
        {
            base.OnGotFocus(e); 

            IsFocused = true;
 
            UpdateVisualState();
        }
 
        /// <summary> 
        /// Responds to the LostFocus event.
        /// </summary> 
        /// <param name="e">The event data for the LostFocus event.</param>
        protected override void OnLostFocus(RoutedEventArgs e)
        { 
            base.OnLostFocus(e);

            IsFocused = false; 
 
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
        /// Responds to the LostMouseCapture event.
        /// </summary> 
        /// <param name="e">The event data for the LostMouseCapture event.</param>
	protected override void OnLostMouseCapture (MouseEventArgs e)
	{
		base.OnLostMouseCapture (e);
		
		ReleaseMouseCaptureInternal ();
		IsPressed = false;
	}

        /// <summary> 
        /// Responds to the KeyDown event.
        /// </summary> 
        /// <param name="e">The event data for the KeyDown event.</param>
        protected override void OnKeyDown(KeyEventArgs e)
        { 
            base.OnKeyDown(e);
            if (e.Handled)
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
                    // captured or if it had been pressed previously.
                    if (!_isMouseCaptured && !_isSpaceKeyDown)
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
                else if (key == Key.Enter)
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
            base.OnKeyUp(e); 
            if (e.Handled)
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
        /// Responds to the MouseEnter event. 
        /// </summary>
        /// <param name="e">The event data for the MouseEnter event.</param>
        protected override void OnMouseEnter(MouseEventArgs e) 
        { 
            base.OnMouseEnter(e);
 
            IsMouseOver = true;

            _suspendStateChanges = true; 
            try
            {
                if ((ClickMode == ClickMode.Hover) && IsEnabled) 
                { 
                    IsPressed = true;
                    OnClick(); 
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
            base.OnMouseLeave(e); 

            IsMouseOver = false; 

            _suspendStateChanges = true;
 
            try
            {
                if ((ClickMode == ClickMode.Hover) && IsEnabled) 
                { 
                    IsPressed = false;
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
            base.OnMouseLeftButtonDown(e);
            if (e.Handled) 
            {
                return;
            } 

            _isMouseLeftButtonDown = true;
 
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
        /// Responds to the MouseLeftButtonUp event.
        /// </summary>
        /// <param name="e"> 
        /// The event data for the MouseLeftButtonUp event.
        /// </param>
        protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e) 
        { 
            base.OnMouseLeftButtonUp(e);
            if (e.Handled) 
            {
                return;
            } 

            _isMouseLeftButtonDown = false;
 
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
        /// Responds to the MouseMove event.
        /// </summary> 
        /// <param name="e">The event data for the MouseMove event.</param>
        protected override void OnMouseMove(MouseEventArgs e)
        { 
            base.OnMouseMove(e);
            // Cache the latest mouse position.
            _mousePosition = e.GetPosition(this); 
 
            // Determine if the button is still pressed based on the mouse's
            // current position. 
            if (_isMouseLeftButtonDown &&
                IsEnabled &&
                (ClickMode != ClickMode.Hover) && 
                _isMouseCaptured &&
                !_isSpaceKeyDown)
            { 
                IsPressed = IsValidMousePosition(); 
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
 
        internal bool GoToState(bool useTransitions, string stateName)
        { 
            Debug.Assert(stateName != null);
            return VisualStateManager.GoToState(this, stateName, useTransitions);
        } 

        internal static DependencyObject GetVisualRoot(DependencyObject d)
        { 
            DependencyObject root = d; 
            for (; ; )
            { 
                FrameworkElement element = root as FrameworkElement;
                if (element == null)
                { 
                    break;
                }
 
                DependencyObject parent = element.Parent as DependencyObject; 
                if (parent == null)
                { 
                    break;
                }
 
                root = parent;
            }
            return root; 
        } 
    }
} 
