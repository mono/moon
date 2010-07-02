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
using System.Windows.Controls.Primitives;

namespace MoonTest.System.Windows {

	[TestClass]
	public class FrameworkElementTest : SilverlightTest {

		class ExceptionalFrameworkElement : ContentControl
		{
			public bool ArrangeException { get; set; }
			public bool MeasureException { get; set; }
			public bool OnApplyTemplateException { get; set; }

			protected override Size ArrangeOverride (Size finalSize)
			{
				if (ArrangeException)
					throw new Exception ("ArrangeException");
				return base.ArrangeOverride (finalSize);
			}

			protected override Size MeasureOverride (Size availableSize)
			{
				if (MeasureException)
					throw new Exception ("MeasureException");
				return base.MeasureOverride (availableSize);
			}

			public override void OnApplyTemplate ()
			{
				if (OnApplyTemplateException)
					throw new Exception ("TemplateException");
				base.OnApplyTemplate ();
			}
		}

		class ConcreteFrameworkElement : FrameworkElement {

			public List<string> Methods = new List<string> ();
			public bool Templated, Arranged, Measured;
			public Size arrangeInput;

			public static readonly DependencyProperty FooProperty = DependencyProperty.Register ("Foo", typeof (object), typeof (ConcreteFrameworkElement), null);

			public object Foo
			{
				get { return GetValue (ConcreteFrameworkElement.FooProperty); }
				set { SetValue (ConcreteFrameworkElement.FooProperty, value); }
			}

			public static readonly DependencyProperty DefaultFooProperty = DependencyProperty.Register ("DefaultFoo", typeof (object), typeof (ConcreteFrameworkElement), new PropertyMetadata (3));

			public object DefaultFoo
			{
				get { return GetValue (ConcreteFrameworkElement.DefaultFooProperty); }
				set { SetValue (ConcreteFrameworkElement.DefaultFooProperty, value); }
			}

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
		public void SetUnset ()
		{
			var obj = new object ();
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement () {
					Width = 30,
					Foo = obj,
					DefaultFoo = obj
				};
		        fe.SetValue(FrameworkElement.WidthProperty, DependencyProperty.UnsetValue);
			fe.SetValue(ConcreteFrameworkElement.FooProperty, DependencyProperty.UnsetValue);
			fe.SetValue(ConcreteFrameworkElement.DefaultFooProperty, DependencyProperty.UnsetValue);
			Assert.IsTrue (double.IsNaN (fe.Width), "Width is NaN (the default value)");
			Assert.AreEqual (3, fe.DefaultFoo, "DefaultFoo is 3 (the default value)");
			Assert.IsNull (fe.Foo, "Foo is null");
		}

		[TestMethod]
		public void SetUnsetClear_Tag ()
		{
			var obj = new object ();
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement () {
					Tag = obj
				};
			fe.SetValue(FrameworkElement.TagProperty, DependencyProperty.UnsetValue);			
			Assert.IsNull (fe.Tag, "Tag is not null (the default value)");
			fe.Tag = 3;
			Assert.AreEqual (3, fe.Tag, "Tag can change though");
			fe.SetValue(FrameworkElement.TagProperty, DependencyProperty.UnsetValue);			
			Assert.IsNull (fe.Tag, "Tag is null");
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
					Assert.VisualChildren (c, "#3",
						new VisualNode<Grid> ("#d",
							new VisualNode<ContentPresenter> ("#e")
						)
					);
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
					Assert.VisualChildren (c, "#3",
						new VisualNode<Grid> ("#d",
							new VisualNode<ContentPresenter> ("#e")
						)
					);
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
					Assert.VisualChildren (c, "#3",
						new VisualNode<Grid> ("#a",
							new VisualNode<ContentPresenter> ("#b")
						)
					);
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

			Grid gridA = null, gridB = null, gridC = null;
			ContentPresenter presenterA = null, presenterB = null, presenterC = null;
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
					Assert.VisualChildren (c, "#2",
						new VisualNode<Grid> ("#d", g => gridB = g,
							new VisualNode<ContentPresenter> ("#e", p => presenterB = p)
						)
					);
				},
				() => Assert.VisualChildren (c, "#3",
					new VisualNode<Grid> ("#g", g => gridC = g,
						new VisualNode<ContentPresenter> ("#h", p => presenterC = p,
							new VisualNode<Rectangle> ("#i")
						)
					)
				),
				() => {
					Assert.AreSame (gridA, gridB, "#4");
					Assert.AreSame (gridA, gridC, "#5");

					Assert.AreSame (presenterA, presenterB, "#6");
					Assert.AreSame (presenterA, presenterC, "#7");
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
			Assert.Throws<Exception>(delegate {
				f.Language = null;
			}, "#1"); // Fails in Silverlight 3 (got System.Exception)
			Assert.Throws<Exception>(delegate {
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

			EventHandler handler = delegate { events.Add ("LayoutUpdated"); };

			Rectangle r = new Rectangle ();

			r.LayoutUpdated += handler;
			r.Loaded += delegate { events.Add ("Loaded"); };

			TestPanel.Children.Add (r);

			r.UpdateLayout ();
			Assert.AreEqual (1, events.Count, "#1");
			Assert.AreEqual ("LayoutUpdated", events [0], "#2");
			Enqueue (() => {
				Assert.IsTrue (events.Count >= 2, "#3");
				Assert.AreEqual ("Loaded", events [1], "#4");
				r.LayoutUpdated -= handler;
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
			EventHandler handler = (o, e) => { layoutUpdated = true; };
			element.LayoutUpdated += handler;
			element.Loaded += (o, e) => { loaded = true; };
			Enqueue (() => TestPanel.Children.Add (element));
			EnqueueConditional (() => loaded );
			EnqueueConditional (() => layoutUpdated);
			Enqueue (() => { layoutUpdated = false; element.InvalidateArrange (); });
			EnqueueConditional (() => layoutUpdated);
			Enqueue (() => { layoutUpdated = false; element.InvalidateMeasure (); });
			EnqueueConditional (() => layoutUpdated);
			Enqueue (() => { element.LayoutUpdated -= handler; });
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
			Assert.IsNull (c.FindName ("Name"), "#1");
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
		public void ItemInItsOwnDataContext ()
		{
			// If this test does not lock up the browser in an infinite loop, it's doing its job
			var fe = new Rectangle { Name = "test" };
			TestPanel.Children.Add (fe);
			fe.DataContext = fe;
		}

		[TestMethod]
		public void SameItemMultipleDataContexts ()
		{
			// Check that we can put the same UIElement in multiple datacontexts
			// without things blowing up. This shows that it's a non-parenting
			// non-nameregistering property.
			var data = new Rectangle { Name="MySuperSecretOtherName" };
			var c1 = new ContentControl ();
			var c2 = new ContentControl ();

			TestPanel.Children.Add (c1);
			TestPanel.Children.Add (c2);

			c1.DataContext = data;
			c2.DataContext = data;
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
		// This is probably still a legitate bug, its just Styles are no longer write-once.
		// We need another write-once property and rewrite this test to use that property.
		// We still don't enforce write-once with ClearValue.
		//
		// ****** Don't remove the moonlight bug until the test is rewritten ******
		[MoonlightBug ("We don't validate when clearing the value")]
		public void SetStyleTest ()
		{
			Style s = new Style (typeof (ConcreteFrameworkElement));
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			// Ensure that we can set the style to null a few times, then give it a real value
			c.Style = null;
			c.Style = null;
			c.Style = s;
			// SL 3+ no longer prevents you from clearing the style
			c.Style = null;
			Assert.IsNull (c.Style, "#1");

			c.Style = s;
			c.ClearValue (FrameworkElement.StyleProperty);
			Assert.IsNull(c.Style, "#2");
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

		[TestMethod]
		[MoonlightBug ("SL catches the exception and wraps it in another exception and throws the wrapper exception to propagate the error")]
		public void UpdateLayout_ArrangeException ()
		{
			bool failed = false;
			TestPanel.Children.Add (new ExceptionalFrameworkElement { ArrangeException = true });

			try {
				TestPanel.UpdateLayout ();
				failed = true;
			} catch (Exception ex) {
				Assert.IsInstanceOfType<Exception> (ex.InnerException, "#1");
				Assert.AreEqual ("ArrangeException", ex.InnerException.Message, "#2");
				Assert.AreSame (TestPanel.Children [0], LayoutInformation.GetLayoutExceptionElement (TestPanel.Dispatcher), "#3");
			} finally {
				TestPanel.Children.Clear ();
			}

			if (failed)
				Assert.Fail ("An exception should've been thrown", "#failed");
		}

		[TestMethod]
		[MoonlightBug ("SL catches the exception and wraps it in another exception and throws the wrapper exception to propagate the error")]
		public void UpdateLayout_MeasureException ()
		{
			bool failed = false;
			TestPanel.Children.Add (new ExceptionalFrameworkElement { MeasureException = true });

			try {
				TestPanel.UpdateLayout ();
				failed = true;
			} catch (Exception ex) {
				Assert.IsInstanceOfType<Exception> (ex.InnerException, "#1");
				Assert.AreEqual ("MeasureException", ex.InnerException.Message, "#2");
				Assert.AreSame (TestPanel.Children[0], LayoutInformation.GetLayoutExceptionElement (TestPanel.Dispatcher));
			} finally {
				TestPanel.Children.Clear ();
			}

			if (failed)
				Assert.Fail ("An exception should've been thrown", "#failed");
		}

		[TestMethod]
		[MoonlightBug ("SL catches the exception and wraps it in another exception and throws the wrapper exception to propagate the error")]
		[Ignore ("Since we don't handle the exception properly, it ends up in the plugin's error handler, which will mark the entire test run as failure.")]
		public void UpdateLayout_OnApplyTemplateException ()
		{
			bool failed = false;
			TestPanel.Children.Add (new ExceptionalFrameworkElement { OnApplyTemplateException = true });

			try {
				TestPanel.UpdateLayout ();
				failed = true;
			} catch (Exception ex) {
				Assert.IsInstanceOfType<Exception> (ex.InnerException, "#1");
				Assert.AreEqual ("TemplateException", ex.InnerException.Message, "#2");
				Assert.IsNull (LayoutInformation.GetLayoutExceptionElement (TestPanel.Dispatcher));
			} finally {
				TestPanel.Children.Clear ();
			}

			if(failed)
				Assert.Fail ("An exception should've been thrown", "#failed");
		}

		class UnstyledControl : Control
		{
			public UnstyledControl ()
			{
				DefaultStyleKey = typeof (UnstyledControl);
				Style = null;
				Template = null;
			}
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_Control_FontFamily ()
		{
			VisualInheritanceCore (Control.FontFamilyProperty, new FontFamily ("Arial"), new FontFamily ("Lucida"));
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_Control_TB_FontFamily ()
		{
			var c1 = TemplatedContentControl ();
			var c2 = new TextBlock ();
			c1.Content = c2;
			VisualInheritanceCore (c1, Control.FontFamilyProperty, c2, TextBlock.FontFamilyProperty, new FontFamily ("Arial"), new FontFamily ("Lucida"));
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_Control_FontSize ()
		{
			VisualInheritanceCore (Control.FontSizeProperty, 50.0, 60.0);
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_Control_TB_FontSize ()
		{
			var c1 = TemplatedContentControl ();
			var c2 = new TextBlock ();
			c1.Content = c2;
			VisualInheritanceCore (c1, Control.FontSizeProperty, c2, TextBlock.FontSizeProperty, 50.0, 60.0);
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_Control_FontStretch ()
		{
			VisualInheritanceCore (Control.FontStretchProperty, FontStretches.UltraCondensed, FontStretches.UltraExpanded);
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_Control_TB_FontStretch ()
		{
			var c1 = TemplatedContentControl ();
			var c2 = new TextBlock ();
			c1.Content = c2;
			VisualInheritanceCore (c1, Control.FontStretchProperty, c2, TextBlock.FontStretchProperty, FontStretches.UltraCondensed, FontStretches.UltraExpanded);
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_Control_FontStyle ()
		{
			VisualInheritanceCore (Control.FontStyleProperty, FontStyles.Italic, FontStyles.Normal);
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_Control_TB_FontStyle ()
		{
			var c1 = TemplatedContentControl ();
			var c2 = new TextBlock ();
			c1.Content = c2;
			VisualInheritanceCore (c1, Control.FontStyleProperty, c2, TextBlock.FontStyleProperty, FontStyles.Italic, FontStyles.Normal);
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_Control_FontWeight ()
		{
			VisualInheritanceCore (Control.FontWeightProperty, FontWeights.Bold, FontWeights.Light);
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_Control_TB_FontWeight ()
		{
			var c1 = TemplatedContentControl ();
			var c2 = new TextBlock ();
			c1.Content = c2;
			VisualInheritanceCore (c1, Control.FontWeightProperty, c2, TextBlock.FontWeightProperty, FontWeights.Bold, FontWeights.Light);
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_Control_Foreground ()
		{
			VisualInheritanceCore (Control.ForegroundProperty, new SolidColorBrush (Colors.Red), new SolidColorBrush (Colors.Blue));
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_FE_DataContext ()
		{
			LogicalInheritanceCore (FrameworkElement.DataContextProperty, new object (), new object ());
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_FE_DataContext_ExpandedTemplate ()
		{
			// Walk all elements in the template visual tree and ensure they've inherited the datacontext
			var c1 = TemplatedContentControl ();
			var c2 = new Rectangle ();
			c1.Content = c2;

			CreateAsyncTest (c1,
				() => {
					c1.ApplyTemplate ();
				}, () => {
					c1.DataContext = new object ();
					foreach (var c in c1.GetVisualChildren (true))
						Assert.AreSame (c1.DataContext, c.DataContext, "#same datacontext");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_FE_DataContext_ExpandedTemplate_ChangeTemplateContext ()
		{
			// If the DataContext is explicitly changed on the first template item, all children of that
			// inherit that value. 
			var c1 = TemplatedContentControl ();
			var c2 = new Rectangle ();
			c1.Content = c2;

			CreateAsyncTest (c1,
				() => {
					c1.ApplyTemplate ();
				}, () => {
					c1.DataContext = new object ();

					var child = (FrameworkElement) VisualTreeHelper.GetChild (c1, 0);
					child.DataContext = new object ();
					Assert.AreSame (c1.DataContext, c2.DataContext, "#unchanged");

					foreach (var c in c1.GetVisualChildren (true)) {
						if (c == c1 || c == c2)
							Assert.AreSame (c1.DataContext, c.DataContext, "#should have c1.DataContext");
						else
							Assert.AreSame (child.DataContext, c.DataContext, "#should have child.DataContext");
					}
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_FE_DataContext_ExpandedTemplate_BrokenTree ()
		{
			// If we expand the template and then break the tree by clearing the grid,
			// c2 is unaffected but the rest of the template tree loses the inherited DataContext.
			var c1 = TemplatedContentControl ();
			var c2 = new Rectangle ();
			c1.Content = c2;
			
			CreateAsyncTest (c1,
				() => {
					c1.ApplyTemplate ();
				}, () => {
					c1.DataContext = new object ();

					var child = (FrameworkElement) VisualTreeHelper.GetChild (c1, 0);
					child.DataContext = new object ();
					Assert.AreSame (c1.DataContext, c2.DataContext, "#unchanged");

					// The templated content control gives us a tree of:
					// UserControl, Grid, ContentPresenter, c2. Clear the children
					// of the grid and ensure that c2 still has the right datacontext
					var children = c1.GetVisualChildren (true);
					Assert.IsInstanceOfType<Grid> (children [1], "#1");
					((Grid) children [1]).Children.Clear ();
					Assert.IsNull (children [2].DataContext, "#2");
					Assert.AreSame (c1.DataContext, children [3].DataContext, "#3");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_FE_Language ()
		{
			VisualInheritanceCore (FrameworkElement.LanguageProperty, XmlLanguage.GetLanguage (""), XmlLanguage.GetLanguage ("DE-de"));
		}

		[TestMethod]
		[Asynchronous]
		public void PropertyInheritance_UI_LayoutRounding ()
		{
			VisualInheritanceCore (UIElement.UseLayoutRoundingProperty, false, true);
		}

		void LogicalInheritanceCore (DependencyProperty prop, object value, object other)
		{
			var c1 = TemplatedContentControl ();
			var c2 = new ContentControl ();
			c1.Content = c2;
			LogicalInheritanceCore (c1, prop, c2, prop, value, other);
		}

		void LogicalInheritanceCore (Control c1, DependencyProperty prop1, FrameworkElement c2, DependencyProperty prop2, object value, object other)
		{
			// Check that we inherit values even if the visual tree doesn't exist,
			// i.e. if we use logical inheritance.
			Assert.AreNotEqual (value, c1.GetValue (prop1), "#1");
			Assert.AreNotEqual (value, c2.GetValue (prop2), "#2");
			Assert.IsNull (VisualTreeHelper.GetParent (c2), "#c2 no parent");
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (c1), "#c1 no children");

			c1.SetValue (prop1, value);
			Assert.IsNotNull (c1.GetValue (prop1), "#3");
			Assert.IsNotNull (c2.GetValue (prop2), "#4");
			Assert.AreEqual (c1.GetValue (prop1), c2.GetValue (prop2), "#5");

			// Now generate a visual tree with 'c2' at the end and check that the visual
			// tree doesn't affect which value is inherited
			CreateAsyncTest (c1,
				() => {
					c1.ApplyTemplate ();
				}, () => {
					var visualParent = (FrameworkElement) VisualTreeHelper.GetChild (c1, 0);
					visualParent.SetValue (prop1, other);
					Assert.AreEqual (c2.GetValue (prop2), value, "#6");
				}
			);
		}

		void VisualInheritanceCore (DependencyProperty property, object value, object other)
		{
			var c1 = TemplatedContentControl ();
			var c2 = new UnstyledControl ();
			c1.Content = c2;
			VisualInheritanceCore (c1, property, c2, property, value, other);
		}

		void VisualInheritanceCore (Control c1, DependencyProperty prop1, FrameworkElement c2, DependencyProperty prop2, object value, object other)
		{
			Assert.AreSame (c1, c2.Parent, "#parented");
			Assert.AreNotEqual (value, c1.GetValue (prop1), "#1");
			Assert.AreNotEqual (value, c2.GetValue (prop2), "#2");
			Assert.IsNull (VisualTreeHelper.GetParent (c2), "#c2 no parent");
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (c1), "#c1 no children");

			c1.SetValue (prop1, value);
			Assert.IsNotNull (c1.GetValue (prop1), "#3");
			Assert.IsNotNull (c2.GetValue (prop2), "#4");
			Assert.AreNotEqual (c1.GetValue (prop1), c2.GetValue (prop2), "#5");

			// Once we connect c2 to c1 via the template visual tree, it will inherit the value
			CreateAsyncTest (c1,
				() => {
					c1.ApplyTemplate ();
				}, () => {
					Assert.AreEqual (c1.GetValue (prop1), c2.GetValue (prop2), "#6");

					// And if we change the value inside the template, it affects c2
					var visualParent = (FrameworkElement) VisualTreeHelper.GetChild (c1, 0);
					visualParent.SetValue (prop1, other);
					Assert.AreEqual (c2.GetValue (prop2), other, "#7");
				}
			);
		}

		ContentControl TemplatedContentControl ()
		{
			return new ContentControl {
				Template = (ControlTemplate) XamlReader.Load (@"
<ControlTemplate
	xmlns=""http://schemas.microsoft.com/client/2007""
	xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
	xmlns:clr=""clr-namespace:MoonTest.System.Windows.Controls;assembly=moon-unit"">
	<UserControl>
		<Grid>
			<ContentPresenter x:Name=""ContentPresenter"" />
		</Grid>
	</UserControl>
</ControlTemplate>
")
			};
		}
	}
}
