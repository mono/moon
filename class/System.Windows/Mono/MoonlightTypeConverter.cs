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
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;

namespace Mono {

	internal class MoonlightTypeConverter : TypeConverter {
		Kind destinationKind;
		Type destinationType;
		string propertyName;

		public MoonlightTypeConverter (string propertyName, Type destinationType)
		{
			this.propertyName = propertyName;
			this.destinationType = destinationType;

			if (destinationType.IsEnum)
				destinationType = typeof (Int32);

			destinationKind = Deployment.Current.Types.TypeToKind (destinationType);
			if (destinationKind == Kind.INVALID)
				throw new InvalidOperationException (string.Format ("Cannot convert to type {0} (property {1})", destinationType, propertyName));
		}

		public override bool CanConvertFrom (ITypeDescriptorContext context, Type sourceType)
		{
			if (sourceType == typeof(string))
				return true;

			// allow specifying SolidColorBrushes using color literals
			if (sourceType == typeof(Color) && destinationType.IsAssignableFrom(typeof(SolidColorBrush)))
				return true;

			if (Helper.IsAssignableToIConvertible (sourceType) && Helper.IsAssignableToIConvertible (destinationType))
				return true;

			return base.CanConvertFrom (context, sourceType);
		}

		public override object ConvertFrom (ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value)
		{
			if (destinationType == typeof (object))
				return value;

			if (value is string) {
				if (destinationType.IsEnum)
					return Enum.Parse (destinationType, (string)value, true);
				
				Kind k = destinationKind;

				/* ugh.  our desire to use enums in
				   unmanaged code when the managed
				   code has structs is painful all
				   over. */
				if (k == Kind.FONTSTRETCH ||
				    k == Kind.FONTWEIGHT ||
				    k == Kind.FONTSTYLE ||
				    k == Kind.CURSOR)
					k = Kind.INT32;

				IntPtr unmanaged_value;

				if (!NativeMethods.value_from_str (k,
								   propertyName,
								   (string)value,
								   out unmanaged_value)) {
					Console.WriteLine ("could not convert value {0} to type {1} (property {1})", value, destinationType, propertyName);
					return base.ConvertFrom (context, culture, value);
				}
			
				return Value.ToObject (destinationType, unmanaged_value);
				// XXX this leaks unmanaged_value?
			}
			else if (value is Color && destinationType.IsAssignableFrom(typeof(SolidColorBrush))) {
				return new SolidColorBrush ((Color)value);
			}
			else if (Helper.IsAssignableToIConvertible (value.GetType ()) && Helper.IsAssignableToIConvertible (destinationType))
				return Helper.ValueFromConvertible (destinationType, (IConvertible) value);
			else if (!base.CanConvertFrom (context, value.GetType ())) {
				Console.Error.WriteLine ("MoonlightTypeConverter: Cannot convert from type {0} to type {1}", value.GetType(), destinationType);
			}

			return base.ConvertFrom (context, culture, value);
		}
	}

}
