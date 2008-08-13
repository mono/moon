//
// Setter.cs
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

using Mono;

namespace System.Windows {

	public sealed partial class Setter : SetterBase {
		private static readonly DependencyProperty PropertyNameProperty = DependencyProperty.Lookup (Kind.SETTER, "Property", typeof (string));
		public static readonly DependencyProperty PropertyProperty = DependencyProperty.Lookup (Kind.SETTER, "DependencyProperty", typeof (DependencyProperty));
		private static readonly DependencyProperty ValueProperty = DependencyProperty.Lookup (Kind.SETTER, "Value", typeof (object));

		public Setter (DependencyProperty property, object value)
		{
			Property = property;
			Value = value;
		}

		string PropertyName {
			get { return (string)GetValue (PropertyNameProperty); }
			set { SetValue (PropertyNameProperty, value); }
		}

		bool propertySet;
		public DependencyProperty Property {
			get {
				if (propertySet) {
					return (DependencyProperty)GetValue (PropertyProperty);
				}
				else {
					string name = PropertyName;
					if (name == null)
						return null;

					// XXX we need to look up a
					// DependencyProperty of
					// named @name from our
					// containing style's type.
					// if we have no containing
					// style, return null here.
					Console.WriteLine ("Setter.get_Property needs some love");
					return null;
				}
			}
			set {
				if (value == null)
					throw new NullReferenceException ();

				propertySet = true;
				SetValue (PropertyProperty, value);
			}
		}

		public object Value {
			get { return GetValue (ValueProperty); }
			set { SetValue (ValueProperty, value); }
		}
	}

}