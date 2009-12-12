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
using System.Windows.Markup;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public partial class ContentPresenterTest : SilverlightTest {

		[TestMethod]
		[Asynchronous]
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
				Assert.AreEqual (1, TestPanel.Children.Count, "#1");
				Assert.AreEqual (p, TestPanel.Children [0], "#2");
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (TestPanel), "#3");
				Assert.AreEqual (p, VisualTreeHelper.GetChild (TestPanel, 0), "#4");
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (p), "#5");
				Assert.AreEqual (r, VisualTreeHelper.GetChild (p, 0), "#6");
				
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

		[TestMethod]
		[Asynchronous]
		public void DataContextTest ()
		{
			ContentPresenter c = (ContentPresenter) XamlReader.Load (@"
<ContentPresenter xmlns=""http://schemas.microsoft.com/client/2007"">
	<ContentPresenter.ContentTemplate>
		<DataTemplate>
			<Rectangle DataContext=""{Binding DataContext}"" />
		</DataTemplate>
	</ContentPresenter.ContentTemplate>
	<Rectangle />
</ContentPresenter>");
 			Assert.IsNull (c.DataContext);
			Assert.AreEqual (DependencyProperty.UnsetValue, c.ReadLocalValue (ContentPresenter.DataContextProperty), "unset 1");
			c.Content = new Rectangle ();
			Assert.IsNull (c.DataContext);
			Assert.AreEqual (DependencyProperty.UnsetValue, c.ReadLocalValue (ContentPresenter.DataContextProperty), "unset 2");

			CreateAsyncTest (c,
				() => {
					Assert.IsNull (c.DataContext);
					Assert.AreEqual (DependencyProperty.UnsetValue, c.ReadLocalValue (ContentPresenter.DataContextProperty), "unset 3");
				},
				() => c.Content = new Rectangle (),
				() => {
					Assert.AreEqual (DependencyProperty.UnsetValue, c.ReadLocalValue (ContentPresenter.DataContextProperty), "unset 4");
					Assert.IsNull (c.DataContext);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void DataContextTest2 ()
		{
			// Changing the content updates the datacontext
			ContentPresenter c = new ContentPresenter ();
			c.Content = new object ();
			CreateAsyncTest (c,
				() => {
					Assert.AreEqual (c.Content, c.DataContext, "#1");
					Assert.AreEqual (c.Content, c.ReadLocalValue (ContentPresenter.DataContextProperty), "#1b");
					c.Content = new object ();
				},
				() => {
					Assert.AreEqual (c.Content, c.DataContext, "#2");
					Assert.AreEqual (c.Content, c.ReadLocalValue (ContentPresenter.DataContextProperty), "#2b");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("DataContext changed notifications are asynchronous")]
		public void DataContextTest3 ()
		{
			// The property change notification should be asynchronous
			ContentPresenter c = new ContentPresenter ();
			c.Content = new object ();
			CreateAsyncTest (c,
				() => {
					c.Content = new object ();
					Assert.AreNotEqual (c.Content, c.DataContext, "#1");
				},
				() => Assert.AreEqual (c.Content, c.DataContext, "#2")
			);
		}

		[TestMethod]
		[Asynchronous]
		public void DataContextTest4 ()
		{
			// When the ContentPresenter is not in the tree, the DataContext does not get reset
			object o = new object ();
			ContentPresenter c = new ContentPresenter { DataContext = o };
			Enqueue (() => Assert.AreEqual(o, c.DataContext, "#1"));
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void DataContextTest5 ()
		{
			// When the ContentPresenter is in the tree, its DataContext gets set to ContentPresenter.Content
			// when it is loaded
			object o = new object ();
			ContentPresenter c = new ContentPresenter { DataContext = o };
			Assert.AreEqual (o, c.DataContext, "#1");
			CreateAsyncTest (c,
				() => {
					Assert.IsNull (c.DataContext, "#2");
					Assert.IsNull (c.ReadLocalValue (ContentPresenter.DataContextProperty), "#2b");
				},
				() => {
					c.DataContext = o;
					Assert.AreEqual (o, c.DataContext, "#3");
					Assert.AreEqual (o, c.ReadLocalValue (ContentPresenter.DataContextProperty), "#3b");
				},
				() => Assert.AreEqual (o, c.DataContext, "#4")
			);
		}

		[TestMethod]
		[Asynchronous]
		public void DataContextTest5b ()
		{
			// When the ContentPresenter is in the tree, its DataContext gets set to ContentPresenter.Content
			// when it is loaded
			object o = new object ();
			object content = new object ();
			ContentPresenter c = new ContentPresenter { Content = content };
			CreateAsyncTest (c,
				() => {
					Assert.AreEqual (content, c.DataContext, "#1");
					Assert.AreEqual (content, c.ReadLocalValue (FrameworkElement.DataContextProperty), "#2");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void DataContextTest6 ()
		{
			// When the ContentPresenter is in the tree, its DataContext is updated as expected
			object o = new object ();
			object o2 = new object();
			ContentPresenter c = new ContentPresenter { DataContext = o };
			Assert.AreEqual (o, c.DataContext, "#1");
			CreateAsyncTest (c,
				() => c.DataContext = o,
				() => {
					Assert.AreEqual (o, c.DataContext, "#3");
					TestPanel.DataContext = o2;
				},
				() => Assert.AreEqual (o, c.DataContext, "#4"),
				() => c.ClearValue (ContentPresenter.DataContextProperty),
				() => Assert.AreEqual (o2, c.DataContext, "#5")
			);
		}

		[TestMethod]
		[Asynchronous]
		public void DataContextTest7 ()
		{
			// A null content is copied to the datacontext too.
			ContentPresenter presenter = new ContentPresenter ();
			CreateAsyncTest (presenter,
				() => {
					presenter.Content = new object ();
				}, () => {
					Assert.AreEqual (presenter.Content, presenter.ReadLocalValue (ContentPresenter.DataContextProperty), "#1");
					presenter.Content = null;
				}, () => {
					Assert.AreEqual (null, presenter.ReadLocalValue (ContentPresenter.DataContextProperty), "#2");

				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void NullContent ()
		{
			bool loaded = false;
			ContentPresenter p = new ContentPresenter ();
			p.Loaded += delegate { loaded = true; };

			TestPanel.Children.Add (p);
			EnqueueConditional (() => loaded);
			Enqueue (() => {
				Assert.AreEqual (1, TestPanel.Children.Count, "#1");
				Assert.AreEqual (p, TestPanel.Children [0], "#2");
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (TestPanel), "#3");
				Assert.AreEqual (p, VisualTreeHelper.GetChild (TestPanel, 0), "#4");
				Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (p), "#5");

			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void ObjectContent ()
		{
			bool loaded = false;
			ContentPresenter p = new ContentPresenter ();
			p.Content = new object ();
			p.Loaded += delegate { loaded = true; };

			TestPanel.Children.Add (p);
			EnqueueConditional (() => loaded);
			Enqueue (() => {
				Assert.AreEqual (1, TestPanel.Children.Count, "#1");
				Assert.AreEqual (p, TestPanel.Children [0], "#2");
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (TestPanel), "#3");
				Assert.AreEqual (p, VisualTreeHelper.GetChild (TestPanel, 0), "#4");
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (p), "#5");
				Grid g = VisualTreeHelper.GetChild (p, 0) as Grid;
				Assert.IsNotNull (g, "#6");
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (g), "#7");
				TextBlock block = VisualTreeHelper.GetChild (g, 0) as TextBlock;
				Assert.IsNotNull (block, "#8");
				Assert.AreEqual ("System.Object", block.Text, "#9");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void StringContent ()
		{
			bool loaded = false;
			ContentPresenter p = new ContentPresenter ();
			p.Content = "text";
			p.Loaded += delegate { loaded = true; };

			TestPanel.Children.Add (p);
			EnqueueConditional (() => loaded);
			Enqueue (() => {
				Assert.AreEqual (1, TestPanel.Children.Count, "#1");
				Assert.AreEqual (p, TestPanel.Children [0], "#2");
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (TestPanel), "#3");
				Assert.AreEqual (p, VisualTreeHelper.GetChild (TestPanel, 0), "#4");
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (p), "#5");
				Grid g = VisualTreeHelper.GetChild (p, 0) as Grid;
				Assert.IsNotNull (g, "#6");
				Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (g), "#7");
				TextBlock block = VisualTreeHelper.GetChild (g, 0) as TextBlock;
				Assert.IsNotNull (block, "#8");
				Assert.AreEqual ("text", block.Text, "#9");
			});
			EnqueueTestComplete ();
		}
		
		[TestMethod]
		[Asynchronous]
		public void VisualTreeTest ()
		{
			ContentPresenter presenter = new ContentPresenter ();
			presenter.Content = new Rectangle ();

			Assert.VisualChildren (presenter, "#1"); // No visual children
			presenter.Measure (Size.Empty);
			Assert.VisualChildren (presenter, "#2",
				new VisualNode<Rectangle> ("#a", (VisualNode [])null)
			);

			CreateAsyncTest (presenter, () => {
				Assert.VisualChildren (presenter, "#3",
					new VisualNode<Rectangle> ("#b", (VisualNode [ ]) null)
				);
			});
		}
		
		[TestMethod]
		[Asynchronous]
		public void VisualTreeTest2 ()
		{
			ContentPresenter presenter = new ContentPresenter ();
			presenter.Content = "I'm a string";

			Assert.VisualChildren (presenter, "#1"); // No visual children
			presenter.Measure (Size.Empty);
			Assert.VisualChildren (presenter, "#2",
				new VisualNode<Grid> ("#a",
					new VisualNode<TextBlock> ("#b")
				)
			);

			CreateAsyncTest (presenter, () => {
				Assert.VisualChildren (presenter, "#3",
					new VisualNode<Grid> ("#c", 
						new VisualNode<TextBlock> ("#d")
					)
				);
			});
		}
		
		[TestMethod]
		[Asynchronous]
		public void VisualTreeTest3 ()
		{
			ContentPresenter presenter = new ContentPresenter ();
			CreateAsyncTest (presenter, () => {
				presenter.Content = new Rectangle ();

				Assert.VisualChildren (presenter, "#1"); // No visual children
				presenter.Measure (Size.Empty);
				Assert.VisualChildren (presenter, "#2",
					new VisualNode<Rectangle> ("#a")
				);

				// Changing content unsets the visual child until the next Measure pass
				presenter.Content = new Ellipse ();
				Assert.VisualChildren (presenter, "#3"); // No visual children

				presenter.Measure (Size.Empty);
				Assert.VisualChildren (presenter, "#4",
					new VisualNode<Ellipse> ("#b")
				);

				Assert.IsNull (presenter.DataContext, "#5");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void VisualTreeTest4 ()
		{
			ContentPresenter presenter = new ContentPresenter ();
			CreateAsyncTest (presenter, () => {
				presenter.Content = new Rectangle ();

				Assert.VisualChildren (presenter, "#1"); // No visual children
				presenter.Measure (Size.Empty);
				Assert.VisualChildren (presenter, "#2",
					new VisualNode<Rectangle> ("#a")
				);

				// Changing the template unsets the visual child until the next Measure pass
				presenter.ContentTemplate = new DataTemplate ();
				Assert.VisualChildren (presenter, "#3"); // No visual children

				presenter.Measure (Size.Empty);
				Assert.VisualChildren (presenter, "#4", // Fails in Silverlight 3
					new VisualNode<Rectangle> ("#b")
				);
				Assert.IsNull (presenter.DataContext, "#5");
			});
		}
	}
}
