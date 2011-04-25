//
// HtmlDocument.cs
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
using System.Collections.Generic;

namespace System.Windows.Browser
{
	public sealed class HtmlDocument : HtmlObject 
	{
		HtmlElement body;
		HtmlWindow window;

		internal HtmlDocument ()
		{
		}

		internal HtmlDocument (IntPtr handle)
			: base (handle, false)
		{
		}

		public HtmlElement DocumentElement {
			get {
				return (HtmlElement) GetProperty ("documentElement");
			}
		}

		public HtmlElement CreateElement (string tagName)
		{
			return (HtmlElement) InvokeInternal ("createElement", tagName);
		}

		public HtmlElement GetElementById (string id)
		{
			return (HtmlElement) InvokeInternal ("getElementById", id);
		}

		public ScriptObjectCollection GetElementsByTagName (string tagName)
		{
			return (ScriptObjectCollection) InvokeInternal ("getElementsByTagName", tagName);
		}
		
		public IDictionary<string,string> QueryString {
			get {
				// document.location.search
				Dictionary<string, string> res = new Dictionary<string, string> ();

#if !ANDROID_HACK
				ScriptObject loc = (ScriptObject) GetProperty ("location");
				string search = (string) loc.GetProperty ("search");

				if (search == null)
					return res;

				int li = 1;
				string name = null;
				bool in_name = true;
				for (int i = 1; i < search.Length; i++) {
					if (in_name) {
						if (search [i] == '=') {
							name = search.Substring (li, i - li);
							in_name = false;
							li = i + 1;
						}
					} else {
						if (search [i] == '&') {
							res.Add (name, search.Substring (li, i - li));
							in_name = true;
							li = i + 1;
						}
					}
				}

				if (name != null && li < search.Length)
					res.Add (name, search.Substring (li, search.Length - li));
#endif
				return res;
			}
		}
		
		public string Cookies {
			get {
				return (string) GetProperty ("cookie");
			}
			set {
				SetProperty ("cookie", value);
			}
		}

		public Uri DocumentUri {
			get {
#if ANDROID_HACK
				return new Uri ("file:///data/local/tmp");
#else
				ScriptObject location = GetProperty ("location") as ScriptObject;
				if (location == null)
					return null;

				return new Uri ((string)location.GetProperty ("href"));
#endif
			}
		}
		
		public HtmlElement Body {
			get {
				if (body == null)
					body = GetProperty ("body") as HtmlElement;

				return body;
			}
		}
		
		HtmlWindow Window {
			get {
				if (window == null)
					window = GetProperty ("defaultView") as HtmlWindow;
				return window;
			}
		}

		public void Submit ()
		{
			ScriptObjectCollection forms = GetElementsByTagName ("form") as ScriptObjectCollection;
			if (forms.Count < 1)
				return;
			forms [0].Invoke ("submit");
		}

		public void Submit (string formId)
		{
			HtmlElement form = GetElementById (formId) as HtmlElement;
			if (form == null)
				return;
			form.Invoke ("submit");
		}
		
		public bool IsReady {
			get {
#if ANDROID_HACK
				return true;
#else
				throw new NotImplementedException ();
#endif
			  }
		}
		
		
		public event EventHandler DocumentReady {
			add { Window.AttachEvent ("load", value); }
			remove { Window.DetachEvent ("load", value); }
		}
	}
}

