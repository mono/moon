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
using System.Reflection;
using System.ComponentModel;
using System.Globalization;
using System.Windows;
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

			if (converter == null)
				converter = new XamlTypeConverter (parser, element, prop_name, dest_type);

			if (!converter.CanConvertFrom (val.GetType ()))
				throw new Exception (string.Format ("type converter {0} can't convert from type {1} destination type: {2}", converter.GetType (), val.GetType (), dest_type));

			return converter.ConvertFrom (null, Helper.DefaultCulture, val);
		}

		private static RoutedEvent ConvertRoutedEventArgs (XamlTypeConverter converter, ITypeDescriptorContext context, CultureInfo culture, object ovalue)
		{
			string value = (string) ovalue;

			int dot = value.IndexOf (".");
			if (dot < 1)
				throw new XamlParseException ("Invalid format for RoutedEvent.");

			string type_name = value.Substring (0, dot);
			string event_name = value.Substring (dot + 1, value.Length - dot - 1);
			Type type = converter.parser.LoadType (null, converter.parser.DefaultXmlns, type_name);

			RoutedEvent res = null;
			Type eventids = typeof (EventIds);

			// Try UIElement first since thats where most of these events live
			if (typeof (UIElement).IsAssignableFrom (type)) {
				res = RoutedEvent (eventids, "UIElement", event_name);
				if (res != null) {
					Console.WriteLine ("returning routed event:  {0}", res);
					return res;
				}
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

			Console.WriteLine ("attempting to load type:  {0}", value);
			return converter.parser.ResolveType (value);
		}

		private static DependencyProperty ConvertDependencyProperty (XamlTypeConverter converter, ITypeDescriptorContext context, CultureInfo culture, object ovalue)
		{
			string value = (string) ovalue;
			Type target_type = GetTargetType (converter);

			Types.Ensure (target_type);

			ManagedType mt = Deployment.Current.Types.Find (target_type);
			DependencyProperty dp = DependencyProperty.Lookup ((Kind) mt.native_handle, value);

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
	}

}


