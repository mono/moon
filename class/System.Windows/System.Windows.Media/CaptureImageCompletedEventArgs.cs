//
// CaptureSource.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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

using System.ComponentModel;
using System.Windows.Media.Imaging;
using Mono;
using System.Collections;
using System.Collections.Generic;

namespace System.Windows.Media {

	public partial class CaptureImageCompletedEventArgs : AsyncCompletedEventArgs, INativeEventObjectWrapper, IRefContainer {

		DependencyObjectHandle handle;

		Mono.EventHandlerList INativeEventObjectWrapper.EventList {
			get { return null; }
		}

		internal CaptureImageCompletedEventArgs (IntPtr raw, Exception exc, bool dropref)
			: base (exc, false, null)
		{
			NativeHandle = raw;
			if (dropref)
				NativeMethods.event_object_unref (raw);
		}

		public CaptureImageCompletedEventArgs (WriteableBitmap image)
			: base (null, false, null)
		{
			// Note: When this is implemented a native peer will have to be created.
			Console.WriteLine ("NIEX: System.Windows.Media.CaptureImageCompletedEventArgs:.ctor");
			throw new NotImplementedException ();
		}

		WriteableBitmap result;
		public WriteableBitmap Result {
			get {
				if (result == null) {
					result = new WriteableBitmap (Source);
				}
				return result;
			}
		}

		BitmapImage Source {
			get; set;
		}

#region "INativeEventObjectWrapper interface"

		internal IntPtr NativeHandle {
			get { return handle.Handle; }
			set {
				if (handle != null) {
					throw new InvalidOperationException ("native handle is already set");
				}

				NativeDependencyObjectHelper.AddNativeMapping (value, this);
				handle = new DependencyObjectHandle (value, this);
			}
		}

		IntPtr INativeEventObjectWrapper.NativeHandle {
			get { return NativeHandle; }
			set { NativeHandle = value; }
		}

		void INativeEventObjectWrapper.MentorChanged (IntPtr mentor_ptr)
		{
		}

		Kind INativeEventObjectWrapper.GetKind ()
		{
			return Deployment.Current.Types.TypeToKind (GetType ());
		}
#endregion

		internal static Exception GetExceptionFromUnmanaged (IntPtr calldata)
		{
			try {
				NativeMethods.capture_image_completed_event_args_get_error (calldata);
				return null;
			}
			catch (Exception e) {
				return e;
			}
		}

		void IRefContainer.SetStrongRef (IntPtr id, object value)
		{
			if (id == (IntPtr) WeakRefs.CaptureImageCompletedEventArgs_Source)
				Source = (BitmapImage) value;
			else
				throw new Exception (string.Format ("CaptureImageCompletedEventArgs.SetStrongRef does not support id: {0}", id));
		}
	}
}