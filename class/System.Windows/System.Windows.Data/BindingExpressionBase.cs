//
// BindingExpressionBase.cs
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

using System.ComponentModel;
using System.Reflection;
using System.Security;
using System.Windows;
using System.Collections.Generic;

using Mono;

namespace System.Windows.Data {

	public abstract class BindingExpressionBase : Expression
	{
		static Dictionary<IntPtr, Binding> bindings = new Dictionary<IntPtr, Binding> ();
		internal bool cached;
		object cachedValue;
		INotifyPropertyChanged cachedTarget;
		
		bool parsedPath;
		PropertyInfo info;
		bool updating;
		
		internal Binding Binding {
			get {
				IntPtr binding = NativeMethods.binding_expression_base_get_binding (Native);
				if (binding == IntPtr.Zero)
					return null;
				if (!bindings.ContainsKey (binding))
					bindings.Add (binding, new Binding (binding));
				return bindings [binding];
			}
			set {
				IntPtr binding = value == null ? IntPtr.Zero : value.Native;
				NativeMethods.binding_expression_base_set_binding (Native, binding);
				if (binding != IntPtr.Zero && !bindings.ContainsKey (binding))
					bindings.Add (binding, value);
			}
		}

		internal FrameworkElement Target {
			get; set;
		}
		
		internal DependencyProperty Property {
			get; set;
		}

		// This is the object we're databound to
		internal object DataSource {
			get {
				object target = null;
				if (Binding.Source != null)
					target = Binding.Source;
				if (target == null && Target != null)
					target = Target.DataContext;

				// If the datasource has changed, disconnect from the old object and reconnect
				// to the new one.
				if (target != cachedTarget) {
					if (cachedTarget != null)
						cachedTarget.PropertyChanged -= PropertyChanged;
					cachedTarget = target as INotifyPropertyChanged;
					if (cachedTarget != null)
						cachedTarget.PropertyChanged += PropertyChanged;
				}
				return target;
			}
		}

		internal IntPtr Native {
			get; set;
		}

		PropertyInfo PropertyInfo {
			get {
				if (!parsedPath) {
					parsedPath = true;
					info = GetPropertyInfo ();
				}
				
				return info;
			}
		}

		// This is the object at the end of the PropertyPath 
		internal object PropertyTarget {
			get; private set;
		}

		
		protected BindingExpressionBase ()
			: this (Mono.NativeMethods.binding_expression_new ())
		{
			
		}

		internal BindingExpressionBase (IntPtr native)
		{
			Native = native;
		}

		~BindingExpressionBase ()
		{
			if (Native != IntPtr.Zero) {
				Mono.NativeMethods.event_object_unref (Native);
				Native = IntPtr.Zero;
			}
		}

		PropertyInfo GetPropertyInfo ()
		{
			object target = DataSource;

			if (target == null)
				return null;

			// FIXME: What if the path is invalid? A.B....C, AB.C£$.D etc
			// Can you have an explicit interface implementation? Probably not.
			string[] parts = Binding.Path.Path.Split (new char[] { '.' });
			if (parts.Length == 0) {
				return null;
			}
			for (int i = 0; i < parts.Length; i++) {
				PropertyInfo p = target.GetType ().GetProperty (parts [i]);

				// The property does not exist, so abort.
				if (p == null)
					return null;

				if (!p.DeclaringType.IsVisible)
					throw new MethodAccessException (string.Format ("Property {0} cannot be accessed", p.Name));
				
				if (i != (parts.Length - 1)) {
					target = p.GetValue (target, null);
					continue;
				}

				PropertyTarget = target;
				return p;
			}

			throw new Exception ("Should not be reached");
		}

		public void Invalidate ()
		{
			if (Binding.Mode == BindingMode.OneTime)
				return;
			
			cached = false;
			cachedValue = null;
			info = null;
			parsedPath = false;
		}
		
		internal object GetValue (DependencyProperty dp)
		{
			if (cached)
				return cachedValue;

			cached = true;
			if (DataSource == null) {
				cachedValue = dp.DefaultValue;
			}
			else if (string.IsNullOrEmpty (Binding.Path.Path)) {
				// If the path is empty, return the active DataSource
				cachedValue = DataSource;
			}	
			else if (PropertyInfo == null) {
				cachedValue = dp.DefaultValue;
			}
			else {
				cachedValue = PropertyInfo.GetValue (PropertyTarget, null);
			}
			if (Binding.Converter != null) {
				cachedValue = Binding.Converter.Convert (cachedValue,
				                                   Property.PropertyType,
				                                   Binding.ConverterParameter,
				                                   Binding.ConverterCulture ?? System.Globalization.CultureInfo.CurrentCulture);
			}

			if (cachedValue != null) {
				if (Property.PropertyType != cachedValue.GetType() && Property.PropertyType.IsValueType && cachedValue.GetType().IsValueType) {
					cachedValue = Convert.ChangeType (cachedValue, Property.PropertyType, null);
				}
			}

			return cachedValue;
		}

		void PropertyChanged (object sender, PropertyChangedEventArgs e)
		{
			if (PropertyInfo.Name.EndsWith (e.PropertyName)) {
				object value = PropertyInfo.GetValue (PropertyTarget, null);
				if (Property.PropertyType.IsValueType && value.GetType () != Property.PropertyType)
					value = Convert.ChangeType (value, Property.PropertyType, null);

				Target.UpdateFromBinding (Property, value);
			}
		}
		
		internal void SetValue (object value)
		{
			if (updating)
				return;
			
			if (value != null && PropertyInfo.PropertyType.IsValueType && PropertyInfo.PropertyType != value.GetType ())
				value = Convert.ChangeType (value, PropertyInfo.PropertyType, null);

			if (Binding.Converter != null)
				value = Binding.Converter.ConvertBack (value,
				                                       PropertyInfo.PropertyType,
				                                       Binding.ConverterParameter,
				                                       Binding.ConverterCulture ?? System.Globalization.CultureInfo.CurrentCulture);
			try {
				updating = true;
				PropertyInfo.SetValue (PropertyTarget, value, null);
			}
			finally {
				updating = false;
			}
		}
	}
}
