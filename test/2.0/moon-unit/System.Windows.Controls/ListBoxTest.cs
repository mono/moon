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
using Microsoft.Silverlight.Testing;
using MoonTest.System.Windows.Controls.Primitives;

namespace MoonTest.System.Windows.Controls {
	public class ListBoxPoker : ListBox, IPoker
	{
		public DependencyObject ContainerItem { get; set; }

		public DependencyObject LastClearedContainer {
			get; set;
		}

		public DependencyObject LastCreatedContainer {
			get;set;
		}

		public DependencyObject LastPreparedContainer {
			get; set;
		}

		public object LastPreparedItem {
			get; set;
		}

		public bool TemplateApplied
		{
			get;
			private set;
		}

		public void ClearContainerForItemOverride_ (DependencyObject element, object item)
		{
			ClearContainerForItemOverride (element, item);
		}

		protected override void ClearContainerForItemOverride (DependencyObject element, object item)
		{
			LastClearedContainer = element;
			base.ClearContainerForItemOverride (element, item);
		}

		public DependencyObject GetContainerForItemOverride_ ()
		{
			return GetContainerForItemOverride ();
		}

		protected override DependencyObject GetContainerForItemOverride ()
		{
			LastCreatedContainer = ContainerItem ?? base.GetContainerForItemOverride ();
			return LastCreatedContainer;
		}

		public DependencyObject GetTemplateChild (string name)
		{
			return base.GetTemplateChild (name);
		}

		public bool IsItemItsOwnContainerOverride_ (object item)
		{
			return IsItemItsOwnContainerOverride (item);
		}

		public override void OnApplyTemplate ()
		{
			TemplateApplied = true;
			base.OnApplyTemplate ();
		}

		public void PrepareContainerForItemOverride_ (DependencyObject element, object item)
		{
			PrepareContainerForItemOverride (element, item);
		}

		protected override void PrepareContainerForItemOverride (DependencyObject element, object item)
		{
			LastPreparedContainer = element;
			LastPreparedItem = item;
			base.PrepareContainerForItemOverride (element, item);
		}
	}

	[TestClass]
	public partial class ListBoxTest : SelectorTest
	{
		protected override IPoker CreateControl ()
		{
			return new ListBoxPoker ();
		}
		protected override object CreateContainer ()
		{
			return new ListBoxItem ();
		}

		[Asynchronous]
		public override void ContainerItemTest2 ()
		{
			base.ContainerItemTest2 ();
			IPoker c = CurrentControl;
			Enqueue (() => {
				Assert.IsInstanceOfType<ListBoxItem> (c.LastCreatedContainer, "#1");
				ListBoxItem lbi = (ListBoxItem) c.LastCreatedContainer;
				Assert.AreEqual (lbi.Content, c.LastPreparedItem, "#2");
				Assert.AreEqual (lbi.DataContext, c.LastPreparedItem, "#3");
			});
			EnqueueTestComplete ();
		}

		[Asynchronous]
		public override void DisableControlTest ()
		{
			ItemsControl c = (ItemsControl) CurrentControl;
			base.DisableControlTest ();
			Enqueue (() => {
				foreach (Control item in c.Items) {
					Assert.IsFalse (item.IsEnabled, "#1");
					Assert.IsFalse ((bool) item.GetValue (Control.IsEnabledProperty), "#2");
				}
			});
			EnqueueTestComplete ();
		}

		[Asynchronous]
		public override void DisplayMemberPathTest ()
		{
			base.DisplayMemberPathTest ();
			ItemsControl c = (ItemsControl) CurrentControl;
			Enqueue (() => {
				ListBoxItem item = (ListBoxItem) CurrentControl.LastCreatedContainer;
				Assert.IsNull (item.ContentTemplate, "#template");
			});
			EnqueueTestComplete ();
		}

		public override void GetContainerForItemOverride2 ()
		{
			base.GetContainerForItemOverride2 ();
			Assert.IsInstanceOfType<ListBoxItem> (CurrentControl.LastCreatedContainer, "#1");
			ListBoxItem c = (ListBoxItem) CurrentControl.LastCreatedContainer;
			Assert.IsNull (c.Style, "null style");
			Assert.IsFalse (c.IsSelected, "Selected");
			Assert.IsNull (c.Content, "content is null");
		}

		[TestMethod]
		public override void GetContainerForItemOverride3 ()
		{
			base.GetContainerForItemOverride3 ();
			Assert.IsNotNull (((ListBoxItem) CurrentControl.LastCreatedContainer).Style, "#1");
		}

		[Asynchronous]
		public override void GetContainerForItemOverride10 ()
		{
			base.GetContainerForItemOverride10 ();
			Enqueue (() => CurrentControl.Items.Add ("Test"));
			EnqueueTestComplete ();
		}

		[Asynchronous]
		[Ignore ("This should throw an InvalidOperationException but SL throws a WrappedException containing the InvalidOperationException")]
		public override void GetInvalidContainerItemTest ()
		{
			base.GetInvalidContainerItemTest ();

			Enqueue (() => {
				try {
					CurrentControl.Items.Add ("New Item");
					Assert.Fail ("An exception should be thrown");
				} catch (Exception ex) {
					// Have to return a UIElement subclass, should throw InvalidOperationException i think
					Assert.AreEqual ("WrappedException", ex.GetType ().Name, "Exception type");
				}
			});
			EnqueueTestComplete ();
		}

		[MoonlightBug]
		public override void IsSelectedTest ()
		{
			base.IsSelectedTest ();
		}

		[Asynchronous]
		public override void IsSelectedTest4 ()
		{
			base.IsSelectedTest4 ();
			IPoker box = CurrentControl;
			Enqueue (() => {
				Assert.AreEqual (-1, box.SelectedIndex);
				Assert.IsNull (box.SelectedItem);
			});
			EnqueueTestComplete ();
		}

		public override void IsItemItsOwnContainerTest ()
		{
			base.IsItemItsOwnContainerTest ();
			Assert.IsTrue (CurrentControl.IsItemItsOwnContainerOverride_ (new ComboBoxItem ()));
			Assert.IsTrue (CurrentControl.IsItemItsOwnContainerOverride_ (new ListBoxItem ()));
		}

		[Asynchronous]
		public override void ItemTemplateTest3 ()
		{
			base.ItemTemplateTest3 ();
			Enqueue (() => {
				CurrentControl.SelectedIndex = 0;
			});
			Enqueue (() => {
				ListBoxItem c = (ListBoxItem) CurrentControl.LastCreatedContainer;
				Assert.AreSame (c, CurrentControl.LastPreparedContainer, "#prepared");
				Assert.IsNull (CurrentControl.LastClearedContainer, "#cleared");
				Assert.IsNotNull (c.ContentTemplate, "#content");
			});
			EnqueueTestComplete ();
		}

		class ListBoxItemSubclass : ListBoxItem { }

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
		public void DefaultValues ()
		{
			ListBox lb = new ListBox ();
			Assert.IsNull (lb.ItemContainerStyle, "ItemContainerStyle = null");
			Assert.AreEqual (lb.ReadLocalValue (ListBox.ItemContainerStyleProperty), DependencyProperty.UnsetValue, "ItemContainerStyle = Unset");

			Assert.IsNull (lb.ItemTemplate, "ItemTemplate == null");
			Assert.AreEqual (ScrollViewer.GetHorizontalScrollBarVisibility (lb), ScrollBarVisibility.Auto, "Horizontal Scroll Vis"); // Fails in Silverlight 3
			Assert.AreEqual (ScrollViewer.GetVerticalScrollBarVisibility (lb), ScrollBarVisibility.Auto, "Vertical Scroll Vis");
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
		[Asynchronous]
		public void VisualTree ()
		{
			ListBoxPoker box = new ListBoxPoker ();
			box.ItemsSource = new int [] { 1, 2, 3 };

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
								new VisualNode<Grid> ("#g", (VisualNode []) null)
							)
						)
					)
				);
			});
		}

		[TestMethod]
		[MoonlightBug ("Arrange should not be calling measure, this causes the template to be incorrectly applied")]
		public void VisualTree2 ()
		{
			ListBoxPoker box = new ListBoxPoker ();
			box.ItemsSource = new int [] { 1, 2, 3 };

			Assert.IsFalse (box.TemplateApplied, "#1");
			Assert.VisualChildren (box, "#2"); // No VisualChildren

			// The presenter is attached after we measure
			box.Arrange (new Rect (0, 0, 100, 100));
			Assert.VisualChildren (box, "#3");
			Assert.IsFalse (box.TemplateApplied, "#4");
		}
	}
}

