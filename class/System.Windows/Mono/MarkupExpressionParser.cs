

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
		private DependencyObject target;
		private string attribute_name;
		private IntPtr parser;
		private IntPtr target_data;

		public MarkupExpressionParser (DependencyObject target, string attribute_name, IntPtr parser, IntPtr target_data)
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
			string orig = expression;

			object result = null;
			if (TryHandler ("^{\\s*Binding\\s*", ParseBinding, ref expression, out result))
				;
			else if (TryHandler ("^{\\s*StaticResource\\s*", ParseStaticResource, ref expression, out result))
				;
			else if (TryHandler ("^{\\s*TemplateBinding\\s*", ParseTemplateBinding, ref expression, out result))
				;

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
			string piece = GetNextPiece (ref remaining, out next);
			

			if (next == '=')
				HandleProperty (binding, piece, ref remaining);
			else
				binding.Path = new PropertyPath (piece);

			do {
				piece = GetNextPiece (ref remaining, out next);

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
			string name = GetNextPiece (ref expression, out next);

			object o = LookupNamedResource (target, name);

#if !__TESTING
			if (o == null)
				o = Application.Current.Resources [name];
#endif

			return o;
		}

		public object ParseTemplateBinding (ref string expression)
		{
			char next;
			string prop = GetNextPiece (ref expression, out next);
			FrameworkTemplate template = GetParentTemplate ();

			template.AddXamlBinding (target, attribute_name, prop);
			return null;
		}

		private object LookupNamedResource (DependencyObject dob, string name)
		{
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

			INativeDependencyObjectWrapper dob = NativeDependencyObjectHelper.FromIntPtr (template);

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
				str_value = GetNextPiece (ref remaining, out next);
			}

			switch (prop) {
			case "Mode":
				if (str_value == null)
					throw new XamlParseException (String.Format ("Invalid type '{0}' for Mode.", value == null ? "null" : value.GetType ().ToString ()));
				b.Mode = (BindingMode) Enum.Parse (typeof (BindingMode), str_value);
				break;
			case "Path":
				if (str_value == null)
					throw new XamlParseException (String.Format ("Invalid type '{0}' for Path.", value == null ? "null" : value.GetType ().ToString ()));
				b.Path = new PropertyPath (str_value);
				break;
			case "Source":
				b.Source = value;
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
			default:
				Console.Error.WriteLine ("Unhandled Binding Property:  '{0}'  value:  {1}", prop, value != null ? value.ToString () : str_value);
				break;
			}
		}

		private static string GetNextPiece (ref string remaining, out char next)
		{
			int end = 0;
			remaining = remaining.TrimStart ();
			if (remaining.Length > 1 && remaining [end] == '\'')
				end = remaining.IndexOf ("'", end + 1, StringComparison.Ordinal) + 1;
			
			if (end == -1 || end == 0) {
				end = 0;
				while (end < remaining.Length && !Char.IsWhiteSpace (remaining [end]) && remaining [end] != '}' && remaining [end] != ',' && remaining [end] != '=')
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

			return res;
		}
	}
}


