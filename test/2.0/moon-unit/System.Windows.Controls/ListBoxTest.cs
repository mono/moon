//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Media;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls {
	[TestClass]
	public partial class ListBoxTest {
		class ListBoxPoker : ListBox {
			public bool Call_IsItemItsOwnContainerOverride (object item)
			{
				return base.IsItemItsOwnContainerOverride (item);
			}

			public DependencyObject Call_GetContainerForItemOverride ()
			{
				return base.GetContainerForItemOverride ();
			}

			public void Call_PrepareContainerForItemOverride (DependencyObject element, object item)
			{
				base.PrepareContainerForItemOverride (element, item);
			}
		}
		

		class ListBoxItemSubclass : ListBoxItem {
		}

		[TestMethod]
		public void DefaultValues ()
		{
			ListBox lb = new ListBox();
			Assert.IsNull (lb.ItemContainerStyle, "ItemContainerStyle = null");
			Assert.AreEqual (lb.ReadLocalValue (ListBox.ItemContainerStyleProperty), DependencyProperty.UnsetValue, "ItemContainerStyle = Unset");

			Assert.IsNull (lb.ItemTemplate, "ItemTemplate == null");
		}

		[TestMethod]
		public void IsItemItsOwnContainer ()
		{
			ListBoxPoker poker = new ListBoxPoker ();

			Assert.IsTrue (poker.Call_IsItemItsOwnContainerOverride (new ListBoxItem ()), "listboxitem");
			Assert.IsTrue (poker.Call_IsItemItsOwnContainerOverride (new ListBoxItemSubclass ()), "listboxitem subclass");
			Assert.IsTrue (poker.Call_IsItemItsOwnContainerOverride (new ComboBoxItem ()), "comboboxitem");
			Assert.IsFalse (poker.Call_IsItemItsOwnContainerOverride (new ItemsControl ()), "itemscontrol");
			Assert.IsFalse (poker.Call_IsItemItsOwnContainerOverride ("hi"), "string");
		}

		[TestMethod]
		public void GetContainerForItem ()
		{
			ListBoxPoker poker = new ListBoxPoker ();

			DependencyObject container = poker.Call_GetContainerForItemOverride ();
			Assert.IsTrue (container is ListBoxItem, "container is listboxitem");
			Assert.IsNull (((ListBoxItem)container).Content, "content is null");
			Assert.IsFalse (((ListBoxItem)container).IsSelected, "!isselected");
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (container), "no children"); // its template hasn't been applied
		}

		[TestMethod]
		[MoonlightBug]
		public void PrepareContainerForItemOverride_defaults ()
		{
			ListBoxPoker poker = new ListBoxPoker ();

			ListBoxItem element = (ListBoxItem)poker.Call_GetContainerForItemOverride ();
			string item = "hi";

			poker.Call_PrepareContainerForItemOverride (element, item);

			Assert.AreEqual (element.Content, item, "string is content");
			Assert.IsNotNull (element.ContentTemplate, "content template is null");
			Assert.IsNull (element.Style, "style is null");
		}


		// XXX we need to add tests to check if the
		// style/template is transmitted to the item
		// in PrepareContainerForItemOverride.


		[TestMethod]
		public void PrepareContainerForItemOverride_IsSelected ()
		{
			ListBoxPoker poker = new ListBoxPoker ();

			ListBoxItem element = (ListBoxItem)poker.Call_GetContainerForItemOverride ();
			string item = "hi";

			element.IsSelected = true;

			poker.Call_PrepareContainerForItemOverride (element, item);

			Assert.IsNull (poker.SelectedItem, "selected item before it's been inserted");
			Assert.AreEqual (-1, poker.SelectedIndex, "-1 selected index");
		}

		[TestMethod]
		[MoonlightBug]
		public void PrepareContainerForItemOverride_DisplayMemberPath ()
		{
			ListBoxPoker poker = new ListBoxPoker ();

			ListBoxItem element = (ListBoxItem)poker.Call_GetContainerForItemOverride ();
			string item = "hi";

			poker.DisplayMemberPath = "length";

			poker.Call_PrepareContainerForItemOverride (element, item);

			Assert.AreEqual (element.ReadLocalValue (ContentControl.ContentProperty), item, "binding is unset");
		}
	}

}

