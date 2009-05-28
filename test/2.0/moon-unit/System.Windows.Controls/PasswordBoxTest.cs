//
// PasswordBox Unit Tests
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
    public class PasswordBoxTest
    {
        PasswordBox box;

        [TestInitialize]
        public void Setup()
        {
            box = new PasswordBox();
        }

        [TestMethod]
        public void Defaults()
        {
            Assert.AreEqual(null, box.FontSource, "#1");
            Assert.AreEqual(0, box.MaxLength, "#2");
            Assert.AreEqual("", box.Password, "#3");
            Assert.AreEqual((char)9679, box.PasswordChar, "#4");
            Assert.AreEqual(null, box.SelectionBackground, "#5");
            Assert.AreEqual(null, box.SelectionForeground, "#6");
        }

        [TestMethod]
        [MoonlightBug]
        public void SetInvalidValues()
        {
            Assert.Throws<ArgumentNullException>(delegate {
                box.Password = null;
            }, "#1");
            Assert.Throws<ArgumentOutOfRangeException>(delegate {
                box.MaxLength = -1;
            }, "#2");
            Assert.Throws<ArgumentException>(delegate {
                box.SetValue(PasswordBox.MaxLengthProperty, -1);
            }, "#3");
            box.MaxLength = Int32.MaxValue;

            Assert.Throws<XamlParseException>(delegate {
                object block = XamlReader.Load(@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">

	<PasswordBox MaxLength=""-1"" />
</Canvas>");
            }, "#4");
        }
        
        [TestMethod]
        [MoonlightBug ("throws ArgumentException instead of Exception")]
        public void SetPasswordTest ()
        {
            PasswordBox box = new PasswordBox ();
            box.Password = "Password";
            Assert.AreEqual ("Password", box.Password, "#1");
            
            Assert.Throws<Exception> (delegate {
                new PasswordBox ().SetValue (TextBox.TextProperty, "Test");
            }, "#1");
        }
        
        [TestMethod]
        [MoonlightBug ("throws ArgumentException instead of Exception")]
        public void SetMaxLengthTest ()
        {
            PasswordBox box = new PasswordBox ();
            box.MaxLength = 5;
            Assert.AreEqual (5, box.MaxLength, "#1");
                
            Assert.Throws<Exception> (delegate {
                new PasswordBox ().SetValue (TextBox.TextProperty, "Test");
            }, "#2");
        }
    }
}
