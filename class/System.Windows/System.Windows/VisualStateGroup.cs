//
// VisualStateGroup.cs
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

using System.Windows.Markup;
using System.Collections;
using System.Collections.ObjectModel;
using Mono;

namespace System.Windows {

	[ContentPropertyAttribute("States")]
	public sealed class VisualStateGroup : DependencyObject
	{
		private static readonly DependencyProperty StatesProperty = DependencyProperty.Register ("States",
													 typeof (IList),
													 typeof (VisualStateGroup),
													 null);
		private static readonly DependencyProperty TransitionsProperty = DependencyProperty.Register ("Transitions",
													      typeof (IList),
													      typeof (VisualStateGroup),
													      null);
		private static readonly DependencyProperty NameProperty = DependencyProperty.Lookup (Kind.DEPENDENCY_OBJECT, "Name", typeof (string));
		
		public string Name {
			get { return (string) GetValue (NameProperty); }
		}

		public IList States {
			get { return (IList) GetValue(StatesProperty); }
		}

		public IList Transitions {
			get { return (IList) GetValue(TransitionsProperty); }
		}

		static object CurrentStateChangingEvent = new object ();
		static object CurrentStateChangedEvent = new object ();
		
		public event EventHandler<VisualStateChangedEventArgs> CurrentStateChanging {
			add {
				if (events[CurrentStateChangingEvent] == null)
					Events.AddHandler (this, "CurrentStateChanging", Events.current_state_changing);
				events.AddHandler (CurrentStateChangingEvent, value);
			}
			remove {
				events.RemoveHandler (CurrentStateChangingEvent, value);
				if (events[CurrentStateChangingEvent] == null)
					Events.RemoveHandler (this, "CurrentStateChanging", Events.current_state_changing);
			}
		}
		
		public event EventHandler<VisualStateChangedEventArgs> CurrentStateChanged {
			add {
				if (events[CurrentStateChangedEvent] == null)
					Events.AddHandler (this, "CurrentStateChanged", Events.current_state_changed);
				events.AddHandler (CurrentStateChangedEvent, value);
			}
			remove {
				events.RemoveHandler (CurrentStateChangedEvent, value);
				if (events[CurrentStateChangedEvent] == null)
					Events.RemoveHandler (this, "CurrentStateChanged", Events.current_state_changed);
			}
		}

		internal void InvokeCurrentStateChanging (VisualStateChangedEventArgs e)
		{
			EventHandler<VisualStateChangedEventArgs> h = (EventHandler<VisualStateChangedEventArgs>) events[CurrentStateChangingEvent];
			if (h != null)
				h (this, e);
		}
		
		internal void InvokeCurrentStateChanged (VisualStateChangedEventArgs e)
		{
			EventHandler<VisualStateChangedEventArgs> h = (EventHandler<VisualStateChangedEventArgs>) events[CurrentStateChangedEvent];
			if (h != null)
				h (this, e);
		}
	}
}