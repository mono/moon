using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Threading;

// test for adding objects in the main thread from a timer

namespace threads2 {
	public partial class Page : UserControl {
		
		public Page () {
			InitializeComponent ();

			Timer t = new Timer (delegate {
				this.Dispatcher.BeginInvoke (delegate {
					TextBlock b = new TextBlock ();
					this.LayoutRoot.Children.Add (b);
					b.Text = "begin invoke";
				});
			});
			t.Change (1000, Timeout.Infinite);
		}
	}
}
