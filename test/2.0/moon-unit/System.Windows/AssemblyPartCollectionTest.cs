//
// Unit tests for System.Windows.AssemblyPartCollection
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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
using System.Reflection;
using System.Windows;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public partial class AssemblyPartCollectionTest {

		[TestMethod]
		public void Add ()
		{
			int n = Deployment.Current.Parts.Count;

			AssemblyPartCollection apc = new AssemblyPartCollection ();
			Assert.Throws<ArgumentNullException> (delegate {
				apc.Add (null);
			}, "Add(null)");

			AssemblyPart ap1 = new AssemblyPart ();
			Assembly a1 = ap1.Load (AssemblyPartTest.GetLibraryStream ());
			Assert.AreEqual (String.Empty, ap1.Source, "Source-1");
			apc.Add (ap1);
			Assert.AreEqual (1, apc.Count, "Count-1");

			AssemblyPart ap2 = new AssemblyPart ();
			Assembly a2 = ap2.Load (AssemblyPartTest.GetLibraryStream ());
			Assert.AreEqual (String.Empty, ap2.Source, "Source-2");
			apc.Add (ap2);
			Assert.AreEqual (2, apc.Count, "Count-2");

			// no side effect on deployment
			Assert.AreEqual (n, Deployment.Current.Parts.Count, "Deployment.Current.Parts.Count");
		}

		[TestMethod]
		public void Deployment_Parts ()
		{
			bool moon_unit = false;
			bool ms_sl_testing = false;
			bool ms_vs_qt_ut_sl = false;
			bool sys_win_ctl = false;
			bool sys_xml_linq = false;
			foreach (AssemblyPart ap in Deployment.Current.Parts) {
				string s = ap.Source;
				switch (s) {
				case "moon-unit.dll":
					moon_unit = true;
					break;
				case "Microsoft.Silverlight.Testing.dll":
					ms_sl_testing = true;
					break;
				case "Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll":
					ms_vs_qt_ut_sl = true;
					break;
				case "System.Windows.Controls.dll":
					sys_win_ctl = true;
					break;
				case "System.Xml.Linq.dll":
					sys_xml_linq = true;
					break;
				default:
					Assert.IsTrue (s.Length > 0, "Source");
					break;
				}
			}
			Assert.IsTrue (moon_unit, "moon-unit.dll");
			Assert.IsTrue (ms_sl_testing, "Microsoft.Silverlight.Testing.dll");
			Assert.IsTrue (ms_vs_qt_ut_sl, "Microsoft.VisualStudio.QualityTools.UnitTesting.Silverlight.dll");
			Assert.IsTrue (sys_win_ctl, "System.Windows.Controls.dll");
			Assert.IsTrue (sys_xml_linq, "System.Xml.Linq.dll");
		}
	}
}

