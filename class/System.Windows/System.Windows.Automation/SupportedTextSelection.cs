/*
 * SupportedTextSelection.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;

namespace System.Windows.Automation {
	[Flags]
	public enum SupportedTextSelection {
		None,
		Single,
		Multiple,
	}
}

