//
// NativeMethods.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2007 Novell, Inc.
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
using System.Globalization;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Markup;

namespace Mono {

	internal delegate IntPtr DownloaderCreateStateFunc (IntPtr dl);
	internal delegate void   DownloaderDestroyStateFunc (IntPtr state);
	internal delegate void   DownloaderOpenFunc (IntPtr state, string verb, string uri, bool custom_header_support, bool disable_cache);
	internal delegate void   DownloaderSendFunc (IntPtr state);
	internal delegate void   DownloaderAbortFunc (IntPtr state);
	internal delegate void   DownloaderHeaderFunc (IntPtr state, string header, string value);
	internal delegate void   DownloaderBodyFunc (IntPtr state, IntPtr body, int length);
	internal delegate IntPtr DownloaderCreateWebRequestFunc (string method, string uri, IntPtr context);
	internal delegate void   DownloaderSetResponseHeaderCallbackFunc (IntPtr native, DownloaderResponseHeaderCallback callback, IntPtr context);
	internal delegate IntPtr DownloaderGetResponseFunc (IntPtr native);

	internal delegate void DownloaderResponseHeaderCallback (IntPtr context, string header, string value);

	internal delegate Size MeasureOverrideCallback (Size availableSize);
	internal delegate Size ArrangeOverrideCallback (Size finalSize);
	internal delegate void LoadedCallback (IntPtr fwe_ptr);

	internal delegate void ApplyDefaultStyleCallback (IntPtr fwe_ptr, IntPtr type_info_ptr);
	internal delegate IntPtr GetDefaultTemplateCallback (IntPtr fwe_ptr);
	internal delegate void ApplyStyleCallback (IntPtr fwe_ptr, IntPtr style_ptr);
	internal delegate void ConvertKeyframeValueCallback (Mono.Kind kind, IntPtr property, IntPtr original, out Value converted);
	internal delegate ManagedStreamCallbacks GetResourceCallback (string resourceBase, string name);

	// Used in databinding to interact with managed objects
	internal delegate Value GetValueCallback ();
	internal delegate void SetValueCallback (IntPtr value);

	internal delegate IntPtr CreateCustomXamlElementCallback (string xmlns, string name);
	internal delegate void SetCustomXamlAttributeCallback (IntPtr target, string name, string value);
	internal delegate void XamlHookupEventCallback (IntPtr target, string name, string value);
	internal delegate void UnmanagedEventHandler (IntPtr sender, IntPtr calldata, IntPtr closure);

	internal delegate void PlainEvent (IntPtr target);

	internal delegate void HttpHeaderHandler (string name, string value);
	internal delegate void AsyncResponseAvailableHandler (IntPtr response, IntPtr context);
	internal delegate void UnmanagedPropertyChangeHandler (IntPtr dependency_object, IntPtr propertyChangedArgs, ref MoonError error, IntPtr closure);

	internal delegate void TickCallHandler (IntPtr handle);


	internal delegate void InvokeDelegate (IntPtr obj_handle, IntPtr method_handle,
								[MarshalAs (UnmanagedType.LPStr)] string name,
								[MarshalAs (UnmanagedType.LPArray, SizeParamIndex = 4)]
								IntPtr[] args,
								int arg_count,
								ref Value return_value);

	internal delegate void SetPropertyDelegate (IntPtr obj_handle,
								[MarshalAs (UnmanagedType.LPStr)] string name,
								[MarshalAs (UnmanagedType.LPArray, SizeParamIndex = 3)]
								IntPtr[] args,
								int arg_count,
								ref Value return_value);
	internal delegate void GetPropertyDelegate (IntPtr obj_handle,
								[MarshalAs (UnmanagedType.LPStr)] string name,
								[MarshalAs (UnmanagedType.LPArray, SizeParamIndex = 3)]
								IntPtr[] args,
								int arg_count,
								ref Value return_value);
	internal delegate void EventHandlerDelegate (IntPtr obj_handle, IntPtr event_handle, IntPtr scriptable_obj, IntPtr closure);
	
	internal delegate uint DownloaderResponseStartedDelegate (IntPtr native, IntPtr context);
	internal delegate uint DownloaderResponseAvailableDelegate (IntPtr native, IntPtr context, IntPtr data, uint length);
	internal delegate uint DownloaderResponseFinishedDelegate (IntPtr native, IntPtr context, [MarshalAs (UnmanagedType.U1)] bool success, IntPtr data);
	internal delegate void HeaderVisitor (IntPtr context, IntPtr name, IntPtr val);

	internal delegate void DomEventCallback (IntPtr context, string name, int client_x, int client_y, int offset_x, int offset_y, 
		[MarshalAs (UnmanagedType.Bool)] bool alt_key,	// glib gboolean is a gint (i.e. 4 bytes just like the FX bool)
		[MarshalAs (UnmanagedType.Bool)] bool ctrl_key,
		[MarshalAs (UnmanagedType.Bool)] bool shift_key, 
		int /* System.Windows.Browser.MouseButtons */ mouse_button,
		int key_code,
		int char_code,
		IntPtr domEvent);

	internal delegate bool ImageUriFunc (int level, int posx, int posy, IntPtr uri, IntPtr ignore);
	
	internal static partial class NativeMethods {
		/*
		 * 
		 * Don't add any P/Invokes here.
		 * 
		 * Add annotations (@GeneratePInvoke) to your C/C++ methods to generate the P/Invokes.
		 * If the generator gets parameters wrong, you can add a @MarshalAs=<whatever> to override.
		 * 
		 */
		internal static Exception CreateManagedException (MoonError err)
		{
			string msg = err.Message;
			Exception ex = null;
			
			if (err.GCHandlePtr != IntPtr.Zero) {
				// We need to get this before calling Dispose.
				ex = err.GCHandle.Target as Exception;
			}

			err.Dispose ();
			
			switch (err.Number) {
			case 1:
			default:
				return new Exception (msg);
			case 2:
				return new ArgumentException (msg);
			case 3:
				return new ArgumentNullException (msg);
			case 4:
				return new ArgumentOutOfRangeException (msg);
			case 5:
				return new InvalidOperationException (msg);
			case 6:
				return new XamlParseException (err.LineNumber, err.CharPosition, msg);
			case 7:
				return new UnauthorizedAccessException (msg);
			case 8:
				return new ExecutionEngineException (msg);
			case 9:
				if (ex != null)
					return ex;
				return new Exception (msg);
			}
		}
	}
}
