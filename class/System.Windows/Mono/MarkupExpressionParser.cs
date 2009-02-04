

using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Data;
using System.Collections.Generic;


namespace Mono.Xaml {

	internal class MarkupExpressionParser {

		private DependencyObject target;
		private string attribute_name;
		private IntPtr xaml_context;

		public MarkupExpressionParser (DependencyObject target, string attribute_name, IntPtr xaml_context)
		{
			this.target = target;
			this.attribute_name = attribute_name;
			this.xaml_context = xaml_context;
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
			return expression.StartsWith ("{TemplateBinding");
		}

		public static bool IsStaticResource (string expression)
		{
			return expression.StartsWith ("{StaticResource");
		}

		public static bool IsBinding (string expression)
		{
			return expression.StartsWith ("{Binding");
		}

		public object ParseExpression (ref string expression)
		{
			if (expression.StartsWith ("{Binding")) {
				int len = "{Binding".Length;
				expression = expression.Substring (len, expression.Length - len);
				expression = expression.TrimStart ();

				return ParseBinding (ref expression);
			} else if (expression.StartsWith ("{StaticResource")) {
				int len = "{StaticResource".Length;
				expression = expression.Substring (len, expression.Length - len);
				expression = expression.TrimStart ();

				return ParseStaticResource (ref expression);
			} else if (expression.StartsWith ("{TemplateBinding")) {
				int len = "{StaticResource".Length;
				expression = expression.Substring (len, expression.Length - len);
				expression = expression.TrimStart ();

				ParseTemplateBinding (ref expression);
			}

			return null;
		}

		public Binding ParseBinding (ref string expression)
		{
			Binding binding = new Binding ();
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

		public void ParseTemplateBinding (ref string expression)
		{
			char next;
			string prop = GetNextPiece (ref expression, out next);
			FrameworkTemplate template = GetParentTemplate (target);

			template.AddXamlBinding (target, attribute_name, prop);
		}

		private object LookupNamedResource (DependencyObject dob, string name)
		{
			FrameworkElement fe = dob as FrameworkElement;

			if (fe != null) {
				object value = fe.Resources [name];
				if (value != null)
					return value;
				if (fe.Parent != null)
					return LookupNamedResource (fe.Parent, name);
			}

#if __TESTING
			/// XXXX: This is just for testing, we need to return null here normally
			return new object ();
#else
			return null;
#endif		
		}

		private FrameworkTemplate GetParentTemplate (DependencyObject item)
		{
			FrameworkElement fe = item as FrameworkElement;

			if (fe == null) {
				Console.Error.WriteLine ("Unable to lookup Template parents on non FrameworkElement types ({0})", item.GetType ());
				return null;
			}

			if (fe.Parent == null)
				return null;

			return GetTemplate (fe.Parent);
		}

		private FrameworkTemplate GetTemplate (DependencyObject item)
		{
			FrameworkTemplate template = item as FrameworkTemplate;
			if (template != null)
				return template;

			FrameworkElement fe = item as FrameworkElement;

			if (fe == null) {
				Console.Error.WriteLine ("Unable to lookup Template parents on non FrameworkElement types ({0})", item.GetType ());
				return null;
			}

			if (fe.Parent == null)
				return null;

			return GetTemplate (fe.Parent);
		}

		private void HandleProperty (Binding b, string prop, ref string remaining)
		{
			char next;
			object value = null;
			string str_value = null;

			if (remaining.StartsWith ("{"))
				value = ParseExpression (ref remaining);
			else
				str_value = GetNextPiece (ref remaining, out next);

			switch (prop) {
			case "Mode":
				b.Mode = (BindingMode) Enum.Parse (typeof (BindingMode), str_value);
				break;
			case "Path":
				b.Path = new PropertyPath (str_value);
				break;
			case "Source":
				if (value == null)
					throw new Exception ("A Binding Source must be available as StaticResouce.");
				b.Source = value;
				break;
			case "Converter":
				if (value == null)
					throw new Exception ("A Binding Converter must be available as StaticResouce.");
				IValueConverter value_converter = value as IValueConverter;
				if (value_converter == null)
					throw new Exception ("A Binding Converter must be of type IValueConverter.");
				b.Converter = value_converter;
				break;
			case "ConverterParameter":
				if (value == null)
					throw new Exception ("A Binding ConverterParameter must be available as StaticResouce.");
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

			while (end < remaining.Length && !Char.IsWhiteSpace (remaining [end]) && remaining [end] != '}' && remaining [end] != ',' && remaining [end] != '=')
				end++;

			if (end == 0) {
				next = Char.MaxValue;
				return null;
			}

			next = remaining [end];
			string res = remaining.Substring (0, end);
			remaining = remaining.Substring (end + 1);

			return res;
		}
	}
}


