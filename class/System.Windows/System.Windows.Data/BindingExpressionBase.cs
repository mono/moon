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
using System.Windows.Controls;

using Mono;

namespace System.Windows.Data {

	public abstract class BindingExpressionBase : Expression
	{
		internal bool cached;
		object cachedValue;
		INotifyPropertyChanged cachedSource;
		
		bool parsedPath;
		PropertyInfo info;
		bool updatingSource;
		
		internal Binding Binding {
			get; private set;
		}
		
		UnmanagedEventHandler LostFocusHandler {
			get; set;
		}

		FrameworkElement Target {
			get; set;
		}
		
		internal bool UpdatingSource {
			get { return updatingSource; }
		}
		
		DependencyProperty Property {
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

		
		internal BindingExpressionBase (Binding binding, FrameworkElement target, DependencyProperty property)
		{
			Binding = binding;
			Target = target;
			Property = property;

			if (target is TextBox && Property == TextBox.TextProperty && binding.Mode == BindingMode.TwoWay) {
				LostFocusHandler = TextBoxLostFocus;
				Events.AddHandler (target, "LostFocus", LostFocusHandler);
			}
		}

		internal override void Dispose ()
		{
			if (LostFocusHandler != null) {
				Events.RemoveHandler (Target, "LostFocus", LostFocusHandler);
				LostFocusHandler = null;
			}

			if (cachedSource != null)
				cachedSource.PropertyChanged -= PropertyChanged;
			cachedSource = null;
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
					if (source == null)
						return null;
					else
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
			try {
				if (updatingSource || PropertyInfo == null)
					return;
	
				if (Binding.Converter != null)
					value = Binding.Converter.ConvertBack (value,
					                                       PropertyInfo.PropertyType,
					                                       Binding.ConverterParameter,
					                                       Binding.ConverterCulture ?? Helper.DefaultCulture);
	
				if (value != null) {
					Type destType = PropertyInfo.PropertyType;
					if (PropertyInfo.PropertyType.IsValueType && PropertyInfo.PropertyType != value.GetType ()) {
						try {
							if (destType.IsGenericType && destType.GetGenericTypeDefinition () == typeof (Nullable<>))
								destType = destType.GetGenericArguments () [0];
							if (destType.IsEnum)
								value = Enum.Parse (destType, value.ToString (), true);
							else
								value = Convert.ChangeType (value, destType, null);
						} catch {
							Console.WriteLine ("Failed to convert '{0}' to '{1}", value.GetType (), PropertyInfo.PropertyType);
							return;
						}
					}
				}
				
				if (cachedValue == null) {
					if (value == null)
						return;
				}
				else if (cachedValue.Equals (value)) {
					return;
				}

				updatingSource = true;
				PropertyInfo.SetValue (PropertySource, value, null);
				cachedValue = value;
			} catch (Exception ex) {
				if (Binding.NotifyOnValidationError && Binding.ValidatesOnExceptions) {
					Target.RaiseBindingValidationError (new ValidationErrorEventArgs (ValidationErrorEventAction.Added, new ValidationError (ex)));
				}
			}
			finally {
				updatingSource = false;
			}
		}

		void PropertyChanged (object sender, PropertyChangedEventArgs e)
		{
			try {
				updatingSource = true;
				if (string.IsNullOrEmpty (Binding.Path.Path)) {
					Target.SetValueImpl (Property, ConvertToDestType (DataSource));
				} else if (PropertyInfo == null) {
					return;
				} else if (PropertyInfo.Name.Equals (e.PropertyName)) {
					object value = ConvertToDestType (PropertyInfo.GetValue (PropertySource, null));
					Target.SetValueImpl (Property, value);
				}
			} finally {
				updatingSource = false;
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
			
			if (defined_converter && value != null && !value.GetType ().IsSubclassOf (Property.PropertyType)) {
				converter = new MoonlightValueConverter ();
				
				value = converter.Convert (value,
				                           Property.PropertyType,
				                           Binding.ConverterParameter,
				                           Binding.ConverterCulture ?? Helper.DefaultCulture);
			}
			
			return value;
		}

		void TextBoxLostFocus (IntPtr sender, IntPtr calldata, IntPtr closure)
		{
			string text = ((TextBox)Target).Text;
			SetValue (text);
		}
	}
}