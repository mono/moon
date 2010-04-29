//
// CollectionViewSource Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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
using System.Collections.Generic;
using System.ComponentModel;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Data {

	[TestClass]
	public partial class CollectionViewSourceTest {
		List<int> Source = new List<int> () { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

		[TestMethod]
		public void ChangeSourceRecreatesView ()
		{
			// If we change the source, we create a new View
			var source = new CollectionViewSource { Source = this.Source };
			var view = source.View;
			source.Source = new List<double> () { 1.5, 2.5, 3.5, 4.5 };
			Assert.AreNotSame (view, source.View, "#1");
		}

		[TestMethod]
		public void ViewIsReadOnly ()
		{
			var source = new CollectionViewSource { Source = this.Source };
			var view = source.View;
			Assert.IsNotNull (view, "#1");

			// We can clear the View even though it has no setter.
			source.ClearValue (CollectionViewSource.ViewProperty);
			Assert.IsNull (source.View, "#2");
		}
	}
}
