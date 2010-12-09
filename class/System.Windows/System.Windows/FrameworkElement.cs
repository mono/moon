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
using System.Windows.Media;
using System.Windows.Automation;
using System.Windows.Controls.Primitives;

namespace System.Windows {
	public abstract partial class FrameworkElement : UIElement, IListenLayoutUpdated {
				static UnmanagedEventHandler template_applied = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((FrameworkElement) NativeDependencyObjectHelper.FromIntPtr (closure)).InvokeOnApplyTemplate ());

		static FrameworkElement ()
		{
			DataContextProperty.AddPropertyChangeCallback (DataContextChanged);
		}

		static void DataContextChanged (DependencyObject sender, DependencyPropertyChangedEventArgs args)
		{
			(sender as FrameworkElement).OnDataContextChanged (args.OldValue, args.NewValue);
		}

		void OnDataContextChanged (object oldValue, object newValue)
		{
			InvalidateLocalBindings ();
			InvalidateSubtreeBindings ();
		}

		public event EventHandler LayoutUpdated {
			add {
				if (layoutUpdatedListener == null)
					layoutUpdatedListener = new WeakLayoutUpdatedListener (Deployment.Current, this);
				layoutUpdated += value;
			}
			remove {
				layoutUpdated -= value;
				if (layoutUpdated == null && layoutUpdatedListener != null) {
					layoutUpdatedListener.Detach ();
					layoutUpdatedListener = null;
				}
			}
		}

		static MeasureOverrideCallback measure_cb = InvokeMeasureOverride;
		static ArrangeOverrideCallback arrange_cb = InvokeArrangeOverride;
		static GetDefaultTemplateCallback get_default_template_cb = InvokeGetDefaultTemplate;
		static LoadedCallback loaded_hook_cb = InvokeLoadedHook;
		EventHandler layoutUpdated;
		IWeakListener layoutUpdatedListener;

		private static bool UseNativeLayoutMethod (Type type)
		{
			return type == typeof (FrameworkElement)
				|| type == typeof (Canvas)
				|| type == typeof (Grid);
		}

		bool OverridesGetDefaultTemplate ()
		{
			return this is ContentPresenter
				|| this is ItemsPresenter
				|| this is ItemsControl
				|| this is ContentControl
				|| this is Viewbox;
		}
		
		private bool OverridesLayoutMethod (string name)
		{
			var method = GetType ().GetMethod (name, BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.FlattenHierarchy);
			if (method == null)
				return false;

			if (!method.IsVirtual || !method.IsFamily)
				return false;

			if (method.ReturnType != typeof (Size))
				return false;

			if (UseNativeLayoutMethod (method.DeclaringType))
				return false;

			var parameters = method.GetParameters ();
			if (parameters.Length != 1 || parameters [0].ParameterType != typeof (Size))
				return false;

			return true;
		}

		private new void Initialize ()
		{
			// FIXME this should not be handled using Events.AddHandler, since those handlers are removable via the plugin

			// hook up the TemplateApplied callback so we
			// can notify controls when their template has
			// been instantiated as a visual tree.
			Events.AddHandler (this, EventIds.FrameworkElement_TemplateAppliedEvent, template_applied);

			MeasureOverrideCallback measure = null;
			ArrangeOverrideCallback arrange = null;
			GetDefaultTemplateCallback getTemplate = null;
			LoadedCallback loaded = null;

			if (OverridesLayoutMethod ("MeasureOverride"))
				measure = FrameworkElement.measure_cb;
			if (OverridesLayoutMethod ("ArrangeOverride"))
				arrange = FrameworkElement.arrange_cb;
			if (OverridesGetDefaultTemplate ())
				getTemplate = FrameworkElement.get_default_template_cb;
			loaded = FrameworkElement.loaded_hook_cb;

			NativeMethods.framework_element_register_managed_overrides (native, measure, arrange, getTemplate, loaded);
		}

		internal void ApplyDefaultStyle ()
		{
			NativeMethods.framework_element_apply_default_style (native);
		}

		internal bool ApplyTemplate ()
		{
			return NativeMethods.framework_element_apply_template (native);
		}

		public new object FindName (string name)
		{
			return base.FindName (name);
		}

		internal void SetTemplateBinding (DependencyProperty dp, TemplateBindingExpression tb)
		{
			try {
				SetValue (dp, tb);
			} catch {
				// Do nothing here - The DP should still have its default value
			}
		}

		public BindingExpressionBase SetBinding (DependencyProperty dp, Binding binding)
		{
			return BindingOperations.SetBinding (this, dp, binding);
		}

		protected virtual Size MeasureOverride (Size availableSize)
		{
			return NativeMethods.framework_element_measure_override (native, availableSize);
		}

		protected virtual Size ArrangeOverride (Size finalSize)
		{
			return NativeMethods.framework_element_arrange_override (native, finalSize);
		}

		public new DependencyObject Parent {
			get; private set;
		}

		internal DependencyObject SubtreeObject {
			get; private set;
		}		

		internal override void AddStrongRef (IntPtr referent, string name)
		{
			if (name == "LogicalParent")
				Parent = Value.ToObject (referent) as DependencyObject;
			else if (name == "SubtreeObject")
				SubtreeObject = Value.ToObject (referent) as DependencyObject;
			else
				base.AddStrongRef (referent, name);
		}

		internal override void ClearStrongRef (IntPtr referent, string name)
		{
			if (name == "LogicalParent")
				Parent = null;
			else if (name == "SubtreeObject")
				SubtreeObject = null;
			else
				base.ClearStrongRef (referent, name);
		}

		public event EventHandler<ValidationErrorEventArgs> BindingValidationError;

		internal virtual void InvokeOnApplyTemplate ()
		{
			OnApplyTemplate ();
		}

		internal virtual void InvokeLoaded ()
		{

		}

		static Size InvokeMeasureOverride (IntPtr fwe_ptr, Size availableSize, ref MoonError error)
		{
			FrameworkElement fe = null;
			try {
				fe = (FrameworkElement) NativeDependencyObjectHelper.Lookup (fwe_ptr);
				return fe.MeasureOverride (availableSize);
			} catch (Exception ex) {
				try {
					if (fe != null)
						LayoutInformation.SetLayoutExceptionElement (Deployment.Current.Dispatcher, fe);
					error = new MoonError (ex);
				} catch (Exception ex2) {
					try {
						Console.WriteLine ("Leaked exception: {0}", ex2);
					} catch {
						// Ignore
					}
				}
			}
			return new Size (); 
		}

		static Size InvokeArrangeOverride (IntPtr fwe_ptr, Size finalSize, ref MoonError error)
		{
			FrameworkElement fe = null;
			try {
				fe = (FrameworkElement) NativeDependencyObjectHelper.Lookup (fwe_ptr);
				return fe.ArrangeOverride (finalSize);
			} catch (Exception ex) {
				try {
					if (fe != null)
						LayoutInformation.SetLayoutExceptionElement (Deployment.Current.Dispatcher, fe);
					error = new MoonError (ex);
				} catch (Exception ex2) {
					try {
						Console.WriteLine ("Leaked exception: {0}", ex2);
					} catch {
						// Ignore
					}
				}
			}
			return new Size ();
		}
		
		static IntPtr InvokeGetDefaultTemplate (IntPtr fwe_ptr)
		{
			IntPtr result = IntPtr.Zero;
			UIElement root = null;
			try {
				FrameworkElement element = (FrameworkElement) NativeDependencyObjectHelper.FromIntPtr (fwe_ptr);
				root = element.GetDefaultTemplate ();
				if (root != null)
					result = root.native;
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in FrameworkElement.InvokeGetDefaultTemplate: {0}", ex);
				} catch {
					// Ignore
				}
			}
			return result;
		}

		static void InvokeLoadedHook (IntPtr fwe_ptr)
		{
			try {
				FrameworkElement element = (FrameworkElement) NativeDependencyObjectHelper.FromIntPtr (fwe_ptr);
				element.InvokeLoaded ();
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in FrameworkElement.InvokeLoaded: {0}", ex);
				} catch {
					// Ignore
				}
			}
		}
		
		internal virtual UIElement GetDefaultTemplate ()
		{
			return null;
		}
		
		public virtual void OnApplyTemplate ()
		{
			// according to doc this is not fully implemented since SL templates applies
			// to Control/ContentPresenter and is defined here for WPF compatibility
		}

		void IListenLayoutUpdated.OnLayoutUpdated (object sender, EventArgs e)
		{
			// Explicitly use null as the sender in this event, as per docs
			var h = layoutUpdated;
			if (h != null)
				h (null, EventArgs.Empty);
		}

		internal void RaiseBindingValidationError (ValidationErrorEventArgs e)
		{
			FrameworkElement element = this;
			e.OriginalSource = this;
			
			while (element != null) {
				EventHandler <ValidationErrorEventArgs> h = element.BindingValidationError;
				if (h != null)
					h (element, e);
				element = VisualTreeHelper.GetParent (element) as FrameworkElement;
			}
		}

		public BindingExpression GetBindingExpression (DependencyProperty dp)
		{
			Expression expression;
			if (expressions.TryGetValue (dp, out expression))
				return expression as BindingExpression;

			return null;
		}

		#region UIA Events and Methods

		// All events are 1-1 to each attached property defined in:
		// System.Windows.Automation.AutomationProperties

		internal event DependencyPropertyChangedEventHandler AcceleratorKeyChanged;
		internal event DependencyPropertyChangedEventHandler AccessKeyChanged;
		internal event DependencyPropertyChangedEventHandler AutomationIdChanged;
		internal event DependencyPropertyChangedEventHandler HelpTextChanged;
		internal event DependencyPropertyChangedEventHandler IsRequiredForFormChanged;
		internal event DependencyPropertyChangedEventHandler ItemStatusChanged;
		internal event DependencyPropertyChangedEventHandler ItemTypeChanged;
		internal event DependencyPropertyChangedEventHandler LabeledByChanged;
		internal event DependencyPropertyChangedEventHandler NameChanged;

		internal void RaiseAcceleratorKeyChanged (DependencyPropertyChangedEventArgs args)
		{
			if (AcceleratorKeyChanged != null)
				AcceleratorKeyChanged (this, args);
		}

		internal void RaiseAccessKeyChanged (DependencyPropertyChangedEventArgs args)
		{
			if (AccessKeyChanged != null)
				AccessKeyChanged (this, args);
		}

		internal void RaiseAutomationIdChanged (DependencyPropertyChangedEventArgs args)
		{
			if (AutomationIdChanged != null)
				AutomationIdChanged (this, args);
		}

		internal void RaiseHelpTextChanged (DependencyPropertyChangedEventArgs args)
		{
			if (HelpTextChanged != null)
				HelpTextChanged (this, args);
		}

		internal void RaiseIsRequiredForFormChanged (DependencyPropertyChangedEventArgs args)
		{
			if (IsRequiredForFormChanged != null)
				IsRequiredForFormChanged (this, args);
		}

		internal void RaiseItemStatusChanged (DependencyPropertyChangedEventArgs args)
		{
			if (ItemStatusChanged != null)
				ItemStatusChanged (this, args);
		}

		internal void RaiseItemTypeChanged (DependencyPropertyChangedEventArgs args)
		{
			if (ItemTypeChanged != null)
				ItemTypeChanged (this, args);
		}

		internal void RaiseLabeledByChanged (DependencyPropertyChangedEventArgs args)
		{
			if (LabeledByChanged != null)
				LabeledByChanged (this, args);
		}

		internal void RaiseNameChanged (DependencyPropertyChangedEventArgs args)
		{
			if (NameChanged != null)
				NameChanged (this, args);
		}

		#endregion
	}
}
