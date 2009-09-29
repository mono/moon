//
// HtmlEventArgs.cs
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

namespace System.Windows.Browser
{
	public class HtmlEventArgs : EventArgs
	{
		HtmlObject source_element;
		int client_x, client_y, offset_x, offset_y;
		bool alt, ctrl, shift;
		MouseButtons mouse_button;
		int key_code, char_code;
		string event_type;
		ScriptObject dom_event;

		internal HtmlEventArgs (HtmlObject source,
				      int clientX, int clientY,
				      int offsetX, int offsetY,
				      bool altKey, bool ctrlKey, bool shiftKey,
				      MouseButtons mouseButton, int keyCode, int charCode,
				      string eventType, ScriptObject domEvent)
		{
			source_element = source;
			client_x = clientX;
			client_y = clientY;
			offset_x = offsetX;
			offset_y = offsetY;

			alt = altKey;
			ctrl = ctrlKey;
			shift = shiftKey;

			mouse_button = mouseButton;

			key_code = keyCode;
			char_code = charCode;

			event_type = eventType;
			dom_event = domEvent;
		}

		public void PreventDefault ()
		{
			dom_event.Invoke ("preventDefault");
		}
		
		public void StopPropagation ()
		{
			dom_event.Invoke ("stopPropagation");
		}
		
		public HtmlObject Source {
			get { return source_element; }
		}

		public string EventType {
			get { return event_type; }
		}

		public int ClientX {
			get { return client_x; }
		}

		public int ClientY {
			get { return client_y; }
		}

		public int ScreenX {
			get { return 0; }
		}

		public int ScreenY {
			get { return 0; }
		}
		
		public int OffsetX {
			get { return offset_x; }
		}

		public int OffsetY {
			get { return offset_y; }
		}

		public bool AltKey {
			get { return alt; }
		}

		public bool CtrlKey {
			get { return ctrl; }
		}

		public bool ShiftKey {
			get { return shift; }
		}

		public MouseButtons MouseButton {
			get { return mouse_button; }
		}

		public int KeyCode {
			get { return key_code; }
		}

		public int CharacterCode {
			get { return char_code; }
		}
		
		public ScriptObject EventObject {
			get { return dom_event; }
		}
	}
}

