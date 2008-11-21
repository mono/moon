// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Windows.Automation.Peers;
using System.Windows.Input; 
using System.Windows.Markup; 
using System.Windows.Media.Animation;
 
namespace System.Windows.Controls.Primitives
{
    /// <summary> 
    /// Initializes a new instance of the Thumb class.
    /// </summary>
    [TemplatePart(Name = Thumb.ElementRootName, Type = typeof(FrameworkElement))] 
    [TemplatePart(Name = Thumb.StateNormalName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = Thumb.StateMouseOverName, Type = typeof(Storyboard))]
    [TemplatePart(Name = Thumb.StatePressedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = Thumb.StateDisabledName, Type = typeof(Storyboard))]
    public sealed class Thumb : Control
    { 
        #region IsDragging
        /// <summary>
        /// Gets whether the Thumb control has logical focus and mouse capture 
        /// and the left mouse button is pressed. 
        /// </summary>
        public bool IsDragging 
        {
            get { return (bool)GetValue(IsDraggingProperty); }
            internal set { SetValue(IsDraggingProperty, value); } 
        }

        /// <summary> 
        /// Identifies the IsDragging dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsDraggingProperty = 
            DependencyProperty.Register(
                "IsDragging",
                typeof(bool), 
                typeof(Thumb),
                new PropertyMetadata(OnIsDraggingPropertyChanged));
 
        /// <summary> 
        /// IsDraggingProperty property changed handler.
        /// </summary> 
        /// <param name="d">Thumb that changed IsDragging.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsDraggingPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            Thumb thumb = d as Thumb;
            Debug.Assert(thumb != null); 
 
            thumb.OnDraggingChanged();
        } 

        /// <summary>
        /// This method is invoked when the IsDragging property changes. 
        /// </summary>
        private void OnDraggingChanged()
        { 
            UpdateVisualState(); 
        }
 
        #endregion IsDragging

        #region Events
        /// <summary> 
        /// Identifies the DragStarted routed event. 
        /// </summary>
        public event DragStartedEventHandler DragStarted; 

        /// <summary>
        /// Identifies the DragDelta routed event. 
        /// </summary>
        public event DragDeltaEventHandler DragDelta;
 
        /// <summary> 
        /// Occurs when the Thumb control loses mouse capture.
        /// </summary> 
        public event DragCompletedEventHandler DragCompleted;
        #endregion Events
 
        #region Constructor
        /// <summary>
        /// Initializes a new instance of the Thumb class. 
        /// </summary> 
        public Thumb()
        { 
            IsEnabled = true;
            this.MouseEnter += delegate(object sender, MouseEventArgs e) { OnMouseEnter(e); };
            this.MouseLeave += delegate(object sender, MouseEventArgs e) { OnMouseLeave(e); }; 
            this.MouseLeftButtonDown += delegate(object sender, MouseButtonEventArgs e) { OnMouseLeftButtonDown(e); };
            this.MouseLeftButtonUp += delegate(object sender, MouseButtonEventArgs e) { OnMouseLeftButtonUp(e); };
            this.MouseMove += delegate(object sender, MouseEventArgs e) { OnMouseMove(e); }; 
        } 

        /// <summary> 
        /// Apply a template to the thumb.
        /// </summary>
        public override void OnApplyTemplate() 
        {
            base.OnApplyTemplate();
 
            // Get the parts 
            ElementRoot = GetTemplateChild(ElementRootName) as FrameworkElement;
 
            // Get the states
            if (ElementRoot != null)
            { 
                StateNormal = ElementRoot.Resources[StateNormalName] as Storyboard;
                StateMouseOver = ElementRoot.Resources[StateMouseOverName] as Storyboard;
                StatePressed = ElementRoot.Resources[StatePressedName] as Storyboard; 
                StateDisabled = ElementRoot.Resources[StateDisabledName] as Storyboard; 
            }
        } 
        #endregion Constructor

        #region Mouse Handlers 
        /// <summary>
        /// Handle the MouseLeftButtonDown event.
        /// </summary> 
        /// <param name="e">MouseButtonEventArgs.</param> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e) 
        {
            if (!IsDragging && IsEnabled)
            { 
                e.Handled = true;

                CaptureMouse(); 
                IsDragging = true; 

                Debug.Assert(this.Parent is UIElement); 

                _origin = _previousPosition = e.GetPosition((UIElement)this.Parent);
 
                // Raise the DragStarted event
                bool success = false;
                try 
                { 
                    DragStartedEventHandler handler = DragStarted;
                    if (handler != null) 
                    {
                        handler(this, new DragStartedEventArgs(_origin.X, _origin.Y));
                    } 
                    success = true;
                }
                finally 
                { 
                    // Cancel the drag if the DragStarted handler failed
                    if (!success) 
                    {
                        CancelDrag();
                    } 
                }
            }
        } 
 
        /// <summary>
        /// Handle the MouseLeftButtonUp event. 
        /// </summary>
        /// <param name="e">MouseButtonEventArgs.</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")] 
        protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e)
        {
            if (IsDragging && IsEnabled) 
            { 
                e.Handled = true;
                IsDragging = false; 
                ReleaseMouseCapture();
                RaiseDragCompleted(false);
            } 
        }

        /// <summary> 
        /// Handle the MouseEnter event. 
        /// </summary>
        /// <param name="e">MouseEventArgs.</param> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        protected override void OnMouseEnter(MouseEventArgs e)
        { 
            e.Handled = true;
            if (IsEnabled)
            { 
                IsMouseOver = true; 
                UpdateVisualState();
            } 
        }

        /// <summary> 
        /// Handle the MouseLeave event.
        /// </summary>
        /// <param name="e">MouseEventArgs.</param> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")] 
        protected override void OnMouseLeave(MouseEventArgs e)
        { 
            e.Handled = true;
            if (IsEnabled)
            { 
                IsMouseOver = false;
                UpdateVisualState();
            } 
        } 

        /// <summary> 
        /// Handle the MouseMove event.
        /// </summary>
        /// <param name="e">MouseEventArgs.</param> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        protected override void OnMouseMove(MouseEventArgs e)
        { 
            if (IsDragging) 
            {
                Debug.Assert(this.Parent is UIElement); 

                Point position = e.GetPosition((UIElement)this.Parent);
 
                if (position != _previousPosition)
                {
                    e.Handled = true; 
 
                    // Raise the DragDelta event
                    DragDeltaEventHandler handler = DragDelta; 
                    if (handler != null)
                    {
                        handler(this, new DragDeltaEventArgs(position.X - _previousPosition.X, position.Y - _previousPosition.Y)); 
                    }

                    _previousPosition = position; 
                } 
            }
        } 
        #endregion Mouse Handlers

        #region Change State 

        /// <summary>
        /// Change to the correct visual state for the thumb. 
        /// </summary> 
        internal void UpdateVisualState()
        { 
            if (!IsEnabled)
            {
                ChangeVisualState(StateDisabled ?? StateNormal); 
            }
            else if (IsDragging)
            { 
                ChangeVisualState(StatePressed ?? StateMouseOver ?? StateNormal); 
            }
            else 
            {
                if (IsMouseOver)
                { 
                    ChangeVisualState(StateMouseOver ?? StateNormal);
                }
                else 
                { 
                    ChangeVisualState(StateNormal);
                } 
            }
        }
 
        /// <summary>
        /// Change the visual state of the thumb.
        /// </summary> 
        /// <param name="state">Next visual state of the thumb.</param> 
        /// <remarks>
        /// This method should not be called by controls to force a change to 
        /// the current visual state.  UpdateVisualState is preferred because
        /// it properly handles suspension of state changes.
        /// </remarks> 
        private void ChangeVisualState(Storyboard state)
        {
            Storyboard previousState = _currentState; 
            if (state == previousState) 
            {
                return; 
            }

            if (state != null) 
            {
                if (previousState != null)
                { 
                    previousState.Stop(); 
                }
                _currentState = state; 
                state.Begin();
            }
        } 
        #endregion Change State

        #region Drag Cancel/Complete 
        /// <summary> 
        /// Cancel a drag operation if it is currently in progress.
        /// </summary> 
        public void CancelDrag()
        {
            if (IsDragging) 
            {
                IsDragging = false;
                RaiseDragCompleted(true); 
            } 
        }
 
        /// <summary>
        /// Raise the DragCompleted event.
        /// </summary> 
        /// <param name="canceled">
        /// A Boolean value that indicates whether the drag operation was
        /// canceled by a call to the CancelDrag method. 
        /// </param> 
        private void RaiseDragCompleted(bool canceled)
        { 
            DragCompletedEventHandler handler = DragCompleted;
            if (handler != null)
            { 
                DragCompletedEventArgs args = new DragCompletedEventArgs(
                    _previousPosition.X - _origin.X,
                    _previousPosition.Y - _origin.Y, 
                    canceled); 
                handler(this, args);
            } 
        }
        #endregion Drag Cancel/Complete
 
	protected override AutomationPeer OnCreateAutomationPeer ()
	{
		throw new NotImplementedException ();
	}

	protected override void OnGotFocus (RoutedEventArgs e)
	{
	}

	protected override void OnLostFocus (RoutedEventArgs e)
	{
	}

        #region Template Parts
        /// <summary>
        /// Root of the thumb template. 
        /// </summary> 
        internal FrameworkElement ElementRoot { get; set; }
        internal const string ElementRootName = "RootElement"; 

        /// <summary>
        /// Transition into the normal state in the thumb template. 
        /// </summary>
        internal Storyboard StateNormal { get; set; }
        internal const string StateNormalName = "Normal State"; 
 
        /// <summary>
        /// Transition into the MouseOver state in the thumb template. 
        /// </summary>
        internal Storyboard StateMouseOver { get; set; }
        internal const string StateMouseOverName = "MouseOver State"; 

        /// <summary>
        /// Transition into the Pressed state in the thumb template. 
        /// </summary> 
        internal Storyboard StatePressed { get; set; }
        internal const string StatePressedName = "Pressed State"; 

        /// <summary>
        /// Transition into the Disabled state in the thumb template. 
        /// </summary>
        internal Storyboard StateDisabled { get; set; }
        internal const string StateDisabledName = "Disabled State"; 
        #endregion Template Parts 

        #region Member Variables 
        /// <summary>
        /// Whether the mouse is currently over the control
        /// </summary> 
        internal bool IsMouseOver { get; set; }
        /// <summary>
        /// Origin of the thumb's drag operation. 
        /// </summary> 
        internal Point _origin;
        /// <summary> 
        /// Last position of the thumb while during a drag operation.
        /// </summary>
        internal Point _previousPosition; 
        /// <summary>
        /// Current state of the control
        /// </summary> 
        internal Storyboard _currentState; 
        #endregion Member Variables
    } 
}
