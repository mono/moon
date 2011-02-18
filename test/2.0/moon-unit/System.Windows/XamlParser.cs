//
// XamlParser Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2011 Novell, Inc.
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
using System.Windows.Markup;
using System.Windows.Media;

using Mono.Moonlight.UnitTesting;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Windows.Shapes;
using System.Collections.Generic;
using System.Windows.Controls.Primitives;

namespace MoonTest.System.Windows {

	[TestClass]
	public class XamlParserTest : SilverlightTest {

		[TestMethod]
		[Asynchronous]
		public void Ignorables ()
		{
			StackPanel c = (StackPanel) XamlReader.Load (@"
<StackPanel
	xmlns=""http://schemas.microsoft.com/client/2007""
	xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
	xmlns:d=""ignorableUri""
	xmlns:mc=""http://schemas.openxmlformats.org/markup-compatibility/2006""
	mc:Ignorable=""d""
>
    <Grid>
      <ContentPresenter />
    </Grid>
	<d:IgnoreMe>
      <StackPanel />
	</d:IgnoreMe>
    <Grid>
      <ContentPresenter />
    </Grid>
</StackPanel>
");
			CreateAsyncTest (c,
				() => {
					Assert.VisualChildren (c, "#1",
						new VisualNode<Grid> ("#a",
							new VisualNode<ContentPresenter> ("#b")
						),
						new VisualNode<Grid> ("#c",
							new VisualNode<ContentPresenter> ("#d")
						)
					);
				}
			);
		}
	}
}
