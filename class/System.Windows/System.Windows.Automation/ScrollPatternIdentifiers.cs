/*
 * ScrollPatternIdentifiers.cs.
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

namespace System.Windows.Automation
{
	public static class ScrollPatternIdentifiers
	{
		public static readonly AutomationProperty HorizontallyScrollableProperty = new AutomationProperty ();
		public static readonly AutomationProperty HorizontalScrollPercentProperty = new AutomationProperty ();
		public static readonly AutomationProperty HorizontalViewSizeProperty = new AutomationProperty ();
		public static readonly AutomationProperty VerticallyScrollableProperty = new AutomationProperty ();
		public static readonly AutomationProperty VerticalScrollPercentProperty = new AutomationProperty ();
		public static readonly AutomationProperty VerticalViewSizeProperty = new AutomationProperty ();
		public const double NoScroll = -1.0;
	}
}
