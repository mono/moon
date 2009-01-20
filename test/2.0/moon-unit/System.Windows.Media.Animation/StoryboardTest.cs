//
// Unit tests for System.Windows.Media.Animation.Storyboard
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008,2009 Novell, Inc (http://www.novell.com)
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
using System.Windows.Shapes;
using System.Windows.Media.Animation;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;
using System.Windows.Markup;
using System.Windows;

namespace MoonTest.System.Windows.Media.Animation {
	
	[TestClass]
	public class StoryboardTest {

		[TestMethod]
		public void InvalidValues ()
		{
			Rectangle r = new Rectangle ();
			Assert.Throws<Exception>(delegate {
				r.SetValue(Storyboard.TargetNameProperty, null);
			}, "#1");
			Assert.Throws<Exception>(delegate {
				r.SetValue(Storyboard.TargetNameProperty, "");
			}, "#1b");
			Assert.Throws<Exception>(delegate {
				r.SetValue(Storyboard.TargetPropertyProperty, null);
			}, "2");
		}

		[TestMethod]
		public void NameAndKey()
		{
			Storyboard board = (Storyboard)XamlReader.Load(
@"		
<Storyboard xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
            x:Name=""Blah""
            x:Key=""Blah"" />
");
			Assert.AreEqual("Blah", board.GetValue(FrameworkElement.NameProperty), "#1");
			board = (Storyboard)XamlReader.Load(
@"	
<Storyboard xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
            x:Name=""Blah2""
            x:Key=""Blah"" />
");
			Assert.AreEqual("Blah2", board.GetValue(FrameworkElement.NameProperty), "#1");
			board = (Storyboard)XamlReader.Load(
@"	
<Storyboard xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
            x:Key=""Blah""
            x:Name=""Blah2"" />
");
			Assert.AreEqual("Blah2", board.GetValue(FrameworkElement.NameProperty), "#1");
		}

		[TestMethod]
		public void SetTarget ()
		{
			Rectangle r = new Rectangle ();
			Assert.Throws<ArgumentNullException> (delegate {
				Storyboard.SetTarget (null, r);
			}, "null, do");

			Timeline t = (Timeline) new ColorAnimation ();
			Assert.Throws<ArgumentNullException> (delegate {
				Storyboard.SetTarget (t, null);
			}, "timeline, null");
		}

		[TestMethod]
		public void SetTargetName ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				Storyboard.SetTargetName (null, "moon");
			}, "null, string");
		}

		[TestMethod]
		[MoonlightBug ("ML throws an exception when a null value is being set")]
		public void SetTargetName_NullName ()
		{
			Timeline t = (Timeline) new ColorAnimation ();
			Storyboard.SetTargetName (t, null);
			// and the behavior is not specific to Storyboard.SetTargetName
			t.SetValue (Storyboard.TargetNameProperty, (string)null);
		}

		[TestMethod]
		public void SetTargetProperty ()
		{
			PropertyPath pp = new PropertyPath ("/moon");
			Assert.Throws<ArgumentNullException> (delegate {
				Storyboard.SetTargetProperty (null, pp);
			}, "null, PropertyPath");

			Timeline t = (Timeline) new ColorAnimation ();
			Assert.Throws<ArgumentNullException> (delegate {
				Storyboard.SetTargetProperty (t, null);
			}, "timeline, null");
		}

		[TestMethod]
		public void GetTargetName ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				Storyboard.GetTargetName (null);
			}, "null");

			Timeline t = (Timeline) new ColorAnimation ();
			Assert.IsNull (Storyboard.GetTargetName (t), "GetTargetName(ColorAnimation)");
		}

		[TestMethod]
		public void GetTargetProperty ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				Storyboard.GetTargetProperty (null);
			}, "null");
		}
	}
}
