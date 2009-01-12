//
// TextBlock Unit Tests
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
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Markup;

namespace Mono.Moonlight.UnitTesting
{
    [TestClass]
    public class TextBoxTest
    {
        TextBox box;

        [TestInitialize]
        public void Setup()
        {
            box = new TextBox();
        }

        [TestMethod]
        public void Defaults()
        {
            Assert.AreEqual(false, box.AcceptsReturn, "#1");
            Assert.AreEqual(null, box.FontSource, "#2");
            Assert.AreEqual(ScrollBarVisibility.Hidden, box.HorizontalScrollBarVisibility, "#3");
            Assert.AreEqual(false, box.IsReadOnly, "#4");
            Assert.AreEqual(0, box.MaxLength, "#5");
            Assert.AreEqual("", box.SelectedText, "#6");
            Assert.AreEqual(null, box.SelectionBackground, "#7");
            Assert.AreEqual(null, box.SelectionForeground, "#8");
            Assert.AreEqual(0, box.SelectionLength, "#9");
            Assert.AreEqual(0, box.SelectionStart, "#10");
            Assert.AreEqual("", box.Text, "#11");
            Assert.AreEqual(TextAlignment.Left, box.TextAlignment, "#12");
            Assert.AreEqual(TextWrapping.NoWrap, box.TextWrapping, "#13");
            Assert.AreEqual(ScrollBarVisibility.Hidden, box.VerticalScrollBarVisibility, "#14");
        }

        [TestMethod]
        [MoonlightBug ("Different validation in managed and unmanaged code")]
        public void InvalidValues()
        {
            Assert.Throws<ArgumentOutOfRangeException>(delegate {
                box.MaxLength = -1;
            }, "#1");
            box.SelectedText = "BLAH";
            Assert.AreEqual("BLAH", box.Text, "#2");
            Assert.Throws<ArgumentOutOfRangeException>(delegate {
                box.SelectionLength = -1;
            }, "#3");

            box.SelectionStart = 6;
            Assert.AreEqual(4, box.SelectionStart, "#3a");
            Assert.AreEqual("", box.SelectedText, "#3b");

            box.SelectionStart = 2;
            box.SelectionLength = 10;
            Assert.AreEqual(2, box.SelectionLength, "#3c");
            Assert.AreEqual("AH", box.SelectedText, "#3d");

            Assert.Throws<ArgumentOutOfRangeException>(delegate {
                box.SelectionStart = -1;
            }, "#5");
            Assert.Throws<ArgumentNullException>(delegate {
                box.Text = null;
            }, "#6");

            box.SetValue(TextBox.TextProperty, null);
            Assert.AreEqual ("", box.Text, "#7");

            box.ClearValue (TextBox.TextProperty);
            Assert.AreEqual("", box.Text, "#8");

            Assert.Throws<XamlParseException>(delegate {
                object block = XamlReader.Load(@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">

	<TextBox MaxLength=""-1"" />
</Canvas>");
            }, "#9");
        }
    }
}
