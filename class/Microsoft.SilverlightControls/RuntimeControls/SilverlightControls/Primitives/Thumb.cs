// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 


using System.Diagnostics; 
using System.Windows; 
using System.Windows.Input;
using System.Windows.Markup; 
using System.Windows.Media.Animation;
using System.Windows.Automation.Peers;
using System.Windows.Controls; 
using System.Windows.Media;
 
namespace System.Windows.Controls.Primitives 
{
    /// <summary> 
    /// Initializes a new instance of the Thumb class.
    /// </summary>
    [TemplateVisualState(Name = VisualStates.StateNormal, GroupName = VisualStates.GroupCommon)] 
    [TemplateVisualState(Name = VisualStates.StateMouseOver, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StatePressed, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateDisabled, GroupName = VisualStates.GroupCommon)] 
    [TemplateVisualState(Name = VisualStates.StateFocused, GroupName = VisualStates.GroupFocus)] 
    [TemplateVisualState(Name = VisualStates.StateUnfocused, GroupName = VisualStates.GroupFocus)]
    public sealed partial class Thumb : Control 
    {
        #region IsDragging
        /// <summary> 
        /// Gets whether the Thumb control has logical focus and mouse capture
        /// and the left mouse button is pressed.
        /// </summary> 
        public bool IsDragging 
        {
            get { return (bool)GetValue(IsDraggingProperty); } 
            internal set { SetValueImpl(IsDraggingProperty, value); }
        }
 
        /// <summary>
        /// Identifies the IsDragging dependency property.
        /// </summary> 
        public static readonly DependencyProperty IsDraggingProperty = 
            DependencyProperty.RegisterReadOnlyCore(
                "IsDragging", 
                typeof(bool),
                typeof(Thumb),
                new PropertyMetadata(OnIsDraggingPropertyChanged)); 

        /// <summary>
        /// IsDraggingProperty property changed handler. 
        /// </summary> 
        /// <param name="d">RangeBase that changed IsDragging.</param>
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
 
        #region IsEnabled

        /// <summary> 
        /// Called when the IsEnabled property changes.
        /// </summary>
        /// <param name="e">Property changed args</param> 
        private void OnIsEnabledChanged(object sender, DependencyPropertyChangedEventArgs e) 
        {
            if (!IsEnabled) 
            {
                IsMouseOver = false;
            } 
            UpdateVisualState();
        }
 
        #endregion IsEnabled 

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
            get { return (bool)GetValue(IsFocusedProperty); }
            internal set { SetValueImpl(IsFocusedProperty, value); }
        } 

        /// <summary>
        /// Identifies the IsFocused dependency property. 
        /// </summary> 
        public static readonly DependencyProperty IsFocusedProperty =
            DependencyProperty.RegisterReadOnlyCore( 
                "IsFocused",
                typeof(bool),
                typeof(Thumb), 
                null);
        #endregion IsFocused
 
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
            DefaultStyleKey = typeof(Thumb); 
            IsEnabledChanged += OnIsEnabledChanged;
        }
 
        /// <summary> 
        /// Apply a template to the thumb.
        /// </summary> 
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate(); 
            UpdateVisualState(false);
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
            base.OnMouseLeftButtonDown(e); 
            if (e.Handled)
            { 
                return;
            }
 
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
        /// Handle the LostMouseCapture event.
        /// </summary> 
        /// <param name="e">MouseEventArgs.</param>
        protected override void OnLostMouseCapture (MouseEventArgs e) 
        {
            base.OnLostMouseCapture (e);
            
            RaiseDragCompleted (false);
            IsDragging = false;
        }

        /// <summary>
        /// Handle the MouseEnter event. 
        /// </summary> 
        /// <param name="e">MouseEventArgs.</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")] 
        protected override void OnMouseEnter(MouseEventArgs e)
        {
            base.OnMouseEnter(e); 

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
            base.OnMouseLeave(e);

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
            base.OnMouseMove(e);
 
            if (IsDragging)
            {
                Debug.Assert(this.Parent is UIElement); 

                Point position = e.GetPosition((UIElement)this.Parent);
 
                if (position != _previousPosition) 
                {
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
 
        #region Focus Handlers
        /// <summary>
        /// Handle the GotFocus event. 
        /// </summary> 
        /// <param name="e">RoutedEventArgs.</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")] 
        protected override void OnGotFocus(RoutedEventArgs e)
        {
            base.OnGotFocus(e); 
            FocusChanged(HasFocus());
        }
 
        /// <summary> 
        /// Handle the LostFocus event.
        /// </summary> 
        /// <param name="e">RoutedEventArgs.</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        protected override void OnLostFocus(RoutedEventArgs e) 
        {
            base.OnLostFocus(e);
            FocusChanged(HasFocus()); 
        } 

        private void FocusChanged(bool hasFocus) 
        {
            IsFocused = hasFocus;
            UpdateVisualState(); 
        }
        #endregion
 
        #region AutomationPeer section

        protected override AutomationPeer OnCreateAutomationPeer ()
        {
            return new ThumbAutomationPeer (this);
        }

        #endregion

        #region Change State 

        /// <summary> 
        /// Change to the correct visual state for the thumb.
        /// </summary>
        internal void UpdateVisualState() 
        {
            UpdateVisualState(true);
        } 
 
        /// <summary>
        /// Change to the correct visual state for the thumb. 
        /// </summary>
        /// <param name="useTransitions">
        /// true to use transitions when updating the visual state, false to 
        /// snap directly to the new visual state.
        /// </param>
        internal void UpdateVisualState(bool useTransitions) 
        { 
            if (!IsEnabled)
            { 
                GoToState(useTransitions, VisualStates.StateDisabled);
            }
            else if (IsDragging) 
            {
                GoToState(useTransitions, VisualStates.StatePressed);
            } 
            else if (IsMouseOver) 
            {
                GoToState(useTransitions, VisualStates.StateMouseOver); 
            }
            else
            { 
                GoToState(useTransitions, VisualStates.StateNormal);
            }
 
            if (IsFocused && IsEnabled) 
            {
                GoToState(useTransitions, VisualStates.StateFocused); 
            }
            else
            { 
                GoToState(useTransitions, VisualStates.StateUnfocused);
            }
        } 
 
        internal bool GoToState(bool useTransitions, string stateName)
        { 
            Debug.Assert(stateName != null);
            return VisualStateManager.GoToState(this, stateName, useTransitions);
        } 
        #endregion Change State

        private bool HasFocus() 
        { 
            for (DependencyObject doh = FocusManager.GetFocusedElement() as DependencyObject;
                doh != null; 
                doh = VisualTreeHelper.GetParent(doh))
            {
                if (object.ReferenceEquals(doh, this)) 
                    return true;
            }
            return false; 
        } 

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
        #endregion Member Variables
    } 
}
