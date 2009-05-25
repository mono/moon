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

// test for checking whether we can create objects from non-ui threads (should blow up)

namespace threads1 {
	public partial class Page : UserControl {
		int events = 0;
		public Page () {
			InitializeComponent ();

			Timer t3 = new Timer (delegate (object state) {
				string s = Thread.CurrentThread.ManagedThreadId.ToString ();
				TextBox t = new TextBox (); // this has to throw an exception
				t.TextChanged += new TextChangedEventHandler (t_TextChanged);
				this.LayoutRoot.Children.Add (t);
			});
			t3.Change (1000, Timeout.Infinite);
		}

		void t_TextChanged (object sender, TextChangedEventArgs e) {
			events++;
		}
	}
}
