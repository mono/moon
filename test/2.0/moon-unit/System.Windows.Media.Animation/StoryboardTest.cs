//
// Unit tests for System.Windows.Media.Animation.Storyboard
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008,2009 Novell, Inc (http://www.novell.com)
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
using System.Windows.Media.Animation;
using System.Windows.Shapes;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Media.Animation {

	public class Test : UserControl {

		public Storyboard Blah;
	}
	
	[TestClass]
	public class StoryboardTest : SilverlightTest {

		[TestMethod]
		public void InvalidValues_NonTimeline ()
		{
			Rectangle r = new Rectangle ();
			Assert.Throws<Exception> (delegate {
				r.SetValue (Storyboard.TargetNameProperty, null);
			}, "#1");
			Assert.Throws<Exception> (delegate {
				r.SetValue (Storyboard.TargetNameProperty, "");
			}, "#1b");
			Assert.Throws<Exception> (delegate {
				r.SetValue (Storyboard.TargetNameProperty, "X");
			}, "#1c");
			Assert.Throws<Exception> (delegate {
				r.SetValue (Storyboard.TargetPropertyProperty, null);
			}, "2");
		}

		[TestMethod]
		public void ValidValues_Timeline ()
		{
			Timeline t = (Timeline) new ColorAnimation ();
			t.SetValue (Storyboard.TargetNameProperty, null);
			t.SetValue (Storyboard.TargetNameProperty, String.Empty);
			t.SetValue (Storyboard.TargetNameProperty, "X");
			t.SetValue (Storyboard.TargetPropertyProperty, null);
		}

		[TestMethod]
		public void NameAndKey()
		{
			Storyboard board = (Storyboard)XamlReader.Load(
@"		
<Storyboard xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
            x:Name=""Blah""
            x:Key=""Blah"" />
");
			Assert.AreEqual("Blah", board.GetValue(FrameworkElement.NameProperty), "#1");
			board = (Storyboard)XamlReader.Load(
@"	
<Storyboard xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
            x:Name=""Blah2""
            x:Key=""Blah"" />
");
			Assert.AreEqual("Blah2", board.GetValue(FrameworkElement.NameProperty), "#1");
			board = (Storyboard)XamlReader.Load(
@"	
<Storyboard xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
            x:Key=""Blah""
            x:Name=""Blah2"" />
");
			Assert.AreEqual("Blah2", board.GetValue(FrameworkElement.NameProperty), "#1");
		}

		[TestMethod]
		public void NameAndKey_Resource ()
		{
			Canvas c = (Canvas) XamlReader.Load (
@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Canvas.Resources>
		<Storyboard x:Name=""Blah"" x:Key=""Blah"" />
	</Canvas.Resources>
</Canvas>");
			Assert.IsTrue (c.FindName ("Blah") is Storyboard, "FindName");

			c = (Canvas) XamlReader.Load (
@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Canvas.Resources>
		<Storyboard x:Name=""Blah1"" x:Key=""Blah2"" />
	</Canvas.Resources>
</Canvas>");
			Assert.IsTrue (c.FindName ("Blah1") is Storyboard, "FindName-Name");
			Assert.IsNull (c.FindName ("Blah2"), "FindName-Key");
		}

		[TestMethod]
		public void NameAndKey_Resource_Namespace ()
		{
			Canvas c = (Canvas) XamlReader.Load (
@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" xmlns:sl=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">
	<sl:Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">
		<sl:Canvas.Resources xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
			<sl:Storyboard x:Name=""Blah"" x:Key=""Blah"" />
		</sl:Canvas.Resources>
	</sl:Canvas>
</Canvas>");
			Assert.IsTrue (c.FindName ("Blah") is Storyboard, "FindName");
		}

		[TestMethod]
		public void SetTarget ()
		{
			Rectangle r = new Rectangle ();
			Assert.Throws<ArgumentNullException> (delegate {
				Storyboard.SetTarget (null, r);
			}, "null, do");

			Timeline t = (Timeline) new ColorAnimation ();
			Assert.Throws<ArgumentNullException> (delegate {
				Storyboard.SetTarget (t, null);
			}, "timeline, null");
		}

		[TestMethod]
		public void SetTargetName ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				Storyboard.SetTargetName (null, "moon");
			}, "null, string");
		}

		[TestMethod]
		public void SetTargetName_NullName ()
		{
			Timeline t = (Timeline) new ColorAnimation ();
			Assert.IsNull (t.GetValue (Storyboard.TargetNameProperty), "GetValue(TargetNameProperty)");
			Storyboard.SetTargetName (t, null);
			// and the behavior is not specific to Storyboard.SetTargetName
			t.SetValue (Storyboard.TargetNameProperty, (string)null);

			// it's even reset-able to null after a value is assigned
			Storyboard.SetTargetName (t, "uho");
			Storyboard.SetTargetName (t, null);
			Assert.IsNull (Storyboard.GetTargetName (t), "GetTargetName-final");
			Assert.IsNull (t.GetValue (Storyboard.TargetNameProperty), "GetValue(TargetNameProperty)-final");
		}

		[TestMethod]
		public void SetTargetProperty ()
		{
			PropertyPath pp = new PropertyPath ("/moon");
			Assert.Throws<ArgumentNullException> (delegate {
				Storyboard.SetTargetProperty (null, pp);
			}, "null, PropertyPath");

			Timeline t = (Timeline) new ColorAnimation ();
			Assert.Throws<ArgumentNullException> (delegate {
				Storyboard.SetTargetProperty (t, null);
			}, "timeline, null");
		}

		[TestMethod]
		public void GetTargetName ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				Storyboard.GetTargetName (null);
			}, "null");

			Timeline t = (Timeline) new ColorAnimation ();
			Assert.IsNull (Storyboard.GetTargetName (t), "GetTargetName(ColorAnimation)");
		}

		[TestMethod]
		public void GetTargetProperty ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				Storyboard.GetTargetProperty (null);
			}, "null");
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void CurrentState ()
		{
			Storyboard sb = new Storyboard { RepeatBehavior = new RepeatBehavior (2) };

			Rectangle r = new Rectangle { Name = "A" };
			Storyboard child = new Storyboard ();
			DoubleAnimation animation = new DoubleAnimation { Duration = new Duration(TimeSpan.FromSeconds (1)), From = 10, To = 100 };
			Storyboard.SetTargetName (animation, "A");
			Storyboard.SetTargetProperty(animation, new PropertyPath ("Width"));

			child.Children.Add (animation);
			sb.Children.Add (child);
			TestPanel.Children.Add(r);

			r = new Rectangle { Name = "B" };
			child = new Storyboard ();
			animation = new DoubleAnimation { Duration = new Duration (TimeSpan.FromSeconds (2)), From = 10, To = 100 };
			Storyboard.SetTargetName (animation, "B");
			Storyboard.SetTargetProperty (animation, new PropertyPath ("Width"));
			
			child.Children.Add (animation);
			sb.Children.Add (child);
			TestPanel.Children.Add(r);

			TestPanel.Resources.Add ("asdas", sb);
			Enqueue (delegate { Assert.AreEqual (ClockState.Stopped, sb.GetCurrentState (), "#1"); });
			Enqueue (delegate { sb.Begin (); });

			EnqueueSleep (500);
			Enqueue (delegate { Assert.AreEqual (ClockState.Active, sb.GetCurrentState (), "#2"); });
			Enqueue (delegate { Assert.AreEqual (ClockState.Active, ((Storyboard)sb.Children[0]).GetCurrentState (), "#3"); });
			Enqueue (delegate { Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#4"); });
			
			EnqueueSleep (750);
			Enqueue (delegate { Assert.AreEqual (ClockState.Active, sb.GetCurrentState (), "#5"); });
			Enqueue (delegate { Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#6"); });
			Enqueue (delegate { Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#7"); });

			Enqueue (delegate { TestPanel.Children.Clear (); TestPanel.Resources.Clear (); });
			EnqueueSleep (100);
			Enqueue (delegate { Assert.AreEqual (ClockState.Active, sb.GetCurrentState (), "#8"); });
			Enqueue (delegate { Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#9"); });
			Enqueue (delegate { Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#10"); });

			Enqueue (delegate { sb.Stop (); });
			Enqueue (delegate { Assert.AreEqual (ClockState.Stopped, sb.GetCurrentState (), "#11"); });
			Enqueue (delegate { Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#12"); });
			Enqueue (delegate { Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#13"); });

			EnqueueSleep (100);
			Enqueue (delegate { Assert.AreEqual (ClockState.Stopped, sb.GetCurrentState (), "#14"); });
			Enqueue (delegate { Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#15"); });
			Enqueue (delegate { Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#16"); });
			EnqueueTestComplete ();
		}
	}
}
