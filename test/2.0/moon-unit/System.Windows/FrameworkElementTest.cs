//
// FrameworkElement Unit Tests
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
using System.Windows.Markup;
using System.Windows.Media;

using Mono.Moonlight.UnitTesting;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Windows.Shapes;
using System.Collections.Generic;

namespace MoonTest.System.Windows {

	
	[TestClass]
	public class FrameworkElementTest : SilverlightTest {

		class ConcreteFrameworkElement : FrameworkElement {

			public List<string> Methods = new List<string> ();
			public bool Templated, Arranged, Measured;
			public Size arrangeInput;

			public override void OnApplyTemplate ()
			{
				Templated = true;
				base.OnApplyTemplate ();
			}

			protected override Size ArrangeOverride (Size finalSize)
			{
				Methods.Add ("Arrange");
				Arranged = true;
				return base.ArrangeOverride (finalSize);
			}

			protected override Size MeasureOverride (Size availableSize)
			{
				Methods.Add ("Measure");
				Measured = true;
				return base.MeasureOverride (availableSize);
			}

			public Size ArrangeOverride_ (Size finalSize)
			{
				arrangeInput = finalSize;
				return base.ArrangeOverride (finalSize);
			}

			public Size MeasureOverride_ (Size availableSize)
			{
				return base.MeasureOverride (availableSize);
			}
		}

		[TestMethod]
		public void BrokenProperties ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			Assert.IsNull (fe.Cursor, "Cursor");
			Assert.IsTrue (Double.IsNaN (fe.Height), "Height");
			Assert.AreEqual (String.Empty, fe.Name, "Name");
			Assert.IsTrue (Double.IsNaN (fe.Width), "Width");
		}
		
		[TestMethod]
		[Asynchronous]
		public void ChangeContentChangesTemplate ()
		{
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <ContentControl.Template>
      <ControlTemplate>
        <Grid>
          <ContentPresenter />
        </Grid>
      </ControlTemplate>
    </ContentControl.Template>
</ContentControl>
");
			// Check what happens going from UIElement -> UIElement
			c.Content = new Rectangle ();
			c.ApplyTemplate ();
			CreateAsyncTest (c,
				() => Assert.VisualChildren (c, "#1",
					new VisualNode<Grid> ("#a",
						new VisualNode<ContentPresenter> ("#b",
							new VisualNode<Rectangle> ("#c")
						)
					)
				),
				() => {
					c.Content = new Rectangle ();
					Assert.VisualChildren (c, "#3"); // Fails in Silverlight 3
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ChangeContentChangesTemplate2 ()
		{
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <ContentControl.Template>
      <ControlTemplate>
        <Grid>
          <ContentPresenter />
        </Grid>
      </ControlTemplate>
    </ContentControl.Template>
</ContentControl>
");
			// Check what happens going from UIElement -> non-UIElement
			c.Content = new Rectangle ();
			c.ApplyTemplate ();
			CreateAsyncTest (c,
				() => Assert.VisualChildren (c, "#1",
					new VisualNode<Grid> ("#a",
						new VisualNode<ContentPresenter> ("#b",
							new VisualNode<Rectangle> ("#c")
						)
					)
				),
				() => {
					c.Content = "I'm a string";
					Assert.VisualChildren (c, "#3"); // Fails in Silverlight 3
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ChangeContentChangesTemplate3 ()
		{
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <ContentControl.Template>
      <ControlTemplate>
        <Grid>
          <ContentPresenter />
        </Grid>
      </ControlTemplate>
    </ContentControl.Template>
</ContentControl>
");
			// Check what happens going from non-UIElement -> UIElement 
			c.Content = "I'm a string";
			c.ApplyTemplate ();
			CreateAsyncTest (c,
				() => Assert.VisualChildren (c, "#1",
					new VisualNode<Grid> ("#a",
						new VisualNode<ContentPresenter> ("#b",
							new VisualNode<Grid> ("#c", (VisualNode []) null)
						)
					)
				),
				() => {
					c.Content = new Rectangle ();
					Assert.VisualChildren (c, "#3"); // Fails in Silverlight 3
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ChangeContentChangesTemplate4 ()
		{
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <ContentControl.Template>
      <ControlTemplate>
        <Grid>
          <ContentPresenter />
        </Grid>
      </ControlTemplate>
    </ContentControl.Template>
</ContentControl>
");
			// Check what happens going from non-UIElement -> non-UIElement
			c.Content = 5;
			c.ApplyTemplate ();
			DependencyObject childA = null;
			DependencyObject childB = null;
			TextBlock textA = null;
			TextBlock textB = null;
			CreateAsyncTest (c,
				() => Assert.VisualChildren (c, "#1",
					new VisualNode<Grid> ("#a",
						new VisualNode<ContentPresenter> ("#b",
							new VisualNode<Grid> ("#c", g => childA = g,
								new VisualNode<TextBlock> ("#d", t => textA = t)
							)
						)
					)
				),
				() => Assert.AreEqual ("5", textA.Text, "#2"),
				() => {
					c.Content = 8;
					Assert.VisualChildren (c, "#3",
						new VisualNode<Grid> ("#e",
							new VisualNode<ContentPresenter> ("#f",
								new VisualNode<Grid> ("#g", (grid) => childB = grid,
									new VisualNode<TextBlock> ("#h", t => textB = t)
								)
							)
						)
					);
				},
				() => Assert.AreSame (childA, childB, "#4"),
				() => Assert.AreSame (textA, textB, "#5"),
				() => Assert.AreEqual ("8", textA.Text, "#6")
			);
		}
		
		[TestMethod]
		[Asynchronous]
		public void ChangeContentChangesTemplate5 ()
		{
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <ContentControl.Template>
      <ControlTemplate>
        <Grid>
          <ContentPresenter />
        </Grid>
      </ControlTemplate>
    </ContentControl.Template>
</ContentControl>
");
			// Check what happens going from UIElement -> UIElement
			// Here the entire template is rebuilt
			c.Content = new Rectangle ();
			c.ApplyTemplate ();

			Grid gridA = null, gridB = null;
			ContentPresenter presenterA = null, presenterB = null;
			CreateAsyncTest (c,
				() => Assert.VisualChildren (c, "#1",
					new VisualNode<Grid> ("#a", g => gridA = g,
						new VisualNode<ContentPresenter> ("#b", p => presenterA = p,
							new VisualNode<Rectangle> ("#c")
						)
					)
				),
				() => {
					c.Content = new Rectangle ();
					Assert.VisualChildren (c, "#3"); // Fails in Silverlight 3
				},
				() => Assert.VisualChildren (c, "#1",
					new VisualNode<Grid> ("#a", g => gridB = g,
						new VisualNode<ContentPresenter> ("#b", p => presenterB = p,
							new VisualNode<Rectangle> ("#c")
						)
					)
				),
				() => {
					Assert.AreNotSame (gridA, gridB, "#4");
					Assert.AreNotSame (presenterA, presenterB, "#5");
				}
			);
		}

		[TestMethod]
		public void CursorTest ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			Assert.IsNull (c.Cursor, "#1");
			c.Cursor = global::System.Windows.Input.Cursors.Arrow;
			Assert.AreEqual (c.Cursor, global::System.Windows.Input.Cursors.Arrow, "#2");
			Assert.IsTrue (c.Cursor == global::System.Windows.Input.Cursors.Arrow, "#3");
			c.Cursor = null;
			Assert.AreEqual (null, c.Cursor, "#4");
		}

		[TestMethod]
		[Asynchronous]
		public void InvalidateArrange ()
		{
			bool loaded = false;
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			c.Loaded += delegate { loaded = true; };
			TestPanel.Children.Add (c);
			EnqueueConditional (() => loaded, "#1");
			EnqueueConditional (() => c.Measured, "#2");
			EnqueueConditional (() => c.Arranged, "#3");
			Enqueue (() => loaded = c.Arranged = c.Measured = c.Templated = false);
			Enqueue (() => c.InvalidateArrange ());
			EnqueueConditional (() => c.Arranged, "#4");

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void InvalidateMeasure ()
		{
			bool loaded = false;
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			c.Loaded += delegate { loaded = true; };
			TestPanel.Children.Add (c);
			EnqueueConditional (() => loaded, "#1");
			EnqueueConditional (() => c.Measured, "#2");
			EnqueueConditional (() => c.Arranged, "#3");
			Enqueue (() => loaded = c.Arranged = c.Measured = c.Templated = false);
			Enqueue (() => c.InvalidateMeasure ());
			EnqueueConditional (() => c.Measured, "#4");
			EnqueueConditional (() => c.Arranged, "#5");
			EnqueueTestComplete ();
		}
		
		[TestMethod]
		public void InvalidValues()
		{
			ConcreteFrameworkElement f = new ConcreteFrameworkElement ();
			Assert.Throws<ArgumentException>(delegate {
				f.Language = null;
			}, "#1"); // Fails in Silverlight 3 (got System.Exception)
			Assert.Throws<ArgumentException>(delegate {
				f.SetValue (FrameworkElement.LanguageProperty, null);
			}, "#2");
		}
		
		
		[TestMethod]
		public void Loaded_itemscontrol ()
		{
			bool loaded = false;
			ItemsControl c = new ItemsControl ();
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (c), "#1");
			c.Loaded += delegate { loaded = true; };
			
			ListBoxItem item = new ListBoxItem {
				Content = new Rectangle { Fill = new SolidColorBrush (Colors.Black), Width = 20, Height = 20 }
			};
			c.Items.Add (item);
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (c), "#2");

			TestPanel.Children.Add (c);
			Assert.IsFalse (loaded, "#3");
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (c), "#4");
		}
		

		[TestMethod]
		[Asynchronous]
		public void Loaded_nonStyled ()
		{
			bool loaded = false;
			ConcreteFrameworkElement element = new ConcreteFrameworkElement ();
			element.Loaded += (o, e) => loaded = true;
			Enqueue (() => TestPanel.Children.Add (element));
			EnqueueConditional (() => loaded );
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void Loaded_styled ()
		{
			Button b = new Button ();
			bool loaded = false;
			bool loaded_sync = false;
			b.Loaded += (o, e) => loaded = true;
			Enqueue (() => { TestPanel.Children.Add (b); loaded_sync = loaded; } );
			EnqueueConditional (() => loaded && !loaded_sync );
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void Loaded_styledChildOfNonStyledParent ()
		{
			Canvas c = new Canvas ();
			Button b = new Button ();

			c.Children.Add (b);

			bool b_loaded = false;
			bool b_loaded_sync = false;
			bool c_loaded = false;
			bool c_loaded_sync = false;
			bool b_loaded_first = false;
			b.Loaded += (o, e) => { b_loaded = true; b_loaded_first = !c_loaded; };
			c.Loaded += (o, e) => c_loaded = true;
			Enqueue (() => { TestPanel.Children.Add (c);
					 b_loaded_sync = b_loaded;
					 c_loaded_sync = c_loaded;
				} );
			EnqueueConditional (() => b_loaded && !b_loaded_sync && c_loaded && !c_loaded_sync && b_loaded_first);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void Loaded_styledChildOfNonStyledParent_parsed ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Button Content=""hi"" />
</Canvas>
");

			Button b = (Button)c.Children[0];

			bool b_loaded = false;
			bool b_loaded_sync = false;
			bool c_loaded = false;
			bool c_loaded_sync = false;
			b.Loaded += (o, e) => b_loaded = true;
			c.Loaded += (o, e) => c_loaded = true;
			Enqueue (() => { TestPanel.Children.Add (c);
					 b_loaded_sync = b_loaded;
					 c_loaded_sync = c_loaded;
				} );
			EnqueueConditional (() => b_loaded && !b_loaded_sync && c_loaded && !c_loaded_sync);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void Loaded_styledChildOfNonStyledParent_nonStyledSiblingAfter ()
		{
			Canvas c = new Canvas ();
			Button b = new Button ();
			ConcreteFrameworkElement cfe = new ConcreteFrameworkElement ();

			c.Children.Add (b);
			c.Children.Add (cfe);

			bool b_loaded = false;
			bool b_loaded_sync = false;
			bool c_loaded = false;
			bool c_loaded_sync = false;
			bool b_loaded_before_sibling = false;
			bool cfe_loaded = false;
			bool cfe_loaded_sync = false;

			cfe.Loaded += (o, e) => cfe_loaded = true;
			b.Loaded += (o, e) => { b_loaded = true; b_loaded_before_sibling = !cfe_loaded; };
			c.Loaded += (o, e) => c_loaded = true;
			Enqueue (() => { TestPanel.Children.Add (c);
					 b_loaded_sync = b_loaded;
					 c_loaded_sync = c_loaded;
					 cfe_loaded_sync = cfe_loaded;
				} );

			EnqueueConditional (() => b_loaded && !b_loaded_sync, "1");
			EnqueueConditional (() => cfe_loaded && !cfe_loaded_sync, "2");
			EnqueueConditional (() => c_loaded && !c_loaded_sync, "3");
			EnqueueConditional (() => b_loaded_before_sibling, "4");

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void Loaded_styledChildOfNonStyledParent_nonStyledSiblingBefore ()
		{
			Canvas c = new Canvas ();
			Button b = new Button ();
			ConcreteFrameworkElement cfe = new ConcreteFrameworkElement ();

			c.Children.Add (cfe);
			c.Children.Add (b);

			bool b_loaded = false;
			bool b_loaded_sync = false;
			bool c_loaded = false;
			bool c_loaded_sync = false;
			bool b_loaded_before_sibling = false;
			bool cfe_loaded = false;
			bool cfe_loaded_sync = false;

			cfe.Loaded += (o, e) => cfe_loaded = true;
			b.Loaded += (o, e) => { b_loaded = true; b_loaded_before_sibling = !cfe_loaded; };
			c.Loaded += (o, e) => c_loaded = true;
			Enqueue (() => { TestPanel.Children.Add (c);
					 b_loaded_sync = b_loaded;
					 c_loaded_sync = c_loaded;
					 cfe_loaded_sync = cfe_loaded;
				} );
			EnqueueConditional (() => b_loaded && !b_loaded_sync, "1");
			EnqueueConditional (() => cfe_loaded && !cfe_loaded_sync, "2");
			EnqueueConditional (() => c_loaded && !c_loaded_sync, "3");
			EnqueueConditional (() => !b_loaded_before_sibling, "4");

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void Loaded_styledChildOfNonStyledParent_styledSiblingInsertedBefore ()
		{
			Canvas c = new Canvas ();
			Button b1 = new Button ();
			Button b2 = new Button ();

			c.Children.Add (b1);

			bool b1_loaded = false;
			bool b1_loaded_sync = false;
			bool b2_loaded = false;
			bool b2_loaded_sync = false;
			bool c_loaded = false;
			bool c_loaded_sync = false;
			bool b1_loaded_before_sibling = false;
			bool b2_loaded_before_c = false;

			b1.Loaded += (o, e) => { b1_loaded = true; b1_loaded_before_sibling = !b2_loaded; };
			b2.Loaded += (o, e) => { b2_loaded = true; b2_loaded_before_c = !c_loaded; };
			c.Loaded += (o, e) => c_loaded = true;
			Enqueue (() => { TestPanel.Children.Add (c);
					 b1_loaded_sync = b1_loaded;
					 c_loaded_sync = c_loaded;
					 c.Children.Insert (0, b2);
					 b2_loaded_sync = b2_loaded;
				} );

			EnqueueConditional (() => b1_loaded && !b1_loaded_sync, "1");
			EnqueueConditional (() => b2_loaded && !b2_loaded_sync, "2");
			EnqueueConditional (() => c_loaded && !c_loaded_sync, "3");
			EnqueueConditional (() => b1_loaded_before_sibling, "4");
			EnqueueConditional (() => !b2_loaded_before_c, "5");

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void Loaded_styledChildOfNonStyledParent_childRemoved ()
		{
			Canvas c = new Canvas ();
			Button b1 = new Button ();

			c.Children.Add (b1);

			bool b1_loaded = false;
			bool c_loaded = false;

			b1.Loaded += (o, e) => b1_loaded = true;
			c.Loaded += (o, e) => c_loaded = true;
			Enqueue (() => { TestPanel.Children.Add (c);
					 c.Children.RemoveAt (0);
				} );

			EnqueueConditional (() => b1_loaded, "1");
			EnqueueConditional (() => c_loaded, "3");

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void Loaded_async ()
		{
			bool loaded = false;
			Canvas canvas = new Canvas ();
			canvas.Loaded += (o, e) => loaded = true;

			// Add a canvas to a loaded subtree and check the event is async.
			Enqueue (() => {
				TestPanel.Children.Add (canvas);
				Assert.IsFalse (loaded, "#1");
			});
			EnqueueConditional (() => loaded, "#2");

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void Loaded_twice ()
		{
			bool loaded = false;
			Canvas canvas = new Canvas ();
			canvas.Loaded += (o, e) => loaded = true;

			Enqueue (() => TestPanel.Children.Add (canvas));
			EnqueueConditional (() => loaded, "#1");

			// Check that removing the element and adding again fires the
			// loaded event again.
			Enqueue (() => {
				loaded = false;
				TestPanel.Children.Clear ();
				TestPanel.Children.Add (canvas);
			});
			EnqueueConditional (() => loaded, "#2");

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void Loaded_subtree_twice ()
		{
			bool loaded_canvas = false;
			bool loaded_button = false;
			Canvas canvas = new Canvas ();
			Button button = new Button ();

			button.Loaded += (o, e) => loaded_button = true;
			canvas.Loaded += (o, e) => loaded_canvas = true;
			canvas.Children.Add (button);

			// Add a canvas + button to a loaded subtree and check the event is async.
			Enqueue (() => TestPanel.Children.Add (canvas));
			EnqueueConditional (() => loaded_canvas, "#1");
			EnqueueConditional (() => loaded_button, "#2");

			// Then check that removing the element and adding again fires the
			// loaded event again on the entire subtree
			Enqueue (() => {
				loaded_button = false;
				loaded_canvas = false;
				TestPanel.Children.Clear ();
				TestPanel.Children.Add (canvas);
			});
			EnqueueConditional (() => loaded_canvas, "#3");
			EnqueueConditional (() => loaded_button, "#4");

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void LayoutUpdatedAndLoaded ()
		{
			List<string> events = new List<string>();

			Rectangle r = new Rectangle ();
			r.LayoutUpdated += delegate { events.Add ("LayoutUpdated"); };
			r.Loaded += delegate { events.Add ("Loaded"); };

			TestPanel.Children.Add (r);

			r.UpdateLayout ();
			Assert.AreEqual (1, events.Count, "#1");
			Assert.AreEqual ("LayoutUpdated", events [0], "#2");
			Enqueue (() => {
				Assert.IsTrue (events.Count >= 2, "#3");
				Assert.AreEqual ("Loaded", events [1], "#4");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void LayoutUpdated ()
		{
			bool layoutUpdated = false;
			bool loaded = false;
			ConcreteFrameworkElement element = new ConcreteFrameworkElement ();
			element.LayoutUpdated += (o, e) => { layoutUpdated = true; Console.WriteLine ("layoutUpdated"); };
			element.Loaded += (o, e) => { loaded = true; Console.WriteLine ("loaded!"); };
			Enqueue (() => TestPanel.Children.Add (element));
			EnqueueConditional (() => loaded );
			EnqueueConditional (() => layoutUpdated);
			Enqueue (() => { layoutUpdated = false; element.InvalidateArrange (); });
			EnqueueConditional (() => layoutUpdated);
			Enqueue (() => { layoutUpdated = false; element.InvalidateMeasure (); });
			EnqueueConditional (() => layoutUpdated);
			EnqueueTestComplete ();
		}

		[TestMethod]
		public void OnApplyTemplate ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			fe.OnApplyTemplate ();
		}

		[TestMethod]
		public void FindName ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			Assert.Throws<ArgumentNullException> (delegate {
				fe.FindName (null);
			}, "FindName(null)");
			Assert.IsNull (fe.FindName (String.Empty), "FindName(Empty)");
		}

		[TestMethod]
		public void SetBinding ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			Assert.Throws<ArgumentNullException> (delegate {
				fe.SetBinding (null, null);
			}, "SetBinding(null,null)");
			Assert.Throws<ArgumentNullException> (delegate {
				fe.SetBinding (FrameworkElement.ActualHeightProperty, null);
			}, "SetBinding(DP,null)");
		}

		[TestMethod]
		public void ArrangeOverride ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			Size result = fe.ArrangeOverride_ (new Size (0, 0));
			Assert.AreEqual (new Size (0, 0), result, "0,0");
			result = fe.MeasureOverride_ (Size.Empty);
			Assert.AreEqual (new Size (0, 0), result, "Empty");
		}

		[TestMethod]
		[MoonlightBug]
		public void ArrangeOverride2 ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			c.Arrange (new Rect (0, 0, 100, 100));
			Assert.IsFalse (c.Arranged, "#1");
			
			c.InvalidateArrange ();
			c.Arrange (new Rect (0, 0, 100, 100));
			Assert.IsFalse (c.Arranged, "#2");
		}
		
		[TestMethod]
		[MoonlightBug]
		[Asynchronous]
		public void ArrangeOverride3 ()
		{
			bool loaded = false;
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			c.Loaded += delegate { loaded = true; };
			TestPanel.Children.Add (c);

			c.Arrange (new Rect (0, 0, 100, 100));
			Assert.IsFalse (c.Arranged, "#1");

			c.InvalidateArrange ();
			c.Arrange (new Rect (0, 0, 100, 100));
			Assert.IsFalse (c.Arranged, "#2");

			EnqueueConditional (() => loaded, "#3");
			Enqueue (() => { Assert.IsTrue (c.Arranged, "#4"); });
			EnqueueTestComplete ();
		}

		[TestMethod]
		public void DataContextTest ()
		{
			object context = new object ();
			Grid grid = new Grid { DataContext = context };
			Rectangle r = new Rectangle ();
			grid.Children.Add (r);

			Assert.AreEqual (context, r.DataContext, "#1");
			Assert.AreEqual (DependencyProperty.UnsetValue, r.ReadLocalValue (FrameworkElement.DataContextProperty), "#2");
		}
		
		[TestMethod]
		public void DataContextTest2 ()
		{
			ContentControl c = new ContentControl();
			TestPanel.Children.Add (c);
			c.DataContext = new Rectangle { Name = "Name" };
			Assert.AreSame(c.DataContext, c.FindName ("Name"), "#1"); // Fails in Silverlight 3
		}
		
		[TestMethod]
		[Asynchronous]
		public void DataContextTest3 ()
		{
			// Check that Content isn't automatically copied to 'DataContext'
			// like it does in ContentPresenter
			ContentControl c = new ContentControl ();
			c.Content = "Hello";
			CreateAsyncTest (c,
				() => Assert.IsNull (c.DataContext, "#1")
			);
		}
		
		[TestMethod]
		public void MeasureOverride ()
		{
			Border b = new Border ();
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			b.Child = fe;
			Size result = fe.MeasureOverride_ (new Size (0, 0));
			Assert.AreEqual (new Size (0, 0), result, "0,0");
			Assert.AreEqual (new Size (0, 0), b.DesiredSize);
			result = fe.MeasureOverride_ (Size.Empty);
			Assert.AreEqual (new Size (0, 0), result, "Empty");
			Assert.AreEqual (new Size (0, 0), b.DesiredSize);
			result = fe.MeasureOverride_ (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (0, 0), result, "Infinity");
			result = fe.MeasureOverride_ (new Size (10, 10));
			Assert.AreEqual (new Size (0, 0), b.DesiredSize);
			Assert.AreEqual (new Size (0, 0), result, "(10,10)");
			fe.Width = 10;
			fe.Height = 10;
			//result = fe.MeasureOverride_ (new Size (100, 100));
			b.Measure (new Size (100, 100));
			Assert.AreEqual (new Size (0, 0), result, "100 with 10");
			Assert.AreEqual (new Size (10,10), b.DesiredSize, "b desired");
			Assert.AreEqual (new Size (10,10), fe.DesiredSize, "fe desired");
			result = fe.MeasureOverride_ (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (0, 0), result, "Infinity");
		}

		[TestMethod]
		[MoonlightBug]
		public void MeasureOverride2 ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			c.Measure (new Size (0, 100));
			Assert.IsFalse (c.Measured, "#1");
			c.InvalidateMeasure ();
			c.Measure (new Size (0, 100));
			Assert.IsFalse (c.Measured, "#2");
		}

		[TestMethod]
		public void MeasureOverride3 ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			TestPanel.Children.Add (c);

			c.Measure (new Size (0, 100));
			Assert.IsTrue (c.Measured, "#1");
		}

		[TestMethod]
		public void MeasureTest ()
		{
			Border b = new Border ();
			var fe = new ConcreteFrameworkElement ();
			b.Child = fe;

			b.Measure (new Size (100,100));

			Assert.AreEqual (new Size (0, 0), fe.DesiredSize, "deisred");

			fe.Width = 10;
			fe.Height = 10;
			
			b.Measure (new Size (100,100));
			
			Assert.AreEqual (new Size (10, 10), fe.DesiredSize, "deisred");
		}
		
		[TestMethod]
		public void ParentlessMeasureTest ()
		{
			var fe = new ConcreteFrameworkElement ();
	
			fe.Measure (new Size (100,100));

			Assert.AreEqual (new Size (0, 0), fe.DesiredSize, "deisred");

			fe.Width = 10;
			fe.Height = 10;
			
			Assert.AreEqual (new Size (10,10), new Size (fe.ActualWidth, fe.ActualHeight), "fe actual");

			fe.InvalidateMeasure ();
			fe.Measure (new Size (100,100));
			
			Assert.AreEqual (new Size (0, 0), fe.DesiredSize, "deisred");
		}

		[TestMethod]
		public void ComputeActualWidth ()
		{
			var c = new ConcreteFrameworkElement ();

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			c.MaxWidth = 25;
			c.Width = 50;
			c.MinHeight = 33;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
		}

		[TestMethod]
		public void ArrangeInfinite ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			var input = new Rect (0, 0, Double.PositiveInfinity, Double.PositiveInfinity);
			Assert.Throws<InvalidOperationException>(delegate { 
					fe.Arrange (input);
				});
		}

		[TestMethod]
		public void SetName ()
		{
			SetName (new ConcreteFrameworkElement ());
		}

		static public void SetName (FrameworkElement fe)
		{
			// a setter exists in SL2 but can only be assigned from XAML
			// so either (a) something else must happen after that; or
			// (b) nothing happens, the name is just not considered (for some uses ?)
			string name = "ouch";
			fe.Name = name;
			// not really
			Assert.AreEqual (name, fe.Name, "set_Name - equal");
			Assert.AreNotSame (name, fe.Name, "set_Name - same");
			// unless it's a set once ?
			name = "ouch^2";
			fe.Name = name;
			Assert.AreEqual (name, fe.Name, "again - equal");
			Assert.AreNotSame (name, fe.Name, "again - same");
			// no, the doc is not (always) right, i.e. other conditions applies
		}
		
		[TestMethod]
		[MoonlightBug ("We don't validate when clearing the value")]
		public void SetStyleTest ()
		{
			// Fails in Silverlight 3
			Style s = new Style (typeof (ConcreteFrameworkElement));
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			c.Style = null;
			c.Style = null;
			c.Style = s;
			Assert.Throws<Exception> (() => c.Style = null);
			Assert.AreEqual (s, c.Style);
			Assert.Throws<Exception> (() => c.ClearValue (FrameworkElement.StyleProperty));
			Assert.AreEqual (s, c.Style);
		}
		
		[TestMethod]
		public void TagTest ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			c.Tag = global::System.UriKind.Absolute;
			Assert.IsTrue (c.Tag is global::System.UriKind, "Type was {0}, should be System.UriKind", c.Tag.GetType ().Name);
		}
		
		[TestMethod]
		[Asynchronous]
		public void TagTest2 ()
		{
			Button b = new Button ();
			TestPanel.Children.Add (b);
			b.ApplyTemplate ();
			
			Enqueue (() => {
				TestPanel.Tag = b;
				TestPanel.Tag = null;
				Assert.IsTrue (b.Focus (), "#1");
			});
			EnqueueTestComplete ();
		}
	}
}
