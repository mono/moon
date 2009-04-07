//
// Unit tests for ContentPresenter
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
using System.Windows;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Shapes;
using System.Windows.Media;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class ContentPresenterTest : SilverlightTest {

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void Test ()
		{
			bool loaded =false;
			Rectangle r = new Rectangle { Width = 10, Height = 10, Fill = new SolidColorBrush (Colors.Black) };
			ContentPresenter p = new ContentPresenter ();
			p.Content = r;
			p.Loaded += delegate { loaded = true; };

			TestPanel.Children.Add (p);
			EnqueueConditional (() => loaded);
			Enqueue (() => {
				Assert.IsNull (r.Parent, "#1");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[MoonlightBug]
		public void AlreadyInTree ()
		{
			Rectangle r = new Rectangle { Width = 10, Height = 10, Fill = new SolidColorBrush (Colors.Black) };
			TestPanel.Children.Add (r);
			Assert.Throws<ArgumentException>(() => new ContentPresenter ().Content = r);
		}

		[TestMethod]
		public void AlreadyInPresenter ()
		{
			Rectangle r = new Rectangle { Width = 10, Height = 10, Fill = new SolidColorBrush (Colors.Black) };
			ContentPresenter c = new ContentPresenter { Content = r };
			TestPanel.Children.Add (r);
		}

		[TestMethod]
		[Ignore("This test passes, but leads to infinite recursion in .NET when the presenter is rendered/arranged")]
		public void AlreadyInPresenter2 ()
		{
			Rectangle r = new Rectangle { Width = 10, Height = 10, Fill = new SolidColorBrush (Colors.Black) };
			ContentPresenter c = new ContentPresenter { Content = r };
			TestPanel.Children.Add (r);
			TestPanel.Children.Add (c);
		}

		[TestMethod]
		public void AlreadyInPresenter3 ()
		{
			Rectangle r = new Rectangle { Width = 10, Height = 10, Fill = new SolidColorBrush (Colors.Black) };
			ContentPresenter c = new ContentPresenter { Content = r };
			TestPanel.Children.Add (r);
			TestPanel.Children.Add (c);
			TestPanel.Children.Remove (r);
		}
	}
}
