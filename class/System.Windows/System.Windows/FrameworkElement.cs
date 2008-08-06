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
	public abstract class FrameworkElement : UIElement {
		public static readonly DependencyProperty ActualHeightProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "ActualHeight", typeof (double));

		public static readonly DependencyProperty ActualWidthProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "ActualWidth", typeof (double));

		public static readonly DependencyProperty DataContextProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "DataContext", typeof (object));

		public static readonly DependencyProperty HeightProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "Height", typeof (double));

		public static readonly DependencyProperty HorizontalAlignmentProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "HorizontalAlignment", typeof (HorizontalAlignment));

		public static readonly DependencyProperty LanguageProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "Language", typeof (XmlLanguage));

		public static readonly DependencyProperty MarginProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "Margin", typeof (Thickness));
		
		public static readonly DependencyProperty MaxHeightProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "MaxHeight", typeof (double));

		public static readonly DependencyProperty MaxWidthProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "MaxWidth", typeof (double));

		public static readonly DependencyProperty MinHeightProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "MinHeight", typeof (double));

		public static readonly DependencyProperty MinWidthProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "MinWidth", typeof (double));

		public static readonly DependencyProperty VerticalAlignmentProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "VerticalAlignment", typeof (VerticalAlignment));

		public static readonly DependencyProperty WidthProperty =
			DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "Width", typeof (double));

		// XXX we look these next four properties up based on
		// Kind.UIELEMENT due to the fact that in order to
		// maintain 1.0 and 2.0 working, we have to leave the
		// unmanaged properties in the UIElement class.
		private static readonly DependencyProperty CursorProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "Cursor", typeof (Cursor));

		private static readonly DependencyProperty ResourcesProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "Resources", typeof (ResourceDictionary));
		
		public static readonly DependencyProperty TagProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "Tag", typeof (object));

		private static readonly DependencyProperty TriggersProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "Triggers", typeof (TriggerCollection));

		
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

		public double ActualHeight {
			get { return (double) GetValue (ActualHeightProperty); }
		}

		public double ActualWidth {
			get { return (double) GetValue (ActualWidthProperty); }
		}

		public Cursor Cursor {
			get { return (Cursor) GetValue (CursorProperty); }
			set { SetValue (CursorProperty, value);	}
		}

		public object DataContext {
			get { return (Cursor) GetValue (DataContextProperty); }
			set { SetValue (DataContextProperty, value);	}
		}
		
		public double Height {
			get { return (double) GetValue (HeightProperty); }
			set { SetValue (HeightProperty, value); }
		}

		public HorizontalAlignment HorizontalAlignment {
			get { return (HorizontalAlignment) GetValue (HorizontalAlignmentProperty); }
			set { SetValue (HorizontalAlignmentProperty, value); }
		}

		public XmlLanguage Language {
			get { return (XmlLanguage) GetValue (LanguageProperty); }
			set { SetValue (LanguageProperty, value); }
		}

		public double MaxHeight {
			get { return (double) GetValue (MaxHeightProperty); }
			set { SetValue (MaxHeightProperty, value); }
		}

		public double MaxWidth {
			get { return (double) GetValue (MaxWidthProperty); }
			set { SetValue (MaxWidthProperty, value); }
		}

		public double MinHeight {
			get { return (double) GetValue (MinHeightProperty); }
			set { SetValue (MinHeightProperty, value); }
		}

		public double MinWidth {
			get { return (double) GetValue (MinWidthProperty); }
			set { SetValue (MinWidthProperty, value); }
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

		public ResourceDictionary Resources {
			get { return (ResourceDictionary)GetValue(ResourcesProperty); }
		}

		// XXX need to make sure we can set an arbitrary object in an unmanaged DP
		public object Tag {
			get { return (object) GetValue (TagProperty); }
			set { SetValue (TagProperty, value); }
		}

		public Thickness Margin {
			get { return (Thickness) GetValue (MarginProperty); }
			set { SetValue (MarginProperty, value); }
		}

		public TriggerCollection Triggers {
			get { return (TriggerCollection) GetValue (TriggersProperty); }
		}
		
		public VerticalAlignment VerticalAlignment {
			get { return (VerticalAlignment) GetValue (VerticalAlignmentProperty); }
			set { SetValue (VerticalAlignmentProperty, value); }
		}

		public double Width {
			get { return (double) GetValue (WidthProperty); }
			set { SetValue (WidthProperty, value); }
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
