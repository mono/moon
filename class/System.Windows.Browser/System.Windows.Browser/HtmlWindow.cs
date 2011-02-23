//
// System.Windows.Browser.HtmlWindow class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007,2009,2011 Novell, Inc (http://www.novell.com)
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

using Mono;
using System.Runtime.InteropServices;
using System.Windows.Interop;
using System.Collections.Generic;

namespace System.Windows.Browser
{
	public sealed class HtmlWindow : HtmlObject
	{
		// These targets open the target Uri in the current window, otherwise
		// the existing window is reused. List was compiled from the MSDN
		// docs for HyperLinkButton.NavigateUri
		static readonly List<string> SameWindowTargets = new List<string> {
			null,
			"",
			"_self",
		};

		internal HtmlWindow (IntPtr handle)
			: base (handle, false)
		{
		}
	
		public string Prompt (string promptText)
		{
			return (string) Invoke ("prompt", promptText ?? String.Empty);
		}
		
		public object Eval (string code)
		{
			if (code == null)
				throw new ArgumentNullException ("code");
			if (code.Length == 0)
				throw new ArgumentException ("code");

			IntPtr result;
			result = Mono.NativeMethods.plugin_instance_evaluate (PluginHost.Handle, code);
			
			if (result != IntPtr.Zero) {
				Value v = (Value)Marshal.PtrToStructure (result, typeof (Value));
				return ScriptObjectHelper.FromValue (v);
			}
			return null;
		}
		
		public bool Confirm (string confirmText)
		{
			return (bool) Invoke ("confirm", confirmText ?? String.Empty);
		}
		
		public void Alert (string alertText)
		{
			Invoke ("alert", alertText ?? String.Empty);
		}
		
		public void Navigate (Uri navigateToUri)
		{
			if (navigateToUri == null)
				throw new ArgumentNullException ("navigateToUri");

			Navigate (navigateToUri, "_self");
		}
		
		public HtmlWindow Navigate (Uri navigateToUri, string target)
		{
			if (navigateToUri == null)
				throw new ArgumentNullException ("navigateToUri");
			if (target == null)
				throw new ArgumentNullException ("target");

			return Navigate (navigateToUri, target, "");
		}

		public HtmlWindow Navigate (Uri navigateToUri, string target, string targetFeatures)
		{
			if (navigateToUri == null)
				throw new ArgumentNullException ("navigateToUri");
			if (target == null)
				throw new ArgumentNullException ("target");
			if (targetFeatures == null)
				throw new ArgumentNullException ("targetFeatures");

			if (SameWindowTargets.Contains (target)) {
				ScriptObject loc = HtmlPage.Document.GetProperty ("location") as ScriptObject;
				SetPropertyInternal (loc.Handle, "href", navigateToUri.ToString ());
				return this;
			} else {
				return (HtmlWindow) Invoke ("open", navigateToUri.ToString (), target, targetFeatures);
			}
		}

		public void NavigateToBookmark (string bookmark)
		{
			CurrentBookmark = bookmark;
		}

		public string CurrentBookmark {
			get {
				ScriptObject loc = HtmlPage.Document.GetProperty ("location") as ScriptObject;
				if (loc == null)
					return String.Empty;
				string hash = GetPropertyInternal (loc.Handle, "hash") as string;

				if (string.IsNullOrEmpty (hash) || hash [0] != '#')
					return String.Empty;
				return hash.Substring (1, hash.Length - 1);
			}
			set {
				if (value == null)
					throw new ArgumentNullException ("CurrentBookmark");

				ScriptObject loc = HtmlPage.Document.GetProperty ("location") as ScriptObject;
				SetPropertyInternal (loc.Handle, "hash", String.Concat ("#", value));
			}
		}

		public ScriptObject CreateInstance (string typeName, params object [] args)
		{
			if (typeName == null)
				throw new ArgumentNullException ("typeName");

			string str = "new function () {{ this.ci = function ({1}) {{ return new {0} ({1}); }}; }}";
			string parms = String.Empty;
			if (args != null) {
				for (int i = 0; i < args.Length; i++) {
					if (i == 0)
						parms += "arg";
					else
						parms += ",args";
					parms += i;
				}
			}
			ScriptObject func = (ScriptObject) this.Eval (String.Format (str, typeName, parms));
			if (func == null)
				throw new ArgumentException ("typeName");
			return (ScriptObject) func.Invoke ("ci", args);
		} 
	}
}
