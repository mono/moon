using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Reflection;
using System.Text;

namespace dependency_properties
{
	public delegate void TestCode ();

	public static class Tester
	{
		static IOutput output;
		static StringBuilder test_output = new StringBuilder ();

		public static void Write (string text)
		{
			test_output.Append (text);
		}

		public static void WriteLine (string line)
		{
			test_output.AppendLine (line);
		}

		public static void Execute (IOutput output)
		{
			ConstructorInfo ctor;
			TestResult result;
			object test;
			int failed = 0, succeeded = 0;

			Tester.output = output;

			output.StartReport ();
			foreach (Type type in Assembly.GetExecutingAssembly ().GetTypes ()) {
				if (!type.IsDefined (typeof (TestClassAttribute), false))
					continue;

				ctor = type.GetConstructor (Type.EmptyTypes);

				if (ctor == null)
					continue;

				try {
					test = ctor.Invoke (null);
				} catch {//(Exception ex) {
					//output.WriteLine ("Exception while trying to instantiate test class: " + ex.ToString ());
					continue;
				}

				foreach (MethodInfo method in type.GetMethods (BindingFlags.Public | BindingFlags.Instance)) {
					if (!method.IsDefined (typeof (TestMethodAttribute), false))
						continue;

					if (method.GetParameters ().Length != 0)
						continue;

					if (method.ReturnType != typeof (void))
						continue;

					result = new TestResult ();
					result.name = type.FullName + "." + method.Name;

					Console.WriteLine ("Executing {0}...", result.name);

					test_output.Length = 0;

					try {
						method.Invoke (test, null);
						result.success = true;
					} catch (TargetInvocationException tie) {
						Exception ex = tie;
						while (ex is TargetInvocationException)
							ex = ex.InnerException;
						result.success = false;
						result.ex = ex;
					} catch (UnitTestAssertException ex) {
						result.success = false;
						result.ex = null;
						result.output = ex.ToString ();
					} catch (Exception ex) {
						result.success = false;
						result.ex = ex;
					}
					result.output = test_output.ToString ();
					if (result.success)
						succeeded++;
					else
						failed++;
					output.Report (result);
					Console.WriteLine (result.success);
				}
			}
			result = new TestResult ();
			result.name = "Summary:";
			result.success = failed == 0;
			result.output = string.Format ("Succeeded: {0}/{2} Failed: {1}/{2}", succeeded, failed, failed + succeeded);
			output.Report (result);
			output.EndReport ();
		}

	}

	public class TestMethodAttribute : Attribute { }
	public class TestClassAttribute : Attribute { }

	public class TestResult
	{
		public bool success;
		public Exception ex;
		public string output;
		public string name;
	}

	public static class Assert
	{
		public static void IsNull (object obj)
		{
			if (obj != null)
				throw new AssertFailedException ();
		}
		public static void IsNull (object obj, string message)
		{
			if (obj != null)
				throw new AssertFailedException (message);
		}
		public static void IsNull (object obj, string message, params object [] parameters) {
			if (obj != null)
				throw new AssertFailedException (string.Format (message, parameters));
		}
		public static void AreSame (object expected, object actual)
		{
			if (expected != actual)
				throw new AssertFailedException ();
		}
		public static void AreSame (object expected, object actual, string message)
		{
			if (expected != actual)
				throw new AssertFailedException (message);
		}
		public static void AreSame (object expected, object actual, string message, params object [] parameters)
		{
			if (expected != actual)
				throw new AssertFailedException (string.Format (message, parameters));
		}

		public static void AreEqual (object expected, object actual)
		{
			AreEqual (expected, actual, null);
		}

		public static void AreEqual (object expected, object actual, string message)
		{
			if (!object.Equals (expected, actual))
				throw new AssertFailedException (message);
		}

		public static void AreEqual (object expected, object actual, string message, params object [] parameters)
		{
			AreEqual (expected, actual, string.Format (message, parameters));

		}

		public static void AreEqual (int expected, int actual)
		{
			if (expected != actual)
				throw new AssertFailedException ();
		}

		public static void AreEqual (int expected, int actual, string message)
		{
			if (expected != actual)
				throw new AssertFailedException (message);
		}

		public static void AreEqual (int expected, int actual, string message, params object [] parameters)
		{
			if (expected != actual)
				throw new AssertFailedException (string.Format (message, parameters));
	
		}

		public static void AreEqual (double expected, double actual)
		{
			if (expected != actual)
				throw new AssertFailedException ();

		}
		public static void AreEqual (double expected, double actual, string message)
		{
			if (expected != actual)
				throw new AssertFailedException (message);

		}
		public static void AreEqual (double expected, double actual, string message, params object [] parameters)
		{
			if (expected != actual)
				throw new AssertFailedException (string.Format (message, parameters));

		}

		public static void IsInstanceOfType (object value, Type expectedType)
		{
			IsInstanceOfType (value, expectedType, null);
		}

		public static void IsInstanceOfType (object value, Type expectedType, string message)
		{
			if (value == null)
				throw new AssertFailedException (message);
			else if (!(value.GetType () == expectedType || value.GetType ().IsSubclassOf (expectedType)))
				throw new AssertFailedException (message);
		}

		public static void IsInstanceOfType (object value, Type expectedType, string message, params object [] parameters)
		{
			IsInstanceOfType (value, expectedType, string.Format (message, parameters));
		}

		public static void Throws (TestCode code, Type expected_exception)
		{
			Throws (code, expected_exception, null);
		}

		public static void Throws (TestCode code, Type expected_exception, string message)
		{
			try {
				code ();
				throw new AssertFailedException (message);
			} catch (Exception ex) {
				if (!(ex.GetType () == expected_exception))
					throw new AssertFailedException (message);
			}
		}
		public static void Throws (TestCode code, Type exception, string message, params object [] parameters)
		{
			Throws (code, exception, string.Format (message, parameters));
		}
	}

	public class UnitTestAssertException : Exception
	{
		public UnitTestAssertException () { }
		public UnitTestAssertException (string message) : base (message) { }
	}
	public class AssertFailedException : UnitTestAssertException 
	{
		public AssertFailedException () { }
		public AssertFailedException (string message) : base (message) { }
	}
	public class AssertInconclusiveException : UnitTestAssertException { }
	public class InternalTestFailureException : UnitTestAssertException { }
}
