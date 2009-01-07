/*
 * Assert.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


using System;

namespace Microsoft.VisualStudio.TestTools.UnitTesting
{
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

		public static void IsNotNull (object obj)
		{
			if (obj == null)
				throw new AssertFailedException ();
		}

		public static void IsNotNull (object obj, string message)
		{
			if (obj == null)
				throw new AssertFailedException (message);
		}

		public static void IsNotNull (object obj, string message, params object [] parameters)
		{
			if (obj == null)
				throw new AssertFailedException (string.Format (message, parameters));
		}

		public static void IsFalse (bool condition)
		{
			if (condition)
				throw new AssertFailedException ();
		}

		public static void IsFalse (bool condition, string message)
		{
			if (condition)
				throw new AssertFailedException (message);
		}

		public static void IsFalse (bool condition, string message, params object [] parameters)
		{
			if (condition)
				throw new AssertFailedException (string.Format (message, parameters));
		}

		public static void IsTrue (bool condition)
		{
			if (!condition)
				throw new AssertFailedException ();
		}

		public static void IsTrue (bool condition, string message)
		{
			if (!condition)
				throw new AssertFailedException (message);
		}

		public static void IsTrue (bool condition, string message, params object [] parameters)
		{
			if (!condition)
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
			if (!object.Equals (expected, actual)) {
				string msg = String.Format ("Actual value is {0} while the expected value was {1}. {2}",
							    actual == null ? "<null>" : string.Format ("'{0}'", actual),
							    expected == null ? "<null>" : string.Format ("'{0}'", expected), message);
				throw new AssertFailedException (msg);
			}
		}

		public static void AreEqual (object expected, object actual, string message, params object [] parameters)
		{
			AreEqual (expected, actual, string.Format (message, parameters));

		}

		public static void AreEqual (int expected, int actual)
		{
			if (expected != actual) {
				string msg = String.Format ("Actual value is '{0}' while the expected value was '{1}'.", actual, expected);
				throw new AssertFailedException (msg);
			}
		}

		public static void AreEqual (int expected, int actual, string message)
		{
			if (expected != actual) {
				string msg = String.Format ("Actual value is '{0}' while the expected value was '{1}'. {2}", actual, expected, message);
				throw new AssertFailedException (msg);
			}
		}

		public static void AreEqual (int expected, int actual, string message, params object [] parameters)
		{
			if (expected != actual) {
				string msg = String.Format ("Actual value is '{0}' while the expected value was '{1}'. {2}", actual, expected, string.Format (message, parameters));
				throw new AssertFailedException (msg);
			}
		}

		public static void AreEqual (double expected, double actual)
		{
			if (expected != actual) {
				string msg = String.Format ("Actual value is '{0}' while the expected value was '{1}'.", actual, expected);
				throw new AssertFailedException (msg);
			}
		}
		public static void AreEqual (double expected, double actual, string message)
		{
			if (expected != actual) {
				string msg = String.Format ("Actual value is '{0}' while the expected value was '{1}'. {2}", actual, expected, message);
				throw new AssertFailedException (msg);
			}

		}
		public static void AreEqual (double expected, double actual, string message, params object [] parameters)
		{
			if (expected != actual) {
				string msg = String.Format ("Actual value is '{0}' while the expected value was '{1}' {2}.", actual, expected, string.Format (message, parameters));
				throw new AssertFailedException (msg);
			}

		}

		public static void AreNotEqual (object expected, object actual)
		{
			AreNotEqual (expected, actual, null);
		}

		public static void AreNotEqual (object expected, object actual, string message)
		{
			if (object.Equals (expected, actual)) {
				string msg = String.Format ("Expected value and actual value were both the same. {0}", message);
				throw new AssertFailedException (msg);
			}
		}

		public static void AreNotEqual (object expected, object actual, string message, params object [] parameters)
		{
			AreNotEqual (expected, actual, string.Format (message, parameters));

		}

		public static void AreNotEqual (int expected, int actual)
		{
			if (expected == actual) {
				string msg = String.Format ("Expected value and actual value were both the same");
				throw new AssertFailedException (msg);
			}
		}

		public static void AreNotEqual (int expected, int actual, string message)
		{
			if (expected == actual) {
				string msg = String.Format ("Expected value and actual value were both the same. {0}", message);
				throw new AssertFailedException (msg);
			}
		}

		public static void AreNotEqual (int expected, int actual, string message, params object [] parameters)
		{
			if (expected == actual) {
				string msg = String.Format ("Expected value and actual value were both the same. {0}", string.Format (message, parameters));
				throw new AssertFailedException (msg);
			}
		}

		public static void AreNotEqual (double expected, double actual)
		{
			if (expected == actual) {
				string msg = String.Format ("Expected value and actual value were both the same");
				throw new AssertFailedException (msg);
			}
		}
		public static void AreNotEqual (double expected, double actual, string message)
		{
			if (expected == actual) {
				string msg = String.Format ("Expected value and actual value were both the same. {0}", message);
				throw new AssertFailedException (msg);
			}

		}
		public static void AreNotEqual (double expected, double actual, string message, params object [] parameters)
		{
			if (expected == actual) {
				string msg = String.Format ("Expected value and actual value were both the same. {0}", string.Format (message, parameters));
				throw new AssertFailedException (msg);
			}

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

		public static void Fail ()
		{
			throw new AssertFailedException (string.Empty);
		}

		public static void Fail (string message)
		{
			throw new AssertFailedException (message);
		}

		public static void Fail (string message, params object [] parameters)
		{
			throw new AssertFailedException (string.Format (message, parameters));
		}
		
		// Moonlight addition
		public static void Throws<TException> (TestCode code) where TException : Exception
		{
			Throws (code, typeof (TException), null);
		}

		// Moonlight addition
		public static void Throws<TException> (TestCode code, string message) where TException : Exception
		{
			Throws (code, typeof (TException), message);
		}

		// Moonlight addition
		public static void Throws (TestCode code, Type expected_exception)
		{
			Throws (code, expected_exception, null);
		}
		
 		// Moonlight addition
		public static void Throws (TestCode code, Type expected_exception, string message)
		{
			bool failed = false;
			try {
				code ();
				failed = true;
			} catch (Exception ex) {
				if (!(ex.GetType () == expected_exception))
					throw new AssertFailedException (string.Format ("Expected '{0}', got '{1}'. {2}", expected_exception.FullName, ex.GetType ().FullName, message));
				//System.Diagnostics.Debug.WriteLine (ex.ToString ());
			}
			if (failed)
				throw new AssertFailedException (string.Format ("Expected '{0}', but got no exception. {1}", expected_exception.FullName, message));
		}

		// Moonlight addition
		public static void Throws (TestCode code, Type exception, string message, params object [] parameters)
		{
			Throws (code, exception, string.Format (message, parameters));
		}
	}
}