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

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Mono.Moonlight.UnitTesting
{
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
			TestInfo result;
			object test;
			Exception e;
			int failed = 0, succeeded = 0;

			Tester.output = output;

			output.StartReport ();
			foreach (Type type in Assembly.GetExecutingAssembly ().GetTypes ()) {
				if (!type.IsDefined (typeof (TestClassAttribute), false) ||
				     type.IsDefined (typeof (IgnoreAttribute), false))
					continue;

				ctor = type.GetConstructor (Type.EmptyTypes);

				if (ctor == null)
					continue;

				try {
					test = ctor.Invoke (null);
				} catch (Exception ex) {
					Console.WriteLine ("Exception while trying to instantiate test class: " + ex.ToString ());
					continue;
				}

				foreach (MethodInfo method in type.GetMethods (BindingFlags.Public | BindingFlags.Instance)) {
					if (!method.IsDefined (typeof (TestMethodAttribute), false))
						continue;

					if (method.GetParameters ().Length != 0)
						continue;

					if (method.ReturnType != typeof (void))
						continue;

					result = new TestInfo ();
					result.name = type.FullName + "." + method.Name;
					result.result = TestResult.NotRun;

					if (method.IsDefined (typeof (IgnoreAttribute), false)) {
						IgnoreAttribute ia = (IgnoreAttribute) method.GetCustomAttributes (typeof (IgnoreAttribute), false) [0];
						Console.WriteLine ("Ignored {0}... (reason: '{1}')", result.name, string.IsNullOrEmpty (ia.Reason) ? "None" : ia.Reason);
						result.reason = ia.Reason;
						result.result = TestResult.Ignore;
					}

					if (result.result == TestResult.NotRun) {
						test_output.Length = 0;

						e = null;
						try {
							Console.WriteLine ("Running: {0}", result.name);
							method.Invoke (test, null);
						} catch (Exception ex) {
							e = ex;
						}

						while (e != null && e.InnerException != null && e is TargetInvocationException)
							e = e.InnerException;

						result.output = test_output.ToString ();

						if (e == null) {
							result.result = TestResult.Pass;
						} else if (e is UnitTestAssertException) {
							UnitTestAssertException ex = e as UnitTestAssertException;
							result.result = TestResult.Fail;
							result.reason = ex.Message;
							result.output += "\n" + ex.ToString ();
						} else {
							result.result = TestResult.Fail;
							result.reason = e.ToString ();
						}
						
						if (result.result == TestResult.Pass)
							succeeded++;
						else
							failed++;

						if (Environment.OSVersion.Platform == PlatformID.Unix &&
						    method.IsDefined (typeof (KnownFailureAttribute), false)) {
							if (result.result == TestResult.Pass)
								result.result = TestResult.UnexpectedPass;
							else
								result.result = TestResult.KnownFailure;
						}
					}

					output.Report (result);
					//Console.WriteLine (result.success);
				}
			}
			output.EndReport ();
		}

	}

	public class KnownFailureAttribute : Attribute  { }

	public class TestInfo
	{
		public TestResult result;
		public Exception ex;
		public string reason;
		public string output;
		public string name;
	}

	public enum TestResult {
		NotRun,
		Pass,
		Fail,
		KnownFailure,
		UnexpectedPass,
		Ignore
	}
}
