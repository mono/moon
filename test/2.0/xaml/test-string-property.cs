
using System;
using System.Windows.Media;
using System.Windows.Controls;

namespace Mono {

	public class StringPropertyTest : Canvas {

		public StringPropertyTest ()
		{
		}

		public void Canvas_Loaded (object sender, EventArgs e)
		{
			TextBlock n = new TextBlock ();
			n.Text = "I am the textblock";
			n.SetValue (TextBlock.ForegroundProperty, "#FF626262");

			Children.Add (n);
		}
        }
}

