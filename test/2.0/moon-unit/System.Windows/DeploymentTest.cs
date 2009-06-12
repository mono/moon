//
// Deployment Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class DeploymentTest {

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
	}
}

