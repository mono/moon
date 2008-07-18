//
// NativeMethods.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
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

	public static class NativeMethods {

	
		[DllImport("moon")]
		public extern static void runtime_init (int flags);
		
		[DllImport("moon")]
		public extern static void runtime_shutdown ();

		[DllImport("moon")]
		public extern static void surface_register_events (
			IntPtr surface,
			PlainEvent surface_resized);
	
#region Base
		[DllImport("moon")]
		public extern static void base_ref (IntPtr ptr);

		[DllImport("moon")]
		public extern static void base_unref (IntPtr ptr);
#endregion

		[DllImport("moon")]
		public extern static bool type_get_value_type (Kind type);

                [DllImport("moon")]
		public extern static bool type_create_instance_from_kind (Kind type);

		[DllImport("moon")]
		public extern static bool type_is_dependency_object (Kind type);
		
		[DllImport("moon")]
		public extern static IntPtr dependency_property_lookup (Kind type, string name);

		[DllImport("moon")]
		public extern static Kind dependency_property_get_value_type (IntPtr obj);
		
		[DllImport("moon")]
		public extern static bool dependency_property_is_nullable (IntPtr obj);
		
		[DllImport("moon", EntryPoint="dependency_property_get_name")]
		public extern static IntPtr _dependency_property_get_name (IntPtr obj);

		public static string dependency_property_get_name (IntPtr obj)
		{
			IntPtr p = _dependency_property_get_name (obj);
			if (p == IntPtr.Zero)
				return null;
			
			return Marshal.PtrToStringAnsi (p);
		}
		
		[DllImport("moon")]
		public extern static IntPtr dependency_object_get_value (IntPtr obj, IntPtr property);
		
		[DllImport("moon")]
		public extern static IntPtr dependency_object_get_value_no_default (IntPtr obj, IntPtr property);
		
		[DllImport("moon")]
		public extern static IntPtr dependency_object_set_value (IntPtr obj, IntPtr property, ref Value val);

		[DllImport("moon")]
		public extern static IntPtr dependency_object_set_value (IntPtr obj, IntPtr property, IntPtr zero);

		[DllImport("moon")]
		public extern static IntPtr dependency_object_find_name (IntPtr obj, string name, out Kind kind);

		[DllImport("moon", EntryPoint="dependency_object_get_name")]
		public extern static IntPtr _dependency_object_get_name (IntPtr obj);

		public static string dependency_object_get_name (IntPtr obj)
		{
			IntPtr p = _dependency_object_get_name (obj);
			if (p == IntPtr.Zero)
				return null;
			
			return Marshal.PtrToStringAnsi (p);
		}
		
		[DllImport("moon")]
		public extern static Kind dependency_object_get_object_type (IntPtr obj);
		
		[DllImport("moon")]
		public extern static IntPtr xaml_create_from_str (IntPtr native_loader, string xaml, bool create_namescope,
								  out Kind kind);

		[DllImport("moon")]
		public extern static IntPtr xaml_hydrate_from_str(IntPtr native_loader, string xaml, IntPtr obj, bool create_namescope, out Kind kind);
		
		[DllImport ("moon")]
		public extern static void xaml_set_property_from_str (IntPtr obj, IntPtr prop, string value);

		[DllImport("moon")]
		public extern static IntPtr xaml_create_from_file (IntPtr native_loader, string filename, bool create_namescope,
								  out Kind kind);
		[DllImport("moon")]
		public extern static void value_free_value (ref Value val);
		
		[DllImport("moon")]
		public extern static void visual_set_surface (IntPtr visual, IntPtr surface);
		
#region UIElements
		[DllImport("moon")]
		public extern static void uielement_transform_point (IntPtr item, ref double x, ref double y);

		[DllImport("moon")]
		public extern static IntPtr uielement_get_parent (IntPtr item);

		[DllImport("moon")]
		public extern static bool uielement_capture_mouse (IntPtr item);

		[DllImport("moon")]
		public extern static void uielement_release_mouse_capture (IntPtr item);

#endregion


#region Panel
		[DllImport("moon")]
		public extern static IntPtr panel_new ();
#endregion

#region Grid
		[DllImport("moon")]
		public extern static IntPtr grid_new ();
		[DllImport("moon")]
		public extern static IntPtr column_definition_new ();
		[DllImport("moon")]
		public extern static IntPtr row_definition_new ();
		[DllImport("moon")]
		public extern static IntPtr column_definition_collection_new ();
		[DllImport("moon")]
		public extern static IntPtr row_definition_collection_new ();

		[DllImport("moon")]
		public extern static double row_definition_get_actual_height (IntPtr handle);
		[DllImport("moon")]
		public extern static double column_definition_get_actual_width (IntPtr handle);
#endregion

#region Controls
		[DllImport("moon")]
		public extern static IntPtr control_new ();

		[DllImport("moon")]
		public extern static IntPtr user_control_new ();

		[DllImport("moon")]
		public extern static IntPtr image_new ();

		[DllImport("moon")]
		public extern static IntPtr media_base_new ();

		[DllImport("moon")]
		public extern static IntPtr text_block_new ();

		[DllImport("moon")]
		public extern static IntPtr run_new ();
#endregion

#region Documents
		[DllImport ("moon")]
		public extern static IntPtr glyphs_new ();
#endregion
		
#region Collections
		[DllImport("moon")]
		public extern static void collection_add (IntPtr collection, IntPtr value);

		[DllImport("moon")]
		public extern static bool collection_remove (IntPtr collection, IntPtr value);

		[DllImport("moon")]
		public extern static bool collection_remove_at (IntPtr collection, int index);

		[DllImport("moon")]
		public extern static void collection_insert (IntPtr collection, int index, IntPtr value);

		[DllImport("moon")]
		public extern static void collection_clear (IntPtr collection);
		
		[DllImport("moon")]
		public extern static int collection_get_count (IntPtr collection);
		
		[DllImport("moon")]
		public extern static IntPtr collection_get_value_at (IntPtr collection, int index);
		
		[DllImport("moon")]
		public extern static IntPtr collection_set_value_at (IntPtr collection, int index, IntPtr value);

		[DllImport("moon")]
		public extern static int collection_get_index_of (IntPtr collection, IntPtr obj);
		
		[DllImport("moon")]
		public extern static IntPtr collection_get_iterator (IntPtr collection);

		[DllImport("moon")]
		public extern static Kind collection_get_element_type (IntPtr collection);
		
		[DllImport("moon")]
		public extern static int collection_iterator_move_next (IntPtr iterator);

		[DllImport("moon")]
		public extern static bool collection_iterator_reset (IntPtr iterator);

		[DllImport("moon")]
		public extern static IntPtr collection_iterator_get_current (IntPtr iterator, out int error);

		[DllImport("moon")]
		public extern static void collection_iterator_destroy (IntPtr iterator);
		
		[DllImport("moon")]
		public extern static IntPtr resource_dictionary_new ();
		
		[DllImport("moon")]
		public extern static IntPtr trigger_collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr gradient_stop_collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr media_attribute_collection_new ();

		[DllImport("moon")]
		public extern static IntPtr media_attribute_collection_get_item_by_name (IntPtr collection, string name);

		[DllImport("moon")]
		public extern static IntPtr path_figure_collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr path_segment_collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr visual_collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr geometry_collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr transform_collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr timeline_collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr inlines_new ();
		
		[DllImport("moon")]
		public extern static IntPtr color_key_frame_collection_new ();

		[DllImport("moon")]
		public extern static IntPtr double_key_frame_collection_new ();

		[DllImport("moon")]
		public extern static IntPtr point_key_frame_collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr timeline_marker_collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr stylus_point_collection_new ();
		
		[DllImport("moon")]
		public extern static IntPtr stroke_collection_new ();

		[DllImport("mono")]
		public extern static IntPtr stroke_collection_hit_test (IntPtr native, IntPtr stylusPointCollection);

		[DllImport("mono")]
		public extern static void stroke_collection_get_bounds (IntPtr native, ref UnmanagedRect urect);

		[DllImport("mono")]
		public extern static bool stroke_hit_test (IntPtr native, IntPtr stylusPointCollection);

		[DllImport("mono")]
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
		public extern static IntPtr ink_presenter_new ();
		
		[DllImport("moon")]
		public extern static IntPtr media_element_new ();
		
		[DllImport("moon")]
		public extern static IntPtr stroke_new ();
		
		[DllImport("moon")]
		public extern static IntPtr drawing_attributes_new ();
		
		[DllImport("moon")]
		public extern static IntPtr stylus_info_new ();
		
		[DllImport("moon")]
		public extern static IntPtr stylus_point_new ();
		
		[DllImport("moon")]
		public extern static IntPtr timeline_marker_new ();
		
		[DllImport("moon")]
		public extern static IntPtr line_break_new ();
		
		[DllImport("moon")]
		public extern static IntPtr spline_point_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr spline_color_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr linear_double_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr linear_point_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr linear_color_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr discrete_double_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr discrete_point_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr discrete_color_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr color_animation_using_key_frames_new ();
		
		[DllImport("moon")]
		public extern static IntPtr point_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr double_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr color_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr key_frame_new ();

		[DllImport("moon")]
		public extern static IntPtr spline_double_key_frame_new ();
		
		[DllImport("moon")]
		public extern static IntPtr key_spline_new ();
		
		[DllImport("moon")]
		public extern static IntPtr rectangle_new ();
		
		[DllImport("moon")]
		public extern static IntPtr framework_element_new ();
		
		[DllImport("moon")]
		public extern static IntPtr visual_new ();
		
		[DllImport("moon")]
		public extern static IntPtr path_segment_new ();
		
		[DllImport("moon")]
		public extern static IntPtr geometry_new ();
		
		[DllImport("moon")]
		public extern static IntPtr timeline_new ();
		
		[DllImport("moon")]
		public extern static IntPtr timeline_group_new ();
		
		[DllImport("moon")]
		public extern static IntPtr parallel_timeline_new ();
		
		[DllImport("moon")]
		public extern static IntPtr shape_new ();
		
		[DllImport("moon")]
		public extern static IntPtr uielement_new ();

		[DllImport("moon")]
		public extern static IntPtr solid_color_brush_new ();
		
		[DllImport("moon")]
		public extern static IntPtr radial_gradient_brush_new ();

		[DllImport("moon")]
		public extern static IntPtr brush_new ();
		
		[DllImport("moon")]
		public extern static IntPtr gradient_brush_new ();
		
		[DllImport("moon")]
		public extern static IntPtr gradient_stop_new ();
		
		[DllImport("moon")]
		public extern static IntPtr image_brush_new ();
		
		[DllImport("moon")]
		public extern static IntPtr linear_gradient_brush_new ();
		
		[DllImport("moon")]
		public extern static IntPtr media_attribute_new ();
		
		[DllImport("moon")]
		public extern static IntPtr skew_transform_new ();
		
		[DllImport("moon")]
		public extern static IntPtr tile_brush_new ();
		
		[DllImport("moon")]
		public extern static IntPtr transform_new ();
		
		[DllImport("moon")]
		public extern static IntPtr video_brush_new ();

		[DllImport("moon")]
		public extern static IntPtr visual_brush_new ();
		
		[DllImport("moon")]
		public extern static IntPtr canvas_new ();

		[DllImport("moon")]
		public extern static IntPtr geometry_group_new ();

		[DllImport("moon")]
		public extern static IntPtr ellipse_geometry_new ();

		[DllImport("moon")]
		public extern static IntPtr line_geometry_new ();

		[DllImport("moon")]
		public extern static IntPtr path_geometry_new ();

		[DllImport("moon")]
		public extern static IntPtr rectangle_geometry_new ();

		[DllImport("moon")]
		public extern static IntPtr path_figure_new ();

		[DllImport("moon")]
		public extern static IntPtr arc_segment_new ();

		[DllImport("moon")]
		public extern static IntPtr bezier_segment_new ();

		[DllImport("moon")]
		public extern static IntPtr line_segment_new ();

		[DllImport("moon")]
		public extern static IntPtr poly_bezier_segment_new ();

		[DllImport("moon")]
		public extern static IntPtr poly_line_segment_new ();

		[DllImport("moon")]
		public extern static IntPtr poly_quadratic_bezier_segment_new ();

		[DllImport("moon")]
		public extern static IntPtr quadratic_bezier_segment_new ();

		[DllImport("moon")]
		public extern static IntPtr ellipse_new ();

		[DllImport("moon")]
		public extern static IntPtr line_new ();

		[DllImport("moon")]
		public extern static IntPtr polygon_new ();

		[DllImport("moon")]
		public extern static IntPtr polyline_new ();

		[DllImport("moon")]
		public extern static IntPtr path_new ();

		[DllImport("moon")]
		public extern static IntPtr rotate_transform_new ();

		[DllImport("moon")]
		public extern static IntPtr translate_transform_new ();

		[DllImport("moon")]
		public extern static IntPtr scale_transform_new ();

		[DllImport("moon")]
		public extern static IntPtr matrix_transform_new ();

		[DllImport("moon")]
		public extern static IntPtr transform_group_new ();
		
		[DllImport("moon")]
		public extern static IntPtr downloader_new ();

		[DllImport("moon")]
		public extern static IntPtr storyboard_new ();

		[DllImport("moon")]
		public extern static IntPtr beginstoryboard_new ();

		[DllImport("moon")]
		public extern static IntPtr animation_new ();

		[DllImport("moon")]
		public extern static IntPtr coloranimation_new ();

		[DllImport("moon")]
		public extern static IntPtr trigger_action_collection_new ();

		[DllImport ("moon")]
		public extern static IntPtr deployment_new ();

		[DllImport ("moon")]
		public extern static IntPtr assembly_part_new ();

		[DllImport ("moon")]
		public extern static IntPtr assembly_part_collection_new ();
		
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
		
		[DllImport("moon")]
		public extern static IntPtr double_animation_new ();

		[DllImport("moon")]
		public extern static IntPtr double_animation_using_key_frames_new ();

		[DllImport("moon")]
		public extern static IntPtr color_animation_new ();

		[DllImport("moon")]
		public extern static IntPtr event_trigger_new ();

		[DllImport("moon")]
		public extern static IntPtr point_animation_new ();

		[DllImport("moon")]
		public extern static IntPtr begin_storyboard_new ();
#endregion
		
#region APIs that we do not have a Managed class implemented yet.

		[DllImport("moon")]
		public extern static IntPtr point_animation_using_key_frames_new ();

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
		public extern static void downloader_request_set_body (IntPtr downloader_request, IntPtr body, int size);
#endregion

		[DllImport ("moon")]
		public extern static void surface_attach (IntPtr surface, IntPtr toplevel);

		[DllImport ("moon")]
		public extern static void surface_resize (IntPtr surface, int width, int height);
		
		[DllImport ("moon")]
		public extern static void surface_paint (IntPtr surface, IntPtr ctx, int x, int y, int width, int height);
		
		[DllImport ("moon")]
		public extern static IntPtr surface_new (int w, int h);
		
		[DllImport ("moon")]
		public extern static IntPtr surface_get_widget (IntPtr surface);

		[DllImport ("moon")]
		public extern static void surface_set_trans (IntPtr surface, bool trans);
		
		[DllImport ("moon")]
		public extern static bool surface_get_trans (IntPtr surface);

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

#region HtmlTimer
		public delegate bool GSourceFunc  (IntPtr data);
		
		[DllImport("moonplugin")]
		public extern static uint plugin_html_timer_timeout_add (IntPtr plugin_handle, int interval, GSourceFunc callback, IntPtr data);
		
		[DllImport("moonplugin")]
		public extern static void plugin_html_timer_timeout_stop (IntPtr plugin_handle, uint source_id);

		//
		// The version to use outside the plugin
		//
		[DllImport("moon")]
		public extern static uint runtime_html_timer_timeout_add (int interval, GSourceFunc callback, IntPtr data);

		[DllImport("moon")]
		public extern static void runtime_html_timer_timeout_stop (uint source_id);

#endregion
		
#region EventObject
		[DllImport("moon")]
		public extern static void event_object_add_event_handler (IntPtr handle, string eventName, UnmanagedEventHandler handler, IntPtr closure);

		[DllImport("moon")]
		public extern static void event_object_remove_event_handler (IntPtr handle, string eventName, UnmanagedEventHandler handler, IntPtr closure);
#endregion

#region MouseEventArgs
		// No longer exposed
		//[DllImport("moon")]
		//public extern static int mouse_event_args_get_state (IntPtr handle);

		[DllImport("moon")]
		public extern static void mouse_event_args_get_position (IntPtr handle, IntPtr uielement_handle, out double x, out double y);

		[DllImport("moon")]
		public extern static IntPtr mouse_event_args_get_stylus_info (IntPtr handle);

		[DllImport("moon")]
		public extern static IntPtr mouse_event_args_get_stylus_points (IntPtr handle, IntPtr uielement_handle);
#endregion

#region plugin
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
	}
}
