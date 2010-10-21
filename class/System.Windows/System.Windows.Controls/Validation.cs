//
// Validation.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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

using System;
using System.Collections.ObjectModel;

namespace System.Windows.Controls {

	public static class Validation {
		public static readonly DependencyProperty ErrorsProperty =
			DependencyProperty.RegisterAttachedCore ("Errors", typeof (ReadOnlyObservableCollection<ValidationError>), typeof (Validation), null);
		public static readonly DependencyProperty ErrorsCoreProperty =
			DependencyProperty.RegisterAttachedCore ("ErrorsCore", typeof (ObservableCollection<ValidationError>), typeof (Validation), null);
		public static readonly DependencyProperty HasErrorProperty =
			DependencyProperty.RegisterAttachedCore ("HasError", typeof (bool), typeof (Validation), null);

		internal static void AddError (FrameworkElement element, ValidationError error)
		{
			var errors = GetErrorsCore (element);
			errors.Add (error);
			if (errors.Count == 1)
				SetHasError (element, true);

			var control = element as Control;
			if (control != null)
				control.UpdateValidationState (false);
		}

		public static ReadOnlyObservableCollection<ValidationError> GetErrors (DependencyObject element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");

			var result = (ReadOnlyObservableCollection<ValidationError>) element.GetValue (ErrorsProperty);
			if (result == null) {
				result = new ReadOnlyObservableCollection<ValidationError> (GetErrorsCore (element));
				element.SetValue (ErrorsProperty, result);
			}
			return result;
		}

		static ObservableCollection<ValidationError> GetErrorsCore (DependencyObject element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");

			var result = (ObservableCollection<ValidationError>) element.GetValue (ErrorsCoreProperty);
			if (result == null) {
				result = new ObservableCollection<ValidationError> ();
				element.SetValue (ErrorsCoreProperty, result);
			}

			return result;
		}

		public static bool GetHasError (DependencyObject element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");
			return (bool) element.GetValue (HasErrorProperty);
		}

		static void SetHasError (DependencyObject element, bool value)
		{
			element.SetValue (HasErrorProperty, value);
		}

		internal static void RemoveError (FrameworkElement element, ValidationError error)
		{
			var errors = GetErrorsCore (element);
			if (errors.Remove (error)) {
				if (errors.Count == 0) {
					SetHasError (element, false);
					var control = element as Control;
					if (control != null)
						control.UpdateValidationState (true);
				}
			}
		}
	}
}
