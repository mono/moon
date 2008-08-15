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
using System.Runtime.InteropServices;

namespace Mono {

	public delegate IntPtr CreateCustomXamlElementCallback (string xmlns, string name);
	public delegate void SetCustomXamlAttributeCallback (IntPtr target, string name, string value);
	public delegate void XamlHookupEventCallback (IntPtr target, string name, string value);
	public delegate void UnmanagedEventHandler (IntPtr sender, IntPtr calldata, IntPtr closure);

	public delegate void PlainEvent (IntPtr target);

	public delegate void HttpHeaderHandler (string name, string value);
	public delegate void AsyncResponseAvailableHandler (IntPtr response, IntPtr context);
	public delegate void NativePropertyChangedHandler (IntPtr dependency_property, IntPtr dependency_object, IntPtr old_value, IntPtr new_value);

	public static partial class NativeMethods {

	
		[DllImport("moon")]
		public extern static void runtime_init (int flags);
		
		[DllImport("moon")]
		public extern static void runtime_shutdown ();

		[DllImport("moon")]
		public extern static void surface_register_events (
			IntPtr surface,
			PlainEvent surface_resized);
		
		[DllImport("moon")]
		public extern static bool type_get_value_type (Kind type);

		[DllImport("moon")]
		public extern static bool type_create_instance_from_kind (Kind type);

		[DllImport("moon")]
		public extern static bool type_is_dependency_object (Kind type);

		[DllImport("moon")]
		public extern static IntPtr dependency_object_set_value (IntPtr obj, IntPtr property, IntPtr zero);
		
		[DllImport("moon")]
		public extern static IntPtr xaml_create_from_str (IntPtr native_loader, string xaml, bool create_namescope,
								  out Kind kind);

		[DllImport("moon")]
		public extern static IntPtr xaml_hydrate_from_str(IntPtr native_loader, string xaml, string assembly_name, string assembly_path, IntPtr obj, bool create_namescope, out Kind kind);
		
		[DllImport ("moon")]
		public extern static void xaml_set_property_from_str (IntPtr obj, IntPtr prop, string value);

		[DllImport("moon")]
		public extern static IntPtr xaml_create_from_file (IntPtr native_loader, string filename, bool create_namescope,
								  out Kind kind);
		[DllImport("moon")]
		public extern static void value_free_value (ref Value val);
		
		[DllImport("moon")]
		public extern static void visual_set_surface (IntPtr visual, IntPtr surface);

#region Transforms
		[DllImport("moon")]
		public extern static void general_transform_transform_point (IntPtr handle, ref UnmanagedPoint p, ref UnmanagedPoint r);
#endregion
		
#region UIElements
		[DllImport("moon")]
		public extern static void uielement_transform_point (IntPtr item, ref double x, ref double y);

		[DllImport("moon")]
		public extern static IntPtr uielement_get_parent (IntPtr item);

		[DllImport("moon")]
		public extern static bool uielement_capture_mouse (IntPtr item);

		[DllImport("moon")]
		public extern static void uielement_release_mouse_capture (IntPtr item);

		[DllImport ("moon")]
		public extern static UnmanagedSize uielement_get_desired_size (IntPtr item);

		[DllImport ("moon")]
		public extern static IntPtr uielement_get_transform_to_uielement (IntPtr item, IntPtr to_item);
#endregion

#region Grid
		[DllImport("moon")]
		public extern static double row_definition_get_actual_height (IntPtr handle);
		[DllImport("moon")]
		public extern static double column_definition_get_actual_width (IntPtr handle);
#endregion

#region Animations
		[DllImport("moon")]
		public extern static void key_spline_set_control_point_1 (IntPtr handle, double x, double y);

		[DllImport("moon")]
		public extern static void key_spline_set_control_point_2 (IntPtr handle, double x, double y);

		[DllImport("moon")]
		public extern static void key_spline_get_control_point_1 (IntPtr handle, out double x, out double y);

		[DllImport("moon")]
		public extern static void key_spline_get_control_point_2 (IntPtr handle, out double x, out double y);

		[DllImport("moon")]
		public extern static void timeline_set_manual_target (IntPtr timeline_handle, IntPtr target_handle);

#endregion
		
#region Collections
		[DllImport("moon")]
		public extern static IntPtr collection_get_iterator (IntPtr collection);
		
		[DllImport("moon")]
		public extern static int collection_iterator_next (IntPtr iterator);

		[DllImport("moon")]
		public extern static bool collection_iterator_reset (IntPtr iterator);

		[DllImport("moon")]
		public extern static IntPtr collection_iterator_get_current (IntPtr iterator, out int error);
		
		[DllImport("moon")]
		public extern static void collection_iterator_destroy (IntPtr iterator);

		[DllImport("moon")]
		public extern static IntPtr media_attribute_collection_get_item_by_name (IntPtr collection, string name);

		[DllImport("moon")]
		public extern static IntPtr stroke_collection_hit_test (IntPtr native, IntPtr stylusPointCollection);

		[DllImport("moon")]
		public extern static void stroke_collection_get_bounds (IntPtr native, ref UnmanagedRect urect);

		[DllImport("moon")]
		public extern static bool stroke_hit_test (IntPtr native, IntPtr stylusPointCollection);

		[DllImport("moon")]
		public extern static void stroke_get_bounds (IntPtr native, ref UnmanagedRect urect);
#endregion
		
#region MediaElement
		[DllImport("moon")]
		public extern static IntPtr media_element_stop (IntPtr native);
		
		[DllImport("moon")]
		public extern static IntPtr media_element_play (IntPtr native);
		
		[DllImport("moon")]
		public extern static IntPtr media_element_pause (IntPtr native);
		
		[DllImport("moon")]
		public extern static IntPtr media_element_set_source (IntPtr native, IntPtr downloader, string PartName);		
#endregion
		
#region Constructors
		[DllImport("moon")]
		public extern static IntPtr downloader_new ();
#endregion

#region Storyboard
		[DllImport("moon")]
		public extern static IntPtr storyboard_begin (IntPtr native);
		
		[DllImport("moon")]
		public extern static IntPtr storyboard_pause (IntPtr native);
		
		[DllImport("moon")]
		public extern static IntPtr storyboard_resume (IntPtr native);
		
		[DllImport("moon")]
		public extern static IntPtr storyboard_seek (IntPtr native, long timespan);
		
		[DllImport("moon")]
		public extern static IntPtr storyboard_stop (IntPtr native);
#endregion

#region Downloader
		[DllImport("moon")]
		public extern static void downloader_abort (IntPtr handle);
		
		[DllImport("moon")]
		public extern static IntPtr downloader_get_response_text (IntPtr handle, string partname, out long size);

		[DllImport("moon")]
		public extern static void downloader_open (IntPtr handle, string verb, string uri);

		[DllImport("moon")]
		public extern static void downloader_send (IntPtr handle);

		public delegate void UpdateFunction (int kind, IntPtr data, IntPtr extra);
		
		[DllImport("moon")]
		public extern static void downloader_want_events (IntPtr handle, UpdateFunction func, IntPtr closure);

		[DllImport("moon")]
		public extern static IntPtr downloader_create_webrequest (IntPtr downloader, string method, string uri);

#endregion

#region DownloaderRequest
		[DllImport("moon")]
		public extern static void downloader_request_abort (IntPtr downloader_request);

		[DllImport("moon")]
		public extern static void downloader_request_get_response (IntPtr downloader_request, IntPtr started, IntPtr available, IntPtr finished, IntPtr context);

		[DllImport("moon")]
		public extern static bool downloader_request_is_aborted (IntPtr downloader_request);

		[DllImport("moon")]
		public extern static void downloader_request_set_http_header (IntPtr doanloader_request, string name, string value);

		[DllImport("moon")]
		public extern static void downloader_request_set_body (IntPtr downloader_request, byte []body, int size);
#endregion

		[DllImport ("moon")]
		public extern static IntPtr moon_window_gtk_new (bool fullscreen, int w, int h);

		[DllImport ("moon")]
		public extern static IntPtr moon_window_gtk_get_widget (IntPtr window);

		[DllImport ("moon")]
		public extern static void surface_attach (IntPtr surface, IntPtr toplevel);

		[DllImport ("moon")]
		public extern static void surface_resize (IntPtr surface, int width, int height);
		
		[DllImport ("moon")]
		public extern static void surface_paint (IntPtr surface, IntPtr ctx, int x, int y, int width, int height);
		
		[DllImport ("moon")]
		public extern static IntPtr surface_new (IntPtr window, bool silverlight2);

		[DllImport ("moon")]
		public extern static void surface_set_trans (IntPtr surface, bool trans);
		
		[DllImport ("moon")]
		public extern static bool surface_get_trans (IntPtr surface);

		[DllImport ("moon")]
		public extern static IntPtr surface_get_time_manager (IntPtr surface);

		[DllImport ("moon")]
		public extern static IntPtr surface_create_downloader (IntPtr surface);
		
		[DllImport ("moon")]
		public extern static void image_set_source (IntPtr image, IntPtr downloader, string PartName);

		[DllImport ("moon")]
		public extern static void image_brush_set_source (IntPtr image_brush, IntPtr downloader, string PartName);

		[DllImport ("moon")]
		public extern static void text_block_set_font_source (IntPtr textblock, IntPtr downloader);

		[DllImport ("moon")]
		public extern static IntPtr control_initialize_from_xaml (IntPtr control, string xaml,
									  out Kind kind);

		[DllImport ("moon")]
		public extern static IntPtr control_initialize_from_xaml_callbacks (IntPtr control, string xaml,
									  out Kind kind, IntPtr native_loader);
		
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

#region RoutedEventArgs
		[DllImport("moon")]
		public extern static IntPtr routed_event_args_new ();
		
		[DllImport("moon")]
		public extern static IntPtr routed_event_args_get_source (IntPtr handle);

		[DllImport("moon")]
		public extern static void routed_event_args_set_source (IntPtr handle, IntPtr source_handle);
#endregion

#region MouseEventArgs
		// No longer exposed
		//[DllImport("moon")]
		//public extern static int mouse_event_args_get_state (IntPtr handle);

		[DllImport("moon")]
		public extern static IntPtr mouse_event_args_new ();

		[DllImport("moon")]
		public extern static void mouse_event_args_get_position (IntPtr handle, IntPtr uielement_handle, out double x, out double y);

		[DllImport("moon")]
		public extern static bool mouse_event_args_get_handled (IntPtr handle);

		[DllImport("moon")]
		public extern static void mouse_event_args_set_handled (IntPtr handle, bool handled);
#endregion

#region KeyEventArgs
		[DllImport("moon")]
		public extern static bool keyboard_event_args_get_handled (IntPtr handle);

		[DllImport("moon")]
		public extern static void keyboard_event_args_set_handled (IntPtr handle, bool handled);
#endregion

#region SizeChangedEventArgs
		[DllImport("moon")]
		public extern static IntPtr size_changed_event_args_new ();

		[DllImport("moon")]
		public extern static void size_changed_event_args_get_new_size (IntPtr handle, ref UnmanagedSize size);

		[DllImport("moon")]
		public extern static void size_changed_event_args_get_prev_size (IntPtr handle, ref UnmanagedSize size);
#endregion

#region Keyboard
		[DllImport("moon")]
		public extern static int keyboard_get_modifiers ();
#endregion

#region plugin
		[DllImport("moonplugin")]
		public extern static IntPtr plugin_instance_get_init_params (IntPtr plugin_handle);

		[DllImport("moonplugin")]
		public extern static IntPtr plugin_instance_get_source (IntPtr plugin_handle);

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
		
		
#endregion

#region xap
		[DllImport("moon")]
		public extern static string xap_unpack (string xapfile);
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
			}
		}
	}
}
