//
// Selector Unit Tests
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

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Shapes;
using Microsoft.Silverlight.Testing;
using System.Collections.Generic;
using System.Windows.Media.Animation;

namespace MoonTest.System.Windows.Controls.Primitives {

	public abstract class SelectorTest : ItemsControlTestBase {

		public override void ClearContainerForItemOverride ()
		{
			Assert.Throws<NullReferenceException> (() => {
				base.ClearContainerForItemOverride (); 
			});
		}

		public override void ClearContainerForItemOverride2 ()
		{
			Assert.Throws<NullReferenceException> (() => {
				base.ClearContainerForItemOverride2 ();
			});
		}

		public override void ClearContainerForItemOverride3 ()
		{
			Assert.Throws<InvalidCastException> (() => {
				base.ClearContainerForItemOverride3 ();
			});
		}

		public override void ClearContainerForItemOverride4 ()
		{
			Assert.Throws<InvalidCastException> (() => {
				base.ClearContainerForItemOverride4 ();
			});
		}

		[TestMethod]
		public virtual void ClearContainerForItemOverrideSelector ()
		{
			IPoker box = CurrentControl;

			ListBoxItem listItem = new ListBoxItem { Content = "Content", IsSelected = true };
			ComboBoxItem comboItem = new ComboBoxItem { Content = "Content", IsSelected = true };

			box.ClearContainerForItemOverride_ (listItem, null);
			box.ClearContainerForItemOverride_ (comboItem, null);

			Assert.IsNull (listItem.Content, "#3");
			Assert.IsNull (comboItem.Content, "#4");

			Assert.IsFalse (listItem.IsSelected, "#5"); // Fails in Silverlight 3
			Assert.IsFalse (comboItem.IsSelected, "#6");
		}

		[TestMethod]
		public virtual void ClearContainerForItemOverrideSelector2 ()
		{
			// Fails in Silverlight 3
			IPoker p = CurrentControl;
			ItemsControl ic = (ItemsControl) p;
			Style style = new Style (typeof (ListBoxItem));
			DependencyProperty prop = ic is ComboBox ? ComboBox.ItemContainerStyleProperty : ListBox.ItemContainerStyleProperty;
			ic.SetValue (prop, style);

			ListBoxItem item = (ListBoxItem) CreateContainer ();
			item.Content = new object ();
			item.ContentTemplate = new DataTemplate ();
			item.Style = style;
			p.ClearContainerForItemOverride_ (item, item);
			Assert.IsNull (item.Content);
			Assert.IsNotNull (item.Style);
			Assert.IsNotNull (item.ContentTemplate);
			p.ClearContainerForItemOverride_ (item, null);
		}

		[TestMethod]
		[Asynchronous]
		public virtual void ChangeContainerStyle ()
		{
			
			Selector selector = (Selector) CurrentControl;
			Style first = new Style (typeof (ListBoxItem));
			Style second = new Style (typeof (ListBoxItem));

			ListBoxItem item = (ListBoxItem) CreateContainer ();
			item.Content = "A";

			CreateAsyncTest (selector,
				() => CurrentControl.ApplyTemplate (),
				() => {
					if (CurrentControl is ComboBox)
						((ComboBox)CurrentControl).IsDropDownOpen = true;
				},
				() => CurrentControl.Items.Add (item),
				() => CurrentControl.ItemContainerStyle = first,
				() => Assert.AreEqual (first, item.Style, "#2"),
				() => Assert.Throws<Exception> (() => CurrentControl.ItemContainerStyle = second, "#3"),
				() => Assert.AreEqual (first, item.Style, "#4")
			);
		}

		[Asynchronous]
		public override void DisplayMemberPathTest2 ()
		{
			base.DisplayMemberPathTest2 ();
			Enqueue (() => { if (CurrentControl is ComboBox)((ComboBox) CurrentControl).IsDropDownOpen = true; });
			Enqueue (() => {
				var p = (ListBoxItem) CurrentControl.LastCreatedContainer;
				Assert.AreEqual (CurrentControl.LastPreparedItem, p.Content, "#content is item");
				Assert.AreEqual (p.Content, p.ReadLocalValue (ListBoxItem.ContentProperty), "#content is local");
				Assert.AreEqual (CurrentControl.LastPreparedItem, p.DataContext, "#datacontext is item");
				Assert.AreEqual (p.DataContext, p.ReadLocalValue (ListBoxItem.DataContextProperty), "#datacontext is local");
				Assert.IsNotNull (p.ContentTemplate, "#ContentTemplate has been set");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		public virtual void GetContainerForItemOverride3 ()
		{
			// Subclass must do the testing of the created item
			// when there's a style
			Selector s = (Selector)CurrentControl;
			s.SetValue (s is ComboBox ? ComboBox.ItemContainerStyleProperty : ListBox.ItemContainerStyleProperty, new Style (typeof (ListBoxItem)));
			CurrentControl.GetContainerForItemOverride_ ();
		}

		[TestMethod]
		[Asynchronous]
		public virtual void GetInvalidContainerItemTest ()
		{
			IPoker box = CurrentControl;

			TestPanel.Children.Add ((FrameworkElement) box);
			Enqueue (() => box.ContainerItem = new Storyboard ());
			Enqueue (() => box.ApplyTemplate ());
		}

		[TestMethod]
		public virtual void IsSelectedTest ()
		{
			IPoker box = CurrentControl;
			foreach (ListBoxItem i in new ListBoxItem [] { new ListBoxItem (), new ComboBoxItem () }) {
				ListBoxItem item = i;

				box.Items.Add (item);
				box.SelectedItem = item;
				Assert.IsTrue (item.IsSelected, "#1");

				box.Items.Remove (item);
				Assert.IsTrue (item.IsSelected, "#2");

				box.Items.Add (item);
				box.SelectedItem = item;
				box.Items.Clear ();
				Assert.IsTrue (item.IsSelected, "#3");
			}
		}

		[TestMethod]
		public virtual void IsSelectedTest2 ()
		{
			// Add a ListBoxItem/ComboBoxItem which has IsSelected set to true
			foreach (var item in new ListBoxItem [] { new ListBoxItem (), new ComboBoxItem () }) {
				item.IsSelected = true;
				CurrentControl.Items.Add (item);
				Assert.AreEqual (0, CurrentControl.SelectedIndex, "#1");
				Assert.AreEqual (item, CurrentControl.SelectedItem, "#2");

				CurrentControl.Items.Clear ();
			}
		}

		[TestMethod]
		public virtual void IsSelectedTest3 ()
		{
			IPoker box = CurrentControl;
			object o = new object ();
			box.Items.Add (o);
			box.SelectedItem = o;
			box.Items.Clear ();
			Assert.IsNull (box.SelectedItem);
			Assert.AreEqual (-1, box.SelectedIndex);
		}

		[TestMethod]
		public virtual void IsSelectedTest4 ()
		{
			List<object> items = new List<object> () {
				new object (), new object (), new object ()
			};
			IPoker box = CurrentControl;
			TestPanel.Children.Add ((FrameworkElement) box);
			Enqueue (() => box.ApplyTemplate ());
			Enqueue (() => {
				items.ForEach (box.Items.Add);
				box.SelectedItem = items [2];
			});
			Enqueue (() => {
				Assert.AreEqual (2, box.SelectedIndex);
				Assert.AreEqual (items [2], box.SelectedItem);
				box.Items.Remove (box.Items [2]);
			});
		}

		[TestMethod]
		public virtual void IsSelectionActiveTest ()
		{
			IPoker p = CurrentControl;
			ListBoxItem item = new ListBoxItem ();
			Assert.IsFalse ((bool) item.GetValue (ListBox.IsSelectionActiveProperty), "#1");
			Assert.Throws<InvalidOperationException> (() => item.SetValue (ListBox.IsSelectionActiveProperty, true));
			Assert.IsFalse ((bool) item.GetValue (ListBox.IsSelectionActiveProperty), "#2");
			p.Items.Add (item);
			p.SelectedItem = item;
			Assert.IsFalse ((bool) item.GetValue (ListBox.IsSelectionActiveProperty), "#3");
			p.SelectedIndex = -1;
			Assert.IsFalse ((bool) item.GetValue (ListBox.IsSelectionActiveProperty), "#4");

			Assert.AreSame (ComboBox.IsSelectionActiveProperty, ListBox.IsSelectionActiveProperty, "#5");
		}

		[TestMethod]
		[Asynchronous]
		public virtual void IsSelectionActiveTest2 ()
		{
			IPoker p = CurrentControl;
			Selector selector = (Selector) p;
			selector.Items.Add (new object ());
			selector.Items.Add (new object ());
			selector.ApplyTemplate ();

			CreateAsyncTest (selector, () => {
				Assert.IsFalse ((bool) selector.GetValue (ListBox.IsSelectionActiveProperty), "#1");
				bool b = selector.Focus ();
				Assert.IsFalse ((bool) selector.GetValue (ListBox.IsSelectionActiveProperty), "#2");
				selector.SelectedIndex = 0;
				Assert.IsFalse ((bool) selector.GetValue (ListBox.IsSelectionActiveProperty), "#2");
			});
		}

		public override void IsItemItsOwnContainerTest ()
		{
			base.IsItemItsOwnContainerTest ();
			Assert.IsFalse (CurrentControl.IsItemItsOwnContainerOverride_ (CurrentControl));
		}

		public override void PrepareContainerForItemOverrideTest ()
		{
			Assert.Throws<NullReferenceException> (() => {
				base.PrepareContainerForItemOverrideTest ();
			}, "#1");
		}

		public override void PrepareContainerForItemOverrideTest2 ()
		{
			Assert.Throws<InvalidCastException> (() => {
				base.PrepareContainerForItemOverrideTest2 ();
			});
		}

		public override void PrepareContainerForItemOverrideTest2b ()
		{
			Assert.Throws<InvalidCastException> (() => {
				base.PrepareContainerForItemOverrideTest2b ();
			});
		}

		public override void PrepareContainerForItemOverrideTest3c ()
		{
			Assert.Throws<InvalidCastException> (() => {
				base.PrepareContainerForItemOverrideTest3c ();
			});
		}

		public override void PrepareContainerForItemOverrideTest6 ()
		{
			Assert.Throws<InvalidCastException> (() => {
				base.PrepareContainerForItemOverrideTest6 ();
			});
		}


		[TestMethod]
		public virtual void PrepareContainerForItemOverrideSelector2 ()
		{
			Style style = new Style (typeof (ListBoxItem));
			Selector box = (Selector) CurrentControl;
			DependencyProperty prop = box is ComboBox ? ComboBox.ItemContainerStyleProperty : ListBox.ItemContainerStyleProperty;
			box.SetValue (prop, style);

			ComboBoxItem item = new ComboBoxItem ();
			Assert.IsNull (item.Style, "#1");

			CurrentControl.PrepareContainerForItemOverride_ (item, null);
			Assert.AreSame (item.Style, box.GetValue (prop), "#4");
		}

		[TestMethod]
		public virtual void PrepareContainerForItemOverrideSelector3 ()
		{
			Style style = new Style (typeof (ListBoxItem));
			Selector box = (Selector) CurrentControl;
			DependencyProperty prop = box is ComboBox ? ComboBox.ItemContainerStyleProperty : ListBox.ItemContainerStyleProperty;
			box.SetValue (prop, style);

			ListBoxItem item = new ListBoxItem ();
			Assert.IsNull (item.Style, "#1");

			CurrentControl.PrepareContainerForItemOverride_ (item, null);

			Assert.AreSame (item.Style, box.GetValue (prop), "#4");
		}

		[TestMethod]
		public virtual void PrepareContainerForItemOverrideSelector4 ()
		{
			IPoker box = CurrentControl;
			ListBoxItem item = (ListBoxItem) CreateContainer ();
			box.PrepareContainerForItemOverride_ (item, item);
			Assert.IsNull (item.Content);
		}

		[TestMethod]
		public virtual void PrepareContainerForItemOverrideSelector5 ()
		{
			Rectangle rect = new Rectangle ();
			IPoker box = CurrentControl;
			ListBoxItem item = (ListBoxItem) CreateContainer ();
			Assert.IsNull (item.Content, "#1");
			box.PrepareContainerForItemOverride_ (item, rect);
			Assert.AreSame (item.Content, rect, "#2");
		}

		[TestMethod]
		public virtual void PrepareContainerForItemOverrideSelector6 ()
		{
			Rectangle rect = new Rectangle ();
			IPoker box = CurrentControl;
			box.Items.Add (rect);
			ListBoxItem item = (ListBoxItem) CreateContainer ();
			Assert.Throws<InvalidOperationException> (() => box.PrepareContainerForItemOverride_ (item, rect), "#2");
		}


		[TestMethod]
		public virtual void PrepareContainerForItemOverrideTest36 ()
		{
			Rectangle rect = new Rectangle ();
			IPoker box = CurrentControl;
			ComboBoxItem item = new ComboBoxItem ();
			Assert.IsNull (item.Content);
			box.PrepareContainerForItemOverride_ (item, rect);
			Assert.AreSame (item.Content, rect);
		}

		[TestMethod]
		public virtual void PrepareContainerForItemOverride_defaults ()
		{
			IPoker poker = CurrentControl;

			ListBoxItem element = (ListBoxItem) poker.GetContainerForItemOverride_ ();
			string item = "hi";

			poker.PrepareContainerForItemOverride_ (element, item);

			Assert.AreEqual (element.Content, item, "string is content");
			Assert.IsNotNull (element.ContentTemplate, "content template is null");
			Assert.IsNull (element.Style, "style is null");
		}

		[TestMethod]
		public virtual void PrepareContainerForItemOverride_IsSelected ()
		{
			IPoker poker = CurrentControl;

			ListBoxItem element = (ListBoxItem) poker.GetContainerForItemOverride_ ();
			string item = "hi";

			element.IsSelected = true;

			poker.PrepareContainerForItemOverride_ (element, item);

			Assert.IsNull (poker.SelectedItem, "selected item before it's been inserted");
			Assert.AreEqual (-1, poker.SelectedIndex, "-1 selected index");
		}

		[TestMethod]
		public virtual void PrepareContainerForItemOverride_DisplayMemberPath ()
		{
			IPoker poker = CurrentControl;

			ListBoxItem element = (ListBoxItem) poker.GetContainerForItemOverride_ ();
			string item = "hi";

			poker.DisplayMemberPath = "length";

			poker.PrepareContainerForItemOverride_ (element, item);

			Assert.AreEqual (element.ReadLocalValue (ContentControl.ContentProperty), item, "binding is unset");
		}

		[TestMethod]
		public virtual void StyleExistsOnContainer ()
		{
			ListBoxItem item = null;

			item = new ListBoxItem ();
			CurrentControl.PrepareContainerForItemOverride_ (item, new object ());
			Assert.IsUnset (item, ListBoxItem.StyleProperty, "#1");

			CurrentControl.ItemContainerStyle = null;
			item = new ListBoxItem ();
			CurrentControl.PrepareContainerForItemOverride_ (item, new object ());
			Assert.IsUnset (item, ListBoxItem.StyleProperty, "#2");

			CurrentControl.ItemContainerStyle = new Style (typeof (ListBoxItem));
			item = new ListBoxItem ();
			CurrentControl.PrepareContainerForItemOverride_ (item, new object ());
			Assert.AreEqual (CurrentControl.ItemContainerStyle, item.ReadLocalValue (ListBoxItem.StyleProperty), "#3");

			Style containerStyle = new Style (typeof (ListBoxItem));
			item = new ListBoxItem { Style = containerStyle };
			CurrentControl.PrepareContainerForItemOverride_ (item, new object ());
			Assert.AreEqual (containerStyle, item.ReadLocalValue (ListBoxItem.StyleProperty), "#4");
		}
	}
}
