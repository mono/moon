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
using System.Reflection;
using System.Collections.Generic;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Markup;
using System.Security;

namespace System.Windows {
	public abstract partial class FrameworkElement : UIElement {

		static FrameworkElement ()
		{
			StyleProperty.Validate = delegate (DependencyObject target, DependencyProperty propety, object value) {
				Type styleType = ((Style)value).TargetType;
				if (!styleType.IsAssignableFrom (target.GetType ()))
					throw new System.Windows.Markup.XamlParseException (string.Format ("Target is of type {0} but the Style requires {1}", target.GetType ().Name, styleType.Name));
			};
		}
		/* 
		 * XXX these are marked internal because making them private seems
		 * to cause the GC to collect them
		 */
		internal MeasureOverrideCallback measure_cb;
		internal ArrangeOverrideCallback arrange_cb;
		Dictionary<DependencyProperty, BindingExpressionBase> bindings = new Dictionary<DependencyProperty, BindingExpressionBase> ();

		private bool OverridesLayoutMethod (string name)
		{
			MethodInfo method = GetType().GetMethod ("MeasureOverride", BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.FlattenHierarchy);
			if (method != null) {
				if (method.DeclaringType != typeof (FrameworkElement)
				    && method.IsFamily && method.IsVirtual
				    && method.ReturnType == typeof (Size)) {

					ParameterInfo[] ps = method.GetParameters();
					if (ps.Length == 1
					    && ps[0].ParameterType == typeof (Size)) {

						return true;
					}
				}
			}
			return false;
		}

		private void Initialize ()
		{
			if (OverridesLayoutMethod ("MeasureOverride"))
				measure_cb = new MeasureOverrideCallback (InvokeMeasureOverride);
			if (true && OverridesLayoutMethod ("ArrangeOverride"))
				arrange_cb = new ArrangeOverrideCallback (InvokeArrangeOverride);
			NativeMethods.framework_element_register_managed_overrides (native, measure_cb, arrange_cb);

			// we always need to attach this event to allow for Controls to load their default style
			Events.AddHandler (this, "Loaded", Events.loaded);
		}

		public object FindName (string name)
		{
			if (name == null)
				throw new ArgumentNullException ("name");
			return DepObjectFindName (name);
		}

		public BindingExpressionBase SetBinding (DependencyProperty dp, Binding binding)
		{
			if (dp == null)
				throw new ArgumentNullException ("dp");
			if (binding == null)
				throw new ArgumentNullException ("binding");

			BindingExpression e = new BindingExpression {
				Binding = binding,
				Target = this,
				Property = dp
			};
			binding.Seal ();
			SetValue (dp, e);
			return e;
		}

		protected virtual Size MeasureOverride (Size availableSize)
		{
			return NativeMethods.framework_element_measure_override (native, availableSize);
		}

		protected virtual Size ArrangeOverride (Size finalSize)
		{
			return NativeMethods.framework_element_arrange_override (native, finalSize);
		}

		public DependencyObject Parent {
			get {
				return NativeDependencyObjectHelper.FromIntPtr (NativeMethods.uielement_get_visual_parent (native)) as DependencyObject;
			}
		}

		internal DependencyObject SubtreeObject {
			[SecuritySafeCritical]
			get {
				return NativeDependencyObjectHelper.FromIntPtr (NativeMethods.uielement_get_subtree_object (native)) as DependencyObject;
			}
		}		

		[MonoTODO ("figure out how to construct routed events")]
		public static readonly RoutedEvent LoadedEvent = new RoutedEvent();

		static object BindingValidationErrorEvent = new object ();
		static object LoadedEvent_ = new object ();
		static object LayoutUpdatedEvent = new object ();
		static object SizeChangedEvent = new object ();
		
		public event EventHandler<ValidationErrorEventArgs> BindingValidationError {
			add {
				RegisterEvent (BindingValidationErrorEvent, "BindingValidationError", Events.binding_validation_error, value);
			}
			remove {
				UnregisterEvent (BindingValidationErrorEvent, "BindingValidationError", Events.binding_validation_error, value);
			}
		}

		public event EventHandler LayoutUpdated {
			add {
				RegisterEvent (LayoutUpdatedEvent, "LayoutUpdated", Events.layout_updated, value);
			}
			remove {
				UnregisterEvent (LayoutUpdatedEvent, "LayoutUpdated", Events.layout_updated, value);
			}
		}

		public event RoutedEventHandler Loaded {
			add { EventList.AddHandler (LoadedEvent_, value); }
			remove { EventList.RemoveHandler (LoadedEvent_, value); }
		}

		public event SizeChangedEventHandler SizeChanged {
			add {
				RegisterEvent (SizeChangedEvent, "SizeChanged", Events.size_changed, value);
			}
			remove {
				UnregisterEvent (SizeChangedEvent, "SizeChanged", Events.size_changed, value);
			}
		}

		internal virtual void InvokeLoaded ()
		{
			// this event is special, in that it is a
			// RoutedEvent that doesn't bubble, so we
			// don't need to worry about doing anything
			// special here.  Create a new RoutedEventArgs
			// here and invoke it as normal.
			RoutedEventHandler reh = (RoutedEventHandler) EventList [LoadedEvent_];
			if (reh != null) {
				RoutedEventArgs args = new RoutedEventArgs();
				args.OriginalSource = this;
				reh (this, args);
			}
		}

		internal void InvokeLayoutUpdated ()
		{
			EventHandler h = (EventHandler) EventList [LayoutUpdatedEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		internal void InvokeSizeChanged (SizeChangedEventArgs args)
		{
			// RoutedEvent subclass, but doesn't bubble.
			SizeChangedEventHandler h = (SizeChangedEventHandler) EventList [SizeChangedEvent];
			if (h != null)
				h (this, args);
		}

		private Size InvokeMeasureOverride (Size availableSize)
		{
			try {
				return MeasureOverride (availableSize);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in FrameworkElement.InvokeMeasureOverride: {0}", ex.Message);
				} catch {
					// Ignore
				}
			}
			return new Size (); 
		}

		private Size InvokeArrangeOverride (Size finalSize)
		{
			try {
				return ArrangeOverride (finalSize);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in FrameworkElement.InvokeArrangeOverride: {0}", ex.Message);
				} catch {
					// Ignore
				}
			}
			return new Size ();
		}
		
		[SecuritySafeCritical]
		public virtual void OnApplyTemplate ()
		{
			// according to doc this is not fully implemented since SL templates applies
			// to Control/ContentPresenter and is defined here for WPF compatibility
		}

		internal override void ClearValueImpl (DependencyProperty dp)
		{
			if (bindings.ContainsKey (dp))
				bindings.Remove (dp);
			base.ClearValueImpl (dp);
		}

		internal override void SetValueImpl (DependencyProperty dp, object value)
		{
			BindingExpressionBase existing;
			BindingExpressionBase expression = value as BindingExpressionBase;
			bindings.TryGetValue (dp, out existing);
			
			if (expression != null) {
				if (existing != null)
					bindings.Remove (dp);
				bindings.Add (dp, expression);

				value = expression.GetValue (dp);
			} else if (existing != null) {
				if (existing.Binding.Mode == BindingMode.TwoWay)
					existing.SetValue (value);
				else
					bindings.Remove (dp);
			}

			base.SetValueImpl (dp, value);
			
			if (dp == FrameworkElement.DataContextProperty && bindings.Count > 0) {
				Dictionary<DependencyProperty, BindingExpressionBase> old = bindings;
				bindings = new Dictionary<DependencyProperty, BindingExpressionBase> ();
				foreach (var keypair in old) {
					keypair.Value.Invalidate ();
					SetValue (keypair.Key, keypair.Value);
				}
			}
		}

		internal void UpdateFromBinding (DependencyProperty dp, object value)
		{
			base.SetValueImpl (dp, value);
		}

		internal override object ReadLocalValueImpl (DependencyProperty dp)
		{
			BindingExpressionBase expression;
			if (bindings.TryGetValue (dp, out expression))
				return expression;
			return base.ReadLocalValueImpl (dp);
		}
	}
}
