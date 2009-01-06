//
// PasswordBox.cs
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
using System;
using System.Windows;
using System.Windows.Documents;
using Mono;

namespace System.Windows.Controls
{
	[TemplatePartAttribute(Name = "ContentElement", Type = typeof(FrameworkElement))]
	[TemplateVisualStateAttribute(Name = "Disabled", GroupName = "CommonStates")]
	[TemplatePartAttribute(Name = "DisabledVisualElement", Type = typeof(FrameworkElement))]
	[TemplateVisualStateAttribute(Name = "Focused", GroupName = "FocusStates")]
	[TemplatePartAttribute(Name = "FocusVisualElement", Type = typeof(FrameworkElement))]
	[TemplateVisualStateAttribute(Name = "MouseOver", GroupName = "CommonStates")]
	[TemplateVisualStateAttribute(Name = "Normal", GroupName = "CommonStates")]
	[TemplatePartAttribute(Name = "RootElement", Type = typeof(FrameworkElement))]
	[TemplateVisualStateAttribute(Name = "Unfocused", GroupName = "FocusStates")]
	public sealed partial class PasswordBox : Control
	{
		static readonly UnmanagedEventHandler password_changed = new UnmanagedEventHandler (password_changed_cb);
		static readonly object PasswordChangedEvent = new object ();
		
		public PasswordBox (string s)
			: base (NativeMethods.password_box_new ())
		{

		}

		public event RoutedEventHandler PasswordChanged {
			add {
				if (events[PasswordChangedEvent] == null)
					Events.AddHandler (this, "PasswordChanged", password_changed);
				events.AddHandler (PasswordChangedEvent, value);
			}
			remove {
				events.RemoveHandler (PasswordChangedEvent, value);
				if (events[PasswordChangedEvent] == null)
					Events.RemoveHandler (this, "PasswordChanged", password_changed);
			}
		}
		
		static void password_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((PasswordBox) Helper.GCHandleFromIntPtr (closure).Target).InvokePasswordChanged ();
		}
		

		void InvokePasswordChanged ()
		{
			RoutedEventHandler h = (RoutedEventHandler) events[PasswordChangedEvent];
			if (h != null)
				h (this, new RoutedEventArgs (native));
		}

		public FontSource FontSource {
			get { throw new NotImplementedException (); }
			set { throw new NotImplementedException (); }
		}

		public void SelectAll ()
		{
			NativeMethods.text_box_select_all (native);
		}
	}
}
