using System;
using System.Reflection;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Windows.Media.Animation;
using System.Windows.Controls;
using System.Windows.Input;

class t
{
	static int Main (string [] args)
	{
		Type DO = typeof (System.Windows.DependencyObject);
		List<Type> types = new List<Type> (DO.Assembly.GetTypes ());

		for (int i = types.Count - 1; i >= 0; i--) {
			if (types [i] == DO)
				types.RemoveAt (i);
			else if (!types [i].IsSubclassOf (DO))
				types.RemoveAt (i);
			else if (types [i].IsNotPublic)
				types.RemoveAt (i);
		}

		if (args.Length >= 1 && args [0] == "generate-test-code") {
			Console.WriteLine (GenerateTestCode (types));
		} else {
			return DoChecks (types);
		}
		
		return 0;
	}

	static int DoChecks (List <Type> types)
	{
		int result = 0;
		int normal_ctor_size = 12;
		int normal_ctor_intptr_size = 8;

		foreach (Type tp in types) {
			//Console.WriteLine ("Checking: " + tp.FullName);
			MethodInfo mi;
			mi = tp.GetMethod ("GetKind", BindingFlags.NonPublic | BindingFlags.Instance);
			if (mi == null || mi.DeclaringType != tp) {
				Console.WriteLine ("Error: 1. The class '{0}' does not implement 'GetKind'.", tp.FullName);
				result = 1;
			} 

			ConstructorInfo ci = tp.GetConstructor (Type.EmptyTypes);

			if (ci == null) {
				if (tp.FullName.IndexOf ("Internal") < 0) {
					Console.WriteLine ("Error: 2a. The class '{0}' does not have an empty constructor.", tp.FullName);
					result = 1;
				}
			} else if (!ci.IsPublic) {
				Console.WriteLine ("Error: 2b. The class' '{0}' empty constructor is not public.", tp.FullName);
				result = 1;				
			} else if (result == 0) {
				MethodBody body = ci.GetMethodBody ();
				byte [] il = body.GetILAsByteArray ();
				if (il.Length != normal_ctor_size)
					Console.WriteLine ("Warning: 2c. {1,3} bytes is the size of the empty constructor of the class '{0}' (normal size = {2}).", tp.FullName, il.Length, normal_ctor_size);
			}
		}

		foreach (Type tp in types) {
			ConstructorInfo ci = tp.GetConstructor (BindingFlags.Instance | BindingFlags.NonPublic, null, new Type [] {typeof (IntPtr)}, null);
			
			if (ci == null) {	
				Console.WriteLine ("Error: 3a. The class '{0}' does not have an IntPtr constructor.", tp.FullName);
				result = 1;
			} else if (!ci.IsAssembly || ci.IsPublic || ci.IsFamily) {
				Console.WriteLine ("Error: 3b. The class' '{0}' IntPtr constructor is not internal.", tp.FullName);
				result = 1;				
			} else if (result == 0) {
				MethodBody body = ci.GetMethodBody ();
				byte [] il = body.GetILAsByteArray ();
				if (il.Length != normal_ctor_intptr_size)
					Console.WriteLine ("Warning: 3c. {1,3} bytes is the size of the IntPtr constructor of the class '{0}' (normal size = {2}).", tp.FullName, il.Length, normal_ctor_intptr_size);
			}
		}
		return result;
	}

	static string GenerateTestCode (List<Type> types)
	{
		StringBuilder result = new StringBuilder ();
		
		result.AppendLine (@"
using System;
using System.Reflection;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Windows.Media.Animation;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Ink;
");
		result.AppendLine ("class tester {");
		result.AppendLine ("static void Main () {");
		result.AppendLine ("string a,b;");
		foreach (Type tp in types) {
			if (tp.IsAbstract) {
				continue;
			}
		
			if (tp.GetConstructor (Type.EmptyTypes) == null) {
				result.AppendLine ("// Don't know how to create a " + tp.FullName);
				continue;
			}

			result.AppendLine ("try {");
			//result.AppendLine (string.Format ("    Console.WriteLine (\"Testing {0}\");", tp.FullName));
			result.AppendLine (string.Format ("    {1} var_{0} = new {1} ();", tp.Name, tp.FullName));
			foreach (PropertyInfo field in tp.GetProperties ()) {
				Type ftp = field.PropertyType;
				string var = string.Empty;
				string nullvar = string.Empty;
				string vartp = ftp.FullName;
				
				if ((field.CanRead && field.GetGetMethod (true).IsStatic) || (field.CanWrite && field.GetSetMethod (true).IsStatic))
					continue;

				if (ftp == typeof(int)) {
					var = "1";
				} else if (ftp == typeof(double)) {
					var = "1.1";
				} else if (ftp == typeof (string)) {
					var = "\"abc\"";
				} else if (ftp == typeof (bool)) {
					var = "true";
				} else if (ftp == typeof (object)) {
					var = "null";
				} else if (ftp == typeof (Uri)) {
					var = "new Uri (\"http://www.mono-project.com\");";
				} else if (ftp == typeof (Point)) {
					var = "new Point (2, 3)";
				} else if (ftp == typeof (Transform)) {
					var = "new TranslateTransform ()";
				} else if (ftp.IsSubclassOf (typeof (DependencyObject)) && !ftp.IsAbstract) {
					var = "new " + ftp.Name + "()";
				} else if (ftp.IsEnum) {
					var = "(" + ftp.Name + ") 1";
				} else if (ftp == typeof (double[])) {
					var = "new double [] {1, 2, 3}";
				} else if (ftp == typeof (Point [])) {
					var = "new Point [] {new Point (1, 2), new Point (3, 4)}";
				} else if (ftp == typeof (Color)) {
					var = "Color.FromArgb (0, 25, 25, 25)";
				} else if (ftp == typeof (Geometry)) {
					var = "new PathGeometry  ()";
				} else if (ftp == typeof (Rect)) {
					var = "new Rect  (1, 2, 3, 4)";
				} else if (ftp == typeof (Matrix)) {
					var = "new Matrix  (1, 2, 3, 4, 5, 6)";
				} else if (ftp == typeof (Timeline)) {
					var = "new Storyboard  ()";
				} else if (ftp == typeof (Duration)) {
					var = "new Duration (new TimeSpan ())";
				} else if (ftp == typeof (RepeatBehavior)) {
					var = "new RepeatBehavior (12.34)";
				} else if (ftp == typeof (Nullable<TimeSpan>)) {
					nullvar = "null";
					var = "new Nullable<TimeSpan> (new TimeSpan ())";
					vartp = "Nullable<TimeSpan>";
				} else if (ftp == typeof (Nullable<double>)) {
					nullvar = "null";
					var = "new Nullable<double> (1.2)";
					vartp = "Nullable<double>";	
				} else if (ftp == typeof (Nullable<Color>)) {
					nullvar = "null";
					var = "new Nullable<Color> (Color.FromArgb (0, 25, 50, 75))";
					vartp = "Nullable<Color>";	
				} else if (ftp == typeof (Nullable<Point>)) {
					nullvar = "null";
					var = "new Nullable<Point> (new Point (25, 75))";
					vartp = "Nullable<Point>";
				} else if (ftp == typeof (Brush)) {
					var = "new SolidColorBrush (Color.FromArgb (0, 25, 50, 75))";
				} else if (ftp == typeof (KeyTime)) {
					var = "KeyTime.Uniform";
				} else if (ftp == typeof (TimeSpan)) {
					var = "new TimeSpan (123456)";
				} else {
					Console.WriteLine ("//Don't know how to test a '" + ftp.FullName + "'.");
				}
				
				if (var == string.Empty)
					continue;
				
				if (field.Name == "Item")
					continue;
				
				if (!field.CanWrite && !field.CanRead) {
					Console.WriteLine ("Can't read nor write to: " + field.Name);
					continue;
				}
				
				if (field.CanWrite || field.CanRead) {
					//result.AppendLine (string.Format ("    Console.WriteLine (\"Testing {0}.{1} <{2}>\");", tp.FullName, field.Name, ftp.FullName));
				}
				
				if (field.CanWrite)
					result.AppendLine (string.Format ("    {2} field_{0}_{1}_a;", tp.Name, field.Name, vartp));
	
				if (field.CanRead) {
					result.AppendLine (string.Format ("    {2} field_{0}_{1}_b;", tp.Name, field.Name, vartp));
				}
				
				foreach (string code in new string [] {var, nullvar}) {
					if (code == string.Empty || code == null)
					    continue;
					bool cw = field.CanWrite && field.GetSetMethod (true).IsPublic; //!field.GetSetMethod (true).IsPrivate && !field.GetSetMethod (true).IsAssembly && !field.;
					bool cr = field.CanRead && field.GetGetMethod (true).IsPublic; //!field.GetGetMethod (true).IsPrivate && !field.GetGetMethod (true).IsAssembly;

					if (cw) {
						result.AppendLine (string.Format ("    field_{0}_{1}_a = {3};", tp.Name, field.Name, vartp, code));
						result.AppendLine (string.Format ("    var_{0}.{1} = field_{0}_{1}_a;", tp.Name, field.Name));
					}
					if (cr) {
						result.AppendLine (string.Format ("    field_{0}_{1}_b = var_{0}.{1};", tp.Name, field.Name, vartp));
					}
					if (cr && cw) {
						if (ftp.IsClass) {
							result.AppendLine (string.Format ("    a = field_{0}_{1}_a == null ? \"<null>\" : field_{0}_{1}_a.ToString ();", tp.Name, field.Name));
							result.AppendLine (string.Format ("    b = field_{0}_{1}_b == null ? \"<null>\" : field_{0}_{1}_b.ToString ();", tp.Name, field.Name));
						} else {
							result.AppendLine (string.Format ("    a = field_{0}_{1}_a.ToString ();", tp.Name, field.Name));
							result.AppendLine (string.Format ("    b = field_{0}_{1}_b.ToString ();", tp.Name, field.Name));
						}
						result.AppendLine (string.Format ("    if (string.CompareOrdinal (a, b) != 0) {{"));
						result.AppendLine (string.Format ("        Console.WriteLine (\"While testing {0}.{1} got:\");", tp.FullName, field.Name));
						result.AppendLine (string.Format ("        Console.WriteLine (\"    Wrote '{{0}}' and got '{{1}}'\", a, b);", tp.Name, field.Name));
						result.AppendLine ("    }");
					}
				}
			}
			result.AppendLine ("} catch (TypeInitializationException ex) {");
			result.AppendLine (string.Format ("        Console.WriteLine (\"While testing {0} got:\");", tp.FullName));
			result.AppendLine ("    Console.WriteLine (\"    \" + ex.InnerException.Message);");
			result.AppendLine ("} catch (EntryPointNotFoundException ex) {");
			result.AppendLine (string.Format ("        Console.WriteLine (\"While testing {0} got EntryPointNotFoundException:\");", tp.FullName));
			result.AppendLine ("    Console.WriteLine (\"    \" + ex.Message);");
			result.AppendLine ("} catch (Exception ex) {");
			result.AppendLine (string.Format ("        Console.WriteLine (\"While testing {0} got:\");", tp.FullName));
			result.AppendLine ("    Console.WriteLine (\"    \" + ex.Message);");
			result.AppendLine ("    Console.WriteLine (ex.StackTrace);");
			result.AppendLine ("}");
			result.AppendLine ("");
		}
		result.AppendLine ("}");
		result.AppendLine ("}");
		return result.ToString ();
	}
}
