using System;
using System.Text;
using System.Reflection;
using System.Windows;
using System.Windows.Media;
using System.Windows.Markup;
using System.Windows.Controls;
using System.Collections.Generic;

namespace DefaultValues {
	public partial class Page : Canvas {
		StringBuilder sb;
		
		public Page ()
		{
			InitializeComponent ();
			
			sb = new StringBuilder ();
			GenerateUnitTests ();
			txtOutput.Text = sb.ToString ();
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
			sb.AppendLine ("using System.Windows.Threading;");
			sb.AppendLine ();
			sb.AppendLine ("using Microsoft.VisualStudio.TestTools.UnitTesting;");
			sb.AppendLine ("using Mono.Moonlight.UnitTesting;");
			sb.AppendLine ("using System.Collections.Generic;");
			sb.AppendLine ("using System.Collections.ObjectModel;");
			sb.AppendLine ();
			
			for (int i = 0; i < types.Length; i++) {
				if (types[i].IsPublic && !types[i].IsAbstract && types[i].IsSubclassOf (typeof (DependencyObject)))
					GenerateUnitTests (types[i]);
			}
		}
		
		void GenerateUnitTests (Type type)
		{
			DependencyObject widget;
			
			try {
				if ((widget = Activator.CreateInstance (type) as DependencyObject) != null) {
					sb.AppendLine ("namespace MoonTest." + type.Namespace);
					sb.AppendLine ("{");
					sb.AppendLine ("    public partial class " + type.Name + "Test");
					sb.AppendLine ("    {");
					GenerateReadLocalValueTests (widget, type);
					GenerateGetValueTests (widget, type);
					GeneratePropertyGetterTests (widget, type);
					GenerateSetStringValueTests (widget, type);
					//GenerateSetValueTests (widget, type);
					sb.AppendLine ("    }");
					sb.AppendLine ("}");
				}
			} catch {
			}
		}
		
		void EmitTestMethod (Type type, string test, bool retval)
		{
			sb.AppendLine ("        [TestMethod]");
			sb.AppendLine ("        public void Test_" + test + " ()");
			sb.AppendLine ("        {");
			sb.AppendLine ("            " + type.Name + " widget = new " + type.Name + " ();");
			if (retval)
				sb.AppendLine ("            object retval;");
			sb.AppendLine ();
		}
		
		bool IgnoreType (Type type)
		{
			if (type == typeof (System.Windows.Threading.Dispatcher))
				return true;
			
			return false;
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

			if (type.IsGenericType) {
				// this is just for PresentationFrameworkCollection`1
				return type.Name.Replace ("`1", "");
			}

			return type.Name;
		}
		
		void AssertDependencyObjectEqual (DependencyObject widget, Type type, string method, string retvalName, object retval)
		{
		}
		
		void AssertValuesEqual (DependencyObject widget, Type type, string method, string retvalName, object retval, bool check_is_type)
		{
			string expected = null;
			string special = null;
			bool tostring = false;
			
			if (retval.GetType ().IsSubclassOf (typeof (DependencyObject))) {
				sb.AppendLine (String.Format ("            Assert.IsTrue({0} is {1}, \"{2} is not of the correct type\");",
							      retvalName, PrettyName (retval.GetType ()), method));
				AssertDependencyObjectEqual (widget, type, method, retvalName, retval);
				return;
			} else if (retval.GetType ().IsGenericType) {
				return;
			}
			
			if (check_is_type)
				sb.AppendLine (String.Format ("            Assert.IsTrue({0} is {1}, \"{2} is not of the correct type\");",
							      retvalName, PrettyName (retval.GetType ()), method));
			
			if (retval.GetType ().IsPrimitive) {
				if (retval is double) {
					// some special cases...
					double d = (double) retval;
					
					if (Double.IsNegativeInfinity (d))
						special = "Double.IsNegativeInfinity((double) " + retvalName + ")";
					else if (Double.IsPositiveInfinity (d))
						special = "Double.IsPositiveInfinity((double) " + retvalName + ")";
					else if (Double.IsNaN (d))
						special = "Double.IsNaN((double) " + retvalName + ")";
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
			} else if (retval is XmlLanguage) {
				// for XmlLanguages, compare the IetfLanguageTags
				expected = "\"" + ((XmlLanguage) retval).IetfLanguageTag + "\"";
				if (retvalName == "retval")
					retvalName = "((XmlLanguage) " + retvalName + ").IetfLanguageTag";
				else
					retvalName += ".IetfLanguageTag";
			} else {
				// everything else serialize to a string for comparison
				expected = "\"" + retval.ToString () + "\"";
				tostring = !(retval is string);
			}
			
			if (special != null)
				sb.AppendLine (String.Format ("            Assert.IsTrue({0}, \"{1} does not match the default value\");",
							      special, method));
			else
				sb.AppendLine (String.Format ("            Assert.AreEqual({0}, {1}{2}, \"{3} does not match the default value\");",
							      expected, retvalName, tostring ? ".ToString ()" : "", method));
		}
		
		void GenerateReadLocalValueTests (DependencyObject widget, Type type)
		{
			DependencyProperty property;
			FieldInfo[] fields;
			Type ctype = type;
			string method;
			object retval;
			
			while (ctype.IsSubclassOf (typeof (DependencyObject)) && !ctype.IsGenericType) {
				fields = ctype.GetFields ();

				bool emit_test = false;

				for (int i = 0; i < fields.Length; i++) {
					if (!fields[i].IsPublic || !fields[i].IsStatic || fields[i].FieldType != typeof (DependencyProperty))
						continue;
					
					try {
						property = fields[i].GetValue (null) as DependencyProperty;
					} catch {
						property = null;
					}
					
					if (property == null)
						continue;

					emit_test = true;
					break;
				}

				if (emit_test) {
					EmitTestMethod (type, "ReadLocalValue_" + PrettyName(ctype), true);

					for (int i = 0; i < fields.Length; i++) {
						if (!fields[i].IsPublic || !fields[i].IsStatic || fields[i].FieldType != typeof (DependencyProperty))
							continue;

						try {
							property = fields[i].GetValue (null) as DependencyProperty;
						} catch {
							property = null;
						}
					
						if (property == null)
							continue;
					
						method = "ReadLocalValue(" + ctype.Name + "." + fields[i].Name + ")";
					
						try {
							retval = widget.ReadLocalValue (property);
						
							sb.AppendLine ("            retval = widget." + method + ";");
						
							if (retval != DependencyProperty.UnsetValue) {
								if (retval != null) {
									sb.AppendLine ("            Assert.IsNotNull(retval, \"" + method + " should not have returned null\");");
									AssertValuesEqual (widget, type, method, "retval", retval, true);
								} else {
									sb.AppendLine ("            Assert.IsNull(retval, \"" + method + " should have returned null\");");
								}
							} else {
								sb.AppendLine ("            Assert.AreEqual(DependencyProperty.UnsetValue, retval, \"" + method + 
									       " should not have a value by default\");");
							}
						} catch (Exception ex) {
							sb.AppendLine ("            // [MoonlightBug] - Moonlight needs to be fixed to throw on some ReadLocalValue invocations");
							sb.AppendLine ("            //Assert.Throws<" + ex.GetType ().Name + ">(delegate {");
							sb.AppendLine ("            //    retval = widget." + method + ";");
							sb.AppendLine ("            //}, \"" + method + " should thow an exception\");");
						}
					}

					sb.AppendLine ("        }");
					sb.AppendLine ();
				}
				
				ctype = ctype.BaseType;
			}
		}
		
		void GenerateGetValueTests (DependencyObject widget, Type type)
		{
			DependencyProperty property;
			FieldInfo[] fields;
			Type ctype = type;
			string method;
			object retval;
			
			while (ctype.IsSubclassOf (typeof (DependencyObject)) && !ctype.IsGenericType) {
				fields = ctype.GetFields ();

				bool emit_test = false;

				for (int i = 0; i < fields.Length; i++) {
					if (!fields[i].IsPublic || !fields[i].IsStatic || fields[i].FieldType != typeof (DependencyProperty))
						continue;
					
					try {
						property = fields[i].GetValue (null) as DependencyProperty;
					} catch {
						property = null;
					}
					
					if (property == null)
						continue;

					emit_test = true;
					break;
				}

				if (emit_test) {
					EmitTestMethod (type, "GetValue_" + PrettyName (ctype), true);

					for (int i = 0; i < fields.Length; i++) {
						if (!fields[i].IsPublic || !fields[i].IsStatic || fields[i].FieldType != typeof (DependencyProperty))
							continue;
					
						try {
							property = fields[i].GetValue (null) as DependencyProperty;
						} catch {
							property = null;
						}
					
						if (property == null)
							continue;
					
						method = "GetValue(" + ctype.Name + "." + fields[i].Name + ")";
					
						try {
							retval = widget.GetValue (property);
						
							sb.AppendLine ("            retval = widget." + method + ";");
						
							if (retval != null) {
								sb.AppendLine ("            Assert.IsNotNull(retval, \"" + method + " should not have returned null\");");
								AssertValuesEqual (widget, type, method, "retval", retval, true);
							} else {
								sb.AppendLine ("            Assert.IsNull(retval, \"" + method + " should have returned null\");");
							}
						} catch (Exception ex) {
							sb.AppendLine ("            Assert.Throws<" + ex.GetType ().Name + ">(delegate {");
							sb.AppendLine ("                retval = widget." + method + ";");
							sb.AppendLine ("            }, \"" + method + " should thow an exception\");");
						}
					}

					sb.AppendLine ("        }");
					sb.AppendLine ();
				}
				
				ctype = ctype.BaseType;
			}
		}
		
		void GeneratePropertyGetterTests (DependencyObject widget, Type type)
		{
			Dictionary<string,bool> tested = new Dictionary<string,bool> ();
			PropertyInfo[] properties;
			MethodInfo getter;
			Type ctype = type;
			string method;
			object retval;
			
			do {
				properties = ctype.GetProperties ();

				bool emit_test = false;

				for (int i = 0; i < properties.Length; i++) {
					if (tested.ContainsKey (properties[i].Name))
						continue;

					try {
						getter = properties[i].GetGetMethod ();
					} catch {
						getter = null;
					}
					
					if (getter == null || getter.IsStatic || !getter.IsPublic || getter.GetParameters().Length > 0)
						continue;
					
					if (IgnoreType (getter.ReturnType))
						continue;

					emit_test = true;
					break;
				}

				if (emit_test) {
					EmitTestMethod (type, "PropertyGetter_" + PrettyName (ctype), false);

					for (int i = 0; i < properties.Length; i++) {
						if (tested.ContainsKey (properties[i].Name))
							continue;
					
						try {
							getter = properties[i].GetGetMethod ();
						} catch {
							getter = null;
						}
					
						if (getter == null || getter.IsStatic || !getter.IsPublic || getter.GetParameters().Length > 0)
							continue;
					
						if (IgnoreType (getter.ReturnType))
							continue;
					
						try {
							retval = getter.Invoke (widget, null);
							tested.Add (properties[i].Name, true);
							method = properties[i].Name;
						
							if (retval != null) {
								if (!retval.GetType ().IsValueType)
									sb.AppendLine ("            Assert.IsNotNull(widget." + method + ", \"" + method + " should not have returned null\");");
								AssertValuesEqual (widget, type, method, "widget." + method, retval, false);
							} else {
								sb.AppendLine ("            Assert.IsNull(widget." + method + ", \"" +
									       method + " should have returned null\");");
							}
						} catch {
							sb.AppendLine ("            // exception generating test for " + properties[i].Name);
						}
					}

					sb.AppendLine ("        }");
					sb.AppendLine ();
				}
				
				ctype = ctype.BaseType;
			} while (ctype.IsSubclassOf (typeof (DependencyObject)));
		}
		
		DependencyProperty GetDependencyPropertyByName (DependencyObject widget, Type type, string name)
		{
			FieldInfo[] fields = type.GetFields ();
			DependencyProperty prop;
			FieldInfo field = null;
			
			for (int i = 0; i < fields.Length; i++) {
				if (fields[i].Name == name) {
					field = fields[i];
					break;
				}
			}
			
			if (field == null || !field.IsPublic || !field.IsStatic || field.FieldType != typeof (DependencyProperty))
				return null;
			
			try {
				prop = field.GetValue (null) as DependencyProperty;
			} catch {
				prop = null;
			}
			
			return prop;
		}
		
		void GenerateSetStringValueTests (DependencyObject widget, Type type)
		{
			DependencyProperty property;
			PropertyInfo[] properties;
			bool testing = false;
			Type ctype = type;
			string prop_name;
			string actual;
			string method;
			object retval;
			
			while (ctype.IsSubclassOf (typeof (DependencyObject)) && !ctype.IsGenericType) {
				properties = ctype.GetProperties ();

				bool emit_test = false;

				for (int i = 0; i < properties.Length; i++) {
					if (properties[i].PropertyType != typeof (string))
						continue;
					
					prop_name = properties[i].Name + "Property";
					property = GetDependencyPropertyByName (widget, ctype, prop_name);
					if (property == null)
						continue;

					emit_test = true;
					break;
				}

				if (emit_test) {
					EmitTestMethod (type, "SetStringValue_" + PrettyName (ctype), true);

					// Note: we iterate over the getter/setter properties because the DependencyProperties do not have type information
					for (int i = 0; i < properties.Length; i++) {
						if (properties[i].PropertyType != typeof (string))
							continue;
					
						prop_name = properties[i].Name + "Property";
						property = GetDependencyPropertyByName (widget, ctype, prop_name);
						if (property == null)
							continue;
					
						// First thing we try is setting the string to something
						method = "SetValue(" + ctype.Name + "." + prop_name + ", \"some text\")";
					
						try {
							widget.SetValue (property, "some text");

							sb.AppendLine ("            widget." + method + ";");
						} catch (Exception ex) {
							sb.AppendLine ("            Assert.Throws<" + ex.GetType ().Name + ">(delegate {");
							sb.AppendLine ("                widget." + method + ";");
							sb.AppendLine ("            }, \"" + method + " should thow an exception\");");
						}
					
						// Then we check that the value is what we we just set
						method = "GetValue(" + ctype.Name + "." + prop_name + ")";
					
						try {
							retval = widget.GetValue (property);
							actual = retval as string;
						
							sb.AppendLine ("            retval = widget." + method + ";");
						
							if (actual == "some text") {
								sb.AppendLine ("            Assert.AreEqual(\"some text\", retval, \"" + method + " should have returned 'some text'\");");
							} else if (actual != null) {
								sb.AppendLine ("            Assert.AreEqual(\"" + actual + "\", retval, \"" + method + " should have returned '" + actual + "'\");");
							} else {
								sb.AppendLine ("            Assert.IsNull(retval, \"" + method + " should have returned null\");");
							}
						} catch (Exception ex) {
							sb.AppendLine ("            Assert.Throws<" + ex.GetType ().Name + ">(delegate {");
							sb.AppendLine ("                retval = widget." + method + ";");
							sb.AppendLine ("            }, \"" + method + " should thow an exception\");");
						}
					
						// Next we try setting the string to null to see if it will let us
						method = "SetValue(" + ctype.Name + "." + prop_name + ", null)";
					
						try {
							widget.SetValue (property, null);
						
							sb.AppendLine ("            widget." + method + ";");
						} catch (Exception ex) {
							sb.AppendLine ("            Assert.Throws<" + ex.GetType ().Name + ">(delegate {");
							sb.AppendLine ("                widget." + method + ";");
							sb.AppendLine ("            }, \"" + method + " should thow an exception\");");
						}
					
						// Then we check that the value was actually set to null as opposed to String.Empty
						method = "GetValue(" + ctype.Name + "." + prop_name + ")";
					
						try {
							retval = widget.GetValue (property);
							actual = retval as string;
						
							sb.AppendLine ("            retval = widget." + method + ";");
						
							if (actual == String.Empty) {
								sb.AppendLine ("            Assert.AreEqual(String.Empty, retval, \"" + method + " should have returned String.Empty\");");
							} else if (actual != null) {
								sb.AppendLine ("            Assert.AreEqual(\"" + actual + "\", retval, \"" + method + " should have returned '" + actual + "'\");");
							} else {
								sb.AppendLine ("            Assert.IsNull(retval, \"" + method + " should have returned null\");");
							}
						} catch (Exception ex) {
							sb.AppendLine ("            Assert.Throws<" + ex.GetType ().Name + ">(delegate {");
							sb.AppendLine ("                retval = widget." + method + ";");
							sb.AppendLine ("            }, \"" + method + " should thow an exception\");");
							retval = actual = null;
						}
					
						// If the GetValue returned String.Empty, then we need to check ReadLocalValue to see wtf is going on
						if (actual == String.Empty) {
							method = "ReadLocalValue(" + PrettyName (ctype) + "." + prop_name + ")";
						
							try {
								retval = widget.ReadLocalValue (property);
							
								sb.AppendLine ("            retval = widget." + method + ";");
							
								if (retval != DependencyProperty.UnsetValue) {
									actual = retval as string;
								
									if (actual == String.Empty) {
										sb.AppendLine ("            Assert.AreEqual(String.Empty, retval, \"" + method + " should have returned String.Empty\");");
									} else if (actual != null) {
										sb.AppendLine ("            Assert.AreEqual(\"" + actual + "\", retval, \"" + method + " should have returned '" + actual + "'\");");
									} else {
										sb.AppendLine ("            Assert.IsNull(retval, \"" + method + " should have returned null\");");
									}
								} else {
									sb.AppendLine ("            Assert.AreEqual(DependencyProperty.UnsetValue, retval, \"" + method + 
										       " should not have a value by default\");");
								}
							} catch (Exception ex) {
								sb.AppendLine ("            // [MoonlightBug] - Moonlight needs to be fixed to throw on some ReadLocalValue invocations");
								sb.AppendLine ("            //Assert.Throws<" + ex.GetType ().Name + ">(delegate {");
								sb.AppendLine ("            //    retval = widget." + method + ";");
								sb.AppendLine ("            //}, \"" + method + " should thow an exception\");");
							}
						}
					}

					sb.AppendLine ("        }");
					sb.AppendLine ();
				}
				
				ctype = ctype.BaseType;
			}
		}
		
		void GenerateSetValueTests (DependencyObject widget, Type type)
		{
			DependencyProperty property;
			FieldInfo[] fields;
			Type ctype = type;
			string method;
			
			while (ctype.IsSubclassOf (typeof (DependencyObject)) && !ctype.IsGenericType) {
				fields = ctype.GetFields ();

				bool emit_test = false;

				for (int i = 0; i < fields.Length; i++) {
					if (!fields[i].IsPublic || !fields[i].IsStatic || fields[i].FieldType != typeof (DependencyProperty))
						continue;
					
					try {
						property = fields[i].GetValue (null) as DependencyProperty;
					} catch {
						property = null;
					}
					
					if (property == null)
						continue;

					emit_test = true;
					break;
				}

				if (emit_test) {
					EmitTestMethod (type, "SetValue_" + PrettyName (ctype), true);

					for (int i = 0; i < fields.Length; i++) {
						if (!fields[i].IsPublic || !fields[i].IsStatic || fields[i].FieldType != typeof (DependencyProperty))
							continue;
					
						try {
							property = fields[i].GetValue (null) as DependencyProperty;
						} catch {
							property = null;
						}
					
						if (property == null)
							continue;
					
						method = "SetValue(" + ctype.Name + "." + fields[i].Name + ", null)";
					
						try {
							widget.SetValue (property, null);
						
							sb.AppendLine ("            widget." + method + ";");
						} catch (Exception ex) {
							sb.AppendLine ("            Assert.Throws<" + ex.GetType ().Name + ">(delegate {");
							sb.AppendLine ("                widget." + method + ";");
							sb.AppendLine ("            }, \"" + method + " should thow an exception\");");
						}
					}

					sb.AppendLine ("        }");
					sb.AppendLine ();
				}
				
				ctype = ctype.BaseType;
			}
		}
	}
}
