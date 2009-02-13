using System;
using System.Text;
using System.Reflection;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;

namespace DefaultValues {
	public partial class Page : Canvas {
		StringBuilder sb;
		
		public Page ()
		{
			InitializeComponent ();
			
			sb = new StringBuilder ();
			GenerateUnitTests ();
			txtOutput.Text = sb.ToString ().Replace ("\t", "    ");
		}
		
		void GenerateUnitTests ()
		{
			Assembly assembly = typeof (DependencyObject).Assembly;
			Type[] types = assembly.GetTypes ();
			
			sb.AppendLine ("using System;");
			sb.AppendLine ("using System.Windows;");
			sb.AppendLine ("using System.Windows.Automation.Peers;");
			sb.AppendLine ("using System.Windows.Browser;");
			sb.AppendLine ("using System.Windows.Controls;");
			sb.AppendLine ("using System.Windows.Controls.Primitives;");
			sb.AppendLine ("using System.Windows.Data;");
			sb.AppendLine ("using System.Windows.Documents;");
			sb.AppendLine ("using System.Windows.Ink;");
			sb.AppendLine ("using System.Windows.Input;");
			sb.AppendLine ("using System.Windows.Interop;");
			sb.AppendLine ("using System.Windows.Markup;");
			sb.AppendLine ("using System.Windows.Media;");
			sb.AppendLine ("using System.Windows.Media.Animation;");
			sb.AppendLine ("using System.Windows.Media.Imaging;");
			sb.AppendLine ("using System.Windows.Resources;");
			sb.AppendLine ("using System.Windows.Shapes;");
			sb.AppendLine ();
			sb.AppendLine ("using Microsoft.VisualStudio.TestTools.UnitTesting;");
			sb.AppendLine ("using Mono.Moonlight.UnitTesting;");
			sb.AppendLine ("using System.Collections.Generic;");
			sb.AppendLine ("using System.Collections.ObjectModel;");
			sb.AppendLine ();
			sb.AppendLine ("namespace MoonTest.System.Windows.Controls");
			sb.AppendLine ("{");
			sb.AppendLine ("\t[TestClass]");
			sb.AppendLine ("\tpublic class DefaultValueTests");
			sb.AppendLine ("\t{");
			
			for (int i = 0; i < types.Length; i++) {
				if (types[i].IsPublic && !types[i].IsAbstract && types[i].IsSubclassOf (typeof (DependencyObject)))
					GenerateUnitTest (types[i]);
			}
			
			sb.AppendLine ("\t}");
			sb.AppendLine ("}");
		}
		
		void GenerateUnitTest (Type type)
		{
			DependencyObject widget;
			
			try {
				if ((widget = Activator.CreateInstance (type) as DependencyObject) != null) {
					GenerateReadLocalValueTest (widget, type);
					GenerateGetValueTest (widget, type);
					//GeneratePropertyTest (type);
					//GenerateSetValueTest (type);
				}
			} catch {
			}
		}
		
		void EmitTestMethod (Type type, string test)
		{
			sb.AppendLine ("\t\t[TestMethod]");
			sb.AppendLine ("\t\tpublic void " + type.Name + "_" + test + " ()");
			sb.AppendLine ("\t\t{");
			sb.AppendLine ("\t\t\t" + type.Name + " widget = new " + type.Name + " ();");
			sb.AppendLine ("\t\t\tobject retval;");
			sb.AppendLine ();
		}
		
		string PrettyName (Type type)
		{
			if (type == typeof (bool))
				return "bool";
			
			if (type == typeof (char))
				return "char";
			
			if (type == typeof (double))
				return "double";
			
			if (type == typeof (int))
				return "int";
			
			if (type == typeof (string))
				return "string";
			
			if (type == typeof (object))
				return "object";
			
			return type.Name;
		}
		
		void AssertDependencyObjectEqual (DependencyObject widget, Type type, string test, string fieldName, object retval)
		{
		}
		
		void AssertValuesEqual (DependencyObject widget, Type type, string test, string fieldName, object retval)
		{
			string expected = null;
			string special = null;
			bool tostring = false;
			
			if (retval.GetType ().IsSubclassOf (typeof (DependencyObject))) {
				sb.AppendLine ("\t\t\tAssert.IsTrue (retval is " + PrettyName (retval.GetType ()) + ", \"" + test + "(" +
					       fieldName + ") is not of the correct type\");");
				AssertDependencyObjectEqual (widget, type, test, fieldName, retval);
				return;
			} else if (retval.GetType ().IsGenericType) {
				return;
			}
			
			sb.AppendLine ("\t\t\tAssert.IsTrue (retval is " + PrettyName (retval.GetType ()) + ", \"" + test + "(" +
				       fieldName + ") is not of the correct type\");");
			
			if (retval.GetType ().IsPrimitive) {
				if (retval is double) {
					// some special cases...
					double d = (double) retval;
					
					if (Double.IsNegativeInfinity (d))
						special = "Double.IsNegativeInfinity ((double) retval)";
					else if (Double.IsPositiveInfinity (d))
						special = "Double.IsPositiveInfinity ((double) retval)";
					else if (Double.IsNaN (d))
						special = "Double.IsNaN ((double) retval)";
					else if ((double) retval == -3.40282346638529E+38)
						expected = "(double) 0";
					else
						expected = "(double) " + retval.ToString ();
				} else if (retval is bool) {
					// prettyification
					expected = (bool) retval ? "true" : "false";
				} else if (retval is char) {
					expected = "\"" + retval.ToString () + "\"";
					tostring = true;
				} else {
					expected = retval.ToString ();
				}
			} else if (retval.GetType ().IsEnum) {
				// serialize the enum value
				expected = retval.GetType ().Name + "." + retval.ToString ();
			} else {
				// everything else serialize to a string for comparison
				expected = "\"" + retval.ToString () + "\"";
				tostring = !(retval is string);
			}
			
			if (special != null)
				sb.AppendLine ("\t\t\tAssert.IsTrue (" + special + ", \"" + test + "(" + fieldName +
					       ") does not match the default value\");");
			else
				sb.AppendLine ("\t\t\tAssert.AreEqual (" + expected + ", retval" + (tostring ? ".ToString ()" : "") +
					       ", \"" + test + "(" + fieldName + ") does not match the default value\");");
		}
		
		void GenerateReadLocalValueTest (DependencyObject widget, Type type)
		{
			FieldInfo[] fields = type.GetFields ();
			DependencyProperty property;
			bool testing = false;
			object retval;
			
			for (int i = 0; i < fields.Length; i++) {
				if (!fields[i].IsPublic || !fields[i].IsStatic || fields[i].FieldType != typeof (DependencyProperty))
					continue;
				
				try {
					if ((property = fields[i].GetValue (null) as DependencyProperty) == null)
						continue;
				} catch {
					continue;
				}
				
				if (!testing) {
					EmitTestMethod (type, "ReadLocalValue");
					testing = true;
				} else {
					sb.AppendLine ();
				}
				
				try {
					retval = widget.ReadLocalValue (property);
					
					sb.AppendLine ("\t\t\tretval = widget.ReadLocalValue (" + type.Name + "." + fields[i].Name + ");");
					
					if (retval != DependencyProperty.UnsetValue) {
						if (retval != null) {
							sb.AppendLine ("\t\t\tAssert.IsNotNull (retval, \"ReadLocalValue(" + fields[i].Name + ") should not have returned null\");");
							AssertValuesEqual (widget, type, "ReadLocalValue", fields[i].Name, retval);
						} else {
							sb.AppendLine ("\t\t\tAssert.IsNull (retval, \"ReadLocalValue(" + fields[i].Name + ") should have returned null\");");
						}
					} else {
						sb.AppendLine ("\t\t\tAssert.AreEqual (DependencyProperty.UnsetValue, retval, \"ReadLocalValue(" +
							       fields[i].Name + ") should not have a value by default\");");
					}
				} catch (Exception ex) {
					sb.AppendLine ("\t\t\tAssert.Throws<" + ex.GetType ().Name + ">(delegate {");
					sb.AppendLine ("\t\t\t\tretval = widget.ReadLocalValue (" + type.Name + "." + fields[i].Name + ");");
					sb.AppendLine ("\t\t\t}, \"ReadLocalValue(" + fields[i].Name + ") should thow an exception\");");
				}
			}
			
			if (testing) {
				sb.AppendLine ("\t\t}");
				sb.AppendLine ();
			}
		}
		
		void GenerateGetValueTest (DependencyObject widget, Type type)
		{
			FieldInfo[] fields = type.GetFields ();
			DependencyProperty property;
			bool testing = false;
			object retval;
			
			for (int i = 0; i < fields.Length; i++) {
				if (!fields[i].IsPublic || !fields[i].IsStatic || fields[i].FieldType != typeof (DependencyProperty))
					continue;
				
				try {
					if ((property = fields[i].GetValue (null) as DependencyProperty) == null)
						continue;
				} catch {
					continue;
				}
				
				if (!testing) {
					EmitTestMethod (type, "GetValue");
					testing = true;
				} else {
					sb.AppendLine ();
				}
				
				try {
					retval = widget.GetValue (property);
					
					sb.AppendLine ("\t\t\tretval = widget.GetValue (" + type.Name + "." + fields[i].Name + ");");
					
					if (retval != null) {
						sb.AppendLine ("\t\t\tAssert.IsNotNull (retval, \"GetValue(" + fields[i].Name + ") should not have returned null\");");
						AssertValuesEqual (widget, type, "GetValue", fields[i].Name, retval);
					} else {
						sb.AppendLine ("\t\t\tAssert.IsNull (retval, \"GetValue(" + fields[i].Name + ") should have returned null\");");
					}
				} catch (Exception ex) {
					sb.AppendLine ("\t\t\tAssert.Throws<" + ex.GetType ().Name + ">(delegate {");
					sb.AppendLine ("\t\t\t\tretval = widget.GetValue (" + type.Name + "." + fields[i].Name + ");");
					sb.AppendLine ("\t\t\t}, \"GetValue(" + fields[i].Name + ") should thow an exception\");");
				}
			}
			
			if (testing) {
				sb.AppendLine ("\t\t}");
				sb.AppendLine ();
			}
		}
	}
}
