/*
 * HtmlPopupWindowOptions.cs.
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

namespace System.Windows.Browser
{
	public sealed class HtmlPopupWindowOptions
	{
		int height;
		int width;
		int left;
		int top;
		bool directories;
		bool location;
		bool menubar;
		bool resizeable;
		bool scrollbars;
		bool status;
		bool toolbar;
		
		public HtmlPopupWindowOptions()
		{
		}

		public int Height {
			get { return height; }
			set { height = value; }
		}

		public int Width {
			get { return width; }
			set { width = value; }
		}

		public int Left {
			get { return left; }
			set { left = value; }
		}

		public int Top {
			get { return top; }
			set { top = value; }
		}

		public bool Directories {
			get { return directories; }
			set { directories = value; }
		}

		public bool Location {
			get { return location; }
			set { location = value; }
		}

		public bool Menubar {
			get { return menubar; }
			set { menubar = value; }
		}

		public bool Resizeable {
			get { return resizeable; }
			set { resizeable = value; }
		}

		public bool Scrollbars {
			get { return scrollbars; }
			set { scrollbars = value; }
		}
				
		public bool Status {
			get { return status; }
			set { status = value; }
		}

		public bool Toolbar {
			get { return toolbar; }
			set { toolbar = value; }
		}		
	}
}
