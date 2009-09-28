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
using System.Windows.Threading;
using System.Text;

namespace LayoutTest
{
	public class TestElement : Panel
	{
		public static StringBuilder sb = new StringBuilder ();
		void Write (string message, params object [] args)
		{
			sb.AppendFormat (message, args);
			sb.AppendLine ();
		}

		public TestElement ()
		{
			this.Loaded += delegate { Write ("Loaded: {0}", Name); };
			this.LayoutUpdated += delegate { Write ("LayoutUpdated: {0}", Name); };
		}

		protected override Size MeasureOverride (Size availableSize)
		{
			Write ("MeasureOverride: {0}", Name);
			return base.MeasureOverride (availableSize);
		}

		public override void OnApplyTemplate ()
		{
			Write ("OnApplyTemplate: {0}", Name);
			base.OnApplyTemplate ();
		}

		protected override Size ArrangeOverride (Size finalSize)
		{
			Write ("ArrangeOverride: {0}", Name);
			return base.ArrangeOverride (finalSize);
		}
	}

	public partial class App : Application
	{
		Dispatcher d;
		public App ()
		{
			this.Startup += this.Application_Startup;
			InitializeComponent ();
		}

		private void Application_Startup (object sender, StartupEventArgs e)
		{
			string Expected = @"
Expected sequence:
Loaded: Root
MeasureOverride: Root
ArrangeOverride: Root
LayoutUpdated: Root
LayoutUpdated: A
LayoutUpdated: B
";

			// Attach a root visual
			this.RootVisual = new TestElement { Name = "Root" };
			d = RootVisual.Dispatcher;

			// 1 second after application startup, print out the event sequence and then create element C
			Delay (() => {
				Console.WriteLine ("First");
				Console.WriteLine (Expected);
				Console.WriteLine ("Actual Sequence:");
				Console.WriteLine (TestElement.sb.ToString ());
				new TestElement { Name = "C" };
			
				// 2 seconds after application startup, print out the event sequence again
				Delay (() => {
					Console.WriteLine ("Second");
					Console.WriteLine (Expected);
					Console.WriteLine (TestElement.sb.ToString ());
				});
			});

			// Create element A and in the LayoutUpdated handler create element B
			// These will both end up being laid out - but not measured, arranged or loaded
			new TestElement { Name = "A" }.LayoutUpdated += delegate {
				new TestElement { Name = "B" };
			};
		}

		void Delay (Action action)
		{
			ThreadPool.QueueUserWorkItem (delegate {
				System.Threading.Thread.Sleep (1000);
				d.BeginInvoke (() => {
					action ();
				});
			});
		}
	}
}

