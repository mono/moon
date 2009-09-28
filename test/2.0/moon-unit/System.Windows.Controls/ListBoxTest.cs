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
		public class ListBoxPoker : ListBox {
			
			public bool TemplateApplied {
				get; private set;
			}

			public bool Call_IsItemItsOwnContainerOverride (object item)
			{
				return base.IsItemItsOwnContainerOverride (item);
			}

			public void ClearContainerForItemOverride_ (DependencyObject element, object item)
			{
				ClearContainerForItemOverride (element, item);
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
			
			public override void OnApplyTemplate ()
			{
				TemplateApplied = true;
				base.OnApplyTemplate ();
			}
		}
		

		class ListBoxItemSubclass : ListBoxItem {
		}

		[TestMethod]
		[Asynchronous]
		[Ignore ("Loaded Corner Case - The template has already been applied under SL even though the rules say it shouldn't be")]
		public void TemplateChild ()
		{
			ListBoxPoker box = new ListBoxPoker ();
			
			CreateAsyncTest (box, () => {
				DependencyObject o = box.GetTemplateChild ("ScrollViewer");
				Assert.IsNotNull (o);
			});
		}
		
		[TestMethod]
		[Asynchronous]
		public void AfterRender ()
		{
			// Fails in Silverlight 3
			ListBox c = new ListBox ();
			ListBoxItem item = new ListBoxItem {
				Content = new Rectangle { Fill = new SolidColorBrush (Colors.Black), Width = 20, Height = 20 }
			};
			
			TestPanel.Children.Add (c);
			Enqueue (() => c.Items.Add (item));
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
		public void AfterRender2 ()
		{
			ListBox c = new ListBox ();
			ListBoxItem item = new ListBoxItem {
				Content = new Rectangle { Fill = new SolidColorBrush (Colors.Black), Width = 20, Height = 20 }
			};
			TestPanel.Children.Add (c);
			Enqueue (() => c.Items.Add (item));
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
				Assert.AreEqual (c, VisualTreeHelper.GetParent (border2), "#11"); // Fails in Silverlight 3
			});
			EnqueueTestComplete ();
		}
		
		[TestMethod]
		[Asynchronous]
		public void AfterRender3 ()
		{
			// Fails in Silverlight 3
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
		[MoonlightBug]
		public void ApplyTemplate ()
		{
			ListBoxPoker poker = new ListBoxPoker ();
			Assert.IsNull (poker.Template, "#1");
			Assert.IsTrue (poker.ApplyTemplate (), "#2");
			Assert.IsNull (poker.Template, "#3");
		}
		
		public void ClearContainerForItemOverride ()
		{
			ListBoxPoker ic = new ListBoxPoker ();
			ic.ClearContainerForItemOverride_ (null, null);
			ic.ClearContainerForItemOverride_ (null, new object ());
			ic.ClearContainerForItemOverride_ (ic, null);
		}

		[TestMethod]
		public void ClearContainerForItemOverride2 ()
		{
			// Fails in Silverlight 3
			ListBoxPoker ic = new ListBoxPoker ();
			ListBoxItem item = new ListBoxItem ();
			item.Content = new object ();
			item.ContentTemplate = new DataTemplate ();
			item.Style = new Style (typeof (ListBoxItem));
			ic.ClearContainerForItemOverride_ (item, item);
			Assert.IsNull (item.Content);
			Assert.IsNotNull (item.Style);
			Assert.IsNotNull (item.ContentTemplate);
			ic.ClearContainerForItemOverride_ (item, null);
		}

		[TestMethod]
		public void ClearContainerForItemOverride3 ()
		{
			ListBoxPoker box = new ListBoxPoker ();

			ListBoxItem listItem = new ListBoxItem { Content = "Content", IsSelected = true };
			ComboBoxItem comboItem = new ComboBoxItem { Content = "Content", IsSelected = true };

			Assert.Throws<NullReferenceException> (() => box.ClearContainerForItemOverride_ (null, null), "#1");
			Assert.Throws<InvalidCastException> (() => box.ClearContainerForItemOverride_ (new Rectangle (), null), "#2");

			box.ClearContainerForItemOverride_ (listItem, null);
			box.ClearContainerForItemOverride_ (comboItem, null);

			Assert.IsNull (listItem.Content, "#3");
			Assert.IsNull (comboItem.Content, "#4");

			Assert.IsFalse (listItem.IsSelected, "#5"); // Fails in Silverlight 3
			Assert.IsFalse (comboItem.IsSelected, "#6");
		}

		[TestMethod]
		public void DefaultValues ()
		{
			ListBox lb = new ListBox();
			Assert.IsNull (lb.ItemContainerStyle, "ItemContainerStyle = null");
			Assert.AreEqual (lb.ReadLocalValue (ListBox.ItemContainerStyleProperty), DependencyProperty.UnsetValue, "ItemContainerStyle = Unset");

			Assert.IsNull (lb.ItemTemplate, "ItemTemplate == null");
			Assert.AreEqual (ScrollViewer.GetHorizontalScrollBarVisibility (lb), ScrollBarVisibility.Auto, "Horizontal Scroll Vis"); // Fails in Silverlight 3
			Assert.AreEqual (ScrollViewer.GetVerticalScrollBarVisibility (lb), ScrollBarVisibility.Auto, "Vertical Scroll Vis");
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
		public void IsSelectionActiveTest ()
		{
			ListBox box = new ListBox ();
			ListBoxItem item = new ListBoxItem ();
			Assert.IsFalse ((bool)item.GetValue (ListBox.IsSelectionActiveProperty), "#1");
			Assert.Throws<InvalidOperationException> (() => item.SetValue (ListBox.IsSelectionActiveProperty, true));
			Assert.IsFalse ((bool) item.GetValue (ListBox.IsSelectionActiveProperty), "#2");
			box.Items.Add (item);
			box.SelectedItem = item;
			Assert.IsFalse ((bool) item.GetValue (ListBox.IsSelectionActiveProperty), "#3");
			box.SelectedIndex = -1;
			Assert.IsFalse ((bool) item.GetValue (ListBox.IsSelectionActiveProperty), "#4");
		}

		[TestMethod]
		[Asynchronous]
		public void IsSelectionActiveTest2 ()
		{
			ListBox box = new ListBox ();
			box.Items.Add (new object ());
			box.Items.Add (new object ());
			box.ApplyTemplate ();

			CreateAsyncTest (box, () => {
			     Assert.IsFalse ((bool) box.GetValue (ListBox.IsSelectionActiveProperty), "#1");
			     bool b = box.Focus ();
			     Assert.IsFalse ((bool) box.GetValue (ListBox.IsSelectionActiveProperty), "#2");
			     box.SelectedIndex = 0;
				 Assert.IsFalse ((bool) box.GetValue (ListBox.IsSelectionActiveProperty), "#2");
			 });
		}

		[TestMethod]
		[Asynchronous]
		public void Focusable ()
		{
			bool loaded = false;
			ListBoxItem item = new ListBoxItem { Content = "Hello World" };
			item.Loaded += delegate { loaded = true; };
			
			ListBox box = new ListBox ();
			box.Items.Add (item);
			TestPanel.Children.Add (box);
			
			EnqueueConditional (() => loaded);
			Enqueue (() => {
				Assert.IsTrue (item.Focus (), "#1");
				box.SelectedItem = item;
				box.SelectedItem = null;
				Assert.IsTrue (item.Focus (), "#2");
			});
			EnqueueTestComplete ();
		}
		
		[TestMethod]
		public void GetContainerForItem ()
		{
			ListBoxPoker poker = new ListBoxPoker ();

			DependencyObject container = poker.Call_GetContainerForItemOverride ();
			Assert.IsTrue (container is ListBoxItem, "container is listboxitem");
			Assert.IsNull (((ListBoxItem) container).Style, "null style");
			Assert.IsNull (((ListBoxItem)container).Content, "content is null");
			Assert.IsFalse (((ListBoxItem)container).IsSelected, "!isselected");
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (container), "no children"); // its template hasn't been applied

			poker.ItemContainerStyle = new Style (typeof (ListBoxItem));
			container = poker.Call_GetContainerForItemOverride ();
			Assert.AreEqual (poker.ItemContainerStyle, ((ListBoxItem) container).Style, "style applied");
		}

		[TestMethod]
		public void OwnContainerTest ()
		{
			ListBoxPoker box = new ListBoxPoker ();
			Assert.IsFalse (box.Call_IsItemItsOwnContainerOverride (null), "#1");
			Assert.IsFalse (box.Call_IsItemItsOwnContainerOverride (new object ()), "#2");
			Assert.IsTrue (box.Call_IsItemItsOwnContainerOverride (new ListBoxItem ()), "#3");
			Assert.IsTrue (box.Call_IsItemItsOwnContainerOverride (new ComboBoxItem ()), "#4");
			Assert.IsFalse (box.Call_IsItemItsOwnContainerOverride (new Rectangle ()), "#5");
		}
		
		[TestMethod]
		public void ParentTest ()
		{
			ListBox box = new ListBox ();
			Rectangle r = new Rectangle ();
			ListBoxItem item = new ListBoxItem { Content = r };
			Assert.IsNull (item.Parent, "#1");
			Assert.AreEqual (item, r.Parent, "#2");
			box.Items.Add (item);
			Assert.AreEqual (item, r.Parent, "#3");
			Assert.IsNotNull (item.Parent, "#4");
			box.SelectedItem = item;
			Assert.AreEqual (item, r.Parent, "#5");
			Assert.IsNotNull (item.Parent, "#6");
			box.SelectedItem = null;
			Assert.AreEqual (item, r.Parent, "#7");
			Assert.IsNotNull (item.Parent, "#8");
		}
		
		[TestMethod]
		public void PrepareContainerForItemOverrideTest ()
		{
			ListBoxPoker box = new ListBoxPoker ();
			Assert.Throws<NullReferenceException> (() => box.Call_PrepareContainerForItemOverride (null, null));
		}

		[TestMethod]
		public void PrepareContainerForItemOverrideTest2 ()
		{
			ListBoxPoker box = new ListBoxPoker ();
			Assert.Throws<InvalidCastException> (() => box.Call_PrepareContainerForItemOverride (new Rectangle (), null));
		}

		[TestMethod]
		[MoonlightBug]
		public void PrepareContainerForItemOverrideTest3 ()
		{
			ListBoxPoker box = new ListBoxPoker ();
			ComboBoxItem item = new ComboBoxItem ();
			Assert.IsNull (item.Style, "#1");
			Assert.IsNull (item.Content, "#2");
			Assert.IsNull (item.ContentTemplate, "#3");
			box.Call_PrepareContainerForItemOverride (item, null);
			Assert.IsNull (item.Style, "#4");
			Assert.IsNotNull(item.Content, "#5"); // What's this? A placeholder when using a null item? // Fails in Silverlight 3
			Assert.IsNotNull (item.ContentTemplate, "#6");
		}

		[TestMethod]
		public void PrepareContainerForItemOverrideTest4 ()
		{
			// Fails in Silverlight 3
			ListBoxPoker box = new ListBoxPoker { ItemContainerStyle = new Style (typeof (ListBoxItem)) };
			box.ItemContainerStyle.Setters.Add (new Setter { Property = Canvas.LeftProperty, Value = 10.5 });
			ComboBoxItem item = new ComboBoxItem ();
			Assert.IsNull (item.Style);
			Assert.IsNull (item.Content);
			Assert.IsNull (item.ContentTemplate);

			box.Call_PrepareContainerForItemOverride (item, item);

			Assert.AreSame (box.ItemContainerStyle, item.Style);
			Assert.IsNull (item.Content);
			Assert.IsNull (item.ContentTemplate);
		}

		[TestMethod]
		public void PrepareContainerForItemOverrideTest5 ()
		{
			ListBoxPoker box = new ListBoxPoker ();
			ComboBoxItem item = new ComboBoxItem ();
			box.Call_PrepareContainerForItemOverride (item, item);
			Assert.IsNull (item.Content);
		}

		[TestMethod]
		public void PrepareContainerForItemOverrideTest6 ()
		{
			Rectangle rect = new Rectangle ();
			ListBoxPoker box = new ListBoxPoker ();
			ComboBoxItem item = new ComboBoxItem ();
			Assert.IsNull (item.Content);
			box.Call_PrepareContainerForItemOverride (item, rect);
			Assert.AreSame (item.Content, rect);
		}

		[TestMethod]
		public void PrepareContainerForItemOverrideTest7 ()
		{
			Rectangle rect = new Rectangle ();
			ListBoxPoker box = new ListBoxPoker ();
			box.Items.Add (rect);
			ComboBoxItem item = new ComboBoxItem ();
			Assert.Throws<InvalidOperationException> (() => box.Call_PrepareContainerForItemOverride (item, rect));
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
		
		[TestMethod]
		[MoonlightBug]
		public void ReferenceDoesNotChangeTest ()
		{
			object str = "Hello";
			ListBox box = new ListBox ();
			box.Items.Add (str);
			box.SelectedItem = str;
			Assert.AreSame (str, box.SelectedItem, "#1");
			Assert.AreSame (str, box.Items [0], "#2");
		}
		
		[TestMethod]
		[MoonlightBug]
		[Asynchronous]
		public void VisualTree ()
		{
			ListBoxPoker box = new ListBoxPoker ();
			box.ItemsSource = new int [ ] { 1, 2, 3 };

			Assert.VisualChildren (box, "#2"); // No VisualChildren
			
			// The presenter is attached after we measure
			Assert.IsFalse (box.TemplateApplied, "#1");
			box.Measure (new Size (100, 100));
			Assert.IsTrue (box.TemplateApplied, "#2");

			// A standard ItemsPresenter attaches itself during Measure
			Assert.VisualChildren (box, "#3",
				new VisualNode<ItemsPresenter> ("#a",
					new VisualNode<StackPanel> ("#b",
						new VisualNode<ListBoxItem> ("#c1", (VisualNode []) null),
						new VisualNode<ListBoxItem> ("#c2", (VisualNode []) null),
						new VisualNode<ListBoxItem> ("#c3", (VisualNode []) null)
					)
				)
			);

			// The template from the attached Style has attached itself here.
			CreateAsyncTest (box, () => {
				Assert.VisualChildren (box, "#4",
					new VisualNode<Border> ("#d", // Fails in Silverlight 3
						new VisualNode<ScrollViewer> ("#e",
							new VisualNode<Border> ("#f",
								new VisualNode<Grid> ("#g", (VisualNode [ ]) null)
							)
						)
					)
				);
			});
		}

		[TestMethod]
		public void VisualTree2 ()
		{
			ListBoxPoker box = new ListBoxPoker ();
			box.ItemsSource = new int [ ] { 1, 2, 3 };

			Assert.IsFalse (box.TemplateApplied, "#1");
			Assert.VisualChildren (box, "#2"); // No VisualChildren

			// The presenter is attached after we measure
			box.Arrange (new Rect (0, 0, 100, 100));
			Assert.VisualChildren (box, "#3");
			Assert.IsFalse (box.TemplateApplied, "#4");
		}
	}
}

