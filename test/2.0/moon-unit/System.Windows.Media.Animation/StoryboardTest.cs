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
using System.Threading;
using System.Collections.Generic;
using Microsoft.Silverlight.Testing.UnitTesting;

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
		[Asynchronous]
		[MoonlightBug]
		public void MultipleStartStop ()
		{
			int count = 0;
			
			Rectangle target = new Rectangle ();
			
			Storyboard a = new Storyboard ();
			DoubleAnimation animation = new DoubleAnimation { From = 5, To = 100, Duration = new Duration (TimeSpan.FromMilliseconds (100)) };
			Storyboard.SetTarget (animation, target);
			Storyboard.SetTargetProperty (animation, new PropertyPath ("Width"));
			a.Children.Add (animation);

			a.Completed += delegate { count++; a.Begin (); };

			Enqueue (() => TestPanel.Children.Add (target));
			Enqueue (() => a.Begin ());
			EnqueueConditional (() => count == 5, TimeSpan.FromMilliseconds (1500));
			EnqueueTestComplete ();
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
		public void AddRunningStoryboard ()
		{
			Canvas c = CreateStoryboard ();
			Storyboard sb = (Storyboard) c.Resources ["Storyboard"];

			Storyboard child = new Storyboard { };
			DoubleAnimation animation = new DoubleAnimation { Duration = new Duration (TimeSpan.FromSeconds (1)), From = 10, To = 100 };
			Storyboard.SetTargetName (animation, "A");
			Storyboard.SetTargetProperty (animation, new PropertyPath ("Width"));
			child.Children.Add (animation);

			TestPanel.Children.Add (c);

			Enqueue (delegate {
				// The TargetName isn't resolvable
				Assert.Throws<InvalidOperationException>(() => child.Begin (), "TargetName should not be resolvable");
				Storyboard.SetTargetName (animation, null);
				Assert.Throws<InvalidOperationException> (() => child.Begin (), "Target is not specified");
				Storyboard.SetTarget (child, c.Children [0]);
				child.Begin ();
			});

			Enqueue (delegate {
				Assert.AreEqual (ClockState.Active, child.GetCurrentState (), "Active");
				sb.Children.Add (child);
			});

			Enqueue (delegate {
				Assert.AreEqual (ClockState.Active, child.GetCurrentState (), "#1");
				Assert.AreEqual (ClockState.Stopped, sb.GetCurrentState (), "#2");
			});

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveChildThenStart ()
		{
			Canvas c = CreateStoryboard ();
			Storyboard sb = (Storyboard) c.Resources ["Storyboard"];
			Storyboard child = (Storyboard) sb.Children [1];
			sb.Children.RemoveAt (1);
			c.Resources.Clear();
			c.Resources.Add ("asdasd", child);
			CreateAsyncTest (c, () =>
				Assert.Throws<InvalidOperationException> (() => child.Begin ())
			);
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveChildThenStart2 ()
		{
			Canvas c = CreateStoryboard ();
			Storyboard sb = (Storyboard) c.Resources ["Storyboard"];
			Storyboard child = (Storyboard) sb.Children [1];
			sb.Children.RemoveAt (1);
			c.Resources.Clear ();

			Storyboard storyboard = new Storyboard ();
			storyboard.Children.Add (child);

			c.Resources.Add ("asdasd", storyboard);
			CreateAsyncTest (c,
				() => storyboard.Begin (),
				() => storyboard.Stop ()
			);
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveChildThenStart3 ()
		{
			Canvas c = CreateStoryboard ();
			Storyboard root = (Storyboard) c.Resources ["Storyboard"];
			Timeline timeline = ((Storyboard) root.Children [0]).Children [0];
			((Storyboard)root.Children[0]).Children.RemoveAt (0);
			c.Resources.Clear ();
			Storyboard storyboard = new Storyboard ();
			storyboard.Children.Add (timeline);

			c.Resources.Add ("asdasd", storyboard);
			CreateAsyncTest (c,
				() => storyboard.Begin (),
				() => storyboard.Stop ()
			);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void RemoveChildThenStart4 ()
		{
			Canvas c = CreateStoryboard ();
			Storyboard sb = (Storyboard) c.Resources ["Storyboard"];
			Storyboard child = (Storyboard) sb.Children [1];
			sb.Children.RemoveAt (1);
			c.Resources.Clear ();

			TimelineCollection collection = new TimelineCollection();
			Storyboard storyboard = new Storyboard ();
			storyboard.Children.Add (child);
			Storyboard.SetTargetName (child, null);
			Storyboard.SetTarget (child, c.Children[0]);

			CreateAsyncTest (c,
				delegate { storyboard.Begin (); },
				delegate { storyboard.Stop (); },
				delegate { collection.Add (storyboard); },
				delegate { storyboard.Begin (); },
				delegate { storyboard.Stop (); },
				delegate { collection.Remove (storyboard); },
				delegate { sb.Children.Add (storyboard); },
				delegate { sb.Children.Remove (storyboard); },
				delegate { Assert.Throws<InvalidOperationException> (delegate { storyboard.Begin (); }); }
			);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void CurrentState ()
		{
			Canvas c = CreateStoryboard ();
			Storyboard sb = (Storyboard) c.Resources ["Storyboard"];
			TestPanel.Children.Add (c);

			Enqueue ( delegate {
				Assert.AreEqual (ClockState.Stopped, sb.GetCurrentState (), "#1");
				sb.Begin ();
			});

			EnqueueConditional (delegate { return sb.GetCurrentTime ().TotalMilliseconds > 300; });
			Enqueue (delegate {
				// Animation1: 300/1000, Animation2: 300/2000
				Assert.IsTrue (sb.GetCurrentTime ().TotalSeconds < 1, "#Sanity1");
				Assert.AreEqual (ClockState.Active, sb.GetCurrentState (), "#2");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [0]).GetCurrentState (), "#3");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#4");
			});

			EnqueueConditional (delegate { return sb.GetCurrentTime ().TotalMilliseconds > 1250; });
			Enqueue (delegate {
				// Animation1: 1250/1000, Animation2: 1250/2000
				Assert.IsTrue (sb.GetCurrentTime ().TotalSeconds > 1, "#Sanity2");
				Assert.AreEqual (ClockState.Active, sb.GetCurrentState (), "#5");
				Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#6");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#7");
			});

			EnqueueConditional (delegate { return sb.GetCurrentTime ().TotalMilliseconds > 1350; });
			Enqueue (delegate {
				// Animation1: 1350/1000, Animation2: 1350/2000
				Assert.IsTrue (sb.GetCurrentTime ().TotalSeconds > 1, "#Sanity3");
				Assert.AreEqual (ClockState.Active, sb.GetCurrentState (), "#8");
				Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#9");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#10");
				sb.Stop ();
			});

			Enqueue (delegate {
				Assert.IsTrue (sb.GetCurrentTime ().TotalSeconds > 1, "#Sanity4");
				Assert.AreEqual (ClockState.Stopped, sb.GetCurrentState (), "#11");
				Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#12");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#13");
			});

			Enqueue (delegate {
				// Animation1: 1450/1000, Animation2: 1450/2000
				Assert.IsTrue (sb.GetCurrentTime ().TotalSeconds > 1, "#Sanity5");
				Assert.AreEqual (ClockState.Stopped, sb.GetCurrentState (), "#14");
				Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#15");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#16");
			});

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void CurrentState2 ()
		{
			Canvas c = CreateStoryboard ();
			Storyboard sb = (Storyboard) c.Resources ["Storyboard"];
			TestPanel.Children.Add (c);

			Enqueue (delegate {
				Assert.AreEqual (ClockState.Stopped, sb.GetCurrentState (), "#1");
				sb.Begin ();
			});

			EnqueueConditional (delegate { return sb.GetCurrentTime ().TotalMilliseconds > 300; });
			Enqueue (delegate {
				// Animation1: 300/1000, Animation2: 300/2000
				Assert.IsTrue (sb.GetCurrentTime ().TotalSeconds < 1, "#Sanity1");
				Assert.AreEqual (ClockState.Active, sb.GetCurrentState (), "#2");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [0]).GetCurrentState (), "#3");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#4");
			});

			EnqueueConditional (delegate { return sb.GetCurrentTime ().TotalMilliseconds > 1250; });
			Enqueue (delegate {
				// Animation1: 1250/1000, Animation2: 1250/2000
				Assert.IsTrue (sb.GetCurrentTime ().TotalSeconds > 1, "#Sanity2");
				Assert.AreEqual (ClockState.Active, sb.GetCurrentState (), "#5");
				Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#6");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#7");
			});

			Enqueue (delegate {
				c.Children.Clear ();
				c.Resources.Clear ();
				TestPanel.Children.Clear ();
				TestPanel.Resources.Clear ();
			});

			EnqueueConditional (delegate { return sb.GetCurrentTime ().TotalMilliseconds > 1350; });
			Enqueue (delegate {
				// Animation1: 1350/1000, Animation2: 1350/2000
				Assert.IsTrue (sb.GetCurrentTime ().TotalSeconds > 1, "#Sanity3");
				Assert.AreEqual (ClockState.Active, sb.GetCurrentState (), "#8");
				Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#9");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#10");
				sb.Stop ();
			});

			Enqueue (delegate {
				Assert.IsTrue (sb.GetCurrentTime ().TotalSeconds > 1, "#Sanity4");
				Assert.AreEqual (ClockState.Stopped, sb.GetCurrentState (), "#11");
				Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#12");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#13");
			});

			Enqueue (delegate {
				// Animation1: 1450/1000, Animation2: 1450/2000
				Assert.IsTrue (sb.GetCurrentTime ().TotalSeconds > 1, "#Sanity5");
				Assert.AreEqual (ClockState.Stopped, sb.GetCurrentState (), "#14");
				Assert.AreEqual (ClockState.Filling, ((Storyboard) sb.Children [0]).GetCurrentState (), "#15");
				Assert.AreEqual (ClockState.Active, ((Storyboard) sb.Children [1]).GetCurrentState (), "#16");
			});

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void CompleteEvent ()
		{
			List<ManualResetEvent> handles = new List<ManualResetEvent> ();
			Canvas c = new Canvas ();

			Storyboard sb = new Storyboard { Duration = new Duration (TimeSpan.FromMilliseconds (500)) };
			ManualResetEvent handle = new ManualResetEvent (false);
			handles.Add (handle);
			bool fillingWhenComplete = false;
			bool childrenNotSignalled = true;
			sb.Completed += delegate {
				fillingWhenComplete = ClockState.Filling == sb.GetCurrentState ();
				//Assert.AreEqual (sb.Duration.TimeSpan, sb.GetCurrentTime (), "#2");
				for (int i = 1; i < handles.Count; i++)
					if (handles [i].WaitOne (0))
						childrenNotSignalled = false;
				handle.Set ();
			};
			c.Resources.Add ("Storyboard", sb);

			for (int i = 0; i < 5; i++) {
				DoubleAnimation anim = new DoubleAnimation { From = 10, To = 50 };
				Storyboard child = new Storyboard { Duration = new Duration (TimeSpan.FromMilliseconds (500)) };

				ManualResetEvent h = new ManualResetEvent (false);
				handles.Add (h);
				child.Completed += delegate {
					int count = i;
					h.Set ();
					Assert.AreEqual (ClockState.Filling, child.GetCurrentState (), "#4." + count);
				};

				child.Children.Add (anim);
				sb.Children.Add (child);
				Rectangle r = new Rectangle ();
				Storyboard.SetTarget (child, r);
				Storyboard.SetTargetProperty (child, new PropertyPath ("Width"));
				c.Children.Add (r);
			}

			TestPanel.Children.Add (c);
			for (int j = 0; j < 3; j++) {
				int temp = j;
				Enqueue (() => handles.ForEach (h => h.Reset ()));
				Enqueue (() => sb.Begin ());
				Enqueue (() => {
					Assert.AreEqual ( ClockState.Active, sb.GetCurrentState (), "#1");
				});
				EnqueueConditional (() => handle.WaitOne (0));
				Enqueue (() => Assert.IsTrue (fillingWhenComplete, "Filling when complete"));
				Enqueue (() => Assert.IsTrue (childrenNotSignalled, "Children not signalled"));
				Enqueue (() => {
					for (int i = 0; i < handles.Count; i++)
						Assert.IsTrue (handles [i].WaitOne (1000));
				});
				Enqueue (() => Assert.AreEqual (ClockState.Filling, sb.GetCurrentState (), "Still filling"));
			}
			Enqueue (() => TestPanel.Children.Clear ());
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void TargetSameProperty ()
		{
			Canvas c = new Canvas ();
			Rectangle r = new Rectangle ();
			c.Children.Add (r);

			Storyboard sb = new Storyboard { Duration = new Duration (TimeSpan.FromMilliseconds (500)) };
			c.Resources.Add ("Storyboard", sb);

			for (int i = 0; i < 5; i++) {
				DoubleAnimation anim = new DoubleAnimation { From = 10, To = 50 };
				Storyboard child = new Storyboard { Duration = new Duration (TimeSpan.FromMilliseconds (500)) };

				child.Children.Add (anim);
				sb.Children.Add (child);
				Storyboard.SetTarget (child, r);
				Storyboard.SetTargetProperty (child, new PropertyPath ("Width"));
			}

			CreateAsyncTest (c, () => Assert.Throws<InvalidOperationException>(() => sb.Begin ()));
		}
		
		[TestMethod]
		[Asynchronous]
		public void StartChildStoryboard ()
		{
			Canvas c = CreateStoryboard ();
			Storyboard sb = (Storyboard) c.Resources ["Storyboard"];
			Storyboard child = (Storyboard) sb.Children [1];
			TestPanel.Children.Add (c);

			Enqueue (() => {
				Assert.Throws<InvalidOperationException> (() => child.Begin ());
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void StopChildStoryboard ()
		{
			Canvas c = CreateStoryboard ();
			Storyboard sb = (Storyboard) c.Resources ["Storyboard"];
			Storyboard child = (Storyboard) sb.Children [1];
			TestPanel.Children.Add (c);

			Enqueue (() => {
				sb.Begin ();
			});

			Sleep (100, () => {
				Assert.Throws<InvalidOperationException> (() => child.Stop (), "#1");
				Assert.Throws<InvalidOperationException> (() => child.Pause (), "#2");
				Assert.Throws<InvalidOperationException> (() => child.Seek(TimeSpan.FromMilliseconds(10)), "#3");
				sb.Pause ();
			});

			Enqueue (() => Assert.Throws<InvalidOperationException> (() => child.Resume (), "#4"));
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void CurrentTime ()
		{
			Canvas c = CreateStoryboard ();
			Storyboard storyboard = (Storyboard) c.Resources ["Storyboard"];
			int start = Environment.TickCount;

			Enqueue (() => TestPanel.Children.Add (c));
			Enqueue (() => storyboard.Begin ());
			EnqueueConditional (()=> Environment.TickCount - start > 300);
			Enqueue (() => {
				double ms = storyboard.GetCurrentTime ().TotalMilliseconds;
				Assert.IsTrue (ms < 350, "Less than 350ms");
				Assert.AreEqual (((Storyboard) storyboard.Children [0]).GetCurrentTime ().TotalMilliseconds, ms, "#2");
				Assert.AreEqual (((Storyboard) storyboard.Children [1]).GetCurrentTime ().TotalMilliseconds, ms, "#3");
			});
			EnqueueConditional (() => Environment.TickCount - start > 1100);
			Enqueue (() => {
				double ms = storyboard.GetCurrentTime ().TotalMilliseconds;
				Assert.IsTrue (ms > 1000, "More than 1000ms");
				Assert.AreEqual (((Storyboard) storyboard.Children [0]).GetCurrentTime ().TotalMilliseconds, 1000, "#4");
				Assert.AreEqual (((Storyboard) storyboard.Children [1]).GetCurrentTime ().TotalMilliseconds, ms, "#5");
			});
			Enqueue (() => TestPanel.Children.Clear ());
			EnqueueTestComplete ();
		}

		private Canvas CreateStoryboard ()
		{
			Canvas c = new Canvas ();

			Storyboard sb = new Storyboard { RepeatBehavior = new RepeatBehavior (2) };

			Rectangle r = new Rectangle { Name = "A" };
			Storyboard child = new Storyboard ();
			DoubleAnimation animation = new DoubleAnimation { Duration = new Duration (TimeSpan.FromSeconds (1)), From = 10, To = 100 };
			Storyboard.SetTargetName (animation, "A");
			Storyboard.SetTargetProperty (animation, new PropertyPath ("Width"));

			child.Children.Add (animation);
			sb.Children.Add (child);
			c.Children.Add (r);

			r = new Rectangle { Name = "B" };
			child = new Storyboard ();
			animation = new DoubleAnimation { Duration = new Duration (TimeSpan.FromSeconds (2)), From = 10, To = 100 };
			Storyboard.SetTargetName (animation, "B");
			Storyboard.SetTargetProperty (animation, new PropertyPath (FrameworkElement.WidthProperty));

			child.Children.Add (animation);
			sb.Children.Add (child);
			c.Children.Add (r);

			c.Resources.Add ("Storyboard", sb);
			return c;
		}
	}
}
