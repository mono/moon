//
// Unit tests for VideoBrush
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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
using System.Windows.Media;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Media {

	[TestClass]
	public partial class VideoBrushTest {

		[TestMethod]
		public void DefaultCtor ()
		{
			VideoBrush vb = new VideoBrush ();
			Assert.AreEqual (String.Empty, vb.SourceName, "SourceName");
			// VideoBrush's Transforms are non-null by default (false)
			TileBrushTest.CheckDefaults (vb, false);

			Assert.Throws<NullReferenceException> (delegate {
				vb.SetSource (null);
			}, "null");
		}

		[TestMethod]
		[MoonlightBug ("ML has the property Matrix frozen")]
		public void EnsureNotFrozen ()
		{
			VideoBrush vb = new VideoBrush ();
			Assert.IsTrue (MatrixTransformTest.CheckFreezer (vb.RelativeTransform as MatrixTransform), "RelativeTransform");
			Assert.IsTrue (MatrixTransformTest.CheckFreezer (vb.Transform as MatrixTransform), "Transform");
		}

		[TestMethod]
		[MoonlightBug ("Looks like a bad SL2 bug")]
		public void Destructive ()
		{
			VideoBrush vb = new VideoBrush ();
			// from this instance we can change all default values
			BrushTest.DestructiveRelativeTransform (vb);
			BrushTest.DestructiveTransform (vb);
			// but it's safe to execute since we revert the changes
		}

		[TestMethod]
		[MoonlightBug ("the transform should not be nullable, but returned to the default value")]
		public void RelativeTransform ()
		{
			VideoBrush vb = new VideoBrush ();
			BrushTest.RelativeTransform (vb);
		}

		[TestMethod]
		[MoonlightBug ("the transform should not be nullable, but returned to the default value")]
		public void Transform ()
		{
			VideoBrush vb = new VideoBrush ();
			BrushTest.Transform (vb);
		}

		[TestMethod]
		public void SetMediaElement ()
		{
			MediaElement media = new MediaElement ();
			VideoBrush brush = new VideoBrush ();
			object retval;
			
			brush.SourceName = "iTube";
			
			media.Name = "YouTube";
			brush.SetSource (media);
			
			Assert.AreEqual (String.Empty, brush.SourceName, "SourceName after SetSource");
			
			// since String.Empty is the default value, we also have to check RLV
			retval = brush.ReadLocalValue (VideoBrush.SourceNameProperty);
			Assert.AreNotEqual (DependencyProperty.UnsetValue, retval, "SourceName is not cleared after SetSource");
		}
	}
}
