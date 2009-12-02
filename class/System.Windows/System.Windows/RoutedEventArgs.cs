//
// RoutedEventArgs.cs
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

namespace System.Windows {

	public class RoutedEventArgs : EventArgs, INativeEventObjectWrapper {

		IntPtr _native;
		DependencyObject source;
		bool source_set;
		bool free_mapping;

		internal IntPtr NativeHandle {
			get { return _native; }
			set {
				if (_native != IntPtr.Zero) {
					throw new InvalidOperationException ("RoutedEventArgs.native is already set");
				}

				_native = value;

				free_mapping = NativeDependencyObjectHelper.AddNativeMapping (value, this);
			}
		}
		
		IntPtr INativeEventObjectWrapper.NativeHandle {
			get { return NativeHandle; }
			set { NativeHandle = value; }
		}
		
		Kind INativeEventObjectWrapper.GetKind ()
		{
			return NativeMethods.event_object_get_object_type (_native);
		}
		
		internal RoutedEventArgs (IntPtr raw, bool dropref)
		{
			NativeHandle = raw;
			if (dropref)
				NativeMethods.event_object_unref (raw);
		}
		
		internal void Free ()
		{
			if (free_mapping) {
				free_mapping = false;
				NativeDependencyObjectHelper.FreeNativeMapping (this);
			}
		}

		~RoutedEventArgs ()
		{
			Free ();
		}

		public RoutedEventArgs () : this (NativeMethods.routed_event_args_new (), true)
		{
		}

		public object OriginalSource {
			get {
				if (source_set)
					return source;

				return NativeDependencyObjectHelper.FromIntPtr (NativeMethods.routed_event_args_get_source (_native));
			}

			internal set {
				if (value == null) {
					NativeMethods.routed_event_args_set_source (_native, IntPtr.Zero);
					source = null;
					source_set = false;
					return;
				}

				DependencyObject v = value as DependencyObject;
				if (v == null)
					throw new ArgumentException ();

				source_set = true;
				source = v;

				NativeMethods.routed_event_args_set_source (_native, v.native);
			}
		}

		internal bool EventHandled {
			get { return NativeMethods.routed_event_args_get_handled (NativeHandle); }
			set { NativeMethods.routed_event_args_set_handled (NativeHandle, value); }
		}
	}
}
