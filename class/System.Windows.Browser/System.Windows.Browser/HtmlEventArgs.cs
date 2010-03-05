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
		ScriptObject eventObject;
		string eventType;

		enum EventKind {
			Mouse,
			Key,
			Html
		}

		EventKind eventKind;

		internal HtmlEventArgs (HtmlObject source,
					ScriptObject eventObj)
		{
			this.source_element = source;
			this.eventObject = eventObj;

			eventType = eventObject.GetPropertyInternal<string>("type");

			if (eventType.StartsWith ("click") ||
			    eventType.StartsWith ("dblclick") ||
			    eventType.StartsWith ("mouse"))
				eventKind = EventKind.Mouse;
			else if (eventType.StartsWith ("key")) {
				eventKind = EventKind.Key;
			}
		}

		public void PreventDefault ()
		{
			eventObject.Invoke ("preventDefault");
		}
		
		public void StopPropagation ()
		{
			eventObject.Invoke ("stopPropagation");
		}
		
		public HtmlObject Source {
			get { return source_element; }
		}

		public string EventType {
			get { return eventType; }
		}

		public int ClientX {
			get { return eventKind == EventKind.Mouse ? eventObject.GetPropertyInternal<int>("clientX") : 0; }
		}

		public int ClientY {
			get { return eventKind == EventKind.Mouse ? eventObject.GetPropertyInternal<int>("clientY") : 0; }
		}

		public int ScreenX {
			get { return eventKind == EventKind.Mouse ? eventObject.GetPropertyInternal<int>("screenX") : 0; }
		}

		public int ScreenY {
			get { return eventKind == EventKind.Mouse ? eventObject.GetPropertyInternal<int>("screenY") : 0; }
		}
		
		public int OffsetX {
			get { return eventKind == EventKind.Mouse ? eventObject.GetPropertyInternal<int>("offsetX") : 0; }
		}

		public int OffsetY {
			get { return eventKind == EventKind.Mouse ? eventObject.GetPropertyInternal<int>("offsetY") : 0; }
		}

		public bool AltKey {
			get { return eventKind != EventKind.Html ? eventObject.GetPropertyInternal<bool>("altKey") : false; }
		}

		public bool CtrlKey {
			get { return eventKind != EventKind.Html ? eventObject.GetPropertyInternal<bool>("ctrlKey") : false; }
		}

		public bool ShiftKey {
			get { return eventKind != EventKind.Html ? eventObject.GetPropertyInternal<bool>("shiftKey") : false; }
		}

		public MouseButtons MouseButton {
			get { return eventKind == EventKind.Mouse ? (MouseButtons)eventObject.GetPropertyInternal<int>("button") : (MouseButtons)0; }
		}

		public int KeyCode {
			get { return eventKind == EventKind.Key ? eventObject.GetPropertyInternal<int>("keyCode") : 0; }
		}

		public int CharacterCode {
			get {
				if (eventKind != EventKind.Key)
					return 0;

				int code = eventObject.GetPropertyInternal<int>("charCode");
				if (code != 0)
					return code;
				return eventObject.GetPropertyInternal<int>("keyCode");
			}
		}
		
		public ScriptObject EventObject {
			get { return eventObject; }
		}
	}
}

