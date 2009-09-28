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

namespace LayoutTest2
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
			TestElement root = new TestElement { Name = "Root" };
			d = root.Dispatcher;
			RootVisual = root;

			TestElement el = new TestElement { Name = "A" };
			el.LayoutUpdated += delegate {

				if (el.Children.Count == 0) {

					Console.WriteLine (@"First Expected: 
Loaded: A
Loaded: Root
MeasureOverride: Root
ArrangeOverride: Root
LayoutUpdated: Root
LayoutUpdated: A
");
					Console.WriteLine ("First Actual");
					Console.WriteLine (TestElement.sb.ToString ());
					TestElement.sb.Length = 0;


					el.Children.Add (new TestElement { Name = "B" });
					Delay (() => {
						Console.WriteLine (@"Second Expected: 
LayoutUpdated: B
MeasureOverride: A
ArrangeOverride: A
LayoutUpdated: Root
LayoutUpdated: A
LayoutUpdated: B
Loaded: B
");
						Console.WriteLine ("Second actual");
						Console.WriteLine (TestElement.sb.ToString ());
					});
				}
			};

			root.Children.Add (el);
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

