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
using Mono;
using System.Runtime.InteropServices;

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
			return (string) HtmlPage.Window.Invoke ("prompt", promptText);
		}
		
		public object Eval (string code)
		{
			IntPtr result;
			result = Mono.NativeMethods.plugin_instance_evaluate (WebApplication.Current.PluginHandle, code);
			
			if (result != IntPtr.Zero) {
				Value v = (Value)Marshal.PtrToStructure (result, typeof (Value));
				return ScriptableObjectWrapper.ObjectFromValue<object> (v);
			}
			return null;
		}
		
		public bool Confirm (string confirmText)
		{
			return (bool) HtmlPage.Window.Invoke ("confirm", confirmText);
		}
		
		public void Alert (string alertText)
		{
			HtmlPage.Window.Invoke ("alert", alertText);
		}
		
		public void Navigate (Uri navigateToUri)
		{
			HtmlPage.Window.SetProperty ("location", navigateToUri.ToString ());
		}
		
		public HtmlWindow Navigate (Uri navigateToUri, string target)
		{
			return new HtmlWindow ((IntPtr) HtmlPage.Window.Invoke ("open", navigateToUri.ToString (), target));
		}

		public HtmlWindow Navigate (Uri navigateToUri, string target, string targetFeatures)
		{
			return new HtmlWindow ((IntPtr) HtmlPage.Window.Invoke ("open", navigateToUri.ToString (), target, targetFeatures));
		}

		public void NavigateToBookmark (string bookmark)
		{
			CurrentBookmark = bookmark;
		}

		public string CurrentBookmark {
			get {
				IntPtr loc = (IntPtr) HtmlPage.Document.GetProperty ("location");
				string hash = GetPropertyInternal<string> (loc, "hash");

				if (string.IsNullOrEmpty (hash) || hash [0] != '#')
					return null;
				return hash.Substring (1, hash.Length - 1);
			}
			set {
				IntPtr loc = (IntPtr) HtmlPage.Document.GetProperty ("location");
				SetPropertyInternal (loc, "hash", String.Concat ("#", value));
			}
		}

		public ScriptObject CreateInstance (string typeName, params object [] args)
		{
			string str = "new function () {{ this.ci = function ({1}) {{ return new {0} ({1}); }}; }}";

			string parms = "";
			for (int i = 0; i < args.Length; i++) {
				if (i == 0)
					parms += "arg";
				else
					parms += ",args";
				parms += i;
			}
			ScriptObject func = (ScriptObject) this.Eval (String.Format (str, typeName, parms));
			return (ScriptObject) func.Invoke ("ci", args);
		} 
	}
}
