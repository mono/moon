using System;
using System.Windows;
using System.Windows.Controls;
using System.Threading;
using System.Diagnostics;
using System.Windows.Threading;

// test to check cross-thread accesses (they should all blow up

namespace threads3 {
	public partial class Page : UserControl {


		public Page () {
			InitializeComponent ();
			this.Loaded += new RoutedEventHandler (Page_Loaded);


			
		}

		void Page_Loaded (object sender, RoutedEventArgs e) {
			Run ();
		}

		void Run () {
			ManualResetEvent wait = new ManualResetEvent (false);

			string s = "A1";
			Timer t = new Timer (delegate {
				try {
					Application a = new Application (); // this has to blow up
					Console.Assert (false, s);
				} catch (UnauthorizedAccessException x) {
					Console.WriteLine (s + " passed");
				} catch (Exception x) {
					Console.WriteLine (s + " threw bad exception, should be UnauthorizedAccessException");
				} finally {
					wait.Set ();
				}
			});
			
			t.Change (0, Timeout.Infinite);
			wait.WaitOne ();

			wait.Reset ();
			s = "A2";
			t = new Timer (delegate {
				try {
					Application.LoadComponent (this, new System.Uri ("/threads3;component/App.xaml", System.UriKind.Relative));
					Console.Assert (false, s);
				} catch (UnauthorizedAccessException x) {
					Console.WriteLine(s + " passed");
				} catch (Exception x) {
					Console.WriteLine (s + " threw bad exception, should be UnauthorizedAccessException");
				} finally {
					wait.Set ();
				}
			});
			t.Change (0, Timeout.Infinite);
			wait.WaitOne ();

			wait.Reset ();

			s = "A3";
			t = new Timer (delegate {
				try {
					UIElement e = Application.Current.RootVisual; // this has to blow up
					Console.Assert (false, s);
				} catch (UnauthorizedAccessException x) {
					Console.WriteLine (s + " passed");
				} catch (Exception x) {
					Console.WriteLine (s + " threw bad exception, should be UnauthorizedAccessException");
				} finally {
					wait.Set ();
				}
			});
			t.Change (0, Timeout.Infinite);
			wait.WaitOne ();

			wait.Reset ();

			s = "A4";
			t = new Timer (delegate {
				try {
					Application.Current.RootVisual = this; // this has to blow up
					Console.Assert (false, s);
				} catch (UnauthorizedAccessException x) {
					Console.WriteLine (s + " passed");
				} catch (Exception x) {
					Console.WriteLine (s + " threw bad exception, should be UnauthorizedAccessException");
				} finally {
					wait.Set ();
				}
			});
			t.Change (0, Timeout.Infinite);
			wait.WaitOne ();

			wait.Reset ();

			s = "A5";
			TextBlock e1 = new TextBlock ();
			t = new Timer (delegate {
				try {
					e1.GotFocus += delegate (object sender, RoutedEventArgs ev) {
						Console.WriteLine ("should not be here");
					};
					Console.Assert (false, s);
				} catch (UnauthorizedAccessException x) {
					Console.WriteLine (s + " passed");
				} catch (Exception x) {
					Console.WriteLine (s + " threw bad exception, should be UnauthorizedAccessException");
				} finally {
					wait.Set ();
				}
			});
			t.Change (0, Timeout.Infinite);
			wait.WaitOne ();

			wait.Reset ();


			s = "A6";
			t = new Timer (delegate {
				try {
					new TextBlock ();
					Console.Assert (false, s);
				} catch (UnauthorizedAccessException x) {
					Console.WriteLine (s + " passed");
				} catch (Exception x) {
					Console.WriteLine (s + " threw bad exception, should be UnauthorizedAccessException");
				} finally {
					wait.Set ();
				}
			});
			t.Change (0, Timeout.Infinite);
			wait.WaitOne ();

			wait.Reset ();
			s = "A7";
			e1 = new TextBlock ();
			t = new Timer (delegate {
				try {
					e1.Text = s;
					Console.Assert (false, s);
				} catch (UnauthorizedAccessException x) {
					Console.WriteLine (s + " passed");
				} catch (Exception x) {
					Console.WriteLine (s + " threw bad exception, should be UnauthorizedAccessException");
				} finally {
					wait.Set ();
				}
			});
			t.Change (0, Timeout.Infinite);
			wait.WaitOne ();

			wait.Reset ();
			s = "A8";
			t = new Timer (delegate {
				try {
					new DispatcherTimer ();
					Console.Assert (false, s);
				} catch (UnauthorizedAccessException x) {
					Console.WriteLine (s + " passed");
				} catch (Exception x) {
					Console.WriteLine (s + " threw bad exception, should be UnauthorizedAccessException");
				} finally {
					wait.Set ();
				}
			});
			t.Change (0, Timeout.Infinite);
			wait.WaitOne ();

			wait.Reset ();
			s = "A9";
			e1 = new TextBlock ();
			t = new Timer (delegate {
				try {
					// ManualResetEvent wait2 = new ManualResetEvent (false);
					e1.Dispatcher.BeginInvoke (delegate {
						try {
							Console.WriteLine (s + " passed");
						} catch {
							Console.Assert (false, s);
						} finally {
							//wait2.Set ();
						}
					});
					//wait2.WaitOne (); // waiting here blocks, as dispatcher operations are invoked in 
										// the main thread, so they will only be called after this method exits
				} catch {
					Console.Assert (false, s);
				} finally {
					wait.Set ();
				}
			});
			t.Change (0, Timeout.Infinite);
			wait.WaitOne ();
		}

	}
}
