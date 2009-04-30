//
// DataTemplate Unit Tests
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
using System.Windows.Controls;
using System.Windows;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Shapes;
using System.Windows.Media;
using System.Windows.Markup;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows
{
	public class MyControl : ContentControl { }
	public class MyDataTemplate : DataTemplate { }

	[TestClass]
	public class DataTemplateTests : SilverlightTest
	{
		[TestMethod]
		[Asynchronous]
		public void CustomDataTemplate ()
		{
			MyControl c = (MyControl) XamlReader.Load (@"
<x:MyControl xmlns=""http://schemas.microsoft.com/client/2007""
			xmlns:x=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"">
	<x:MyControl.ContentTemplate>
		<x:MyDataTemplate>
			<Rectangle />
		</x:MyDataTemplate>
	</x:MyControl.ContentTemplate>
</x:MyControl>");
			Rectangle r = new Rectangle ();
			c.Content = r;
			c.ApplyTemplate ();
			
			// Check that the visual tree matches up:
			// MyControl->ContentPresenter->Rectangle
			CreateAsyncTest (c,
				() => {
					Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (c), "#1");
					ContentPresenter presenter = (ContentPresenter) VisualTreeHelper.GetChild (c, 0);
					Assert.IsNotNull (presenter, "#2");

					Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (presenter), "#3");
					Rectangle rect = (Rectangle) VisualTreeHelper.GetChild (presenter, 0);
					Assert.IsNotNull (rect, "#4");

					Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (rect), "#5");
				}
			);
		}
	}
}
