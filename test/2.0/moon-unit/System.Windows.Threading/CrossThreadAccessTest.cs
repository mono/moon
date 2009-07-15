using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;

using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Threading {

	[TestClass]
	public class CrossThreadAccessTest : SilverlightTest {

		private BackgroundWorker bw = new BackgroundWorker ();
		private bool complete;
		// async tests are nice but we need to make sure the async code gets executed
		private bool executed;

		public CrossThreadAccessTest ()
		{
			bw.DoWork += delegate (object sender, DoWorkEventArgs e) {
				Action action = (e.Argument as Action);
				Assert.Throws<UnauthorizedAccessException> (delegate {
					executed = true; // been there, done that :)
					action ();
				});
			};
			bw.RunWorkerCompleted += delegate {
				complete = true;
			};
		}

		private void CrossThreadTest (Action action)
		{
			executed = complete = false;
			Enqueue (() => { bw.RunWorkerAsync (action); });
			EnqueueConditional (() => { return executed && complete; });
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void ApplicationTest () 
		{
			CrossThreadTest (() => {
				new Application ();
			});
		}

		[TestMethod]
		[Asynchronous]
		public void LoadComponentTest ()
		{
			CrossThreadTest (() => {
				Application.LoadComponent (this, new Uri ("/threads3;component/App.xaml", UriKind.Relative));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void GetRootVisualTest () 
		{
			CrossThreadTest (() => {
				Assert.IsNotNull (Application.Current.RootVisual);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void SetRootVisualTest () 
		{
			TextBlock b = new TextBlock ();
			CrossThreadTest (() => {
				Application.Current.RootVisual = b;
			});
		}

		[TestMethod]
		[Asynchronous]
		public void EventTest ()
		{
			TextBlock e1 = new TextBlock ();
			CrossThreadTest (() => {
				e1.GotFocus += delegate (object sender, RoutedEventArgs ev) {
				};
			});
		}

		[TestMethod]
		[Asynchronous]
		public void CreateDependencyObjectTest ()
		{
			CrossThreadTest (() => {
				new TextBlock ();
			});
		}

		[TestMethod]
		[Asynchronous]
		public void DependencyObjectFindNameTest ()
		{
			TextBlock b = new TextBlock ();
			CrossThreadTest (() => {
				b.FindName ("t");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void CreateDependencyPropertyTest ()
		{
			TextBlock e1 = new TextBlock ();
			CrossThreadTest (() => {
				e1.Text = "";
			});
		}

		[TestMethod]
		[Asynchronous]
		public void DispatcherTimerTest ()
		{
			CrossThreadTest (() => {
				new DispatcherTimer ();
			});
		}
	}
}

