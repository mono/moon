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
using System.Windows.Media.Imaging;
using System.Windows.Documents;
using System.Reflection;

namespace Mono {

	internal class MoonlightTypeConverter : TypeConverter {

		protected bool nullableDestination;
		protected Kind destinationKind;
		protected Type destinationType;
		protected string propertyName;

		public MoonlightTypeConverter (string propertyName, Type destinationType)
		{
			this.propertyName = propertyName;
			this.destinationType = destinationType;

			destinationKind = Deployment.Current.Types.TypeToKind (destinationType);
			if (destinationKind == Kind.INVALID)
				throw new InvalidOperationException (string.Format ("Cannot convert to type {0} (property {1})", destinationType, propertyName));

			var nullable = Nullable.GetUnderlyingType (destinationType);
			if (nullable != null) {
				this.destinationType = nullable;
				nullableDestination = true;
			}
		}

		public override bool CanConvertFrom (ITypeDescriptorContext context, Type sourceType)
		{
			if (sourceType == typeof(string))
				return true;

			// allow specifying SolidColorBrushes using color literals
			if (sourceType == typeof(Color) && destinationType.IsAssignableFrom(typeof(SolidColorBrush)))
				return true;

			if (IsAssignableToIConvertible (sourceType) && IsAssignableToIConvertible (destinationType))
				return true;

			if (destinationType.IsAssignableFrom (sourceType))
				return true;

			// in 2.0 we need to allow converting from integers to FontStyle/FontStretch/FontWeight
			if (Deployment.Current.RuntimeVersion[0] == '2' &&
			    sourceType == typeof (UInt32) &&
			    (destinationType == typeof (FontStyle) ||
			     destinationType == typeof (FontStretch) ||
			     destinationType == typeof (FontWeight)))
				return true;
			    
			return base.CanConvertFrom (context, sourceType);
		}

		public override object ConvertFrom (ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value)
		{
			if (destinationType == typeof (object))
				return value;

			if (destinationType == typeof (string))
				return value.ToString ();

			if (destinationType.IsInstanceOfType (value))
				return value;
			
			string str_val = value as String;
			if (str_val != null) {
				if (destinationType.IsEnum)
					return Enum.Parse (destinationType, str_val, true);
				
				if (destinationType == typeof (GridLength)) {
					if (String.Compare (str_val, "Auto", true) == 0)
						return new GridLength (1, GridUnitType.Auto);
					else {
						str_val = str_val.Trim ();
						var length = 1.0;
						var type = str_val.EndsWith ("*") ? GridUnitType.Star : GridUnitType.Pixel;
						if (str_val.Length == 0)
							length = 0.0;
						if (type == GridUnitType.Star)
							str_val = str_val.Substring (0, str_val.Length - 1);
						if (str_val.Length > 0)
							length = double.Parse (str_val, Helper.DefaultCulture);

						return new GridLength (length, type);
					}
				}

				if (destinationType == typeof (int))
					return int.Parse (str_val, NumberStyles.Any, Helper.DefaultCulture);

				if (destinationType == typeof (double) && str_val == "Auto")
					return Double.NaN;
				
				if (destinationType == typeof (TimeSpan)) {
					TimeSpan span;
					if (TimeSpan.TryParse (str_val, out span))
						return span;
				}

				if (destinationType == typeof (FontWeight))
					return new FontWeight ((FontWeightKind) Enum.Parse (typeof (FontWeightKind), str_val, true));

				if (destinationType == typeof (FontStyle))
					return new FontStyle ((FontStyleKind) Enum.Parse (typeof (FontStyleKind), str_val, true));

				if (destinationType == typeof (FontStretch))
					return new FontStretch ((FontStretchKind) Enum.Parse (typeof (FontStretchKind), str_val, true));

				if (destinationType == typeof (Cursor))
					return Cursors.FromEnum ((CursorType) Enum.Parse (typeof (CursorType), str_val, true));

				if (destinationType == typeof (CacheMode)) {
					if (str_val == "BitmapCache")
						return new BitmapCache ();
				}

				if (destinationType == typeof (Rect)) {
					return Rect.FromString (str_val);
				}

				if (destinationType == typeof (Point)) {
					return Point.FromString (str_val);
				}

				if (destinationType == typeof (FontFamily)) {
					return new FontFamily (str_val);
				}
				
				if (destinationType == typeof (TextDecorationCollection)) {
					if (str_val == "Underline") {
						return TextDecorations.Underline;
					} else if (str_val == "None") {
						return null;
					}
				}

				if (destinationType == typeof (System.Globalization.CultureInfo))
					return CultureInfo.GetCultureInfo (str_val);

				if (destinationType == typeof (ImageSource) ||
				    destinationType == typeof (BitmapSource) ||
				    destinationType == typeof (BitmapImage))
					return new BitmapImage (new Uri (str_val, UriKind.RelativeOrAbsolute));
			}

			if (value is Color && destinationType.IsAssignableFrom(typeof(SolidColorBrush))) {
				return new SolidColorBrush ((Color)value);
			}

			// FontStretch/Style/Weight and enums are stored as uint32 in 2.0
			if (Deployment.Current.RuntimeVersion[0] == '2' && value is UInt32) {
				if (destinationType == typeof (FontStyle))
					return new FontStyle ((FontStyleKind)value);
				
				if (destinationType == typeof (FontStretch))
					return new FontStretch ((FontStretchKind)value);
				
				if (destinationType == typeof (FontWeight))
					return new FontWeight ((FontWeightKind)value);
				
				if (destinationType.IsEnum) {
					try {
						object v = Enum.ToObject (destinationType, value);
						
						if (Enum.IsDefined (destinationType, v))
							return v;
					} catch (Exception) {
					}
				}
			}
			
			if (IsAssignableToIConvertible (value.GetType ()) && IsAssignableToIConvertible (destinationType))
				return ValueFromConvertible (destinationType, (IConvertible) value);
			
			if (value is Thickness) {
				if (destinationType == typeof (CornerRadius)) {
					Thickness thickness = (Thickness) value;
					
					// Give the same results as if we did Thickness.ToString() and then parsed that as a CornerRadius
					return new CornerRadius (thickness.Left, thickness.Top, thickness.Right, thickness.Bottom);
				} else if (destinationType == typeof (double))
					return ((Thickness) value).Left;
			}
			
			if (str_val != null) {
				Kind k = destinationKind;

				IntPtr unmanaged_value = IntPtr.Zero;
				try {
					if (NativeMethods.value_from_str (k,
									   propertyName,
									   str_val,
									   out unmanaged_value)) {
						value = Value.ToObject (destinationType, unmanaged_value);
						return value;
					}
				} finally {
					NativeMethods.value_delete_value2 (unmanaged_value);
				}
			}

			if (destinationType.IsAssignableFrom (value.GetType ()))
				return value;
			
			// The base implementation doesn't do anything but
			// throw, so throw here instead so we have more
			// context.
			throw new NotImplementedException (String.Format ("Unimplemented type conversion from {0} to {1}",
									  value.GetType ().ToString (),
									  destinationType.ToString ()));
			
			return base.ConvertFrom (context, culture, value);
		}

		public override object ConvertTo (ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
		{
			// Yeah, this is kinda broken. Why would ConvertTo just call ConvertFrom in a proper converter, eh?
			// We need this so we can support ConvertFrom/ConvertTo properly in databinding.
			this.destinationType = destinationType;
			this.destinationKind = Deployment.Current.Types.TypeToKind (destinationType);
			if (destinationKind == Kind.STRING)
				return value == null ? null : value.ToString ();
			return ConvertFrom (context, culture, value);
		}

		public static bool IsAssignableToIConvertible (Type type)
		{
			return typeof (IConvertible).IsAssignableFrom (type);
		}

		public static object ValueFromConvertible (Type type, IConvertible value)
		{
			if (type == typeof (string))
				return Convert.ToString (value, CultureInfo.InvariantCulture);
			if (type == typeof (bool))
				return Convert.ToBoolean (value, CultureInfo.InvariantCulture);
			if (type == typeof (byte))
				return Convert.ToByte (value, CultureInfo.InvariantCulture);
			if (type == typeof (char))
				return Convert.ToChar (value, CultureInfo.InvariantCulture);
			if (type == typeof (DateTime))
				return Convert.ToDateTime (value, CultureInfo.InvariantCulture);
			if (type == typeof (Decimal))
				return Convert.ToDecimal (value, CultureInfo.InvariantCulture);
			if (type == typeof (double))
				return Convert.ToDouble (value, CultureInfo.InvariantCulture);
			if (type == typeof (Int16))
				return Convert.ToInt16 (value, CultureInfo.InvariantCulture);
			if (type == typeof (Int32))
				return Convert.ToInt32 (value, CultureInfo.InvariantCulture);
			if (type == typeof (Int64))
				return Convert.ToInt64 (value, CultureInfo.InvariantCulture);
			if (type == typeof (SByte))
				return Convert.ToSByte (value, CultureInfo.InvariantCulture);
			if (type == typeof (Single))
				return Convert.ToSingle (value, CultureInfo.InvariantCulture);
			if (type == typeof (UInt16))
				return Convert.ToUInt16 (value, CultureInfo.InvariantCulture);
			if (type == typeof (UInt32))
				return Convert.ToUInt32 (value, CultureInfo.InvariantCulture);
			if (type == typeof (UInt64))
				return Convert.ToUInt64 (value, CultureInfo.InvariantCulture);
			
			return value;
		}

		public static object ConvertObject (PropertyInfo prop, object val, Type objectType)
		{
			// Should i return default(T) if property.PropertyType is a valuetype?
			if (val == null)
				return val;
			
			if (prop.PropertyType.IsAssignableFrom (val.GetType ()))
				return val;

			if (prop.PropertyType == typeof (string))
				return val.ToString ();
			
			TypeConverter tc = Helper.GetConverterFor (prop, prop.PropertyType);
			if (tc == null)
				tc = new MoonlightTypeConverter (prop.Name, prop.PropertyType);

			return tc.ConvertFrom (null, Helper.DefaultCulture, val);
		}

		public static object ConvertObject (DependencyProperty dp, object val, Type objectType, bool doToStringConversion)
		{
			// Should i return default(T) if property.PropertyType is a valuetype?
			if (val == null)
				return val;
			
			if (dp.PropertyType.IsAssignableFrom (val.GetType ()))
				return val;

			if (dp.PropertyType == typeof (string))
				return doToStringConversion ? val.ToString() : "";
			
			TypeConverter tc = null;
			
			if (dp.IsAttached) {
				tc = Helper.GetConverterFor (GetGetterMethodForAttachedDP (dp, val), dp.PropertyType);
			}
			else {
				PropertyInfo pi = dp.DeclaringType.GetProperty (dp.Name);
				if (pi != null) {
					tc = Helper.GetConverterFor (pi, pi.PropertyType);
					if (tc == null)
						tc = new MoonlightTypeConverter (pi.Name, pi.PropertyType);
				}
			}
			
			if (tc == null)
				tc = new MoonlightTypeConverter (dp.Name, dp.PropertyType);
			
			return tc.ConvertFrom (val);
		}

		private static MethodInfo GetGetterMethodForAttachedDP (DependencyProperty dp, object obj)
		{
			MethodInfo res = dp.DeclaringType.GetMethod (String.Concat ("Get", dp.Name), BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
			return res;
		}

	}
}
