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
// Copyright (c) 2007-2008 Novell, Inc.
//
// Authors:
//	Jackson Harper (jackson@ximian.com)
//


using System;
using System.IO;
using System.Xml;
using System.Threading;
using System.Diagnostics;
using System.Collections.Generic;

using NDesk.DBus;
using org.freedesktop.DBus;


namespace MoonlightTests {

	public static class DbusServices {

		private static Thread worker_thread;
		private static readonly string BusName = "mono.moonlight.tests";

		private static List<IDbusService> services = new List<IDbusService> ();

		public static void Register (IDbusService service)
		{
			services.Add (service);			
		}

		public static void Start ()
		{
			Bus bus = Bus.Session;
			RequestNameReply reply;
			
			reply = bus.RequestName (BusName, NameFlag.ReplaceExisting | NameFlag.AllowReplacement);
			if (!(reply == RequestNameReply.PrimaryOwner)) {
				Console.Error.WriteLine ("Unable to request dbus bus name while registering object ({0})", reply);
				return;
			}

			foreach (IDbusService service in services) {
				bus.Register (BusName, service.GetObjectPath (), service);
			}

			worker_thread = new Thread (DbusWorker);
			worker_thread.IsBackground = true;
			worker_thread.Start ();
		}

		public static void Stop ()
		{
			if (worker_thread != null)
				worker_thread.Abort ();
		}

		private static void DbusWorker ()
		{
			Bus bus = Bus.Session;
			while (true)
				bus.Iterate ();
		}
	}
}

