/*
 * HtmlPopupWindowOptions.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008, 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

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
		
		public HtmlPopupWindowOptions ()
		{
			height = 450;
			width = 600;
			left = 100;
			top = 100;
			directories = true;
			location = true;
			menubar = true;
			resizeable = true;
			scrollbars = true;
			status = true;
			toolbar = true;
		}

		public int Height {
			get { return height; }
			set {
				if (value < 0)
					throw new ArgumentOutOfRangeException ("Height");
				height = value;
			}
		}

		public int Width {
			get { return width; }
			set {
				if (value < 0)
					throw new ArgumentOutOfRangeException ("Width");
				width = value;
			}
		}

		public int Left {
			get { return left; }
			set {
				if (value < 0)
					throw new ArgumentOutOfRangeException ("Left");
				left = value;
			}
		}

		public int Top {
			get { return top; }
			set {
				if (value < 0)
					throw new ArgumentOutOfRangeException ("Top");
				top = value;
			}
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

		// to avoid confusing (e.g. full screen window) or annoying (e.g. out of screen window) the end-users there are 
		// some location and size restrictions (wrt physical screen) described in:
		// http://msdn.microsoft.com/en-us/library/system.windows.browser.htmlpage.popupwindow(VS.95).aspx
		internal string AsString ()
		{
			ScriptObject screen = HtmlPage.Window.GetPropertyInternal<ScriptObject> ("screen");
			int screen_width = screen == null ? 1024 : Convert.ToInt32 (screen.GetProperty ("availWidth"));
			int screen_height = screen == null ? 768 : Convert.ToInt32 (screen.GetProperty ("availHeight"));

			int top_limit = screen_height - 100;
			int left_limit = screen_width - 100;
			int height_limit = screen_height - 250;
			int width_limit = screen_width - 250;

			return String.Format ("height={0},width={1},left={2},top={3},directories={4},location={5},menubar={6},resizable={7},scrollbars={8},status={9},toolbar={10}",
						Height > height_limit ? height_limit : Height,
						Width > width_limit ? width_limit : Width, 
						Left > left_limit ? left_limit : Left, 
						Top > top_limit ? top_limit : Top,
						Directories ? "yes" : "no",
						Location ? "yes" : "no",
						Menubar ? "yes" : "no",
						Resizeable ? "yes" : "no",
						Scrollbars ? "yes" : "no",
						Status ? "yes" : "no",
						Toolbar ? "yes" : "no");
		}		
	}
}
