//
// LocalMessageSender Unit Tests
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
	public partial class LocalMessageSenderTest {

		static string LongName = new String ('a', 256);
		static string TooLongName = new String ('z', 257);

		static string InvalidNameComma = "I,got,some,commas";
		static string InvalidNameColon = "I got one colon:";

		[TestMethod]
		public void Ctor_String ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				new LocalMessageSender (null);
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				new LocalMessageSender (TooLongName);
			}, "TooLongName");

			LocalMessageSender lms = new LocalMessageSender (String.Empty);
			Assert.IsNull (lms.ReceiverDomain, "ReceiverDomain");
			Assert.AreEqual (String.Empty, lms.ReceiverName, "ReceiverName");

			lms = new LocalMessageSender (InvalidNameComma);
			Assert.AreEqual (InvalidNameComma, lms.ReceiverName, "ReceiverName/InvalidNameComma");

			lms = new LocalMessageSender (InvalidNameColon);
			Assert.AreEqual (InvalidNameColon, lms.ReceiverName, "ReceiverName/InvalidNameColon");
		}

		[TestMethod]
		public void Ctor_StringString ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				new LocalMessageSender (null, String.Empty);
			}, "Name/null");
			Assert.Throws<ArgumentException> (delegate {
				new LocalMessageSender (TooLongName, String.Empty);
			}, "Name/TooLongName");
			Assert.Throws<ArgumentNullException> (delegate {
				new LocalMessageSender (String.Empty, null);
			}, "Domain/null");
			Assert.Throws<ArgumentException> (delegate {
				new LocalMessageSender (String.Empty, TooLongName);
			}, "Domain/TooLongName");
			Assert.Throws<ArgumentException> (delegate {
				new LocalMessageSender (String.Empty, InvalidNameComma);
			}, "Domain/InvalidNameComma");
			Assert.Throws<ArgumentException> (delegate {
				new LocalMessageSender (String.Empty, InvalidNameColon);
			}, "Domain/InvalidNameColon");

			LocalMessageSender lms = new LocalMessageSender (InvalidNameComma, LongName);
			Assert.AreEqual (LongName, lms.ReceiverDomain, "ReceiverDomain");
			Assert.AreEqual (InvalidNameComma, lms.ReceiverName, "ReceiverName/InvalidNameComma");

			lms = new LocalMessageSender (InvalidNameColon, LongName);
			Assert.AreEqual (LongName, lms.ReceiverDomain, "ReceiverDomain");
			Assert.AreEqual (InvalidNameColon, lms.ReceiverName, "ReceiverName/InvalidNameColon");
		}
	}
}

