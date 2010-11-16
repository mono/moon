//
// PropertyPathNode.cs
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
using System.Collections;
using System.ComponentModel;
using System.Reflection;
using Mono;

namespace System.Windows.Data
{
	class StandardPropertyPathNode : PropertyPathNode {

		Mono.UnmanagedPropertyChangeHandler dpChanged;

		public string PropertyName {
			get; private set;
		}

		public string TypeName {
			get; private set;
		}

		public StandardPropertyPathNode (string typeName, string propertyName)
		{
			TypeName = typeName;
			PropertyName = propertyName;
		}

		protected override void OnSourceChanged (object oldSource, object newSource)
		{
			base.OnSourceChanged (oldSource, newSource);

			var old_do = oldSource as DependencyObject;
			var new_do = newSource as DependencyObject;
			if (dpChanged != null) {
				Mono.NativeMethods.dependency_object_remove_property_change_handler (old_do.native, DependencyProperty.Native, dpChanged);
				dpChanged = null;
			}

			DependencyProperty = null;
			PropertyInfo = null;
			if (Source == null)
				return;

			var type = Source.GetType ();
			if (TypeName != null)
				type = Application.GetComponentTypeFromName (TypeName);

			if (type == null)
				return;

			DependencyProperty prop;
			Types.Ensure (type);
			if (new_do != null && DependencyProperty.TryLookup (Deployment.Current.Types.TypeToKind (type), PropertyName, out prop)) {
				DependencyProperty = prop;
				dpChanged = DPChanged;
				Mono.NativeMethods.dependency_object_add_property_change_handler (new_do.native, DependencyProperty.Native, dpChanged, IntPtr.Zero);
			}

			// If there's an attached DP called 'Foo' and also a regular CLR property
			// called 'Foo', we cannot use the property info from the CLR property.
			// Otherwise if the CLR property declares a type converter, we could
			// inadvertantly use it when we should not.
			if (DependencyProperty == null || !DependencyProperty.IsAttached) {
				PropertyInfo = type.GetProperty (PropertyName);
			}
		}

		void DPChanged (IntPtr dependency_object, IntPtr property_changed_event_args, ref MoonError error, IntPtr closure)
		{
			try {
				UpdateValue ();
				if (Next != null)
					Next.SetSource (Value);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Unhandled exception in StandardPropertyPathNode.DBChanged: {0}", ex);
				} catch {
					// Ignore
				}
			}
		}

		protected override void OnSourcePropertyChanged (object o, PropertyChangedEventArgs e)
		{
			if (e.PropertyName == PropertyName && PropertyInfo != null) {
				UpdateValue ();
				if (Next != null)
					Next.SetSource (Value);
			}
		}

		public override void SetValue (object value)
		{
			if (DependencyProperty != null)
				((DependencyObject) Source).SetValue (DependencyProperty, value);
			else if (PropertyInfo != null)
				PropertyInfo.SetValue (Source, value, null);
		}

		public override void UpdateValue ()
		{
			if (DependencyProperty != null) {
				ValueType = DependencyProperty.PropertyType;
				Value = ((DependencyObject) Source).GetValue (DependencyProperty);
			} else if (PropertyInfo != null) {
				ValueType = PropertyInfo.PropertyType;
				try {
					Value = PropertyInfo.GetValue (Source, null);
				} catch {
					Value = null;
				}
			} else {
				ValueType = null;
				Value = null;
			}
		}
	}

	class BooleanToVisibilityConverter
	{
		private static readonly bool?[] inverseMappings = new Nullable<bool>[] { true, false, null };
		private readonly Visibility[] mappings = new Visibility[3] { Visibility.Visible, Visibility.Collapsed, Visibility.Collapsed };

	}
}
