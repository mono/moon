# [SecurityCritical] needed to execute code inside 'System.ServiceModel.Web, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e'.
# 4 methods needs to be decorated.

# overrides 'System.Object System.Runtime.Serialization.Json.TypeMapMember::GetMemberOf(System.Object)'.
+SC-M: System.Object System.Runtime.Serialization.Json.TypeMapField::GetMemberOf(System.Object)

# overrides 'System.Object System.Runtime.Serialization.Json.TypeMapMember::GetMemberOf(System.Object)'.
+SC-M: System.Object System.Runtime.Serialization.Json.TypeMapProperty::GetMemberOf(System.Object)

# overrides 'System.Void System.Runtime.Serialization.Json.TypeMapMember::SetMemberValue(System.Object,System.Object)'.
+SC-M: System.Void System.Runtime.Serialization.Json.TypeMapField::SetMemberValue(System.Object,System.Object)

# overrides 'System.Void System.Runtime.Serialization.Json.TypeMapMember::SetMemberValue(System.Object,System.Object)'.
+SC-M: System.Void System.Runtime.Serialization.Json.TypeMapProperty::SetMemberValue(System.Object,System.Object)

