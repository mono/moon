//
// TabControl Unit Tests
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
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using Mono.Moonlight.UnitTesting;

namespace Mono.Moonlight
{
	[TestClass]
	public class TabControlTests : SilverlightTest
	{
		TabControl CreateTabControl ()
		{
			TabItem item;
			TabControl tab = new TabControl ();

			item = new TabItem ();
			item.Header = new Rectangle { Fill = new SolidColorBrush (Colors.Red) };
			item.Content = new Rectangle { Fill = new SolidColorBrush (Colors.Black) };
			tab.Items.Add (item);

			item = new TabItem ();
			item.Header = new Rectangle { Fill = new SolidColorBrush (Colors.Green) };
			item.Content = new Rectangle { Fill = new SolidColorBrush (Colors.Brown) };
			tab.Items.Add (item);

			item = new TabItem ();
			item.Header = new Rectangle { Fill = new SolidColorBrush (Colors.Blue) };
			item.Content = new Rectangle { Fill = new SolidColorBrush (Colors.Orange) };
			tab.Items.Add (item);

			return tab;
		}

		[TestMethod]
		public void SelectedIndex ()
		{
			TabItem item;
			TabControl tabcontrol = new TabControl ();

			Assert.AreEqual (null, tabcontrol.SelectedItem, "#1");
			Assert.AreEqual (null, tabcontrol.SelectedContent, "#2");
			Assert.AreEqual (-1, tabcontrol.SelectedIndex, "#3");

			item = new TabItem ();
			item.Header = new Rectangle { Width = 100, Height = 33, Fill = new SolidColorBrush (Colors.Red) };
			item.Content = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Black) };
			Assert.IsNull (item.Parent, "#4");
			tabcontrol.Items.Add (item);

			Assert.AreEqual (tabcontrol.Items [0], tabcontrol.SelectedItem, "#5");
			Assert.AreEqual (((TabItem) tabcontrol.Items [0]).Content, tabcontrol.SelectedContent, "#6");
			Assert.AreEqual (0, tabcontrol.SelectedIndex, "#7");
			Assert.AreSame (item.Parent, tabcontrol, "#8");

			item = new TabItem ();
			item.Header = new Rectangle { Width = 100, Height = 33, Fill = new SolidColorBrush (Colors.Green) };
			item.Content = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Brown) };
			tabcontrol.Items.Add (item);

			Assert.AreEqual (tabcontrol.Items [0], tabcontrol.SelectedItem, "#5b");
			Assert.AreEqual (((TabItem) tabcontrol.Items [0]).Content, tabcontrol.SelectedContent, "#6b");
			Assert.AreEqual (0, tabcontrol.SelectedIndex, "#7b");
			Assert.AreSame (item.Parent, tabcontrol, "#8b");

			item = new TabItem ();
			item.Header = new Rectangle { Width = 100, Height = 33, Fill = new SolidColorBrush (Colors.Blue) };
			item.Content = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Orange) };
			tabcontrol.Items.Add (item);
		}

		[TestMethod]  
		public void TestIndexChanging ()
		{
			// This is really just testing the Selector class.
			TabControl c = CreateTabControl ();
			Assert.AreEqual (0, c.SelectedIndex, "#1");
			Assert.AreEqual (c.Items[0], c.SelectedItem, "#2");

			c.SelectedItem = null;
			Assert.AreEqual (-1, c.SelectedIndex, "#3");
			Assert.AreEqual (null, c.SelectedItem, "#4");

			c.SelectedItem = c.Items[2];
			Assert.AreEqual (2, c.SelectedIndex, "#3");
			Assert.AreEqual (c.Items [2], c.SelectedItem, "#4");
		}
	}
}
