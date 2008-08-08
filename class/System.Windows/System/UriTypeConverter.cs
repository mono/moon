//
// UriTypeConverter.cs
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

// Notes
// * We do not need to provide this type outside of Silverlight 2.0 (NET_2_1)
//   because this type already exists inside System.dll 2.0+ (same namepace).
// * This SL-specific version use different signatures - so the needed code 
//   was copy-pasted from mcs/class/System/System/UriTypeConverter.cs and
//   adapted.

#if NET_2_1

using System;
using System.ComponentModel;

namespace System
{	
	public sealed class UriTypeConverter : TypeConverter
	{
		public UriTypeConverter ()
		{
		}

		private bool CanConvert (Type type)
		{
			if (type == typeof (string))
				return true;
			return (type == typeof (Uri));
		}

		public override bool CanConvertFrom (Type sourceType)
		{
			if (sourceType == null)
				throw new ArgumentNullException ("sourceType");

			return CanConvert (sourceType);
		}

		public override object ConvertFrom (object value)
		{
			if (value == null)
				throw new ArgumentNullException ("value");

			Type type = value.GetType ();
			if (!CanConvertFrom (type))
				throw new NotSupportedException (type.ToString ());

			if (value is Uri)
				return value;

			string s = (value as string);
			if (s != null)
				return new Uri (s, UriKind.RelativeOrAbsolute);

			return base.ConvertFrom (value);
		}

		public override object ConvertFromString (string text)
		{
			if (text == null)
				throw new ArgumentNullException ("text");

			return new Uri (text, UriKind.RelativeOrAbsolute);
		}
	}
}
#endif
