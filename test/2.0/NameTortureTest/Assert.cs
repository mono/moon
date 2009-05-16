using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Net;
using System.Text;
using System.Windows;
using System.Windows.Browser;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace NameTortureTest
{
	public delegate void TestCode ();

	public static class Assert {
		public static int Count { get; private set; }
		public static int Failures { get; set; }

		public static int TotalCount { get; private set; }
		public static int TotalFailures { get; set; }

		static TextBox log;

		public static void SetLog (TextBox tb)
		{
			log = tb;
		}

		public static void Reset ()
		{
			Count = 0;
			Failures = 0;
		}

		public static void IsTrue (bool f, string msg)
		{
			Count ++;
			TotalCount ++;

			if (f)
				return;

			Failures ++;
			TotalFailures ++;
			log.Text += string.Format ("    Expected true, was false: {0}\n", msg);
		}

		public static void IsFalse (bool f, string msg)
		{
			Count ++;
			TotalCount ++;

			if (!f)
				return;

			Failures ++;
			TotalFailures ++;
			log.Text += string.Format ("    Expected false, was true: {0}\n", msg);
		}

		public static void IsNull (object o, string msg)
		{
			Count ++;
			TotalCount ++;

			if (o == null)
				return;

			Failures ++;
			TotalFailures ++;
			log.Text += string.Format ("    Expected null but object was '{0}': {1}\n", o, msg);
		}

		public static void IsNotNull (object o, string msg)
		{
			Count ++;
			TotalCount ++;

			if (o != null)
				return;

			Failures ++;
			TotalFailures ++;
			log.Text += string.Format ("    Expected non-null but object was null: {1}\n", o, msg);
		}

		public static void DoesNotThrow (TestCode code, string message)
		{
			Count ++;
			TotalCount ++;

                        try {
                                code ();
                        } catch (Exception ex) {
				log.Text += string.Format ("    Expected no exception, got '{0}'. {1}\n", ex.GetType ().FullName, message);
				Failures ++;
				TotalFailures ++;
				return;
                        }
		}

		public static void Throws<TException> (TestCode code, string message)
                {
			Count ++;
			TotalCount ++;

                        bool failed = false;
                        try {
                                code ();
                                failed = true;
                        } catch (Exception ex) {
                                if (!(ex.GetType () == typeof (TException))) {
					log.Text += string.Format ("    Expected '{0}', got '{1}'. {2}\n", typeof (TException).FullName, ex.GetType ().FullName, message);
					Failures ++;
					TotalFailures ++;
					return;
				}
                        }
                        if (failed) {
                                log.Text += string.Format ("    Expected '{0}', but got no exception. {1}\n", typeof (TException).FullName, message);
				Failures ++;
				TotalFailures ++;
			}
                }
	}

	[ScriptableType]
	public class JSAsserter {
		public void IsNull (object o, string msg) {
			Assert.IsNull (o, msg);
		}

		public void IsNotNull (object o, string msg) {
			Assert.IsNotNull (o, msg);
		}
	}
}