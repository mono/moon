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

using System.Collections;
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

		FrameworkElement Target {
			get; set;
		}
		
		internal bool UpdatingSource {
			get { return updatingSource; }
		}
		
		DependencyProperty Property {
			get; set;
		}
		
		bool TwoWayTextBoxText {
			get { return Target is TextBox && Property == TextBox.TextProperty && Binding.Mode == BindingMode.TwoWay; }
		}

		// This is the object we're databound to
		internal object DataSource {
			get {
				object source = null;
				// There are four possible ways to get the source:
				// Binding.Source, Binding.ElementName, Binding.RelativeSource and finally the fallback to DataContext.
				// Only one of the first three will be non-null
				if (Binding.Source != null) {
					source = Binding.Source;
				} else if (Binding.ElementName != null) {
					source = Target.FindName (Binding.ElementName);
					// FIXME: Not sure if this should be an exception or not. Tests need to be written.
					if (source == null)
						Console.WriteLine ("*** WARNING *** The element referenced in Binding.ElementName could not be found");
				} else if (Binding.RelativeSource != null) {
					if (Binding.RelativeSource.Mode == RelativeSourceMode.Self) {
						source = Target;
					} else if (Binding.RelativeSource.Mode == RelativeSourceMode.TemplatedParent) {
						// FIXME: What happens if there is no template owner?
						source = Target.TemplateOwner;
					} else {
						Console.WriteLine ("*** WARNING *** Unsupported RelativeSourceMode '{0}'", Binding.RelativeSource.Mode);
					}
				}

				// If DataContext is bound, then we need to read the parents datacontext or use null
				if (source == null && Target != null) {
					if (Property == FrameworkElement.DataContextProperty || Property == ContentPresenter.ContentProperty) {
						FrameworkElement e = Target.Parent as FrameworkElement;
						if (e != null) {
							source = e.DataContext;
						}
					} else {
						source = Target.DataContext;
					}
				}

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

		internal int PropertyIndexer {
			get; private set;
		}

		internal BindingExpressionBase (Binding binding, FrameworkElement target, DependencyProperty property)
		{
			Binding = binding;
			Target = target;
			Property = property;

			if (TwoWayTextBoxText)
				((TextBox) target).LostFocus += TextBoxLostFocus;

			PropertyIndexer = -1;
		}

		internal override void Dispose ()
		{
			if (TwoWayTextBoxText)
				((TextBox) Target).LostFocus -= TextBoxLostFocus;

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
				string prop_name = parts [i];
				string indexer = null;
				PropertyInfo p = null;
				int idx = -1;

				int close = parts [i].LastIndexOf (']');
				if (close > -1) {
					int open = parts [i].LastIndexOf ("[");
					prop_name = parts [i].Substring (0, open);
					indexer = parts [i].Substring (open + 1, close - open - 1);
					
				}

				p = source.GetType ().GetProperty (prop_name);
				
				// The property does not exist, so abort.
				if (p == null)
					return null;

				if (!p.DeclaringType.IsVisible)
					throw new MethodAccessException (string.Format ("Property {0} cannot be accessed", p.Name));

				if (indexer != null) {
					if (!Int32.TryParse (indexer, out idx))
						throw new ArgumentException ("Invalid value for indexer.");
				}

				if (i != (parts.Length - 1)) {
					if (indexer != null) {
						IList list = p.GetValue (source, null) as IList;
						if (list == null)
							throw new ArgumentException ("Indexer on non list type.");
						source = list [idx];
					} else 
						source = p.GetValue (source, null);
					if (source == null)
						return null;
					else
						continue;
				}

				PropertySource = source;
				PropertyIndexer = idx;
				return p;
			}

			throw new Exception ("Should not be reached");
		}

		public void Invalidate ()
		{
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
				cachedValue = GetPropertyValue ();
			}
			try {
				cachedValue = ConvertToType (dp, cachedValue);
			} catch {
				cachedValue  = dp.DefaultValue;
			}
			
			return cachedValue;
		}

		internal void SetValue (object value)
		{
			try {
				// TextBox.Text only updates a two way binding if it is *not* focused.
				if (TwoWayTextBoxText && System.Windows.Input.FocusManager.GetFocusedElement () == Target)
					return;
				
				if (updatingSource || PropertyInfo == null)
					return;
				
				if (Binding.Converter != null)
					value = Binding.Converter.ConvertBack (value,
					                                       PropertyInfo.PropertyType,
					                                       Binding.ConverterParameter,
					                                       Binding.ConverterCulture ?? Helper.DefaultCulture);
				
				value = MoonlightTypeConverter.ConvertObject (PropertyInfo, value, Target.GetType ());
				if (cachedValue == null) {
					if (value == null)
						return;
				}
				else if (cachedValue.Equals (value)) {
					return;
				}

				updatingSource = true;
				SetPropertyValue (value);
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
					Target.SetValueImpl (Property, ConvertToType (Property, DataSource));
				} else if (PropertyInfo == null) {
					return;
				} else if (PropertyInfo.Name.Equals (e.PropertyName)) {
					object value = ConvertToType (Property, GetPropertyValue ());
					Target.SetValueImpl (Property, value);
				}
			} catch {
				//Type conversion exceptions are silently swallowed
			} finally {
				updatingSource = false;
			}
		}
		
		object ConvertToType (DependencyProperty dp, object value)
		{
			if (Binding.Converter != null) {
				value = Binding.Converter.Convert (value,
			                           Property.PropertyType,
			                           Binding.ConverterParameter,
			                           Binding.ConverterCulture ?? Helper.DefaultCulture);
			}
			return MoonlightTypeConverter.ConvertObject (dp, value, Target.GetType (), true);
		}

		void TextBoxLostFocus (object sender, RoutedEventArgs e)
		{
			SetValue (((TextBox) sender).Text);
		}

		object GetPropertyValue ()
		{
			object res = PropertyInfo.GetValue (PropertySource, null);

			if (res != null && PropertyIndexer != -1) {
				IList list = res as IList;
				if (list == null)
					throw new ArgumentException ("Indexer on non list type");
				res = list [PropertyIndexer];
			}

			return res;
		}

		void SetPropertyValue (object value)
		{
			if (PropertyIndexer == -1) {
				PropertyInfo.SetValue (PropertySource, value, null);
				return;
			}

			object source = PropertyInfo.GetValue (PropertySource, null);

			if (source != null && PropertyIndexer != -1) {
				IList list = source as IList;
				if (list == null)
					throw new ArgumentException ("Indexer on non list type");
				list [PropertyIndexer] = value;
			}
			
		}
	}
}
