//
// MarkupExpressionParser.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Data;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Windows.Markup;


namespace Mono.Xaml {

	internal class MarkupExpressionParser {

		private bool parsingBinding;
		private object target;
		private string attribute_name;
		private IntPtr parser;
		private IntPtr target_data;

		public MarkupExpressionParser (object target, string attribute_name, IntPtr parser, IntPtr target_data)
		{
			this.target = target;
			this.attribute_name = attribute_name;
			this.parser = parser;
			this.target_data = target_data;
		}
#if __TESTING
		public static void Main ()
		{
			TestParseBinding ("{Binding}");
			TestParseBinding ("{Binding MyProperty}");
			TestParseBinding ("{Binding Fill, Mode=OneWay}");
			TestParseBinding ("{Binding Source={StaticResource Brush}}");
			TestParseBinding ("{Binding Width, Path=Height, Source={StaticResource rect}, Mode=OneTime, Path=RadiusX}");
			TestParseBinding ("{Binding OpacityString, Source={StaticResource CLRObject}, Mode=OneTime}");

			Console.WriteLine ("done with tests");
		}

		public static void TestParseBinding (string exp)
		{
			MarkupExpressionParser p = new MarkupExpressionParser (null, String.Empty, IntPtr.Zero);

			try {
				p.ParseExpression (ref exp);
			} catch (Exception e) {
				Console.WriteLine ("exception while parsing:  {0}", exp);
				Console.WriteLine (e);
			}
		}
#endif

		public static bool IsTemplateBinding (string expression)
		{
			return Regex.IsMatch (expression, "^{\\s*TemplateBinding");
		}

		public static bool IsStaticResource (string expression)
		{
			return Regex.IsMatch (expression, "^{\\s*StaticResource");
		}

		public static bool IsBinding (string expression)
		{
			return Regex.IsMatch (expression, "^{\\s*Binding");
		}

		private delegate object ExpressionHandler (ref string expression);

		public object ParseExpression (ref string expression)
		{
			object result = null;
			bool rv = false;

			if (!rv)
				rv = TryHandler ("^{\\s*Binding\\s*", ParseBinding, ref expression, out result);
			if (!rv)
				rv = TryHandler ("^{\\s*StaticResource\\s*", ParseStaticResource, ref expression, out result);
			if (!rv)
				rv = TryHandler ("^{\\s*TemplateBinding\\s*", ParseTemplateBinding, ref expression, out result);
			if (!rv)
				rv = TryHandler ("^{\\s*RelativeSource\\s*", ParseRelativeSource, ref expression, out result);

			return result;
		}

		private bool TryHandler (string match, ExpressionHandler handler, ref string expression, out object result)
		{
			Match m = Regex.Match (expression, match);
			if (m == Match.Empty) {
				result = null;
				return false;
			}

			int len = m.Index + m.Length;
			expression = expression.Substring (len);
			result = handler (ref expression);
			return true;
		}

		public Binding ParseBinding (ref string expression)
		{
			Binding binding = new Binding ();
			parsingBinding  = true;
			char next;

			if (expression [0] == '}')
				return binding;

			string remaining = expression;
			string piece = GetNextPiece (ref remaining, out next, false);
			

			if (next == '=')
				HandleProperty (binding, piece, ref remaining);
			else
				binding.Path = new PropertyPath (piece);

			do {
				piece = GetNextPiece (ref remaining, out next, false);

				if (piece == null)
					break;

				HandleProperty (binding, piece, ref remaining);
			} while (true);

			parsingBinding = false;
			return binding;
		}

		public object ParseStaticResource (ref string expression)
		{
			char next;
			string name = GetNextPiece (ref expression, out next, true);

			object o = LookupNamedResource (null, name);

#if !__TESTING
			if (o == null)
				o = Application.Current.Resources [name];
#endif

			return o;
		}

		public object ParseTemplateBinding (ref string expression)
		{
			TemplateBindingExpression tb = new TemplateBindingExpression ();

			char next;
			string prop = GetNextPiece (ref expression, out next, false);
			FrameworkTemplate template = GetParentTemplate ();

			tb.Target = target as FrameworkElement;
			tb.TargetPropertyName = attribute_name;
			tb.SourcePropertyName = prop;
			// tb.Source will be filled in elsewhere between attaching the change handler.

			return tb;
		}

		public object ParseRelativeSource (ref string expression)
		{
			char next;
			string mode_str = GetNextPiece (ref expression, out next, false);

			if (!Enum.IsDefined (typeof (RelativeSourceMode), mode_str))
				throw new XamlParseException (String.Format ("MarkupExpressionParser:  Error parsing RelativeSource, unknown mode: {0}", mode_str));
				
			return new RelativeSource ((RelativeSourceMode) Enum.Parse (typeof (RelativeSourceMode), mode_str, true));
		}

		private object LookupNamedResource (DependencyObject dob, string name)
		{
			if (name == null)
				throw new XamlParseException ("you must specify a key in {StaticResource}");

			IntPtr value_ptr = NativeMethods.xaml_lookup_named_item (parser, target_data, name);
			object o = Value.ToObject (null, value_ptr);

			if (o == null && !parsingBinding)
				throw new XamlParseException (String.Format ("Resource '{0}' must be available as a static resource", name));
			return o;
		}

		private FrameworkTemplate GetParentTemplate ()
		{
			IntPtr template = NativeMethods.xaml_get_template_parent (parser, target_data);

			if (template == IntPtr.Zero)
				return null;

			INativeEventObjectWrapper dob = NativeDependencyObjectHelper.FromIntPtr (template);

			return dob as FrameworkTemplate;
		}

		private void HandleProperty (Binding b, string prop, ref string remaining)
		{
			char next;
			object value = null;
			string str_value = null;

			if (remaining.StartsWith ("{")) {
				value = ParseExpression (ref remaining);

				remaining = remaining.TrimStart ();

				if (remaining.Length > 0 && remaining[0] == ',')
					remaining = remaining.Substring (1);

				if (value is string)
					str_value = (string) value;
			}
			else {
				str_value = GetNextPiece (ref remaining, out next, false);
			}

			switch (prop) {
			case "Mode":
				if (str_value == null)
					throw new XamlParseException (String.Format ("Invalid type '{0}' for Mode.", value == null ? "null" : value.GetType ().ToString ()));
				b.Mode = (BindingMode) Enum.Parse (typeof (BindingMode), str_value, true);
				break;
			case "Path":
				if (str_value == null)
					throw new XamlParseException (String.Format ("Invalid type '{0}' for Path.", value == null ? "null" : value.GetType ().ToString ()));
				b.Path = new PropertyPath (str_value);
				break;
			case "Source":
				// if the expression was: Source="{StaticResource xxx}" then 'value' will be populated
				// If the expression was  Source="5" then 'str_value' will be populated.
				b.Source = value ?? str_value;
				break;
			case "Converter":
				IValueConverter value_converter = value as IValueConverter;
				if (value_converter == null && value != null)
					throw new Exception ("A Binding Converter must be of type IValueConverter.");
				b.Converter = value_converter;
				break;
			case "ConverterParameter":
				if (value == null)
					b.ConverterParameter = str_value;
				else
					b.ConverterParameter = value;
				break;
			case "NotifyValidationOnError":
				bool bl;
				if (!Boolean.TryParse (str_value, out bl))
					throw new Exception (String.Format ("Invalid value {0} for NotifyValidationOnError.", str_value));
				b.NotifyOnValidationError = bl;
				break;
			case "ValidatesOnExceptions":
				if (!Boolean.TryParse (str_value, out bl))
					throw new Exception (String.Format ("Invalid value {0} for ValidatesOnExceptions.", str_value));
				b.ValidatesOnExceptions = bl;
				break;
			case "RelativeSource":
				RelativeSource rs = value as RelativeSource;
				if (rs == null)
					throw new Exception (String.Format ("Invalid value {0} for RelativeSource.", value));
				 b.RelativeSource = rs;
				break;
			case "ElementName":
				b.ElementName = str_value;
				break;
			default:
				Console.Error.WriteLine ("Unhandled Binding Property:  '{0}'  value:  {1}", prop, value != null ? value.ToString () : str_value);
				break;
			}
		}

		private static string GetNextPiece (ref string remaining, out char next, bool allow_spaces)
		{
			int end = 0;
			remaining = remaining.TrimStart ();
			if (remaining.Length > 1 && remaining [end] == '\'')
				end = remaining.IndexOf ("'", end + 1, StringComparison.Ordinal) + 1;
			
			if (end == -1 || end == 0) {
				end = 0;
				while (end < remaining.Length && (allow_spaces || !Char.IsWhiteSpace (remaining [end])) && remaining [end] != '}' && remaining [end] != ',' && remaining [end] != '=')
					end++;
			}

			if (end == 0) {
				next = Char.MaxValue;
				return null;
			}

			next = remaining [end];
			string res;
			if (remaining [0] == '\'' && remaining [end - 1] == '\'')
				res = remaining.Substring (1, end - 2);
			else
				res = remaining.Substring (0, end);
			remaining = remaining.Substring (end + 1);

			return res.TrimEnd ();
		}
	}
}


