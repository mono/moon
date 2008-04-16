//
// UIElemtn.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
//
// Copyright 2007 Novell, Inc.
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
using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Input;
using Mono;

namespace System.Windows {
	
	public abstract class UIElement : Visual {
	        public static readonly DependencyProperty ClipProperty;
	        public static readonly DependencyProperty CursorProperty;
	        public static readonly DependencyProperty IsHitTestVisibleProperty;
	        public static readonly DependencyProperty OpacityMaskProperty;
	        public static readonly DependencyProperty OpacityProperty;
	        public static readonly DependencyProperty RenderTransformOriginProperty;
	        public static readonly DependencyProperty RenderTransformProperty;
	        public static readonly DependencyProperty ResourcesProperty;
	        public static readonly DependencyProperty TagProperty;
	        public static readonly DependencyProperty TriggersProperty;
	        public static readonly DependencyProperty VisibilityProperty;
	        public static readonly DependencyProperty ZIndexProperty;

		static UIElement ()
		{
	        	ClipProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "Clip", typeof (Geometry));
			CursorProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "Cursor", typeof (Cursors));
			IsHitTestVisibleProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "IsHitTestVisible", typeof (bool));
			OpacityMaskProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "OpacityMask", typeof (Brush));
	        	OpacityProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "Opacity", typeof (double));
			RenderTransformOriginProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "RenderTransformOrigin", typeof (Point));
	        	RenderTransformProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "RenderTransform", typeof (Transform));
			ResourcesProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "Resources", typeof (ResourceDictionary));
			TagProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "Tag", typeof (string));
	        	TriggersProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "Triggers", typeof (TriggerCollection));
			VisibilityProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "Visibility", typeof (Visibility));
			ZIndexProperty = DependencyProperty.Lookup (Kind.UIELEMENT, "ZIndex", typeof (int));
		}
			
		public UIElement () : base (NativeMethods.uielement_new ())
		{
		}
		
		internal UIElement (IntPtr raw) : base (raw)
		{
		}
		
		public Geometry Clip {
			get {
				return (Geometry) GetValue (ClipProperty);
			}
				
			set {
				SetValue (ClipProperty, value);
			}
		}
		
		public Cursors Cursor {
			get {
				return (Cursors) GetValue (CursorProperty);
			}
				
			set {
				SetValue (CursorProperty, value);
			}
		}
		
		public bool IsHitTestVisible {
			get {
				return (bool) GetValue (IsHitTestVisibleProperty);
			}
				
			set {
				SetValue (IsHitTestVisibleProperty, value);
			}
		}
		
		public double Opacity {
			get {
				return (double) GetValue (OpacityProperty);
			}
				
			set {
				SetValue (OpacityProperty, value);
			}
		}
		
		public Brush OpacityMask {
			get {
				return (Brush) GetValue (OpacityMaskProperty);
			}
				
			set {
				SetValue (OpacityMaskProperty, value);
			}
		}
		
		public Transform RenderTransform {
			get {
				return (Transform) GetValue (RenderTransformProperty);
			}
				
			set {
				SetValue (RenderTransformProperty, value);
			}
		}
		
		public Point RenderTransformOrigin {
			get {
				return (Point) GetValue (RenderTransformOriginProperty);
			}
				
			set {
				SetValue (RenderTransformOriginProperty, value);
			}
		}
		
		public ResourceDictionary Resources {
			get {
				return (ResourceDictionary) GetValue (ResourcesProperty);
			}
				
			set {
				SetValue (ResourcesProperty, value);
			}
		}

		public string Tag {
			get {
				return (string) GetValue (TagProperty);
			}

			set {
				SetValue (TagProperty, value);
			}
		}

		public TriggerCollection Triggers {
			get {
				return (TriggerCollection) GetValue (TriggersProperty);
			}
				
			set {
				SetValue (TriggersProperty, value);
			}
		}
		
		public Visibility Visibility {
			get {
				return (Visibility) GetValue (VisibilityProperty);
			}
				
			set {
				SetValue (VisibilityProperty, value);
			}
		}

		/* MS has removed this from the public API as of 1.0 RC1 / 1.1 refresh... not sure why */
		internal int ZIndex {
			get {
				return (int) GetValue (ZIndexProperty);
			}
				
			set {
				SetValue (ZIndexProperty, value);
			}
		}

		static object GotFocusEvent = new object ();
		static object LostFocusEvent = new object ();
		static object LoadedEvent = new object ();
		static object KeyDownEvent = new object ();
		static object KeyUpEvent = new object ();
		static object MouseEnterEvent = new object ();
		static object MouseLeaveEvent = new object ();
		static object MouseLeftButtonDownEvent = new object ();
		static object MouseLeftButtonUpEvent = new object ();
		static object MouseMoveEvent = new object ();

		public event EventHandler GotFocus {
			add {
				if (events[GotFocusEvent] == null)
					Events.AddHandler (this, "GotFocus", Events.got_focus);
				events.AddHandler (GotFocusEvent, value);
			}
			remove {
				events.RemoveHandler (GotFocusEvent, value);
				if (events[GotFocusEvent] == null)
					Events.RemoveHandler (this, "GotFocus", Events.got_focus);
			}
		}

		public event EventHandler LostFocus {
			add {
				if (events[LostFocusEvent] == null)
					Events.AddHandler (this, "LostFocus", Events.lost_focus);
				events.AddHandler (LostFocusEvent, value);
			}
			remove {
				events.RemoveHandler (LostFocusEvent, value);
				if (events[LostFocusEvent] == null)
					Events.RemoveHandler (this, "LostFocus", Events.lost_focus);
			}
		}

		public event EventHandler Loaded {
			add {
				if (events[LoadedEvent] == null)
					Events.AddHandler (this, "Loaded", Events.loaded);
				events.AddHandler (LoadedEvent, value);
			}
			remove {
				events.RemoveHandler (LoadedEvent, value);
				if (events[LoadedEvent] == null)
					Events.RemoveHandler (this, "Loaded", Events.loaded);
			}
		}
			
		public event KeyboardEventHandler KeyDown {
			add {
				if (events[KeyDownEvent] == null)
					Events.AddHandler (this, "KeyDown", Events.key_down);
				events.AddHandler (KeyDownEvent, value);
			}
			remove {
				events.RemoveHandler (KeyDownEvent, value);
				if (events[KeyDownEvent] == null)
					Events.RemoveHandler (this, "KeyDown", Events.key_down);
			}
		}

		public event KeyboardEventHandler KeyUp {
			add {
				if (events[KeyUpEvent] == null)
					Events.AddHandler (this, "KeyUp", Events.key_up);
				events.AddHandler (KeyUpEvent, value);
			}
			remove {
				events.RemoveHandler (KeyUpEvent, value);
				if (events[KeyUpEvent] == null)
					Events.RemoveHandler (this, "KeyUp", Events.key_up);
			}
		}

		public event MouseEventHandler MouseEnter {
			add {
				if (events[MouseEnterEvent] == null)
					Events.AddHandler (this, "MouseEnter", Events.mouse_enter);
				events.AddHandler (MouseEnterEvent, value);
			}
			remove {
				events.RemoveHandler (MouseEnterEvent, value);
				if (events[MouseEnterEvent] == null)
					Events.RemoveHandler (this, "MouseEnter", Events.mouse_enter);
			}
		}

		public event EventHandler MouseLeave {
			add {
				if (events[MouseLeaveEvent] == null)
					Events.AddHandler (this, "MouseLeave", Events.mouse_leave);
				events.AddHandler (MouseLeaveEvent, value);
			}
			remove {
				events.RemoveHandler (MouseLeaveEvent, value);
				if (events[MouseLeaveEvent] == null)
					Events.RemoveHandler (this, "MouseLeave", Events.mouse_leave);
			}
		}

		public event MouseEventHandler MouseLeftButtonDown {
			add {
				if (events[MouseLeftButtonDownEvent] == null)
					Events.AddHandler (this, "MouseLeftButtonDown", Events.mouse_button_down);
				events.AddHandler (MouseLeftButtonDownEvent, value);
			}
			remove {
				events.RemoveHandler (MouseLeftButtonDownEvent, value);
				if (events[MouseLeftButtonDownEvent] == null)
					Events.RemoveHandler (this, "MouseLeftButtonDown", Events.mouse_button_down);
			}
		}

		public event MouseEventHandler MouseLeftButtonUp {
			add {
				if (events[MouseLeftButtonUpEvent] == null)
					Events.AddHandler (this, "MouseLeftButtonUp", Events.mouse_button_up);
				events.AddHandler (MouseLeftButtonUpEvent, value);
			}
			remove {
				events.RemoveHandler (MouseLeftButtonUpEvent, value);
				if (events[MouseLeftButtonUpEvent] == null)
					Events.RemoveHandler (this, "MouseLeftButtonUp", Events.mouse_button_up);
			}
		}

		public event MouseEventHandler MouseMove {
			add {
				if (events[MouseMoveEvent] == null)
					Events.AddHandler (this, "MouseMove", Events.mouse_motion);
				events.AddHandler (MouseMoveEvent, value);
			}
			remove {
				events.RemoveHandler (MouseMoveEvent, value);
				if (events[MouseMoveEvent] == null)
					Events.RemoveHandler (this, "MouseMove", Events.mouse_motion);
			}
		}


		internal void InvokeGotFocus ()
		{
			EventHandler h = (EventHandler)events[GotFocusEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		internal void InvokeLostFocus ()
		{
			EventHandler h = (EventHandler)events[LostFocusEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		internal void InvokeMouseMove (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler)events[MouseMoveEvent];
			if (h != null)
				h (this, m);
		}

		internal void InvokeMouseButtonDown (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler)events[MouseLeftButtonDownEvent];
			if (h != null)
				h (this, m);
		}

		internal void InvokeMouseButtonUp (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler)events[MouseLeftButtonUpEvent];
			if (h != null)
				h (this, m);
		}

		internal void InvokeKeyDown (KeyboardEventArgs k)
		{
			KeyboardEventHandler h = (KeyboardEventHandler)events[KeyDownEvent];
			if (h != null)
				h (this, k);
		}

		internal void InvokeKeyUp (KeyboardEventArgs k)
		{
			KeyboardEventHandler h = (KeyboardEventHandler)events[KeyUpEvent];
			if (h != null)
				h (this, k);
		}

		internal void InvokeMouseLeave ()
		{
			EventHandler h = (EventHandler)events[MouseLeaveEvent];
			if (h != null)
				h (this, null);
		}

		internal void InvokeMouseEnter (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler)events[MouseEnterEvent];
			if (h != null)
				h (this, m);
		}

		internal void InvokeLoaded ()
		{
			EventHandler h = (EventHandler)events[LoadedEvent];
			if (h != null)
				h (this, null);
		}

		internal override Kind GetKind ()
		{
			return Kind.UIELEMENT;
		}
	}
}
