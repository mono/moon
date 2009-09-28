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
using Microsoft.Silverlight.Testing;

namespace Mono.Moonlight.UnitTesting
{
    [TestClass]
    public class TextBoxTest : SilverlightTest
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

        [TestMethod]
        public void SetIdenticalSelectedText ()
        {
            // Test setting SelectedText to the same value
            box.Text = "abcdefg";
            box.Select(3, 3);
            box.SelectedText = "def";
            // make sure Text property matches expectations
            Assert.AreEqual("abcdefg", box.Text, "#10a");
            // make sure that SelectedText is an empty string after setting it
            Assert.AreEqual("", box.SelectedText, "#10b");
            // make sure that SelectionStart is set to Start + Length
            Assert.AreEqual(6, box.SelectionStart, "#10c");
            // make sure that SelectionLength is reset to 0
            Assert.AreEqual(0, box.SelectionLength, "#10d");
        }

        [TestMethod]
        public void Selection ()
        {
            box.Text = "STARTING VALUE";
            box.Select(0, 5);

            // Test that setting new Text resets the selection
            box.Text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            // make sure selected text is reset back to ""
            Assert.AreEqual("", box.SelectedText, "#11a");
            // make sure that SelectionStart is 0
            Assert.AreEqual(0, box.SelectionStart, "#11b");
            // make sure that SelectionStart is 0
            Assert.AreEqual(0, box.SelectionLength, "#11c");

            // Test setting SelectionStart/Length result in 0 SelectionChanged events
            box.SelectionStart = 3;
            box.SelectionLength = 3;
            // make sure that manual selection result matches expectations
            Assert.AreEqual("DEF", box.SelectedText, "#12");

            // Test that Select() does not emit a SelectionChanged event
            box.Text = "ABCdefghijklmnopqrstuvwxyz";
            box.Select(0, 3);
            // make sure that the Select() result matches expectations
            Assert.AreEqual("ABC", box.SelectedText, "#13");

            // Test setting SelectedText
            box.SelectedText = "abc";
            // make sure Text property matches expectations
            Assert.AreEqual("abcdefghijklmnopqrstuvwxyz", box.Text, "#14a");
            // make sure that SelectedText is an empty string after setting it
            Assert.AreEqual("", box.SelectedText, "#14b");
            // make sure that SelectionStart is set to Start + Length
            Assert.AreEqual(3, box.SelectionStart, "#14c");
            // make sure that SelectionLength is reset to 0
            Assert.AreEqual(0, box.SelectionLength, "#14d");

            // Test SelectionChanged event if Text property is changed
            box.Select(5, 5);
            box.Text = "this should clear the selection";
            // make sure that SelectionStart is reset to 0
            Assert.AreEqual(0, box.SelectionStart, "#15a");
            // make sure that SelectionLength is reset to 0
            Assert.AreEqual(0, box.SelectionLength, "#15b");

            // Test setting SelectedText after setting fresh Text
            box.Text = "this is a cursor test...";
            box.SelectedText = "|";
            // make sure that "|" was inserted at 0
            Assert.AreEqual("|this is a cursor test...", box.Text, "#16");

            // Test inserting text into the middle of the buffer using Selection/SelectedText
            box.Text = "This is a test string";
            box.Select(8, 0);
            box.SelectedText = "not ";
            // make sure that Text property matches expectations
            Assert.AreEqual("This is not a test string", box.Text, "#17");

            // Test inserting text into the middle of the buffer using SelectionStart/SelectedText
            box.Text = "This is a test string";
            box.SelectionStart = 8;
            box.SelectionLength = 0;
            box.SelectedText = "not ";
            // make sure that Text property matches expectations
            Assert.AreEqual("This is not a test string", box.Text, "#18");
        }

        [TestMethod]
        public void SetText ()
        {
            TextBox box = new TextBox ();
            Assert.AreEqual ("", box.Text, "#1");
            Assert.Throws<ArgumentNullException> (delegate {
                box.Text = null;
            }, "#2");
            box.SetValue (TextBox.TextProperty, null);
            Assert.AreEqual ("", box.Text, "#3");
        }
        
        [TestMethod]
        [MoonlightBug]
        public void SetText2 ()
        {
            TextBox box = new TextBox { Text = "Blah" };
            Assert.AreEqual ("Blah", box.GetValue (TextBox.TextProperty), "#1");
            box.SetValue (TextBox.TextProperty, null);
            Assert.AreEqual ("", box.GetValue (TextBox.TextProperty), "#2");
	    box.Text = "o hi";
            box.ClearValue (TextBox.TextProperty);
            Assert.AreEqual ("", box.GetValue (TextBox.TextProperty), "#3");
        }
        
        [TestMethod]
        [Asynchronous]
        public void TextChangedTest ()
        {
            bool changed = false;
            TextBox box = new TextBox ();
            box.TextChanged += delegate { changed = true; };
            box.Text = "Hi";
            box.Text = "No events";
            box.Text = "Are emitted";
	    
            // Check that setting SelectedText to "" when there is no
            // selection does not cause a TextChanged event.
            box.SelectedText = "";

            // Check that the event wasn't synchronous
            Assert.IsFalse (changed, "#1");

            // Ensure that the event never fires even after the control is loaded
            CreateAsyncTest (box,
                () => Assert.IsFalse (changed, "#2"),
                () => Assert.IsFalse (changed, "#3"),
                () => Assert.IsFalse (changed, "#4"),
                () => Assert.IsFalse (changed, "#5"),
                // Lets make the event fire now
                () => box.Text = "Emit!",
                () => Assert.IsTrue (changed, "#6")
            );
        }
    }
}
