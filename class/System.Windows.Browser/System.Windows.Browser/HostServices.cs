//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 20010 Novell, Inc (http://www.novell.com)
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

using System.Reflection;

namespace System.Windows.Browser {

	internal class HostServices {
		[ScriptableMember(ScriptAlias="createObject")]
		public ScriptObject CreateObject (string name)
		{
			if (!HtmlPage.ScriptableTypes.ContainsKey (name))
				return null;

			object o = Activator.CreateInstance (HtmlPage.ScriptableTypes[name]);

			return new ManagedObject (o);
		}

		[ScriptableMember(ScriptAlias="jsonSerialize")]
		public string JsonSerialize (ScriptObject obj)
		{
			throw new NotImplementedException ();
		}

		[ScriptableMember(ScriptAlias="requiresManagedSerializer")]
		public bool RequiresManagedSerializer (ScriptObject obj)
		{
			throw new NotImplementedException ();
		}

		public override string ToString ()
		{
			return "[ManagedServices]";
		}


		static HostServices services = new HostServices ();

		public static HostServices Services {
			get { return services; }
		}
	}

}