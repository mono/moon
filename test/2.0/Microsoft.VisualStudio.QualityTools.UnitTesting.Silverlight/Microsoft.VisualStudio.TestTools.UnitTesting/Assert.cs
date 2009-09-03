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
using System.Windows;
using System.Windows.Media;

namespace Microsoft.VisualStudio.TestTools.UnitTesting
{
	public delegate void VisualElement (DependencyObject o);

	public static class Assert
	{
		public static void VisualChildren (DependencyObject control, params VisualNode [ ] nodes)
		{
			VisualChildren (control, "", nodes);
		}
		public static void VisualChildren (DependencyObject control, string error, params VisualNode[] nodes)
		{
			int count = VisualTreeHelper.GetChildrenCount (control);
			if (nodes.Length != count)
				Assert.Fail ("Initial control has {0} children but should have {1}. {2}", count, nodes.Length, error);

			for (int i = 0; i < count; i++) {
				DependencyObject child = VisualTreeHelper.GetChild (control, i);
				Assert.IsInstanceOfType (child, nodes [i].Type, "Node {0}", nodes[i].Name);
				nodes [i].DoCheck (child);


				// This means we explicitly don't want to check the children of this node
				if (nodes[i].Siblings == null)
					continue;

				int children = VisualTreeHelper.GetChildrenCount (child);
				if (children != nodes [i].Siblings.Length)
				Assert.Fail ("Node {0} should have {1} children but has {2} children",
																		nodes [i].Name,
																		nodes [i].Siblings.Length,
																		children);
				VisualChildren (child, nodes [i].Siblings);
			}
		}

		public static void VisualParent (DependencyObject control, VisualNode node)
		{
			DependencyObject p = VisualTreeHelper.GetParent (control);
			Assert.IsInstanceOfType (p, node.Type, "Node {0}", node.Name);
			if (node.Siblings.Length == 0)
				return;
			else if (node.Siblings.Length == 1)
				VisualParent (p, node.Siblings [0]);
			else
				throw new Exception (string.Format ("Invalid test - Node {0} contains more than 1 parent", node.Name));
		}
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


		public static void IsBetween (double min, double max, double actual)
		{
			if (actual > max || actual < min)
				throw new AssertFailedException (string.Format ("Actual value '{0}' is not between '{1}' and '{2}'). ", actual, min, max));
		}

		public static void IsBetween (double min, double max, double actual, string message)
		{
			if (actual > max || actual < min)
				throw new AssertFailedException (string.Format ("Actual value '{0}' is not between '{1}' and '{2}'). " + message, actual, min, max));
		}

		public static void IsBetween (double min, double max, double actual, string message, params object [ ] parameters)
		{
			if (actual > max || actual < min)
				throw new AssertFailedException (string.Format ("Actual value '{0}' is not between '{1}' and '{2}'). {3}", actual, min, max, string.Format (message, parameters)));
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

		public static void IsGreater (double expected, double actual)
		{
			if (expected >= actual) {
				string msg = String.Format ("Actual value '{0}' is not greater than expected value '{1}'.", actual, expected);
				throw new AssertFailedException (msg);
			}
		}

		public static void IsGreater (double expected, double actual, string message)
		{
			if (expected >= actual) {
				string msg = String.Format ("Actual value '{0}' is not greater than expected value '{1}'. {2}", actual, expected, message);
				throw new AssertFailedException (msg);
			}
		}

		public static void IsGreater (double expected, double actual, string message, params object [ ] parameters)
		{
			if (expected >= actual) {
				string msg = String.Format ("Actual value '{0}' is not greater than expected value '{1}'.", actual, expected, string.Format (message, parameters));
				throw new AssertFailedException (msg);
			}
		}

		public static void IsLess (double expected, double actual)
		{
			if (expected <= actual) {
				string msg = String.Format ("Actual value '{0}' is not less than expected value '{1}'.", actual, expected);
				throw new AssertFailedException (msg);
			}
		}

		public static void IsLess (double expected, double actual, string message)
		{
			if (expected <= actual) {
				string msg = String.Format ("Actual value '{0}' is not less than expected value '{1}'. {2}", actual, expected, message);
				throw new AssertFailedException (msg);
			}
		}

		public static void IsLess (int expected, int actual, string message, params object [ ] parameters)
		{
			if (expected <= actual) {
				string msg = String.Format ("Actual value '{0}' is not less than expected value '{1}'.", actual, expected, string.Format (message, parameters));
				throw new AssertFailedException (msg);
			}
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

		public static void AreNotSame (object expected, object actual)
		{
			if (expected == actual) {
				string msg = String.Format ("Expected value and actual value were both the same");
				throw new AssertFailedException (msg);
			}
		}
		
		public static void AreNotSame (object expected, object actual, string message)
		{
			if (expected == actual) {
				string msg = String.Format ("Expected value and actual value were both the same. {0}", message);
				throw new AssertFailedException (msg);
			}
		}
		
		public static void AreNotSame (object expected, object actual, string message, params object [] parameters)
		{
			if (expected == actual) {
				string msg = String.Format ("Expected value and actual value were both the same. {0}", string.Format (message, parameters));
				throw new AssertFailedException (msg);
			}
		}

		public static void IsInstanceOfType<T> (object value)
		{
			IsInstanceOfType (value, typeof (T));
		}
		
		public static void IsInstanceOfType (object value, Type expectedType)
		{
			IsInstanceOfType (value, expectedType, null);
		}

		public static void IsInstanceOfType<T> (object value, string message)
		{
			IsInstanceOfType (value, typeof (T), message);
		}

		public static void IsInstanceOfType (object value, Type expectedType, string message)
		{
			message = string.Format ("Expected '{0}' but was '{1}'. {2}", expectedType.Name, value == null ? "<null>" : value.GetType ().Name, message);
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

		public static void IsUnset (DependencyObject d, DependencyProperty prop, string message)
		{
			if (d.ReadLocalValue (prop) != DependencyProperty.UnsetValue)
				Assert.Fail ("Property should not have a local value set. {0}", message);
		}

		// Moonlight addition
		public static void Matrix (Matrix matrix, int m11, int m12, int m21, int m22, int offsetX, int offsetY, string message, params object [] paramters)
		{
			Assert.AreEqual (matrix.M11, m11, "{0} - {1}", message, "M11");
			Assert.AreEqual (matrix.M12, m12, "{0} - {1}", message, "M12");
			Assert.AreEqual (matrix.M21, m21, "{0} - {1}", message, "M21");
			Assert.AreEqual (matrix.M22, m22, "{0} - {1}", message, "M22");
			Assert.AreEqual (matrix.OffsetX, offsetX, "{0} - {1}", message, "OffsetX");
			Assert.AreEqual (matrix.OffsetY, offsetY, "{0} - {1}", message, "OffsetY");
		}

		// Moonlight addition
		public static void Throws<TException> (TestCode code) where TException : Exception
		{
			Throws (code, typeof (TException), null, String.Empty);
		}

		// Moonlight addition
		public static void Throws<TException, TInnerException> (TestCode code)
			where TException : Exception
			where TInnerException : Exception
		{
			Throws (code, typeof (TException), typeof (TInnerException), String.Empty);
		}

		// Moonlight addition
		public static void Throws<TException> (TestCode code, string message) where TException : Exception
		{
			Throws (code, typeof (TException), null, message);
		}

		// Moonlight addition
		public static void Throws<TException, TInnerException> (TestCode code, string message)
			where TException : Exception
			where TInnerException : Exception
		{
			Throws (code, typeof (TException), typeof (TInnerException), message);
		}

		// Moonlight addition
		public static void Throws (TestCode code, Type expected_exception)
		{
			Throws (code, expected_exception, null, String.Empty);
		}
		
 		// Moonlight addition
		public static void Throws (TestCode code, Type expected_exception, string message)
		{
			Throws (code, expected_exception, null, message);
		}

		// Moonlight addition
		public static void Throws (TestCode code, Type expected_exception, Type expected_inner_exception, string message)
		{
			bool failed = false;
			try {
				code ();
				failed = true;
			} catch (Exception ex) {
				if (!(ex.GetType () == expected_exception))
					throw new AssertFailedException (string.Format ("Expected '{0}', got '{1}'. {2}", expected_exception.FullName, ex.GetType ().FullName, message));
				//System.Diagnostics.Debug.WriteLine (ex.ToString ());
				if (expected_inner_exception != null) {
					// we only check if the inner exception was supplied
					if (ex.InnerException.GetType () != expected_inner_exception)
						throw new AssertFailedException (string.Format ("Expected InnerException '{0}', got '{1}'. {2}", expected_inner_exception.FullName, ex.InnerException.GetType ().FullName, message));
				}
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
