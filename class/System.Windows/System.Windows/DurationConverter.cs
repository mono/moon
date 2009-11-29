//
// DurationConverter.cs
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

using System;
using System.ComponentModel;
using System.Globalization;
using System.Windows.Controls;

namespace System.Windows
{
	public class DurationConverter : TypeConverter
	{
		public DurationConverter ()
		{
		}

#if NET_2_1
		override
#endif
		public bool CanConvertFrom (ITypeDescriptorContext context, Type sourceType)
		{
			return TypeConverters.CanConvertFrom<Duration>(sourceType);
		}

#if NET_2_1
		override
#endif
		public object ConvertFrom (ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			if (value is string) {
				string text = (string) value;

				if (string.IsNullOrEmpty(text))
					return Duration.Automatic;

				if (text.ToLower () == "automatic")
					return Duration.Automatic;
				else if (text.ToLower () == "forever")
					return Duration.Forever;
				else
					return new Duration(TimeSpan.Parse (text));
			}

			return TypeConverters.ConvertFrom<Duration>(this, value);
		}
	}
}
