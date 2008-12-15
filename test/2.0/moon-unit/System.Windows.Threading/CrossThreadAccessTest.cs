using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;
using System.Threading;
using System.Windows.Threading;
using s=System;


namespace MoonTest.System.Windows.Threading {

	[TestClass]
	public class CrossThreadAccessTest {

		[TestMethod]
		public void ApplicationTest () {

			Timer t = new Timer (delegate {
				Assert.Throws<UnauthorizedAccessException> (delegate {
					Application a = new Application ();
				});
			});
			t.Change (0, Timeout.Infinite);
		}

		[TestMethod]
		public void LoadComponentTest () {

			Timer t = new Timer (delegate {
				Assert.Throws<UnauthorizedAccessException> (delegate {
					Application.LoadComponent (this, new s.Uri ("/threads3;component/App.xaml", s.UriKind.Relative));
				});
			});
			t.Change (0, Timeout.Infinite);
		}

		[TestMethod]
		public void GetRootVisualTest () {

			Timer t = new Timer (delegate {
				Assert.Throws<UnauthorizedAccessException> (delegate {
					UIElement e = Application.Current.RootVisual;
				});
			});
			t.Change (0, Timeout.Infinite);
		}

		[TestMethod]
		public void SetRootVisualTest () {
			TextBlock b = new TextBlock ();
			Timer t = new Timer (delegate {
				Assert.Throws<UnauthorizedAccessException> (delegate {
					Application.Current.RootVisual = b;
				});
			});
			t.Change (0, Timeout.Infinite);
		}

		[TestMethod]
		public void EventTest () {
			TextBlock e1 = new TextBlock ();
			Timer t = new Timer (delegate {
				Assert.Throws<UnauthorizedAccessException> (delegate {
					e1.GotFocus += delegate (object sender, RoutedEventArgs ev) {

					};
				});
			});
			t.Change (0, Timeout.Infinite);
		}

		[TestMethod]
		public void CreateDependencyObjectTest () {
			Timer t = new Timer (delegate {
				Assert.Throws<UnauthorizedAccessException> (delegate {
					new TextBlock ();
				});
			});
			t.Change (0, Timeout.Infinite);
		}

		[TestMethod]
		public void DependencyObjectFindNameTest () {
			TextBlock b = new TextBlock ();
			Timer t = new Timer (delegate {
				Assert.Throws<UnauthorizedAccessException> (delegate {
					b.FindName ("t");
				});
			});
			t.Change (0, Timeout.Infinite);
		}

		[TestMethod]
		public void CreateDependencyPropertyTest () {
			TextBlock e1 = new TextBlock ();
			Timer t = new Timer (delegate {
				Assert.Throws<UnauthorizedAccessException> (delegate {
					e1.Text = "";
				});
			});
			t.Change (0, Timeout.Infinite);
		}

		[TestMethod]
		public void DispatcherTimerTest () {
			Timer t = new Timer (delegate {
				Assert.Throws<UnauthorizedAccessException> (delegate {
					new DispatcherTimer ();
				});
			});
			t.Change (0, Timeout.Infinite);
		}
	}
}
