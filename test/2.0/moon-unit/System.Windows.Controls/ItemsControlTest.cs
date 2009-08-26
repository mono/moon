//
// Unit tests for ItemsControl
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
using System.Collections.Specialized;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

using System.Windows.Shapes;
using System.Windows.Media;
using Microsoft.Silverlight.Testing;
using System.Windows.Controls.Primitives;
using System.Windows.Markup;

namespace MoonTest.System.Windows.Controls {

	public class ObjectCollection : Collection<object> { }

	public class ItemsControlPoker : ItemsControl
	{
		public int CountAfterChange {
			get; set;
		}

		public bool ReadonlyAfterChange {
			get; set;
		}

		public bool? IsOwnContainer {
			get; set;
		}

		public DependencyObject ContainerItem {
			get; set;
		}

		public DependencyObject LastContainer {
			get; set;
		}

		protected override void ClearContainerForItemOverride (DependencyObject element, object item)
		{
			base.ClearContainerForItemOverride (element, item);
		}

		public void ClearContainerForItemOverride_ (DependencyObject element, object item)
		{
			ClearContainerForItemOverride (element, item);
		}

		protected override DependencyObject GetContainerForItemOverride ()
		{
			if (ContainerItem != null)
				return (LastContainer = ContainerItem);

			return (LastContainer = base.GetContainerForItemOverride ());
		}

		public DependencyObject GetContainerForItemOverride_ ()
		{
			return GetContainerForItemOverride ();
		}

		public new DependencyObject GetTemplateChild (string name)
		{
			return base.GetTemplateChild (name);
		}

		protected override bool IsItemItsOwnContainerOverride (object item)
		{
			return IsOwnContainer.HasValue ? IsOwnContainer.Value : base.IsItemItsOwnContainerOverride (item);
		}

		public bool IsItemItsOwnContainerOverride_ (object item)
		{
			return IsItemItsOwnContainerOverride (item);
		}

		public void OnItemsChanged_ (NotifyCollectionChangedEventArgs e)
		{
			base.OnItemsChanged (e);
		}

		protected override void PrepareContainerForItemOverride (DependencyObject element, object item)
		{
			base.PrepareContainerForItemOverride (element, item);
		}

		public void PrepareContainerForItemOverride_ (DependencyObject element, object item)
		{
			PrepareContainerForItemOverride (element, item);
		}

		public int ItemAdded { get; private set; }
		public int ItemRemove { get; private set; }
		public int ItemReplace { get; private set; }
		public int ItemReset { get; private set; }
		public NotifyCollectionChangedEventArgs EventArgs { get; private set; }

		public void ResetCounter ()
		{
			ItemAdded = 0;
			ItemRemove = 0;
			ItemReplace = 0;
			ItemReset = 0;
		}

		protected override void OnItemsChanged (NotifyCollectionChangedEventArgs e)
		{
			CountAfterChange = Items.Count;
			ReadonlyAfterChange = Items.IsReadOnly;
			switch (e.Action) {
				case NotifyCollectionChangedAction.Add:
					ItemAdded++;
					break;
				case NotifyCollectionChangedAction.Remove:
					ItemRemove++;
					break;
				case NotifyCollectionChangedAction.Replace:
					ItemReplace++;
					break;
				case NotifyCollectionChangedAction.Reset:
					ItemReset++;
					break;
			}
			EventArgs = e;
			base.OnItemsChanged (e);
		}
		
		public void AssertCollectionChanges (int added, int removed, int replaced, int reset, string message)
		{
			Assert.AreEqual (added, ItemAdded, string.Format ("{0} - ItemAdded", message));
			Assert.AreEqual (removed, ItemRemove, string.Format ("{0} - ItemRemoved", message));
			Assert.AreEqual (replaced, ItemReplace, string.Format ("{0} - ItemReplace", message));
			Assert.AreEqual (reset, ItemReset, string.Format ("{0} - ItemReset", message));
		}
	}
	
	[TestClass]
	public partial class ItemsControlTest : SilverlightTest {

		[TestMethod]
		[Asynchronous]
		public void AfterRender ()
		{
			ItemsControl c = new ItemsControl ();
			ListBoxItem item = new ListBoxItem {
				Content = new Rectangle { Fill = new SolidColorBrush (Colors.Black), Width = 20, Height = 20 }
			};
			c.Items.Add (item);
			CreateAsyncTest (c, () => {
				ItemsPresenter itemsPresenter = (ItemsPresenter) VisualTreeHelper.GetChild (c, 0);
				Assert.IsNotNull (itemsPresenter, "#2");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (itemsPresenter), "#3");
				StackPanel stackPanel = (StackPanel) VisualTreeHelper.GetChild (itemsPresenter, 0);
				Assert.IsNotNull (stackPanel, "#4");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (stackPanel), "#5");
				Assert.AreEqual (item, VisualTreeHelper.GetChild (stackPanel, 0), "#6");


				Grid grid = (Grid) VisualTreeHelper.GetChild (item, 0);
				Assert.IsNotNull (grid, "#7");

				Assert.AreEqual (4, VisualTreeHelper.GetChildrenCount (grid), "#8");
				Assert.IsTrue (grid.Children [0] is Rectangle, "#10");
				Assert.IsTrue (grid.Children [1] is Rectangle, "#11");
				Assert.IsTrue (grid.Children [2] is ContentPresenter, "#12");
				Assert.IsTrue (grid.Children [3] is Rectangle, "#13");

				Assert.AreEqual (item.Content, VisualTreeHelper.GetChild (grid.Children[2], 0), "#14");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void AfterRender1 ()
		{
			ItemsControl c = new ItemsControl ();
			TestPanel.Children.Add (c);
			Enqueue (() => {
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (c), "#1"); // Fails in Silverlight 3
				ItemsPresenter itemsPresenter = (ItemsPresenter) VisualTreeHelper.GetChild (c, 0);
				Assert.IsNotNull (itemsPresenter, "#2");

				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (itemsPresenter), "#3");
				StackPanel stackPanel = (StackPanel) VisualTreeHelper.GetChild (itemsPresenter, 0);
				Assert.IsNotNull (stackPanel, "#4");

				Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (stackPanel), "#5");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void AfterRender2 ()
		{
			ItemsControl c = new ItemsControl ();
			ListBoxItem item = new ListBoxItem {
				Content = new Rectangle { Fill = new SolidColorBrush (Colors.Black), Width = 20, Height = 20 }
			};
			TestPanel.Children.Add (c);
			c.Items.Add (item);
			Enqueue (() => {
				ContentPresenter presenter = (ContentPresenter) VisualTreeHelper.GetParent ((Rectangle) item.Content);
				Assert.IsNotNull (presenter, "#1"); // Fails in Silverlight 3

				Grid grid = (Grid) VisualTreeHelper.GetParent (presenter);
				Assert.IsNotNull (grid, "#2");

				ListBoxItem gitem = (ListBoxItem) VisualTreeHelper.GetParent (grid);
				Assert.IsNotNull (gitem, "#3");

				StackPanel panel = (StackPanel) VisualTreeHelper.GetParent (gitem);
				Assert.IsNotNull (panel, "#4");

				ItemsPresenter itempresenter = (ItemsPresenter) VisualTreeHelper.GetParent (panel);
				Assert.IsNotNull (itempresenter, "#5");

				Assert.AreEqual (c, VisualTreeHelper.GetParent (itempresenter), "#6");
			});
			EnqueueTestComplete ();
		}
		
		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void ApplyTemplate ()
		{
			ItemsControl c = new ItemsControl ();
			Assert.VisualChildren (c, "#1");
			Assert.IsTrue (c.ApplyTemplate (), "#2");

			// A default ItemsPresenter is applied here
			Assert.VisualChildren (c, "#3",
				new VisualNode<ItemsPresenter> ("#a", (VisualNode []) null)
			);

			// There's no style, so we keep the presenter
			CreateAsyncTest (c, () => {
				Assert.VisualChildren (c, "#4",
					new VisualNode<ItemsPresenter> ("#b", (VisualNode []) null)
				);
			});
		}

		[TestMethod]
		public void DefaultValues ()
		{
			ItemsControl ic = new ItemsControl ();
			Assert.IsNull (ic.DisplayMemberPath, "DisplayMemberPath");
			Assert.IsNull (ic.ItemsPanel, "ItemsPanel");
			Assert.IsNull (ic.ItemsSource, "ItemsSource");
			Assert.IsNull (ic.ItemTemplate, "ItemTemplate");
			Assert.AreEqual (0, ic.Items.Count, "Items.Count");
		}

		[TestMethod]
		public void ClearContainerForItemOverride ()
		{
			ItemsControlPoker ic = new ItemsControlPoker ();
			ic.ClearContainerForItemOverride_ (null, null);
			ic.ClearContainerForItemOverride_ (null, new object ());
			ic.ClearContainerForItemOverride_ (ic, null);
		}

		[TestMethod]
		public void ClearContainerForItemOverride2 ()
		{
			ItemsControlPoker ic = new ItemsControlPoker ();
			ListBoxItem item = new ListBoxItem ();
			item.Content = new object ();
			item.ContentTemplate = new DataTemplate ();
			item.Style = new Style (typeof (ListBoxItem));
			ic.ClearContainerForItemOverride_ (item, item);
			Assert.IsNotNull (item.Content);
			Assert.IsNotNull (item.Style);
			Assert.IsNotNull (item.ContentTemplate);
			ic.ClearContainerForItemOverride_ (item, null);
		}
		
		[TestMethod]
		[Asynchronous]
		public void ContainerItemTest ()
		{
			ItemsControlPoker box = new ItemsControlPoker ();
			ListBoxItem item = new ListBoxItem ();
			box.ApplyTemplate ();
			CreateAsyncTest (box,
				() => box.Items.Add (item),
				() => Assert.IsNull (item.DataContext)
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ContainerItemTest2 ()
		{
			object item = new object ();
			ItemsControlPoker c = new ItemsControlPoker ();
			c.ApplyTemplate ();
			CreateAsyncTest (c, () => {
				c.Items.Add (item);
				Assert.IsInstanceOfType<ContentPresenter> (c.LastContainer, "#1");
				ContentPresenter lbi = (ContentPresenter) c.LastContainer;
				Assert.AreEqual (lbi.Content, item, "#2");
				Assert.AreEqual (lbi.DataContext, item, "#3");
			});
		}

		class ConceteElement : FrameworkElement { }
		
		[TestMethod]
		[Asynchronous]
		public void ContainerItemTest3 ()
		{
			ConceteElement item = new ConceteElement ();
			ItemsControlPoker c = new ItemsControlPoker ();
			c.ApplyTemplate ();
			CreateAsyncTest (c, () => {
				c.Items.Add (item);
				Assert.IsNull (c.LastContainer, "#1"); // No autogenerated container
				Assert.IsNull(item.DataContext, "#3");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ContainerItemTest4 ()
		{
			object item = new object ();
			ConceteElement container = new ConceteElement ();
			ItemsControlPoker c = new ItemsControlPoker ();
			c.ContainerItem = container;
			c.ApplyTemplate ();
			CreateAsyncTest (c, () => {
				c.Items.Add (item);
				Assert.AreEqual (container, c.LastContainer, "#1");
				Assert.AreEqual (container.DataContext, item, "#2");
				Assert.AreEqual (container.ReadLocalValue (FrameworkElement.DataContextProperty), item, "#3");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ContainerItemTest5 ()
		{
			// Force all elements to *not* be their own container
			ItemsControlPoker c = new ItemsControlPoker { IsOwnContainer = false };
			c.ApplyTemplate ();

			CreateAsyncTest (c, () => {
				ContentPresenter item;
				object content;

				content = new Rectangle ();
				c.Items.Add (content);
				Assert.IsInstanceOfType<ContentPresenter> (c.LastContainer, "#1");
				item = (ContentPresenter) c.LastContainer;
				Assert.AreEqual (content, item.Content, "#2");
				Assert.IsNull (item.DataContext, "#3"); // Fails in Silverlight 3
				c.LastContainer = null;

				content = "I'm a string";
				c.Items.Add (content);
				Assert.IsInstanceOfType<ContentPresenter> (c.LastContainer, "#4");
				item = (ContentPresenter) c.LastContainer;
				Assert.AreEqual (content, item.Content, "#5");
				Assert.AreEqual (content, item.DataContext, "#6");
			});
		}
		
		[TestMethod]
		public void IsItemItsOwnContainerOverride ()
		{
			ItemsControlPoker ic = new ItemsControlPoker ();
			Assert.IsFalse (ic.IsItemItsOwnContainerOverride_ (null), "null");
			Assert.IsFalse (ic.IsItemItsOwnContainerOverride_ (new OpenFileDialog ()), "OpenFileDialog");
			Assert.IsFalse (ic.IsItemItsOwnContainerOverride_ (ic.Items), "ItemCollection");
			Assert.IsFalse (ic.IsItemItsOwnContainerOverride_ (new RowDefinition ()), "RowDefinition");

			Assert.IsTrue (ic.IsItemItsOwnContainerOverride_ (ic), "self");
			Assert.IsTrue (ic.IsItemItsOwnContainerOverride_ (new Slider ()), "Slider");
			Assert.IsTrue (ic.IsItemItsOwnContainerOverride_ (new Border ()), "Border");
			Assert.IsTrue (ic.IsItemItsOwnContainerOverride_ (new Grid ()), "Grid");
			Assert.IsTrue (ic.IsItemItsOwnContainerOverride_ (new ContentPresenter ()), "ContentPresenter");
			Assert.IsTrue (ic.IsItemItsOwnContainerOverride_ (new ContentControl ()), "ContentControl");
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsPanelTemplateTest ()
		{
			ItemsControl c = (ItemsControl) XamlReader.Load (@"
<ItemsControl xmlns=""http://schemas.microsoft.com/client/2007"">
	<ItemsControl.ItemsPanel>
		<ItemsPanelTemplate>
			<Grid />
		</ItemsPanelTemplate>
	</ItemsControl.ItemsPanel>
</ItemsControl>
");
			CreateAsyncTest (c, () => {
				Assert.VisualChildren (c, "#2",
					new VisualNode<ItemsPresenter> ("#a",
						new VisualNode<Grid> ("#b")
					)
				);
			});
		}

		[TestMethod]
		public void OnItemsChanged_Null ()
		{
			ItemsControlPoker ic = new ItemsControlPoker ();
			ic.OnItemsChanged_ (null);
		}

		[TestMethod]
		public void OnItemsChanged_Reset ()
		{
			ItemsControlPoker c = new ItemsControlPoker ();
			c.Items.Add (new object ());
			c.OnItemsChanged_ (new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset));
			Assert.AreEqual (1, c.Items.Count, "#1");
		}
		
		[TestMethod]
		public void PrepareContainerForItemOverride ()
		{
			ItemsControlPoker ic = new ItemsControlPoker ();
			ic.PrepareContainerForItemOverride_ (null, new object ());
			ic.PrepareContainerForItemOverride_ (ic, null);
		}

		[TestMethod]
		public void GetContainerForItemOverride ()
		{
			ItemsControlPoker ic = new ItemsControlPoker ();
			ContentPresenter cp1 = (ContentPresenter) ic.GetContainerForItemOverride_ ();

			Assert.AreEqual (cp1.GetType(), typeof (ContentPresenter), "ContentPresenter type");
			Assert.IsNull (cp1.Content, "Content");
			Assert.IsNull (cp1.ContentTemplate, "ContentTemplate");

			ContentPresenter cp2 = (ContentPresenter) ic.GetContainerForItemOverride_ ();
			// a new instance is returned each time
			Assert.IsFalse (Object.ReferenceEquals (cp1, cp2), "ReferenceEquals");
		}

		[TestMethod]
		public void OnItemsChanged ()
		{
			ItemsControlPoker ic = new ItemsControlPoker ();
			Assert.AreEqual (0, ic.ItemAdded, "ItemAdded-0");

			ic.Items.Add ("string");
			Assert.AreEqual (1, ic.ItemAdded, "ItemAdded-1");
			Assert.AreEqual (NotifyCollectionChangedAction.Add, ic.EventArgs.Action, "Action-1");
			Assert.AreEqual ("string", ic.EventArgs.NewItems [0], "NewItems-1");
			Assert.AreEqual (0, ic.EventArgs.NewStartingIndex, "NewStartingIndex-1");
			Assert.IsNull (ic.EventArgs.OldItems, "OldItems-1");
			Assert.AreEqual (-1, ic.EventArgs.OldStartingIndex, "OldStartingIndex-1");

			ic.Items.Insert (0, this);
			Assert.AreEqual (2, ic.ItemAdded, "ItemAdded-2");
			Assert.AreEqual (NotifyCollectionChangedAction.Add, ic.EventArgs.Action, "Action-2");
			Assert.AreEqual (this, ic.EventArgs.NewItems [0], "NewItems-2");
			Assert.AreEqual (0, ic.EventArgs.NewStartingIndex, "NewStartingIndex-2");
			Assert.IsNull (ic.EventArgs.OldItems, "OldItems-2");
			Assert.AreEqual (-1, ic.EventArgs.OldStartingIndex, "OldStartingIndex-2");

			Assert.AreEqual (0, ic.ItemRemove, "ItemRemove");
			Assert.IsTrue (ic.Items.Remove ("string"), "Remove");
			Assert.AreEqual (1, ic.ItemRemove, "ItemRemove-3");
			Assert.AreEqual (NotifyCollectionChangedAction.Remove, ic.EventArgs.Action, "Action-3");
			Assert.IsNull (ic.EventArgs.NewItems, "NewItems-3");
			Assert.AreEqual (-1, ic.EventArgs.NewStartingIndex, "NewStartingIndex-3");
			Assert.AreEqual ("string", ic.EventArgs.OldItems [0], "OldItems-3");
			Assert.AreEqual (1, ic.EventArgs.OldStartingIndex, "OldStartingIndex-3");

			ic.Items.RemoveAt (0);
			Assert.AreEqual (2, ic.ItemRemove, "ItemRemove-4");
			Assert.AreEqual (NotifyCollectionChangedAction.Remove, ic.EventArgs.Action, "Action-4");
			Assert.IsNull (ic.EventArgs.NewItems, "NewItems-4");
			Assert.AreEqual (-1, ic.EventArgs.NewStartingIndex, "NewStartingIndex-4");
			Assert.AreEqual (this, ic.EventArgs.OldItems [0], "OldItems-4");
			Assert.AreEqual (0, ic.EventArgs.OldStartingIndex, "OldStartingIndex-4");

			ic.Items.Add ("string");
			Assert.AreEqual (3, ic.ItemAdded, "ItemAdded-5");

			Assert.AreEqual (0, ic.ItemReplace, "ItemReplace");
			ic.Items [0] = this;
			Assert.AreEqual (1, ic.ItemReplace, "ItemReplace-6");
			Assert.AreEqual (NotifyCollectionChangedAction.Replace, ic.EventArgs.Action, "Action-6");
			Assert.AreEqual (this, ic.EventArgs.NewItems [0], "NewItems-6");
			Assert.AreEqual (0, ic.EventArgs.NewStartingIndex, "NewStartingIndex-6");
			Assert.AreEqual ("string", ic.EventArgs.OldItems [0], "OldItems-6");
			Assert.AreEqual (-1, ic.EventArgs.OldStartingIndex, "OldStartingIndex-6");

			Assert.AreEqual (0, ic.ItemReset, "ItemReset");
			ic.Items.Clear ();
			Assert.AreEqual (1, ic.ItemReset, "ItemReset-7");
			Assert.AreEqual (NotifyCollectionChangedAction.Reset, ic.EventArgs.Action, "Action-7");
			Assert.IsNull (ic.EventArgs.NewItems, "NewItems-7");
			Assert.AreEqual (-1, ic.EventArgs.NewStartingIndex, "NewStartingIndex-7");
			Assert.IsNull (ic.EventArgs.OldItems, "OldItems-7");
			Assert.AreEqual (-1, ic.EventArgs.OldStartingIndex, "OldStartingIndex-7");
		}

		class FE : FrameworkElement { }
		[TestMethod]
		public void OwnContainerTest ()
		{
			ItemsControlPoker box = new ItemsControlPoker ();
			Assert.IsFalse (box.IsItemItsOwnContainerOverride_ (null), "#1");
			Assert.IsFalse (box.IsItemItsOwnContainerOverride_ (new object ()), "#2");
			Assert.IsTrue (box.IsItemItsOwnContainerOverride_ (new ListBoxItem ()), "#3");
			Assert.IsTrue (box.IsItemItsOwnContainerOverride_ (new ComboBoxItem ()), "#4");
			Assert.IsTrue (box.IsItemItsOwnContainerOverride_ (new FE ()), "#5");
		}
		
		[TestMethod]
		public void PrepareContainerForItemOverrideTest ()
		{
			ItemsControlPoker box = new ItemsControlPoker ();
			box.PrepareContainerForItemOverride_ (null, null);
		}

		[TestMethod]
		public void PrepareContainerForItemOverrideTest2 ()
		{
			ItemsControlPoker box = new ItemsControlPoker ();
			box.PrepareContainerForItemOverride_ (new Rectangle (), null);
		}

		[TestMethod]
		[MoonlightBug]
		public void PrepareContainerForItemOverrideTest3 ()
		{
			ItemsControlPoker box = new ItemsControlPoker ();
			ComboBoxItem item = new ComboBoxItem ();
			Assert.IsNull (item.Style, "#1");
			Assert.IsNull (item.Content, "#2");
			Assert.IsNull (item.ContentTemplate, "#3");
			box.PrepareContainerForItemOverride_ (item, null);
			Assert.IsNull (item.Style, "#4");
			Assert.IsNotNull (item.Content, "#5"); // What's this? A placeholder when using a null item? // Fails in Silverlight 3
			Assert.IsNotNull (item.ContentTemplate, "#6");
		}

		[TestMethod]
		public void PrepareContainerForItemOverrideTest4 ()
		{
			ItemsControlPoker box = new ItemsControlPoker ();
			ComboBoxItem item = new ComboBoxItem ();
			Assert.IsNull (item.Style);
			Assert.IsNull (item.Content);
			Assert.IsNull (item.ContentTemplate);

			box.PrepareContainerForItemOverride_ (item, item);

			Assert.IsNull (item.Content);
			Assert.IsNull (item.ContentTemplate);
		}

		[TestMethod]
		public void PrepareContainerForItemOverrideTest6 ()
		{
			Rectangle rect = new Rectangle ();
			ItemsControlPoker box = new ItemsControlPoker ();
			ComboBoxItem item = new ComboBoxItem ();
			Assert.IsNull (item.Content);
			box.PrepareContainerForItemOverride_ (item, rect);
			Assert.AreSame (item.Content, rect);
		}

		[TestMethod]
		public void PrepareContainerForItemOverrideTest7 ()
		{
			Rectangle rect = new Rectangle ();
			ItemsControlPoker box = new ItemsControlPoker ();
			box.Items.Add (rect);
			ComboBoxItem item = new ComboBoxItem ();
			Assert.Throws<InvalidOperationException> (() => box.PrepareContainerForItemOverride_ (item, rect));
		}
		
		[TestMethod]
		[MoonlightBug]
		public void PrepareContainerForItemOverrideTest8 ()
		{
			ItemsControlPoker box = new ItemsControlPoker ();
			ContentPresenter item = new ContentPresenter ();
			Assert.IsNull (item.Style, "#1");
			Assert.IsNull (item.Content, "#2");
			Assert.IsNull (item.ContentTemplate, "#3");
			box.PrepareContainerForItemOverride_ (item, null);
			Assert.IsNull (item.Style, "#4");
			Assert.IsNotNull (item.Content, "#5"); // What's this? A placeholder when using a null item? // Fails in Silverlight 3
			Assert.IsNotNull (item.ContentTemplate, "#6");
		}

		[TestMethod]
		public void PrepareContainerForItemOverrideTest9 ()
		{
			ItemsControlPoker box = new ItemsControlPoker ();
			ContentPresenter item = new ContentPresenter ();
			Assert.IsNull (item.Style);
			Assert.IsNull (item.Content);
			Assert.IsNull (item.ContentTemplate);

			box.PrepareContainerForItemOverride_ (item, item);

			Assert.IsNull (item.Content);
			Assert.IsNull (item.ContentTemplate);
		}

		[TestMethod]
		public void PrepareContainerForItemOverrideTest10 ()
		{
			Rectangle rect = new Rectangle ();
			ItemsControlPoker box = new ItemsControlPoker ();
			ContentPresenter item = new ContentPresenter ();
			Assert.IsNull (item.Content);
			box.PrepareContainerForItemOverride_ (item, rect);
			Assert.AreSame (item.Content, rect);
		}
		
		[TestMethod]
		public void ItemsSource ()
		{
			ItemsControl ic = new ItemsControl ();
			ic.Items.Add ("hi");
			Assert.AreEqual ("hi", ic.Items[0], "first item #1");
			Assert.IsFalse (ic.Items.IsReadOnly, "items IsReadOnly #1");

			Assert.Throws<InvalidOperationException>(delegate {
					ic.ItemsSource = new string[] { "hi", "there" };
				}, "assigning ItemsSource when Items is not empty");

			ic.Items.Clear ();
			ic.ItemsSource = new string[] { "hi", "there" };
			Assert.AreEqual (2, ic.Items.Count, "count after setting ItemsSource");
			Assert.AreEqual ("hi", ic.Items[0], "first item #2");
			Assert.IsTrue (ic.Items.IsReadOnly, "items IsReadOnly #2");
			Assert.Throws<InvalidOperationException>(delegate {
					ic.Items.Add ("hi");
				}, "adding element to Items when ItemsSource is in use");


			ic.ItemsSource = null;
			Assert.AreEqual (0, ic.Items.Count, "count after setting ItemsSource to null");
			Assert.IsFalse (ic.Items.IsReadOnly, "items IsReadOnly #3");
			ic.Items.Add ("hi");
		}
		
		[TestMethod]
		public void ItemsTest ()
		{
			ItemsControlPoker c = new ItemsControlPoker ();
			ItemCollection original = c.Items;
			string [] array = new string [] { "Test", "Test2", "Test3" }; ;
			c.ItemsSource = array;
			Assert.AreSame (original, c.Items, "#1");
			Assert.Throws<InvalidOperationException>(() => c.Items.Add ("1"));
			c.AssertCollectionChanges (0, 0, 0, 1, "#2");
			Assert.IsTrue (c.ReadonlyAfterChange, "#2b");
			Assert.AreEqual (3, c.CountAfterChange, "#2c");

			c.ResetCounter ();
			c.ItemsSource = array;
			c.AssertCollectionChanges (0, 0, 0, 0, "#3");

			c.ResetCounter ();
			c.ItemsSource = null;
			Assert.AreSame (original, c.Items, "#4");
			c.AssertCollectionChanges (0, 0, 0, 1, "#5");
			Assert.IsFalse (c.ReadonlyAfterChange, "#5b");
			Assert.AreEqual (0, c.CountAfterChange, "#5c");

			c.Items.Add ("2");
			c.AssertCollectionChanges (1, 0, 0, 1, "#6");
		}

		[TestMethod]
		public void ItemsSource_ObservableCollection ()
		{
			ItemsControl ic = new ItemsControl ();

			ObservableCollection<string> stringCollection = new ObservableCollection<string>();

			ic.ItemsSource = stringCollection;

			Assert.AreEqual (ic.Items.Count, 0, "1");

			stringCollection.Add ("hi");

			Assert.AreEqual (ic.Items.Count, 1, "2");

			Assert.AreEqual (ic.Items[0], "hi", "3");

			ic.DisplayMemberPath = "Length";

			// i would really love it if this was "2"...
			Assert.AreEqual (ic.Items[0], "hi", "4");

		}

		[TestMethod]
		[MoonlightBug ("due to our string->char*->string marshaling (i think), we end up with a different object")]
		public void ItemsSource_ObservableCollection_testReferenceEquals ()
		{
			ItemsControl ic = new ItemsControl ();

			ObservableCollection<string> stringCollection = new ObservableCollection<string>();

			ic.ItemsSource = stringCollection;

			string f = "foo";

			stringCollection.Add("foo");

			Assert.IsTrue (object.ReferenceEquals (ic.Items[0], f), "string is the same object");
		}

		[TestMethod]
		public void ItemsSourceFromXaml ()
		{
			ItemsControl c = (ItemsControl) XamlReader.Load (@"
<ItemsControl xmlns=""http://schemas.microsoft.com/client/2007""
			xmlns:clr=""clr-namespace:MoonTest.System.Windows.Controls;assembly=moon-unit"">
	<ItemsControl.ItemsSource>
		<clr:ObjectCollection>
			<Rectangle />
		</clr:ObjectCollection>
	</ItemsControl.ItemsSource>
</ItemsControl>");
			Assert.AreEqual (1, c.Items.Count);
		}
	}
}
