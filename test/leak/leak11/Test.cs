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
			// An unstarted storyboard should be GCable
			WeakStoryboard = new Storyboard ();

			GCAndInvoke (() => {
				if (WeakStoryboard != null)
					Fail ("Storyboard should be GC'ed");
				else
					Succeed ();
			});
		}
	}
}
