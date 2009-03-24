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
using System.Windows;
using System.Collections.Generic;

using Mono;

namespace System.Windows.Data {

	public abstract class BindingExpressionBase : Expression
	{
		static Dictionary<IntPtr, Binding> bindings = new Dictionary<IntPtr, Binding> ();
		
		internal bool cached;
		object cachedValue;
		INotifyPropertyChanged cachedSource;
		
		bool parsedPath;
		PropertyInfo info;
		bool updating;
		
		internal Binding Binding {
			get; set;
		}

		internal FrameworkElement Target {
			get; set;
		}
		
		internal bool Updating {
			get { return updating; }
		}
		
		internal DependencyProperty Property {
			get; set;
		}

		// This is the object we're databound to
		internal object DataSource {
			get {
				object source = null;
				if (Binding.Source != null)
					source = Binding.Source;
				if (source == null && Target != null)
					source = Target.DataContext;

				// If the datasource has changed, disconnect from the old object and reconnect
				// to the new one.
				if (source != cachedSource) {
					if (cachedSource != null)
						cachedSource.PropertyChanged -= PropertyChanged;
					cachedSource = source as INotifyPropertyChanged;
					if (cachedSource != null)
						cachedSource.PropertyChanged += PropertyChanged;
				}
				return source;
			}
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
		internal object PropertySource {
			get; private set;
		}

		
		internal BindingExpressionBase ()
		{
			
		}

		PropertyInfo GetPropertyInfo ()
		{
			object source = DataSource;

			if (source == null)
				return null;

			// FIXME: What if the path is invalid? A.B....C, AB.C£$.D etc
			// Can you have an explicit interface implementation? Probably not.
			string[] parts = Binding.Path.Path.Split (new char[] { '.' });
			if (parts.Length == 0) {
				return null;
			}
			for (int i = 0; i < parts.Length; i++) {
				PropertyInfo p = source.GetType ().GetProperty (parts [i]);

				// The property does not exist, so abort.
				if (p == null)
					return null;

				if (!p.DeclaringType.IsVisible)
					throw new MethodAccessException (string.Format ("Property {0} cannot be accessed", p.Name));
				
				if (i != (parts.Length - 1)) {
					source = p.GetValue (source, null);
					continue;
				}

				PropertySource = source;
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
		
		internal override object GetValue (DependencyProperty dp)
		{
			if (cached)
				return cachedValue;

			cached = true;
			if (DataSource == null) {
				cachedValue = dp.DefaultValue;
				return cachedValue;
			}
			else if (string.IsNullOrEmpty (Binding.Path.Path)) {
				// If the path is empty, return the active DataSource
				cachedValue = DataSource;
			}	
			else if (PropertyInfo == null) {
				cachedValue = dp.DefaultValue;
				return cachedValue;
			}
			else {
				cachedValue = PropertyInfo.GetValue (PropertySource, null);
			}
			cachedValue = ConvertToDestType (cachedValue);
			
			return cachedValue;
		}

		internal void SetValue (object value)
		{
			if (updating)
				return;

			if (Binding.Converter != null)
				value = Binding.Converter.ConvertBack (value,
				                                       PropertyInfo.PropertyType,
				                                       Binding.ConverterParameter,
				                                       Binding.ConverterCulture ?? Helper.DefaultCulture);
			
			if (value != null && PropertyInfo.PropertyType.IsValueType && PropertyInfo.PropertyType != value.GetType ())
				value = Convert.ChangeType (value, PropertyInfo.PropertyType, null);

			try {
				updating = true;
				PropertyInfo.SetValue (PropertySource, value, null);
			}
			finally {
				updating = false;
			}
		}

		void PropertyChanged (object sender, PropertyChangedEventArgs e)
		{
			try {
				updating = true;
				if (string.IsNullOrEmpty (Binding.Path.Path)) {
					Target.SetValueImpl (Property, ConvertToDestType (DataSource));
				} else if (PropertyInfo == null) {
					return;
				} else if (PropertyInfo.Name.Equals (e.PropertyName)) {
					object value = ConvertToDestType (PropertyInfo.GetValue (PropertySource, null));
					Target.SetValueImpl (Property, value);
				}
			} finally {
				updating = false;
			}
		}
		
		object ConvertToDestType (object value)
		{
			IValueConverter converter = Binding.Converter;
			bool defined_converter = true;
			if (converter == null) {
				defined_converter = false;
				converter = new MoonlightValueConverter();
			}
			
			value = converter.Convert (value,
			                           Property.PropertyType,
			                           Binding.ConverterParameter,
			                           Binding.ConverterCulture ?? Helper.DefaultCulture);
			
			if (defined_converter && !value.GetType ().IsSubclassOf (Property.PropertyType)) {
				converter = new MoonlightValueConverter ();
				
				value = converter.Convert (value,
				                           Property.PropertyType,
				                           Binding.ConverterParameter,
				                           Binding.ConverterCulture ?? Helper.DefaultCulture);
			}
			
			return value;
		}
	}
}