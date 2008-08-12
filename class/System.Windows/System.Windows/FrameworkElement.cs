//
// FrameworkElement.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Markup;
using System.Security;

namespace System.Windows {
	public abstract partial class FrameworkElement : UIElement {
		
		internal FrameworkElement () : base (NativeMethods.framework_element_new ())
		{
			Console.WriteLine ("*** Created a {0} (frameworkelement) with {1}", this.GetType (), native);
		}
		
		internal FrameworkElement (IntPtr raw) : base (raw)
		{
		}
		
		internal override Kind GetKind ()
		{
			return Kind.FRAMEWORKELEMENT;
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public object FindName (string name)
		{
			return DepObjectFindName (name);
		}

		public BindingExpressionBase SetBinding (DependencyProperty property, Binding binding)
		{
			throw new NotImplementedException ();
		}

		public override object GetValue (DependencyProperty property)
		{
			// XXX reason for the override?  maybe this is
			// where some portion of databinding is done?

			return base.GetValue (property);
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		protected virtual Size MeasureOverride (Size size)
		{
			throw new NotImplementedException ();
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		protected virtual Size ArrangeOverride (Size size)
		{
			throw new NotImplementedException ();
		}

		public DependencyObject Parent {
#if NET_2_1
			[SecuritySafeCritical]
#endif
			get {
				IntPtr parent_handle = NativeMethods.uielement_get_parent (native);
				if (parent_handle == IntPtr.Zero)
					return null;

				Kind k = NativeMethods.dependency_object_get_object_type (parent_handle);
				return DependencyObject.Lookup (k, parent_handle);
			}
		}

		[MonoTODO ("figure out how to construct routed events")]
		public static readonly RoutedEvent LoadedEvent = new RoutedEvent();

		static object BindingValidationErrorEvent = new object ();
		static object LoadedEvent_ = new object ();
		static object LayoutUpdatedEvent = new object ();
		static object SizeChangedEvent = new object ();
		
#if notyet
		public event EventHandler<ValidationErrorEventArgs> BindingValidationError {
			add {
				if (events[BindingValidationErrorEvent] == null)
					Events.AddHandler (this, "BindingValidationError", Events.binding_validation_error);
				events.AddHandler (BindingValidationErrorEvent, value);
			}
			remove {
				events.RemoveHandler (BindingValidationErrorEvent, value);
				if (events[BindingValidationErrorEvent] == null)
					Events.RemoveHandler (this, "BindingValidationError", Events.binding_validation_error);
			}
		}
#endif
		public event EventHandler LayoutUpdated {
			add {
				if (events[LayoutUpdatedEvent] == null)
					Events.AddHandler (this, "LayoutUpdated", Events.layout_updated);
				events.AddHandler (LayoutUpdatedEvent, value);
			}
			remove {
				events.RemoveHandler (LayoutUpdatedEvent, value);
				if (events[LayoutUpdatedEvent] == null)
					Events.RemoveHandler (this, "LayoutUpdated", Events.layout_updated);
			}
		}

		public event RoutedEventHandler Loaded {
			add {
				if (events[LoadedEvent_] == null)
					Events.AddHandler (this, "Loaded", Events.loaded);
				events.AddHandler (LoadedEvent_, value);
			}
			remove {
				events.RemoveHandler (LoadedEvent_, value);
				if (events[LoadedEvent_] == null)
					Events.RemoveHandler (this, "Loaded", Events.loaded);
			}
		}

		public event SizeChangedEventHandler SizeChanged {
			add {
				if (events[SizeChangedEvent] == null)
					Events.AddHandler (this, "SizeChanged", Events.size_changed);
				events.AddHandler (SizeChangedEvent, value);
			}
			remove {
				events.RemoveHandler (SizeChangedEvent, value);
				if (events[SizeChangedEvent] == null)
					Events.RemoveHandler (this, "SizeChanged", Events.size_changed);
			}
		}

		internal void InvokeLoaded ()
		{
			// this event is special, in that it is a
			// RoutedEvent that doesn't bubble, so we
			// don't need to worry about doing anything
			// special here.  Create a new RoutedEventArgs
			// here and invoke it as normal.
			RoutedEventHandler reh = (RoutedEventHandler)events[LoadedEvent_];
			if (reh != null) {
				RoutedEventArgs args = new RoutedEventArgs();
				args.Source = this;
				reh (this, args);
			}
		}

		internal void InvokeLayoutUpdated ()
		{
			EventHandler h = (EventHandler)events[LayoutUpdatedEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		internal void InvokeSizeChanged (SizeChangedEventArgs args)
		{
			// RoutedEvent subclass, but doesn't bubble.
			SizeChangedEventHandler h = (SizeChangedEventHandler)events[SizeChangedEvent];
			if (h != null)
				h (this, args);
		}
	}
}
