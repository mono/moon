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
		bool parsedPath;
		PropertyInfo info;
		
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
				if (Binding.Source != null)
					return Binding.Source;
				if (Target != null)
					return Target.DataContext;
				return null;
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

			if (target == null) {
				Console.WriteLine ("Target was null");
				return null;
			}
			// FIXME: What if the path is invalid? A.B....C, AB.C£$.D etc
			// Can you have an explicit interface implementation? Probably not.
			string[] parts = Binding.Path.Path.Split (new char[] { '.' });
			if (parts.Length == 0) {
				return null;
			}
			for (int i = 0; i < parts.Length; i++) {
				PropertyInfo p = target.GetType ().GetProperty (parts [i]);

				// The property does not exist, so abort.
				if (p == null) {
					Console.WriteLine ("The property didn't exist");
					return null;
				}
				if (!p.DeclaringType.IsVisible)
					throw new MethodAccessException (string.Format ("Property {0} cannot be accessed", p.Name));
				
				if (i != (parts.Length - 1)) {
					target = p.GetValue (target, null);
					continue;
				}
				
				if (Binding.Mode == BindingMode.OneWay && target is INotifyPropertyChanged) {
					((INotifyPropertyChanged)target).PropertyChanged += delegate(object sender, PropertyChangedEventArgs e) {
						if (p.Name.EndsWith (e.PropertyName))
							Target.SetValue (Property, PropertyInfo.GetValue (PropertyTarget, null));
					};
				}
				
				PropertyTarget = target;
				return p;
			}

			throw new Exception ("Should not be reached");
		}

		
		internal bool TryGetValue (out object value)
		{
			if (string.IsNullOrEmpty (Binding.Path.Path)) {
				// If the path is empty, return the active DataSource
				value = DataSource;
				return true;
			}	
			else if (PropertyInfo == null) {
				// If the property doesn't exist, the value is null
				value = null;
				return false;
			}
			else
				value = PropertyInfo.GetValue (PropertyTarget, null);

			if (Binding.Converter != null) {
				value = Binding.Converter.Convert (value,
				                                   Property.PropertyType,
				                                   Binding.ConverterParameter,
				                                   Binding.ConverterCulture ?? System.Globalization.CultureInfo.CurrentCulture);
			}
			return true;
		}
		
		internal void SetValue (object value)
		{
			if (Binding.Converter != null)
				value = Binding.Converter.ConvertBack (value,
				                                       PropertyInfo.PropertyType,
				                                       Binding.ConverterParameter,
				                                       Binding.ConverterCulture ?? System.Globalization.CultureInfo.CurrentCulture);

			PropertyInfo.SetValue (PropertyTarget, value, null);
		}
	}
}
