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
using System.Linq;
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

		internal bool cached;
		object cachedValue;
		
		UnmanagedPropertyChangeHandler updateDataSourceCallback;
		
		internal Binding Binding {
			get; private set;
		}
		
		ValidationError CurrentError {
			get; set;
		}

		INotifyDataErrorInfo CurrentNotifyError {
			get; set;
		}

		FrameworkElement DataContextSource {
			get; set;
		}

		internal object DataSource {
			get { return PropertyPathWalker.Source; }
		}

		bool IsBoundToAnyDataContext {
			get {
				return string.IsNullOrEmpty (Binding.ElementName)
					&& Binding.Source == null
					&& Binding.RelativeSource == null;
			}
		}

		bool IsSelfDataContextBound {
			get {
				return IsBoundToAnyDataContext
					&& Target is FrameworkElement
					&& Property != FrameworkElement.DataContextProperty;
			}
		}

		bool IsParentDataContextBound {
			get {
				return IsBoundToAnyDataContext
					&& Target is FrameworkElement
					&& (Property == FrameworkElement.DataContextProperty || Property == ContentPresenter.ContentProperty);
			}
		}

		bool IsMentorDataContextBound {
			get {
				return IsBoundToAnyDataContext && (!(Target is FrameworkElement));
			}
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
		void CalculateDataSource ()
		{
			object source = null;
			// There are four possible ways to get the source:
			// Binding.Source, Binding.ElementName, Binding.RelativeSource and finally the fallback to DataContext.
			// Only one of the first three will be non-null
			if (Binding.Source != null) {
				source = Binding.Source;
			} else if (Binding.ElementName != null) {
				// If we 'Target' in a custom DP it's possible
				// 'Target' won't be able to find the ElementName.
				// In this case we just use the Mentor and hope.
				source = Target.FindName (Binding.ElementName);
				if (source == null && Target.Mentor != null)
					source = Target.Mentor.FindName (Binding.ElementName);

				// When doing ElementName bindings we need to know when we've been
				// added to the live tree in order to invalidate the binding and do
				// the name lookup again. If we can't find a mentor and Target isn't
				// a FrameworkElement, we need to wait for the mentor to be attached
				// and then do the lookup when it's loaded.
				var feTarget = Target as FrameworkElement ?? Target.Mentor;
				if (feTarget == null) {
					Target.MentorChanged += InvalidateAfterMentorChanged;
				} else {
					feTarget.Loaded += HandleFeTargetLoaded;
				}
			} else if (Binding.RelativeSource != null) {
				if (Binding.RelativeSource.Mode == RelativeSourceMode.Self) {
					source = Target;
				} else if (Binding.RelativeSource.Mode == RelativeSourceMode.TemplatedParent) {
					source = Target.TemplateOwner;
				}
			} else {
				// If we've bound to a FrameworkElements own DataContext property or the ContentProperty, we need
				// to read the datacontext of the parent element.
				var fe = Target as FrameworkElement;
				if (fe != null && (Property == FrameworkElement.DataContextProperty || Property == ContentPresenter.ContentProperty))
					fe = (FrameworkElement) fe.VisualParent;

				// If the FE's parent is null or if the Target is not an FE, we should take the datacontext from the mentor.
				// Note that we use the mentor here for the first time because we don't want to accidentally select the mentors
				// parent in the previous block of code.
				fe = fe ?? Target.Mentor;
				if (fe != null)
					source = fe.DataContext;
			}

			if (PropertyPathWalker != null)
				PropertyPathWalker.Update (source);
		}

		void InvalidateAfterMentorChanged (object sender, EventArgs e)
		{
			// This is only called if we bound to a DependencyObject with an ElementName
			// based binding and the DO initially had no mentor. We now have a mentor so
			// we can do our name lookup.
			Target.MentorChanged -= InvalidateAfterMentorChanged;
			var o = Target.Mentor.FindName (Binding.ElementName);
			if (o == null) {
				Target.Mentor.Loaded += HandleFeTargetLoaded;
			} else {
				PropertyPathWalker.Update (o);
			}
			Invalidate ();
			Target.SetValue (Property, this);
		}

		void HandleFeTargetLoaded (object sender, RoutedEventArgs e)
		{
			// This is only called if we have an ElementName based binding
			// and could not find the object named by 'ElementName'. This means
			// that the element has been added to the live tree and has been loaded
			// so the odds are we should be able to find the named element.
			FrameworkElement fe = (FrameworkElement) sender;
			fe.Loaded -= HandleFeTargetLoaded;
			PropertyPathWalker.Update (Target.FindName (Binding.ElementName));
			Invalidate ();
			Target.SetValue (Property, this);
		}

		internal BindingExpressionBase (Binding binding, DependencyObject target, DependencyProperty property)
		{
			Binding = binding;
			Target = target;
			Property = property;

			bool bindsToView = property == FrameworkElement.DataContextProperty || property.PropertyType == typeof (IEnumerable) || property.PropertyType == typeof (ICollectionView);
			PropertyPathWalker = new PropertyPathWalker (Binding.Path.ParsePath, binding.BindsDirectlyToSource, bindsToView, IsBoundToAnyDataContext);
			if (Binding.Mode != BindingMode.OneTime)
				PropertyPathWalker.ValueChanged += PropertyPathValueChanged;
		}

		internal override void OnAttached (DependencyObject element)
		{
			if (Attached)
				return;

			base.OnAttached (element);
			CalculateDataSource ();

			if (TwoWayTextBoxText)
				((TextBox) Target).LostFocus += TextBoxLostFocus;

			var targetFE = element as FrameworkElement;
			if (IsMentorDataContextBound) {
				Target.MentorChanged += MentorChanged;
				SetDataContextSource (Target.Mentor);
			} else if (IsParentDataContextBound) {
				targetFE.VisualParentChanged += ParentChanged;
				SetDataContextSource ((FrameworkElement) targetFE.VisualParent);
			} else if (IsSelfDataContextBound) {
				SetDataContextSource (targetFE);
			}

			if (Binding.Mode == BindingMode.TwoWay && Property is CustomDependencyProperty) {
				updateDataSourceCallback = delegate {
					try {
						if (!Updating)
							TryUpdateSourceObject (Target.GetValue (Property));
					} catch (Exception ex) {
						try {
							Console.WriteLine ("Moonlight: Unhandled exception in BindingExpressionBase.OnAttached's updateDataSourceCallback: {0}", ex);
						} catch {
							// Ignore
						}
					}
				};
				Target.AddPropertyChangedHandler (Property, updateDataSourceCallback);
			}
		}

		internal override void OnDetached (DependencyObject element)
		{
			if (!Attached)
				return;

			base.OnDetached (element);
			if (TwoWayTextBoxText)
				((TextBox) Target).LostFocus -= TextBoxLostFocus;

			var targetFE = element as FrameworkElement;
			if (IsMentorDataContextBound) {
				Target.MentorChanged -= MentorChanged;
				SetDataContextSource (null);
			} else if (IsParentDataContextBound) {
				targetFE.VisualParentChanged -= ParentChanged;
				SetDataContextSource (null);
			} else if (IsSelfDataContextBound) {
				SetDataContextSource (null);
			}

			if (updateDataSourceCallback != null) {
				Target.RemovePropertyChangedHandler (Property, updateDataSourceCallback);
				updateDataSourceCallback = null;
			}

			PropertyPathWalker.Update (null);
		}

		void SetDataContextSource (FrameworkElement fe)
		{
			if (DataContextSource != null)
				DataContextSource.RemovePropertyChangedHandler (FrameworkElement.DataContextProperty, DataContextChanged);
			DataContextSource = fe;
			if (DataContextSource != null) {
				DataContextSource.AddPropertyChangedHandler (FrameworkElement.DataContextProperty, DataContextChanged);
				PropertyPathWalker.Update (DataContextSource.DataContext);
			}
		}

		void AttachToNotifyError (INotifyDataErrorInfo element)
		{
			if (CurrentNotifyError == element || !Binding.ValidatesOnNotifyDataErrors)
				return;

			string property = "";
			if (PropertyPathWalker.FinalNode.PropertyInfo != null)
				property = PropertyPathWalker.FinalNode.PropertyInfo.Name;
			if (CurrentNotifyError != null) {
				CurrentNotifyError.ErrorsChanged -= NotifyErrorsChanged;
				MaybeEmitError (null, null);
			}

			CurrentNotifyError = element;

			if (CurrentNotifyError != null) {
				CurrentNotifyError.ErrorsChanged += NotifyErrorsChanged;
				if (CurrentNotifyError.HasErrors) {
					foreach (var v in CurrentNotifyError.GetErrors (property)) {
						MaybeEmitError (v as string, v as Exception);
					}
				} else {
					MaybeEmitError (null, null);
				}
			}
		}

		void NotifyErrorsChanged (object o, DataErrorsChangedEventArgs e)
		{
			string property = "";
			if (PropertyPathWalker.FinalNode.PropertyInfo != null)
				property = PropertyPathWalker.FinalNode.PropertyInfo.Name;
			if (e.PropertyName == property) {
				var errors = CurrentNotifyError.GetErrors (property);
				if (errors != null) {
					var errorList = CurrentNotifyError.GetErrors (property).Cast <object> ().ToArray ();
					if (errorList.Length > 0) {
						foreach (var v in errorList) {
							MaybeEmitError (v, v as Exception);
						}
					} else {
						MaybeEmitError (null, null);
					}
				} else {
					MaybeEmitError (null, null);
				}
			}
		}

		void MentorChanged (object sender, EventArgs e)
		{
			try {
				SetDataContextSource (Target.Mentor);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in BindingExpressionBase.MentorChanged: {0}", ex);
				} catch {
					// Ignore
				}
			}
		}

		void ParentChanged (object sender, EventArgs e)
		{
			try {
				var targetFE = (FrameworkElement) Target;
				SetDataContextSource ((FrameworkElement) targetFE.VisualParent);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in BindingExpressionBase.ParentChanged: {0}", ex);
				} catch {
					// Ignore
				}
			}
		}

		void DataContextChanged (IntPtr dependency_object, IntPtr propertyChangedEventArgs, ref MoonError error, IntPtr closure)
		{
			try {
				var fe = (FrameworkElement) NativeDependencyObjectHelper.Lookup (dependency_object);
				PropertyPathWalker.Update (fe.DataContext);
				// OneTime bindings refresh when the datacontext changes. As these bindings do not listen
				// for the ValueChanged notifications from the PropertyPathWalker we need to force a refresh
				Refresh ();
			} catch (Exception ex) {
				try {
					error = new MoonError (ex);
				} catch {
				}
			}
		}

		void Refresh ()
		{
			string dataError = null;
			Exception exception = null;
			// If we detach the binding, we set the Source of the PropertyPathWalker to null
			// and emit a ValueChanged event which tries to update the target. We need to ignore this.
			if (!Attached)
				return;

			var node = PropertyPathWalker.FinalNode;
			AttachToNotifyError (node.Source as INotifyDataErrorInfo);

			if (!Updating && Binding.ValidatesOnDataErrors && node.Source is IDataErrorInfo && node.PropertyInfo != null)
				dataError = ((IDataErrorInfo) node.Source) [node.PropertyInfo.Name];

			bool oldUpdating = Updating;
			try {
				Updating = true;
				Invalidate ();
				Target.SetValue (Property, this);
			} catch (Exception ex) {
				if (Binding.ValidatesOnExceptions) {
					if (ex is TargetInvocationException)
						ex = ex.InnerException;
					exception = ex;
				}
			}
			finally {
				Updating = oldUpdating;
			}

			MaybeEmitError (dataError, exception);
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

		TypeConverter GetConverterFrom ()
		{
			if (PropertyPathWalker.IsPathBroken)
				return null;

			var sourceType = Property.PropertyType;
			var destType = PropertyPathWalker.FinalNode.ValueType;
			if (destType.IsAssignableFrom (sourceType)) {
				return null;
			}

			return GetConverter (destType, String.Empty, destType);
		}

		TypeConverter GetConverterTo (Type sourceType)
		{
			if (PropertyPathWalker.IsPathBroken)
				return null;

			var destType = Property.PropertyType;
			if (destType.IsAssignableFrom (sourceType))
				return null;

			return GetConverter (sourceType, Property.Name, Property.PropertyType);
		}

		// refactor to reduce [SSC] code duplication
		TypeConverter GetConverter (Type type, string name, Type propertyType)
		{
			TypeConverter converter = Helper.GetConverterFor (type);
			if (converter == null)
				converter = new MoonlightTypeConverter (name, propertyType);
			return converter;
		}

		internal override object GetValue (DependencyProperty dp)
		{
			if (cached)
				return cachedValue;

			cached = true;
			if (PropertyPathWalker.IsPathBroken) {
				cachedValue = null;
			}
			else {
				cachedValue = PropertyPathWalker.Value;
			}
			try {
				cachedValue = ConvertToType (dp, cachedValue);
			} catch {
				cachedValue = dp.GetDefaultValue (Target);
			}
			
			return cachedValue;
		}

		object ConvertToType (DependencyProperty dp, object value)
		{
			try {
				if (!PropertyPathWalker.IsPathBroken && Binding.Converter != null) {
					value = Binding.Converter.Convert (
						value,
						Property.PropertyType,
						Binding.ConverterParameter,
						GetConverterCulture ()
					);
				}

				if (value == DependencyProperty.UnsetValue || PropertyPathWalker.IsPathBroken) {
					value = Binding.FallbackValue ?? dp.GetDefaultValue (Target);
				}
				else if (value == null) {
					value = Binding.TargetNullValue;
					if (value == null && IsBoundToAnyDataContext && string.IsNullOrEmpty (Binding.Path.Path))
						value = dp.GetDefaultValue (Target);
				} else {
					string format = Binding.StringFormat;
					if (!string.IsNullOrEmpty (format)) {
						if (!format.Contains ("{0"))
							format = "{0:" + format + "}";
						value = string.Format (GetConverterCulture (), format, value);
					}
				}

				if (value != null) {
					var converter = GetConverterTo (value.GetType ());
					if (converter == null) {
						if (PropertyPathWalker.IsPathBroken) {
							value = MoonlightTypeConverter.ConvertObject (dp, value, Target.GetType (), true);
						}
					}
					else {
						value = converter.ConvertTo (null, GetConverterCulture (), value, dp.PropertyType);
					}
				}
			} catch (Exception ex) {
				return MoonlightTypeConverter.ConvertObject (dp, Binding.FallbackValue, Target.GetType (), true);
			}
			return value;
		}

		void TextBoxLostFocus (object sender, RoutedEventArgs e)
		{
			UpdateSourceObject ();
		}

		void PropertyPathValueChanged (object o, EventArgs EventArgs)
		{
			Refresh ();
		}

		void MaybeEmitError (object message, Exception exception)
		{
			// If we've databound to a DependencyObject we need to emit
			// the error on the Mentor, if it has one.
			var fe = Target as FrameworkElement ?? Target.Mentor;
			if (fe == null) {
				return;
			}

			if (message is string && (string) message == "")
				message = null;

			var oldError = CurrentError;
			if (message != null)
				CurrentError = new ValidationError (message, null);
			else if (exception != null)
				CurrentError = new ValidationError (null, exception);
			else
				CurrentError = null;

			// We had an error and now we have a new error
			if (oldError != null && CurrentError != null) {
				Validation.AddError (fe, CurrentError);
				Validation.RemoveError (fe, oldError);
				if (Binding.NotifyOnValidationError) {
					fe.RaiseBindingValidationError (new ValidationErrorEventArgs(ValidationErrorEventAction.Removed, oldError));
					fe.RaiseBindingValidationError (new ValidationErrorEventArgs(ValidationErrorEventAction.Added, CurrentError));
				}
			} else if (oldError != null) {
				Validation.RemoveError (fe, oldError);
				if (Binding.NotifyOnValidationError)
					fe.RaiseBindingValidationError (new ValidationErrorEventArgs(ValidationErrorEventAction.Removed, oldError));
			} else if (CurrentError != null) {
				Validation.AddError (fe, CurrentError);
				if (Binding.NotifyOnValidationError)
					fe.RaiseBindingValidationError (new ValidationErrorEventArgs(ValidationErrorEventAction.Added, CurrentError));
			}
		}

		internal void TryUpdateSourceObject (object value)
		{
			if (!Updating && Binding.UpdateSourceTrigger == UpdateSourceTrigger.Default) {
				UpdateSourceObject (value, false);
			}
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
			UpdateSourceObject (false);
		}

		internal void UpdateSourceObject (bool force)
		{
			UpdateSourceObject (Target.GetValue (Property), force);
		}

		internal void UpdateSourceObject (object value, bool force)
		{
			// We can only update two way bindings.
			if (Binding.Mode != BindingMode.TwoWay)
				return;

			string dataError = null;
			Exception exception = null;
			bool oldUpdating = Updating;
			var node = PropertyPathWalker.FinalNode;

			try {
				// If the user calls BindingExpresion.UpdateSource (), we must update regardless of focus state.
				// Otherwise we only update if the textbox is unfocused.
				if (!force && TwoWayTextBoxText && System.Windows.Input.FocusManager.GetFocusedElement () == Target) {
					return;
				}
				
				if (PropertyPathWalker.IsPathBroken) {
					return;
				}
				
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
					TryUseParseMethod ((string) value, node.ValueType, ref value);
				}
				
				try {
					if (value != null) {
						var converter = GetConverterFrom ();
						value = converter == null ? value : converter.ConvertFrom (null, GetConverterCulture (), value);
					}
				} catch {
					return;
				}
				if (cachedValue == null) {
					if (value == null) {
						return;
					}
				}

				Updating = true;
				node.SetValue (value);
				cachedValue = value;
			} catch (Exception ex) {
				if (Binding.ValidatesOnExceptions) {
					if (ex is TargetInvocationException)
						ex = ex.InnerException;
					exception = ex;
				}
			}
			finally {
				Updating = oldUpdating;
				if (Binding.ValidatesOnDataErrors && exception == null && node.Source is IDataErrorInfo && node.PropertyInfo != null) {
					dataError = ((IDataErrorInfo) node.Source) [node.PropertyInfo.Name];
				}
			}

			MaybeEmitError (dataError, exception);
		}
	}
}
