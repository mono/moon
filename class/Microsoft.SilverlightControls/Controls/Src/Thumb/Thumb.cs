// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Windows.Automation.Peers;
using System.Windows.Input; 
using System.Windows.Markup; 
using System.Windows.Media.Animation;

// use the cool stuff from SL Toolkit
using Microsoft.Windows.Controls;
 
namespace System.Windows.Controls.Primitives
{
    /// <summary> 
    /// Initializes a new instance of the Thumb class.
    /// </summary>
	[TemplateVisualState (Name = VisualStates.StateNormal, GroupName = VisualStates.GroupCommon)]
	[TemplateVisualState (Name = VisualStates.StateMouseOver, GroupName = VisualStates.GroupCommon)]
	[TemplateVisualState (Name = VisualStates.StatePressed, GroupName = VisualStates.GroupCommon)]
	[TemplateVisualState (Name = VisualStates.StateDisabled, GroupName = VisualStates.GroupCommon)]

	[TemplateVisualState (Name = VisualStates.StateFocused, GroupName = VisualStates.GroupFocus)]
	[TemplateVisualState (Name = VisualStates.StateUnfocused, GroupName = VisualStates.GroupFocus)]
	public sealed class Thumb : Control, IUpdateVisualState {

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
            UpdateVisualState (true); 
        }
 
        #endregion IsDragging

        #region IsFocused 
	// copy-pasted from Slider.cs

        /// <summary>
        /// Gets a value that determines whether this element has logical focus.
        /// </summary> 
        public bool IsFocused 
        {
            get { return (bool)GetValue(IsFocusedProperty); } 
            internal set { SetValue(IsFocusedProperty, value); }
        }
 
        /// <summary>
        /// Identifies the IsFocused dependency property.
        /// </summary> 
        public static readonly DependencyProperty IsFocusedProperty = 
            DependencyProperty.Register(
                "IsFocused", 
                typeof(bool),
                typeof(Thumb),
                new PropertyMetadata(OnIsFocusedPropertyChanged)); 

        /// <summary>
        /// IsFocusedProperty property changed handler. 
        /// </summary> 
        /// <param name="d">Thumb that changed IsFocused.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnIsFocusedPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Thumb t = d as Thumb; 
            Debug.Assert(t != null);

            if (t.ElementRoot != null) 
            { 
                t.UpdateVisualState (true); 
            } 
        }

	protected override void OnGotFocus (RoutedEventArgs e)
	{
		if (Interaction.AllowGotFocus (e)) {
			Interaction.OnGotFocusBase ();
		}
		// we're not calling base on purpose
		// SL2 does not fire the GotFocus event on Thumbs
	}

	protected override void OnLostFocus (RoutedEventArgs e)
	{
		if (Interaction.AllowLostFocus (e)) {
			Interaction.OnLostFocusBase ();
		}
		// we're not calling base on purpose
		// SL2 does not fire the GotFocus event on Thumbs
	}

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
 
		#region Visual state management
		/// <summary>
		/// Gets or sets the helper that provides all of the standard
		/// interaction functionality.
		/// </summary>
		private InteractionHelper Interaction { get; set; }

		/// <summary>
		/// Update the visual state of the control.
		/// </summary>
		/// <param name="useTransitions">
		/// A value indicating whether to automatically generate transitions to
		/// the new state, or instantly transition to the new state.
		/// </param>
		void IUpdateVisualState.UpdateVisualState (bool useTransitions)
		{
			UpdateVisualState (useTransitions);
		}
		#endregion

        #region Constructor
        /// <summary>
        /// Initializes a new instance of the Thumb class. 
        /// </summary> 
        public Thumb()
        { 
		this.DefaultStyleKey = typeof (Thumb);
		Interaction = new InteractionHelper (this);
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
		Interaction.OnApplyTemplateBase ();
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
            if (!IsDragging && Interaction.AllowMouseLeftButtonDown (e))
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
			Interaction.OnMouseLeftButtonDownBase ();
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
            if (IsDragging && Interaction.AllowMouseLeftButtonUp (e)) 
            { 
                e.Handled = true;
                IsDragging = false; 
                ReleaseMouseCapture();
                RaiseDragCompleted(false);

		Interaction.OnMouseLeftButtonUpBase ();
            } 
        }

        /// <summary> 
        /// Handle the MouseEnter event. 
        /// </summary>
        /// <param name="e">MouseEventArgs.</param> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        protected override void OnMouseEnter(MouseEventArgs e)
        { 
		if (Interaction.AllowMouseEnter (e))
			Interaction.OnMouseEnterBase ();
        }

        /// <summary> 
        /// Handle the MouseLeave event.
        /// </summary>
        /// <param name="e">MouseEventArgs.</param> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")] 
        protected override void OnMouseLeave(MouseEventArgs e)
        { 
		if (Interaction.AllowMouseLeave (e))
			Interaction.OnMouseLeaveBase ();
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
//                    e.Handled = true; 
 
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
        internal void UpdateVisualState (bool useTransitions)
        { 
		// all states are managed by the default InteractionHelper
		Interaction.UpdateVisualStateBase (useTransitions);
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

        #region Template Parts
        /// <summary>
        /// Root of the thumb template. 
        /// </summary> 
        internal FrameworkElement ElementRoot { get; set; }
        internal const string ElementRootName = "RootElement"; 
        #endregion Template Parts 

        #region Member Variables 
        /// <summary>
        /// Whether the mouse is currently over the control
        /// </summary> 
	internal bool IsMouseOver {
		get { return Interaction.IsMouseOver; }
	}
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
