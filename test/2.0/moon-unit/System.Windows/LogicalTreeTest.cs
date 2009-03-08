//
// Unit Tests dealing with the logical tree
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

namespace MoonTest.System.Windows {

	[TestClass]
	public class LogicalTreeTest {

		class ConcreteFrameworkElement : FrameworkElement {
			public static readonly DependencyProperty LocalContentProperty = DependencyProperty.Register ("LocalContent",
														      typeof (FrameworkElement),
														      typeof (ConcreteFrameworkElement),
														      null);

			public FrameworkElement LocalContent {
				get { return (FrameworkElement)GetValue(LocalContentProperty); }
				set { SetValue(LocalContentProperty, value); }
			}
		}

		[TestMethod]
		public void LogicalParentTest1 ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			Canvas canvas = new Canvas ();

			canvas.Children.Add (c);

			Assert.AreEqual (canvas, c.Parent);
		}

		[TestMethod]
		public void LogicalParentTest2 ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			Canvas canvas1 = new Canvas ();
			Canvas canvas2 = new Canvas ();

			canvas1.Children.Add (c);
			Assert.Throws<InvalidOperationException> (delegate { canvas2.Children.Add (c); }, "1");

			Assert.AreEqual (canvas1, c.Parent, "2");

			Assert.AreEqual (1, canvas1.Children.Count, "3");
			Assert.AreEqual (0, canvas2.Children.Count, "4");
		}

		[TestMethod]
		public void LogicalParentTest3 ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			ContentControl contentControl = new ContentControl ();

			contentControl.Content = c;

			Assert.AreEqual (contentControl, c.Parent);
		}

		[TestMethod]
		public void LogicalParentTest4 ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			ContentControl contentControl1 = new ContentControl ();
			ContentControl contentControl2 = new ContentControl ();

			contentControl1.Content = c;
			Assert.Throws<InvalidOperationException> (delegate { contentControl2.Content = c; }, "1");

			Assert.AreEqual (c, contentControl1.Content, "2");
			Assert.AreEqual (c, contentControl2.Content, "3");

			Assert.AreEqual (contentControl1, c.Parent, "4");
		}

		[TestMethod]
		public void LogicalParentTest5 ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			ContentControl contentControl = new ContentControl ();
			Canvas canvas = new Canvas ();

			contentControl.Content = c;
			canvas.Children.Add (c);

			Assert.AreEqual (contentControl, c.Parent, "1");
			Assert.AreEqual (1, canvas.Children.Count, "2");
		}

		[TestMethod]
		[MoonlightBug]
		public void LogicalParentTest6 ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			ContentControl contentControl = new ContentControl ();
			Canvas canvas = new Canvas ();

			canvas.Children.Add (c);

			Assert.Throws<ArgumentException> ( delegate { contentControl.Content = c; } );

			Assert.AreEqual (canvas, c.Parent, "1");
			Assert.AreEqual (1, canvas.Children.Count, "2");
			Assert.IsNull (contentControl.Content);
		}

		[TestMethod]
		[MoonlightBug]
		public void LogicalParentTest7 ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			ContentControl contentControl = new ContentControl ();
			ContentPresenter contentPresenter = new ContentPresenter ();

			contentControl.Content = c;
			contentPresenter.Content = c;

			Assert.AreEqual (contentControl, c.Parent, "1");
			Assert.AreEqual (c, contentControl.Content, "2");
			Assert.AreEqual (c, contentPresenter.Content, "3");

			c = new ConcreteFrameworkElement ();
			contentControl = new ContentControl ();
			contentPresenter = new ContentPresenter ();

			contentPresenter.Content = c;

			Assert.IsNull (c.Parent, "4");

			contentControl.Content = c;

			Assert.AreEqual (contentControl, c.Parent, "5");
			Assert.AreEqual (c, contentControl.Content, "6");
			Assert.AreEqual (c, contentPresenter.Content, "7");
		}
		
		[TestMethod]
		public void LogicalParentTest8 ()
		{
			Canvas canvas = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Canvas.Resources>
    <Style TargetType=""Button"" x:Key=""ButtonStyle"">
      <Setter Property=""Template"">
        <Setter.Value>
          <ControlTemplate TargetType=""Button"">
	    <ContentPresenter Content=""{TemplateBinding Content}"" />
          </ControlTemplate>
        </Setter.Value>
      </Setter>
    </Style>
  </Canvas.Resources>
  <Button Style=""{StaticResource ButtonStyle}"" />
</Canvas>
");

			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			
			Button b = (Button)canvas.Children[0];

			b.ApplyTemplate ();

			ContentPresenter cp = (ContentPresenter)VisualTreeHelper.GetChild (b, 0);

			b.Content = c;

			Assert.AreEqual (c, cp.Content, "1");
			Assert.AreEqual (c, b.Content, "2");
			Assert.AreEqual (b, c.Parent, "3");
		}

		[TestMethod]
		public void LogicalParentTest9 ()
		{
			Canvas canvas = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Button />
</Canvas>
");

			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			
			Button b = (Button)canvas.Children[0];

			b.ApplyTemplate ();

			FrameworkElement b_child = (FrameworkElement)VisualTreeHelper.GetChild (b, 0);

			Assert.IsNull (b_child.Parent, "1");
		}

		[TestMethod]
		[MoonlightBug]
		public void CustomPropertyParent ()
		{
			ConcreteFrameworkElement cf1 = new ConcreteFrameworkElement ();
			ConcreteFrameworkElement cf2 = new ConcreteFrameworkElement ();

			cf1.LocalContent = cf2;

			Assert.IsNull (cf2.Parent, "1");
		}

		[TestMethod]
		public void BuiltinPropertyFindName ()
		{
			Canvas c = new Canvas ();
			ConcreteFrameworkElement cf1 = new ConcreteFrameworkElement ();

			cf1.Name = "MyName";

			c.Children.Add (cf1);

			Assert.IsNull (c.FindName ("MyName"), "1");
		}

		[TestMethod]
		public void CustomPropertyFindName ()
		{
			ConcreteFrameworkElement cf1 = new ConcreteFrameworkElement ();
			ConcreteFrameworkElement cf2 = new ConcreteFrameworkElement ();

			cf2.Name = "MyName";

			cf1.LocalContent = cf2;

			Assert.IsNull (cf1.FindName ("MyName"), "1");
		}

		class UserControlPoker : UserControl 
		{
			public void SetContent (UIElement element)
			{
				Content = element;
			}
		}

		[TestMethod]
		public void UserControlContentParent ()
		{
			UserControlPoker uc = new UserControlPoker ();
			ConcreteFrameworkElement cf = new ConcreteFrameworkElement ();

			uc.SetContent (cf);

			Assert.AreEqual (uc, cf.Parent, "1");
		}
	}

}