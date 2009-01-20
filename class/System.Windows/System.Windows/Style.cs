//
// Style.cs
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
using System.Windows.Markup;

using Mono;

namespace System.Windows {

	[ContentPropertyAttribute("Setters", true)]
	public sealed partial class Style : DependencyObject {

		public Style (Type targetType) : this ()
		{
			TargetType = targetType;
		}

		public void Seal()
		{
			NativeMethods.style_seal (native);
		}

		internal void ConvertSetterValue (Setter s)
		{
			// can't use s.Value and s.Property here due to wonky behavior with their accessors
			DependencyProperty dp = s.GetValue (Setter.PropertyProperty) as DependencyProperty;
			object val = s.GetValue (Setter.ValueProperty);

			Console.WriteLine ("Converting setter ({0}, {1}/{2})", dp.Name, val, val == null ? "<null>" : val.GetType().ToString());

			if (val != null && dp.PropertyType.IsAssignableFrom (val.GetType())) {
				Console.WriteLine ("+ property type is assignable.  we're golden");
				s.ConvertedValue = val;
			}
			else if (dp.PropertyType == typeof (string)) {
				Console.WriteLine ("+ property type is string");
				if (val == null) 
					throw new ArgumentException ("foo1");
				else if (val.GetType () != typeof (string))
					throw new XamlParseException ("foo2");

				s.ConvertedValue = val;
			}
			else if (val != null) {
				Console.WriteLine ("+ property type is more complex");

				PropertyInfo pi = TargetType.GetProperty (dp.Name);
				if (pi == null) {
					Console.WriteLine ("+ failed to look up CLR property wrapper");
					throw new XamlParseException ("foo3");
				}

				TypeConverter tc = Helper.GetConverterFor (pi, pi.PropertyType);
				if (tc == null)
					tc = new MoonlightTypeConverter (pi.PropertyType);

				if (!tc.CanConvertFrom (val.GetType())) {
					Console.WriteLine ("+ type converter can't convert from type {0}", val.GetType());
					throw new XamlParseException ("foo5");
				}

				try {
					s.ConvertedValue = tc.ConvertFrom (val);
					Console.WriteLine ("s.ConvertedValue == {0}", s.ConvertedValue);
				} catch (Exception e) {
					Console.WriteLine ("Exception raised in ConvertFrom():\n{0}", e);
					throw new XamlParseException ("foo6");
				}
			}
			else {
				Console.WriteLine ("how did we make it here?");
			}
		}

		internal void ConvertSetterValues ()
		{
			Console.WriteLine ("Inside ConvertSetterValues (TargetType = {0})", TargetType);

			foreach (Setter s in Setters)
				ConvertSetterValue (s);
		}
	}

}
