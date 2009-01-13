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

	internal delegate Size MeasureOverrideCallback (Size availableSize);
	internal delegate Size ArrangeOverrideCallback (Size finalSize);

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
	internal delegate void NativePropertyChangedHandler (IntPtr dependency_property, IntPtr dependency_object, IntPtr old_value, IntPtr new_value);

	internal static partial class NativeMethods {

	
		[DllImport("moon")]
		public extern static void runtime_init (int flags);

		[DllImport("moon")]
		public extern static void runtime_init_browser ();

		[DllImport("moon")]
		public extern static void runtime_init_desktop ();
		
		[DllImport("moon")]
		public extern static void runtime_shutdown ();

		[DllImport("moon")]
		public extern static bool type_get_value_type (Kind type);

		[DllImport("moon")]
		public extern static bool type_create_instance_from_kind (Kind type);

		[DllImport("moon")]
		public extern static bool type_is_dependency_object (Kind type);

		[DllImport ("moon")]
		public extern static void xaml_set_property_from_str (IntPtr obj, IntPtr prop, string value);

		[DllImport("moon")]
		public extern static IntPtr xaml_create_from_file (IntPtr native_loader, string filename, bool create_namescope,
								  out Kind kind);
		[DllImport("moon")]
		public extern static void value_free_value (ref Value val);
		
#region Transforms
		[DllImport("moon")]
		public extern static void general_transform_transform_point (IntPtr handle, ref Point p, ref Point r);
#endregion
		
#region Collections
		[DllImport("moon")]
		public extern static void stroke_collection_get_bounds (IntPtr native, ref Rect urect);

		[DllImport("moon")]
		public extern static void stroke_get_bounds (IntPtr native, ref Rect urect);
#endregion
		
#region Constructors
		[DllImport("moon")]
		public extern static IntPtr downloader_new ();
#endregion

#region Downloader
		[DllImport("moon")]
		public extern static void downloader_abort (IntPtr handle);
		
		[DllImport("moon")]
		public extern static IntPtr downloader_get_response_text (IntPtr handle, string partname, out long size);

//		[DllImport("moon")]
//		public extern static void downloader_open (IntPtr handle, string verb, string uri);

		[DllImport("moon")]
		public extern static void downloader_send (IntPtr handle);

		public delegate void UpdateFunction (int kind, IntPtr data, IntPtr extra);
		
		[DllImport("moon")]
		public extern static void downloader_want_events (IntPtr handle, UpdateFunction func, IntPtr closure);

		[DllImport("moon")]
		public extern static IntPtr downloader_create_webrequest (IntPtr downloader, string method, string uri);

		public delegate void DownloadedHandler (string path);
		[DllImport ("moon")]
		public extern static void deep_zoom_image_tile_source_download_urisource (IntPtr instance, string uri, DownloadedHandler callback);

#endregion

#region DownloaderRequest
		public delegate uint DownloaderResponseStartedDelegate (IntPtr native, IntPtr context);
		public delegate uint DownloaderResponseAvailableDelegate (IntPtr native, IntPtr context, IntPtr data, uint length);
		public delegate uint DownloaderResponseFinishedDelegate (IntPtr native, IntPtr context, bool success, IntPtr data);
		public delegate void HeaderVisitor (IntPtr name, IntPtr val);

		[DllImport("moon")]
		public extern static void downloader_request_abort (IntPtr downloader_request);

		[DllImport("moon")]
		public extern static bool downloader_request_get_response (IntPtr downloader_request, DownloaderResponseStartedDelegate started, DownloaderResponseAvailableDelegate available, DownloaderResponseFinishedDelegate finished, IntPtr context);

		[DllImport("moon")]
		public extern static bool downloader_request_is_aborted (IntPtr downloader_request);

		[DllImport("moon")]
		public extern static void downloader_request_set_http_header (IntPtr doanloader_request, string name, string value);

		[DllImport("moon")]
		public extern static void downloader_request_set_body (IntPtr downloader_request, byte []body, int size);
		
		[DllImport("moon")]
		public extern static void downloader_response_set_header_visitor (IntPtr downloader_response, HeaderVisitor visitor);
#endregion

		[DllImport ("moon")]
		public extern static void xaml_loader_set_callbacks (IntPtr native_object, Xaml.XamlLoaderCallbacks callbacks);		
		
		[DllImport ("moon")]
		public extern static void xaml_loader_add_missing (IntPtr native_object, string file);
		
		[DllImport ("moon")]
		public extern static IntPtr xaml_loader_new (string filename, string str, IntPtr surface);
		
		[DllImport ("moon")]
		public extern static void xaml_loader_free (IntPtr loader);
#if DEBUG
		[DllImport ("moon")]
		public extern static void print_stack_trace ();
#endif

		[DllImport ("moon")]
		public extern static IntPtr value_from_str_with_typename (string type_name, string prop_name, string str);

#region Time manager
		public delegate bool GSourceFunc (IntPtr data);
		
		[DllImport("moon")]
		public extern static uint time_manager_add_timeout (IntPtr manager, int interval, GSourceFunc callback, IntPtr data);
		[DllImport("moon")]
		public extern static void time_manager_remove_timeout (IntPtr manager, uint source_id);

		public delegate void TickCallHandler (IntPtr handle);

		[DllImport("moon")]
		public extern static uint time_manager_add_tick_call (IntPtr manager, TickCallHandler callback, IntPtr data);
#endregion

#region SizeChangedEventArgs
		[DllImport("moon")]
		public extern static void size_changed_event_args_get_new_size (IntPtr handle, ref Size size);

		[DllImport("moon")]
		public extern static void size_changed_event_args_get_prev_size (IntPtr handle, ref Size size);
#endregion

#region plugin
		[DllImport("moonplugin")]
		public extern static IntPtr plugin_instance_get_init_params (IntPtr plugin_handle);

		[DllImport ("moonplugin", EntryPoint="plugin_instance_get_id")]
		private extern static IntPtr plugin_instance_get_id_ (IntPtr plugin_handle);
		public static string plugin_instance_get_id (IntPtr instance)
		{
			IntPtr result;
			result = plugin_instance_get_id_ (instance);
			return (result == IntPtr.Zero) ? null : Marshal.PtrToStringAnsi (result);
		}

		[DllImport("moonplugin")]
		public extern static IntPtr plugin_instance_get_host (IntPtr plugin_handle);
		
		[DllImport("moonplugin")]
		public extern static IntPtr plugin_instance_get_source (IntPtr plugin_handle);
		
		[DllImport("moonplugin")]
		public extern static IntPtr plugin_instance_get_source_location (IntPtr plugin_handle);

		[DllImport("moonplugin")]
		public extern static int plugin_instance_get_actual_height (IntPtr plugin_handle);

		[DllImport("moonplugin")]
		public extern static int plugin_instance_get_actual_width (IntPtr plugin_handle);
		
		[DllImport("moonplugin")]
		public extern static IntPtr plugin_instance_get_surface (IntPtr plugin_handle);
		
		[DllImport("moonplugin")]
		public extern static void plugin_instance_report_exception (IntPtr plugin_handle, string msg, string details, string[] stack_trace, int num_frames);

		[DllImport("moonplugin")]
		public extern static IntPtr plugin_instance_load_url (IntPtr plugin_handle, string url, ref int length);

		[DllImport("moonplugin")]
		public extern static IntPtr plugin_instance_evaluate  (IntPtr plugin_handle, string code);
		
		[DllImport("moonplugin")]
		public extern static IntPtr browser_http_request_new (IntPtr plugin_handle, string method, string uri);

		[DllImport("moonplugin")]
		public extern static void browser_http_request_destroy (IntPtr handle);

		[DllImport("moonplugin")]
		public extern static void browser_http_request_abort (IntPtr handle);

		[DllImport("moonplugin")]
		public extern static void browser_http_request_set_header (IntPtr handle, string name, string value);

		[DllImport("moonplugin")]
		public extern static void browser_http_request_set_body (IntPtr handle, byte [] body, int size);

		[DllImport("moonplugin")]
		public extern static IntPtr browser_http_request_get_response (IntPtr handle);

		[DllImport("moonplugin")]
		public extern static bool browser_http_request_get_async_response (IntPtr handle, AsyncResponseAvailableHandler handler, IntPtr context);


		[DllImport("moonplugin")]
		public extern static IntPtr browser_http_response_read (IntPtr handler, out int size);

		[DllImport("moonplugin")]
		public extern static void browser_http_response_visit_headers (IntPtr handle, HttpHeaderHandler handler);

		[DllImport("moonplugin")]
		public extern static string browser_http_response_get_status (IntPtr handle, out int code);

		[DllImport("moonplugin")]
		public extern static void browser_http_response_destroy (IntPtr handle);

		public delegate void DomEventCallback (IntPtr context, string name, int client_x, int client_y, int offset_x, int offset_y, bool alt_key, bool ctrl_key, bool shift_key, int /* System.Windows.Browser.MouseButtons */ mouse_button);

		[DllImport ("moonplugin")]
		public static extern void html_object_get_property (IntPtr plugin, IntPtr obj, string name, out Mono.Value result);

		[DllImport ("moonplugin")]
		public static extern void html_object_set_property (IntPtr plugin, IntPtr obj, string name, ref Mono.Value value);

		[DllImport ("moonplugin")]
		public static extern void html_object_invoke (IntPtr plugin, IntPtr obj, string name, Mono.Value [] args, int arg_count, out Mono.Value result);

		[DllImport ("moonplugin")]
		public static extern IntPtr html_object_attach_event (IntPtr plugin, IntPtr obj, string name, DomEventCallback cb, IntPtr context);

		[DllImport ("moonplugin")]
		public static extern IntPtr html_object_detach_event (IntPtr plugin, string name, IntPtr wrapper);
		
#endregion

		private static Exception CreateManagedException (MoonError err)
		{
			string msg = err.Message;
			err.Dispose ();
			
			switch (err.Number) {
			case 1:
			default:
				throw new Exception (msg);
			case 2:
				throw new ArgumentException (msg);
			case 3:
				throw new ArgumentNullException (msg);
			case 4:
				throw new ArgumentOutOfRangeException (msg);
			case 5:
				throw new InvalidOperationException (msg);
			case 6: {
				throw new XamlParseException (msg);
			}
			case 7:
				throw new UnauthorizedAccessException (msg);
			}
		}
	}
}
