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
using System.Windows.Automation.Peers;

namespace System.Windows.Controls.Primitives {

	public sealed partial class Popup : FrameworkElement
	{
		static UnmanagedEventHandler on_opened = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Popup) NativeDependencyObjectHelper.FromIntPtr (closure)).OnOpened ());

		void Initialize ()
		{
			Events.AddOnEventHandler (this, EventIds.Popup_OpenedEvent, on_opened);
		}

		internal event EventHandler ClickedOutside;

		internal UIElement RealChild {
			get {
				if (_clickCatcher != null) {
					Canvas root = Child as Canvas;
					return root.Children[1];
				}

				return Child;
			}
		}

		
		private void OnOpened ()
		{
			UpdateCatcher ();
			
			NativeMethods.event_object_do_emit_current_context (native, EventIds.Popup_OpenedEvent, IntPtr.Zero);
		}

		Canvas _clickCatcher;
		
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
		}

		void UpdateCatcher ()
		{
			if (_clickCatcher == null)
				return;
			
			try {
				GeneralTransform xform = Application.Current.RootVisual.TransformToVisual (this);
				
				_clickCatcher.RenderTransform = (Transform)xform;
			} catch (ArgumentException) {
				// Drop errors looking up the transform
			}

			Canvas.SetTop (_clickCatcher, -VerticalOffset);
			Canvas.SetLeft (_clickCatcher, -HorizontalOffset);
			_clickCatcher.Height = Application.Current.Host.Content.ActualHeight;
			_clickCatcher.Width = Application.Current.Host.Content.ActualWidth;		
		}

		protected override AutomationPeer OnCreateAutomationPeer ()
		{
			return new PopupAutomationPeer (this);
		}
	}
}

