//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using Mono;
using System.ComponentModel;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Automation.Peers;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Documents;
using System.Collections.Generic;
using System.Collections.Specialized;

namespace System.Windows.Controls {
	
	[TemplateVisualStateAttribute (Name = "Disabled",	GroupName = "CommonStates")]
	[TemplateVisualStateAttribute (Name = "Normal",		GroupName = "CommonStates")]
	[TemplateVisualStateAttribute (Name = "MouseOver",	GroupName = "CommonStates")]
	[TemplateVisualStateAttribute (Name = "ReadOnly",	GroupName = "CommonStates")]
	[TemplateVisualStateAttribute (Name = "Focused",	GroupName = "FocusStates")]
	[TemplateVisualStateAttribute (Name = "Unfocused",	GroupName = "FocusStates")]
	[TemplateVisualStateAttribute (Name = "InvalidFocused", GroupName = "ValidationStates")]
	[TemplateVisualStateAttribute (Name = "InvalidUnfocused", GroupName = "ValidationStates")]
	[TemplateVisualStateAttribute (Name = "Valid",		GroupName = "ValidationStates")]
	[TemplatePartAttribute (Name = "ContentElement",	Type = typeof (FrameworkElement))]
	public partial class TextBox : Control {
		object contentElement;
		
		bool IsFocused {
			get; set;
		}
		
		bool IsMouseOver {
			get; set;
		}
		
		[MonoTODO]
		public double BaselineOffset {
			get {
				Console.WriteLine ("System.Windows.Controls.TextBox.get_BaselineOffset: TODO");
				//throw new NotImplementedException ();
				return Double.NaN;
			}
		}

		static TextBox ()
		{
			IsReadOnlyProperty.AddPropertyChangeCallback (IsReadOnlyChanged);
			TextProperty.AddPropertyChangeCallback (TextPropertyChanged);
		}

		private new void Initialize ()
		{
			// FIXME: Should use Events.AddOnEventHandler or something similar.
			CursorPositionChanged += OnCursorPositionChanged;

			var errors = Validation.GetErrors (this) as INotifyCollectionChanged;
			errors.CollectionChanged += (sender, args) => { ChangeVisualState (true); };
		}

		static void IsReadOnlyChanged (DependencyObject sender, DependencyPropertyChangedEventArgs args)
		{
			TextBox textbox = sender as TextBox;
			textbox.ChangeVisualState (false);

			if (textbox.AutomationPeer != null) 
				textbox.AutomationPeer.RaisePropertyChangedEvent (ValuePatternIdentifiers.IsReadOnlyProperty, 
				                                                  args.OldValue,
										  args.NewValue);
		}

		static void TextPropertyChanged (DependencyObject sender, DependencyPropertyChangedEventArgs args)
		{
			((TextBox) sender).RaiseUIATextChanged (args);
		}

		internal override void InvokeIsEnabledPropertyChanged ()
		{
			base.InvokeIsEnabledPropertyChanged ();
			ChangeVisualState (false);
		}

		internal override void InvokeOnApplyTemplate ()
		{
			base.InvokeOnApplyTemplate ();
			ChangeVisualState (false);
		}

		protected override void OnKeyDown (KeyEventArgs k)
		{
			// Chain up to our parent first, so that TabNavigation
			// works as well as allowing developers to filter our
			// input.
			base.OnKeyDown (k);
			NativeMethods.text_box_base_on_key_down (native, k.NativeHandle);
		}

		internal override void PostOnKeyDown (KeyEventArgs k)
		{
			base.PostOnKeyDown (k);
			NativeMethods.text_box_base_post_on_key_down (native, k.NativeHandle);
		}
		
		protected override void OnKeyUp (KeyEventArgs k)
		{
			base.OnKeyUp (k);
			NativeMethods.text_box_base_on_key_up (native, k.NativeHandle);
		}
		
		protected override void OnMouseLeftButtonDown (MouseButtonEventArgs e)
		{
			NativeMethods.text_box_base_on_mouse_left_button_down (native, e.NativeHandle);
			base.OnMouseLeftButtonDown (e);
		}
		
		protected override void OnMouseLeftButtonUp (MouseButtonEventArgs e)
		{
			NativeMethods.text_box_base_on_mouse_left_button_up (native, e.NativeHandle);
			base.OnMouseLeftButtonUp (e);
		}
		
		protected override void OnMouseMove (MouseEventArgs e)
		{
			NativeMethods.text_box_base_on_mouse_move (native, e.NativeHandle);
			base.OnMouseMove (e);
		}
		
		protected override void OnMouseEnter (MouseEventArgs e)
		{
			IsMouseOver = true;
			ChangeVisualState ();
			base.OnMouseEnter (e);
		}
		
		protected override void OnMouseLeave (MouseEventArgs e)
		{
			IsMouseOver = false;
			ChangeVisualState ();
			base.OnMouseLeave (e);
		}

		protected override void OnTextInput (TextCompositionEventArgs e)
		{
			base.OnTextInput (e);
		}

		protected override void OnTextInputStart (TextCompositionEventArgs e)
		{
			base.OnTextInputStart (e);
		}

		protected override void OnTextInputUpdate (TextCompositionEventArgs e)
		{
			base.OnTextInputUpdate (e);
		}

		protected override void OnGotFocus (RoutedEventArgs e)
		{
			IsFocused = true;
			ChangeVisualState ();
			base.OnGotFocus (e);
			NativeMethods.text_box_base_on_got_focus (native, e.NativeHandle);
		}
		
		protected override void OnLostFocus (RoutedEventArgs e)
		{
			IsFocused = false;
			ChangeVisualState ();
			base.OnLostFocus (e);
			NativeMethods.text_box_base_on_lost_focus (native, e.NativeHandle);
		}

		protected override void OnLostMouseCapture (MouseEventArgs e)
		{
			base.OnLostMouseCapture (e);
		}

		public string Text {
			get {
				return (string)GetValue (TextProperty) ?? "";
			}
			set {
				if (value == null)
					throw new ArgumentNullException ("Text cannot be null");
				SetValue (TextProperty, value);
			}
		}
		
		public void Select (int start, int length)
		{
			if (start < 0)
				throw new ArgumentOutOfRangeException ("start");
			
			if (length < 0)
				throw new ArgumentOutOfRangeException ("length");
			
			NativeMethods.text_box_base_select (this.native, start, length);
		}
		
		public void SelectAll ()
		{
			NativeMethods.text_box_base_select_all (native);
		}

		[EditorBrowsable (EditorBrowsableState.Never)]
		public Rect GetRectFromCharacterIndex (int charIndex)
		{
			EnsureDesignMode ();
			// design-mode is not really supported by Moonlight but some tests requires it to "work"
			return Rect.Empty;
		}

		[EditorBrowsable (EditorBrowsableState.Never)]
		public Rect GetRectFromCharacterIndex (int charIndex, bool trailingEdge)
		{
			EnsureDesignMode ();
			// design-mode is not really supported by Moonlight but some tests requires it to "work"
			return Rect.Empty;
		}

		void OnCursorPositionChanged (object sender, CursorPositionChangedEventArgs args)
		{
			if (contentElement == null)
				contentElement = GetTemplateChild ("ContentElement");
			
			if (contentElement != null && contentElement is ScrollViewer) {
				ScrollViewer scrollview = contentElement as ScrollViewer;
				double offset = scrollview.HorizontalOffset;
				
				// Note: for horizontal scrollage, we offset by 1.0 pixel for the width of the cursor ibeam
				
				if (args.CursorX < offset) {
					// need to scroll to the left a bit
					scrollview.ScrollToHorizontalOffset (Math.Max (args.CursorX - 1.0, 0.0));
				} else if (args.CursorX > offset + scrollview.ViewportWidth) {
					// need to scroll to the right
					offset = (args.CursorX + 1.0) - scrollview.ViewportWidth;
					scrollview.ScrollToHorizontalOffset (offset);
				}
				
				offset = scrollview.VerticalOffset;
				if (args.CursorY < offset) {
					// need to scroll up a bit
					scrollview.ScrollToVerticalOffset (args.CursorY);
				} else if (args.CursorY + args.CursorHeight > offset + scrollview.ViewportHeight) {
					// need to scroll down a bit
					offset = (args.CursorY + args.CursorHeight) - Math.Max (args.CursorHeight, scrollview.ViewportHeight);
					scrollview.ScrollToVerticalOffset (offset);
				}
			}
		}
		
		event CursorPositionChangedEventHandler CursorPositionChanged {
			add {
				RegisterEvent (EventIds.TextBoxBase_CursorPositionChangedEvent, value,
					       Events.CreateCursorPositionChangedEventHandlerDispatcher (value));
			}
			remove {
				UnregisterEvent (EventIds.TextBoxBase_CursorPositionChangedEvent, value);
			}
		}
		
		void ChangeVisualState ()
		{
			ChangeVisualState (true);
		}

		void ChangeVisualState (bool useTransitions)
		{
			if (!IsEnabled) {
				VisualStateManager.GoToState (this, "Disabled", useTransitions);
			} else if (IsReadOnly) {
				VisualStateManager.GoToState (this, "ReadOnly", useTransitions);
			} else if (IsMouseOver) {
				VisualStateManager.GoToState (this, "MouseOver", useTransitions);
			} else {
				VisualStateManager.GoToState (this, "Normal", useTransitions);
			}

			if (Validation.GetErrors (this).Count > 0) {
				VisualStateManager.GoToState (this, IsFocused ? "InvalidFocused" : "InvalidUnfocused", useTransitions);
			} else {
				VisualStateManager.GoToState (this, "Valid", useTransitions);
			}
			
			if (IsFocused) {
				VisualStateManager.GoToState (this, "Focused", useTransitions);
			} else {
				VisualStateManager.GoToState (this, "Unfocused", useTransitions);
			}
		}

		// InputScope and Watermark properties are not supported outside of design mode
		static void EnsureDesignMode ()
		{
			// NOTE:
			// If you ended up here from drt 539 you should know
			// it doesn't pass on Silverlight but MobileStubsTests
			// does so tread lightly

			if (//Int32.Parse (Deployment.Current.RuntimeVersion.Split('.')[0]) < 4 &&
			    !DesignerProperties.GetIsInDesignMode (Application.Current.RootVisual))
				throw new NotImplementedException ();
		}

		[EditorBrowsable (EditorBrowsableState.Never)]
		public InputScope InputScope {
			get {
				EnsureDesignMode ();
				return (InputScope) GetValue (InputScopeProperty);
			}
			set {
				EnsureDesignMode ();
				SetValue (InputScopeProperty, value);
			}
		}

		[EditorBrowsable (EditorBrowsableState.Never)]
		public object Watermark {
			get {
				EnsureDesignMode ();
				return GetValue (WatermarkProperty);
			}
			set {
				EnsureDesignMode ();
				SetValue (WatermarkProperty, value);
			}
		}

		protected override AutomationPeer OnCreateAutomationPeer ()
		{
			return new TextBoxAutomationPeer (this);
		}

		#region UIA Events 	 
	  	 
		internal event DependencyPropertyChangedEventHandler UIATextChanged;

		internal void RaiseUIATextChanged (DependencyPropertyChangedEventArgs args)
		{
			if (UIATextChanged != null)
				UIATextChanged (this, args);
		}

		#endregion
	}
}
