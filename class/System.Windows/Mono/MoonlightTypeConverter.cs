//
// MoonlightTypeConverter.cs
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
using System;
using System.ComponentModel;
using System.Globalization;
using System.Windows.Input;

namespace Mono {

	internal class MoonlightTypeConverter : TypeConverter {
		Kind destinationKind;
		Type destinationType;
		string propertyName;

		public MoonlightTypeConverter (string propertyName, Type destinationType)
		{
			this.propertyName = propertyName;
			this.destinationType = destinationType;

			if (destinationType == typeof (Cursor))
				destinationType = typeof (Int32);

			destinationKind = Types.TypeToKind (destinationType);
			if (destinationKind == Kind.INVALID)
				throw new InvalidOperationException (string.Format ("Cannot convert to type {0} (property {1})", destinationType, propertyName));
		}

		public override bool CanConvertFrom (ITypeDescriptorContext context, Type sourceType)
		{
			return sourceType == typeof(string);
		}

		public override object ConvertFrom (ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value)
		{
			IntPtr unmanaged_value;

			if (!NativeMethods.value_from_str (destinationKind,
							   propertyName,
							   (string)value,
							   out unmanaged_value,
							   true)) {
				Console.Error.WriteLine ("XXX not sure what should go here...");
				return null;
			}
			
			return Value.ToObject (destinationType, unmanaged_value);
			// XXX this leaks unmanaged_value?
		}
	}

}
