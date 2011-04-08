# [SecurityCritical] needed to execute code inside 'System.Windows.Browser, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e'.
# 2 methods needs to be decorated.

# overrides 'System.Void Mono.ToggleRef::AddToggleRefNotifyCallback()'.
+SC-M: System.Void Mono.ScriptObjectToggleRef::AddToggleRefNotifyCallback()

# overrides 'System.Void Mono.ToggleRef::RemoveToggleRefNotifyCallback()'.
+SC-M: System.Void Mono.ScriptObjectToggleRef::RemoveToggleRefNotifyCallback()

