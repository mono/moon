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

namespace System.Windows.Data {

	public abstract class BindingExpressionBase : Expression
	{
		bool parsedPath;
		PropertyInfo info;
		
		internal Binding Binding {
			get; set;
		}

		internal FrameworkElement Element {
			get; set;
		}
		
		internal DependencyProperty Property {
			get; set;
		}

		internal object DataSource {
			get {
				object target = Binding.Source;
				FrameworkElement element = Element;
				
				while (target == null && element != null) {
					if (element.DataContext != null)
					target = element.DataContext;
					else
						element = element.Parent as FrameworkElement;
				}
				return target;
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

		internal object PropertyTarget {
			get; private set;
		}

		
		protected BindingExpressionBase ()
		{
			
		}

		PropertyInfo GetPropertyInfo ()
		{
			object target = DataSource;
			
			// FIXME: What if the path is invalid? A.B....C, AB.C£$.D etc
			// Can you have an explicit interface implementation? Probably not.
			string[] parts = Binding.Path.Path.Split (new char[] { '.' });
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
				
				if (Binding.Mode == BindingMode.OneWay && target is INotifyPropertyChanged) {
					((INotifyPropertyChanged)target).PropertyChanged += delegate(object sender, PropertyChangedEventArgs e) {
						if (p.Name.EndsWith (e.PropertyName))
							Element.SetValue (Property, PropertyInfo.GetValue (PropertyTarget, null));
					};
				}
				
				PropertyTarget = target;
				return p;
			}

			throw new Exception ("Should not be reached");
		}

		internal bool TryGetValue (out object value)
		{
			value = null;

			// If the property doesn't exist, return false
			if (PropertyInfo == null)
				return false;
			else
				value = PropertyInfo.GetValue (PropertyTarget, null);
			
			return true;
		}
	}
}
