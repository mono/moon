//
// HtmlElement.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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

namespace System.Windows.Browser
{
	public sealed class HtmlElement : HtmlObject
	{
		// When does this .ctor make sense?
		internal HtmlElement ()
		{
		}

		internal HtmlElement (IntPtr handle)
			: base (handle, false)
		{
		}

		public void AppendChild (HtmlElement element)
		{
			Invoke ("appendChild", element);
		}

		public void AppendChild (HtmlElement element, HtmlElement referenceElement)
		{
			Invoke ("insertBefore", element, referenceElement);
		}

		public void Focus ()
		{
			Invoke ("focus");
		}

		public string GetAttribute (string name)
		{
			return (string) Invoke ("getAttribute", name);
		}

		public string GetStyleAttribute (string name)
		{
			ScriptObject so = GetProperty ("style") as ScriptObject;
			if (so == null)
				return null;

			string o = so.GetProperty (name) as string;
			if (o == null || o.Equals (String.Empty))
				return null;
			return o;
		}

		public void RemoveAttribute (string name)
		{
			Invoke ("removeAttribute", name);
		}

		public void RemoveChild (HtmlElement element)
		{
			Invoke ("removeChild", element);
		}

				
		public void RemoveStyleAttribute (string name)
		{
			ScriptObject so = GetProperty ("style") as ScriptObject;
			if (so == null)
				return;
			so.Invoke ("removeProperty", name);
		}

		public void SetAttribute (string name, string value)
		{
			Invoke ("setAttribute", name, value);
		}

		public void SetStyleAttribute (string name, string value)
		{
			ScriptObject so = GetProperty ("style") as ScriptObject;
			if (so == null)
				return;
			so.SetProperty (name, value);
		}

		public ScriptObjectCollection Children {
			get { return GetProperty ("childNodes") as ScriptObjectCollection; }
		}

		public string CssClass {
			get { return (string) GetProperty ("class"); }
			set { SetProperty ("class", value); }
		}

		public string Id {
			get { return (string) GetProperty ("id"); }
			set { SetProperty ("id", value); }
		}

		public HtmlElement Parent {
			get {
				return (HtmlElement) GetProperty ("parentNode");
			}
		}

		public string TagName {
			get { return (string) GetProperty ("tagName"); }
		}
	}
}

