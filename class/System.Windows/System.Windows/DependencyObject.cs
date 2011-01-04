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
	public abstract partial class DependencyObject : INativeDependencyObjectWrapper, IRefContainer {
		internal static Thread moonlight_thread;
		DependencyObjectHandle _native;
		EventHandlerList event_list;
		List<UnmanagedPropertyChangeHandler> propertyChangedHandlers;

		internal EventHandlerList EventList {
			get {
				if (event_list == null)
					event_list = new EventHandlerList (this);
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
				return _native.Handle;
			}

			set {
				if (_native != null) {
					throw new InvalidOperationException ("DependencyObject.native is already set");
				}

				NativeDependencyObjectHelper.AddNativeMapping (value, this);
				_native = new DependencyObjectHandle (value, this);
			}
		}

		internal DependencyObject TemplateOwner {
			get {
				var owner = Mono.NativeMethods.dependency_object_get_template_owner (native);
				return (DependencyObject) NativeDependencyObjectHelper.Lookup (owner);
			}
			set {
				var owner = value == null ? IntPtr.Zero : value.native;
				Mono.NativeMethods.dependency_object_set_template_owner (native, owner);
			}
		}

		internal Uri ResourceBase {
			get {
				IntPtr native_uri = NativeMethods.dependency_object_get_resource_base (native);
				return UriHelper.FromNativeUri (native_uri);
			}
			set {
				IntPtr native_uri = UriHelper.ToNativeUri (value);
				NativeMethods.dependency_object_set_resource_base (native, native_uri);
				NativeMethods.uri_free (native_uri);
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
			NativeDependencyObjectHelper.SetManagedPeerCallbacks (this);

			NativeMethods.event_object_set_object_type (native, GetKind ());
			// Objects created on the managed side have a normal managed lifetime,
			// so drop the native ref hold
			if (dropref)
				NativeMethods.event_object_unref (native);
		}

		Dictionary<IntPtr,object> strongRefs;
		Dictionary<string,object> namedRefs;

		void IRefContainer.AddStrongRef (IntPtr referent, string name)
		{
			AddStrongRef (referent, name);
		}

		internal virtual void AddStrongRef (IntPtr referent, string name)
		{
			var o = Value.ToObject (null, referent);

			if (name == "" && strongRefs != null && strongRefs.ContainsKey (referent))
				return;
			if (namedRefs != null && namedRefs.ContainsKey (name))
				return;

			if (o != null) {

				if (name == "") {
#if DEBUG_REF
					Console.WriteLine ("Adding ref from {0}/{1} to {2}/{3} (referent = {4:x})", GetHashCode(), this, o.GetHashCode(), o, (int) referent);
#endif
					if (strongRefs == null) {
						strongRefs = new Dictionary<IntPtr, object> ();
					}
					strongRefs.Add (referent, o);
				}
				else {
#if DEBUG_REF
					Console.WriteLine ("Adding ref named `{4}' from {0}/{1} to {2}/{3} (refrent = {5:x})", GetHashCode(), this, o.GetHashCode(), o, name, (int)referent);
#endif
					if (namedRefs == null)
						namedRefs = new Dictionary<string,object> (StringComparer.Ordinal);
					namedRefs.Add (name, o);
				}
			}
		}

		void IRefContainer.ClearStrongRef (IntPtr referent, string name)
		{
			ClearStrongRef (referent, name);
			
		}

		internal virtual void ClearStrongRef (IntPtr referent, string name)
		{
			if (name == "") {

#if DEBUG_REF
				Console.WriteLine ("Clearing ref from {0}/{1} to referent = {2:x}", GetHashCode(), this, (int) referent);
#endif
				if (strongRefs != null)
					strongRefs.Remove (referent);
			}
			else {
#if DEBUG_REF
				Console.WriteLine ("Clearing ref named `{4}' from {0}/{1} to referent = {2:x}", GetHashCode(), this, name, (int)referent);
#endif
				if (namedRefs != null)
					namedRefs.Remove (name);
			}
		}

#if HEAPVIZ
		System.Collections.ICollection IRefContainer.GetManagedRefs ()
		{
			List<HeapRef> refs = new List<HeapRef> ();

			AccumulateManagedRefs (refs);

			return refs;
		}

		internal virtual void AccumulateManagedRefs (List<HeapRef> refs)
		{
			if (strongRefs != null)
				foreach (IntPtr nativeref in strongRefs.Keys)
					if (strongRefs[nativeref] is INativeEventObjectWrapper)
						refs.Add (new HeapRef ((INativeEventObjectWrapper)strongRefs[nativeref]));

			if (namedRefs != null)
			foreach (string name in namedRefs.Keys)
				if (namedRefs[name] is INativeEventObjectWrapper)
					refs.Add (new HeapRef (true, (INativeEventObjectWrapper)namedRefs[name], name));

			if (TemplateOwner != null)
				refs.Add (new HeapRef (true,
						       TemplateOwner,
						       "TemplateOwner"));
		}
#endif
	
		void INativeEventObjectWrapper.MentorChanged (IntPtr mentor_ptr)
		{
			var h = MentorChanged;
			if (h != null)
				MentorChanged (this, EventArgs.Empty);
		}

		internal FrameworkElement Mentor {
			get {
				var mentor = NativeMethods.dependency_object_get_mentor (native);
				return (FrameworkElement) NativeDependencyObjectHelper.Lookup (mentor);
			}
		}

		internal event EventHandler MentorChanged;

		void INativeEventObjectWrapper.OnDetached ()
		{

		}

		void INativeEventObjectWrapper.OnAttached ()
		{

		}

		public object GetValue (DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.GetValue (this, dp);
		}
		
		public object GetAnimationBaseValue (DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.GetAnimationBaseValue (this, dp);
		}
		
		public object ReadLocalValue (DependencyProperty dp)
		{
			return ReadLocalValueImpl (dp);
		}

		internal void AddPropertyChangedHandler (DependencyProperty property, UnmanagedPropertyChangeHandler handler)
		{
			// Store the delegate in managed land to prevent it being GC'ed early
			if (propertyChangedHandlers == null)
				propertyChangedHandlers = new List<UnmanagedPropertyChangeHandler> ();

			propertyChangedHandlers.Add (handler);
			NativeMethods.dependency_object_add_property_change_handler (native, property.Native, handler, IntPtr.Zero);
		}

		internal void RemovePropertyChangedHandler (DependencyProperty property, UnmanagedPropertyChangeHandler handler)
		{
			if (propertyChangedHandlers == null)
				return;

			// When removing a delegate from native, we have to ensure we pass the exact same object reference
			// to native code, otherwise the native pointer will be different and we'll fail to remove it from
			// the native list. To enforce this, use the delegate in the List when invoking the native code.
			int index = propertyChangedHandlers.IndexOf (handler);
			if (index != -1) {
				NativeMethods.dependency_object_remove_property_change_handler (native, property.Native, propertyChangedHandlers [index]);
				propertyChangedHandlers.RemoveAt (index);
			}
		}

		internal void RegisterEvent (int eventId, Delegate managedHandler, UnmanagedEventHandler nativeHandler)
		{
			RegisterEvent (eventId, managedHandler, nativeHandler, false);
		}

		internal void RegisterEvent (int eventId, Delegate managedHandler, UnmanagedEventHandler nativeHandler, bool handledEventsToo)
		{
			EventList.RegisterEvent (this, eventId, managedHandler, nativeHandler, handledEventsToo);
		}

		internal void UnregisterEvent (int eventId, Delegate managedHandler)
		{
			EventList.UnregisterEvent (this, eventId, managedHandler);
		}

		internal object ReadLocalValueImpl (DependencyProperty dp)
		{
			Expression expression;
			if (expressions != null && expressions.TryGetValue (dp, out expression))
				return expression;
			return NativeDependencyObjectHelper.ReadLocalValue (this, dp);
		}
		
		public void ClearValue (DependencyProperty dp)
		{
			if (dp == null)
				throw new ArgumentNullException ("dp");
			if (dp.IsReadOnly && !(dp is CustomDependencyProperty))
				throw new ArgumentException ("This property is readonly");

			ClearValueImpl (dp);
		}

		internal void ClearValueImpl (DependencyProperty dp)
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

		internal void SetValueImpl (DependencyProperty dp, object value)
		{
			bool updateTwoWay = false;
			bool addingExpression = false;
			Expression existing = null;
			Expression expression = value as Expression;
			BindingExpressionBase bindingExpression = expression as BindingExpressionBase;
			
			if (bindingExpression != null) {
				string path = bindingExpression.Binding.Path.Path;
				if ((string.IsNullOrEmpty (path) || path == ".") &&
				    bindingExpression.Binding.Mode == BindingMode.TwoWay)
					throw new ArgumentException ("TwoWay bindings require a non-empty Path");
				bindingExpression.Binding.Seal ();
			}

			if (expressions != null)
				if (!expressions.TryGetValue (dp, out existing))
					existing = null;
			
			if (expression != null) {
				if (existing != expression) {
					if (expression.Attached)
						throw new ArgumentException ("Cannot attach the same Expression to multiple FrameworkElements");

					if (existing != null)
						RemoveExpression (dp);
					if (expressions == null)
						expressions = new Dictionary<DependencyProperty, Expression> ();

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
			return (DependencyObject) NativeDependencyObjectHelper.Lookup (o);
		}

		internal bool SetNameOnScope (string name, NameScope scope)
		{
			if (name == null)
				throw new ArgumentNullException ("name");
			if (scope == null)
				throw new ArgumentNullException ("scope");
			return NativeMethods.dependency_object_set_name_on_scope (native, name, scope.NativeHandle);
		}

		Kind INativeEventObjectWrapper.GetKind ()
		{
			return GetKind ();
		}

		internal Kind GetKind ()
		{
			return Deployment.Current.Types.Find (GetType()).native_handle;
		}
		
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
			if (expressions == null || expressions.Count == 0)
				return;

			if (invalidatingLocalBindings)
				return;

			invalidatingLocalBindings = true;

			DependencyProperty[] keys = new DependencyProperty [expressions.Keys.Count];
			Expression[] values = new Expression [expressions.Values.Count];

			expressions.Keys.CopyTo (keys, 0);
			expressions.Values.CopyTo (values, 0);

			for (int i = 0; i < keys.Length; i ++) {
				if (values[i] is BindingExpressionBase) {
					BindingExpressionBase beb = (BindingExpressionBase) values[i];
					beb.Invalidate ();
					SetValue (keys[i], beb);
				}
			}

			invalidatingLocalBindings = false;
		}

		void RemoveExpression (DependencyProperty dp)
		{
			Expression e;
			if (expressions != null && expressions.TryGetValue (dp, out e)) {
				expressions.Remove (dp);
				e.OnDetached (this);
			}
		}
	}
}
