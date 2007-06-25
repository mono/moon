using System;
using System.Threading;
using IO=System.IO;

using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklet
{
	public class Calendar : Canvas 
	{

		public void PageLoaded (object o, EventArgs e) 
	    {
			Console.WriteLine ("page loaded");
		}

	}
}
