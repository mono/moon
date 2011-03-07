# [SecurityCritical] needed to execute code inside 'System.Windows, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e'.
# 595 methods needs to be decorated.

# p/invoke declaration
+SC-M: Mono.Kind Mono.NativeMethods::collection_get_element_type(System.IntPtr)

# p/invoke declaration
+SC-M: Mono.Kind Mono.NativeMethods::dependency_property_get_property_type(System.IntPtr)

# p/invoke declaration
+SC-M: Mono.Kind Mono.NativeMethods::event_object_get_object_type(System.IntPtr)

# p/invoke declaration
+SC-M: Mono.Kind Mono.NativeMethods::types_register_type(System.IntPtr,System.String,System.String,System.IntPtr,Mono.Kind,System.Boolean,System.Boolean,Mono.Kind[],System.Int32)

# using 'Mono.Value*' as a parameter type
+SC-M: Mono.Value Mono.Xaml.XamlParser::HydrateFromString(System.String,Mono.Value*,System.Boolean,System.Boolean)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::accessibility_bridge_is_accessibility_enabled(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::application_install_(System.IntPtr,Mono.MoonError&,System.Boolean,System.Boolean)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::application_is_running_out_of_browser(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::check_and_download_update_completed_event_args_get_update_available(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::collection_clear(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::collection_contains(System.IntPtr,Mono.Value&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::collection_insert_(System.IntPtr,System.Int32,Mono.Value&,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::collection_iterator_next_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::collection_iterator_reset(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::collection_remove(System.IntPtr,Mono.Value&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::collection_remove_at_(System.IntPtr,System.Int32,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::collection_set_value_at_(System.IntPtr,System.Int32,Mono.Value&,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::consent_prompt_user_for(System.Int32,System.Boolean&,System.Boolean)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::content_control_get_content_sets_parent(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::dependency_object_set_name_on_scope(System.IntPtr,System.String,System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::dependency_object_set_value_(System.IntPtr,System.IntPtr,Mono.Value&,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::dependency_property_get_has_hidden_default_value(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::dependency_property_is_attached(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::dependency_property_is_nullable(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::dependency_property_is_read_only(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::deployment_initialize_app_domain(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::framework_element_apply_template_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::html_object_has_method(System.IntPtr,System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::html_object_has_property(System.IntPtr,System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::html_object_invoke(System.IntPtr,System.IntPtr,System.String,Mono.Value[],System.UInt32,Mono.Value&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::html_object_invoke_self(System.IntPtr,System.IntPtr,Mono.Value[],System.UInt32,Mono.Value&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::internal_transform_try_transform(System.IntPtr,System.Windows.Point,System.Windows.Point&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::managed_unzip_stream_to_stream(Mono.ManagedStreamCallbacks&,Mono.ManagedStreamCallbacks&,System.String)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::managed_unzip_stream_to_stream_nth_file(Mono.ManagedStreamCallbacks&,Mono.ManagedStreamCallbacks&,System.Int32)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::media_frame_allocate_buffer(System.IntPtr,System.UInt32,System.UInt32)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::moon_clipboard_contains_text(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::moon_network_service_get_is_network_available(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::moon_window_get_transparent(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::moon_windowing_system_show_consent_dialog(System.IntPtr,System.String,System.String,System.String,System.Boolean&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::name_scope_get_temporary(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::plugin_instance_get_allow_html_popup_window(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::plugin_instance_get_enable_frame_rate_counter(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::plugin_instance_get_enable_html_access(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::plugin_instance_get_enable_navigation(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::plugin_instance_get_enable_redraw_regions(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::plugin_instance_get_windowless(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::resource_dictionary_add_(System.IntPtr,System.String,Mono.Value&,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::resource_dictionary_clear(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::resource_dictionary_contains_key(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::resource_dictionary_remove(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::resource_dictionary_set(System.IntPtr,System.String,Mono.Value&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::routed_event_args_get_handled(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::storyboard_begin_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::stroke_hit_test(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::surface_get_full_screen(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::surface_in_main_thread()

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::surface_is_loaded(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::surface_is_user_initiated_event(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::surface_is_version_supported(System.String)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::tab_navigation_walker_focus(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::text_box_base_select_(System.IntPtr,System.Int32,System.Int32,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::text_pointer_get_is_at_insertion_position(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::text_selection_select_(System.IntPtr,System.IntPtr,System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::transform_try_transform(System.IntPtr,System.Windows.Point,System.Windows.Point&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::type_get_value_type(Mono.Kind)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::type_is_event_object(Mono.Kind)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::uielement_capture_mouse(System.IntPtr)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::uielement_focus(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::value_from_str(Mono.Kind,System.String,System.String,System.IntPtr&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::value_from_str_with_typename(System.String,System.String,System.String,System.IntPtr&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::window_activate_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::xaml_is_property_set(System.IntPtr,System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Boolean Mono.NativeMethods::xaml_value_from_str_with_parser(System.IntPtr,Mono.Kind,System.String,System.String,System.IntPtr&,System.Boolean&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.AddChildCallback::Invoke(Mono.Xaml.XamlCallbackData*,Mono.Value*,System.Boolean,System.String,Mono.Value*,System.IntPtr,Mono.Value*,System.IntPtr,Mono.MoonError&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ImportXamlNamespaceCallback::Invoke(Mono.Xaml.XamlCallbackData*,System.String,Mono.MoonError&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.LookupObjectCallback::Invoke(Mono.Xaml.XamlCallbackData*,Mono.Value*,System.String,System.String,System.Boolean,System.Boolean,Mono.Value&,Mono.MoonError&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::AddChild(Mono.Xaml.XamlCallbackData*,Mono.Value*,System.Boolean,System.String,Mono.Value*,System.IntPtr,Mono.Value*,System.IntPtr)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::AddChildToItem(Mono.Xaml.XamlCallbackData*,Mono.Value*,System.Object,System.IntPtr,Mono.Value*,System.Object,System.IntPtr)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::AddChildToProperty(Mono.Xaml.XamlCallbackData*,System.Object,System.String,System.Object,System.Object,System.IntPtr)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::cb_add_child(Mono.Xaml.XamlCallbackData*,Mono.Value*,System.Boolean,System.String,Mono.Value*,System.IntPtr,Mono.Value*,System.IntPtr,Mono.MoonError&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::cb_import_xaml_xmlns(Mono.Xaml.XamlCallbackData*,System.String,Mono.MoonError&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::cb_lookup_object(Mono.Xaml.XamlCallbackData*,Mono.Value*,System.String,System.String,System.Boolean,System.Boolean,Mono.Value&,Mono.MoonError&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::cb_set_property(Mono.Xaml.XamlCallbackData*,System.String,Mono.Value*,System.IntPtr,Mono.Value*,System.String,System.String,Mono.Value*,System.IntPtr,Mono.MoonError&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::IsAttachedProperty(Mono.Xaml.XamlCallbackData*,System.Object,System.String,System.String,System.String)

# using 'Mono.Value*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::LookupComponentFromName(Mono.Value*,System.String,System.Boolean,Mono.Value&)

# using 'Mono.Value*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::LookupObject(Mono.Value*,Mono.Value*,System.String,System.String,System.Boolean,System.Boolean,Mono.Value&)

# using 'Mono.Value*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::LookupPropertyObject(Mono.Value*,Mono.Value*,System.String,System.String,System.Int32,System.Boolean,Mono.Value&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::SetProperty(Mono.Xaml.XamlCallbackData*,System.String,Mono.Value*,System.IntPtr,Mono.Value*,System.String,System.String,Mono.Value*,System.IntPtr)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::SetPropertyFromValue(Mono.Xaml.XamlCallbackData*,System.Object,System.IntPtr,Mono.Value*,System.Reflection.PropertyInfo,Mono.Value*,System.IntPtr,System.String&)

# using 'Mono.Value*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::TryGetDefaultAssemblyName(Mono.Value*,System.String&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::TrySetAttachedProperty(Mono.Xaml.XamlCallbackData*,System.String,System.Object,System.IntPtr,System.String,System.String,Mono.Value*)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::TrySetAttachedProperty(Mono.Xaml.XamlCallbackData*,System.String,System.Object,System.IntPtr,System.String,System.String,System.Object)

# using 'Mono.Value*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::TrySetCollectionContentProperty(System.String,System.Object,Mono.Value*,System.IntPtr,Mono.Value*,System.IntPtr)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::TrySetEnumContentProperty(Mono.Xaml.XamlCallbackData*,System.String,System.Object,Mono.Value*,System.IntPtr,Mono.Value*,System.IntPtr)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::TrySetEventReflection(Mono.Xaml.XamlCallbackData*,System.String,System.Object,System.String,System.String,Mono.Value*,System.String&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::TrySetExpression(Mono.Xaml.XamlCallbackData*,System.String,System.Object,System.IntPtr,Mono.Value*,System.String,System.String,System.String,System.String,Mono.Value*,System.IntPtr)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::TrySetObjectTextProperty(Mono.Xaml.XamlCallbackData*,System.String,System.Object,Mono.Value*,System.IntPtr,Mono.Value*,System.IntPtr)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.ManagedXamlLoader::TrySetPropertyReflection(Mono.Xaml.XamlCallbackData*,System.String,System.Object,System.IntPtr,Mono.Value*,System.String,System.String,Mono.Value*,System.IntPtr,System.String&)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Boolean Mono.Xaml.SetPropertyCallback::Invoke(Mono.Xaml.XamlCallbackData*,System.String,Mono.Value*,System.IntPtr,Mono.Value*,System.String,System.String,Mono.Value*,System.IntPtr,Mono.MoonError&)

# overrides 'System.Boolean Mono.Xaml.MarkupExpressionParser::get_ThrowOnNullConverter()'.
+SC-M: System.Boolean Mono.Xaml.SL3MarkupExpressionParser::get_ThrowOnNullConverter()

# overrides 'System.Boolean Mono.Xaml.MarkupExpressionParser::get_ThrowOnNullConverter()'.
+SC-M: System.Boolean Mono.Xaml.SL4MarkupExpressionParser::get_ThrowOnNullConverter()

# overrides 'System.Boolean Mono.MoonlightTypeConverter::CanConvertFrom(System.ComponentModel.ITypeDescriptorContext,System.Type)'.
+SC-M: System.Boolean Mono.Xaml.XamlTypeConverter::CanConvertFrom(System.ComponentModel.ITypeDescriptorContext,System.Type)

# p/invoke declaration
+SC-M: System.Double Mono.NativeMethods::cursor_position_changed_event_args_get_cursor_height(System.IntPtr)

# p/invoke declaration
+SC-M: System.Double Mono.NativeMethods::cursor_position_changed_event_args_get_cursor_x(System.IntPtr)

# p/invoke declaration
+SC-M: System.Double Mono.NativeMethods::cursor_position_changed_event_args_get_cursor_y(System.IntPtr)

# p/invoke declaration
+SC-M: System.Double Mono.NativeMethods::download_progress_event_args_get_progress(System.IntPtr)

# p/invoke declaration
+SC-M: System.Double Mono.NativeMethods::easing_function_base_ease_in_core(System.IntPtr,System.Double)

# p/invoke declaration
+SC-M: System.Double Mono.NativeMethods::glyph_typeface_get_version(System.IntPtr)

# p/invoke declaration
+SC-M: System.Double Mono.NativeMethods::stylus_point_collection_add_stylus_points(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Double Mono.NativeMethods::stylus_point_get_pressure_factor(System.IntPtr)

# p/invoke declaration
+SC-M: System.Double Mono.NativeMethods::stylus_point_get_x(System.IntPtr)

# p/invoke declaration
+SC-M: System.Double Mono.NativeMethods::stylus_point_get_y(System.IntPtr)

# p/invoke declaration
+SC-M: System.Double Mono.NativeMethods::surface_get_zoom_factor(System.IntPtr)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.IAsyncResult Mono.Xaml.AddChildCallback::BeginInvoke(Mono.Xaml.XamlCallbackData*,Mono.Value*,System.Boolean,System.String,Mono.Value*,System.IntPtr,Mono.Value*,System.IntPtr,Mono.MoonError&,System.AsyncCallback,System.Object)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.IAsyncResult Mono.Xaml.ImportXamlNamespaceCallback::BeginInvoke(Mono.Xaml.XamlCallbackData*,System.String,Mono.MoonError&,System.AsyncCallback,System.Object)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.IAsyncResult Mono.Xaml.LookupObjectCallback::BeginInvoke(Mono.Xaml.XamlCallbackData*,Mono.Value*,System.String,System.String,System.Boolean,System.Boolean,Mono.Value&,Mono.MoonError&,System.AsyncCallback,System.Object)

# using 'Mono.Value*' as a parameter type
+SC-M: System.IAsyncResult Mono.Xaml.ParseTemplateFunc::BeginInvoke(Mono.Value*,System.IntPtr,System.IntPtr,System.IntPtr,System.String,Mono.MoonError&,System.AsyncCallback,System.Object)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.IAsyncResult Mono.Xaml.SetPropertyCallback::BeginInvoke(Mono.Xaml.XamlCallbackData*,System.String,Mono.Value*,System.IntPtr,Mono.Value*,System.String,System.String,Mono.Value*,System.IntPtr,Mono.MoonError&,System.AsyncCallback,System.Object)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::capture_source_get_state(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::collection_add_(System.IntPtr,Mono.Value&,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::collection_changed_event_args_get_index(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::collection_get_count(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::collection_index_of(System.IntPtr,Mono.Value&)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::dependency_object_add_managed_property_change_handler(System.IntPtr,System.IntPtr,Mono.UnmanagedPropertyChangeHandlerInvoker,System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::error_event_args_get_error_code(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::error_event_args_get_error_type(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::event_object_add_managed_handler(System.IntPtr,System.Int32,Mono.UnmanagedEventHandlerInvoker,System.IntPtr,Mono.DestroyUnmanagedEvent,System.Boolean,System.Boolean)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::event_object_add_xaml_handler(System.IntPtr,System.Int32,Mono.UnmanagedEventHandler,System.IntPtr,Mono.DestroyUnmanagedEvent,System.Boolean,System.Boolean)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::event_object_get_ref_count(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::external_demuxer_add_stream(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::http_response_get_response_status(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::key_event_args_get_key(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::key_event_args_get_platform_key_code(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::keyboard_get_modifiers()

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::local_message_receiver_get_receiver_name_scope(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::message_received_event_args_get_namescope(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::mouse_wheel_event_args_get_wheel_delta(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::plugin_instance_get_actual_height(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::plugin_instance_get_actual_width(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::plugin_instance_get_max_frame_rate(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::property_changed_event_args_get_id(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::sample_ready_event_args_get_sample_data_length(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::storyboard_get_current_state(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::surface_get_user_initiated_counter(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::text_pointer_compare_to_(System.IntPtr,System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::time_manager_get_maximum_refresh_rate(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Mono.NativeMethods::touch_device_get_id(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int64 Mono.NativeMethods::rendering_event_args_get_rendering_time(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int64 Mono.NativeMethods::sample_ready_event_args_get_frame_duration(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int64 Mono.NativeMethods::sample_ready_event_args_get_sample_time(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int64 Mono.NativeMethods::storyboard_get_current_time(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::application_get_current()

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::audio_stream_new(System.IntPtr,System.Int32,System.Int32,System.Int32,System.Int32,System.Int32,System.Int32,System.IntPtr,System.UInt32)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::bitmap_source_get_bitmap_data(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::capture_device_configuration_get_default_audio_capture_device()

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::capture_device_configuration_get_default_video_capture_device()

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::capture_format_changed_event_args_get_new_audio_format(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::capture_format_changed_event_args_get_new_video_format(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::capture_image_completed_event_args_get_source(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::check_and_download_update_completed_event_args_get_error_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::check_and_download_update_completed_event_args_new(System.Boolean,System.String)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::collection_changed_event_args_get_new_item(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::collection_changed_event_args_get_old_item(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::collection_get_iterator(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::collection_get_value_at_(System.IntPtr,System.Int32,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::collection_iterator_get_current_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::color_from_str(System.String)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::content_control_changed_event_args_get_new_content(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::content_control_changed_event_args_get_old_content(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::control_get_template_child(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::control_get_template_root(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_object_find_name(System.IntPtr,System.String,Mono.Kind&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_object_get_animation_base_value_(System.IntPtr,System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_object_get_mentor(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_object_get_name_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_object_get_resource_base(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_object_get_template_owner(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_object_get_value_no_default_(System.IntPtr,System.IntPtr,Mono.MoonError&)

# internal call
+SC-M: System.IntPtr Mono.NativeMethods::dependency_object_get_value_with_error(System.IntPtr,System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_object_read_local_value_(System.IntPtr,System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_property_get_default_value(System.IntPtr,Mono.Kind)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_property_get_dependency_property(Mono.Kind,System.String)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_property_get_dependency_property_full(Mono.Kind,System.String,System.Boolean)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_property_get_name_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_property_register_core_property(System.String,Mono.Kind,Mono.Kind,Mono.Value&,System.Boolean,System.Boolean,Mono.UnmanagedPropertyChangeHandler)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::dependency_property_register_custom_property(System.String,Mono.Kind,Mono.Kind,Mono.Value&,System.Boolean,System.Boolean,Mono.UnmanagedPropertyChangeHandler)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::deployment_create_http_request(System.IntPtr,Mono.HttpRequestOptions)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::deployment_get_current()

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::deployment_get_surface_reffed(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::deployment_get_system_typefaces(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::deployment_get_types(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::error_event_args_get_error_message_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::error_event_args_get_moon_error(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::event_object_get_type_name_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::font_manager_get_system_glyph_typefaces(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::framework_element_get_logical_parent(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::framework_template_get_visual_tree_(System.IntPtr,System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::general_transform_get_matrix(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::glyph_typeface_get_font_uri_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::http_request_get_response(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::http_request_write_event_args_get_data(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::http_response_get_response_status_text_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::imedia_object_get_media_reffed(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::internal_transform_get_inverse(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::internal_transform_get_matrix3_d(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::local_message_receiver_get_receiver_name_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::local_message_receiver_new(System.String,System.Int32)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::local_message_sender_new(System.String,System.String)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::log_ready_routed_event_args_get_log_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::matrix_get_matrix_values(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::matrix3_d_get_matrix_values(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::media_attribute_get_name_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::media_attribute_get_value_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::media_element_set_demuxer_source(System.IntPtr,System.IntPtr,System.Windows.Media.MediaStreamSource/CloseDemuxerDelegate,System.Windows.Media.MediaStreamSource/GetDiagnosticAsyncDelegate,System.Windows.Media.MediaStreamSource/GetFrameAsyncDelegate,System.Windows.Media.MediaStreamSource/OpenDemuxerAsyncDelegate,System.Windows.Media.MediaStreamSource/SeekAsyncDelegate,System.Windows.Media.MediaStreamSource/SwitchMediaStreamAsyncDelegate)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::media_frame_get_buffer(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::media_frame_new(System.IntPtr,System.IntPtr,System.UInt32,System.UInt64,System.Boolean)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::message_received_event_args_get_message_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::message_received_event_args_get_receiver_name_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::message_received_event_args_get_response_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::message_received_event_args_get_sender_domain_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::moon_clipboard_get_text_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::moon_window_cocoa_get_native_widget(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::moon_window_get_clipboard(System.IntPtr,System.Int32)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::moon_window_get_platform_window(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::moon_window_gtk_get_native_widget(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::moon_windowing_system_create_window(System.IntPtr,Mono.MoonWindowType,System.Int32,System.Int32,System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::moon_windowing_system_get_system_color(System.IntPtr,System.Int32)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::moon_windowing_system_show_open_file_dialog(System.IntPtr,System.String,System.Boolean,System.String,System.Int32)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::moon_windowing_system_show_save_file_dialog_(System.IntPtr,System.String,System.String,System.Int32)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::moonlight_scriptable_object_create(System.IntPtr,Mono.InvalidateHandleDelegate,Mono.HasMemberDelegate,Mono.HasMemberDelegate,Mono.InvokeDelegate,Mono.SetPropertyDelegate,Mono.GetPropertyDelegate)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::mouse_event_args_get_stylus_info(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::mouse_event_args_get_stylus_points(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::plugin_instance_evaluate(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::plugin_instance_get_accessibility_bridge(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::plugin_instance_get_browser_host(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::plugin_instance_get_init_params_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::plugin_instance_get_npwindow(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::plugin_instance_get_source_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::plugin_instance_get_source_location_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::plugin_instance_get_source_location_original_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::plugin_instance_get_source_original_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::plugin_instance_get_surface(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::point_from_str(System.String)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::property_changed_event_args_get_new_value(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::property_changed_event_args_get_old_value(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::property_changed_event_args_get_property(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::rect_from_str(System.String)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::resource_dictionary_changed_event_args_get_key_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::resource_dictionary_changed_event_args_get_new_item(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::resource_dictionary_changed_event_args_get_old_item(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::resource_dictionary_get(System.IntPtr,System.String,System.Boolean&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::resource_dictionary_iterator_get_current_key_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::rich_text_box_get_content_end(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::rich_text_box_get_content_start(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::rich_text_box_get_position_from_point(System.IntPtr,System.Windows.Point)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::rich_text_box_get_selection(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::routed_event_args_get_source(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::runtime_get_network_service()

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::runtime_get_windowing_system()

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::sample_ready_event_args_get_sample_data(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::send_completed_event_args_get_managed_user_state(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::send_completed_event_args_get_message_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::send_completed_event_args_get_receiver_domain_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::send_completed_event_args_get_receiver_name_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::send_completed_event_args_get_response_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::shape_get_geometry_transform(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::sl3_xaml_loader_create_from_file_(System.IntPtr,System.String,System.Boolean,Mono.Kind&,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::sl3_xaml_loader_create_from_string_(System.IntPtr,System.String,System.Boolean,Mono.Kind&,System.Int32,Mono.MoonError&,System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::sl3_xaml_loader_get_context(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::sl3_xaml_loader_hydrate_from_string_(System.IntPtr,System.String,Mono.Value&,System.Boolean,Mono.Kind&,System.Int32,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::storyboard_get_target_dependency_property(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::stroke_collection_hit_test(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::surface_get_background_color(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::surface_get_focused_element(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::surface_get_normal_window(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::surface_get_source_location(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::surface_get_time_manager(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::surface_get_time_manager_reffed(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::surface_get_toplevel(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::surface_new(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_element_get_content_end(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_element_get_content_start(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_element_get_element_end(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_element_get_element_start(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_pointer_get_next_insertion_position(System.IntPtr,System.Windows.Documents.LogicalDirection)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_pointer_get_parent(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_pointer_get_position_at_offset(System.IntPtr,System.Int32,System.Windows.Documents.LogicalDirection)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_selection_get_end(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_selection_get_property_value(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_selection_get_start(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_selection_get_text_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::text_selection_get_xaml_(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::timeline_get_manual_target(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::timeline_marker_routed_event_args_get_marker(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::timeline_marker_routed_event_args_new(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::touch_device_get_directly_over(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::touch_point_get_position(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::touch_point_get_size(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::touch_point_get_touch_device(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::transform_get_inverse(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::types_find(System.IntPtr,Mono.Kind)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::uielement_get_subtree_object(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::uielement_get_transform_to_uielement_(System.IntPtr,System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::uielement_get_visual_parent(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::uri_get_gchandle(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::uri_new(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::video_stream_new(System.IntPtr,System.Int32,System.UInt32,System.UInt32,System.UInt64,System.IntPtr,System.UInt32)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::window_get_moon_window(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::writeable_bitmap_initialize_from_bitmap_source(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::xaml_context_get_template_binding_source(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::xaml_get_element_key_(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::xaml_get_element_name_(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::xaml_get_template_parent(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::xaml_loader_create_from_file_(System.IntPtr,System.String,System.Boolean,Mono.Kind&,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::xaml_loader_create_from_string_(System.IntPtr,System.String,System.Boolean,Mono.Kind&,System.Int32,Mono.MoonError&,System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::xaml_loader_hydrate_from_string_(System.IntPtr,System.String,Mono.Value&,System.Boolean,Mono.Kind&,System.Int32,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::xaml_loader_new(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::xaml_lookup_named_item(System.IntPtr,System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::xaml_uri_for_prefix_(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.IntPtr Mono.NativeMethods::xap_unpack_(System.String)

# overrides 'System.IntPtr Mono.Xaml.XamlLoader::CreateFromFileInternal(System.String,System.Boolean,Mono.Kind&)'.
+SC-M: System.IntPtr Mono.Xaml.ManagedXamlLoader::CreateFromFileInternal(System.String,System.Boolean,Mono.Kind&)

# overrides 'System.IntPtr Mono.Xaml.XamlLoader::CreateFromStringInternal(System.String,System.Boolean,System.Boolean,System.Boolean,Mono.Kind&)'.
+SC-M: System.IntPtr Mono.Xaml.ManagedXamlLoader::CreateFromStringInternal(System.String,System.Boolean,System.Boolean,System.Boolean,Mono.Kind&)

# using 'Mono.Value*' as a parameter type
+SC-M: System.IntPtr Mono.Xaml.ParseTemplateFunc::Invoke(Mono.Value*,System.IntPtr,System.IntPtr,System.IntPtr,System.String,Mono.MoonError&)

# overrides 'System.IntPtr Mono.Xaml.XamlLoader::CreateFromFileInternal(System.String,System.Boolean,Mono.Kind&)'.
+SC-M: System.IntPtr Mono.Xaml.SL4XamlLoader::CreateFromFileInternal(System.String,System.Boolean,Mono.Kind&)

# overrides 'System.IntPtr Mono.Xaml.XamlLoader::CreateFromStringInternal(System.String,System.Boolean,System.Boolean,System.Boolean,Mono.Kind&)'.
+SC-M: System.IntPtr Mono.Xaml.SL4XamlLoader::CreateFromStringInternal(System.String,System.Boolean,System.Boolean,System.Boolean,Mono.Kind&)

# using 'Mono.Value*' as a parameter type
+SC-M: System.IntPtr Mono.Xaml.XamlParser::ParseTemplate(Mono.Value*,System.IntPtr,System.IntPtr,System.IntPtr,System.String,Mono.MoonError&)

# internal call
+SC-M: System.Object Mono.NativeMethods::event_object_get_managed_object(System.IntPtr)

# using 'Mono.Value*' as a parameter type
+SC-M: System.Object Mono.Value::ToObject(System.Type,Mono.Value*)

# overrides 'System.Object Mono.Xaml.XamlLoader::CreateObjectFromFile(System.String,System.Boolean)'.
+SC-M: System.Object Mono.Xaml.ManagedXamlLoader::CreateObjectFromFile(System.String,System.Boolean)

# overrides 'System.Object Mono.Xaml.XamlLoader::CreateObjectFromReader(System.IO.StreamReader,System.Boolean)'.
+SC-M: System.Object Mono.Xaml.ManagedXamlLoader::CreateObjectFromReader(System.IO.StreamReader,System.Boolean)

# overrides 'System.Object Mono.Xaml.XamlLoader::CreateObjectFromString(System.String,System.Boolean)'.
+SC-M: System.Object Mono.Xaml.ManagedXamlLoader::CreateObjectFromString(System.String,System.Boolean)

# overrides 'System.Object Mono.Xaml.XamlLoader::CreateObjectFromString(System.String,System.Boolean,System.Boolean)'.
+SC-M: System.Object Mono.Xaml.ManagedXamlLoader::CreateObjectFromString(System.String,System.Boolean,System.Boolean)

# using 'Mono.Value*' as a parameter type
+SC-M: System.Object Mono.Xaml.ManagedXamlLoader::GetObjectValue(System.Object,System.IntPtr,System.String,System.IntPtr,Mono.Value*,System.String&)

# overrides 'System.Object Mono.Xaml.MarkupExpressionParser::LookupNamedResource(System.Windows.DependencyObject,System.String)'.
+SC-M: System.Object Mono.Xaml.SL3MarkupExpressionParser::LookupNamedResource(System.Windows.DependencyObject,System.String)

# overrides 'System.Object Mono.Xaml.MarkupExpressionParser::LookupNamedResource(System.Windows.DependencyObject,System.String)'.
+SC-M: System.Object Mono.Xaml.SL4MarkupExpressionParser::LookupNamedResource(System.Windows.DependencyObject,System.String)

# overrides 'System.Object Mono.Xaml.XamlLoader::CreateObjectFromFile(System.String,System.Boolean)'.
+SC-M: System.Object Mono.Xaml.SL4XamlLoader::CreateObjectFromFile(System.String,System.Boolean)

# overrides 'System.Object Mono.Xaml.XamlLoader::CreateObjectFromReader(System.IO.StreamReader,System.Boolean)'.
+SC-M: System.Object Mono.Xaml.SL4XamlLoader::CreateObjectFromReader(System.IO.StreamReader,System.Boolean)

# overrides 'System.Object Mono.Xaml.XamlLoader::CreateObjectFromString(System.String,System.Boolean)'.
+SC-M: System.Object Mono.Xaml.SL4XamlLoader::CreateObjectFromString(System.String,System.Boolean)

# overrides 'System.Object Mono.Xaml.XamlLoader::CreateObjectFromString(System.String,System.Boolean,System.Boolean)'.
+SC-M: System.Object Mono.Xaml.SL4XamlLoader::CreateObjectFromString(System.String,System.Boolean,System.Boolean)

# overrides 'System.Object Mono.Xaml.XamlPropertySetter::ConvertTextValue(System.String)'.
+SC-M: System.Object Mono.Xaml.XamlReflectionEventSetter::ConvertTextValue(System.String)

# overrides 'System.Object Mono.MoonlightTypeConverter::ConvertFrom(System.ComponentModel.ITypeDescriptorContext,System.Globalization.CultureInfo,System.Object)'.
+SC-M: System.Object Mono.Xaml.XamlTypeConverter::ConvertFrom(System.ComponentModel.ITypeDescriptorContext,System.Globalization.CultureInfo,System.Object)

# using 'Mono.Value*' as a parameter type
+SC-M: System.Reflection.MethodInfo Mono.Xaml.ManagedXamlLoader::GetGetMethodForAttachedProperty(Mono.Value*,System.String,System.String,System.String,System.String)

# using 'Mono.Value*' as a parameter type
+SC-M: System.Reflection.MethodInfo Mono.Xaml.ManagedXamlLoader::GetMethodForAttachedProperty(Mono.Value*,System.String,System.String,System.String,System.String,System.String,System.Type[])

# using 'Mono.Value*' as a parameter type
+SC-M: System.Reflection.MethodInfo Mono.Xaml.ManagedXamlLoader::GetSetMethodForAttachedProperty(Mono.Value*,System.String,System.String,System.String,System.String)

# using 'System.IntPtr*' as a parameter type
+SC-M: System.String Mono.Helper::CreateMediaLogXml(System.IntPtr*,System.IntPtr*)

# using 'Mono.Value*' as a parameter type
+SC-M: System.Type Mono.Xaml.ManagedXamlLoader::LookupType(Mono.Value*,System.String,System.String)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Type Mono.Xaml.ManagedXamlLoader::TypeFromString(Mono.Xaml.XamlCallbackData*,System.String)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Type Mono.Xaml.ManagedXamlLoader::TypeFromString(Mono.Xaml.XamlCallbackData*,System.String,System.String)

# overrides 'System.Type Mono.Xaml.XamlPropertySetter::get_DeclaringType()'.
+SC-M: System.Type Mono.Xaml.XamlAttachedPropertySetter::get_DeclaringType()

# overrides 'System.Type Mono.Xaml.XamlPropertySetter::get_Type()'.
+SC-M: System.Type Mono.Xaml.XamlAttachedPropertySetter::get_Type()

# overrides 'System.Type Mono.Xaml.XamlPropertySetter::get_DeclaringType()'.
+SC-M: System.Type Mono.Xaml.XamlNamePropertySetter::get_DeclaringType()

# overrides 'System.Type Mono.Xaml.XamlPropertySetter::get_Type()'.
+SC-M: System.Type Mono.Xaml.XamlNamePropertySetter::get_Type()

# overrides 'System.Type Mono.Xaml.XamlPropertySetter::get_DeclaringType()'.
+SC-M: System.Type Mono.Xaml.XamlReflectionEventSetter::get_DeclaringType()

# overrides 'System.Type Mono.Xaml.XamlPropertySetter::get_Type()'.
+SC-M: System.Type Mono.Xaml.XamlReflectionEventSetter::get_Type()

# overrides 'System.Type Mono.Xaml.XamlPropertySetter::get_DeclaringType()'.
+SC-M: System.Type Mono.Xaml.XamlReflectionPropertySetter::get_DeclaringType()

# overrides 'System.Type Mono.Xaml.XamlPropertySetter::get_Type()'.
+SC-M: System.Type Mono.Xaml.XamlReflectionPropertySetter::get_Type()

# p/invoke declaration
+SC-M: System.UInt32 Mono.NativeMethods::http_request_write_event_args_get_count(System.IntPtr)

# p/invoke declaration
+SC-M: System.UInt32 Mono.NativeMethods::moon_windowing_system_get_screen_height(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.UInt32 Mono.NativeMethods::moon_windowing_system_get_screen_width(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.UInt32 Mono.NativeMethods::text_pointer_get_location(System.IntPtr)

# p/invoke declaration
+SC-M: System.UInt64 Mono.NativeMethods::http_request_write_event_args_get_offset(System.IntPtr)

# Promoting interface member to [SecurityCritical] because of 'System.Void Mono.TimeManager::set_NativeHandle(System.IntPtr)'.
+SC-M: System.Void Mono.INativeEventObjectWrapper::set_NativeHandle(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::application_check_and_download_update_async(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::application_register_callbacks(System.IntPtr,Mono.GetDefaultStyleCallback,Mono.ConvertSetterValuesCallback,Mono.GetResourceCallback,Mono.ConvertKeyframeValueCallback)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::application_set_current(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::bitmap_image_pixbuf_write(System.IntPtr,System.IntPtr,System.Int32,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::bitmap_image_pixmap_complete(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::bitmap_source_invalidate(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::bitmap_source_set_bitmap_data(System.IntPtr,System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::capture_device_configuration_get_available_audio_capture_devices(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::capture_device_configuration_get_available_video_capture_devices(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::capture_image_completed_event_args_get_error_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::capture_source_capture_image_async(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::capture_source_start(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::capture_source_stop(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::collection_changed_event_args_set_changed_action(System.IntPtr,System.Windows.CollectionChangedAction)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::collection_changed_event_args_set_index(System.IntPtr,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::collection_changed_event_args_set_new_item(System.IntPtr,Mono.Value&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::collection_changed_event_args_set_old_item(System.IntPtr,Mono.Value&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::collection_iterator_destroy(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::content_control_set_content_sets_parent(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::control_update_is_enabled_source(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::dependency_object_clear_value_(System.IntPtr,System.IntPtr,System.Boolean,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::dependency_object_remove_property_change_handler(System.IntPtr,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::dependency_object_set_name(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::dependency_object_set_resource_base(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::dependency_object_set_template_owner(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::dependency_property_set_is_nullable(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::dependency_property_set_property_changed_callback(System.IntPtr,Mono.UnmanagedPropertyChangeHandler)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::deployment_set_current(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::deployment_set_current_application(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::deployment_set_ensure_managed_peer_callback(System.IntPtr,Mono.EnsureManagedPeerCallback)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::deployment_set_initialization(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::deployment_set_is_loaded_from_xap(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::deployment_set_uri_functions(System.IntPtr,Mono.UriFunctions&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::dispatcher_timer_start(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::dispatcher_timer_stop(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::easing_function_base_set_easing_function(System.IntPtr,Mono.EasingFunctionCallback)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::event_object_add_on_event_handler(System.IntPtr,System.Int32,Mono.UnmanagedEventHandler,System.IntPtr,Mono.DestroyUnmanagedEvent,System.Boolean,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::event_object_do_emit_current_context(System.IntPtr,System.Int32,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::event_object_ref(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::event_object_remove_handler(System.IntPtr,System.Int32,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::event_object_remove_on_event_handler(System.IntPtr,System.Int32,Mono.UnmanagedEventHandler,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::event_object_set_managed_handle(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::event_object_set_managed_peer_callbacks(System.IntPtr,Mono.ManagedRefCallback,Mono.ManagedRefCallback,Mono.MentorChangedCallback)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::event_object_set_object_type(System.IntPtr,Mono.Kind)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::event_object_unref(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::external_demuxer_clear_callbacks(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::external_demuxer_set_can_seek(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::framework_element_apply_default_style(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::framework_element_register_managed_overrides(System.IntPtr,Mono.MeasureOverrideCallback,Mono.ArrangeOverrideCallback,Mono.GetDefaultTemplateCallback,Mono.LoadedCallback)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::framework_element_set_logical_parent_(System.IntPtr,System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::framework_template_set_xaml_buffer(System.IntPtr,Mono.Xaml.ParseTemplateFunc,Mono.Value&,System.String,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::general_transform_transform_point(System.IntPtr,System.Windows.Point&,System.Windows.Point&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::html_object_get_property(System.IntPtr,System.IntPtr,System.String,Mono.Value&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::html_object_release(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::html_object_retain(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::html_object_set_property(System.IntPtr,System.IntPtr,System.String,Mono.Value&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::http_request_abort(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::http_request_open(System.IntPtr,System.String,System.IntPtr,Mono.DownloaderAccessPolicy)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::http_request_send(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::http_request_set_body(System.IntPtr,System.Byte[],System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::http_request_set_header(System.IntPtr,System.String,System.String,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::http_response_visit_headers(System.IntPtr,Mono.HttpHeaderVisitor,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::image_set_source(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::imedia_demuxer_report_get_frame_completed(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::imedia_demuxer_report_get_frame_progress(System.IntPtr,System.Double)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::imedia_demuxer_report_open_demuxer_completed(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::imedia_demuxer_report_seek_completed(System.IntPtr,System.UInt64)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::imedia_demuxer_report_switch_media_stream_completed(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::imedia_demuxer_set_is_drm(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::imedia_object_report_error_occurred(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::local_message_receiver_dispose_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::local_message_receiver_listen_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::local_message_receiver_set_allowed_sender_domains(System.IntPtr,System.String[],System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::local_message_sender_send_async_(System.IntPtr,System.String,System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::media_base_set_source(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::media_element_pause(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::media_element_play(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::media_element_report_error_occurred(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::media_element_request_log(System.IntPtr,System.Windows.Media.LogSource)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::media_element_set_stream_source(System.IntPtr,Mono.ManagedStreamCallbacks&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::media_element_stop(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::media_frame_set_demuxer_height(System.IntPtr,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::media_frame_set_demuxer_width(System.IntPtr,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::message_received_event_args_set_response(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::moon_clipboard_set_text(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::moon_window_set_transparent(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::moonlight_object_add_toggle_ref_notifier(System.IntPtr,Mono.ToggleRef/ToggleNotifyHandler)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::moonlight_object_remove_toggle_ref_notifier(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::moonlight_scriptable_object_register(System.IntPtr,System.String,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::mouse_event_args_get_position(System.IntPtr,System.IntPtr,System.Double&,System.Double&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::multi_scale_image_zoom_about_logical_point(System.IntPtr,System.Double,System.Double,System.Double)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::multi_scale_tile_source_invalidate_tile_layer(System.IntPtr,System.Int32,System.Int32,System.Int32,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::multi_scale_tile_source_set_image_uri_func(System.IntPtr,Mono.ImageUriFunc)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::name_scope_set_temporary(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::plugin_instance_set_enable_frame_rate_counter(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::plugin_instance_set_enable_redraw_regions(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::plugin_instance_set_max_frame_rate(System.IntPtr,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::resource_dictionary_changed_event_args_set_changed_action(System.IntPtr,System.Windows.CollectionChangedAction)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::resource_dictionary_changed_event_args_set_key(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::resource_dictionary_changed_event_args_set_new_item(System.IntPtr,Mono.Value&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::resource_dictionary_changed_event_args_set_old_item(System.IntPtr,Mono.Value&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::resource_dictionary_set_internal_source_(System.IntPtr,System.String,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::rich_text_box_on_got_focus(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::rich_text_box_on_key_down(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::rich_text_box_on_key_up(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::rich_text_box_on_lost_focus(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::rich_text_box_on_mouse_left_button_down(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::rich_text_box_on_mouse_left_button_up(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::rich_text_box_on_mouse_move(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::rich_text_box_post_on_key_down(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::rich_text_box_select_all(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::routed_event_args_set_handled(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::routed_event_args_set_source(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::runtime_gfree(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::runtime_init_desktop()

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::send_completed_event_args_get_error_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::shader_effect_update_shader_constant(System.IntPtr,System.Int32,System.Double,System.Double,System.Double,System.Double)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::shader_effect_update_shader_sampler(System.IntPtr,System.Int32,System.Int32,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::size_changed_event_args_get_new_size(System.IntPtr,System.Windows.Size&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::size_changed_event_args_get_prev_size(System.IntPtr,System.Windows.Size&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::storyboard_flatten_timelines(System.IntPtr,Mono.FlattenTimelinesCallback)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::storyboard_pause_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::storyboard_resume_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::storyboard_seek_(System.IntPtr,System.Int64,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::storyboard_seek_aligned_to_last_tick_(System.IntPtr,System.Int64,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::storyboard_skip_to_fill_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::storyboard_stop_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::stroke_collection_get_bounds(System.IntPtr,System.Windows.Rect&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::stroke_get_bounds(System.IntPtr,System.Windows.Rect&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::style_seal(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::stylus_point_set_pressure_factor(System.IntPtr,System.Double)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::stylus_point_set_x(System.IntPtr,System.Double)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::stylus_point_set_y(System.IntPtr,System.Double)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::surface_attach(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::surface_emit_error(System.IntPtr,System.IntPtr,System.Int32,System.Int32,System.String)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::surface_paint(System.IntPtr,System.IntPtr,System.Int32,System.Int32,System.Int32,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::surface_resize(System.IntPtr,System.Int32,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::surface_set_full_screen(System.IntPtr,System.Boolean)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::surface_set_full_screen_options(System.IntPtr,System.Windows.Interop.FullScreenOptions)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_box_base_on_got_focus(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_box_base_on_key_down(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_box_base_on_key_up(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_box_base_on_lost_focus(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_box_base_on_mouse_left_button_down(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_box_base_on_mouse_left_button_up(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_box_base_on_mouse_move(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_box_base_post_on_key_down(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_box_base_select_all(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_pointer_free(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_selection_apply_property_value(System.IntPtr,System.IntPtr,Mono.Value&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_selection_free(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_selection_insert(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_selection_set_text(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::text_selection_set_xaml_(System.IntPtr,System.String,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::time_manager_add_dispatcher_call(System.IntPtr,Mono.TickCallHandler,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::time_manager_add_tick_call(System.IntPtr,Mono.TickCallHandler,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::time_manager_remove_tick_call(System.IntPtr,Mono.TickCallHandler,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::time_manager_set_maximum_refresh_rate(System.IntPtr,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::timeline_set_manual_target_(System.IntPtr,System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::touch_device_set_directly_over(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::touch_device_set_id(System.IntPtr,System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::touch_point_set_position(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::touch_point_set_size(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::touch_point_set_touch_device(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::types_register_interfaces(System.IntPtr,Mono.Kind,Mono.Kind[],System.Int32)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uielement_arrange_(System.IntPtr,System.Windows.Rect,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uielement_element_added(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uielement_element_removed(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uielement_find_elements_in_host_coordinates_p(System.IntPtr,System.Windows.Point,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uielement_find_elements_in_host_coordinates_r(System.IntPtr,System.Windows.Rect,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uielement_invalidate_arrange(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uielement_invalidate_measure(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uielement_measure_(System.IntPtr,System.Windows.Size,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uielement_release_mouse_capture(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uielement_set_subtree_object(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uielement_update_layout_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::uri_free(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::value_delete_value(Mono.Value&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::value_delete_value2(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::value_free_value(Mono.Value&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::value_free_value2(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::video_brush_set_source(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::window_close_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::window_drag_move_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::window_drag_resize_(System.IntPtr,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::window_set_moon_window(System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::window_set_style(System.IntPtr,System.Windows.WindowStyle)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::window_set_title(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::writeable_bitmap_lock(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::writeable_bitmap_render(System.IntPtr,System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::writeable_bitmap_unlock(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::xaml_delay_set_property(System.IntPtr,System.IntPtr,System.String,System.String,Mono.Value&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::xaml_loader_free(System.IntPtr)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::xaml_loader_set_callbacks(System.IntPtr,Mono.Xaml.XamlLoaderCallbacks&)

# p/invoke declaration
+SC-M: System.Void Mono.NativeMethods::xaml_mark_property_as_set(System.IntPtr,System.IntPtr,System.String)

# implements 'System.Void Mono.INativeEventObjectWrapper::set_NativeHandle(System.IntPtr)'.
+SC-M: System.Void Mono.TimeManager::set_NativeHandle(System.IntPtr)

# overrides 'System.Void Mono.Xaml.XamlLoader::HydrateInternal(System.Object,System.IO.Stream,System.Boolean,System.Boolean,System.Boolean)'.
+SC-M: System.Void Mono.Xaml.ManagedXamlLoader::HydrateInternal(System.Object,System.IO.Stream,System.Boolean,System.Boolean,System.Boolean)

# overrides 'System.Void Mono.Xaml.XamlLoader::HydrateInternal(System.Object,System.String,System.Boolean,System.Boolean,System.Boolean)'.
+SC-M: System.Void Mono.Xaml.ManagedXamlLoader::HydrateInternal(System.Object,System.String,System.Boolean,System.Boolean,System.Boolean)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Void Mono.Xaml.ManagedXamlLoader::SetCLRPropertyFromString(Mono.Xaml.XamlCallbackData*,System.IntPtr,System.Object,System.Reflection.PropertyInfo,System.String,System.String&,System.IntPtr&)

# overrides 'System.Void Mono.Xaml.XamlLoader::Setup(System.IntPtr,System.IntPtr,System.IntPtr)'.
+SC-M: System.Void Mono.Xaml.ManagedXamlLoader::Setup(System.IntPtr,System.IntPtr,System.IntPtr)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Void Mono.Xaml.ManagedXamlLoader::SetValue(Mono.Xaml.XamlCallbackData*,System.IntPtr,System.Reflection.PropertyInfo,System.Object,System.Object)

# overrides 'System.Void Mono.Xaml.XamlLoader::HydrateInternal(System.Object,System.IO.Stream,System.Boolean,System.Boolean,System.Boolean)'.
+SC-M: System.Void Mono.Xaml.SL4XamlLoader::HydrateInternal(System.Object,System.IO.Stream,System.Boolean,System.Boolean,System.Boolean)

# overrides 'System.Void Mono.Xaml.XamlLoader::HydrateInternal(System.Object,System.String,System.Boolean,System.Boolean,System.Boolean)'.
+SC-M: System.Void Mono.Xaml.SL4XamlLoader::HydrateInternal(System.Object,System.String,System.Boolean,System.Boolean,System.Boolean)

# overrides 'System.Void Mono.Xaml.XamlPropertySetter::SetValue(Mono.Xaml.XamlObjectElement,System.Object)'.
+SC-M: System.Void Mono.Xaml.XamlAttachedPropertySetter::SetValue(Mono.Xaml.XamlObjectElement,System.Object)

# overrides 'System.Void Mono.Xaml.XamlPropertySetter::SetValue(Mono.Xaml.XamlObjectElement,System.Object)'.
+SC-M: System.Void Mono.Xaml.XamlNamePropertySetter::SetValue(Mono.Xaml.XamlObjectElement,System.Object)

# overrides 'System.Void Mono.Xaml.XamlPropertySetter::SetValue(Mono.Xaml.XamlObjectElement,System.Object)'.
+SC-M: System.Void Mono.Xaml.XamlReflectionEventSetter::SetValue(Mono.Xaml.XamlObjectElement,System.Object)

# overrides 'System.Void Mono.Xaml.XamlPropertySetter::SetValue(Mono.Xaml.XamlObjectElement,System.Object)'.
+SC-M: System.Void Mono.Xaml.XamlReflectionPropertySetter::SetValue(Mono.Xaml.XamlObjectElement,System.Object)

# implements 'System.Void Mono.INativeEventObjectWrapper::set_NativeHandle(System.IntPtr)'.
+SC-M: System.Void System.Windows.Application::set_NativeHandle(System.IntPtr)

# implements 'System.Void Mono.INativeEventObjectWrapper::set_NativeHandle(System.IntPtr)'.
+SC-M: System.Void System.Windows.Media.CaptureImageCompletedEventArgs::set_NativeHandle(System.IntPtr)

# implements 'System.Void Mono.INativeEventObjectWrapper::set_NativeHandle(System.IntPtr)'.
+SC-M: System.Void System.Windows.Messaging.LocalMessageReceiver::set_NativeHandle(System.IntPtr)

# implements 'System.Void Mono.INativeEventObjectWrapper::set_NativeHandle(System.IntPtr)'.
+SC-M: System.Void System.Windows.Messaging.LocalMessageSender::set_NativeHandle(System.IntPtr)

# implements 'System.Void Mono.INativeEventObjectWrapper::set_NativeHandle(System.IntPtr)'.
+SC-M: System.Void System.Windows.Messaging.MessageReceivedEventArgs::set_NativeHandle(System.IntPtr)

# implements 'System.Void Mono.INativeEventObjectWrapper::set_NativeHandle(System.IntPtr)'.
+SC-M: System.Void System.Windows.Messaging.SendCompletedEventArgs::set_NativeHandle(System.IntPtr)

# implements 'System.Void Mono.INativeEventObjectWrapper::set_NativeHandle(System.IntPtr)'.
+SC-M: System.Void System.Windows.RoutedEventArgs::set_NativeHandle(System.IntPtr)

# p/invoke declaration
+SC-M: System.Windows.CollectionChangedAction Mono.NativeMethods::collection_changed_event_args_get_changed_action(System.IntPtr)

# p/invoke declaration
+SC-M: System.Windows.CollectionChangedAction Mono.NativeMethods::resource_dictionary_changed_event_args_get_changed_action(System.IntPtr)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Windows.DependencyProperty Mono.Xaml.ManagedXamlLoader::DependencyPropertyFromString(Mono.Xaml.XamlCallbackData*,System.Object,Mono.Value*,System.String)

# using 'Mono.Xaml.XamlCallbackData*' as a parameter type
+SC-M: System.Windows.DependencyProperty Mono.Xaml.ManagedXamlLoader::LookupDependencyPropertyForBinding(Mono.Xaml.XamlCallbackData*,System.Windows.DependencyObject,System.String,System.String)

# p/invoke declaration
+SC-M: System.Windows.Documents.LogicalDirection Mono.NativeMethods::text_pointer_get_logical_direction(System.IntPtr)

# p/invoke declaration
+SC-M: System.Windows.InstallState Mono.NativeMethods::application_get_install_state(System.IntPtr)

# p/invoke declaration
+SC-M: System.Windows.Interop.FullScreenOptions Mono.NativeMethods::surface_get_full_screen_options(System.IntPtr)

# p/invoke declaration
+SC-M: System.Windows.Media.LogSource Mono.NativeMethods::log_ready_routed_event_args_get_log_source(System.IntPtr)

# p/invoke declaration
+SC-M: System.Windows.MessageBoxResult Mono.NativeMethods::moon_windowing_system_show_message_box(System.IntPtr,System.Int32,System.String,System.String,System.Windows.MessageBoxButton)

# p/invoke declaration
+SC-M: System.Windows.Point Mono.NativeMethods::multi_scale_image_element_to_logical_point(System.IntPtr,System.Windows.Point)

# p/invoke declaration
+SC-M: System.Windows.Point Mono.NativeMethods::multi_scale_image_logical_to_element_point(System.IntPtr,System.Windows.Point)

# overrides 'System.Windows.PropertyPath Mono.Xaml.MarkupExpressionParser::ParsePropertyPath(System.String)'.
+SC-M: System.Windows.PropertyPath Mono.Xaml.SL3MarkupExpressionParser::ParsePropertyPath(System.String)

# overrides 'System.Windows.PropertyPath Mono.Xaml.MarkupExpressionParser::ParsePropertyPath(System.String)'.
+SC-M: System.Windows.PropertyPath Mono.Xaml.SL4MarkupExpressionParser::ParsePropertyPath(System.String)

# p/invoke declaration
+SC-M: System.Windows.Rect Mono.NativeMethods::geometry_get_bounds(System.IntPtr)

# p/invoke declaration
+SC-M: System.Windows.Rect Mono.NativeMethods::text_pointer_get_character_rect(System.IntPtr,System.Windows.Documents.LogicalDirection)

# p/invoke declaration
+SC-M: System.Windows.Size Mono.NativeMethods::framework_element_arrange_override_(System.IntPtr,System.Windows.Size,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Windows.Size Mono.NativeMethods::framework_element_measure_override_(System.IntPtr,System.Windows.Size,Mono.MoonError&)

# p/invoke declaration
+SC-M: System.Windows.Size Mono.NativeMethods::uielement_get_desired_size(System.IntPtr)

# p/invoke declaration
+SC-M: System.Windows.Size Mono.NativeMethods::uielement_get_render_size(System.IntPtr)

