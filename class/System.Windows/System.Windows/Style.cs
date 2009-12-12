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
using System.Windows.Markup;

using Mono;

namespace System.Windows {

	public sealed partial class Style : DependencyObject {

		public Style (Type targetType) : this ()
		{
			TargetType = targetType;
		}

		public void Seal()
		{
			NativeMethods.style_seal (native);
		}

		private void ConvertSetterValue (Setter s)
		{
			DependencyProperty dp = s.Property as DependencyProperty;
			object val = s.Value;

			if (dp.PropertyType == typeof (string)) {
				if (val == null) 
					throw new ArgumentException ("foo1");
				else if (val.GetType () != typeof (string))
					throw new XamlParseException ("foo2");
			}
			
			try {
				s.ConvertedValue = MoonlightTypeConverter.ConvertObject (dp, val, TargetType, true);
			} catch (Exception ex) {
				throw new XamlParseException (ex.Message);
			}
		}

		internal void ConvertSetterValues ()
		{
			foreach (Setter s in Setters)
				ConvertSetterValue (s);
		}

		public Style BasedOn {
			get; set;
		}
	}

}
