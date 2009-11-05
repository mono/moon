//
// Unit tests for PropertyPath
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
using System.Windows.Shapes;
using System.Windows.Controls;
using System.Windows.Markup;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Media.Animation;

namespace MoonTest.System.Windows
{
	[TestClass]
	public class PropertyPathTest
	{
		[TestMethod]
		public void PropertyPathCtors ()
		{
			PropertyPath prop;
			
			Assert.Throws <ArgumentOutOfRangeException>(() =>
				prop = new PropertyPath ("first", "second")
			, "#1");
			
			Assert.Throws<ArgumentOutOfRangeException>(delegate {
				prop = new PropertyPath (null, "arg1");
			}, "null path throws ArgumentOutOfRangeException");
			
			prop = new PropertyPath (Rectangle.WidthProperty);
			Assert.AreEqual ("(0)", prop.Path, "Normal PropertyPath");
			
			prop = new PropertyPath (Canvas.LeftProperty);
			Assert.AreEqual ("(0)", prop.Path, "Attached PropertyPath");
			
			prop = new PropertyPath (5);
			Assert.IsNull (prop.Path, "numeric PropertyPath is null");

			Assert.Throws<NullReferenceException> (delegate {
				prop = new PropertyPath (null, null);
			}, "Both null");

			Assert.Throws<NullReferenceException> (delegate {
				prop = new PropertyPath ("pathstring", null);
			}, "second null");

			prop = new PropertyPath ((string) null);
			Assert.IsNull (prop.Path, "null PropertyPath is null");
		}

		[TestMethod]
		public void SetName ()
		{
			Storyboard sb = new Storyboard ();
			PropertyPath path = new PropertyPath ("Width");
			Storyboard.SetTargetProperty (sb, path);
			PropertyPath native = Storyboard.GetTargetProperty (sb);

			Assert.AreNotEqual (path, native, "#1");
			Assert.IsFalse (path == native, "#2");
			Assert.AreEqual(path.Path, native.Path, "#3");
			Assert.AreEqual (path.Path, "Width", "#4");
		}


		[TestMethod]
		public void SetToStoryboard ()
		{
			Storyboard sb = new Storyboard ();
			PropertyPath path = new PropertyPath ("Width");
			Storyboard.SetTargetProperty (sb, path);
			PropertyPath native = Storyboard.GetTargetProperty (sb);

			Assert.AreNotEqual (path, native, "#1");
			Assert.IsFalse (path == native, "#2");
			Assert.AreEqual (path.Path, native.Path, "#3");
			Assert.AreEqual (path.Path, "Width", "#4");
		}

		[TestMethod]
		public void SetToStoryboard2 ()
		{
			Storyboard sb = new Storyboard ();
			PropertyPath path = new PropertyPath (null);
			Assert.Throws<NullReferenceException> (delegate {
				Storyboard.SetTargetProperty (sb, path);
			}, "#A");
		}

		[TestMethod]
		public void SetToStoryboard3 ()
		{
			Storyboard sb = new Storyboard ();
			PropertyPath path = new PropertyPath (Storyboard.FillBehaviorProperty);
			Storyboard.SetTargetProperty (sb, path);
			PropertyPath native = Storyboard.GetTargetProperty (sb);

			Assert.AreEqual (path.Path, "(0)", "#1");
			Assert.IsNull (native, "#2");
		}

		[TestMethod]
		public void SetToStoryboard4 ()
		{
			Storyboard sb = new Storyboard ();
			PropertyPath path = new PropertyPath (Rectangle.WidthProperty);
			Storyboard.SetTargetProperty (sb, path);
			PropertyPath native = Storyboard.GetTargetProperty (sb);

			Assert.AreEqual (path.Path, "(0)", "#1");
			Assert.IsNull (native, "#2");
		}

		[TestMethod]
		public void SetToTimeline ()
		{
			DoubleAnimation sb = new DoubleAnimation ();
			PropertyPath path = new PropertyPath ("Width");
			Storyboard.SetTargetProperty (sb, path);
			PropertyPath native = Storyboard.GetTargetProperty (sb);

			Assert.AreNotEqual (path, native, "#1");
			Assert.IsFalse (path == native, "#2");
			Assert.AreEqual (path.Path, native.Path, "#3");
			Assert.AreEqual (path.Path, "Width", "#4");
		}

		[TestMethod]
		public void SetToTimeline2 ()
		{
			DoubleAnimation sb = new DoubleAnimation ();
			PropertyPath path = new PropertyPath (null);
			Assert.Throws<NullReferenceException> (delegate {
				Storyboard.SetTargetProperty (sb, path);
			}, "#A");
		}

		[TestMethod]
		public void SetToTimeline3 ()
		{
			DoubleAnimation sb = new DoubleAnimation ();
			PropertyPath path = new PropertyPath (Storyboard.FillBehaviorProperty);
			Storyboard.SetTargetProperty (sb, path);
			PropertyPath native = Storyboard.GetTargetProperty (sb);

			Assert.AreEqual (path.Path, "(0)", "#1");
			Assert.IsNull (native, "#2");
		}

		[TestMethod]
		public void SetToTimeline4 ()
		{
			DoubleAnimation sb = new DoubleAnimation ();
			PropertyPath path = new PropertyPath (Rectangle.WidthProperty);
			Storyboard.SetTargetProperty (sb, path);
			PropertyPath native = Storyboard.GetTargetProperty (sb);

			Assert.AreEqual (path.Path, "(0)", "#1");
			Assert.IsNull (native, "#2");
		}
		
		[TestMethod]
		public void InvalidPropertyPath1 ()
		{
			/* this specific TargetProperty ("[12].") was causing a crash */
			Canvas c = (Canvas) XamlReader.Load (
				@"<Canvas
    xmlns=""http://schemas.microsoft.com/client/2007""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Canvas.Triggers>
    <EventTrigger RoutedEvent=""Canvas.Loaded"">
      <EventTrigger.Actions>
        <BeginStoryboard>
          <Storyboard x:Name=""sb"">
	     <ColorAnimation Storyboard.TargetName=""e1_brush"" Storyboard.TargetProperty=""[12]."" From=""Red"" To=""Blue"" Duration=""0:0:5"" />
          </Storyboard>
        </BeginStoryboard>
      </EventTrigger.Actions>
    </EventTrigger>
  </Canvas.Triggers>
  
  <Ellipse Fill=""Black""
      Height=""100"" Width=""100"" Canvas.Left=""30"" Canvas.Top=""30"" x:Name=""e1_brush"" >
  </Ellipse>
 
</Canvas>
");
			Storyboard sb = (Storyboard) c.FindName ("sb");
			Assert.Throws<InvalidOperationException> (() => sb.Begin ());
		}
	}
}