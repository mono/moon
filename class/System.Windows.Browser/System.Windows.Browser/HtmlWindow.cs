//
// System.Windows.Browser.HtmlObject class
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
using System.Security;

namespace System.Windows.Browser
{
	public class HtmlWindow : HtmlObject
	{	
		internal HtmlWindow (IntPtr handle)
				: base (handle)
		{
		}
	
		public string Prompt (string promptText)
		{
			throw new NotImplementedException ();
		}
		
		[SecuritySafeCritical ()]
		public object Eval (string code)
		{
			IntPtr result;
			result = Mono.NativeMethods.plugin_instance_evaluate (WebApplication.Current.PluginHandle, code);
			
			if (result == IntPtr.Zero) {
				return null;
			} else {
				throw new NotImplementedException ();
			}
		}
		
		public bool Confirm (string confirmText)
		{
			throw new NotImplementedException ();
		}
		
		public void Alert (string alertText)
		{
			throw new NotImplementedException ();
		}
		
		public void Navigate (Uri navigateToUri)
		{
			new HtmlWindow (InvokeInternal<IntPtr> (HtmlPage.Window.Handle, "open", navigateToUri, null));
		}
		
		public HtmlWindow Navigate (Uri navigateToUri, string target)
		{
			return new HtmlWindow (InvokeInternal<IntPtr> (HtmlPage.Window.Handle, "open", navigateToUri, target));
		}

		public HtmlWindow Navigate (Uri navigateToUri, string target, string targetFeatures)
		{
			return new HtmlWindow (InvokeInternal<IntPtr> (HtmlPage.Window.Handle, "open", navigateToUri, target, targetFeatures));
		}

		public void NavigateToBookmark (string bookmark)
		{
			CurrentBookmark = bookmark;
		}

		public string CurrentBookmark {
			get {
				IntPtr loc = GetPropertyInternal<IntPtr> (HtmlPage.Document.Handle, "location");
				string hash = GetPropertyInternal<string> (loc, "hash");

				if (hash == null || hash [0] != '#')
					return null;
				return hash.Substring (1, hash.Length - 1);
			}
			set {
				IntPtr loc = GetPropertyInternal<IntPtr> (HtmlPage.Document.Handle, "location");
				SetPropertyInternal (loc, "hash", String.Concat ("#", value));
			}
		}


		public ScriptObject CreateInstance (string typeName, params object [] args)
		{
			throw new NotImplementedException ();
		} 
	}
}
