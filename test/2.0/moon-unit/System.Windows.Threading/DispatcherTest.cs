//Permission is hereby granted, free of charge, to any person obtaining
//a copy of this software and associated documentation files (the
//"Software"), to deal in the Software without restriction, including
//without limitation the rights to use, copy, modify, merge, publish,
//distribute, sublicense, and/or sell copies of the Software, and to
//permit persons to whom the Software is furnished to do so, subject to
//the following conditions:
//
//The above copyright notice and this permission notice shall be
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//Copyright (c) 2008 Novell, Inc.
//
//Authors:
//	Andreia Gaita (shana@jitted.com)
//

using System;
using System.Windows;
using System.Windows.Controls;
using System.Threading;
using System.Diagnostics;
using System.Windows.Threading;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Windows.Shapes;
using System.Windows.Media.Animation;


namespace MoonTest.System.Windows.Threading
{
	[TestClass]
	public class DispatcherTest : SilverlightTest
	{
		public static int count;

		public void InvokeAndDestroy () {
			TextBlock t = new TextBlock ();
			Dispatcher e = t.Dispatcher;

			ManualResetEvent wait = new ManualResetEvent (false);

			Timer timer = new Timer ( delegate {
				try {
					Console.WriteLine (count++);
					e.BeginInvoke (delegate {
						Console.WriteLine ("invoked");
					});
				} finally {
					wait.Set ();
				}
			});

			for (int i = 0; i < 20; i++) {
				if (i == 10)
					e = null;
				timer.Change (0, Timeout.Infinite);
				wait.WaitOne ();
			}
			Console.WriteLine ("done");

		}

		[TestMethod]
		[SilverlightBug]
		[Ignore ("this can crash the process/IE with SL2")]
		public void SyncContextCheck ()
		{
			int mainui = Thread.CurrentThread.ManagedThreadId;
			ManualResetEvent wait = new ManualResetEvent (false);
			Timer timer = new Timer ( delegate {
				try {
					DispatcherSynchronizationContext.Current.Send (delegate {
						Assert.AreEqual (mainui, Thread.CurrentThread.ManagedThreadId, "MainThreadCheck");
					}, null);
				} finally {
					wait.Set ();
				}
			});
			timer.Change (0, Timeout.Infinite);
			wait.WaitOne ();
		}

		[TestMethod]
		public void DODispatcher () {
			Button b = new Button ();

			int mainui = Thread.CurrentThread.ManagedThreadId;
			ManualResetEvent wait = new ManualResetEvent (false);
			Timer timer = new Timer (delegate {
				try {
					b.Dispatcher.BeginInvoke ( delegate {
						Assert.AreEqual (mainui, Thread.CurrentThread.ManagedThreadId, "MainThreadCheck");
					});
				} finally {
					wait.Set ();
				}
			});
			timer.Change (0, Timeout.Infinite);
			wait.WaitOne ();
		}
		
		[TestMethod]
		[Asynchronous]
		public void RestartTimer ()
		{
			int count = 0;
			DispatcherTimer timer = new DispatcherTimer ();
			timer.Interval = TimeSpan.FromMilliseconds (5);
			timer.Tick += delegate { count++; if (count == 5) timer.Stop (); };
			Enqueue (() => {count = 0; timer.Start ();});
			EnqueueConditional (() => count == 5);
			Enqueue (() => { count = 0; timer.Start (); });
			EnqueueConditional (() => count == 5);
			Enqueue (() => { count = 0; timer.Start (); });
			EnqueueConditional (() => count == 5);
			EnqueueTestComplete ();
		}
		
		[TestMethod]
		[Asynchronous]
		public void RestartTimer2 ()
		{
			bool complete=false;
			Rectangle r = new Rectangle();
			Storyboard sb = new Storyboard ();
			DoubleAnimation animation = new DoubleAnimation { Duration = new Duration(TimeSpan.FromMilliseconds(100)), From = 10, To = 100 };
			Storyboard.SetTarget (animation, r);
			Storyboard.SetTargetProperty (animation, new PropertyPath ("Width"));
			sb.Children.Add (animation);
			sb.Completed += delegate {complete = true;};
			int count = 0;
			DispatcherTimer timer = new DispatcherTimer ();
			timer.Interval = TimeSpan.FromMilliseconds (5);
			timer.Tick += delegate { if (count < 5) count++; };
			Enqueue (() => { timer.Start (); });
			Enqueue (() => { sb.Begin (); });
			Enqueue (() => complete = true);
			Enqueue (() => { timer.Stop (); complete = false; });
			Enqueue (() => { sb.Begin (); });
			Enqueue (() => complete = true);
			Enqueue (() => {count =0; timer.Start (); complete = false; });
			EnqueueConditional (() => count == 5, TimeSpan.FromMilliseconds (1000));
			EnqueueTestComplete ();
		}
	}
}
