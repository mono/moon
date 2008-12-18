//
// NotifyCollectionChangedEventArgs Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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
using System.Collections.Specialized;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Collections.Specialized {

	[TestClass]
	public class NotifyCollectionChangedEventArgsTest {

		[TestMethod]
		public void Ctor_Action ()
		{
			Assert.Throws<NotSupportedException> (delegate {
				new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add);
			}, "Add");

			Assert.Throws<NotSupportedException> (delegate {
				new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove);
			}, "Remove");

			Assert.Throws<NotSupportedException> (delegate {
				new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Replace);
			}, "Replace");

			NotifyCollectionChangedEventArgs e = new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset);
			Assert.AreEqual (NotifyCollectionChangedAction.Reset, e.Action, "Action");
			Assert.IsNull (e.NewItems, "NewItems");
			Assert.AreEqual (-1, e.NewStartingIndex, "NewStartingIndex");
			Assert.IsNull (e.OldItems, "OldItems");
			Assert.AreEqual (-1, e.OldStartingIndex, "OldStartingIndex");

			Assert.Throws<NotSupportedException> (delegate {
				NotifyCollectionChangedAction action = (NotifyCollectionChangedAction) Int32.MinValue;
				new NotifyCollectionChangedEventArgs (action);
			}, "Bad");
		}

		[TestMethod]
		public void Ctor_ActionObjectInt ()
		{
			NotifyCollectionChangedEventArgs e = new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add, this, 1);
			Assert.AreEqual (NotifyCollectionChangedAction.Add, e.Action, "Action");
			Assert.AreSame (this, e.NewItems [0], "NewItems");
			Assert.AreEqual (1, e.NewStartingIndex, "NewStartingIndex");
			Assert.IsNull (e.OldItems, "OldItems");
			Assert.AreEqual (-1, e.OldStartingIndex, "OldStartingIndex");

			e = new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove, this, 1);
			Assert.AreEqual (NotifyCollectionChangedAction.Remove, e.Action, "Action");
			Assert.IsNull (e.NewItems, "NewItems");
			Assert.AreEqual (-1, e.NewStartingIndex, "NewStartingIndex");
			Assert.AreSame (this, e.OldItems [0], "OldItems");
			Assert.AreEqual (1, e.OldStartingIndex, "OldStartingIndex");

			Assert.Throws<NotSupportedException> (delegate {
				new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Replace, this, 1);
			}, "Replace");

			Assert.Throws<NotSupportedException> (delegate {
				new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset, this, 1);
			}, "Reset");

			Assert.Throws<NotSupportedException> (delegate {
				NotifyCollectionChangedAction action = (NotifyCollectionChangedAction) Int32.MinValue;
				new NotifyCollectionChangedEventArgs (action, this, 1);
			}, "Bad");
		}

		[TestMethod]
		public void Ctor_ActionObjectObjectInt ()
		{
			Assert.Throws<NotSupportedException> (delegate {
				new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Add, this, this, 1);
			}, "Add");

			Assert.Throws<NotSupportedException> (delegate {
				new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Remove, this, this, 1);
			}, "Remove");

			NotifyCollectionChangedEventArgs e = new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Replace, this, this, 1);
			Assert.AreEqual (NotifyCollectionChangedAction.Replace, e.Action, "Action");
			Assert.AreSame (this, e.NewItems[0], "NewItems");
			Assert.AreEqual (1, e.NewStartingIndex, "NewStartingIndex");
			Assert.AreSame (this, e.OldItems[0], "OldItems");
			Assert.AreEqual (-1, e.OldStartingIndex, "OldStartingIndex");

			Assert.Throws<NotSupportedException> (delegate {
				new NotifyCollectionChangedEventArgs (NotifyCollectionChangedAction.Reset, this, this, 1);
			}, "Remove");

			Assert.Throws<NotSupportedException> (delegate {
				NotifyCollectionChangedAction action = (NotifyCollectionChangedAction) Int32.MinValue;
				new NotifyCollectionChangedEventArgs (action, this, this, 1);
			}, "Bad");
		}
	}
}
