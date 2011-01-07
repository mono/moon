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
		static ScriptObject comparer;

		internal enum NodeType : int {
			NotSet		= 0,
			Element 	= 1,
			Attribute 	= 2,
			Text		= 3,
			CDATA		= 4,
			EntityRef	= 5,
			Entity		= 6,
			Instruction = 7,
			Comment		= 8,
			Document	= 9,
			DocType		= 10,
			DocFragment	= 11,
			Notation	= 12,
		}

		NodeType type;
		NodeType Type {
			get {
				if (type == NodeType.NotSet) {
					type = (NodeType) int.Parse (GetProperty ("nodeType").ToString());
				}
				return type;
			}
		}

		static HtmlElement ()
		{
			string comparison = "(new function () {{ this.ci = function (a, b) {{ return a == b; }}; }}).ci";
			comparer = (ScriptObject) HtmlPage.Window.Eval (comparison);
		}

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

			if (!so.HasPropertyInternal (name))
				return null;

			string o = so.GetProperty (name) as string;
			return o ?? String.Empty;
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
			get { return GetProperty ("children") as ScriptObjectCollection; }
		}

		public string CssClass {
			get { return (string) GetProperty ("className"); }
			set { SetProperty ("className", value); }
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
			get {
				if (Type == NodeType.Comment)
					return "!";
				else if (Type == NodeType.Text)
					return String.Empty;
				return GetProperty ("tagName").ToString().ToLower ();
			}
		}

		public override bool Equals (object obj)
		{
			return this == (obj as HtmlElement);
		}

		public static bool operator == (HtmlElement left, HtmlElement right)
		{
			if ((object)left == (object)right)
				return true;

			if ((object)left == null || (object)right == null)
				return false;

			return (bool) comparer.InvokeSelf (left, right);
		}

		public static bool operator != (HtmlElement left, HtmlElement right)
		{
			return !(left == right);
		}

	}
}

