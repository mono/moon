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

using System.Windows.Shapes;
using System.Windows.Media;
using Microsoft.Silverlight.Testing;
using System.Windows.Controls.Primitives;

namespace MoonTest.System.Windows.Controls {
	[TestClass]
	public partial class ListBoxTest : SilverlightTest {
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
			
			public DependencyObject GetTemplateChild (string name)
			{
				return base.GetTemplateChild (name);
			}
		}
		

		class ListBoxItemSubclass : ListBoxItem {
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void TemplateChild ()
		{
			ListBoxPoker box = new ListBoxPoker ();

			CreateAsyncTest (box,
				() => {
					DependencyObject o = box.GetTemplateChild ("ScrollViewer");
					Assert.IsNotNull (o);
				});
		}
		
		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void AfterRender ()
		{
			ListBox c = new ListBox ();
			ListBoxItem item = new ListBoxItem {
				Content = new Rectangle { Fill = new SolidColorBrush (Colors.Black), Width = 20, Height = 20 }
			};
			TestPanel.Children.Add (c);
			c.Items.Add (item);
			Enqueue (() => {
				Console.WriteLine ("Starting");
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (c), "#1");
				Console.WriteLine (VisualTreeHelper.GetChild (c, 0));
				Border b = (Border) VisualTreeHelper.GetChild (c, 0);
				Assert.IsNotNull (b, "#2");
				Console.WriteLine ("789");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (b), "#3");
				Console.WriteLine (VisualTreeHelper.GetChild (b, 0));
				ScrollViewer scroller = (ScrollViewer) VisualTreeHelper.GetChild (b, 0);
				Assert.IsNotNull (scroller, "#4");
				Console.WriteLine ("456");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (scroller), "#5");
				Console.WriteLine (VisualTreeHelper.GetChild (scroller, 0));
				Border border = (Border) VisualTreeHelper.GetChild (scroller, 0);
				Assert.IsNotNull (border, "#6");
				Console.WriteLine ("123");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (border), "#7");
				Console.WriteLine (VisualTreeHelper.GetChild (border, 0));
				Grid grid = (Grid) VisualTreeHelper.GetChild (border, 0);
				Assert.IsNotNull (grid, "#8");
				Console.WriteLine ("A");
				Assert.AreEqual (4, grid.Children.Count, "#9");
				Assert.IsTrue (grid.Children [0] is ScrollContentPresenter, "#10");
				Assert.IsTrue (grid.Children [1] is Rectangle, "#11");
				Assert.IsTrue (grid.Children [2] is ScrollBar, "#12");
				Assert.IsTrue (grid.Children [3] is ScrollBar, "#13");
				Assert.AreNotSame (grid.Children [1], item.Content, "#14");
				Console.WriteLine ("b");

				// The items 
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (grid.Children [0]), "#15");
				ItemsPresenter itemsPresenter = (ItemsPresenter) VisualTreeHelper.GetChild (grid.Children [0], 0);
				Assert.IsNotNull (itemsPresenter, "#16");
				Console.WriteLine ("c");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (itemsPresenter), "#a");
				StackPanel panel = (StackPanel) VisualTreeHelper.GetChild (itemsPresenter, 0);
				Assert.IsNotNull (panel, "#17");
				Console.WriteLine ("D");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (panel), "#b");
				ListBoxItem gitem = (ListBoxItem) VisualTreeHelper.GetChild (panel, 0);
				Assert.IsNotNull (gitem, "#18");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (gitem), "#c");
				Grid grid2 = (Grid) VisualTreeHelper.GetChild (gitem, 0);
				Assert.IsNotNull (grid2, "#19");

				Assert.AreEqual (4, VisualTreeHelper.GetChildrenCount (grid2), "#d");
				Assert.AreEqual (4, grid2.Children.Count, "#20");
				Assert.IsTrue (VisualTreeHelper.GetChild (grid2, 0) is Rectangle, "#21");
				Assert.IsTrue (VisualTreeHelper.GetChild (grid2, 1) is Rectangle, "#22");
				Assert.IsTrue (VisualTreeHelper.GetChild (grid2, 2) is ContentPresenter, "#23");
				Assert.IsTrue (VisualTreeHelper.GetChild (grid2, 3) is Rectangle, "#24");

				ContentPresenter presenter = (ContentPresenter) VisualTreeHelper.GetChild (grid2, 2);
				Assert.IsNotNull (presenter, "#25");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (presenter), "#26");
				Assert.AreSame (VisualTreeHelper.GetChild (presenter, 0), item.Content, "#27");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void AfterRender2 ()
		{
			ListBox c = new ListBox ();
			ListBoxItem item = new ListBoxItem {
				Content = new Rectangle { Fill = new SolidColorBrush (Colors.Black), Width = 20, Height = 20 }
			};
			TestPanel.Children.Add (c);
			c.Items.Add (item);
			Enqueue (() => {

				ContentPresenter presenter = (ContentPresenter) VisualTreeHelper.GetParent ((Rectangle) item.Content);
//				Console.WriteLine ("Should have presenter");
				Assert.IsNotNull (presenter, "#1");

				Console.WriteLine (VisualTreeHelper.GetParent (presenter));
				Grid grid = (Grid) VisualTreeHelper.GetParent (presenter);
				Assert.IsNotNull (grid, "#2");

				Console.WriteLine (VisualTreeHelper.GetParent (grid));
				ListBoxItem gitem = (ListBoxItem) VisualTreeHelper.GetParent (grid);
				Assert.IsNotNull (gitem, "#3");

				Console.WriteLine (VisualTreeHelper.GetParent (gitem));
				StackPanel panel = (StackPanel) VisualTreeHelper.GetParent (gitem);
				Assert.IsNotNull (panel, "#4");

				Console.WriteLine (VisualTreeHelper.GetParent (panel));
				ItemsPresenter itempresenter = (ItemsPresenter) VisualTreeHelper.GetParent (panel);
				Assert.IsNotNull (itempresenter, "#5");

				Console.WriteLine (VisualTreeHelper.GetParent (itempresenter));
				ScrollContentPresenter scrollpresenter = (ScrollContentPresenter) VisualTreeHelper.GetParent (itempresenter);
				Assert.IsNotNull (scrollpresenter, "#6");

				Console.WriteLine (VisualTreeHelper.GetParent (scrollpresenter));
				Grid grid2 = (Grid) VisualTreeHelper.GetParent (scrollpresenter);
				Assert.IsNotNull (grid2, "#7");
				
				Console.WriteLine (VisualTreeHelper.GetParent (grid2));
				Border border = (Border) VisualTreeHelper.GetParent (grid2);
				Assert.IsNotNull (border, "#8");

				Console.WriteLine (VisualTreeHelper.GetParent (border));
				ScrollViewer viewer = (ScrollViewer) VisualTreeHelper.GetParent (border);
				Assert.IsNotNull (viewer, "#9");

				Console.WriteLine (VisualTreeHelper.GetParent (viewer));
				Border border2 = (Border) VisualTreeHelper.GetParent (viewer);
				Assert.IsNotNull (border2, "#10");

				Console.WriteLine (VisualTreeHelper.GetParent (border2));
				Assert.AreEqual (c, VisualTreeHelper.GetParent (border2), "#11");
			});
			EnqueueTestComplete ();
		}
		
		[TestMethod]
		[Asynchronous]
		public void AfterRender3 ()
		{
			ListBox c = new ListBox ();
			ListBoxItem item = new ListBoxItem { };
			TestPanel.Children.Add (c);
			c.Items.Add (item);
			Enqueue (() => {
				Console.WriteLine ("Starting");
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (c), "#1");
				Console.WriteLine (VisualTreeHelper.GetChild (c, 0));
				Border b = (Border) VisualTreeHelper.GetChild (c, 0);
				Assert.IsNotNull (b, "#2");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (b), "#3");
				Console.WriteLine (VisualTreeHelper.GetChild (b, 0));
				ScrollViewer scroller = (ScrollViewer) VisualTreeHelper.GetChild (b, 0);
				Assert.IsNotNull (scroller, "#4");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (scroller), "#5");
				Console.WriteLine (VisualTreeHelper.GetChild (scroller, 0));
				Border border = (Border) VisualTreeHelper.GetChild (scroller, 0);
				Assert.IsNotNull (border, "#6");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (border), "#7");
				Console.WriteLine (VisualTreeHelper.GetChild (border, 0));
				Grid grid = (Grid) VisualTreeHelper.GetChild (border, 0);
				Assert.IsNotNull (grid, "#8");

				Assert.AreEqual (4, grid.Children.Count, "#9");
				Assert.IsTrue (grid.Children [0] is ScrollContentPresenter, "#10");
				Assert.IsTrue (grid.Children [1] is Rectangle, "#11");
				Assert.IsTrue (grid.Children [2] is ScrollBar, "#12");
				Assert.IsTrue (grid.Children [3] is ScrollBar, "#13");
				Assert.AreNotSame (grid.Children [1], item.Content, "#14");

			});
			EnqueueTestComplete ();
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

