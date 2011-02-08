//
// XamlTypeConverter.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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
using System.Text;
using System.Reflection;
using System.ComponentModel;
using System.Globalization;
using System.Windows;
using System.Windows.Media;
using System.Windows.Markup;
using System.Collections.Generic;


namespace Mono.Xaml {

	internal sealed class XamlTypeConverter : MoonlightTypeConverter {

		private XamlParser parser;
		private XamlObjectElement element;

		private delegate object TypeConverterHandler (XamlTypeConverter converter, ITypeDescriptorContext context, CultureInfo culture, object value);

		private static Dictionary<Type,TypeConverterHandler> string_converters = new Dictionary<Type,TypeConverterHandler> ();

		static XamlTypeConverter () {
			string_converters [typeof (RoutedEvent)] = ConvertRoutedEventArgs;
			string_converters [typeof (Type)] = ConvertType;
			string_converters [typeof (DependencyProperty)] = ConvertDependencyProperty;
			string_converters [typeof (Color)] = ConvertColor;
			string_converters [typeof (PropertyPath)] = ConvertPropertyPath;
			string_converters [typeof (Double)] = ConvertDouble;
			string_converters [typeof (XmlLanguage)] = ConvertXmlLanguage;
		}

		public XamlTypeConverter (XamlParser parser, XamlObjectElement element, string propertyName, Type destinationType) : base (propertyName, destinationType)
		{
			this.parser = parser;
			this.element = element;
		}
		
		public override bool CanConvertFrom (ITypeDescriptorContext context, Type sourceType)
		{
			if (destinationType == typeof (object))
				return true;

			if (sourceType == typeof (string) && CanConvertFromString (context))
				return true;

			return base.CanConvertFrom (context, sourceType);
		}

		public override object ConvertFrom (ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			if (destinationType == typeof (object))
				return true;

			string str_value = value as string;

			if (str_value != null) {
				if (destinationType.IsEnum)
					return Enum.Parse (destinationType, str_value, true);

				object o = ConvertFromString (context, culture, str_value);
				if (o != null)
					return o;
			}

			return base.ConvertFrom (context, culture, value);
		}

		private bool CanConvertFromString (ITypeDescriptorContext context)
		{
			return GetStringConverter () != null;
		}

		private TypeConverterHandler GetStringConverter ()
		{
			TypeConverterHandler handler;

			if (string_converters.TryGetValue (destinationType, out handler))
				return handler;

			foreach (Type t in string_converters.Keys) {
				if (destinationType.IsAssignableFrom (t))
					return string_converters [t];
			}

			return null;
		}

		private object ConvertFromString (ITypeDescriptorContext context, CultureInfo culture, string value)
		{
			TypeConverterHandler handler = GetStringConverter ();

			if (handler == null)
				return null;
			return handler (this, context, culture, value);
		}

		public static object ConvertObject (XamlParser parser, XamlObjectElement element, Type dest_type, TypeConverter converter, string prop_name, object val)
		{
			// Should i return default(T) if property.PropertyType is a valuetype?
			if (val == null)
				return val;
			
			if (dest_type.IsAssignableFrom (val.GetType ()))
				return val;

			if (dest_type == typeof (string))
				return val.ToString ();

			if (converter == null || ConverterIsBlackListed (converter))
				converter = new XamlTypeConverter (parser, element, prop_name, dest_type);

			return converter.ConvertFrom (null, CultureInfo.InvariantCulture, val);
		}

		//
		// Generally useless converters
		//
		private static bool ConverterIsBlackListed (TypeConverter converter)
		{
			if (converter.GetType () == typeof (PropertyPathConverter))
				return true;
			return false;
		}

		private static RoutedEvent ConvertRoutedEventArgs (XamlTypeConverter converter, ITypeDescriptorContext context, CultureInfo culture, object ovalue)
		{
			string value = (string) ovalue;

			int dot = value.IndexOf (".");
			if (dot < 1)
				throw new XamlParseException ("Invalid format for RoutedEvent.");

			string type_name = value.Substring (0, dot);
			string event_name = value.Substring (dot + 1, value.Length - dot - 1);
			Type type = converter.parser.LoadType (null, converter.parser.Context.DefaultXmlns, type_name);

			RoutedEvent res = null;
			Type eventids = typeof (EventIds);

			// Try UIElement first since thats where most of these events live
			if (typeof (UIElement).IsAssignableFrom (type)) {
				res = RoutedEvent (eventids, "UIElement", event_name);
				if (res != null)
					return res;
			}

			if (!type.IsValueType) {
				Type walk = type;
				while (walk != typeof (object)) {
					res = RoutedEvent (eventids, walk.Name, event_name);
					if (res != null)
						return res;
					walk = walk.BaseType;
				}
			}

			return null;
				
		}

		private static RoutedEvent RoutedEvent (Type eventids, string type, string event_name)
		{
			string field_name = String.Concat (type, "_", event_name, "Event");
			FieldInfo field = eventids.GetField (field_name);

			if (field == null)
				return null;

			return new RoutedEvent ((int) field.GetValue (null));
		}

		private static Type ConvertType (XamlTypeConverter converter, ITypeDescriptorContext context, CultureInfo culture, object ovalue)
		{
			string value = (string) ovalue;

			Type t = converter.parser.ResolveType (value);

			if (t == null)
				Console.Error.WriteLine ("could not convert type from: '{0}'", value);
			return t;
		}

		private static DependencyProperty ConvertDependencyProperty (XamlTypeConverter converter, ITypeDescriptorContext context, CultureInfo culture, object ovalue)
		{
			string value = (string) ovalue;

			Type target_type;

			int idx = value.IndexOf ('.');
			if (idx > 0) {
				target_type = converter.parser.ResolveType (value.Substring (0, idx));
				value = value.Substring (idx + 1, value.Length - idx - 1);
			} else
				target_type = GetTargetType (converter);

			Types.Ensure (target_type);

			ManagedType mt = Deployment.Current.Types.Find (target_type);

			DependencyProperty dp = XamlParser.LookupDependencyProperty ((Kind) mt.native_handle, value);
			return dp;
		}

		private static Type GetTargetType (XamlTypeConverter converter)
		{
			XamlElement p = converter.parser.CurrentElement.Parent;
			XamlObjectElement parent = p as XamlObjectElement;

			if (p == null)
				throw new XamlParseException ("Attempting to create a DP from an item without a target property.");

			if (parent == null)
				parent = p.Parent as XamlObjectElement;
			if (parent == null)
				throw new XamlParseException ("Attempting to create a DP from an item without a target property.");

			Style s = parent.Object as Style;

			if (s == null)
				throw new XamlParseException ("Attempting to create a DP from a non style object.");

			return s.TargetType;
		}

		private static object ConvertColor (XamlTypeConverter converter, ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			object res = Color.FromString ((string) value);
			return res;
		}
		
		private static object ConvertXmlLanguage (XamlTypeConverter converter, ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			return XmlLanguage.GetLanguage ((string) value);
		}

		private static object ConvertPropertyPath (XamlTypeConverter converter, ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			string str = (string) value;
			StringBuilder expanded = new StringBuilder (str);
			bool has_expanded = false;

			for (int i = 0; i < expanded.Length; i++) {
				if (expanded [i] == ':') {
					int e = i;
					int s = i - 1;
					int te = i + 1;
					for ( ; s > 0; s--) {
						if (!Char.IsLetterOrDigit (expanded [s]))
							break;
					}

					for ( ; te < str.Length; te++) {
						if (!Char.IsLetterOrDigit (expanded [te]) || expanded [te] == '_')
							break;
					}

					string prefix = expanded.ToString (s + 1, e - s - 1);
					string type = expanded.ToString (e + 1, te - e - 1);

					expanded.Remove (s + 1, te - s - 1);

					string ns = null;

					if (!converter.parser.Context.Xmlns.TryGetValue (prefix, out ns)) {
						Console.WriteLine ("could not find xmlns value:  {0}", prefix);
						return null;
					}

					Type t = converter.parser.ResolveType (prefix + ":" + type);
					string uri = String.Format ("'{0}'", t);
							
					expanded.Insert (s + 1, uri);

					i = s + 1 + uri.Length;
					has_expanded = true;
				}
			}

			if (has_expanded)
				return new PropertyPath (str, expanded.ToString ());

			return new PropertyPath (str);
		}

		private static object ConvertDouble (XamlTypeConverter converter, ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			string str = (string) value;

			if (StringComparer.OrdinalIgnoreCase.Equals (str, "Auto"))
				return Double.NaN;

			return null;
		}
	}

}


