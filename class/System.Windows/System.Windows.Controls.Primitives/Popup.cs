//
// Popup.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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
using System.Collections.Generic;
using System.Windows;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Input;

namespace System.Windows.Controls.Primitives {

	public sealed partial class Popup : FrameworkElement
	{
		static UnmanagedEventHandler DoNothing = delegate { };
		static UnmanagedEventHandler isopen_changed = Events.CreateSafeHandler (delegate (IntPtr a, IntPtr b, IntPtr closure) {
			((Popup) Helper.ObjectFromIntPtr (closure)).InvokeIsOpenChanged ();
		});
		
		EventHandler closed_event;
		EventHandler opened_event;
		
		internal event EventHandler ClickedOutside;

		Canvas _clickCatcher;
		
		public event EventHandler Closed {
			add {
				if (EventList [IsOpenChangedEvent] == null)
					RegisterEvent (IsOpenChangedEvent, "IsOpenChanged", isopen_changed, DoNothing);
				closed_event += value;
			}
			remove {
				closed_event -= value;
				if (closed_event == null && opened_event == null)
					UnregisterEvent (IsOpenChangedEvent, "IsOpenChanged", isopen_changed, DoNothing);
			}
		}
		
		public event EventHandler Opened {
			add {
				if (EventList [IsOpenChangedEvent] == null)
					RegisterEvent (IsOpenChangedEvent, "IsOpenChanged", isopen_changed, DoNothing);
				opened_event += value;
			}
			remove  {
				opened_event -= value;
				if (closed_event == null && opened_event == null)
					UnregisterEvent (IsOpenChangedEvent, "IsOpenChanged", isopen_changed, DoNothing);
			}
		}
		
		static object IsOpenChangedEvent = new object ();
		
		void InvokeIsOpenChanged ()
		{
			Console.WriteLine ("\nIs Open? {0}", IsOpen);
			EventHandler h = IsOpen ? opened_event : closed_event;
			if (h != null)
				h (this, EventArgs.Empty);
		}
		
		internal void CatchClickedOutside ()
		{
			RearrangePopup ();
		}
		
		void RearrangePopup ()
		{
			if (Child == null)
				return;
			
			UIElement child = Child;
			Canvas root = new Canvas ();
			_clickCatcher = new Canvas { Background = new SolidColorBrush (Colors.Transparent) };
			Child = root;
			root.Children.Add (_clickCatcher);
			root.Children.Add (child);
			
			Child = root;

			_clickCatcher.MouseLeftButtonDown += delegate(object sender, MouseButtonEventArgs e) {
				EventHandler h = ClickedOutside;
				if (h != null)
					h (this, EventArgs.Empty);
			};
			
			_clickCatcher.Height = Application.Current.Host.Content.ActualHeight;
			_clickCatcher.Width = Application.Current.Host.Content.ActualWidth;		
		}
	}
}

