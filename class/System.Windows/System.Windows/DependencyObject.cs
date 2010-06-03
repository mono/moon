//
// DependencyObject.cs
//
// Author:
//   Iain McCoy (iain@mccoy.id.au)
//   Moonlight Team (moonlight-list@lists.ximian.com)
//
// Copyright 2005 Iain McCoy
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
	
#pragma warning disable 3003 // "Type of 'X' is not CLS-compliant" shown for the Dispatcher property

using Mono;
using System.Collections.Generic;
using System.Windows;
using System.Runtime.InteropServices;
using System.Windows.Threading;
using System.Threading;
using System.Windows.Data;
using System.Windows.Media;

namespace System.Windows {
	public abstract partial class DependencyObject : INativeDependencyObjectWrapper {
		internal static Thread moonlight_thread;
		internal IntPtr _native;
		EventHandlerList event_list;
		bool free_mapping;

		internal event EventHandler MentorChanged {
			add {
				var val = value;
				UnmanagedEventHandler h = delegate { val (this, EventArgs.Empty); };
				RegisterEvent (EventIds.EventObject_MentorChangedEvent, value, h);
			}
			remove {
				UnregisterEvent (EventIds.EventObject_MentorChangedEvent, value);
			}
		}

		internal EventHandlerList EventList {
			get {
				if (event_list == null)
					event_list = new EventHandlerList ();
				return event_list;
			}
		}

		IntPtr INativeEventObjectWrapper.NativeHandle {
			get { return native; }
			set { native = value; }
		}

		bool invalidatingLocalBindings;
		internal Dictionary<DependencyProperty, Expression> expressions;

		internal IntPtr native {
			get {
				return _native;
			}

			set {
				if (_native != IntPtr.Zero) {
					throw new InvalidOperationException ("DependencyObject.native is already set");
				}

				_native = value;
				free_mapping = NativeDependencyObjectHelper.AddNativeMapping (value, this);
			}
		}

		internal FrameworkElement Mentor {
			get { return (FrameworkElement) NativeDependencyObjectHelper.Lookup (NativeMethods.dependency_object_get_mentor (native)); }
		}

		internal DependencyObject TemplateOwner {
			get { return (DependencyObject) NativeDependencyObjectHelper.Lookup (Mono.NativeMethods.dependency_object_get_template_owner (native)); }
			set {
				IntPtr owner = value == null ? IntPtr.Zero : value.native;
				Mono.NativeMethods.dependency_object_set_template_owner (native, owner);
			}
		}

		static DependencyObject ()
		{
			moonlight_thread = Thread.CurrentThread;
		}

		// This created objects with a managed lifetime, so the native ref will be dropped
		// as soon as the object is created and handed to ToggleRef
		protected DependencyObject () : this (SafeNativeMethods.dependency_object_new (), true)
		{
		}

		internal DependencyObject (IntPtr raw, bool dropref)
		{
			native = raw;
			expressions = new Dictionary<DependencyProperty, Expression> ();
			NativeMethods.event_object_set_object_type (native, GetKind ());
			// Objects created on the managed side have a normal managed lifetime,
			// so drop the native ref hold
			if (dropref)
				NativeMethods.event_object_unref (native);
		}

		internal void Free ()
		{
			UnregisterAllEvents ();

			if (free_mapping)
				NativeDependencyObjectHelper.FreeNativeMapping (this);
		}

		~DependencyObject ()
		{
			Free ();
		}

		internal void AddPropertyChangedHandler (DependencyProperty property, UnmanagedPropertyChangeHandler handler)
		{
			Mono.NativeMethods.dependency_object_add_property_change_handler (native, property.Native, handler, IntPtr.Zero);
		}

		internal void RemovePropertyChangedHandler (DependencyProperty property, UnmanagedPropertyChangeHandler handler)
		{
			Mono.NativeMethods.dependency_object_remove_property_change_handler (native, property.Native, handler);
		}

		// This method is emitted as virtual due to: https://bugzilla.novell.com/show_bug.cgi?id=446507
		public object GetValue (DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.GetValue (this, dp);
		}
		
		// This method is emitted as virtual due to: https://bugzilla.novell.com/show_bug.cgi?id=446507
		public object GetAnimationBaseValue (DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.GetAnimationBaseValue (this, dp);
		}
		
		// This method is emitted as virtual due to: https://bugzilla.novell.com/show_bug.cgi?id=446507
		public object ReadLocalValue (DependencyProperty dp)
		{
			return ReadLocalValueImpl (dp);
		}

		internal void RegisterEvent (int eventId, Delegate managedHandler, UnmanagedEventHandler nativeHandler)
		{
			if (managedHandler == null)
				return;

			int token = Events.AddHandler (this, eventId, nativeHandler);

			EventList.AddHandler (eventId, token, managedHandler, nativeHandler);
		}

		internal void UnregisterEvent (int eventId, Delegate managedHandler)
		{
			UnmanagedEventHandler nativeHandler = EventList.RemoveHandler (eventId, managedHandler);

			if (nativeHandler == null)
				return;

			Events.RemoveHandler (this, eventId, nativeHandler);
		}

		void UnregisterAllEvents ()
		{
			foreach (int eventId in EventList.Keys) {
				foreach (EventHandlerData d in EventList[eventId].Values) {
					Events.RemoveHandler (this, eventId, d.NativeHandler);
				}
			}
		}

		internal virtual object ReadLocalValueImpl (DependencyProperty dp)
		{
			Expression expression;
			if (expressions.TryGetValue (dp, out expression))
				return expression;
			return NativeDependencyObjectHelper.ReadLocalValue (this, dp);
		}
		
		// This method is emitted as virtual due to: https://bugzilla.novell.com/show_bug.cgi?id=446507
		public void ClearValue (DependencyProperty dp)
		{
			if (dp == null)
				throw new ArgumentNullException ("dp");
			if (dp.IsReadOnly && !(dp is CustomDependencyProperty))
				throw new ArgumentException ("This property is readonly");

			ClearValueImpl (dp);
		}

		internal virtual void ClearValueImpl (DependencyProperty dp)
		{
			RemoveExpression (dp);
			NativeDependencyObjectHelper.ClearValue (this, dp);
		}
		
		[System.ComponentModel.EditorBrowsable (System.ComponentModel.EditorBrowsableState.Advanced)]
		public Dispatcher Dispatcher {
			get {
				return Dispatcher.Main;
			}
		}
		
		
		// This method is emitted as virtual due to: https://bugzilla.novell.com/show_bug.cgi?id=446507
		public void SetValue (DependencyProperty dp, object value)
		{
			if (dp == null)
				throw new ArgumentNullException ("property");
			if (dp.IsReadOnly) {
				if (dp is CustomDependencyProperty)
					throw new InvalidOperationException ();
				else
					throw new ArgumentException ();
			}
			SetValueImpl (dp, value);
		}

		internal virtual void SetValueImpl (DependencyProperty dp, object value)
		{
			bool updateTwoWay = false;
			bool addingExpression = false;
			Expression existing;
			Expression expression = value as Expression;
			BindingExpressionBase bindingExpression = expression as BindingExpressionBase;
			
			if (bindingExpression != null) {
				string path = bindingExpression.Binding.Path.Path;
				if ((string.IsNullOrEmpty (path) || path == ".") &&
				    bindingExpression.Binding.Mode == BindingMode.TwoWay)
					throw new ArgumentException ("TwoWay bindings require a non-empty Path");
				bindingExpression.Binding.Seal ();
			}

			expressions.TryGetValue (dp, out existing);
			
			if (expression != null) {
				if (existing != expression) {
					if (expression.Attached)
						throw new ArgumentException ("Cannot attach the same Expression to multiple FrameworkElements");

					if (existing != null)
						RemoveExpression (dp);
					expressions.Add (dp, expression);
					expression.OnAttached (this);
				}
				addingExpression = true;
				value = expression.GetValue (dp);
			} else if (existing != null) {
				if (existing is BindingExpressionBase) {
					BindingExpressionBase beb = (BindingExpressionBase)existing;

					if (beb.Binding.Mode == BindingMode.TwoWay) {
						updateTwoWay = !beb.Updating && !(dp is CustomDependencyProperty);
					} else if (!beb.Updating || beb.Binding.Mode == BindingMode.OneTime) {
						RemoveExpression (dp);
					}
				}
				else if (!existing.Updating) {
					RemoveExpression (dp);
				}
			}

			try {
				NativeDependencyObjectHelper.SetValue (this, dp, value);
				if (updateTwoWay)
					((BindingExpressionBase)existing).TryUpdateSourceObject (value);
			} catch {
				if (!addingExpression)
					throw;
				else {
					NativeDependencyObjectHelper.SetValue (this, dp, dp.GetDefaultValue (this));
					if (updateTwoWay)
						((BindingExpressionBase)existing).TryUpdateSourceObject (value);
				}
			}
		}

		internal DependencyObject FindName (string name)
		{
			if (name == null)
				throw new ArgumentNullException ("name");

			Kind k;
			IntPtr o = NativeMethods.dependency_object_find_name (native, name, out k);
			if (o == IntPtr.Zero)
				return null;

			return NativeDependencyObjectHelper.Lookup (k, o) as DependencyObject;
		}

		Kind INativeEventObjectWrapper.GetKind ()
		{
			return GetKind ();
		}

		internal Kind GetKind ()
		{
			return Deployment.Current.Types.Find (GetType()).native_handle;
		}
		
		// This method is emitted as virtual due to: https://bugzilla.novell.com/show_bug.cgi?id=446507
		[System.ComponentModel.EditorBrowsable (System.ComponentModel.EditorBrowsableState.Never)]
		public bool CheckAccess ()
		{
			return Thread.CurrentThread == moonlight_thread;
		}
		
		internal static void Initialize ()
		{
			// Here just to ensure that the static ctor is executed and
			// runtime init is initialized from some entry points
		}

		internal void InvalidateSubtreeBindings ()
		{
			for (int c = 0; c < VisualTreeHelper.GetChildrenCount (this); c++) {
				FrameworkElement obj = VisualTreeHelper.GetChild (this, c) as FrameworkElement;
				if (obj == null)
					continue;
				obj.InvalidateLocalBindings ();
				obj.InvalidateSubtreeBindings ();
			}
		}

		internal void InvalidateLocalBindings ()
		{
			if (expressions.Count == 0)
				return;

			if (invalidatingLocalBindings)
				return;

			invalidatingLocalBindings = true;

			foreach (var keypair in expressions) {
				if (keypair.Value is BindingExpressionBase) {
					BindingExpressionBase beb = (BindingExpressionBase) keypair.Value;
					beb.Invalidate ();
					SetValue (keypair.Key, beb);
				}
			}

			invalidatingLocalBindings = false;
		}

		void RemoveExpression (DependencyProperty dp)
		{
			Expression e;
			if (expressions.TryGetValue (dp, out e)) {
				expressions.Remove (dp);
				e.OnDetached (this);
			}
		}
	}
}
