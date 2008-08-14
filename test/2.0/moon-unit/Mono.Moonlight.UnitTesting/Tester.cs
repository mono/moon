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

namespace Mono.Moonlight.UnitTesting
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
			Exception e;
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

					if (method.IsDefined (typeof (IgnoreAttribute), false)) {
						IgnoreAttribute ia = (IgnoreAttribute) method.GetCustomAttributes (typeof (IgnoreAttribute), false) [0];
						Console.WriteLine ("Ignored {0}... (reason: '{1}')", result.name, string.IsNullOrEmpty (ia.Reason) ? "None" : ia.Reason);
						continue;
					} else {
						Console.WriteLine ("Executing {0}...", result.name);
					}


					test_output.Length = 0;

					e = null;
					try {
						method.Invoke (test, null);
					} catch (Exception ex) {
						e = ex;
					}

					while (e != null && e.InnerException != null && e is TargetInvocationException)
						e = e.InnerException;

					result.output = test_output.ToString ();

					if (e == null) {
						result.success = true;
					} else if (e is UnitTestAssertException) {
						UnitTestAssertException ex = e as UnitTestAssertException;
						result.success = false;
						result.reason = ex.FailedMessage;
						result.output += "\n" + ex.ToString ();
					} else {
						result.success = false;
						result.reason = e.ToString();
					}
					if (result.success)
						succeeded++;
					else
						failed++;
					output.Report (result);
					//Console.WriteLine (result.success);
				}
			}
			output.EndReport ();
		}

	}

	public class TestMethodAttribute : Attribute { }
	public class TestClassAttribute : Attribute { }

	public class IgnoreAttribute : Attribute
	{
		private string reason;
		public IgnoreAttribute () { }
		public IgnoreAttribute (string reason) 
		{
			this.reason = reason; 
		}
		public string Reason {
			get { return reason; }
			set { reason = value; }
		}
	}

	public class TestResult
	{
		public bool success;
		public string reason;
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
			bool failed = false;
			try {
				code ();
				failed = true;
			} catch (Exception ex) {
				if (!(ex.GetType () == expected_exception))
					throw new AssertFailedException (message, string.Format ("Expected '{0}', got '{1}'", expected_exception.FullName, ex.GetType ().FullName));
				//System.Diagnostics.Debug.WriteLine (ex.ToString ());
			}
			if (failed)
				throw new AssertFailedException (message, string.Format ("Expected '{0}', but got no exception.", expected_exception.FullName));
		}
		public static void Throws (TestCode code, Type exception, string message, params object [] parameters)
		{
			Throws (code, exception, string.Format (message, parameters));
		}
	}

	public class UnitTestAssertException : Exception
	{
		private string failed_message;
		public UnitTestAssertException () { }
		public UnitTestAssertException (string message) : base (message) { }
		public UnitTestAssertException (string message, string failed_message)
			: base (message)
		{
			this.failed_message = failed_message;
		}
		public string FailedMessage	{
			get { return failed_message; }
		}
	}
	public class AssertFailedException : UnitTestAssertException 
	{
		public AssertFailedException () { }
		public AssertFailedException (string message) : base (message) { }
		public AssertFailedException (string message, string failed_message) : base (message, failed_message) { }
	}
	public class AssertInconclusiveException : UnitTestAssertException { }
	public class InternalTestFailureException : UnitTestAssertException { }
}
