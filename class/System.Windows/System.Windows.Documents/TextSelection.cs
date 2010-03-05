// 
// TextSelection.cs
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

namespace System.Windows.Documents {
	public sealed class TextSelection {
		internal TextSelection ()
		{
			Console.WriteLine ("System.Windows.Documents.TextSelection.ctor: () NIEX");
			throw new NotImplementedException ();
		}

		public bool CanInsert (TextElement element)
		{
			Console.WriteLine ("System.Windows.Documents.TextSelection.CanInsert: () NIEX");
			throw new NotImplementedException ();
		}

		public object GetPropertyValue (DependencyProperty formattingProperty)
		{
			Console.WriteLine ("System.Windows.Documents.TextSelection.GetPropertyValue () NIEX");
			throw new NotImplementedException ();
		}

		public void SetPropertyValue (DependencyProperty formattingProperty, object value)
		{
			Console.WriteLine ("System.Windows.Documents.TextSelection.SetPropertyValue: () NIEX");
			throw new NotImplementedException ();
		}

		public void Insert (TextElement element)
		{
			Console.WriteLine ("System.Windows.Documents.TextSelection.Insert: () NIEX");
			throw new NotImplementedException ();
		}

		public string Text {
			get {
				Console.WriteLine ("System.Windows.Documents.TextSelection.get_Text: () NIEX");
				throw new NotImplementedException ();
			}
			set {
				Console.WriteLine ("System.Windows.Documents.TextSelection.set_Text: () NIEX");
				throw new NotImplementedException ();
			}
		}
	}
}

