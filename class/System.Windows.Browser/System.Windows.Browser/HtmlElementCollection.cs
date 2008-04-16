//
// HtmlElementCollection.cs
//
// Authors:
//	Atsushi Enomoto  <atsushi@ximian.com>
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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
using System.Collections;
using System.Collections.Generic;


namespace System.Windows.Browser
{
	public sealed class HtmlElementCollection : IEnumerable, IEnumerable<HtmlElement>
	{
		private IntPtr node_list;

		internal HtmlElementCollection (IntPtr nodeList)
		{
			this.node_list = nodeList;
		}

		public int Count {
			get {
				return HtmlObject.GetPropertyInternal<int> (node_list, "length");
			}
		}

		public HtmlElement this [int i] {
			// is this approach (creating HtmlElement every time) bogus?
			get {
				return new HtmlElement (HtmlObject.InvokeInternal<IntPtr> (node_list, "item", i));
			}
		}

		IEnumerator<HtmlElement> IEnumerable<HtmlElement>.GetEnumerator ()
		{
			for (int i = 0; i < Count; i++)
				yield return this [i];
		}

		IEnumerator IEnumerable.GetEnumerator ()
		{
			for (int i = 0; i < Count; i++)
				yield return this [i];
		}
	}
}

