/*
 * ScriptObjectCollection.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Collections;
using System.Collections.Generic;
using System.Security;

namespace System.Windows.Browser
{
	public sealed class ScriptObjectCollection : ScriptObject, IEnumerable, IEnumerable<ScriptObject>
	{
		private IntPtr node_list;

		internal ScriptObjectCollection (IntPtr nodeList)
		{
			this.node_list = nodeList;
		}

		public int Count {
			[SecuritySafeCritical]
			get {
				return HtmlObject.GetPropertyInternal<int> (node_list, "length");
			}
		}

		public ScriptObject this [int i] {
			// is this approach (creating HtmlElement every time) bogus?
			[SecuritySafeCritical]
			get {
				return new ScriptObject (HtmlObject.InvokeInternal<IntPtr> (node_list, "item", i));
			}
		}

		IEnumerator<ScriptObject> IEnumerable<ScriptObject>.GetEnumerator ()
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
