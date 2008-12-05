/*
 * HeaderCollection.cs.
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
using System.Collections.Generic;
using System.IO;
using System.Text;

static class HeaderCollection
{
	public static void Write (List<string> headers, StringBuilder text)
	{
		foreach (string header in headers) {
			text.Append ("#include \"");
			text.Append (header);
			text.AppendLine ("\"");
		}
	}
}