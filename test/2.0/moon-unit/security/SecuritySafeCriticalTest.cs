//
// SecuritySafeCritical Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.ComponentModel;
using System.Reflection;
using System.Text;
using System.Threading;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	[TestClass]
	public partial class SecuritySafeCriticalTest {

		// misc tests to validate some behaviours around SSC code

		[TestMethod]
		public void Encoding_ ()
		{
			Assert.IsNotNull (Encoding.BigEndianUnicode, "BigEndianUnicode");
			Assert.IsNotNull (Encoding.Unicode, "Unicode");
			Assert.IsNotNull (Encoding.UTF8, "UTF8");

			// this can dynamically load code under "standard" FX

			Encoding e = Encoding.GetEncoding (Encoding.UTF8.WebName);
			Assert.AreSame (Encoding.UTF8, e, "!same");
		}

		[TestMethod]
		[MoonlightBug ("Moonlight provides some extra encoding")]
		public void Encoding_Extra_Moonlight ()
		{
			// X509Certificate support needs UTF7
			Assert.Throws<ArgumentException> (delegate {
				Encoding.GetEncoding ("utf-7");
			}, "utf-7");

			// SMCS requires UTF32 (e.g. to parse some DRT sources)
			Assert.Throws<ArgumentException> (delegate {
				Encoding.GetEncoding ("UTF-32");
			}, "UTF-32");
		}

		[TestMethod]
		public void SynchronizationContext_ ()
		{
			SynchronizationContext sc1 = SynchronizationContext.Current;
			Assert.IsNotNull (sc1, "SynchronizationContext.Current");
			SynchronizationContext sc2 = AsyncOperationManager.SynchronizationContext;
			Assert.IsNotNull (sc2, "AsyncOperationManager.SynchronizationContext");
			Assert.AreSame (sc1, sc2, "same");
		}

		[TestMethod]
		public void TimeZoneInfo_ ()
		{
			Assert.IsNotNull (TimeZoneInfo.Utc, "Utc");
			// this one requires file access (/etc/locatime and such) for ML
			Assert.IsNotNull (TimeZoneInfo.Local, "Local");
		}
	}
}

