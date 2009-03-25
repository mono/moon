# [SecurityCritical] needed to execute code inside 'System.Windows.Browser, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e'.
# 9 methods needs to be decorated.

# p/invoke declaration
+SC-M: System.IntPtr System.Windows.Browser.ScriptableObjectWrapper/ScriptableNativeMethods::moonlight_object_to_npobject(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr System.Windows.Browser.ScriptableObjectWrapper/ScriptableNativeMethods::npobject_to_moonlight_object(System.IntPtr)

# p/invoke declaration
+SC-M: System.IntPtr System.Windows.Browser.ScriptableObjectWrapper/ScriptableNativeMethods::wrapper_create(System.IntPtr,System.IntPtr,System.Windows.Browser.InvokeDelegate,System.Windows.Browser.SetPropertyDelegate,System.Windows.Browser.GetPropertyDelegate,System.Windows.Browser.EventHandlerDelegate,System.Windows.Browser.EventHandlerDelegate)

# p/invoke declaration
+SC-M: System.IntPtr System.Windows.Browser.ScriptableObjectWrapper/ScriptableNativeMethods::wrapper_create_root(System.IntPtr,System.IntPtr,System.Windows.Browser.InvokeDelegate,System.Windows.Browser.SetPropertyDelegate,System.Windows.Browser.GetPropertyDelegate,System.Windows.Browser.EventHandlerDelegate,System.Windows.Browser.EventHandlerDelegate)

# p/invoke declaration
+SC-M: System.Void System.Windows.Browser.ScriptableObjectWrapper/ScriptableNativeMethods::add_event(System.IntPtr,System.IntPtr,System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Void System.Windows.Browser.ScriptableObjectWrapper/ScriptableNativeMethods::add_method(System.IntPtr,System.IntPtr,System.IntPtr,System.String,System.TypeCode,System.TypeCode[],System.Int32)

# p/invoke declaration
+SC-M: System.Void System.Windows.Browser.ScriptableObjectWrapper/ScriptableNativeMethods::add_property(System.IntPtr,System.IntPtr,System.IntPtr,System.String,System.TypeCode,System.Boolean,System.Boolean)

# p/invoke declaration
+SC-M: System.Void System.Windows.Browser.ScriptableObjectWrapper/ScriptableNativeMethods::emit_event(System.IntPtr,System.IntPtr,System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void System.Windows.Browser.ScriptableObjectWrapper/ScriptableNativeMethods::register(System.IntPtr,System.String,System.IntPtr)

