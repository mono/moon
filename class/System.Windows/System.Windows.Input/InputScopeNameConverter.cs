//
// InputScopeNameConverter.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using System.ComponentModel;
using System.Globalization;

namespace System.Windows.Input {
	[EditorBrowsable (EditorBrowsableState.Never)]
	public class InputScopeNameConverter : TypeConverter {
		public InputScopeNameConverter ()
		{
		}

		public override bool CanConvertFrom (ITypeDescriptorContext context, Type sourceType)
		{
			return sourceType == typeof (string);
		}

		public override bool CanConvertTo (ITypeDescriptorContext context, Type destinationType)
		{
			if ((context == null) || (context.Instance == null) || !(context.Instance is InputScopeName))
				return false;

			return (destinationType == typeof (string));
		}

		public override object ConvertFrom (ITypeDescriptorContext context, CultureInfo culture, object source)
		{
			InputScopeName isn = new InputScopeName ();
			if (source == null)
				return isn;

			Type type = source.GetType ();
			if (!CanConvertFrom (type))
				return isn;

			if (type == typeof (string)) {
				string value = (source as string);
				if (value.Length == 0)
					return isn;

				if (value.IndexOf (',') == -1) {
					isn.NameValue = Parse (value);
				} else {
					string[] values = value.Split (new char [] { ',' }, StringSplitOptions.RemoveEmptyEntries);
					foreach (string v in values)
						Parse (v);
					isn.NameValue = InputScopeNameValue.PhraseList;
				}
				return isn;
			}

			throw new NotImplementedException ();
		}

		public override object ConvertTo (ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
		{
			InputScopeName isn = (value as InputScopeName);
			if ((isn == null) || (destinationType != typeof (string)))
				throw new NotImplementedException ();

			return isn.NameValue.ToString ();
		}

		internal static InputScopeNameValue Parse (string s)
		{
			if (String.IsNullOrEmpty (s))
				return InputScopeNameValue.Default;

			if (s.IndexOf (',') == -1)
				return ParseValue (s);

			string[] values = s.Split (new char [] { ',' }, StringSplitOptions.RemoveEmptyEntries);
			foreach (string v in values)
				ParseValue (v);

			return InputScopeNameValue.PhraseList;
		}

		static InputScopeNameValue ParseValue (string s)
		{
			for (int i = (int) InputScopeNameValue.EnumString; i <= (int) InputScopeNameValue.ApplicationEnd; i++) {
				InputScopeNameValue isnv = (InputScopeNameValue) i;
				if (String.Compare (s, isnv.ToString (), StringComparison.Ordinal) == 0)
					return isnv;
			}
			throw new ArgumentException ();
		}
	}
}

