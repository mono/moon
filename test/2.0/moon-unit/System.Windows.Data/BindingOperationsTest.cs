//
// Unit tests for BindingOperations
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using System.Windows.Data;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Data
{
	[TestClass]
	public class BindingOperationsTest
	{
		public class CustomBinding : BindingBase { }

		public class TypeWithBindingProperty :DependencyObject {

			public static DependencyProperty TheBinding = DependencyProperty.Register ("TheBinding", typeof (Binding), typeof (TypeWithBindingProperty), null);
		}

		
		[TestMethod]
		public void CustomBindingThrowsInvalidCast ()
		{
			var target = new Rectangle ();
			var property = Rectangle.WidthProperty;
			var binding = new CustomBinding ();

			Assert.Throws<InvalidCastException> (() =>
				BindingOperations.SetBinding (target, property, binding)
			, "#1");
		}

		[TestMethod]
		[MoonlightBug ("We need to throw exception is we use Rectangle Dps on non-rectangles etc")]
		public void InvalidArguments ()
		{
			var target = new Rectangle ();
			var property = Rectangle.WidthProperty;
			var binding = new Binding ();

			Assert.Throws<Exception> (() =>
				BindingOperations.SetBinding (new Storyboard (), property, binding)
			, "#1");
		}

		[TestMethod]
		public void NullArguments ()
		{
			var target = new Rectangle ();
			var property = Rectangle.WidthProperty;
			var binding = new Binding ();

			Assert.Throws<ArgumentNullException> (() => 
				BindingOperations.SetBinding (null, property, binding)
			, "#1");

			Assert.Throws<ArgumentNullException> (() =>
				BindingOperations.SetBinding (target, null, binding)
			, "#2");

			Assert.Throws<ArgumentNullException> (() =>
				BindingOperations.SetBinding (target, property, null)
			, "#3");
		}

		[TestMethod]
		public void SetBinding_PropertyIsOfTypeBinding_DoesNotSetValue ()
		{
			var target = new TypeWithBindingProperty ();
			var property = TypeWithBindingProperty.TheBinding;
			var binding = new Binding ();

			BindingOperations.SetBinding (target, property, binding);

			Assert.IsNull (target.GetValue (property));
		}
	}
}
