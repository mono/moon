//
// UIElementCollection Unit Tests
//
// Author:
//   Moonlight Team (moonlight-list@lists.ximian.com)
// 
// Copyright 2009 Novell, Inc. (http://www.novell.com)
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
using System.Windows;
using System.Windows.Controls;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class UIElementCollectionTest {

		UIElementCollection GetUIElementCollection ()
		{
			return new Canvas ().Children;
		}

		[TestMethod]
		public void AddRemove ()
		{
			UIElementCollection uiec = GetUIElementCollection ();
			Assert.AreEqual (0, uiec.Count, "Count-0");

			Slider s = new Slider ();
			uiec.Add (s);
			Assert.AreEqual (1, uiec.Count, "Count-1");

			uiec.Remove (s);
			Assert.AreEqual (0, uiec.Count, "Count-2");
		}

		[TestMethod]
		public void AddTwice ()
		{
			UIElementCollection uiec = GetUIElementCollection ();
			Assert.AreEqual (0, uiec.Count, "Count-0");

			Slider s = new Slider ();
			uiec.Add (s);
			Assert.AreEqual (1, uiec.Count, "Count-1");

			// can't add twice
			Assert.Throws<InvalidOperationException> (delegate {
				uiec.Add (s);
			}, "Add twice");
			Assert.AreEqual (1, uiec.Count, "Count-2");
		}

		[TestMethod]
		public void AddSameName ()
		{
			UIElementCollection uiec = GetUIElementCollection ();
			Assert.AreEqual (0, uiec.Count, "Count-0");

			Slider s1 = new Slider ();
			s1.Name = "MySlider";
			uiec.Add (s1);
			Assert.AreEqual (1, uiec.Count, "Count-1");

			Slider s2 = new Slider ();
			s2.Name = "MySlider";
			Assert.IsTrue (uiec.Contains (s1), "Contains(s1)-1");
			Assert.IsFalse (uiec.Contains (s2), "Contains(s2)-1");

			uiec.Add (s2);
			Assert.AreEqual (2, uiec.Count, "Count-2");

			Assert.IsTrue (uiec.Contains (s1), "Contains(s1)-2");
			Assert.IsTrue (uiec.Contains (s2), "Contains(s2)-2");
		}

		[TestMethod]
		public void AddRemoveAdd ()
		{
			UIElementCollection uiec = GetUIElementCollection ();
			Assert.AreEqual (0, uiec.Count, "Count-0");

			Slider s = new Slider ();
			uiec.Add (s);
			Assert.AreEqual (1, uiec.Count, "Count-1");

			uiec.Remove (s);
			Assert.AreEqual (0, uiec.Count, "Count-2");

			uiec.Add (s);
			Assert.AreEqual (1, uiec.Count, "Count-3");
		}
	}
}
