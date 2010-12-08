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
			public Value { get; set;
		}

		void RunTest ()
		{
			// Hold the Storyboard with a strong ref and the target with a weak ref.

			WeakControl = new ContentControl ();
			Storyboard = new Storyboard ();
			DoubleAnimation anim = new DoubleAnimation { From = 0, To = 1, Duration = new Duration (TimeSpan.FromSeconds (1)) };
			Storyboard.SetTarget (Storyboard, WeakControl);
			Storyboard.SetTargetProperty (Storyboard, new PropertyPath ("Opacity"));
			Storyboard.Children.Add (anim);

			Storyboard.Begin ();
			
			GCAndInvoke (() => {
				if (WeakControl != null)
					Fail ("Storyboard target should be GC'ed");
				else
					Succeed ();
			});
		}
	}
}
