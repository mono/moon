//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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

using System.Windows;
using System.Windows.Data;
using System.Windows.Media;
using Mono;

namespace System.Windows.Controls {
	public partial class ContentControl : Control {
		static UnmanagedEventHandler content_changed = Events.SafeDispatcher (content_changed_callback);
		
		internal class ContentChangedEventArgs : EventArgs {
			internal IntPtr native;
			
			internal ContentChangedEventArgs (IntPtr raw)
			{
				native = raw;
				NativeMethods.event_object_ref (native);
			}
			
			~ContentChangedEventArgs ()
			{
				if (native != IntPtr.Zero) {
					NativeMethods.event_object_unref (native);
					native = IntPtr.Zero;
				}
			}
			
			public object OldContent {
				get {
					IntPtr result = NativeMethods.content_changed_event_args_get_old_content (native);
					
					if (result == IntPtr.Zero)
						return null;
					
					return Value.ToObject (typeof (object), result);
				}
			}
			
			public object NewContent {
				get {
					IntPtr result = NativeMethods.content_changed_event_args_get_new_content (native);
					
					if (result == IntPtr.Zero)
						return null;
					
					return Value.ToObject (typeof (object), result);
				}
			}
		}
		
		static void content_changed_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			ContentControl cc = (ContentControl) NativeDependencyObjectHelper.FromIntPtr (closure);
			ContentChangedEventArgs args = new ContentChangedEventArgs (calldata);
			
			cc.OnContentChanged (args.OldContent, args.NewContent);
			cc.RaiseUIAContentChanged (args.OldContent, args.NewContent);
		}

		// Needed in case OnContentChanged is overwritten in a subclass
		internal event Action<object, object> UIAContentChanged;

		internal void RaiseUIAContentChanged (object oldContent, object newContent)
		{
			if (UIAContentChanged != null)
				UIAContentChanged (oldContent, newContent);
 		}
		
		internal bool ContentSetsParent {
			get { return (bool) Mono.NativeMethods.content_control_get_content_sets_parent (native); }
			set { Mono.NativeMethods.content_control_set_content_sets_parent (native, value); }
		}
		
		UIElement _fallbackRoot;

		UIElement FallbackRoot {
			get {
				if (_fallbackRoot == null)
					_fallbackRoot = CreateFallbackRoot ();
				return _fallbackRoot;
			}
		}
		
		void Initialize ()
		{
			Events.AddHandler (this, EventIds.ContentControl_ContentChangedEvent, content_changed);
		}
		
		internal override UIElement GetDefaultTemplate ()
		{
			return FallbackRoot;
		}
		
		internal static Grid CreateFallbackRoot ()
		{
			Grid grid = new Grid ();
			TextBlock block = new TextBlock ();
			block.SetBinding (TextBlock.TextProperty, new Binding ());
			grid.Children.Add (block);
			return grid;
		}

		protected virtual void OnContentChanged (object oldContent, object newContent)
		{
			// no-op
		}
	}
}
