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
using System.Globalization;

namespace System.Windows.Data {

	public abstract class BindingExpressionBase : Expression
	{
		static readonly Type [] fullParseParams = new Type [] { typeof (string), typeof (NumberStyles), typeof (IFormatProvider) };
		static readonly Type [] midParseParams = new Type [] { typeof (string), typeof (IFormatProvider) };
		static readonly Type [] simpleParseParams = new Type [] { typeof (string) };

		FrameworkElement mentor;
		UnmanagedPropertyChangeHandler mentorDataContextChangedCallback;

		internal bool cached;
		object cachedValue;
		
		UnmanagedPropertyChangeHandler updateDataSourceCallback;
		
		internal Binding Binding {
			get; private set;
		}
		
		ValidationError LastError {
			get; set;
		}

		bool IsMentorBound {
			get { return Binding.ElementName == null && Binding.Source == null && Binding.RelativeSource == null && !(Target is FrameworkElement); }
		}

		DependencyObject Target {
			get; set;
		}
		
		DependencyProperty Property {
			get; set;
		}

		PropertyPathWalker PropertyPathWalker {
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
					if (source == null)
						Console.WriteLine ("*** WARNING *** The element referenced in Binding.ElementName ('{0}') could not be found", Binding.ElementName);
				} else if (Binding.RelativeSource != null) {
					if (Binding.RelativeSource.Mode == RelativeSourceMode.Self) {
						source = Target;
					} else if (Binding.RelativeSource.Mode == RelativeSourceMode.TemplatedParent) {
						source = Target.TemplateOwner;
					} else {
						var mentor = Target.Mentor;
						if (mentor != null)
							source = mentor.DataContext;
					}
				}

				// If DataContext is bound, then we need to read the parents datacontext or use null
				if (source == null && Target != null) {
					if (Property == FrameworkElement.DataContextProperty || Property == ContentPresenter.ContentProperty) {
						FrameworkElement e = ((FrameworkElement) Target).Parent as FrameworkElement;
						if (e != null) {
							source = e.DataContext;
						}
					} else if (Target is FrameworkElement) {
						source = ((FrameworkElement) Target).DataContext;
					} else {
						var mentor = Target.Mentor;
						if (mentor != null)
							source = mentor.DataContext;
					}
				}
				if (PropertyPathWalker != null)
					PropertyPathWalker.Update (source);
				return source;
			}
		}

		internal BindingExpressionBase (Binding binding, DependencyObject target, DependencyProperty property)
		{
			Binding = binding;
			Target = target;
			Property = property;

			mentorDataContextChangedCallback = OnNativeMentorDataContextChanged;

			bool bindsToView = property == FrameworkElement.DataContextProperty || property.PropertyType == typeof (IEnumerable) || property.PropertyType == typeof (ICollectionView);
			PropertyPathWalker = new PropertyPathWalker (Binding.Path.Path, binding.BindsDirectlyToSource, bindsToView);
			if (Binding.Mode != BindingMode.OneTime)
				PropertyPathWalker.ValueChanged += PropertyPathValueChanged;
		}

		internal override void OnAttached (DependencyObject element)
		{
			base.OnAttached (element);
			if (TwoWayTextBoxText)
				((TextBox) Target).LostFocus += TextBoxLostFocus;

			if (IsMentorBound) {
				Target.MentorChanged += MentorChanged;
				mentor = Target.Mentor;
				AttachDataContextHandlers (mentor);
			}

			if (Binding.Mode == BindingMode.TwoWay && Property is CustomDependencyProperty) {
				updateDataSourceCallback = delegate {
					if (!Updating)
						TryUpdateSourceObject (Target.GetValue (Property));
				};
				Target.AddPropertyChangedHandler (Property, updateDataSourceCallback);
			}
		}

		internal override void OnDetached (DependencyObject element)
		{
			base.OnDetached (element);
			if (TwoWayTextBoxText)
				((TextBox) Target).LostFocus -= TextBoxLostFocus;

			if (IsMentorBound) {
				DetachDataContextHandlers (mentor);
				mentor = null;
				Target.MentorChanged -= MentorChanged;;
			}

			if (updateDataSourceCallback != null)
				Target.RemovePropertyChangedHandler (Property, updateDataSourceCallback);

			PropertyPathWalker.Update (null);
		}

		void AttachDataContextHandlers (FrameworkElement mentor)
		{
			if (mentor != null)
				mentor.AddPropertyChangedHandler (FrameworkElement.DataContextProperty, mentorDataContextChangedCallback);
		}

		void DetachDataContextHandlers (FrameworkElement mentor)
		{
			if (mentor != null)
				mentor.RemovePropertyChangedHandler (FrameworkElement.DataContextProperty, mentorDataContextChangedCallback);
		}

		void MentorChanged (object sender, EventArgs e)
		{
			DetachDataContextHandlers (mentor);
			mentor = Target.Mentor;
			AttachDataContextHandlers (mentor);
			MentorDataContextChanged ();
		}

		void OnNativeMentorDataContextChanged (IntPtr dependency_object, IntPtr propertyChangedEventArgs, ref MoonError error, IntPtr closure)
		{
			MentorDataContextChanged ();
		}

		void MentorDataContextChanged ()
		{
			try {
				Invalidate ();
				Updating = true;
				Target.SetValue (Property, this);
			} finally {
				Updating = false;
			}
		}

		internal void Invalidate ()
		{
			cached = false;
			cachedValue = null;
		}

		CultureInfo GetConverterCulture ()
		{
			if (Binding.ConverterCulture != null)
				return Binding.ConverterCulture;
			if (Target is FrameworkElement && ((FrameworkElement) Target).Language != null)
				return new CultureInfo (((FrameworkElement) Target).Language.IetfLanguageTag);
			return Helper.DefaultCulture;
		}

		internal override object GetValue (DependencyProperty dp)
		{
			if (cached)
				return cachedValue;

			cached = true;
			if (DataSource == null) {
				cachedValue = dp.GetDefaultValue (Target);
				return cachedValue;
			}
			else if (PropertyPathWalker.IsPathBroken) {
				// If it the path is broken, don't call the converter unless we use the fallback.
				// FIXME: Add an explicit test on this.
				if (Binding.FallbackValue == null) {
					cachedValue = dp.GetDefaultValue (Target);
					return cachedValue;
				}
				cachedValue = Binding.FallbackValue;
			}
			else {
				cachedValue = PropertyPathWalker.Value;
			}
			try {
				cachedValue = ConvertToType (dp, cachedValue);
			} catch {
				cachedValue  = dp.GetDefaultValue (Target);
			}
			
			return cachedValue;
		}

		object ConvertToType (DependencyProperty dp, object value)
		{
			if (Binding.Converter != null) {
				value = Binding.Converter.Convert (value,
			                           Property.PropertyType,
			                           Binding.ConverterParameter,
			                           GetConverterCulture ());
			}

			try {
				if (value == null) {
					value = Binding.TargetNullValue;
				} else if (value == DependencyProperty.UnsetValue) {
					value = Binding.FallbackValue;
				} else {
					string format = Binding.StringFormat;
					if (!string.IsNullOrEmpty (format)) {
						if (!format.Contains ("{0"))
							format = "{0:" + format + "}";
						value = string.Format (GetConverterCulture (), format, value);
					}
				}
				return MoonlightTypeConverter.ConvertObject (dp, value, Target.GetType (), true);
			} catch {
				return MoonlightTypeConverter.ConvertObject (dp, Binding.FallbackValue, Target.GetType (), true);
			}
		}

		void TextBoxLostFocus (object sender, RoutedEventArgs e)
		{
			UpdateSourceObject ();
		}

		void PropertyPathValueChanged (object o, EventArgs EventArgs)
		{
			Exception exception = null;
			// If we detach the binding, we set the Source of the PropertyPathWalker to null
			// and emit a ValueChanged event which tries to update the target. We need to ignore this.
			if (!Attached)
				return;

			bool oldUpdating = Updating;
			try {
				Updating = true;
				Invalidate ();
				Target.SetValueImpl (Property, this);
			} catch (Exception ex) {
				if (ex is TargetInvocationException)
					ex = ex.InnerException;
				exception = ex;
			}
			finally {
				Updating = oldUpdating;
			}
			MaybeEmitError (exception);
		}

		void MaybeEmitError (Exception exception)
		{
			var fe = Target as FrameworkElement;
			if (!Binding.ValidatesOnExceptions || !Binding.NotifyOnValidationError || fe == null)
				return;

			ValidationErrorEventArgs args;
			if (LastError != null) {
				args = new ValidationErrorEventArgs (ValidationErrorEventAction.Removed, LastError);
				fe.RaiseBindingValidationError (args);
				LastError = null;
			}

			if (exception != null) {
				LastError = new ValidationError (exception);
				args = new ValidationErrorEventArgs(ValidationErrorEventAction.Added, LastError);
				fe.RaiseBindingValidationError (args);
			}
		}

		internal void TryUpdateSourceObject (object value)
		{
			if (!Updating && Binding.Mode == BindingMode.TwoWay && Binding.UpdateSourceTrigger == UpdateSourceTrigger.Default)
				UpdateSourceObject (value);
		}

		internal bool TryUseParseMethod (string value, Type target, ref object result)
		{
			MethodInfo method;
			if ((method = target.GetMethod ("Parse", fullParseParams)) != null)
				result = method.Invoke (null, new object [] { value, NumberStyles.Any, GetConverterCulture () });
			else if ((method = target.GetMethod ("Parse", midParseParams)) != null)
				result = method.Invoke (null, new object [] { value, GetConverterCulture () });
			else if ((method = target.GetMethod ("Parse", simpleParseParams)) != null)
			    result = method.Invoke (null, new object [] { value });

			return method != null;
		}

		internal void UpdateSourceObject ()
		{
			UpdateSourceObject (Target.GetValue (Property));
		}

		internal void UpdateSourceObject (object value)
		{
			Exception exception = null;
			bool oldUpdating = Updating;
			try {
				// TextBox.Text only updates a two way binding if it is *not* focused.
				if (TwoWayTextBoxText && System.Windows.Input.FocusManager.GetFocusedElement () == Target)
					return;
				
				if (PropertyPathWalker.IsPathBroken) {
					return;
				}
				
				var node = PropertyPathWalker.FinalNode;
				if (Binding.TargetNullValue != null) {
					try {
						var v = MoonlightTypeConverter.ConvertObject (Property, Binding.TargetNullValue, Target.GetType (), true);
						if (Helper.AreEqual (v, value))
							value = null;
					} catch {
						// FIXME: I'm fairly sure we ignore this, but we need a test to be 100% sure.
					}
				}
				
				if (Binding.Converter != null)
					value = Binding.Converter.ConvertBack (value,
					                                       node.ValueType,
					                                       Binding.ConverterParameter,
					                                       GetConverterCulture ());
				if (value is string) {
					TryUseParseMethod ((string) value, node.PropertyInfo.PropertyType, ref value);
				}
				
				try {
					value = MoonlightTypeConverter.ConvertObject (node.PropertyInfo, value, Target.GetType ());
				} catch {
					return;
				}
				if (cachedValue == null) {
					if (value == null) {
						return;
					}
				}
				else if (Helper.AreEqual (cachedValue, value)) {
					return;
				}

				Updating = true;
				node.SetValue (value);
				cachedValue = value;
			} catch (Exception ex) {
				if (ex is TargetInvocationException)
					ex = ex.InnerException;
				exception = ex;
			}
			finally {
				Updating = oldUpdating;
			}

			MaybeEmitError (exception);
		}
	}
}
