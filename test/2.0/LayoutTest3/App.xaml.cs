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
using System.Windows.Markup;

namespace LayoutTest3
{
	public class MyButton : Button
	{
		public MyButton ()
		{
			this.Loaded += delegate { App.Write (this, "Loaded"); };
			this.LayoutUpdated += delegate { App.Write (this, "LayoutUpdated"); };
		}

		public override void OnApplyTemplate ()
		{
			App.Write (this, "OnApplyTemplate");
			base.OnApplyTemplate ();
		}

		protected override Size ArrangeOverride (Size finalSize)
		{
			App.Write (this, "ArrangeOverride");
			return base.ArrangeOverride (finalSize);
		}

		protected override Size MeasureOverride (Size availableSize)
		{
			App.Write (this, "MeasureOverride");
			return base.MeasureOverride (availableSize);
		}
	}

	public class TestElement : Canvas
	{
		public TestElement ()
		{
			this.Loaded += delegate { App.Write (this, "Loaded"); Children.Add (App.CreateTemplated ("Button2")); };
			this.LayoutUpdated += delegate { App.Write (this, "LayoutUpdated"); };
		}

		public override void OnApplyTemplate ()
		{
			App.Write (this, "OnApplyTemplate");
			base.OnApplyTemplate ();
		}
	}

	public partial class App : Application
	{
		public static void Write (DependencyObject o, string message, params object [] args)
		{
			sb.AppendFormat ("{0}: {1}", o.GetValue (FrameworkElement.NameProperty), message);
			sb.AppendLine ();
		}

		static StringBuilder sb = new StringBuilder ();

		Dispatcher d;
		public App ()
		{
			this.Startup += this.Application_Startup;
			InitializeComponent ();
		}

		private void Application_Startup (object sender, StartupEventArgs e)
		{
			Panel root = new TestElement { Name = "Root" };
			root.Children.Add (CreateTemplated ("Button1"));
			d = root.Dispatcher;
			RootVisual = root;
			Delay (() => {
				Console.WriteLine ("Actual");
				Console.Write (sb.ToString ());

				Console.WriteLine ();
				Console.WriteLine ();
				Console.WriteLine (@"Expected
Button1: Loaded
Root: Loaded
Button2: Loaded
Button1: OnApplyTemplate
Button1: MeasureOverride
Button2: OnApplyTemplate
Button2: MeasureOverride
Button1: ArrangeOverride
Button2: ArrangeOverride
Root: LayoutUpdated
Button1: LayoutUpdated
Button2: LayoutUpdated");
			});
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

		public static MyButton CreateTemplated (string name)
		{
			MyButton b = (MyButton) XamlReader.Load (@"
<x:MyButton	xmlns=""http://schemas.microsoft.com/client/2007""
					xmlns:x=""clr-namespace:LayoutTest3;assembly=LayoutTest3"">

</x:MyButton>");
			b.Width = 200;
			b.Height = 50;
			b.Name = name;
			b.Content = "Hello Button!";
			return b;
		}
	}
}
