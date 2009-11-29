//
// DataTemplate Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.Windows.Controls;
using System.Windows;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Shapes;
using System.Windows.Media;
using System.Windows.Markup;
using Mono.Moonlight.UnitTesting;
using System.Windows.Media.Animation;

namespace MoonTest.System.Windows
{
	public class MyControl : ContentControl { }
	public class MyDataTemplate : DataTemplate { }

    [TestClass]
    public class DataTemplateTests : SilverlightTest
    {
        [TestMethod]
        [Asynchronous]
        public void CustomDataTemplate()
        {
            MyControl c = (MyControl)XamlReader.Load(@"
<x:MyControl xmlns=""http://schemas.microsoft.com/client/2007""
			xmlns:x=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"">
	<x:MyControl.ContentTemplate>
		<x:MyDataTemplate>
			<Rectangle />
		</x:MyDataTemplate>
	</x:MyControl.ContentTemplate>
</x:MyControl>");
            Rectangle r = new Rectangle();
            c.Content = r;
            c.ApplyTemplate();

            // Check that the visual tree matches up:
            // MyControl->ContentPresenter->Rectangle
            CreateAsyncTest(c,
                () =>
                {
                    Assert.VisualChildren(c,
                        new VisualNode<ContentPresenter>("#1",
                            new VisualNode<Rectangle>("#2")
                        )
                    );
                }
            );
        }

        [TestMethod]
        public void DataTemplateNamescopeTest()
        {
            Grid grid = (Grid)XamlReader.Load(@"
<Grid	xmlns=""http://schemas.microsoft.com/client/2007""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Grid.Resources>
		<DataTemplate x:Key=""Template"">
			<Grid x:Name=""A"">
				<Grid x:Name=""B"" />
			</Grid>
		</DataTemplate>
	</Grid.Resources>
</Grid>");
            DataTemplate t = (DataTemplate)grid.Resources["Template"];
            Grid root = (Grid)t.LoadContent();
            Assert.AreEqual(root, root.FindName("A"), "#1");
            Assert.IsInstanceOfType<Grid>(root.FindName("B"), "#2");

            TestPanel.Children.Add(root);
            Assert.IsNull(TestPanel.FindName("A"), "#3");
            Assert.IsNull(TestPanel.FindName("B"), "#4");
        }

		[TestMethod]
		[MoonlightBug]
		public void InvalidContentTemplate ()
		{
			Assert.Throws<XamlParseException> (() => {
				XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<ContentControl.ContentTemplate>
		<DataTemplate>
			INVALID!
		</DataTemplate>
	</ContentControl.ContentTemplate>
</ContentControl>");
			});
		}

		[TestMethod]
		public void NonUIElementRoot ()
		{
			var c = CreateTemplated ("<Storyboard />");
			Assert.IsInstanceOfType<Storyboard> (c.ContentTemplate.LoadContent (), "#1");
		}

		[TestMethod]
		[MoonlightBug]
		public void UseNonUIElementRoot ()
		{
			// Something in the default template notices that the
			// DataTemplate isn't giving a UIElement and throws an exception
			var c = CreateTemplated ("<Storyboard />");
			c.Content = "Content";
			c.ApplyTemplate ();

			Assert.VisualChildren (c,
				new VisualNode<ContentPresenter> ("#2")
			);

			Assert.Throws<ArgumentException> (() => c.Measure (Size.Empty), "#3");
		}

		[TestMethod]
		public void UseNonUIElementRoot2 ()
		{
			// If we don't use the default template we don't get an exception
			var c = CreateTemplated ("<Storyboard />");
			c.Content = "Content";
			c.Template = null;
			c.ApplyTemplate ();

			Assert.VisualChildren (c,
				new VisualNode<Grid> ("#2",
					new VisualNode<TextBlock> ("#3")
				)
			);

			// No exception this time
			c.Measure (Size.Empty);
		}

		[TestMethod]
		[MoonlightBug]
		public void NonUIElementTemplateRoot ()
		{
			var c = CreateTemplated (null, "<Storyboard />");
			Assert.Throws<XamlParseException>(() => c.ApplyTemplate (), "#1");
		}

		[TestMethod]
		[Asynchronous]
		public void GridRoot ()
		{
			var c = CreateTemplated ("<Grid />");
			c.Content = new Ellipse {Width =100, Height = 100, Fill = new SolidColorBrush(Colors.Red) };
			CreateAsyncTest (c,
				() => c.ApplyTemplate (),
				() => {
					Assert.VisualChildren (c,
						new VisualNode<ContentPresenter> ("#1",
							new VisualNode<Grid>("#2")
						)
					);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void GridRoot2 ()
		{
			var c = CreateTemplated ("<Grid />");
			c.Template = null;
			c.Content = new Ellipse { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Red) };
			CreateAsyncTest (c,
				() => c.ApplyTemplate (),
				() => {
					Assert.VisualChildren (c,
						new VisualNode<Ellipse> ("#1"));
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void GridRoot3 ()
		{
			var c = CreateTemplated ("<StackPanel />", "<ContentPresenter />");
			c.Content = new Ellipse { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Red) };
			CreateAsyncTest (c,
				() => c.ApplyTemplate (),
				() => {
					Assert.VisualChildren (c,
						new VisualNode<ContentPresenter> ("#1",
							new VisualNode<StackPanel> ("#2")
						)
					);
				}
			);
		}


		[TestMethod]
		public void EmptyTemplates ()
		{
			var c = CreateTemplated ("", "");
			c.Content = "Hi";
			Assert.IsTrue (c.ApplyTemplate (), "#1");
			Assert.IsFalse (c.ApplyTemplate (), "#2");

			Assert.VisualChildren (c,
				new VisualNode<Grid> ("#3",
					new VisualNode<TextBlock> ("#4")
				)
			);

			c = CreateTemplated ("", "");
			Assert.IsFalse (c.ApplyTemplate (), "#5");
			Assert.VisualChildren (c, "#6");
		}

		[TestMethod]
		public void EmptyContentTemplate ()
		{
			var c = CreateTemplated ("", "<StackPanel x:Name=\"A\" />");
			c.Content = "Hi";
			Assert.IsTrue (c.ApplyTemplate (), "#1");
			Assert.IsFalse (c.ApplyTemplate (), "#2");
			Assert.VisualChildren (c,
				new VisualNode<StackPanel> ("#3", p => Assert.AreEqual ("A", p.Name, "#4"))
			);

			c = CreateTemplated ("", "<StackPanel x:Name=\"A\" />");
			Assert.IsTrue (c.ApplyTemplate (), "#5");
			Assert.VisualChildren (c, "#6",
				new VisualNode<StackPanel> ("#7", p => Assert.AreEqual ("A", p.Name))
			);
		}

		[TestMethod]
		public void EmptyTemplate ()
		{
			var c = CreateTemplated ("<StackPanel x:Name=\"A\" />", "");
			c.Content = "Hi";
			Assert.IsTrue (c.ApplyTemplate (), "#1");
			Assert.IsFalse (c.ApplyTemplate (), "#2");
			Assert.VisualChildren (c,
				new VisualNode<Grid> ("#3",
					new VisualNode<TextBlock> ("#4")
				)
			);

			c = CreateTemplated ("<StackPanel x:Name=\"A\" />", "");
			Assert.IsFalse (c.ApplyTemplate (), "#5");
			Assert.VisualChildren (c, "#6");
		}

		[TestMethod]
		public void NullTemplates ()
		{
			var c = CreateTemplated ("<StackPanel x:Name=\"A\" />", "<StackPanel x:Name=\"B\" />");
			c.Content = "Hi";
			c.Template = null;
			c.ContentTemplate = null;
			Assert.IsTrue (c.ApplyTemplate (), "#1");
			Assert.IsFalse (c.ApplyTemplate (), "#2");

			Assert.VisualChildren (c,
				new VisualNode<Grid> ("#3",
					new VisualNode<TextBlock> ("#4")
				)
			);

			c = CreateTemplated ("<StackPanel x:Name=\"A\" />", "<StackPanel x:Name=\"B\" />");
			c.Template = null;
			c.ContentTemplate = null;
			Assert.IsFalse (c.ApplyTemplate (), "#6");
			Assert.VisualChildren (c, "#7");
		}

		[TestMethod]
		public void NullContentTemplate ()
		{
			var c = CreateTemplated ("<StackPanel x:Name=\"A\" />", "<StackPanel x:Name=\"B\" />");
			c.Content = "Hi";
			c.ContentTemplate = null;
			Assert.IsTrue (c.ApplyTemplate (), "#1");
			Assert.IsFalse (c.ApplyTemplate (), "#2");
			Assert.VisualChildren (c,
				new VisualNode<StackPanel> ("#3")
			);

			c = CreateTemplated ("<StackPanel x:Name=\"A\" />", "<StackPanel x:Name=\"B\" />");
			c.ContentTemplate = null;
			Assert.IsTrue (c.ApplyTemplate (), "#4");
			Assert.VisualChildren (c,
				new VisualNode<StackPanel>("#5")
			);
		}

		[TestMethod]
		public void NullTemplate ()
		{
			var c = CreateTemplated ("<StackPanel x:Name=\"A\" />", "<StackPanel x:Name=\"B\" />");
			c.Content = "Hi";
			c.Template= null;
			Assert.IsTrue (c.ApplyTemplate (), "#1");
			Assert.IsFalse (c.ApplyTemplate (), "#2");
			Assert.VisualChildren (c,
				new VisualNode<Grid> ("#3",
					new VisualNode<TextBlock> ("#4")
				)
			);

			c = CreateTemplated ("<StackPanel x:Name=\"A\" />", "<StackPanel x:Name=\"B\" />");
			c.Template = null;
			Assert.IsFalse (c.ApplyTemplate (), "#5");
			Assert.VisualChildren (c);
		}

		[TestMethod]
		public void BothTemplates ()
		{
			var c = CreateTemplated ("<StackPanel x:Name=\"A\" />", "<StackPanel x:Name=\"B\" />");
			c.Content = "Hi";
			Assert.IsTrue (c.ApplyTemplate (), "#1");
			Assert.IsFalse (c.ApplyTemplate (), "#2");
			Assert.VisualChildren(c,
				new VisualNode<StackPanel>("#3", p => Assert.AreEqual("B", p.Name, "#6"))
			);
		}

		ContentControl CreateTemplated(string datatemplate)
		{
			return CreateTemplated (datatemplate, null);
		}

		ContentControl CreateTemplated (string datatemplate, string template)
		{
			if (datatemplate != null)
				datatemplate = "<ContentControl.ContentTemplate><DataTemplate>" + datatemplate + "</DataTemplate></ContentControl.ContentTemplate>";
			else
				datatemplate = "";

			if (template != null)
				template = "<ContentControl.Template><ControlTemplate>" + template + "</ControlTemplate></ContentControl.Template>";
			else
				template = "";
			return (ContentControl) XamlReader.Load (@"
<ContentControl xmlns=""http://schemas.microsoft.com/client/2007""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	" + template + datatemplate + @"
</ContentControl>");
		}

		public static DataTemplate Create (string content)
		{
			return (DataTemplate) XamlReader.Load (@"
<DataTemplate xmlns=""http://schemas.microsoft.com/client/2007""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	" + content + @"
</DataTemplate>");
		}
    }
}