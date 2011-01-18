using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Markup;
using System.ComponentModel;
using System.Windows.Data;
using System.Collections.ObjectModel;
using System.Threading;

namespace Leak
{
	public partial class Page
	{
		int counter = 0;
		List<Control> controls = new List<Control>();

		void RunTest ()
		{
			for (int i = 0; i < 5; i++)
				controls.Add(new ContentControl());

			Queue(() => Create(false));
			Queue(GCCollect, TimeSpan.FromSeconds(0));
		}
		
		void Create(bool stop)
		{
			// If we haven't crashed yet, the test has been successful
			if (counter ++ == 5)
				Succeed ();
				
			for (int i = 0; i < controls.Count; i++)
			{
				var control = controls[i];
				CreateStoryboard(control, stop);
				CreateStoryboard(control, stop);
				CreateStoryboard(control, stop);
			}

			Queue(() => Create(true));
		}

		private void CreateStoryboard(Control control, bool stop)
		{
			var sb = new Storyboard();
			var timeline = new DoubleAnimationUsingKeyFrames();
			timeline.KeyFrames.Add(new DiscreteDoubleKeyFrame { Value = 4, KeyTime = KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(100)) });
			timeline.KeyFrames.Add(new DiscreteDoubleKeyFrame { Value = 5, KeyTime = KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(200)) });
			timeline.KeyFrames.Add(new DiscreteDoubleKeyFrame { Value = 6, KeyTime = KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(300)) });
			sb.Children.Add(timeline);

			Storyboard.SetTarget(sb, control);
			Storyboard.SetTargetProperty(sb, new PropertyPath("Width"));
			if (stop)
				sb.Completed += (s, e) => ((Storyboard)s).Stop();

			sb.Begin();
		}

		void GCCollect()
		{
			GC.Collect();
			GC.WaitForPendingFinalizers();
			GC.Collect();
			GC.WaitForPendingFinalizers();
			GC.Collect();
			GC.WaitForPendingFinalizers();

			Queue(GCCollect, TimeSpan.FromSeconds(0));
		}

		void Queue(Action a, TimeSpan span)
		{
			var d = Dispatcher;
			ThreadPool.QueueUserWorkItem(delegate
			{
				Thread.Sleep((int)span.TotalMilliseconds);
				GC.Collect();
				GC.WaitForPendingFinalizers();
				d.BeginInvoke(a);
			});
		}
	}
}
