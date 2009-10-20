//
// Dispatcher.cs
//
// Copyright 2008 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.ComponentModel;
using System.Threading;
using System.Collections.Generic;
using Mono;

namespace System.Windows.Threading {

	[CLSCompliant (false)]
	public sealed class Dispatcher {
		TickCallHandler callback;
		Queue<DispatcherOperation> queuedOperations;
		Surface surface;
		bool pending;

		static Dispatcher main;
		internal static Dispatcher Main {
			get { return main; }
		}

		static Dispatcher ()
		{
			main = new Dispatcher ();
		}

		internal Dispatcher ()
		{
			queuedOperations = new Queue<DispatcherOperation>();
			surface = Deployment.Current.Surface;
			pending = false;
		}

		~Dispatcher () {
			Dispose ();
		}

		void Dispose () {
			lock (queuedOperations) {
				if (callback != null)
					NativeMethods.time_manager_remove_tick_call (NativeMethods.surface_get_time_manager (surface.Native), callback, IntPtr.Zero);
				pending = false;
#if DEBUG_DISPATCHER
				if (queuedOperations.Count > 0) {
					Console.WriteLine ("Dispatcher was destroyed with " + queuedOperations.Count + " call to be processed");
					foreach (DispatcherOperation op in queuedOperations) {
						Console.WriteLine (op.GetDebugString ());
					}
				}
#endif
				queuedOperations.Clear ();
			}
		}

		[EditorBrowsable (EditorBrowsableState.Never)]
		public bool CheckAccess ()
		{
			return NativeMethods.surface_in_main_thread ();
		}

		public DispatcherOperation BeginInvoke (Action a)
		{
			return BeginInvoke (a, null);
		}

		public DispatcherOperation BeginInvoke (Delegate d, params object[] args)
		{
			DispatcherOperation op = null;
			lock (queuedOperations) {
				op = new DispatcherOperation (d, args);
				queuedOperations.Enqueue (op);
				if (!pending) {
					if (callback == null)
						callback = new TickCallHandler (dispatcher_callback);
					NativeMethods.time_manager_add_dispatcher_call (NativeMethods.surface_get_time_manager (surface.Native),
					                                                   callback, IntPtr.Zero);
					pending = true;
				}
			}
			return op;
		}

		internal void Invoke (Delegate d, params object[] args)
		{
			if (CheckAccess ()) {
				try {
					d.DynamicInvoke (args);
				} catch (Exception ex) {
					Application.OnUnhandledException (this, ex);
				}
			} else {
				ManualResetEvent wait = new ManualResetEvent (false);
				BeginInvoke (delegate {
					try {
						d.DynamicInvoke (args);
					} catch (Exception ex) {
						Application.OnUnhandledException (this, ex);
					} finally {
						wait.Set ();
					}
				});
				wait.WaitOne ();
			}
		}

		void Dispatch ()
		{
			DispatcherOperation[] ops;
			lock (queuedOperations) {
				ops = queuedOperations.ToArray ();
				queuedOperations.Clear ();
				pending = false;
			}

			foreach (DispatcherOperation op in ops) {
				op.Invoke ();
			}
		}

		void dispatcher_callback (IntPtr data)
		{
			try {
				Dispatch ();
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in DependencyProperty.NativePropertyChangedCallback: {0}", ex);
				} catch {
				}
			}
		}

	}

}
