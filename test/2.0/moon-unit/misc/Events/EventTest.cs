/*
 * EventTest.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Net;
using System.Resources;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.Collections.Generic;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

#pragma warning disable 169 // we have unused methods which are bound to by reflection

namespace MoonTest.Misc
{
	[TestClass]
	public class EventTest : SilverlightTest
	{
		public string FailMessage;
		EventTestCanvas etc;

		private void EnqueueCompleteIfThrows (Action action)
		{
			try {
				etc = null;
				action ();
			} catch (Exception ex){
				Console.WriteLine ("EnqueueCompleteIfThrows: {0}", ex.Message);
				if (etc != null)
					etc.Done = true;
				EnqueueTestComplete ();
				throw;
			}
		}

		private void EnqueueCompleteIfEtcIsNull (Action action)
		{
			try {
				Console.WriteLine ("EnqueueCompleteIfEtcIsNull");
				etc = null;
				action ();
			} catch (Exception ex) {
				Console.WriteLine ("EnqueueCompleteIfEtcIsNull: {0}", ex.Message);
				throw;
			} finally {
				if (etc == null) {
					Console.WriteLine ("EnqueueCompleteIfEtcIsNull: EnqueueTestComplete ()");
					EnqueueTestComplete ();
				} else {
					etc.StartTimeout ();
					Console.WriteLine ("EnqueueCompleteIfEtcIsNull [Done]");
				}
			}
		}

		[TestMethod]
		[Asynchronous]
		public void TestBasic ()
		{
			EnqueueCompleteIfThrows (delegate ()
			{
				etc = new EventTestCanvas (this, "Event_Basic.xaml");
				etc.StartTimeout ();
				//Assert.AreEqual (123, etc.Width);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestRoutedEventArgs ()
		{
			EnqueueCompleteIfThrows (delegate ()
			{
				etc = new EventTestCanvas (this, "Event_RoutedEventArgs.xaml");
				etc.StartTimeout ();
				Assert.AreEqual (123, etc.Width);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestObjectObject ()
		{
			EnqueueCompleteIfThrows (delegate ()
			{
				etc = new EventTestCanvas (this, "Event_ObjectObject.xaml");
				etc.StartTimeout ();
				Assert.AreEqual (123, etc.Width);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestProtected ()
		{
			EnqueueCompleteIfThrows (delegate ()
			{
				etc = new EventTestCanvas (this, "Event_Protected.xaml");
				etc.StartTimeout ();
				Assert.AreEqual (123, etc.Width);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestInternal ()
		{
			EnqueueCompleteIfThrows (delegate ()
			{
				etc = new EventTestCanvas (this, "Event_Internal.xaml");
				etc.StartTimeout ();
				Assert.AreEqual (123, etc.Width);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestPrivate ()
		{
			EnqueueCompleteIfThrows (delegate ()
			{
				etc = new EventTestCanvas (this, "Event_Private.xaml");
				etc.StartTimeout ();
				Assert.AreEqual (123, etc.Width);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestExDerived ()
		{
			EnqueueCompleteIfThrows (delegate ()
			{
				etc = new EventTestCanvas (this, "Event_Ex_Derived.xaml");
				Assert.AreEqual (123, etc.Width);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestExDerived2 ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				Assert.Throws<XamlParseException> (delegate () { new EventTestCanvas (this, "Event_Ex_Derived2.xaml"); });
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestExDerived3 ()
		{
			EnqueueCompleteIfThrows (delegate ()
			{
				etc = new EventTestCanvas (this, "Event_Ex_Derived3.xaml");
				Assert.AreEqual (123, etc.Width);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestInexistentResource ()
		{
			// Note "moon_unit" instead of "moon-unit"
			// this isn't supposed to neither throw an exception (this xaml file should throw an exception), nor load anything at all
			EnqueueCompleteIfThrows (delegate ()
			{
				etc = new EventTestCanvas (this, new Uri ("/moon_unit;component/misc/Events/Event_InexistentResource.xaml", UriKind.Relative));
				Assert.AreNotEqual (123, etc.Width);
				EnqueueTestComplete ();
			});

		}

		[TestMethod]
		[Asynchronous]
		public void Test0 ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				Assert.Throws (delegate () { etc = new EventTestCanvas (this, "Event_0.xaml"); }, typeof (XamlParseException));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void Test1 ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				Assert.Throws (delegate () { etc = new EventTestCanvas (this, "Event_1.xaml"); }, typeof (XamlParseException));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void Test3 ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				Assert.Throws (delegate () { etc = new EventTestCanvas (this, "Event_3.xaml"); }, typeof (XamlParseException));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestDO ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				Assert.Throws (delegate () { etc = new EventTestCanvas (this, "Event_DO.xaml"); }, typeof (XamlParseException));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestExact ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				Assert.Throws (delegate () { etc = new EventTestCanvas (this, "Event_Exact.xaml"); }, typeof (XamlParseException));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestExact_CaseMismatch ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				// Case doesn't matter. VS always embeds resources in lowercase
				Assert.Throws<XamlParseException>(delegate () { etc = new EventTestCanvas (this, "event_exactcasemismatch.xaml"); });					
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestMouseArgs ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				Assert.Throws (delegate () { etc = new EventTestCanvas (this, "Event_MouseArgs.xaml"); }, typeof (XamlParseException));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestNonVoid ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				Assert.Throws (delegate () { etc = new EventTestCanvas (this, "Event_NonVoid.xaml"); }, typeof (XamlParseException));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestOverloadedInvalid ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				Assert.Throws (delegate () { etc = new EventTestCanvas (this, "Event_OverloadedInvalid.xaml"); }, typeof (XamlParseException));

			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestOverloadedValid ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				Assert.Throws (delegate () { etc = new EventTestCanvas (this, "Event_OverloadedValid.xaml"); }, typeof (XamlParseException));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestCaseMismatch ()
		{
			EnqueueCompleteIfEtcIsNull (delegate ()
			{
				Assert.Throws (delegate () { etc = new EventTestCanvas (this, "Event_CaseMismatch.xaml"); }, typeof (XamlParseException));
			});
		}

		public void MarkAsDone (EventTestCanvas canvas)
		{
			// The test framework automatically clears TestPanel's children
			EnqueueTestComplete ();
		}

		public void MarkTestAsFailed (EventTestCanvas canvas, string message)
		{
			Enqueue (() => Assert.Fail (message));
			EnqueueTestComplete ();
		}

		public void MarkTestAsTimedOut (EventTestCanvas canvas)
		{
			EnqueueCallback (delegate () { Assert.Fail ("Test timed out."); });
			EnqueueTestComplete ();
		}

	}

	public class EventTestCanvas : Canvas
	{
		private EventTest event_test;
		public bool Done;

		public EventTestCanvas (EventTest event_test, string xaml_resource_file)
			: this (event_test, new Uri ("/moon-unit;component/misc/Events/" + xaml_resource_file, UriKind.Relative))
		{
		}

		public EventTestCanvas (EventTest event_test, Uri uri)
		{
			this.event_test = event_test;
			this.event_test.FailMessage = null;

			Application.LoadComponent (this, uri);

			event_test.TestPanel.Children.Add (this);
		}

		public void StartTimeout ()
		{
			Console.WriteLine ("StartTimeout");
			DispatcherTimer timer = new DispatcherTimer ();
			timer.Tick += delegate (object o, EventArgs e) 
			{ 
				timer.Stop ();
				if (!Done) 
					event_test.MarkTestAsTimedOut (this); 
			};
			timer.Interval = TimeSpan.FromMilliseconds (200);
			timer.Start ();
		}

		private void Fail ()
		{
			Done = true;
			event_test.MarkTestAsFailed (this, "This xaml file is not supposed to be loaded.");
		}

		private void Success ()
		{
			Done = true;
			event_test.MarkAsDone (this);
		}

		#region Working events
		public void Loaded_Basic (object o, EventArgs e)
		{
			Console.WriteLine ("Loaded_Basic");
			Success ();
		}

		public void Loaded_ObjectObject (object o, object e)
		{
			Success ();
		}

		public void Loaded_RoutedEventArgs (object o, RoutedEventArgs e)
		{
			Success ();
		}

		protected void Loaded_Protected (object obj, EventArgs e)
		{
			Success ();
		}

		internal void Loaded_Internal (object obj, EventArgs e)
		{
			Success ();
		}

		private void Loaded_Private (object obj, EventArgs e)
		{
			Success ();
		}

		// custom events in derived classes
		public event EventHandler LoadedEx;

		public void Loaded_Derived (object obj, EventArgs e)
		{
			if (LoadedEx != null)
				LoadedEx (obj, e);
		}
		public void LoadedEx_Derived (object obj, EventArgs e)
		{
			Success ();
		}

		// event in derived class with non-standard signature
		public delegate void Loaded_Derived2Delegate (string str);
		public event Loaded_Derived2Delegate LoadedEx2;
		public void Loaded_Derived2 (object obj, EventArgs e)
		{
			if (LoadedEx2 != null)
				LoadedEx2 (null);
		}
		public void LoadedEx_Derived2 (string str)
		{
			Success ();
		}

		// event in derived class with signature mismatch 
		// (the signature for the method specified in xaml is a valid 
		// event signature, but it doesn't match the event delegate)
		public delegate void Loaded_Derived3Delegate (string str);
		public event Loaded_Derived3Delegate LoadedEx3;
		public void Loaded_Derived3 (object obj, EventArgs e)
		{
			if (LoadedEx3 == null)
				Success ();
			else
				Fail ();
		}
		public void LoadedEx_Derived3 (object obj, EventArgs e)
		{
			Success ();
		}

		#endregion
		#region Non-working events
		public void Loaded_Exact (Canvas c, RoutedEvent e)
		{
			Fail ();
		}

		public void Loaded_ExactCaSeMiSmAtCh (Canvas c, RoutedEvent e)
		{
			Fail ();
		}

		public void Loaded_InexistentResource (object o, EventArgs e)
		{
			Fail ();
		}

		public void Loaded_0 ()
		{
			Fail ();
		}

		public void Loaded_1 (object o)
		{
			Fail ();
		}

		public void Loaded_3 (object o, EventArgs e, object third)
		{
			Fail ();
		}


		public int Loaded_NonVoid (object o, EventArgs e)
		{
			Fail ();
			return 0;
		}

		// 2 valid methods with the same name
		public void Loaded_OverloadedValid (object o, EventArgs e)
		{
			Fail ();
		}

		public void Loaded_OverloadedValid (object o, RoutedEventArgs e)
		{
			Fail ();
		}

		// 2 methods with same name, only 1 can be the right one
		public void Loaded_OverloadedInvalid (object o, EventArgs e)
		{
			Fail ();
		}

		public void Loaded_OverloadedInvalid (object o)
		{
			Fail ();
		}

		// theoretically valid signature (Loaded event is (Canvas, RoutedEventArgs)
		// however it looks like the first parameter has to be object 
		public void Loaded_DO (DependencyObject dob, EventArgs e)
		{
			Fail ();
		}

		public void Loaded_MouseArgs (object obj, MouseEventArgs e)
		{
			Fail ();
		}

		private void Loaded_CaSeMiSmAtCh (object obj, EventArgs e)
		{
			Fail ();
		}
		#endregion
	}
}
