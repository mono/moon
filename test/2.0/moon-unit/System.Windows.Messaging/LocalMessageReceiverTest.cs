//
// LocalMessageReceiver Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Messaging;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Messaging {

	[TestClass]
	public partial class LocalMessageReceiverTest {

		static string LongName = new String ('a', 256);
		static string TooLongName = new String ('z', 257);

		static string InvalidNameComma = "I,got,some,commas";
		static string InvalidNameColon = "I got one colon:";

		void CheckForSingleValue (IEnumerable<string> list, string value)
		{
			int n = 0;
			foreach (string s in list) {
				Assert.AreEqual (value, s, "value");
				n++;
			}
			Assert.AreEqual (1, n, "#");
		}

		[TestMethod]
		public void AnyDomain ()
		{
			CheckForSingleValue (LocalMessageReceiver.AnyDomain, "*");
			// ensure items are read-only (not just the field)
			Assert.AreEqual (typeof (ReadOnlyCollection<string>), LocalMessageReceiver.AnyDomain.GetType (), "AnyDomain/Type");
		}

		[TestMethod]
		public void Ctor_String ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				new LocalMessageReceiver (null);
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				new LocalMessageReceiver (TooLongName);
			}, "TooLongName");

			LocalMessageReceiver lmr = new LocalMessageReceiver (String.Empty);
			Assert.IsNull (lmr.AllowedSenderDomains, "AllowedSenderDomains");
			Assert.IsFalse (lmr.DisableSenderTrustCheck, "DisableSenderTrustCheck");
			Assert.AreEqual (ReceiverNameScope.Domain, lmr.NameScope, "NameScope");
			Assert.AreEqual (String.Empty, lmr.ReceiverName, "ReceiverName");

			lmr = new LocalMessageReceiver (InvalidNameComma);
			Assert.AreEqual (InvalidNameComma, lmr.ReceiverName, "ReceiverName/InvalidNameComma");

			lmr = new LocalMessageReceiver (InvalidNameColon);
			Assert.AreEqual (InvalidNameColon, lmr.ReceiverName, "ReceiverName/InvalidNameColon");
		}

		[TestMethod]
		public void Ctor_StringReceiverNameScopeIEnumerable ()
		{
			List<string> list = new List<string> ();
			Assert.Throws<ArgumentNullException> (delegate {
				new LocalMessageReceiver (null, ReceiverNameScope.Domain, list);
			}, "Name/null");
			Assert.Throws<ArgumentException> (delegate {
				new LocalMessageReceiver (TooLongName, ReceiverNameScope.Domain, list);
			}, "Name/TooLongName");
			Assert.Throws<ArgumentNullException> (delegate {
				new LocalMessageReceiver (String.Empty, ReceiverNameScope.Domain, null);
			}, "AllowedSenderDomains/null");
			Assert.Throws<ArgumentNullException> (delegate {
				string[] null_entry = new string[] { "a", null, "b" };
				new LocalMessageReceiver (LongName, ReceiverNameScope.Domain, null_entry);
			}, "AllowedSenderDomains/null-entry");
			Assert.Throws<ArgumentException> (delegate {
				string [] bad_entry = new string [] { "a", InvalidNameComma, "b" };
				new LocalMessageReceiver (LongName, ReceiverNameScope.Domain, bad_entry);
			}, "AllowedSenderDomains/bad-entry/comma");
			Assert.Throws<ArgumentException> (delegate {
				string [] bad_entry = new string [] { "a", InvalidNameColon, "b" };
				new LocalMessageReceiver (LongName, ReceiverNameScope.Domain, bad_entry);
			}, "AllowedSenderDomains/bad-entry/colon");

			LocalMessageReceiver lmr = new LocalMessageReceiver (String.Empty, (ReceiverNameScope) Int32.MinValue, list);
			Assert.AreEqual ((ReceiverNameScope) Int32.MinValue, lmr.NameScope, "NameScope/Bad");

			lmr = new LocalMessageReceiver ("name", ReceiverNameScope.Global, list);
			Assert.IsFalse (Object.ReferenceEquals (list, lmr.AllowedSenderDomains), "AllowedSenderDomains");
			Assert.AreEqual (typeof (ReadOnlyCollection<string>), lmr.AllowedSenderDomains.GetType (), "AllowedSenderDomains/Type");

			Assert.IsFalse (lmr.DisableSenderTrustCheck, "DisableSenderTrustCheck");
			Assert.AreEqual (ReceiverNameScope.Global, lmr.NameScope, "NameScope");
			Assert.AreEqual ("name", lmr.ReceiverName, "ReceiverName");

			lmr = new LocalMessageReceiver (LongName, ReceiverNameScope.Domain, LocalMessageReceiver.AnyDomain);
			Assert.IsNotNull (lmr.AllowedSenderDomains, "AllowedSenderDomains/AnyDomain");
			Assert.AreEqual (LongName, lmr.ReceiverName, "ReceiverName/LongName");
		}

		[TestMethod]
		public void AllowedSenderDomains_Immutable ()
		{
			string [] list = new string [] { "xxx" };
			LocalMessageReceiver lmr = new LocalMessageReceiver (String.Empty, ReceiverNameScope.Domain, list);
			list [0] = "yyy"; // original list can be modified but it won't affect the LocalMessageReceiver
			CheckForSingleValue (lmr.AllowedSenderDomains, "xxx");
		}

		[TestMethod]
		public void DisableSenderTrustCheck ()
		{
			LocalMessageReceiver lmr = new LocalMessageReceiver ("x");
			Assert.IsFalse (lmr.DisableSenderTrustCheck, "DisableSenderTrustCheck/Default");

			lmr.DisableSenderTrustCheck = true;
			Assert.IsTrue (lmr.DisableSenderTrustCheck, "DisableSenderTrustCheck/Change");

			lmr.Listen ();
			Assert.Throws<InvalidOperationException> (delegate {
				// throw, even if setting the actual value
				lmr.DisableSenderTrustCheck = true;
			}, "Listen/InvalidOperationException");

			lmr.Dispose ();
			Assert.Throws<ObjectDisposedException> (delegate {
				// throw, even if setting the actual value
				lmr.DisableSenderTrustCheck = true;
			}, "Dispose/ObjectDisposedException");

			// other properties are still available after dispose
			Assert.IsNull (lmr.AllowedSenderDomains, "AllowedSenderDomains");
			Assert.AreEqual (ReceiverNameScope.Domain, lmr.NameScope, "NameScope");
			Assert.AreEqual ("x", lmr.ReceiverName, "ReceiverName");
		}

		[TestMethod]
		[MoonlightBug ("call to Listen fails and throws (empty name) while this works on SL")]
		public void Listen_Empty ()
		{
			LocalMessageReceiver lmr = new LocalMessageReceiver (String.Empty);
			lmr.Listen ();
		}
	}
}

