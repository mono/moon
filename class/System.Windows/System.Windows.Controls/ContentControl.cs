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
using System.Windows.Markup;

namespace System.Windows.Controls {
	public partial class ContentControl : Control {
		static readonly UnmanagedEventHandler content_changed = Events.SafeDispatcher (content_changed_callback);
		internal static readonly ControlTemplate FallbackTemplate = CreateFallbackTemplate ();

		static void content_changed_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			ContentControl cc = (ContentControl) NativeDependencyObjectHelper.FromIntPtr (closure);
			var oldContent = Value.ToObject (typeof (object), NativeMethods.content_control_changed_event_args_get_old_content (calldata));
			var newContent = Value.ToObject (typeof (object), NativeMethods.content_control_changed_event_args_get_new_content (calldata));

			cc.OnContentChanged (oldContent, newContent);
			cc.RaiseUIAContentChanged (oldContent, newContent);
		}

		// Needed in case OnContentControlChanged is overwritten in a subclass
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
					_fallbackRoot = (UIElement) FallbackTemplate.GetVisualTree (this);
				return _fallbackRoot;
			}
		}
		
		private new void Initialize ()
		{
			Events.AddHandler (this, EventIds.ContentControl_ContentControlChangedEvent, content_changed);
		}
		
		internal override UIElement GetDefaultTemplate ()
		{
			return FallbackRoot;
		}
		
		internal static ControlTemplate CreateFallbackTemplate ()
		{
			// Note - we're not specifying a TargetType as we don't need it.
			return (ControlTemplate) XamlReader.Load (@"
<ControlTemplate xmlns=""http://schemas.microsoft.com/client/2007"">
	<Grid>
	    <TextBlock Text=""{Binding}"" />
	</Grid>
</ControlTemplate>");
		}

		protected virtual void OnContentChanged (object oldContent, object newContent)
		{
			// no-op
		}
	}
}
