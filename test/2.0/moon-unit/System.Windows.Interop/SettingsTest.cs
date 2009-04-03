//
// Unit tests for System.Windows.Interop.Settings
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008 Novell, Inc (http://www.novell.com)
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
using System.IO;
using System.Windows;
using System.Windows.Interop;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Interop {

	/* It's important this test runs first so it sets the fps to a higher value after checking
	 * for the default values, so things don't run so slowly */
	 
	[TestClass]
	public class _SettingsTest {

		void Check (Settings settings)
		{
			Assert.IsFalse (settings.EnableFrameRateCounter, "Host.Settings.EnableFrameRateCounter");
			Assert.IsTrue (settings.EnableHTMLAccess, "Host.Settings.EnableHTMLAccess");
			Assert.IsFalse (settings.EnableRedrawRegions, "Host.Settings.EnableRedrawRegions");
			if (Environment.OSVersion.Platform == PlatformID.MacOSX)
				Assert.AreEqual (400, settings.MaxFrameRate, "Host.Settings.MaxFrameRate");
			else
				Assert.AreEqual (60, settings.MaxFrameRate, "Host.Settings.MaxFrameRate");
			Assert.IsFalse (settings.Windowless, "Host.Settings.Windowless");
		}

		[TestMethod]
		public void New ()
		{
			Settings settings = new Settings ();
			Check (settings);
		}

		[TestMethod]
		public void Current ()
		{
			Settings settings = Application.Current.Host.Settings;
			Check (settings);
		}

		[TestMethod]
		[MoonlightBug]
		public void Updates ()
		{
			Settings settings = new Settings ();
			Assert.IsFalse (Application.Current.Host.Settings.EnableFrameRateCounter, "Host.Settings.EnableFrameRateCounter==false");
			try {
				settings.EnableFrameRateCounter = true;
				Assert.IsTrue (settings.EnableFrameRateCounter, "EnableFrameRateCounter==true");
				// any instance will change the same (plugin) data
				Assert.IsTrue (Application.Current.Host.Settings.EnableFrameRateCounter, "Host.Settings.EnableFrameRateCounter==true");
			}
			finally {
				settings.EnableFrameRateCounter = false;
			}
		}

		[TestMethod]
		public void MaxFrameRate ()
		{
			Settings settings = new Settings ();
			int max = settings.MaxFrameRate;
			try {
				settings.MaxFrameRate = 0;
				Assert.AreEqual (1, settings.MaxFrameRate, "Zero");
				settings.MaxFrameRate = Int32.MaxValue;
				Assert.AreEqual (Int32.MaxValue, settings.MaxFrameRate, "Max");
				settings.MaxFrameRate = Int32.MinValue;
				Assert.AreEqual (Int32.MinValue, settings.MaxFrameRate, "Min");
				Assert.AreEqual(new Settings().MaxFrameRate, settings.MaxFrameRate, "inherit settings");
			} finally {
				settings.MaxFrameRate = max;
			}

			// no validation
		}
		
		[TestMethod]
		public void ZeEnd () {
			Application.Current.Host.Settings.MaxFrameRate = 400;
		}
		
	}
}
