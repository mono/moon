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

using Mono;
using System.Windows;
using System.Windows.Media;
using System.Windows.Documents;
using System.Collections.Generic;

namespace System.Windows.Controls {
	public partial class TextBox : Control {
		public FontSource FontSource {
			get { throw new NotImplementedException (); }
			set { throw new NotImplementedException (); }
		}
		
		public void Select (int start, int length)
		{
			if (start < 0)
				throw new ArgumentOutOfRangeException ("start");
			
			if (length < 0)
				throw new ArgumentOutOfRangeException ("length");
			
			// FIXME: do exceptions get thrown if the
			// start/length values are out of range? If
			// not, what is supposed to happen?
			
			NativeMethods.text_box_select (this.native, start, length);
		}

		public void SelectAll ()
		{
			NativeMethods.text_box_select_all (native);
		}
		
		static UnmanagedEventHandler selection_changed = Events.CreateSafeHandler (selection_changed_cb);
		static UnmanagedEventHandler text_changed = Events.CreateSafeHandler (text_changed_cb);
		
		static object SelectionChangedEvent = new object ();
		static object TextChangedEvent = new object ();
		
		void InvokeSelectionChanged (RoutedEventArgs args)
		{
			RoutedEventHandler h = (RoutedEventHandler) EventList [SelectionChangedEvent];
			
			if (h != null)
				h (this, args);
		}
		
		static void selection_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			TextBox textbox = (TextBox) Helper.ObjectFromIntPtr (closure);
			RoutedEventArgs args = new RoutedEventArgs (calldata);
			
			textbox.InvokeSelectionChanged (args);
		}
		
		public event RoutedEventHandler SelectionChanged {
			add {
				RegisterEvent (SelectionChangedEvent, "SelectionChanged", selection_changed, value);
			}
			remove {
				UnregisterEvent (SelectionChangedEvent, "SelectionChanged", selection_changed, value);           				
			}
		}
		
		void InvokeTextChanged (TextChangedEventArgs args)
		{
			TextChangedEventHandler h = (TextChangedEventHandler) EventList [TextChangedEvent];
			
			if (h != null)
				h (this, args);
		}
		
		static void text_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			TextBox textbox = (TextBox) Helper.ObjectFromIntPtr (closure);
			TextChangedEventArgs args = new TextChangedEventArgs (calldata);
			
			textbox.InvokeTextChanged (args);
		}
		
		public event TextChangedEventHandler TextChanged {
			add {
				RegisterEvent (TextChangedEvent, "TextChanged", text_changed, value);
			}
			remove {
				UnregisterEvent (TextChangedEvent, "TextChanged", text_changed, value);
			}
		}
	}
}
