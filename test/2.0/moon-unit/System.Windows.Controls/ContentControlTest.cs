//
// ContentControl Unit Tests
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
using System.Windows;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class ContentControlTest {

		class ContentControlPoker : ContentControl {

			public object DefaultStyleKey_ {
				get { return base.DefaultStyleKey; }
				set { base.DefaultStyleKey = value; }
			}

			public object OldContent;
			public object NewContent;

			protected override void OnContentChanged (object oldContent, object newContent)
			{
				OldContent = oldContent;
				NewContent = newContent;
				base.OnContentChanged (oldContent, newContent);
			}
		}

		[TestMethod]
		public void DefaultProperties ()
		{
			ContentControl cc = new ContentControl ();
			// default properties on ContentControl
			Assert.IsNull (cc.Content, "Content");
			Assert.IsNull (cc.ContentTemplate, "ContentTemplate");

			// default properties on Control...
			ControlTest.CheckDefaultProperties (cc);
		}

		[TestMethod]
		[MoonlightBug]
		public void PeekProperties ()
		{
			ContentControlPoker cc = new ContentControlPoker ();
			Assert.IsNotNull (cc.DefaultStyleKey_, "DefaultStyleKey");
		}

		[TestMethod]
		public void DefaultMethods ()
		{
			ContentControl cc = new ContentControl ();
			ControlTest.CheckDefaultMethods (cc);
		}

		[TestMethod]
		public void Content ()
		{
			ContentControlPoker cc = new ContentControlPoker ();
			cc.Content = cc;
			Assert.IsNull (cc.OldContent, "OldContent/OldContent");
			Assert.AreSame (cc, cc.NewContent, "OldContent/NewContent");
		}

		[TestMethod]
		[MoonlightBug ("can't assign DataTemplate to ContentTemplate")]
		public void ContentTemplate ()
		{
			ContentControlPoker cc = new ContentControlPoker ();
			// Note: OnContentTemplateChanged was "removed" in SL2 final
			cc.ContentTemplate = new DataTemplate ();
			// and not merged (as expected) with OnContentChanged
			Assert.IsNull (cc.OldContent, "ContentTemplate/OldContent");
			Assert.IsNull (cc.NewContent, "ContentTemplate/NewContent");
		}
	}
}
