//
// ContentControl Unit Tests
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

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Markup;
using Microsoft.Silverlight.Testing;
using System.Windows.Shapes;
using System.Windows.Data;
using System.Windows.Media;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public partial class ContentControlTest : SilverlightTest {

		class CanvasControl : UserControl
		{
			public Canvas Canvas {
				get { return (Canvas) Content; }
			}

			public CanvasControl ()
			{
				Content = new Canvas ();
			}
		}

		class ContentControlPoker : ContentControl {

			public object DefaultStyleKey_ {
				get { return base.DefaultStyleKey; }
				set { base.DefaultStyleKey = value; }
			}

			public object OldContent;
			public object NewContent;
			public bool Measured;

			protected override void OnContentChanged (object oldContent, object newContent)
			{
				OldContent = oldContent;
				NewContent = newContent;
				base.OnContentChanged (oldContent, newContent);
			}

			protected override Size MeasureOverride (Size availableSize)
			{
				Measured = true;
				return base.MeasureOverride (availableSize);
			}
		}

		[TestMethod]
		public void PeekProperties ()
		{
			ContentControlPoker cc = new ContentControlPoker ();
			Assert.IsNotNull (cc.DefaultStyleKey_, "DefaultStyleKey");
			Assert.AreEqual (typeof (ContentControl), cc.DefaultStyleKey_, "DefaultStyleKey/Type");
		}

		[TestMethod]
		public void DefaultMethods ()
		{
			ContentControl cc = new ContentControl ();
			ControlTest.CheckDefaultMethods (cc);
		}

		[TestMethod]
		[Asynchronous]
		public void ChangeDefaultTemplate ()
		{
			// Fails in Silverlight 3
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007"">
    <ContentControl.Template>
        <ControlTemplate>
            <ContentPresenter />
        </ControlTemplate>
    </ContentControl.Template>
</ContentControl>");

			c.Content = new ConcreteFrameworkElement ();
			c.ContentTemplate = new DataTemplate ();
			ContentPresenter p = null;
			CreateAsyncTest (c,
				() => Assert.VisualChildren (c, "#1",
						new VisualNode<ContentPresenter> ("#2", (pr) => p = pr, (VisualNode[]) null)
					),
				() => {
					Assert.IsNull (p.DataContext, "#3");
					Assert.AreEqual (c.Content, p.Content, "#4");
				},
				() => {
					Assert.AreSame (c.Content, p.Content);
					Assert.IsInstanceOfType<TemplateBindingExpression> (p.ReadLocalValue (ContentPresenter.ContentProperty), "#5");

					Assert.AreSame (c.ContentTemplate, p.ContentTemplate);
					Assert.IsInstanceOfType<TemplateBindingExpression> (p.ReadLocalValue (ContentPresenter.ContentTemplateProperty), "#6");
				},
				() => Assert.VisualChildren (p, new VisualNode <ConcreteFrameworkElement> ("#7"))
			);
		}
		
		[TestMethod]
		public void ChangingContentInvalidatesMeasure ()
		{
			var p = new ContentControlPoker ();
			p.Measure (new Size (50, 50));
			Assert.IsTrue (p.Measured, "#1");
			Assert.VisualChildren (p, "#2");

			p.Measured = false;
			p.Content = "a";
			p.Measure (new Size (50, 50));
			Assert.IsTrue (p.Measured, "#3");
			Assert.VisualChildren (p, "#4",
				new VisualNode<Grid> ("#a",
					new VisualNode<TextBlock> ("#b")
				)
			);

			p.Measured = false;
			p.Content = "b";
			p.Measure (new Size (50, 50));
			Assert.IsTrue (p.Measured, "#5");
			Assert.VisualChildren (p, "#6",
				new VisualNode<Grid> ("#a",
					new VisualNode<TextBlock> ("#b")
				)
			);
		}

		[TestMethod]
		public void Content ()
		{
			ContentControlPoker cc = new ContentControlPoker ();
			cc.Content = cc;
			Assert.IsNull (cc.OldContent, "OldContent");
			Assert.IsNotNull (cc.NewContent, "NewContent");
			Assert.AreSame (cc, cc.NewContent, "OldContent/NewContent");
		}

		[TestMethod]
		public void ContentTemplate ()
		{
			ContentControlPoker cc = new ContentControlPoker ();
			// Note: OnContentTemplateChanged was "removed" in SL2 final
			cc.ContentTemplate = new DataTemplate ();
			// and not merged (as expected) with OnContentChanged
			Assert.IsNull (cc.OldContent, "ContentTemplate/OldContent");
			Assert.IsNull (cc.NewContent, "ContentTemplate/NewContent");
		}

		[TestMethod]
		public void ContentShareString ()
		{
			ContentControl cc1 = new ContentControl ();
			cc1.Content = "share strings is good for the karma";
			ContentControl cc2 = new ContentControl ();
			cc2.Content = cc1.Content;
			Assert.AreEqual (cc1.Content, cc2.Content, "string");
		}

		void CanShare (object obj)
		{
			ContentControl cc1 = new ContentControl ();
			cc1.Content = obj;
			ContentControl cc2 = new ContentControl ();
			cc2.Content = cc1.Content;
			Assert.IsTrue (Object.ReferenceEquals (cc1.Content, cc2.Content), "non-DO");
		}

		[TestMethod]
		public void ContentShareNonDependencyObject ()
		{
			CanShare (new object ());
		}

		public class ConcreteDependencyObject : DependencyObject {
		}

		[TestMethod]
		public void ContentShareDependencyObject ()
		{
			CanShare (new ConcreteDependencyObject ());
		}

		void CanNotShare (FrameworkElement fe)
		{
			ContentControl cc1 = new ContentControl ();
			Assert.IsNull (fe.Parent, "!Parent");
			cc1.Content = fe;
			Assert.IsTrue (Object.ReferenceEquals (cc1, fe.Parent), "Parent");

			ContentControl cc2 = new ContentControl ();
			Assert.Throws<InvalidOperationException> (delegate {
				cc2.Content = fe;
			}, "shared");

			// remove it from cc1.Content and use it inside cc2
			cc1.Content = null;
			cc2.Content = fe;
			Assert.IsTrue (Object.ReferenceEquals (cc2.Content, fe), "non-shared");
		}

		public class ConcreteFrameworkElement : FrameworkElement {
		}

		[TestMethod]
		public void ContentShareFrameworkElement ()
		{
			CanNotShare (new ConcreteFrameworkElement ());
		}

		[TestMethod]
		public void ContentShareWithCanvas ()
		{
			ConcreteFrameworkElement cfe = new ConcreteFrameworkElement ();
			Assert.IsNull (cfe.Parent, "!Parent");

			Canvas canvas = new Canvas ();
			canvas.Children.Add (cfe);
			Assert.IsNotNull (cfe.Parent, "Parent");

			ContentControl cc = new ContentControl ();
			Assert.Throws<ArgumentException> (delegate {
				// there's a parent to 'cfe' so ArgumentException is thrown
				cc.Content = cfe;
			}, "shared canvas/content");
			Assert.IsNull (cc.Content, "Content");
		}

		public class ContentControlSharer : ContentControl {

			protected override void OnContentChanged (object oldContent, object newContent)
			{
				// don't call base to see if this is where the sharing is checked
			}
		}

		[TestMethod]
		[Asynchronous]
		public void ContentTemplateNotUsed ()
		{
			ContentControl c = new ContentControl ();
			c.Content = "Test";
			c.ContentTemplate = CreateDataTemplate ("<ContentControl />");
			CreateAsyncTest (c,
				() => c.ApplyTemplate (),
				() => {
					// Start off with the default template
					Assert.VisualChildren (c, "#1",
						new VisualNode<ContentPresenter> ("#a", (VisualNode []) null)
					);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ContentTemplateNotUsed2 ()
		{
			ContentControl c = new ContentControl {
				Content = "Test",
				ContentTemplate = CreateDataTemplate ("<ContentControl />"),
				Template = null
			};
			CreateAsyncTest (c,
				() => c.ApplyTemplate (),
				() => {
					// Start off with the default template
					Assert.VisualChildren (c, "#1",
						new VisualNode<Grid> ("#a",
							new VisualNode <TextBlock> ("#b")
						)
					);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ContentTemplateNotUsed3 ()
		{
			ContentControl c = new ContentControl {
				Content = new Button { Content = "Hello World" },
				ContentTemplate = CreateDataTemplate ("<ContentControl />"),
				Template = null
			};
			CreateAsyncTest (c,
				() => c.ApplyTemplate (),
				() => {
					// Start off with the default template
					Assert.VisualChildren (c, "#1",
						new VisualNode<Button> ("#a", (VisualNode []) null)
					);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void DataTemplateTest ()
		{
			ContentControl c = new ContentControl ();
			c.Content = new ConcreteFrameworkElement ();
			CreateAsyncTest (c, () =>
				Assert.VisualChildren (c, "#1",
					new VisualNode<ContentPresenter> ("#a",
						new VisualNode<ConcreteFrameworkElement> ("#b")
					)
				)
			);
		}

		[TestMethod]
		[Asynchronous]
		public void DataTemplateTest2 ()
		{
			// Fails in Silverlight 3
			ContentControl c = new ContentControl ();
			c.Content = new ConcreteFrameworkElement ();
			c.ContentTemplate = new DataTemplate ();
			CreateAsyncTest (c, () =>
				Assert.VisualChildren (c, "#1",
					new VisualNode<ContentPresenter> ("#a",
						new VisualNode<ConcreteFrameworkElement> ("#b")
					)
				)
			);
		}

		[TestMethod]
		[Asynchronous]
		public void DataTemplateTest3 ()
		{
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007"">
    <ContentControl.ContentTemplate>
        <DataTemplate>
            <Grid />
        </DataTemplate>
    </ContentControl.ContentTemplate>
</ContentControl>");

			c.Content = new ConcreteFrameworkElement ();
			CreateAsyncTest (c, () =>
				Assert.VisualChildren (c, "#1",
					new VisualNode<ContentPresenter> ("#a",
						new VisualNode<Grid> ("#b")
					)
				)
			);
		}

		[TestMethod]
		[Asynchronous]
		public void DataTemplateTest4 ()
		{
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007"">
    <ContentControl.ContentTemplate>
        <DataTemplate>
            <Grid>
				<ContentPresenter />
			</Grid>
        </DataTemplate>
    </ContentControl.ContentTemplate>
</ContentControl>");

			c.Content = new ConcreteFrameworkElement ();
			ContentPresenter p = null;
			CreateAsyncTest (c, () =>
				Assert.VisualChildren (c, "#1",
					new VisualNode<ContentPresenter> ("#a",
						new VisualNode<Grid> ("#b",
							new VisualNode<ContentPresenter> ("#c", pr => p = pr)
						)
					)
				),
				() => {
					Assert.AreEqual (DependencyProperty.UnsetValue, p.ReadLocalValue (ContentPresenter.ContentProperty), "#2");
					Assert.AreEqual (DependencyProperty.UnsetValue, p.ReadLocalValue (ContentPresenter.ContentTemplateProperty), "#3");
				}
			);
		}

		[Asynchronous]
		[TestMethod]
		public void IsEnabledTest ()
		{
			ContentControl a = new ContentControl { Name = "a" };
			ContentControl b = new ContentControl { Name = "b" };
			ContentControl c = new ContentControl { Name = "c" };
			
			a.Content = b;
			b.Content = c;

			CreateAsyncTest (a,
				() => {
					a.IsEnabled = false;
					Assert.IsFalse (a.IsEnabled, "#1");
					Assert.IsFalse (b.IsEnabled, "#2");
					Assert.IsFalse (c.IsEnabled, "#3");

					b.IsEnabled = false;
					a.IsEnabled = true;
					Assert.IsTrue (a.IsEnabled, "#4");
					Assert.IsFalse (b.IsEnabled, "#5");
					Assert.IsFalse (c.IsEnabled, "#6");

					b.IsEnabled = true;
					Assert.IsTrue (a.IsEnabled, "#7");
					Assert.IsTrue (b.IsEnabled, "#8");
					Assert.IsTrue (c.IsEnabled, "#9");
				}
			);
		}
		
		[TestMethod]
		[Asynchronous]
		public void IsEnabledTest2 ()
		{
			ContentControl a = new ContentControl ();
			ContentControl b = new ContentControl ();
			a.Content = b;

			a.IsEnabled = false;
			Assert.IsTrue (b.IsEnabled, "#1");
			
			CreateAsyncTest (a,
				() => {
					// Disabling A disables all children
					Assert.IsFalse (b.IsEnabled, "#2");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void IsEnabledTest3 ()
		{
			ContentControl a = new ContentControl ();
			ContentControl b = new ContentControl ();

			a.Content = b;
			a.IsEnabled = false;
			Assert.IsTrue (b.IsEnabled, "#1");

			a.ApplyTemplate ();
			Assert.IsTrue (b.IsEnabled, "#2");

			a.Measure (new Size { Height = 10,  Width = 10 });
			Assert.IsTrue (b.IsEnabled, "#3");

			CreateAsyncTest (a,
				() => Assert.IsFalse (b.IsEnabled, "#4")
			);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void IsEnabledTest4 ()
		{
			int count = 0;
			ContentControl a = new ContentControl ();
			ContentControl b = new ContentControl ();

			a.IsEnabledChanged += delegate { count++; };
			b.IsEnabledChanged += delegate { count++; };

			a.Content = b;

			CreateAsyncTest (a,
				() => {
					a.IsEnabled = false;
					Assert.AreEqual (0, count, "#1");
				},
				() => {
					Assert.AreEqual (2, count, "#2");
				}
			);
		}

		[TestMethod]
		[MoonlightBug]
		public void IsEnabledTest5 ()
		{
			int count = 0;
			ContentControl a = new ContentControl ();
			a.IsEnabledChanged += delegate { count++; };
			a.IsEnabled = false;
			Assert.AreEqual (0, count, "#1");
		}

		[TestMethod]
		public void IsEnabled_Propagate ()
		{
			// IsEnabled changes propagate immediately on an element not in the live tree
			CanvasControl root = new CanvasControl ();
			CanvasControl child = new CanvasControl ();
			root.Canvas.Children.Add (child);

			root.IsEnabled = false;
			Assert.IsFalse (child.IsEnabled, "#1");
		}

		[TestMethod]
		[Asynchronous]
		public void IsEnabled_Propagate2a ()
		{
			IsEnabledPropgate2Core (false);
		}

		[TestMethod]
		[Asynchronous]
		public void IsEnabled_Propagate2b ()
		{
			IsEnabledPropgate2Core (true);
		}
		
		void IsEnabledPropgate2Core (bool useMiddleElement)
		{
			// Attaching a new control to the tree does not
			// pick up the parent 'IsEnabled' value until
			// the control is loaded.
			bool enabled_when_loaded = false;
			
			CanvasControl root = new CanvasControl ();
			Canvas middle = new Canvas ();
			CanvasControl child = new CanvasControl ();

			child.Loaded += (o, e) => enabled_when_loaded = child.IsEnabled;
			root.IsEnabled = false;

			if (useMiddleElement) {
				root.Canvas.Children.Add (middle);
				middle.Children.Add (child);
			} else {
				root.Canvas.Children.Add (child);
			}

			Assert.IsTrue (child.IsEnabled, "#1");
			Enqueue (() => Assert.IsTrue (child.IsEnabled, "#2"));
			Enqueue (() => TestPanel.Children.Add (root));
			Enqueue (() => Assert.IsFalse (child.IsEnabled, "#3"));
			Enqueue (() => Assert.IsFalse (enabled_when_loaded, "#4"));
			EnqueueTestComplete ();	
		}

		[TestMethod]
		public void IsEnabled_Propagate3 ()
		{
			// Check if disabling the control a second time
			// propagates the value to the child
			CanvasControl root = new CanvasControl ();
			CanvasControl child = new CanvasControl ();

			root.IsEnabled = false;
			root.Canvas.Children.Add (child);
			Assert.IsTrue (child.IsEnabled, "#1");
			root.IsEnabled = false;
			Assert.IsTrue (child.IsEnabled, "#2");
		}


		[TestMethod]
		[Asynchronous]
		[Ignore ("Invalid templates cause Silverlight to barf")]
		public void InvalidTemplateObjectChild ()
		{
			// ContentTemplate is ignored if there is a Template
			ContentControl c = new ContentControl {
				Content = "Test",
				Template = CreateTemplate ("<Storyboard />"),
			};
			CreateAsyncTest (c,
				() => c.ApplyTemplate (),
				() => {
					// Start off with the default template
					Assert.VisualChildren (c, "#1",
						new VisualNode<Grid> ("#a",
							new VisualNode<TextBlock> ("#b")
						)
					);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void NewTemplateDoesNotApplyInstantly ()
		{
			ContentControl c = new ContentControl ();
			c.Content = "Test";
			CreateAsyncTest (c,
				() => c.ApplyTemplate (),
				() => {
					// Start off with the default template
					Assert.VisualChildren (c, "#1",
						new VisualNode<ContentPresenter> ("#a", (VisualNode []) null)
					);

					// Changing the template does not make it apply instantly.
					// It just clears the children.
					c.Template = CreateTemplate ("<Canvas />");
					Assert.VisualChildren (c, "#2");
				}, () => {
					Assert.VisualChildren (c, "#3",
						new VisualNode<Canvas> ("#c")
					);
				}
			);
		}

		[TestMethod]
		public void OverrideContentShareControl ()
		{
			ContentControlSharer cc1 = new ContentControlSharer ();
			cc1.Content = cc1;
			ContentControlSharer cc2 = new ContentControlSharer ();
			Assert.Throws<InvalidOperationException> (delegate {
				cc2.Content = cc1;
			}, "shared");

			// remove it from cc1.Content and use it inside cc2
			cc1.Content = null;
			cc2.Content = cc1;
			Assert.IsTrue (Object.ReferenceEquals (cc2.Content, cc1), "non-shared");
		}

		[TestMethod]
		[Asynchronous]
		public void MultiplePresenters ()
		{
			ControlTemplate template = CreateTemplate (@"
<ContentControl>
	<Grid>
		<ContentPresenter/>
		<ContentPresenter/>
		<ContentPresenter/>
	</Grid>
</ContentControl>
");
			ContentControl c = new ContentControl {
				Template = template,
				Content = "content"
			};

			CreateAsyncTest (c,() => {
				Grid grid = null;
				Assert.VisualChildren (c,
					new VisualNode<ContentControl> ("#1",
						new VisualNode<ContentPresenter> ("#2",
							new VisualNode <Grid> ("#3", g => grid = g, null)
						)
					)
				);
				for (int i =0; i < 3; i++) {
					ContentPresenter p = (ContentPresenter) grid.Children [i];
					Assert.IsInstanceOfType<TemplateBindingExpression> (p.ReadLocalValue (ContentPresenter.ContentProperty), "#4." + i);
					Assert.AreEqual ("content", p.Content, "#5." + i);
				}
			});
		}

		[TestMethod]
		[Asynchronous]
		public void VisualParentTest ()
		{
			Button b = new Button();
			ContentControl c = new ContentControl {
				Content = b
			};

			CreateAsyncTest (c, () => {
				Assert.VisualParent (b,
					new VisualNode<ContentPresenter> ("#1",
						new VisualNode<ContentControl> ("#2")
					)
				);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void VisualParentTest2 ()
		{
			ControlTemplate template = CreateTemplate (@"
<ContentControl>
	<ContentPresenter/>
</ContentControl>
");
			ContentControl c = new ContentControl {
				Template = template,
				Content = new ContentControl ()
			};

			CreateAsyncTest (c, () => {
				Assert.VisualParent (c.Content as UIElement,
					new VisualNode<ContentPresenter> ("#1",
						new VisualNode<ContentPresenter> ("#2")
					)
				);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void VisualChildTest ()
		{
			Button b = new Button ();
			ContentControl c = new ContentControl {
				Content = b
			};

			CreateAsyncTest (c, () => {
				Assert.VisualChildren (c,
					new VisualNode<ContentPresenter> ("#1",
						new VisualNode<ContentControl> ("#2", (VisualNode []) null)
					)
				);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void VisualChildTest2 ()
		{
			ControlTemplate template = CreateTemplate (@"
<ContentControl x:Name=""TemplateControl"">
	<ContentPresenter x:Name=""TemplatePresenter""/>
</ContentControl>
");
			ContentControl c = new ContentControl {
				Name = "Root",
				Template = template,
				Content = new ContentControl { Name = "Content" }
			};

			CreateAsyncTest (c, () => {
				Assert.VisualChildren (c,
					new VisualNode<ContentControl> ("#1", d => Assert.AreEqual ("TemplateControl", d.Name),
						new VisualNode<ContentPresenter> ("#2",
							new VisualNode<ContentPresenter> ("#3", d => Assert.AreEqual ("TemplatePresenter", d.Name),
								new VisualNode<ContentControl> ("#4", d => Assert.AreEqual ("Content", d.Name),
									new VisualNode<ContentPresenter> ("#5")
								)
							)
						)
					)
				);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void VisualTreeTest ()
		{
			ContentControl c = new ContentControl ();
			c.Content = new Rectangle ();

			Assert.VisualChildren (c, "#1"); // No visual children
			c.Measure (Size.Empty);
			Assert.VisualChildren (c, "#2",
				new VisualNode<Rectangle> ("#a", (VisualNode [ ]) null)
			);

			CreateAsyncTest (c, () => {
				Assert.VisualChildren (c, "#3",
					new VisualNode<ContentPresenter> ("#b",
						new VisualNode<Rectangle> ("#c")
					)
				);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void VisualTreeTest2 ()
		{
			ContentControl c = new ContentControl ();
			Assert.VisualChildren (c, "#1");
			Assert.IsFalse (c.ApplyTemplate (), "#2");

			c.Content = new Rectangle ();

			Assert.VisualChildren (c, "#3"); // No visual children
			Assert.IsTrue (c.ApplyTemplate (), "#4");

			Assert.VisualChildren (c, "#5",
				new VisualNode<Rectangle> ("#a", (VisualNode [ ]) null)
			);

			CreateAsyncTest (c, () => {
				Assert.VisualChildren (c, "#6",
					new VisualNode<ContentPresenter> ("#b",
						new VisualNode<Rectangle> ("#c")
					)
				);
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("Detailed comment in ContentPresenter.PrepareContentPresenter () explaining the issue")]
		public void VisualTreeTest3 ()
		{
			ContentControl c = new ContentControl ();

			c.Content = "I'm a string";

			Assert.VisualChildren (c, "#3"); // No visual children
			Assert.IsTrue (c.ApplyTemplate (), "#4");

			Assert.VisualChildren (c, "#5",
				new VisualNode<Grid> ("#a",
					new VisualNode<TextBlock> ("#b")
				)
			);

			TextBlock block = null;
			CreateAsyncTest (c,
				() => {
					Assert.VisualChildren (c, "#6",
						new VisualNode<ContentPresenter> ("#c",
							new VisualNode<Grid> ("#d",
								new VisualNode<TextBlock> ("#e", (b) => block = b)
							)
						)
					);
				},
				// This is probably a once off binding
				() => Assert.IsInstanceOfType<BindingExpressionBase> (block.ReadLocalValue (TextBlock.TextProperty), "#6")
			);
		}
		
		[TestMethod]
		public void VisualTreeTest3b ()
		{
			// Check whether the grid + TextBlock is reused or replaced
			Grid grid = null;
			TextBlock textBlock = null;
			ContentControl c = new ContentControl ();

			// If there is no control template, a Grid + TextBlock is
			// appended.
			c.Content = "I'm a string";
			Assert.IsTrue (c.ApplyTemplate (), "#1");
			Assert.VisualChildren (c, "#2",
				new VisualNode<Grid> ("#a", g => grid = g,
					new VisualNode<TextBlock> ("#b", b => textBlock = b)
				)
			);
			Assert.IsNull (textBlock.DataContext, "#3");
			Assert.AreEqual ("", textBlock.Text, "#4");

			// Changing the content to anther non-UIElement does not change
			// the grid/textblock instance
			c.Content = "Other string";
			Assert.IsFalse (c.ApplyTemplate (), "#5");
			Assert.VisualChildren (c, "#6",
				new VisualNode<Grid> ("#a", g => Assert.AreSame (g, grid, "#b"),
					new VisualNode<TextBlock> ("#c", b => Assert.AreSame (textBlock, b))
				)
			);
			Assert.AreEqual ("", textBlock.Text, "#7");
		}
		
		[TestMethod]
		public void VisualTreeTest4 ()
		{
			ContentControl c = (ContentControl) XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007"">
    <ContentControl.Template>
		<ControlTemplate>
			<ContentPresenter />
		</ControlTemplate>
    </ContentControl.Template>
</ContentControl>");

			c.Content = new ConcreteFrameworkElement ();

			// No children
			Assert.VisualChildren (c, "#1");
			Assert.IsTrue (c.ApplyTemplate (), "#2");
			
			// Templated contents have been attached
			Assert.VisualChildren (c, "#3",
				new VisualNode<ContentPresenter> ("#4")
			);

			// The Presenter attaches itself on the call to Measure
			c.Measure (Size.Empty);
			Assert.VisualChildren (c, "#5",
				new VisualNode<ContentPresenter> ("#6",
					new VisualNode<ConcreteFrameworkElement>("#7")
				)
			);

			// This clears the template completely
			c.Content = new Rectangle ();

			// No children
			Assert.VisualChildren (c, "#8"); // Fails in Silverlight 3
			Assert.IsTrue (c.ApplyTemplate (), "#9");

			// Templated contents have been attached
			Assert.VisualChildren (c, "#10",
				new VisualNode<ContentPresenter> ("#11")
			);

			// The Presenter attaches itself on the call to Measure
			c.Measure (Size.Empty);
			Assert.VisualChildren (c, "#12",
				new VisualNode<ContentPresenter> ("#13",
					new VisualNode<Rectangle> ("#14")
				)
			);
		}
		
		[TestMethod]
		[Asynchronous]
		public void VisualTreeTest5 ()
		{
			ContentPresenter presenter = null;
			ContentControl c = new ContentControl ();
			c.Content = "I'm a string";
			c.Measure (Size.Empty);

			CreateAsyncTest (c,
				() => {
					Assert.VisualChildren (c, "#1",
						new VisualNode<ContentPresenter> ("#a", p => presenter = p, null)
					);
					Assert.AreEqual (c.Content, presenter.DataContext, "#2");

					c.Content = new ConcreteFrameworkElement ();
				},
				() => {
					ContentPresenter old = presenter;
					Assert.VisualChildren (c, "#3",
						new VisualNode<ContentPresenter> ("#b", p => presenter = p, null)
					);
					Assert.AreNotSame (old, presenter, "#4"); // Fails in Silverlight 3
					Assert.IsNull (presenter.DataContext, "#5");
				}
			);
		}

		static ControlTemplate CreateTemplate (string content)
		{
			return (ControlTemplate) XamlReader.Load (@"
<ControlTemplate xmlns=""http://schemas.microsoft.com/client/2007""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	" + content + @"
</ControlTemplate>");
		}

		static DataTemplate CreateDataTemplate (string content)
		{
			return (DataTemplate) XamlReader.Load (@"
<DataTemplate xmlns=""http://schemas.microsoft.com/client/2007""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	" + content + @"
</DataTemplate>");
		}
	}
}
