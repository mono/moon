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

namespace Leak
{
	public partial class Page
	{
		class Wrapper {
			public object Value { get; set; }
		}

		void RunTest ()
		{
			// Hold the storyboard and target weakly and GC after the storyboard completes
			// Both should be collected then.
			WeakControl = new ContentControl ();
			WeakStoryboard = new Storyboard ();
			DoubleAnimation anim = new DoubleAnimation { From = 0, To = 1, Duration = new Duration (TimeSpan.FromSeconds (1)) };
			Storyboard.SetTarget (WeakStoryboard, WeakControl);
			Storyboard.SetTargetProperty (WeakStoryboard, new PropertyPath ("Opacity"));
			WeakStoryboard.Children.Add (anim);

			WeakStoryboard.Completed += delegate {
				GCAndInvoke (() => {
					if (WeakControl != null)
						Fail ("Control should be GC'ed");
					else if (WeakStoryboard != null)
						Fail ("Storyboard should be GC'ed");
					else
						Succeed ();
				});
			};
			
			WeakStoryboard.Begin ();
		}
	}
}
