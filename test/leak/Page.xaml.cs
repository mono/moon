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
using System.Windows.Browser;

namespace Leak
{
	public partial class Page : Canvas
	{
		WeakReference weakSubtree = new WeakReference (null);
		WeakReference weakControl = new WeakReference (null);
		WeakReference weakStoryboard = new WeakReference (null);

		Control Control {
			get; set;
		}

		Storyboard Storyboard {
			get;set;
		}

		FrameworkElement Subtree {
			get; set;
		}

		Control WeakControl {
			get { return (Control) weakControl.Target; }
			set { weakControl = new WeakReference (value); }
		}

		Storyboard WeakStoryboard
		{
			get { return (Storyboard) weakStoryboard.Target; }
			set { weakStoryboard = new WeakReference (value); }
		}

		FrameworkElement WeakSubtree {
			get { return (FrameworkElement) weakSubtree.Target; }
			set { weakSubtree = new WeakReference (value); }
		}
		
		public Page()
		{
			InitializeComponent();
			Queue (RunTest);
		}
		
		T ApplyTemplate <T> (T c) where T : FrameworkElement
		{
			// Add the element to the live tree so the default style
			// is set, then apply the template and remove it from the
			// live tree.
			Children.Add (c);
			c.UpdateLayout ();
			Children.Remove (c);
			return c;
		}
		
		void GCAndInvoke (Action a)
		{
			var d = Dispatcher;
			System.Threading.ThreadPool.QueueUserWorkItem (delegate {
				System.Threading.Thread.Sleep (500);
				GC.Collect ();
				d.BeginInvoke (a);
			});
		}
		
		void Fail (string message)
		{
			ScriptObject so = HtmlPage.Window.GetProperty ("Fail") as ScriptObject;
			so.InvokeSelf (message);
		}
		
		void Succeed ()
		{
			ScriptObject so = HtmlPage.Window.GetProperty ("ShutdownHarness") as ScriptObject;
			so.InvokeSelf ();
		}

		void Queue (Action a)
		{
			var d = Dispatcher;
			System.Threading.ThreadPool.QueueUserWorkItem (delegate {
				System.Threading.Thread.Sleep (500);
				d.BeginInvoke (a);
			});
		}
	}
}
