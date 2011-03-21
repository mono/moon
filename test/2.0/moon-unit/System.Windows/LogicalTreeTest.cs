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
using System.Windows.Shapes;

using Mono.Moonlight.UnitTesting;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

    public class ConcreteFrameworkElement : FrameworkElement
    {
        public static readonly DependencyProperty LocalContentProperty = DependencyProperty.Register("LocalContent",
                                                          typeof(FrameworkElement),
                                                          typeof(ConcreteFrameworkElement),
                                                          null);

        public FrameworkElement LocalContent
        {
            get { return (FrameworkElement)GetValue(LocalContentProperty); }
            set { SetValue(LocalContentProperty, value); }
        }

        public FrameworkElement NoDP
        {
            get;
            set;
        }
    }

	[TestClass]
	public class LogicalTreeTest {

		[TestMethod]
		public void BrushWithTwoParents_TwoSubtrees_SetOneFirst()
		{
			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");

			var first = (Canvas)XamlReader.Load(@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (Canvas)XamlReader.Load(@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			first.Children.Add(new Canvas { Background = brush });
			second.Children.Add(new Canvas { Background = brush });

			Assert.AreSame(brush, first.FindName("Test"), "#1");
			Assert.IsNull(second.FindName("Test"), "#2");
		}

		[TestMethod]
		public void BrushWithTwoParents_TwoSubtrees_SetBothFirst()
		{
			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");

			var first = (Canvas)XamlReader.Load(@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (Canvas)XamlReader.Load(@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			var subtree1 = new Canvas { Background = brush };
			var subtree2 = new Canvas { Background = brush };

			first.Children.Add(subtree1);
			second.Children.Add(subtree2);

			Assert.IsNull (first.FindName("Test"), "#1");
			Assert.IsNull(second.FindName("Test"), "#2");
		}

		[TestMethod]
		public void BrushWithOneParent_Null()
		{
			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");

			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			first.Foreground = brush;

			first.Foreground = null;
			Assert.IsNull (first.FindName("Test"), "#1");
		}

		[TestMethod]
		public void BrushWithTwoParents_SecondCannotFindName()
		{
			// Create two TextBlocks with unique namescopes and add the same brush to  both
			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");
			first.Foreground = brush;
			second.Foreground = brush;

			Assert.AreSame(brush, first.FindName("Test"), "#1");
			Assert.IsNull(second.FindName("Test"), "#2");
		}

		[TestMethod]
		public void BrushWithTwoParents_NullFirst_ThenPutInNewNamescope()
		{
			// Create two TextBlocks with unique namescopes and add the same brush to  both
			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");
			first.Foreground = brush;
			second.Foreground = brush;

			first.Foreground = null;

			var third = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			third.Foreground = brush;
			Assert.IsNull(third.FindName("Test"), "#1");
		}

		[TestMethod]
		public void BrushWithTwoParents_NullSecond_ThenPutInNewNamescope()
		{
			// Create two TextBlocks with unique namescopes and add the same brush to  both
			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");
			first.Foreground = brush;
			second.Foreground = brush;

			second.Foreground = null;

			var third = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			third.Foreground = brush;
			Assert.IsNull(third.FindName("Test"), "#1");
		}

		[TestMethod]
		public void BrushWithTwoParents_NullBoth()
		{
			// Create two TextBlocks with unique namescopes and add the same brush to  both
			// By setting the Foreground properties to null, the brush is still in the namescope
			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");
			first.Foreground = brush;
			second.Foreground = brush;

			first.Foreground = null;
			second.Foreground = null;
			Assert.AreSame(brush, first.FindName("Test"), "#1");
			Assert.IsNull(second.FindName("Test"), "#2");
		}

		[TestMethod]
		public void BrushWithTwoParents_NullBoth_NamescopeClash()
		{
			// Create two TextBlocks with unique namescopes and add the same brush to  both
			// If we then clear the two Foregrounds by setting them to null, the name is never
			// unregistered from the first namescope so if we try to reuse the name we blow up.
			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");
			first.Foreground = brush;
			second.Foreground = brush;

			first.Foreground = null;
			second.Foreground = null;

			brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");

			Assert.Throws<ArgumentException>(() => first.Foreground = brush, "#1");
		}

		[TestMethod]
		public void BrushWithTwoParents_NullBoth_ThenPutInNewNamescope()
		{
			// Create two TextBlocks with unique namescopes and add the same brush to  both.
			// Then null the properties and add the brush to a third namescope to see if it
			// is registered there.
			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");

			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			first.Foreground = brush;
			second.Foreground = brush;

			first.Foreground = null;
			second.Foreground = null;

			var third = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			third.Foreground = brush;
			Assert.AreSame (brush, third.FindName ("Test"), "#1");
		}

		[TestMethod]
		public void BrushWithThreeParents_NullTwo_ThenPutInNewNamescope()
		{
			// Create two TextBlocks with unique namescopes and add the same brush to  both.
			// Then null the properties and add the brush to a third namescope to see if it
			// is registered there.
			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");

			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var third = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			first.Foreground = brush;
			second.Foreground = brush;
			third.Foreground = brush;

			first.Foreground = null;
			second.Foreground = null;

			var fourth = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			fourth.Foreground = brush;
			Assert.IsNull(fourth.FindName("Test"), "#1");
		}

		[TestMethod]
		public void BrushWithThreeParents_NullAll_ThenPutInNewNamescope()
		{
			// Create two TextBlocks with unique namescopes and add the same brush to  both.
			// Then null the properties and add the brush to a third namescope to see if it
			// is registered there.
			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");

			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var third = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var fourth = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			first.Foreground = null;
			second.Foreground = null;
			third.Foreground = null;
			fourth.Foreground = null;

			first.Foreground = brush;
			second.Foreground = brush;
			third.Foreground = brush;

			first.Foreground = null;
			second.Foreground = null;
			third.Foreground = null;

			fourth.Foreground = brush;
			Assert.AreSame(brush, fourth.FindName("Test"), "#1");
		}

		[TestMethod]
		public void BrushWithThreeParents_NullInParentOrder()
		{
			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");

			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var third = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			first.Foreground = brush;
			second.Foreground = brush;
			third.Foreground = brush;

			first.Foreground = null;
			second.Foreground = null;
			third.Foreground = null;

			Assert.AreSame(brush, first.FindName("Test"), "#1");
			Assert.IsNull(second.FindName("Test"), "#1");
			Assert.IsNull(third.FindName("Test"), "#1");
		}

		[TestMethod]
		public void BrushWithThreeParents_NullInReverseOrder()
		{
			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");

			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var third = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			first.Foreground = brush;
			second.Foreground = brush;
			third.Foreground = brush;

			third.Foreground = null;
			second.Foreground = null;
			first.Foreground = null;

			Assert.IsNull(first.FindName("Test"), "#1");
			Assert.IsNull(second.FindName("Test"), "#1");
			Assert.IsNull(third.FindName("Test"), "#1");
		}

		[TestMethod]
		public void BrushWithTwoParents_UnregisterWrongObject()
		{
			// Create two brushes with the same name. Put one of them in both
			// textboxes and the other just in the second textbox. See if clearing
			// the first brush in the second textbox will unregister the name of the
			// second brush from the second textbox.
			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");
			
			var secondBrush = new SolidColorBrush();
			secondBrush.SetValue(FrameworkElement.NameProperty, "Test");

			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			first.Foreground = brush;
			second.Foreground = brush;

			second.Background = secondBrush;
			second.Foreground = null;

			Assert.AreSame(secondBrush, second.FindName("Test"), "#1");
		}


		[TestMethod]
		public void BrushWithThreeParents_ComplexParenting_AlternateNull()
		{
			var brush = new SolidColorBrush();
			brush.SetValue(FrameworkElement.NameProperty, "Test");

			var first = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var third = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var fourth = (TextBox)XamlReader.Load(@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			first.Foreground = brush;
			Assert.AreSame(brush, first.FindName("Test"), "#1a");
			Assert.IsNull(second.FindName("Test"), "#2a");
			Assert.IsNull(third.FindName("Test"), "#3a");
			Assert.IsNull(fourth.FindName("Test"), "#4a");

			second.Foreground = brush;
			first.Foreground = null;
			Assert.AreSame(brush, first.FindName("Test"), "#1b");
			Assert.IsNull(second.FindName("Test"), "#2b");
			Assert.IsNull(third.FindName("Test"), "#3b");
			Assert.IsNull(fourth.FindName("Test"), "#4b");

			third.Foreground = brush;
			second.Foreground = null;
			Assert.AreSame(brush, first.FindName("Test"), "#1c");
			Assert.IsNull(second.FindName("Test"), "#2c");
			Assert.IsNull(third.FindName("Test"), "#3c");
			Assert.IsNull(fourth.FindName("Test"), "#4c");

			fourth.Foreground = brush;
			third.Foreground = null;
			Assert.AreSame(brush, first.FindName("Test"), "#5a");
			Assert.IsNull(second.FindName("Test"), "#5b");
			Assert.IsNull(third.FindName("Test"), "#5c");
			Assert.IsNull(fourth.FindName("Test"), "#5d");

			fourth.Foreground = null;
			Assert.AreSame(brush, first.FindName("Test"), "#6a");
			Assert.IsNull(second.FindName("Test"), "#6b");
			Assert.IsNull(third.FindName("Test"), "#6c");
			Assert.IsNull(fourth.FindName("Test"), "#6d");
		}

		[TestMethod]
		public void UIElementWithTwoParents ()
		{
			var element = new ContentControl { Name = "Test" };

			var first = (ContentControl) XamlReader.Load (@"<ContentControl xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");
			var second = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" />");

			first.Content = element;
			second.Children.Add (element);

			Assert.AreSame (element, first.FindName ("Test"), "#1");
			Assert.AreSame (element, second.FindName ("Test"), "#2");
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
		public void LogicalParentTest6 ()
		{
			ConcreteFrameworkElement c = new ConcreteFrameworkElement ();
			ContentControl contentControl = new ContentControl ();
			Canvas canvas = new Canvas ();

			canvas.Children.Add (c);

			Assert.Throws<ArgumentException> ( delegate { contentControl.Content = c; } );

			Assert.AreEqual (canvas, c.Parent, "1");
			Assert.AreEqual (1, canvas.Children.Count, "2");
			Assert.IsNull (contentControl.Content, "3");
		}

		[TestMethod]
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

			Button b = (Button)canvas.Children[0];

			b.ApplyTemplate ();

			FrameworkElement b_child = (FrameworkElement)VisualTreeHelper.GetChild (b, 0);

			Assert.IsNull (b_child.Parent, "1");
		}

		[TestMethod]
		public void LogicalParentTest10 ()
		{
			ComboBox first = new ComboBox ();
			StackPanel second = new StackPanel ();
			Rectangle r = new Rectangle ();
			first.Items.Add (r);
			second.Children.Add (r);

			// When the item is removed from the combobox, its parent
			// should be set to null
			Assert.AreEqual (first, r.Parent, "#1");
			first.Items.Remove (r);
			Assert.IsNull (r.Parent, "#2");
		}

		[TestMethod]
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

		[TestMethod]
		[MoonlightBug]
		public void CustomPropertyFindName_XamlReader()
		{
			// Let alan know when this is fixed as drt 869 relies on this.
			var cf = (ConcreteFrameworkElement)XamlReader.Load(@"
<clr:ConcreteFrameworkElement
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
    xmlns:clr=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"">
    <clr:ConcreteFrameworkElement.LocalContent>
        <Rectangle x:Name=""Registered""/>
    </clr:ConcreteFrameworkElement.LocalContent>
    <clr:ConcreteFrameworkElement.NoDP>
        <Rectangle x:Name=""Hidden""/>
    </clr:ConcreteFrameworkElement.NoDP>
</clr:ConcreteFrameworkElement>
");
			Assert.IsNotNull(cf.FindName ("Registered"), "#1");
			Assert.IsNull(cf.FindName ("Hidden"), "#2");
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

		class ContentControlPoker : ContentControl
		{
			protected override void OnContentChanged (object oldContent, object newContent)
			{
				if (getParents) {
					oldParent = ((FrameworkElement)oldContent).Parent;
					newParent = ((FrameworkElement)newContent).Parent;
				}
			}

			public bool getParents;

			public DependencyObject oldParent;
			public DependencyObject newParent;
		}

		[TestMethod]
		public void ContentControlOldNewContent ()
		{
			ContentControlPoker cp = new ContentControlPoker ();
			ConcreteFrameworkElement cf1 = new ConcreteFrameworkElement ();
			ConcreteFrameworkElement cf2 = new ConcreteFrameworkElement ();

			cp.Content = cf1;

			cp.getParents = true;

			cp.Content = cf2;

			Assert.IsNull (cp.oldParent, "1");
			Assert.AreEqual (cp, cp.newParent, "2");
		}

		[TestMethod]
		public void BorderChild ()
		{
			Border b1 = new Border ();
			Border b2 = new Border ();
			ConcreteFrameworkElement cf = new ConcreteFrameworkElement ();

			b1.Child = cf;

			Assert.AreEqual (b1, cf.Parent, "1");

			Assert.Throws<ArgumentException> (delegate { b2.Child = cf; }, "2");

			Assert.AreEqual (b1, cf.Parent, "2");

			Assert.AreEqual (cf, b1.Child, "3");
			Assert.AreEqual (cf, b1.Child, "4");
		}
	}

}
