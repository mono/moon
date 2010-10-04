//
// HyperlinkButton Unit Tests
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
using System.Windows;
using System.ComponentModel;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Threading;
using System.Threading;

using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;
using MoonTest.System.Windows.Controls.Primitives;
using MoonTest.System.Windows.Data;

namespace MoonTest.System.Windows.Controls
{
	public class MyHyperlinkButton : HyperlinkButton
	{
		public void Clickify ()
		{
			OnClick ();
		}
	}

	public class MyHyperlinkTarget : TextBox, INavigate
	{
		public MyHyperlinkTarget ()
		{
			Text = "This is a hyperlink target";
		}
		
		public bool Navigated { get; set; }
		
		public bool Navigate (Uri uri) { Navigated = true; return true; }
	}
	
	[TestClass]
	public partial class HyperlinkButtonTest : SilverlightTest
	{
		[TestMethod]
		[Asynchronous]
		public void RelativeUrisNotAllowedTest ()
		{
			MyHyperlinkButton hlb = new MyHyperlinkButton ();
			hlb.NavigateUri = new Uri ("Relative", UriKind.Relative);
			hlb.TargetName = "";
			Canvas root = new Canvas ();
			root.Children.Add (hlb);
			
			CreateAsyncTest (root, () => {
				Assert.Throws<NotSupportedException>(() => { hlb.Clickify (); }, "Relative URI test");
			});
		}
		
		[TestMethod]
		[Asynchronous]
		public void DeepLinkingNoTargetNameTest ()
		{
			MyHyperlinkTarget target = new MyHyperlinkTarget ();
			MyHyperlinkButton hlb = new MyHyperlinkButton ();
			hlb.NavigateUri = new Uri ("DeepLink", UriKind.Relative);
			
			Canvas root = new Canvas ();
			root.Children.Add (target);
			root.Children.Add (hlb);
			
			CreateAsyncTest (root, () => {
				hlb.Clickify ();
				Assert.IsTrue (target.Navigated, "Deep linked target navigation");
			});
		}
	}
}
