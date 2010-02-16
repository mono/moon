//
// Deployment Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009-2010 Novell, Inc.
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
using System.Windows;
using System.Threading;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class DeploymentTest : SilverlightTest {

		[TestMethod]
		public void ExternalCallersFromCrossDomain_ReadOnly ()
		{
			Deployment d = Deployment.Current;

			Assert.AreEqual (CrossDomainAccess.NoAccess, d.ExternalCallersFromCrossDomain, "Property");
			CrossDomainAccess cad = (CrossDomainAccess) d.GetValue (Deployment.ExternalCallersFromCrossDomainProperty);
			Assert.AreEqual (CrossDomainAccess.NoAccess, cad, "GetValue/Default");

			Assert.Throws<ArgumentException> (delegate {
				d.SetValue (Deployment.ExternalCallersFromCrossDomainProperty, cad);
			}, "SetValue/Default");
			Assert.Throws<ArgumentException> (delegate {
				d.SetValue (Deployment.ExternalCallersFromCrossDomainProperty, CrossDomainAccess.NoAccess);
			}, "SetValue/NoAccess");
			Assert.Throws<ArgumentException> (delegate {
				d.SetValue (Deployment.ExternalCallersFromCrossDomainProperty, CrossDomainAccess.ScriptableOnly);
			}, "SetValue/ScriptableOnly");
			Assert.Throws<ArgumentException> (delegate {
				d.SetValue (Deployment.ExternalCallersFromCrossDomainProperty, (CrossDomainAccess)1);
			}, "SetValue/FullAccess(in beta)");
		}

		[TestMethod]
		public void Current ()
		{
			Deployment d = Deployment.Current;
			Assert.IsNotNull (d, "Current");
			Assert.IsNotNull (d.Dispatcher, "");
			Assert.AreEqual ("moon-unit", d.EntryPointAssembly, "EntryPointAssembly");
			Assert.AreEqual (typeof (App).FullName, d.EntryPointType, "EntryPointType");
			Assert.AreEqual (0, d.ExternalParts.Count, "ExternalParts");
			Assert.IsNull (d.OutOfBrowserSettings, "OutOfBrowserSettings");
			Assert.IsTrue (d.Parts.Count > 1, "Parts");
			Assert.IsNotNull (d.RuntimeVersion, "RuntimeVersion"); // not identical to Environment.Version
		}

		[TestMethod]
		[Asynchronous]
		public void Current_Dispatched ()
		{
			bool complete = false;
			bool status = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			Application.Current.RootVisual.Dispatcher.BeginInvoke (() => {
				try {
					Assert.AreEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");
					Current ();
					status = true;
				}
				finally {
					complete = true;
					Assert.IsTrue (status, "Success");
				}
			});
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void Current_UserThread ()
		{
			bool complete = false;
			bool status = false;
			int tid = Thread.CurrentThread.ManagedThreadId;
			Thread t = new Thread (() => {
				try {
					Assert.AreNotEqual (Thread.CurrentThread.ManagedThreadId, tid, "ManagedThreadId");

					Deployment d = Deployment.Current;
					Assert.IsNotNull (d, "Current");
					Assert.IsNotNull (d.Dispatcher, "Dispatcher");

					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (Deployment.Current.EntryPointAssembly);
					}, "EntryPointAssembly");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (d.EntryPointType);
					}, "EntryPointType");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (d.ExternalParts);
					}, "ExternalParts");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (d.OutOfBrowserSettings);
					}, "OutOfBrowserSettings");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (d.Parts);
					}, "Parts");
					Assert.Throws<UnauthorizedAccessException> (delegate {
						Assert.IsNotNull (d.RuntimeVersion);
					}, "RuntimeVersion");

					status = true;
				}
				finally {
					complete = true;
					Assert.IsTrue (status, "Success");
				}
			});
			t.Start ();
			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}
	}
}

