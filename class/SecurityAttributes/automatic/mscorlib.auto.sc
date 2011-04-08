# [SecurityCritical] needed to execute code inside 'mscorlib, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e'.
# 611 methods needs to be decorated.

# internal call
+SC-M: System.AppDomain System.AppDomain::getCurDomain()

# internal call
+SC-M: System.AppDomain System.AppDomain::InternalSetDomain(System.AppDomain)

# internal call
+SC-M: System.AppDomain System.AppDomain::InternalSetDomainByID(System.Int32)

# internal call
+SC-M: System.AppDomainSetup System.AppDomain::getSetup()

# internal call
+SC-M: System.Array System.Array::CreateInstanceImpl(System.Type,System.Int32[],System.Int32[])

# [VISIBLE] overrides 'System.Boolean System.Runtime.InteropServices.CriticalHandle::get_IsInvalid()'.
+SC-M: System.Boolean Microsoft.Win32.SafeHandles.CriticalHandleMinusOneIsInvalid::get_IsInvalid()

# [VISIBLE] overrides 'System.Boolean System.Runtime.InteropServices.SafeHandle::get_IsInvalid()'.
+SC-M: System.Boolean Microsoft.Win32.SafeHandles.SafeHandleMinusOneIsInvalid::get_IsInvalid()

# [VISIBLE] overrides 'System.Boolean System.Runtime.InteropServices.SafeHandle::get_IsInvalid()'.
+SC-M: System.Boolean Microsoft.Win32.SafeHandles.SafeHandleZeroOrMinusOneIsInvalid::get_IsInvalid()

# localloc
+SC-M: System.Boolean Mono.Globalization.Unicode.SimpleCollator::IsPrefix(System.String,System.String,System.Int32,System.Int32,System.Globalization.CompareOptions)

# using 'System.Byte*' as a parameter type
+SC-M: System.Boolean Mono.Globalization.Unicode.SimpleCollator::MatchesBackward(System.String,System.Int32&,System.Int32,System.Int32,System.Int32,System.Byte*,System.Boolean,Mono.Globalization.Unicode.SimpleCollator/Context&)

# using 'System.Byte*' as a parameter type
+SC-M: System.Boolean Mono.Globalization.Unicode.SimpleCollator::MatchesBackwardCore(System.String,System.Int32&,System.Int32,System.Int32,System.Int32,System.Byte*,System.Boolean,Mono.Globalization.Unicode.SimpleCollator/ExtenderType,Mono.Globalization.Unicode.Contraction&,Mono.Globalization.Unicode.SimpleCollator/Context&)

# using 'System.Byte*' as a parameter type
+SC-M: System.Boolean Mono.Globalization.Unicode.SimpleCollator::MatchesForward(System.String,System.Int32&,System.Int32,System.Int32,System.Byte*,System.Boolean,Mono.Globalization.Unicode.SimpleCollator/Context&)

# using 'System.Byte*' as a parameter type
+SC-M: System.Boolean Mono.Globalization.Unicode.SimpleCollator::MatchesForwardCore(System.String,System.Int32&,System.Int32,System.Int32,System.Byte*,System.Boolean,Mono.Globalization.Unicode.SimpleCollator/ExtenderType,Mono.Globalization.Unicode.Contraction&,Mono.Globalization.Unicode.SimpleCollator/Context&)

# using 'System.Byte*' as a parameter type
+SC-M: System.Boolean Mono.Globalization.Unicode.SimpleCollator::MatchesPrimitive(System.Globalization.CompareOptions,System.Byte*,System.Int32,Mono.Globalization.Unicode.SimpleCollator/ExtenderType,System.Byte*,System.Int32,System.Boolean)

# internal call
+SC-M: System.Boolean System.Array::FastCopy(System.Array,System.Int32,System.Array,System.Int32,System.Int32)

# internal call
+SC-M: System.Boolean System.Buffer::BlockCopyInternal(System.Array,System.Int32,System.Array,System.Int32,System.Int32)

# internal call
+SC-M: System.Boolean System.CurrentSystemTimeZone::GetTimeZoneData(System.Int32,System.Int64[]&,System.String[]&)

# internal call
+SC-M: System.Boolean System.Diagnostics.Debugger::IsAttached_internal()

# internal call
+SC-M: System.Boolean System.Diagnostics.StackFrame::get_frame_info(System.Int32,System.Boolean,System.Reflection.MethodBase&,System.Int32&,System.Int32&,System.String&,System.Int32&,System.Int32&)

# internal call
+SC-M: System.Boolean System.Double::ParseImpl(System.Byte*,System.Double&)

# internal call
+SC-M: System.Boolean System.Globalization.CultureInfo::construct_internal_locale_from_current_locale(System.Globalization.CultureInfo)

# internal call
+SC-M: System.Boolean System.Globalization.CultureInfo::construct_internal_locale_from_lcid(System.Int32)

# internal call
+SC-M: System.Boolean System.Globalization.CultureInfo::construct_internal_locale_from_name(System.String)

# internal call
+SC-M: System.Boolean System.Globalization.RegionInfo::construct_internal_region_from_name(System.String)

# p/invoke declaration
+SC-M: System.Boolean System.IO.IsolatedStorage.IsolatedStorageFile::isolated_storage_increase_quota_to(System.String,System.String)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::Close(System.IntPtr,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::CopyFile(System.String,System.String,System.Boolean,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::CreateDirectory(System.String,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::DeleteFile(System.String,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::Flush(System.IntPtr,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::GetFileStat(System.String,System.IO.MonoIOStat&,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::MoveFile(System.String,System.String,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::RemoveDirectory(System.String,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::SetCurrentDirectory(System.String,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::SetFileAttributes(System.String,System.IO.FileAttributes,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::SetFileTime(System.IntPtr,System.Int64,System.Int64,System.Int64,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.IO.MonoIO::SetLength(System.IntPtr,System.Int64,System.IO.MonoIOError&)

# internal call
+SC-M: System.Boolean System.MonoCustomAttrs::IsDefinedInternal(System.Reflection.ICustomAttributeProvider,System.Type)

# internal call
+SC-M: System.Boolean System.Reflection.Assembly::GetManifestResourceInfoInternal(System.String,System.Reflection.ManifestResourceInfo)

# internal call
+SC-M: System.Boolean System.Reflection.AssemblyName::ParseName(System.Reflection.AssemblyName,System.String)

# internal call
+SC-M: System.Boolean System.Runtime.InteropServices.GCHandle::CheckCurrentDomain(System.Int32)

# implements 'System.Boolean System.Runtime.Remoting.Messaging.IMethodMessage::get_HasVarArgs()'.
+SC-M: System.Boolean System.Runtime.Remoting.Messaging.ErrorMessage::get_HasVarArgs()

# Promoting interface member to [SecurityCritical] because of 'System.Boolean System.Runtime.Remoting.Messaging.ErrorMessage::get_HasVarArgs()'.
+SC-M: System.Boolean System.Runtime.Remoting.Messaging.IMethodMessage::get_HasVarArgs()

# implements 'System.Boolean System.Runtime.Remoting.Messaging.IMethodMessage::get_HasVarArgs()'.
+SC-M: System.Boolean System.Runtime.Remoting.Messaging.MethodCall::get_HasVarArgs()

# implements 'System.Boolean System.Runtime.Remoting.Messaging.IMethodMessage::get_HasVarArgs()'.
+SC-M: System.Boolean System.Runtime.Remoting.Messaging.MethodResponse::get_HasVarArgs()

# implements 'System.Boolean System.Runtime.Remoting.Messaging.IMethodMessage::get_HasVarArgs()'.
+SC-M: System.Boolean System.Runtime.Remoting.Messaging.MonoMethodMessage::get_HasVarArgs()

# implements 'System.Boolean System.Runtime.Remoting.Messaging.IMethodMessage::get_HasVarArgs()'.
+SC-M: System.Boolean System.Runtime.Remoting.Messaging.ReturnMessage::get_HasVarArgs()

# overrides 'System.Boolean System.Runtime.Remoting.Metadata.SoapAttribute::get_UseAttribute()'.
+SC-M: System.Boolean System.Runtime.Remoting.Metadata.SoapMethodAttribute::get_UseAttribute()

# overrides 'System.Boolean System.Runtime.Remoting.Metadata.SoapAttribute::get_UseAttribute()'.
+SC-M: System.Boolean System.Runtime.Remoting.Metadata.SoapTypeAttribute::get_UseAttribute()

# internal call
+SC-M: System.Boolean System.Runtime.Remoting.RemotingServices::IsTransparentProxy(System.Object)

# overrides 'System.Boolean System.Runtime.Serialization.Formatters.Binary.TypeMetadata::get_RequiresTypes()'.
+SC-M: System.Boolean System.Runtime.Serialization.Formatters.Binary.ClrTypeMetadata::get_RequiresTypes()

# overrides 'System.Boolean System.Runtime.Serialization.Formatters.Binary.TypeMetadata::get_RequiresTypes()'.
+SC-M: System.Boolean System.Runtime.Serialization.Formatters.Binary.SerializableTypeMetadata::get_RequiresTypes()

# overrides 'System.Boolean System.Runtime.Serialization.Formatters.Binary.TypeMetadata::IsCompatible(System.Runtime.Serialization.Formatters.Binary.TypeMetadata)'.
+SC-M: System.Boolean System.Runtime.Serialization.Formatters.Binary.SerializableTypeMetadata::IsCompatible(System.Runtime.Serialization.Formatters.Binary.TypeMetadata)

# internal call
+SC-M: System.Boolean System.Security.Cryptography.RNGCryptoServiceProvider::RngOpen()

# internal call
+SC-M: System.Boolean System.Security.SecurityManager::get_RequiresElevatedPermissions()

# internal call
+SC-M: System.Boolean System.Security.SecurityManager::get_SecurityEnabled()

# using 'System.Security.RuntimeDeclSecurityActions*' as a parameter type
+SC-M: System.Boolean System.Security.SecurityManager::InheritanceDemand(System.AppDomain,System.Reflection.Assembly,System.Security.RuntimeDeclSecurityActions*)

# using 'System.Security.RuntimeDeclSecurityActions*' as a parameter type
+SC-M: System.Boolean System.Security.SecurityManager::LinkDemand(System.Reflection.Assembly,System.Security.RuntimeDeclSecurityActions*,System.Security.RuntimeDeclSecurityActions*)

# internal call
+SC-M: System.Boolean System.Threading.Monitor::Monitor_test_synchronised(System.Object)

# internal call
+SC-M: System.Boolean System.Threading.Monitor::Monitor_try_enter(System.Object,System.Int32)

# internal call
+SC-M: System.Boolean System.Threading.Monitor::Monitor_wait(System.Object,System.Int32)

# internal call
+SC-M: System.Boolean System.Threading.Mutex::ReleaseMutex_internal(System.IntPtr)

# internal call
+SC-M: System.Boolean System.Threading.NativeEventCalls::ResetEvent_internal(System.IntPtr)

# internal call
+SC-M: System.Boolean System.Threading.NativeEventCalls::SetEvent_internal(System.IntPtr)

# internal call
+SC-M: System.Boolean System.Threading.Thread::Join_internal(System.Threading.InternalThread,System.Int32,System.IntPtr)

# internal call
+SC-M: System.Boolean System.Threading.WaitHandle::WaitAll_internal(System.Threading.WaitHandle[],System.Int32,System.Boolean)

# internal call
+SC-M: System.Boolean System.Threading.WaitHandle::WaitOne_internal(System.IntPtr,System.Int32,System.Boolean)

# internal call
+SC-M: System.Boolean System.Type::EqualsInternal(System.Type)

# internal call
+SC-M: System.Boolean System.Type::IsArrayImpl(System.Type)

# internal call
+SC-M: System.Boolean System.Type::IsInstanceOfType(System.Type,System.Object)

# internal call
+SC-M: System.Boolean System.Type::type_is_assignable_from(System.Type,System.Type)

# internal call
+SC-M: System.Boolean System.Type::type_is_subtype_of(System.Type,System.Type,System.Boolean)

# internal call
+SC-M: System.Boolean System.ValueType::InternalEquals(System.Object,System.Object,System.Object[]&)

# internal call
+SC-M: System.Byte System.Buffer::GetByteInternal(System.Array,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Byte[] Mono.Security.BitConverterLE::GetUIntBytes(System.Byte*)

# using 'System.Byte*' as a parameter type
+SC-M: System.Byte[] Mono.Security.BitConverterLE::GetULongBytes(System.Byte*)

# using 'System.Byte*' as a parameter type
+SC-M: System.Byte[] Mono.Security.BitConverterLE::GetUShortBytes(System.Byte*)

# using 'System.Byte*' as a parameter type
+SC-M: System.Byte[] System.BitConverter::GetBytes(System.Byte*,System.Int32)

# internal call
+SC-M: System.Byte[] System.Convert::InternalFromBase64CharArray(System.Char[],System.Int32,System.Int32)

# internal call
+SC-M: System.Byte[] System.Convert::InternalFromBase64String(System.String,System.Boolean)

# internal call
+SC-M: System.Byte[] System.Reflection.Emit.CustomAttributeBuilder::GetBlob(System.Reflection.Assembly,System.Reflection.ConstructorInfo,System.Object[],System.Reflection.PropertyInfo[],System.Object[],System.Reflection.FieldInfo[],System.Object[])

# overrides 'System.Byte[] System.Reflection.Module::ResolveSignature(System.Int32)'.
+SC-M: System.Byte[] System.Reflection.Emit.ModuleBuilder::ResolveSignature(System.Int32)

# internal call
+SC-M: System.Byte[] System.Reflection.Emit.SignatureHelper::get_signature_field()

# internal call
+SC-M: System.Byte[] System.Reflection.Emit.SignatureHelper::get_signature_local()

# internal call
+SC-M: System.Byte[] System.Reflection.Module::ResolveSignature(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)

# overrides 'System.Byte[] System.Reflection.Module::ResolveSignature(System.Int32)'.
+SC-M: System.Byte[] System.Reflection.MonoModule::ResolveSignature(System.Int32)

# internal call
+SC-M: System.Char System.IO.MonoIO::get_AltDirectorySeparatorChar()

# internal call
+SC-M: System.Char System.IO.MonoIO::get_DirectorySeparatorChar()

# internal call
+SC-M: System.Char System.IO.MonoIO::get_PathSeparator()

# internal call
+SC-M: System.Char System.IO.MonoIO::get_VolumeSeparatorChar()

# using 'System.Char*' as a parameter type
+SC-M: System.Char[] System.Text.UTF8Encoding::GetFallbackChars(System.Char*,System.Char*,System.Text.EncoderFallback,System.Text.EncoderFallbackBuffer&)

# implements 'System.Collections.IDictionary System.Runtime.Remoting.Messaging.IMessage::get_Properties()'.
+SC-M: System.Collections.IDictionary System.Runtime.Remoting.Messaging.ConstructionCall::get_Properties()

# implements 'System.Collections.IDictionary System.Runtime.Remoting.Messaging.IMessage::get_Properties()'.
+SC-M: System.Collections.IDictionary System.Runtime.Remoting.Messaging.ConstructionResponse::get_Properties()

# implements 'System.Collections.IDictionary System.Runtime.Remoting.Messaging.IMessage::get_Properties()'.
+SC-M: System.Collections.IDictionary System.Runtime.Remoting.Messaging.ErrorMessage::get_Properties()

# Promoting interface member to [SecurityCritical] because of 'System.Collections.IDictionary System.Runtime.Remoting.Messaging.ConstructionCall::get_Properties()'.
+SC-M: System.Collections.IDictionary System.Runtime.Remoting.Messaging.IMessage::get_Properties()

# implements 'System.Collections.IDictionary System.Runtime.Remoting.Messaging.IMessage::get_Properties()'.
+SC-M: System.Collections.IDictionary System.Runtime.Remoting.Messaging.MethodCall::get_Properties()

# implements 'System.Collections.IDictionary System.Runtime.Remoting.Messaging.IMessage::get_Properties()'.
+SC-M: System.Collections.IDictionary System.Runtime.Remoting.Messaging.MethodResponse::get_Properties()

# implements 'System.Collections.IDictionary System.Runtime.Remoting.Messaging.IMessage::get_Properties()'.
+SC-M: System.Collections.IDictionary System.Runtime.Remoting.Messaging.MonoMethodMessage::get_Properties()

# implements 'System.Collections.IDictionary System.Runtime.Remoting.Messaging.IMessage::get_Properties()'.
+SC-M: System.Collections.IDictionary System.Runtime.Remoting.Messaging.ReturnMessage::get_Properties()

# internal call
+SC-M: System.Delegate System.Delegate::CreateDelegate_internal(System.Type,System.Object,System.Reflection.MethodInfo,System.Boolean)

# internal call
+SC-M: System.Delegate System.Runtime.InteropServices.Marshal::GetDelegateForFunctionPointerInternal(System.IntPtr,System.Type)

# internal call
+SC-M: System.Diagnostics.StackFrame[] System.Diagnostics.StackTrace::get_trace(System.Exception,System.Int32,System.Boolean)

# internal call
+SC-M: System.Double System.Decimal::decimal2double(System.Decimal&)

# internal call
+SC-M: System.Double System.Math::Round2(System.Double,System.Int32,System.Boolean)

# internal call
+SC-M: System.Double System.Threading.Interlocked::CompareExchange(System.Double&,System.Double,System.Double)

# overrides 'System.Guid System.Reflection.Module::get_ModuleVersionId()'.
+SC-M: System.Guid System.Reflection.Emit.ModuleBuilder::get_ModuleVersionId()

# overrides 'System.Guid System.Reflection.Module::GetModuleVersionId()'.
+SC-M: System.Guid System.Reflection.Emit.ModuleBuilder::GetModuleVersionId()

# overrides 'System.Guid System.Reflection.Module::get_ModuleVersionId()'.
+SC-M: System.Guid System.Reflection.MonoModule::get_ModuleVersionId()

# localloc
+SC-M: System.Int32 Mono.Globalization.Unicode.SimpleCollator::Compare(System.String,System.Int32,System.Int32,System.String,System.Int32,System.Int32,System.Globalization.CompareOptions)

# localloc
+SC-M: System.Int32 Mono.Globalization.Unicode.SimpleCollator::IndexOf(System.String,System.Char,System.Int32,System.Int32,System.Globalization.CompareOptions)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 Mono.Globalization.Unicode.SimpleCollator::IndexOf(System.String,System.String,System.Int32,System.Int32,System.Byte*,Mono.Globalization.Unicode.SimpleCollator/Context&)

# localloc
+SC-M: System.Int32 Mono.Globalization.Unicode.SimpleCollator::IndexOf(System.String,System.String,System.Int32,System.Int32,System.Globalization.CompareOptions)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 Mono.Globalization.Unicode.SimpleCollator::IndexOfSortKey(System.String,System.Int32,System.Int32,System.Byte*,System.Char,System.Int32,System.Boolean,Mono.Globalization.Unicode.SimpleCollator/Context&)

# localloc
+SC-M: System.Int32 Mono.Globalization.Unicode.SimpleCollator::LastIndexOf(System.String,System.Char,System.Int32,System.Int32,System.Globalization.CompareOptions)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 Mono.Globalization.Unicode.SimpleCollator::LastIndexOf(System.String,System.String,System.Int32,System.Int32,System.Byte*,Mono.Globalization.Unicode.SimpleCollator/Context&)

# localloc
+SC-M: System.Int32 Mono.Globalization.Unicode.SimpleCollator::LastIndexOf(System.String,System.String,System.Int32,System.Int32,System.Globalization.CompareOptions)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 Mono.Globalization.Unicode.SimpleCollator::LastIndexOfSortKey(System.String,System.Int32,System.Int32,System.Int32,System.Byte*,System.Int32,System.Boolean,Mono.Globalization.Unicode.SimpleCollator/Context&)

# internal call
+SC-M: System.Int32 System.Array::GetRank()

# internal call
+SC-M: System.Int32 System.Buffer::ByteLengthInternal(System.Array)

# internal call
+SC-M: System.Int32 System.Decimal::decimal2Int64(System.Decimal&,System.Int64&)

# internal call
+SC-M: System.Int32 System.Decimal::decimal2UInt64(System.Decimal&,System.UInt64&)

# internal call
+SC-M: System.Int32 System.Decimal::decimalCompare(System.Decimal&,System.Decimal&)

# internal call
+SC-M: System.Int32 System.Decimal::decimalDiv(System.Decimal&,System.Decimal&,System.Decimal&)

# internal call
+SC-M: System.Int32 System.Decimal::decimalIncr(System.Decimal&,System.Decimal&)

# internal call
+SC-M: System.Int32 System.Decimal::decimalMult(System.Decimal&,System.Decimal&)

# internal call
+SC-M: System.Int32 System.Decimal::decimalSetExponent(System.Decimal&,System.Int32)

# internal call
+SC-M: System.Int32 System.Decimal::double2decimal(System.Decimal&,System.Double,System.Int32)

# internal call
+SC-M: System.Int32 System.Decimal::string2decimal(System.Decimal&,System.String,System.UInt32,System.Int32)

# internal call
+SC-M: System.Int32 System.Enum::compare_value_to(System.Object)

# internal call
+SC-M: System.Int32 System.Enum::get_hashcode()

# internal call
+SC-M: System.Int32 System.GC::CollectionCount(System.Int32)

# internal call
+SC-M: System.Int32 System.GC::GetGeneration(System.Object)

# internal call
+SC-M: System.Int32 System.IO.MonoIO::FindClose(System.IntPtr)

# internal call
+SC-M: System.Int32 System.IO.MonoIO::Read(System.IntPtr,System.Byte[],System.Int32,System.Int32,System.IO.MonoIOError&)

# internal call
+SC-M: System.Int32 System.IO.MonoIO::Write(System.IntPtr,System.Byte[],System.Int32,System.Int32,System.IO.MonoIOError&)

# internal call
+SC-M: System.Int32 System.Object::InternalGetHashCode(System.Object)

# internal call
+SC-M: System.Int32 System.Reflection.Assembly::MonoDebugger_GetMethodToken(System.Reflection.MethodBase)

# internal call
+SC-M: System.Int32 System.Reflection.Emit.ModuleBuilder::getMethodToken(System.Reflection.Emit.ModuleBuilder,System.Reflection.MethodInfo,System.Type[])

# internal call
+SC-M: System.Int32 System.Reflection.Emit.ModuleBuilder::getToken(System.Reflection.Emit.ModuleBuilder,System.Object,System.Boolean)

# internal call
+SC-M: System.Int32 System.Reflection.Emit.ModuleBuilder::getUSIndex(System.Reflection.Emit.ModuleBuilder,System.String)

# internal call
+SC-M: System.Int32 System.Reflection.Module::get_MetadataToken(System.Reflection.Module)

# internal call
+SC-M: System.Int32 System.Reflection.Module::GetMDStreamVersion(System.IntPtr)

# internal call
+SC-M: System.Int32 System.Reflection.MonoMethodInfo::get_method_attributes(System.IntPtr)

# overrides 'System.Int32 System.Reflection.Module::get_MDStreamVersion()'.
+SC-M: System.Int32 System.Reflection.MonoModule::get_MDStreamVersion()

# internal call
+SC-M: System.Int32 System.Reflection.ParameterInfo::GetMetadataToken()

# internal call
+SC-M: System.Int32 System.Runtime.InteropServices.GCHandle::GetTargetHandle(System.Object,System.Int32,System.Runtime.InteropServices.GCHandleType)

# internal call
+SC-M: System.Int32 System.Runtime.InteropServices.Marshal::AddRefInternal(System.IntPtr)

# internal call
+SC-M: System.Int32 System.Runtime.InteropServices.Marshal::QueryInterfaceInternal(System.IntPtr,System.Guid&,System.IntPtr&)

# internal call
+SC-M: System.Int32 System.Runtime.InteropServices.Marshal::ReleaseInternal(System.IntPtr)

# implements 'System.Int32 System.Runtime.Remoting.Channels.IChannel::get_ChannelPriority()'.
+SC-M: System.Int32 System.Runtime.Remoting.Channels.CrossAppDomainChannel::get_ChannelPriority()

# Promoting interface member to [SecurityCritical] because of 'System.Int32 System.Runtime.Remoting.Messaging.MethodResponse::get_OutArgCount()'.
+SC-M: System.Int32 System.Runtime.Remoting.Messaging.IMethodReturnMessage::get_OutArgCount()

# implements 'System.Int32 System.Runtime.Remoting.Messaging.IMethodReturnMessage::get_OutArgCount()'.
+SC-M: System.Int32 System.Runtime.Remoting.Messaging.MethodResponse::get_OutArgCount()

# implements 'System.Int32 System.Runtime.Remoting.Messaging.IMethodReturnMessage::get_OutArgCount()'.
+SC-M: System.Int32 System.Runtime.Remoting.Messaging.MonoMethodMessage::get_OutArgCount()

# implements 'System.Int32 System.Runtime.Remoting.Messaging.IMethodReturnMessage::get_OutArgCount()'.
+SC-M: System.Int32 System.Runtime.Remoting.Messaging.ReturnMessage::get_OutArgCount()

# internal call
+SC-M: System.Int32 System.String::GetLOSLimit()

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.ASCIIEncoding::GetByteCount(System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.ASCIIEncoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.ASCIIEncoding::InternalGetBytes(System.Char*,System.Int32,System.Int32,System.Int32,System.Byte[],System.Int32,System.Text.EncoderFallbackBuffer&,System.Char[]&)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.Encoding::GetByteCount(System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UnicodeEncoding::GetByteCount(System.Char*,System.Int32)

# [VISIBLE] overrides 'System.Int32 System.Text.Encoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)'.
+SC-M: System.Int32 System.Text.UnicodeEncoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UnicodeEncoding::GetBytesInternal(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UnicodeEncoding::GetCharsInternal(System.Byte*,System.Int32,System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF32Encoding::GetByteCount(System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF32Encoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF7Encoding::GetByteCount(System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF7Encoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding/UTF8Encoder::GetByteCount(System.Char*,System.Int32,System.Boolean)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding/UTF8Encoder::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32,System.Boolean)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::Fallback(System.Object,System.Text.DecoderFallbackBuffer&,System.Byte[]&,System.Byte*,System.Int64,System.UInt32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::GetByteCount(System.Char*,System.Int32)

# [VISIBLE] overrides 'System.Int32 System.Text.Encoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)'.
+SC-M: System.Int32 System.Text.UTF8Encoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::InternalGetByteCount(System.Char*,System.Int32,System.Text.EncoderFallback,System.Char&,System.Boolean)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::InternalGetBytes(System.Char*,System.Int32,System.Byte*,System.Int32,System.Text.EncoderFallback,System.Text.EncoderFallbackBuffer&,System.Char&,System.Boolean)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::InternalGetCharCount(System.Byte*,System.Int32,System.UInt32,System.UInt32,System.Object,System.Text.DecoderFallbackBuffer&,System.Byte[]&,System.Boolean)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::InternalGetChars(System.Byte*,System.Int32,System.Char*,System.Int32,System.UInt32&,System.UInt32&,System.Object,System.Text.DecoderFallbackBuffer&,System.Byte[]&,System.Boolean)

# internal call
+SC-M: System.Int32 System.Threading.Thread::GetDomainID()

# internal call
+SC-M: System.Int32 System.Threading.WaitHandle::WaitAny_internal(System.Threading.WaitHandle[],System.Int32,System.Boolean)

# internal call
+SC-M: System.Int32 System.Type::GetGenericParameterPosition()

# internal call
+SC-M: System.Int32 System.ValueType::InternalGetHashCode(System.Object,System.Object[]&)

# internal call
+SC-M: System.Int64 System.DateTime::GetNow()

# internal call
+SC-M: System.Int64 System.DateTime::GetTimeMonotonic()

# p/invoke declaration
+SC-M: System.Int64 System.IO.IsolatedStorage.IsolatedStorage::isolated_storage_get_current_usage(System.String)

# internal call
+SC-M: System.Int64 System.IO.MonoIO::GetLength(System.IntPtr,System.IO.MonoIOError&)

# internal call
+SC-M: System.Int64 System.IO.MonoIO::Seek(System.IntPtr,System.Int64,System.IO.SeekOrigin,System.IO.MonoIOError&)

# internal call
+SC-M: System.IntPtr System.ArgIterator::IntGetNextArgType()

# internal call
+SC-M: System.IntPtr System.IO.MonoIO::get_ConsoleError()

# internal call
+SC-M: System.IntPtr System.IO.MonoIO::get_ConsoleInput()

# internal call
+SC-M: System.IntPtr System.IO.MonoIO::get_ConsoleOutput()

# internal call
+SC-M: System.IntPtr System.IO.MonoIO::Open(System.String,System.IO.FileMode,System.IO.FileAccess,System.IO.FileShare,System.IO.FileOptions,System.IO.MonoIOError&)

# internal call
+SC-M: System.IntPtr System.Object::obj_address()

# internal call
+SC-M: System.IntPtr System.Reflection.Assembly::GetManifestResourceInternal(System.String,System.Int32&,System.Reflection.Module&)

# internal call
+SC-M: System.IntPtr System.Reflection.Module::ResolveFieldToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)

# internal call
+SC-M: System.IntPtr System.Reflection.Module::ResolveMethodToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)

# internal call
+SC-M: System.IntPtr System.Reflection.Module::ResolveTypeToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.GCHandle::GetAddrOfPinnedObject(System.Int32)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.Marshal::AllocCoTaskMem(System.Int32)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.Marshal::AllocHGlobal(System.IntPtr)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.Marshal::GetFunctionPointerForDelegateInternal(System.Delegate)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.Marshal::OffsetOf(System.Type,System.String)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.Marshal::ReadIntPtr(System.IntPtr,System.Int32)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.Marshal::ReAllocCoTaskMem(System.IntPtr,System.Int32)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.Marshal::ReAllocHGlobal(System.IntPtr,System.IntPtr)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.Marshal::StringToBSTR(System.String)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.Marshal::StringToHGlobalAnsi(System.String)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.Marshal::StringToHGlobalUni(System.String)

# internal call
+SC-M: System.IntPtr System.Runtime.InteropServices.Marshal::UnsafeAddrOfPinnedArrayElement(System.Array,System.Int32)

# internal call
+SC-M: System.IntPtr System.Security.Cryptography.RNGCryptoServiceProvider::RngGetBytes(System.IntPtr,System.Byte[])

# internal call
+SC-M: System.IntPtr System.Security.Cryptography.RNGCryptoServiceProvider::RngInitialize(System.Byte[])

# internal call
+SC-M: System.IntPtr System.Threading.Interlocked::CompareExchange(System.IntPtr&,System.IntPtr,System.IntPtr)

# internal call
+SC-M: System.IntPtr System.Threading.Mutex::CreateMutex_internal(System.Boolean,System.String,System.Boolean&)

# internal call
+SC-M: System.IntPtr System.Threading.NativeEventCalls::CreateEvent_internal(System.Boolean,System.Boolean,System.String,System.Boolean&)

# internal call
+SC-M: System.IntPtr System.Threading.Thread::Thread_internal(System.MulticastDelegate)

# internal call
+SC-M: System.IO.FileAttributes System.IO.MonoIO::GetFileAttributes(System.String,System.IO.MonoIOError&)

# [VISIBLE] implements 'System.IO.FileStream System.Runtime.InteropServices._Assembly::GetFile(System.String)'.
+SC-M: System.IO.FileStream System.Reflection.Assembly::GetFile(System.String)

# [VISIBLE] overrides 'System.IO.FileStream System.Reflection.Assembly::GetFile(System.String)'.
+SC-M: System.IO.FileStream System.Reflection.Emit.AssemblyBuilder::GetFile(System.String)

# Promoting interface member to [SecurityCritical] because of 'System.IO.FileStream System.Reflection.Assembly::GetFile(System.String)'.
+SC-M: System.IO.FileStream System.Runtime.InteropServices._Assembly::GetFile(System.String)

# [VISIBLE] implements 'System.IO.FileStream[] System.Runtime.InteropServices._Assembly::GetFiles(System.Boolean)'.
+SC-M: System.IO.FileStream[] System.Reflection.Assembly::GetFiles(System.Boolean)

# [VISIBLE] overrides 'System.IO.FileStream[] System.Reflection.Assembly::GetFiles(System.Boolean)'.
+SC-M: System.IO.FileStream[] System.Reflection.Emit.AssemblyBuilder::GetFiles(System.Boolean)

# Promoting interface member to [SecurityCritical] because of 'System.IO.FileStream[] System.Reflection.Assembly::GetFiles(System.Boolean)'.
+SC-M: System.IO.FileStream[] System.Runtime.InteropServices._Assembly::GetFiles(System.Boolean)

# internal call
+SC-M: System.IO.MonoFileType System.IO.MonoIO::GetFileType(System.IntPtr,System.IO.MonoIOError&)

# Promoting interface member to [SecurityCritical] because of 'System.Object System.AppDomain::GetData(System.String)'.
+SC-M: System.Object System._AppDomain::GetData(System.String)

# internal call
+SC-M: System.Object System.Activator::CreateInstanceInternal(System.Type)

# [VISIBLE] implements 'System.Object System._AppDomain::GetData(System.String)'.
+SC-M: System.Object System.AppDomain::GetData(System.String)

# internal call
+SC-M: System.Object System.Array::GetValueImpl(System.Int32)

# implements 'System.Object System.Runtime.Serialization.IObjectReference::GetRealObject(System.Runtime.Serialization.StreamingContext)'.
+SC-M: System.Object System.DelegateSerializationHolder::GetRealObject(System.Runtime.Serialization.StreamingContext)

# internal call
+SC-M: System.Object System.Enum::get_value()

# internal call
+SC-M: System.Object System.GC::get_ephemeron_tombstone()

# [VISIBLE] overrides 'System.Object System.Delegate::DynamicInvokeImpl(System.Object[])'.
+SC-M: System.Object System.MulticastDelegate::DynamicInvokeImpl(System.Object[])

# internal call
+SC-M: System.Object System.Reflection.Assembly::GetFilesInternal(System.String,System.Boolean)

# implements 'System.Object System.Runtime.Serialization.IObjectReference::GetRealObject(System.Runtime.Serialization.StreamingContext)'.
+SC-M: System.Object System.Reflection.MemberInfoSerializationHolder::GetRealObject(System.Runtime.Serialization.StreamingContext)

# internal call
+SC-M: System.Object System.Reflection.MonoCMethod::InternalInvoke(System.Object,System.Object[],System.Exception&)

# internal call
+SC-M: System.Object System.Reflection.MonoField::GetValueInternal(System.Object)

# internal call
+SC-M: System.Object System.Reflection.MonoMethod::InternalInvoke(System.Object,System.Object[],System.Exception&)

# internal call
+SC-M: System.Object System.Reflection.MonoPropertyInfo::get_default_value(System.Reflection.MonoProperty)

# internal call
+SC-M: System.Object System.Runtime.InteropServices.GCHandle::GetTarget(System.Int32)

# internal call
+SC-M: System.Object System.Runtime.Remoting.Activation.ActivationServices::AllocateUninitializedClassInstance(System.Type)

# overrides 'System.Object System.MarshalByRefObject::InitializeLifetimeService()'.
+SC-M: System.Object System.Runtime.Remoting.Activation.RemoteActivator::InitializeLifetimeService()

# implements 'System.Object System.Runtime.Remoting.Channels.IChannelReceiver::get_ChannelData()'.
+SC-M: System.Object System.Runtime.Remoting.Channels.CrossAppDomainChannel::get_ChannelData()

# Promoting interface member to [SecurityCritical] because of 'System.Object System.Runtime.Remoting.Messaging.MethodResponse::GetOutArg(System.Int32)'.
+SC-M: System.Object System.Runtime.Remoting.Messaging.IMethodReturnMessage::GetOutArg(System.Int32)

# implements 'System.Object System.Runtime.Remoting.Messaging.IMethodReturnMessage::GetOutArg(System.Int32)'.
+SC-M: System.Object System.Runtime.Remoting.Messaging.MethodResponse::GetOutArg(System.Int32)

# implements 'System.Object System.Runtime.Remoting.Messaging.IMethodReturnMessage::GetOutArg(System.Int32)'.
+SC-M: System.Object System.Runtime.Remoting.Messaging.MonoMethodMessage::GetOutArg(System.Int32)

# implements 'System.Object System.Runtime.Serialization.ISerializationSurrogate::SetObjectData(System.Object,System.Runtime.Serialization.SerializationInfo,System.Runtime.Serialization.StreamingContext,System.Runtime.Serialization.ISurrogateSelector)'.
+SC-M: System.Object System.Runtime.Remoting.Messaging.ObjRefSurrogate::SetObjectData(System.Object,System.Runtime.Serialization.SerializationInfo,System.Runtime.Serialization.StreamingContext,System.Runtime.Serialization.ISurrogateSelector)

# implements 'System.Object System.Runtime.Serialization.ISerializationSurrogate::SetObjectData(System.Object,System.Runtime.Serialization.SerializationInfo,System.Runtime.Serialization.StreamingContext,System.Runtime.Serialization.ISurrogateSelector)'.
+SC-M: System.Object System.Runtime.Remoting.Messaging.RemotingSurrogate::SetObjectData(System.Object,System.Runtime.Serialization.SerializationInfo,System.Runtime.Serialization.StreamingContext,System.Runtime.Serialization.ISurrogateSelector)

# implements 'System.Object System.Runtime.Remoting.Messaging.IMethodReturnMessage::GetOutArg(System.Int32)'.
+SC-M: System.Object System.Runtime.Remoting.Messaging.ReturnMessage::GetOutArg(System.Int32)

# implements 'System.Object System.Runtime.Serialization.IObjectReference::GetRealObject(System.Runtime.Serialization.StreamingContext)'.
+SC-M: System.Object System.Runtime.Remoting.ObjRef::GetRealObject(System.Runtime.Serialization.StreamingContext)

# internal call
+SC-M: System.Object System.Runtime.Remoting.RemotingServices::InternalExecute(System.Reflection.MethodBase,System.Object,System.Object[],System.Object[]&)

# implements 'System.Object System.Runtime.Serialization.IFormatter::Deserialize(System.IO.Stream)'.
+SC-M: System.Object System.Runtime.Serialization.Formatters.Binary.BinaryFormatter::Deserialize(System.IO.Stream)

# implements 'System.Object System.Runtime.Remoting.Messaging.IRemotingFormatter::Deserialize(System.IO.Stream,System.Runtime.Remoting.Messaging.HeaderHandler)'.
+SC-M: System.Object System.Runtime.Serialization.Formatters.Binary.BinaryFormatter::Deserialize(System.IO.Stream,System.Runtime.Remoting.Messaging.HeaderHandler)

# Promoting interface member to [SecurityCritical] because of 'System.Object System.DelegateSerializationHolder::GetRealObject(System.Runtime.Serialization.StreamingContext)'.
+SC-M: System.Object System.Runtime.Serialization.IObjectReference::GetRealObject(System.Runtime.Serialization.StreamingContext)

# implements 'System.Object System.Runtime.Serialization.IObjectReference::GetRealObject(System.Runtime.Serialization.StreamingContext)'.
+SC-M: System.Object System.UnitySerializationHolder::GetRealObject(System.Runtime.Serialization.StreamingContext)

# internal call
+SC-M: System.Object[] System.MonoCustomAttrs::GetCustomAttributesInternal(System.Reflection.ICustomAttributeProvider,System.Type,System.Boolean)

# implements 'System.Object[] System.Runtime.Remoting.IChannelInfo::get_ChannelData()'.
+SC-M: System.Object[] System.Runtime.Remoting.ChannelInfo::get_ChannelData()

# Promoting interface member to [SecurityCritical] because of 'System.Object[] System.Runtime.Remoting.Messaging.MethodResponse::get_OutArgs()'.
+SC-M: System.Object[] System.Runtime.Remoting.Messaging.IMethodReturnMessage::get_OutArgs()

# implements 'System.Object[] System.Runtime.Remoting.Messaging.IMethodReturnMessage::get_OutArgs()'.
+SC-M: System.Object[] System.Runtime.Remoting.Messaging.MethodResponse::get_OutArgs()

# implements 'System.Object[] System.Runtime.Remoting.Messaging.IMethodReturnMessage::get_OutArgs()'.
+SC-M: System.Object[] System.Runtime.Remoting.Messaging.MonoMethodMessage::get_OutArgs()

# implements 'System.Object[] System.Runtime.Remoting.Messaging.IMethodReturnMessage::get_OutArgs()'.
+SC-M: System.Object[] System.Runtime.Remoting.Messaging.ReturnMessage::get_OutArgs()

# internal call
+SC-M: System.PlatformID System.Environment::get_Platform()

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.Assembly System.AppDomain::Load(System.Byte[])'.
+SC-M: System.Reflection.Assembly System._AppDomain::Load(System.Byte[])

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.Assembly System.AppDomain::Load(System.Byte[],System.Byte[])'.
+SC-M: System.Reflection.Assembly System._AppDomain::Load(System.Byte[],System.Byte[])

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.Assembly System.AppDomain::Load(System.Byte[],System.Byte[],System.Security.Policy.Evidence)'.
+SC-M: System.Reflection.Assembly System._AppDomain::Load(System.Byte[],System.Byte[],System.Security.Policy.Evidence)

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.Assembly System.AppDomain::Load(System.Reflection.AssemblyName)'.
+SC-M: System.Reflection.Assembly System._AppDomain::Load(System.Reflection.AssemblyName)

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.Assembly System.AppDomain::Load(System.Reflection.AssemblyName,System.Security.Policy.Evidence)'.
+SC-M: System.Reflection.Assembly System._AppDomain::Load(System.Reflection.AssemblyName,System.Security.Policy.Evidence)

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.Assembly System.AppDomain::Load(System.String)'.
+SC-M: System.Reflection.Assembly System._AppDomain::Load(System.String)

# implements 'System.Reflection.Assembly System._AppDomain::Load(System.Byte[])'.
+SC-M: System.Reflection.Assembly System.AppDomain::Load(System.Byte[])

# implements 'System.Reflection.Assembly System._AppDomain::Load(System.Byte[],System.Byte[])'.
+SC-M: System.Reflection.Assembly System.AppDomain::Load(System.Byte[],System.Byte[])

# implements 'System.Reflection.Assembly System._AppDomain::Load(System.Byte[],System.Byte[],System.Security.Policy.Evidence)'.
+SC-M: System.Reflection.Assembly System.AppDomain::Load(System.Byte[],System.Byte[],System.Security.Policy.Evidence)

# implements 'System.Reflection.Assembly System._AppDomain::Load(System.Reflection.AssemblyName)'.
+SC-M: System.Reflection.Assembly System.AppDomain::Load(System.Reflection.AssemblyName)

# implements 'System.Reflection.Assembly System._AppDomain::Load(System.Reflection.AssemblyName,System.Security.Policy.Evidence)'.
+SC-M: System.Reflection.Assembly System.AppDomain::Load(System.Reflection.AssemblyName,System.Security.Policy.Evidence)

# implements 'System.Reflection.Assembly System._AppDomain::Load(System.String)'.
+SC-M: System.Reflection.Assembly System.AppDomain::Load(System.String)

# internal call
+SC-M: System.Reflection.Assembly System.AppDomain::LoadAssembly(System.String,System.Security.Policy.Evidence,System.Boolean)

# internal call
+SC-M: System.Reflection.Assembly System.AppDomain::LoadAssemblyRaw(System.Byte[],System.Byte[],System.Security.Policy.Evidence,System.Boolean)

# internal call
+SC-M: System.Reflection.Assembly System.Reflection.Assembly::LoadFrom(System.String,System.Boolean)

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.Assembly[] System.AppDomain::GetAssemblies()'.
+SC-M: System.Reflection.Assembly[] System._AppDomain::GetAssemblies()

# implements 'System.Reflection.Assembly[] System._AppDomain::GetAssemblies()'.
+SC-M: System.Reflection.Assembly[] System.AppDomain::GetAssemblies()

# internal call
+SC-M: System.Reflection.Assembly[] System.AppDomain::GetAssemblies(System.Boolean)

# [VISIBLE] implements 'System.Reflection.AssemblyName System.Runtime.InteropServices._Assembly::GetName()'.
+SC-M: System.Reflection.AssemblyName System.Reflection.Assembly::GetName()

# [VISIBLE] implements 'System.Reflection.AssemblyName System.Runtime.InteropServices._Assembly::GetName(System.Boolean)'.
+SC-M: System.Reflection.AssemblyName System.Reflection.Assembly::GetName(System.Boolean)

# [VISIBLE] overrides 'System.Reflection.AssemblyName System.Reflection.Assembly::GetName(System.Boolean)'.
+SC-M: System.Reflection.AssemblyName System.Reflection.Emit.AssemblyBuilder::GetName(System.Boolean)

# overrides 'System.Reflection.AssemblyName System.Reflection.Assembly::UnprotectedGetName()'.
+SC-M: System.Reflection.AssemblyName System.Reflection.Emit.AssemblyBuilder::UnprotectedGetName()

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.AssemblyName System.Reflection.Assembly::GetName()'.
+SC-M: System.Reflection.AssemblyName System.Runtime.InteropServices._Assembly::GetName()

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.AssemblyName System.Reflection.Assembly::GetName(System.Boolean)'.
+SC-M: System.Reflection.AssemblyName System.Runtime.InteropServices._Assembly::GetName(System.Boolean)

# internal call
+SC-M: System.Reflection.AssemblyName[] System.Reflection.Assembly::GetReferencedAssemblies(System.Reflection.Assembly)

# internal call
+SC-M: System.Reflection.ConstructorInfo System.MonoType::GetCorrespondingInflatedConstructor(System.Reflection.ConstructorInfo)

# internal call
+SC-M: System.Reflection.ConstructorInfo[] System.MonoType::GetConstructors_internal(System.Reflection.BindingFlags,System.Type)

# internal call
+SC-M: System.Reflection.CustomAttributeData[] System.MonoCustomAttrs::GetCustomAttributesDataInternal(System.Reflection.ICustomAttributeProvider)

# overrides 'System.Reflection.Emit.UnmanagedMarshal System.Reflection.FieldInfo::get_UMarshal()'.
+SC-M: System.Reflection.Emit.UnmanagedMarshal System.Reflection.Emit.FieldBuilder::get_UMarshal()

# internal call
+SC-M: System.Reflection.Emit.UnmanagedMarshal System.Reflection.FieldInfo::GetUnmanagedMarshal()

# internal call
+SC-M: System.Reflection.Emit.UnmanagedMarshal System.Reflection.MonoMethodInfo::get_retval_marshal(System.IntPtr)

# internal call
+SC-M: System.Reflection.EventInfo System.MonoType::InternalGetEvent(System.String,System.Reflection.BindingFlags)

# internal call
+SC-M: System.Reflection.EventInfo System.Reflection.Emit.TypeBuilder::get_event_info(System.Reflection.Emit.EventBuilder)

# internal call
+SC-M: System.Reflection.EventInfo[] System.MonoType::GetEvents_internal(System.Reflection.BindingFlags,System.Type)

# overrides 'System.Reflection.FieldInfo System.Reflection.Module::ResolveField(System.Int32,System.Type[],System.Type[])'.
+SC-M: System.Reflection.FieldInfo System.Reflection.Emit.ModuleBuilder::ResolveField(System.Int32,System.Type[],System.Type[])

# internal call
+SC-M: System.Reflection.FieldInfo System.Reflection.FieldInfo::internal_from_handle_type(System.IntPtr,System.IntPtr)

# overrides 'System.Reflection.FieldInfo System.Reflection.Module::ResolveField(System.Int32,System.Type[],System.Type[])'.
+SC-M: System.Reflection.FieldInfo System.Reflection.MonoModule::ResolveField(System.Int32,System.Type[],System.Type[])

# internal call
+SC-M: System.Reflection.FieldInfo[] System.MonoType::GetFields_internal(System.Reflection.BindingFlags,System.Type)

# internal call
+SC-M: System.Reflection.GenericParameterAttributes System.Type::GetGenericParameterAttributes()

# implements 'System.Reflection.ManifestResourceInfo System.Runtime.InteropServices._Assembly::GetManifestResourceInfo(System.String)'.
+SC-M: System.Reflection.ManifestResourceInfo System.Reflection.Assembly::GetManifestResourceInfo(System.String)

# overrides 'System.Reflection.ManifestResourceInfo System.Reflection.Assembly::GetManifestResourceInfo(System.String)'.
+SC-M: System.Reflection.ManifestResourceInfo System.Reflection.Emit.AssemblyBuilder::GetManifestResourceInfo(System.String)

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.ManifestResourceInfo System.Reflection.Assembly::GetManifestResourceInfo(System.String)'.
+SC-M: System.Reflection.ManifestResourceInfo System.Runtime.InteropServices._Assembly::GetManifestResourceInfo(System.String)

# overrides 'System.Reflection.MemberInfo System.Reflection.Module::ResolveMember(System.Int32,System.Type[],System.Type[])'.
+SC-M: System.Reflection.MemberInfo System.Reflection.Emit.ModuleBuilder::ResolveMember(System.Int32,System.Type[],System.Type[])

# internal call
+SC-M: System.Reflection.MemberInfo System.Reflection.Module::ResolveMemberToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)

# overrides 'System.Reflection.MemberInfo System.Reflection.Module::ResolveMember(System.Int32,System.Type[],System.Type[])'.
+SC-M: System.Reflection.MemberInfo System.Reflection.MonoModule::ResolveMember(System.Int32,System.Type[],System.Type[])

# implements 'System.Reflection.MethodBase System.Runtime.InteropServices._Exception::get_TargetSite()'.
+SC-M: System.Reflection.MethodBase System.Exception::get_TargetSite()

# internal call
+SC-M: System.Reflection.MethodBase System.Reflection.MethodBase::GetMethodFromHandleInternalType(System.IntPtr,System.IntPtr)

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.MethodBase System.Exception::get_TargetSite()'.
+SC-M: System.Reflection.MethodBase System.Runtime.InteropServices._Exception::get_TargetSite()

# implements 'System.Reflection.MethodBase System.Runtime.Remoting.Messaging.IMethodMessage::get_MethodBase()'.
+SC-M: System.Reflection.MethodBase System.Runtime.Remoting.Messaging.ErrorMessage::get_MethodBase()

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.MethodBase System.Runtime.Remoting.Messaging.ErrorMessage::get_MethodBase()'.
+SC-M: System.Reflection.MethodBase System.Runtime.Remoting.Messaging.IMethodMessage::get_MethodBase()

# implements 'System.Reflection.MethodBase System.Runtime.Remoting.Messaging.IMethodMessage::get_MethodBase()'.
+SC-M: System.Reflection.MethodBase System.Runtime.Remoting.Messaging.MethodCall::get_MethodBase()

# implements 'System.Reflection.MethodBase System.Runtime.Remoting.Messaging.IMethodMessage::get_MethodBase()'.
+SC-M: System.Reflection.MethodBase System.Runtime.Remoting.Messaging.MethodResponse::get_MethodBase()

# implements 'System.Reflection.MethodBase System.Runtime.Remoting.Messaging.IMethodMessage::get_MethodBase()'.
+SC-M: System.Reflection.MethodBase System.Runtime.Remoting.Messaging.MonoMethodMessage::get_MethodBase()

# implements 'System.Reflection.MethodBase System.Runtime.Remoting.Messaging.IMethodMessage::get_MethodBase()'.
+SC-M: System.Reflection.MethodBase System.Runtime.Remoting.Messaging.ReturnMessage::get_MethodBase()

# internal call
+SC-M: System.Reflection.MethodBase System.Runtime.Remoting.RemotingServices::GetVirtualMethod(System.Type,System.Reflection.MethodBase)

# internal call
+SC-M: System.Reflection.MethodInfo System.MonoType::GetCorrespondingInflatedMethod(System.Reflection.MethodInfo)

# internal call
+SC-M: System.Reflection.MethodInfo System.Reflection.MonoMethod::GetGenericMethodDefinition_impl()

# internal call
+SC-M: System.Reflection.MethodInfo System.Reflection.MonoMethod::MakeGenericMethod_impl(System.Type[])

# internal call
+SC-M: System.Reflection.MethodInfo[] System.MonoType::GetMethodsByName(System.String,System.Reflection.BindingFlags,System.Boolean,System.Type)

# internal call
+SC-M: System.Reflection.Module System.Reflection.Assembly::GetManifestModuleInternal()

# internal call
+SC-M: System.Reflection.Module System.Reflection.Emit.AssemblyBuilder::InternalAddModule(System.String)

# internal call
+SC-M: System.Reflection.MonoMethod System.Reflection.MonoMethod::get_base_method(System.Reflection.MonoMethod,System.Boolean)

# internal call
+SC-M: System.Reflection.ParameterInfo[] System.Reflection.MonoMethodInfo::get_parameter_info(System.IntPtr,System.Reflection.MemberInfo)

# internal call
+SC-M: System.Reflection.PropertyInfo[] System.MonoType::GetPropertiesByName(System.String,System.Reflection.BindingFlags,System.Boolean,System.Type)

# internal call
+SC-M: System.Reflection.TypeAttributes System.MonoType::get_attributes(System.Type)

# internal call
+SC-M: System.Runtime.InteropServices.DllImportAttribute System.Reflection.MonoMethod::GetDllImportAttribute(System.IntPtr)

# implements 'System.Runtime.Remoting.Activation.IConstructionReturnMessage System.Runtime.Remoting.Activation.IActivator::Activate(System.Runtime.Remoting.Activation.IConstructionCallMessage)'.
+SC-M: System.Runtime.Remoting.Activation.IConstructionReturnMessage System.Runtime.Remoting.Activation.RemoteActivator::Activate(System.Runtime.Remoting.Activation.IConstructionCallMessage)

# internal call
+SC-M: System.Runtime.Remoting.Contexts.Context System.AppDomain::InternalGetContext()

# internal call
+SC-M: System.Runtime.Remoting.Contexts.Context System.AppDomain::InternalGetDefaultContext()

# internal call
+SC-M: System.Runtime.Remoting.Contexts.Context System.AppDomain::InternalSetContext(System.Runtime.Remoting.Contexts.Context)

# implements 'System.Runtime.Remoting.Lifetime.LeaseState System.Runtime.Remoting.Lifetime.ILease::get_CurrentState()'.
+SC-M: System.Runtime.Remoting.Lifetime.LeaseState System.Runtime.Remoting.Lifetime.Lease::get_CurrentState()

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Channels.CrossAppDomainSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# overrides 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.ServerIdentity::SyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.ClientActivatedIdentity::SyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Contexts.CrossContextChannel/ContextRestoreSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Contexts.CrossContextChannel::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Contexts.SynchronizedClientContextSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Contexts.SynchronizedContextReplySink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Contexts.SynchronizedServerContextSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.DisposerReplySink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Lifetime.LeaseSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.AsyncResult::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.ClientContextReplySink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.ClientContextTerminatorSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.EnvoyTerminatorSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# Promoting interface member to [SecurityCritical] because of 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.AsyncResult::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.ServerContextTerminatorSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.ServerObjectReplySink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.ServerObjectTerminatorSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.IMessageSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Messaging.StackBuilderSink::SyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# overrides 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.ServerIdentity::SyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.SingleCallIdentity::SyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# overrides 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.ServerIdentity::SyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.SingletonIdentity::SyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Channels.CrossAppDomainSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# overrides 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.ServerIdentity::AsyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.ClientActivatedIdentity::AsyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Contexts.CrossContextChannel/ContextRestoreSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Contexts.CrossContextChannel::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Contexts.SynchronizedClientContextSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Contexts.SynchronizedContextReplySink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Contexts.SynchronizedServerContextSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.DisposerReplySink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Lifetime.LeaseSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.AsyncResult::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.ClientContextReplySink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.ClientContextTerminatorSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.EnvoyTerminatorSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# Promoting interface member to [SecurityCritical] because of 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.AsyncResult::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.ServerContextTerminatorSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.ServerObjectReplySink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.ServerObjectTerminatorSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.IMessageSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.Messaging.StackBuilderSink::AsyncProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# overrides 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.ServerIdentity::AsyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.SingleCallIdentity::AsyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# overrides 'System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.ServerIdentity::AsyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageCtrl System.Runtime.Remoting.SingletonIdentity::AsyncObjectProcessMessage(System.Runtime.Remoting.Messaging.IMessage,System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Channels.IChannelSender::CreateMessageSink(System.String,System.Object,System.String&)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Channels.CrossAppDomainChannel::CreateMessageSink(System.String,System.Object,System.String&)

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Channels.CrossAppDomainSink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Contexts.CrossContextChannel/ContextRestoreSink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Contexts.CrossContextChannel::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Contexts.IContributeClientContextSink::GetClientContextSink(System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Contexts.SynchronizationAttribute::GetClientContextSink(System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Contexts.IContributeServerContextSink::GetServerContextSink(System.Runtime.Remoting.Messaging.IMessageSink)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Contexts.SynchronizationAttribute::GetServerContextSink(System.Runtime.Remoting.Messaging.IMessageSink)

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Contexts.SynchronizedClientContextSink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Contexts.SynchronizedContextReplySink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Contexts.SynchronizedServerContextSink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.DisposerReplySink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Lifetime.LeaseSink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.AsyncResult::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.ClientContextReplySink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.ClientContextTerminatorSink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.EnvoyTerminatorSink::get_NextSink()

# Promoting interface member to [SecurityCritical] because of 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.AsyncResult::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.ServerContextTerminatorSink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.ServerObjectReplySink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.ServerObjectTerminatorSink::get_NextSink()

# implements 'System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.IMessageSink::get_NextSink()'.
+SC-M: System.Runtime.Remoting.Messaging.IMessageSink System.Runtime.Remoting.Messaging.StackBuilderSink::get_NextSink()

# overrides 'System.Runtime.Remoting.ObjRef System.Runtime.Remoting.Identity::CreateObjRef(System.Type)'.
+SC-M: System.Runtime.Remoting.ObjRef System.Runtime.Remoting.ClientIdentity::CreateObjRef(System.Type)

# overrides 'System.Runtime.Remoting.ObjRef System.Runtime.Remoting.Identity::CreateObjRef(System.Type)'.
+SC-M: System.Runtime.Remoting.ObjRef System.Runtime.Remoting.ServerIdentity::CreateObjRef(System.Type)

# implements 'System.Runtime.Serialization.ISerializationSurrogate System.Runtime.Serialization.ISurrogateSelector::GetSurrogate(System.Type,System.Runtime.Serialization.StreamingContext,System.Runtime.Serialization.ISurrogateSelector&)'.
+SC-M: System.Runtime.Serialization.ISerializationSurrogate System.Runtime.Remoting.Messaging.RemotingSurrogateSelector::GetSurrogate(System.Type,System.Runtime.Serialization.StreamingContext,System.Runtime.Serialization.ISurrogateSelector&)

# implements 'System.Runtime.Serialization.ISurrogateSelector System.Runtime.Serialization.IFormatter::get_SurrogateSelector()'.
+SC-M: System.Runtime.Serialization.ISurrogateSelector System.Runtime.Serialization.Formatters.Binary.BinaryFormatter::get_SurrogateSelector()

# implements 'System.Runtime.Serialization.SerializationBinder System.Runtime.Serialization.IFormatter::get_Binder()'.
+SC-M: System.Runtime.Serialization.SerializationBinder System.Runtime.Serialization.Formatters.Binary.BinaryFormatter::get_Binder()

# implements 'System.Runtime.Serialization.StreamingContext System.Runtime.Serialization.IFormatter::get_Context()'.
+SC-M: System.Runtime.Serialization.StreamingContext System.Runtime.Serialization.Formatters.Binary.BinaryFormatter::get_Context()

# internal call
+SC-M: System.Single System.Threading.Interlocked::CompareExchange(System.Single&,System.Single,System.Single)

# internal call
+SC-M: System.String System.AppDomain::getFriendlyName()

# internal call
+SC-M: System.String System.AppDomain::InternalGetProcessGuid(System.String)

# internal call
+SC-M: System.String System.Environment::get_MachineName()

# internal call
+SC-M: System.String System.Environment::get_UserName()

# internal call
+SC-M: System.String System.Environment::GetMachineConfigPath()

# internal call
+SC-M: System.String System.Environment::GetNewLine()

# internal call
+SC-M: System.String System.Environment::GetOSVersionString()

# internal call
+SC-M: System.String System.Environment::GetWindowsFolderPath(System.Int32)

# internal call
+SC-M: System.String System.Environment::internalGetEnvironmentVariable(System.String)

# internal call
+SC-M: System.String System.Environment::internalGetHome()

# implements 'System.String System.Runtime.InteropServices._Exception::get_Source()'.
+SC-M: System.String System.Exception::get_Source()

# internal call
+SC-M: System.String System.IO.MonoIO::FindFirst(System.String,System.String,System.IO.FileAttributes&,System.IO.MonoIOError&,System.IntPtr&)

# internal call
+SC-M: System.String System.IO.MonoIO::FindNext(System.IntPtr,System.IO.FileAttributes&,System.IO.MonoIOError&)

# internal call
+SC-M: System.String System.IO.MonoIO::GetCurrentDirectory(System.IO.MonoIOError&)

# internal call
+SC-M: System.String System.IO.Path::get_temp_path()

# internal call
+SC-M: System.String System.MonoType::getFullName(System.Boolean,System.Boolean)

# internal call
+SC-M: System.String System.Reflection.Assembly::get_code_base(System.Boolean)

# [VISIBLE] implements 'System.String System.Runtime.InteropServices._Assembly::get_CodeBase()'.
+SC-M: System.String System.Reflection.Assembly::get_CodeBase()

# internal call
+SC-M: System.String System.Reflection.Assembly::get_fullname()

# internal call
+SC-M: System.String System.Reflection.Assembly::get_location()

# [VISIBLE] implements 'System.String System.Runtime.InteropServices._Assembly::get_Location()'.
+SC-M: System.String System.Reflection.Assembly::get_Location()

# internal call
+SC-M: System.String System.Reflection.Assembly::InternalImageRuntimeVersion()

# [VISIBLE] overrides 'System.String System.Reflection.Assembly::get_CodeBase()'.
+SC-M: System.String System.Reflection.Emit.AssemblyBuilder::get_CodeBase()

# [VISIBLE] overrides 'System.String System.Reflection.Assembly::get_Location()'.
+SC-M: System.String System.Reflection.Emit.AssemblyBuilder::get_Location()

# [VISIBLE] overrides 'System.String System.Reflection.Module::get_FullyQualifiedName()'.
+SC-M: System.String System.Reflection.Emit.ModuleBuilder::get_FullyQualifiedName()

# internal call
+SC-M: System.String System.Reflection.Module::GetGuidInternal()

# internal call
+SC-M: System.String System.Reflection.Module::ResolveStringToken(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)

# internal call
+SC-M: System.String System.Reflection.MonoMethod::get_name(System.Reflection.MethodBase)

# overrides 'System.String System.Reflection.Module::get_FullyQualifiedName()'.
+SC-M: System.String System.Reflection.MonoModule::get_FullyQualifiedName()

# Promoting interface member to [SecurityCritical] because of 'System.String System.Reflection.Assembly::get_CodeBase()'.
+SC-M: System.String System.Runtime.InteropServices._Assembly::get_CodeBase()

# Promoting interface member to [SecurityCritical] because of 'System.String System.Reflection.Assembly::get_Location()'.
+SC-M: System.String System.Runtime.InteropServices._Assembly::get_Location()

# Promoting interface member to [SecurityCritical] because of 'System.String System.Exception::get_Source()'.
+SC-M: System.String System.Runtime.InteropServices._Exception::get_Source()

# internal call
+SC-M: System.String System.Runtime.InteropServices.Marshal::PtrToStringBSTR(System.IntPtr)

# implements 'System.String System.Runtime.Remoting.Channels.IChannel::get_ChannelName()'.
+SC-M: System.String System.Runtime.Remoting.Channels.CrossAppDomainChannel::get_ChannelName()

# implements 'System.String System.Runtime.Remoting.Contexts.IContextProperty::get_Name()'.
+SC-M: System.String System.Runtime.Remoting.Contexts.ContextAttribute::get_Name()

# implements 'System.String System.Runtime.Remoting.Messaging.IMethodMessage::GetArgName(System.Int32)'.
+SC-M: System.String System.Runtime.Remoting.Messaging.ErrorMessage::GetArgName(System.Int32)

# Promoting interface member to [SecurityCritical] because of 'System.String System.Runtime.Remoting.Messaging.ErrorMessage::GetArgName(System.Int32)'.
+SC-M: System.String System.Runtime.Remoting.Messaging.IMethodMessage::GetArgName(System.Int32)

# Promoting interface member to [SecurityCritical] because of 'System.String System.Runtime.Remoting.Messaging.MethodResponse::GetOutArgName(System.Int32)'.
+SC-M: System.String System.Runtime.Remoting.Messaging.IMethodReturnMessage::GetOutArgName(System.Int32)

# implements 'System.String System.Runtime.Remoting.Messaging.IMethodMessage::GetArgName(System.Int32)'.
+SC-M: System.String System.Runtime.Remoting.Messaging.MethodCall::GetArgName(System.Int32)

# implements 'System.String System.Runtime.Remoting.Messaging.IMethodMessage::GetArgName(System.Int32)'.
+SC-M: System.String System.Runtime.Remoting.Messaging.MethodResponse::GetArgName(System.Int32)

# implements 'System.String System.Runtime.Remoting.Messaging.IMethodReturnMessage::GetOutArgName(System.Int32)'.
+SC-M: System.String System.Runtime.Remoting.Messaging.MethodResponse::GetOutArgName(System.Int32)

# implements 'System.String System.Runtime.Remoting.Messaging.IMethodMessage::GetArgName(System.Int32)'.
+SC-M: System.String System.Runtime.Remoting.Messaging.MonoMethodMessage::GetArgName(System.Int32)

# implements 'System.String System.Runtime.Remoting.Messaging.IMethodReturnMessage::GetOutArgName(System.Int32)'.
+SC-M: System.String System.Runtime.Remoting.Messaging.MonoMethodMessage::GetOutArgName(System.Int32)

# implements 'System.String System.Runtime.Remoting.Messaging.IMethodMessage::GetArgName(System.Int32)'.
+SC-M: System.String System.Runtime.Remoting.Messaging.ReturnMessage::GetArgName(System.Int32)

# implements 'System.String System.Runtime.Remoting.Messaging.IMethodReturnMessage::GetOutArgName(System.Int32)'.
+SC-M: System.String System.Runtime.Remoting.Messaging.ReturnMessage::GetOutArgName(System.Int32)

# overrides 'System.String System.Runtime.Remoting.Metadata.SoapAttribute::get_XmlNamespace()'.
+SC-M: System.String System.Runtime.Remoting.Metadata.SoapMethodAttribute::get_XmlNamespace()

# overrides 'System.String System.Runtime.Remoting.Metadata.SoapAttribute::get_XmlNamespace()'.
+SC-M: System.String System.Runtime.Remoting.Metadata.SoapTypeAttribute::get_XmlNamespace()

# using 'System.Char*' as a parameter type
+SC-M: System.String System.String::CreateString(System.Char*)

# using 'System.Char*' as a parameter type
+SC-M: System.String System.String::CreateString(System.Char*,System.Int32,System.Int32)

# using 'System.SByte*' as a parameter type
+SC-M: System.String System.String::CreateString(System.SByte*)

# using 'System.SByte*' as a parameter type
+SC-M: System.String System.String::CreateString(System.SByte*,System.Int32,System.Int32)

# using 'System.SByte*' as a parameter type
+SC-M: System.String System.String::CreateString(System.SByte*,System.Int32,System.Int32,System.Text.Encoding)

# internal call
+SC-M: System.String System.String::InternalAllocateStr(System.Int32)

# internal call
+SC-M: System.String System.String::InternalIntern(System.String)

# internal call
+SC-M: System.String System.String::InternalIsInterned(System.String)

# localloc
+SC-M: System.String System.String::ReplaceUnchecked(System.String,System.String)

# internal call
+SC-M: System.String System.Text.Encoding::InternalCodePage(System.Int32&)

# internal call
+SC-M: System.String System.Threading.Thread::GetName_internal(System.Threading.InternalThread)

# internal call
+SC-M: System.String[] System.IO.MonoIO::GetFileSystemEntries(System.String,System.String,System.Int32,System.Int32,System.IO.MonoIOError&)

# internal call
+SC-M: System.String[] System.Reflection.Assembly::GetNamespaces()

# internal call
+SC-M: System.String[] System.String::InternalSplit(System.Char[],System.Int32,System.Int32)

# internal call
+SC-M: System.Threading.InternalThread System.Threading.Thread::CurrentInternalThread_internal()

# internal call
+SC-M: System.Threading.ThreadState System.Threading.Thread::GetState(System.Threading.InternalThread)

# implements 'System.TimeSpan System.Runtime.Remoting.Lifetime.ILease::get_CurrentLeaseTime()'.
+SC-M: System.TimeSpan System.Runtime.Remoting.Lifetime.Lease::get_CurrentLeaseTime()

# implements 'System.TimeSpan System.Runtime.Remoting.Lifetime.ILease::get_RenewOnCallTime()'.
+SC-M: System.TimeSpan System.Runtime.Remoting.Lifetime.Lease::get_RenewOnCallTime()

# implements 'System.TimeSpan System.Runtime.Remoting.Lifetime.ILease::Renew(System.TimeSpan)'.
+SC-M: System.TimeSpan System.Runtime.Remoting.Lifetime.Lease::Renew(System.TimeSpan)

# internal call
+SC-M: System.Type System.Enum::get_underlying_type(System.Type)

# internal call
+SC-M: System.Type System.Reflection.Assembly::InternalGetType(System.Reflection.Module,System.String,System.Boolean,System.Boolean)

# internal call
+SC-M: System.Type System.Reflection.Emit.ModuleBuilder::create_modified_type(System.Reflection.Emit.TypeBuilder,System.String)

# internal call
+SC-M: System.Type System.Reflection.Emit.TypeBuilder::create_runtime_class(System.Reflection.Emit.TypeBuilder)

# internal call
+SC-M: System.Type System.Reflection.Module::GetGlobalType()

# internal call
+SC-M: System.Type System.Reflection.MonoField::GetParentType(System.Boolean)

# internal call
+SC-M: System.Type System.Reflection.MonoField::ResolveType()

# internal call
+SC-M: System.Type System.Runtime.Remoting.Proxies.RealProxy::InternalGetProxyType(System.Object)

# internal call
+SC-M: System.Type System.Type::GetGenericTypeDefinition_impl()

# internal call
+SC-M: System.Type System.Type::internal_from_handle(System.IntPtr)

# internal call
+SC-M: System.Type System.Type::internal_from_name(System.String,System.Boolean,System.Boolean)

# internal call
+SC-M: System.Type System.Type::make_array_type(System.Int32)

# internal call
+SC-M: System.Type System.Type::make_byref_type()

# internal call
+SC-M: System.Type System.Type::MakeGenericType(System.Type,System.Type[])

# internal call
+SC-M: System.Type System.Type::MakePointerType(System.Type)

# internal call
+SC-M: System.Type[] System.Reflection.Module::InternalGetTypes()

# internal call
+SC-M: System.Type[] System.Type::GetGenericParameterConstraints_impl()

# internal call
+SC-M: System.TypeCode System.Type::GetTypeCodeInternal(System.Type)

# internal call
+SC-M: System.TypedReference System.ArgIterator::IntGetNextArg()

# internal call
+SC-M: System.TypedReference System.ArgIterator::IntGetNextArg(System.IntPtr)

# using 'System.Byte*' as a parameter type
+SC-M: System.UInt32 Mono.Globalization.Unicode.MSCompatUnicodeTable::UInt32FromBytePtr(System.Byte*,System.UInt32)

# implements 'System.Void System.Diagnostics.SymbolStore.ISymbolWriter::Initialize(System.IntPtr,System.String,System.Boolean)'.
+SC-M: System.Void Mono.CompilerServices.SymbolWriter.SymbolWriterImpl::Initialize(System.IntPtr,System.String,System.Boolean)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void Mono.Globalization.Unicode.SimpleCollator/Context::.ctor(System.Globalization.CompareOptions,System.Byte*,System.Byte*,System.Byte*,System.Byte*,System.Byte*,System.Boolean)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void Mono.Globalization.Unicode.SimpleCollator::ClearBuffer(System.Byte*,System.Int32)

# localloc
+SC-M: System.Void Mono.Globalization.Unicode.SimpleCollator::GetSortKey(System.String,System.Int32,System.Int32,Mono.Globalization.Unicode.SortKeyBuffer,System.Globalization.CompareOptions)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void Mono.Security.BitConverterLE::UIntFromBytes(System.Byte*,System.Byte[],System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void Mono.Security.BitConverterLE::ULongFromBytes(System.Byte*,System.Byte[],System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void Mono.Security.BitConverterLE::UShortFromBytes(System.Byte*,System.Byte[],System.Int32)

# Promoting interface member to [SecurityCritical] because of 'System.Void System.AppDomain::add_AssemblyResolve(System.ResolveEventHandler)'.
+SC-M: System.Void System._AppDomain::add_AssemblyResolve(System.ResolveEventHandler)

# Promoting interface member to [SecurityCritical] because of 'System.Void System.AppDomain::add_UnhandledException(System.UnhandledExceptionEventHandler)'.
+SC-M: System.Void System._AppDomain::add_UnhandledException(System.UnhandledExceptionEventHandler)

# Promoting interface member to [SecurityCritical] because of 'System.Void System.AppDomain::remove_AssemblyResolve(System.ResolveEventHandler)'.
+SC-M: System.Void System._AppDomain::remove_AssemblyResolve(System.ResolveEventHandler)

# Promoting interface member to [SecurityCritical] because of 'System.Void System.AppDomain::remove_UnhandledException(System.UnhandledExceptionEventHandler)'.
+SC-M: System.Void System._AppDomain::remove_UnhandledException(System.UnhandledExceptionEventHandler)

# Promoting interface member to [SecurityCritical] because of 'System.Void System.AppDomain::SetData(System.String,System.Object)'.
+SC-M: System.Void System._AppDomain::SetData(System.String,System.Object)

# [VISIBLE] implements 'System.Void System._AppDomain::add_AssemblyResolve(System.ResolveEventHandler)'.
+SC-M: System.Void System.AppDomain::add_AssemblyResolve(System.ResolveEventHandler)

# [VISIBLE] implements 'System.Void System._AppDomain::add_UnhandledException(System.UnhandledExceptionEventHandler)'.
+SC-M: System.Void System.AppDomain::add_UnhandledException(System.UnhandledExceptionEventHandler)

# internal call
+SC-M: System.Void System.AppDomain::InternalPopDomainRef()

# internal call
+SC-M: System.Void System.AppDomain::InternalPushDomainRefByID(System.Int32)

# [VISIBLE] implements 'System.Void System._AppDomain::remove_AssemblyResolve(System.ResolveEventHandler)'.
+SC-M: System.Void System.AppDomain::remove_AssemblyResolve(System.ResolveEventHandler)

# [VISIBLE] implements 'System.Void System._AppDomain::remove_UnhandledException(System.UnhandledExceptionEventHandler)'.
+SC-M: System.Void System.AppDomain::remove_UnhandledException(System.UnhandledExceptionEventHandler)

# [VISIBLE] implements 'System.Void System._AppDomain::SetData(System.String,System.Object)'.
+SC-M: System.Void System.AppDomain::SetData(System.String,System.Object)

# using 'System.Void*' as a parameter type
+SC-M: System.Void System.ArgIterator::.ctor(System.RuntimeArgumentHandle,System.Void*)

# internal call
+SC-M: System.Void System.ArgIterator::Setup(System.IntPtr,System.IntPtr)

# internal call
+SC-M: System.Void System.Array::ClearInternal(System.Array,System.Int32,System.Int32)

# internal call
+SC-M: System.Void System.Array::GetGenericValueImpl<T>(System.Int32,T&)

# internal call
+SC-M: System.Void System.Array::SetGenericValueImpl<T>(System.Int32,T&)

# internal call
+SC-M: System.Void System.Array::SetValueImpl(System.Object,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.BitConverter::PutBytes(System.Byte*,System.Byte[],System.Int32,System.Int32)

# internal call
+SC-M: System.Void System.Buffer::SetByteInternal(System.Array,System.Int32,System.Int32)

# internal call
+SC-M: System.Void System.Char::GetDataTablePointers(System.Byte*&,System.Byte*&,System.Double*&,System.UInt16*&,System.UInt16*&,System.UInt16*&,System.UInt16*&)

# localloc
+SC-M: System.Void System.DateTimeUtils::ZeroPad(System.Text.StringBuilder,System.Int32,System.Int32)

# internal call
+SC-M: System.Void System.Decimal::decimalFloorAndTrunc(System.Decimal&,System.Int32)

# internal call
+SC-M: System.Void System.Delegate::SetMulticastInvoke()

# Promoting interface member to [SecurityCritical] because of 'System.Void Mono.CompilerServices.SymbolWriter.SymbolWriterImpl::Initialize(System.IntPtr,System.String,System.Boolean)'.
+SC-M: System.Void System.Diagnostics.SymbolStore.ISymbolWriter::Initialize(System.IntPtr,System.String,System.Boolean)

# internal call
+SC-M: System.Void System.Environment::Exit(System.Int32)

# internal call
+SC-M: System.Void System.GC::InternalCollect(System.Int32)

# internal call
+SC-M: System.Void System.GC::RecordPressure(System.Int64)

# internal call
+SC-M: System.Void System.GC::register_ephemeron_array(System.Runtime.CompilerServices.Ephemeron[])

# internal call
+SC-M: System.Void System.Globalization.CultureInfo::construct_datetime_format()

# internal call
+SC-M: System.Void System.Globalization.CultureInfo::construct_number_format()

# using 'System.Void*' as a parameter type
+SC-M: System.Void System.Globalization.TextInfo::.ctor(System.Globalization.CultureInfo,System.Int32,System.Void*,System.Boolean)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.IO.UnmanagedMemoryStream::.ctor(System.Byte*,System.Int64)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.IO.UnmanagedMemoryStream::.ctor(System.Byte*,System.Int64,System.Int64,System.IO.FileAccess)

# internal call
+SC-M: System.Void System.MonoEnumInfo::get_enum_info(System.Type,System.MonoEnumInfo&)

# internal call
+SC-M: System.Void System.NumberFormatter::GetFormatterTables(System.UInt64*&,System.Int32*&,System.Char*&,System.Char*&,System.Int64*&,System.Int32*&)

# internal call
+SC-M: System.Void System.Reflection.Assembly::FillName(System.Reflection.Assembly,System.Reflection.AssemblyName)

# internal call
+SC-M: System.Void System.Reflection.Emit.AssemblyBuilder::basic_init(System.Reflection.Emit.AssemblyBuilder)

# internal call
+SC-M: System.Void System.Reflection.Emit.DerivedType::create_unmanaged_type(System.Type)

# internal call
+SC-M: System.Void System.Reflection.Emit.DynamicMethod::create_dynamic_method(System.Reflection.Emit.DynamicMethod)

# internal call
+SC-M: System.Void System.Reflection.Emit.EnumBuilder::setup_enum_type(System.Type)

# internal call
+SC-M: System.Void System.Reflection.Emit.GenericTypeParameterBuilder::initialize()

# internal call
+SC-M: System.Void System.Reflection.Emit.ModuleBuilder::basic_init(System.Reflection.Emit.ModuleBuilder)

# internal call
+SC-M: System.Void System.Reflection.Emit.ModuleBuilder::RegisterToken(System.Object,System.Int32)

# internal call
+SC-M: System.Void System.Reflection.Emit.ModuleBuilder::set_wrappers_type(System.Reflection.Emit.ModuleBuilder,System.Type)

# internal call
+SC-M: System.Void System.Reflection.Emit.TypeBuilder::create_generic_class()

# internal call
+SC-M: System.Void System.Reflection.Emit.TypeBuilder::create_internal_class(System.Reflection.Emit.TypeBuilder)

# internal call
+SC-M: System.Void System.Reflection.Emit.TypeBuilder::setup_generic_class()

# internal call
+SC-M: System.Void System.Reflection.Emit.TypeBuilder::setup_internal_class(System.Reflection.Emit.TypeBuilder)

# internal call
+SC-M: System.Void System.Reflection.Module::GetPEKind(System.IntPtr,System.Reflection.PortableExecutableKinds&,System.Reflection.ImageFileMachine&)

# internal call
+SC-M: System.Void System.Reflection.MonoEventInfo::get_event_info(System.Reflection.MonoEvent,System.Reflection.MonoEventInfo&)

# internal call
+SC-M: System.Void System.Reflection.MonoField::SetValueInternal(System.Reflection.FieldInfo,System.Object,System.Object)

# internal call
+SC-M: System.Void System.Reflection.MonoGenericClass::initialize(System.Reflection.MethodInfo[],System.Reflection.ConstructorInfo[],System.Reflection.FieldInfo[],System.Reflection.PropertyInfo[],System.Reflection.EventInfo[])

# internal call
+SC-M: System.Void System.Reflection.MonoGenericClass::register_with_runtime(System.Type)

# internal call
+SC-M: System.Void System.Reflection.MonoMethodInfo::get_method_info(System.IntPtr,System.Reflection.MonoMethodInfo&)

# overrides 'System.Void System.Reflection.Module::GetPEKind(System.Reflection.PortableExecutableKinds&,System.Reflection.ImageFileMachine&)'.
+SC-M: System.Void System.Reflection.MonoModule::GetPEKind(System.Reflection.PortableExecutableKinds&,System.Reflection.ImageFileMachine&)

# internal call
+SC-M: System.Void System.Reflection.MonoPropertyInfo::get_property_info(System.Reflection.MonoProperty,System.Reflection.MonoPropertyInfo&,System.Reflection.PInfo)

# internal call
+SC-M: System.Void System.Runtime.CompilerServices.RuntimeHelpers::InitializeArray(System.Array,System.IntPtr)

# internal call
+SC-M: System.Void System.Runtime.CompilerServices.RuntimeHelpers::RunClassConstructor(System.IntPtr)

# internal call
+SC-M: System.Void System.Runtime.InteropServices.GCHandle::FreeHandle(System.Int32)

# internal call
+SC-M: System.Void System.Runtime.InteropServices.Marshal::copy_from_unmanaged(System.IntPtr,System.Int32,System.Array,System.Int32)

# internal call
+SC-M: System.Void System.Runtime.InteropServices.Marshal::copy_to_unmanaged(System.Array,System.Int32,System.IntPtr,System.Int32)

# internal call
+SC-M: System.Void System.Runtime.InteropServices.Marshal::DestroyStructure(System.IntPtr,System.Type)

# internal call
+SC-M: System.Void System.Runtime.InteropServices.Marshal::FreeBSTR(System.IntPtr)

# internal call
+SC-M: System.Void System.Runtime.InteropServices.Marshal::FreeCoTaskMem(System.IntPtr)

# internal call
+SC-M: System.Void System.Runtime.InteropServices.Marshal::FreeHGlobal(System.IntPtr)

# internal call
+SC-M: System.Void System.Runtime.InteropServices.Marshal::WriteInt16(System.IntPtr,System.Int32,System.Char)

# internal call
+SC-M: System.Void System.Runtime.InteropServices.Marshal::WriteIntPtr(System.IntPtr,System.Int32,System.IntPtr)

# internal call
+SC-M: System.Void System.Runtime.Remoting.Activation.ActivationServices::EnableProxyActivation(System.Type,System.Boolean)

# implements 'System.Void System.Runtime.Remoting.Channels.IChannelReceiver::StartListening(System.Object)'.
+SC-M: System.Void System.Runtime.Remoting.Channels.CrossAppDomainChannel::StartListening(System.Object)

# overrides 'System.Void System.Runtime.Remoting.ServerIdentity::OnLifetimeExpired()'.
+SC-M: System.Void System.Runtime.Remoting.ClientActivatedIdentity::OnLifetimeExpired()

# implements 'System.Void Mono.Xml.SmallXmlParser/IContentHandler::OnChars(System.String)'.
+SC-M: System.Void System.Runtime.Remoting.ConfigHandler::OnChars(System.String)

# implements 'System.Void Mono.Xml.SmallXmlParser/IContentHandler::OnEndElement(System.String)'.
+SC-M: System.Void System.Runtime.Remoting.ConfigHandler::OnEndElement(System.String)

# implements 'System.Void Mono.Xml.SmallXmlParser/IContentHandler::OnEndParsing(Mono.Xml.SmallXmlParser)'.
+SC-M: System.Void System.Runtime.Remoting.ConfigHandler::OnEndParsing(Mono.Xml.SmallXmlParser)

# implements 'System.Void Mono.Xml.SmallXmlParser/IContentHandler::OnIgnorableWhitespace(System.String)'.
+SC-M: System.Void System.Runtime.Remoting.ConfigHandler::OnIgnorableWhitespace(System.String)

# implements 'System.Void Mono.Xml.SmallXmlParser/IContentHandler::OnProcessingInstruction(System.String,System.String)'.
+SC-M: System.Void System.Runtime.Remoting.ConfigHandler::OnProcessingInstruction(System.String,System.String)

# implements 'System.Void Mono.Xml.SmallXmlParser/IContentHandler::OnStartElement(System.String,Mono.Xml.SmallXmlParser/IAttrList)'.
+SC-M: System.Void System.Runtime.Remoting.ConfigHandler::OnStartElement(System.String,Mono.Xml.SmallXmlParser/IAttrList)

# implements 'System.Void Mono.Xml.SmallXmlParser/IContentHandler::OnStartParsing(Mono.Xml.SmallXmlParser)'.
+SC-M: System.Void System.Runtime.Remoting.ConfigHandler::OnStartParsing(Mono.Xml.SmallXmlParser)

# implements 'System.Void System.Runtime.Remoting.Lifetime.ILease::set_InitialLeaseTime(System.TimeSpan)'.
+SC-M: System.Void System.Runtime.Remoting.Lifetime.Lease::set_InitialLeaseTime(System.TimeSpan)

# implements 'System.Void System.Runtime.Remoting.Lifetime.ILease::set_RenewOnCallTime(System.TimeSpan)'.
+SC-M: System.Void System.Runtime.Remoting.Lifetime.Lease::set_RenewOnCallTime(System.TimeSpan)

# implements 'System.Void System.Runtime.Remoting.Lifetime.ILease::set_SponsorshipTimeout(System.TimeSpan)'.
+SC-M: System.Void System.Runtime.Remoting.Lifetime.Lease::set_SponsorshipTimeout(System.TimeSpan)

# implements 'System.Void System.Runtime.Remoting.Lifetime.ILease::Unregister(System.Runtime.Remoting.Lifetime.ISponsor)'.
+SC-M: System.Void System.Runtime.Remoting.Lifetime.Lease::Unregister(System.Runtime.Remoting.Lifetime.ISponsor)

# overrides 'System.Void System.Runtime.Remoting.Messaging.MethodCall::InitMethodProperty(System.String,System.Object)'.
+SC-M: System.Void System.Runtime.Remoting.Messaging.ConstructionCall::InitMethodProperty(System.String,System.Object)

# implements 'System.Void System.Runtime.Serialization.ISerializationSurrogate::GetObjectData(System.Object,System.Runtime.Serialization.SerializationInfo,System.Runtime.Serialization.StreamingContext)'.
+SC-M: System.Void System.Runtime.Remoting.Messaging.ObjRefSurrogate::GetObjectData(System.Object,System.Runtime.Serialization.SerializationInfo,System.Runtime.Serialization.StreamingContext)

# implements 'System.Void System.Runtime.Serialization.ISerializationSurrogate::GetObjectData(System.Object,System.Runtime.Serialization.SerializationInfo,System.Runtime.Serialization.StreamingContext)'.
+SC-M: System.Void System.Runtime.Remoting.Messaging.RemotingSurrogate::GetObjectData(System.Object,System.Runtime.Serialization.SerializationInfo,System.Runtime.Serialization.StreamingContext)

# overrides 'System.Void System.Runtime.Remoting.Metadata.SoapAttribute::SetReflectionObject(System.Object)'.
+SC-M: System.Void System.Runtime.Remoting.Metadata.SoapFieldAttribute::SetReflectionObject(System.Object)

# overrides 'System.Void System.Runtime.Remoting.Metadata.SoapAttribute::SetReflectionObject(System.Object)'.
+SC-M: System.Void System.Runtime.Remoting.Metadata.SoapMethodAttribute::SetReflectionObject(System.Object)

# overrides 'System.Void System.Runtime.Remoting.Metadata.SoapAttribute::SetReflectionObject(System.Object)'.
+SC-M: System.Void System.Runtime.Remoting.Metadata.SoapTypeAttribute::SetReflectionObject(System.Object)

# implements 'System.Void System.Runtime.Serialization.IFormatter::Serialize(System.IO.Stream,System.Object)'.
+SC-M: System.Void System.Runtime.Serialization.Formatters.Binary.BinaryFormatter::Serialize(System.IO.Stream,System.Object)

# implements 'System.Void System.Runtime.Remoting.Messaging.IRemotingFormatter::Serialize(System.IO.Stream,System.Object,System.Runtime.Remoting.Messaging.Header[])'.
+SC-M: System.Void System.Runtime.Serialization.Formatters.Binary.BinaryFormatter::Serialize(System.IO.Stream,System.Object,System.Runtime.Remoting.Messaging.Header[])

# implements 'System.Void System.Runtime.Serialization.IFormatter::set_SurrogateSelector(System.Runtime.Serialization.ISurrogateSelector)'.
+SC-M: System.Void System.Runtime.Serialization.Formatters.Binary.BinaryFormatter::set_SurrogateSelector(System.Runtime.Serialization.ISurrogateSelector)

# overrides 'System.Void System.Runtime.Serialization.Formatters.Binary.TypeMetadata::WriteAssemblies(System.Runtime.Serialization.Formatters.Binary.ObjectWriter,System.IO.BinaryWriter)'.
+SC-M: System.Void System.Runtime.Serialization.Formatters.Binary.SerializableTypeMetadata::WriteAssemblies(System.Runtime.Serialization.Formatters.Binary.ObjectWriter,System.IO.BinaryWriter)

# overrides 'System.Void System.Runtime.Serialization.Formatters.Binary.TypeMetadata::WriteObjectData(System.Runtime.Serialization.Formatters.Binary.ObjectWriter,System.IO.BinaryWriter,System.Object)'.
+SC-M: System.Void System.Runtime.Serialization.Formatters.Binary.SerializableTypeMetadata::WriteObjectData(System.Runtime.Serialization.Formatters.Binary.ObjectWriter,System.IO.BinaryWriter,System.Object)

# overrides 'System.Void System.Runtime.Serialization.Formatters.Binary.TypeMetadata::WriteTypeData(System.Runtime.Serialization.Formatters.Binary.ObjectWriter,System.IO.BinaryWriter,System.Boolean)'.
+SC-M: System.Void System.Runtime.Serialization.Formatters.Binary.SerializableTypeMetadata::WriteTypeData(System.Runtime.Serialization.Formatters.Binary.ObjectWriter,System.IO.BinaryWriter,System.Boolean)

# internal call
+SC-M: System.Void System.Security.Cryptography.RNGCryptoServiceProvider::RngClose(System.IntPtr)

# using 'System.Char*' as a parameter type
+SC-M: System.Void System.String::CharCopy(System.Char*,System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Void System.String::CharCopyReverse(System.Char*,System.Char*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.String::memcpy(System.Byte*,System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.String::memcpy1(System.Byte*,System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.String::memcpy2(System.Byte*,System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.String::memcpy4(System.Byte*,System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.String::memset(System.Byte*,System.Int32,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Void System.Text.Decoder::CheckArguments(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Void System.Text.Encoder::CheckArguments(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.Text.UnicodeEncoding::CopyChars(System.Byte*,System.Byte*,System.Int32,System.Boolean)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.Text.UTF8Encoding::Fallback(System.Object,System.Text.DecoderFallbackBuffer&,System.Byte[]&,System.Byte*,System.Int64,System.UInt32,System.Char*,System.Int32&)

# internal call
+SC-M: System.Void System.Threading.InternalThread::Thread_free_internal(System.IntPtr)

# internal call
+SC-M: System.Void System.Threading.Monitor::Monitor_pulse(System.Object)

# internal call
+SC-M: System.Void System.Threading.Monitor::Monitor_pulse_all(System.Object)

# internal call
+SC-M: System.Void System.Threading.Monitor::try_enter_with_atomic_var(System.Object,System.Int32,System.Boolean&)

# internal call
+SC-M: System.Void System.Threading.NativeEventCalls::CloseEvent_internal(System.IntPtr)

# internal call
+SC-M: System.Void System.Threading.Thread::Abort_internal(System.Threading.InternalThread,System.Object)

# internal call
+SC-M: System.Void System.Threading.Thread::ClrState(System.Threading.InternalThread,System.Threading.ThreadState)

# internal call
+SC-M: System.Void System.Threading.Thread::ConstructInternalThread()

# internal call
+SC-M: System.Void System.Threading.Thread::ResetAbort_internal()

# internal call
+SC-M: System.Void System.Threading.Thread::SetName_internal(System.Threading.InternalThread,System.String)

# internal call
+SC-M: System.Void System.Threading.Thread::SetState(System.Threading.InternalThread,System.Threading.ThreadState)

# internal call
+SC-M: System.Void System.Threading.Thread::Sleep_internal(System.Int32)

# internal call
+SC-M: System.Void System.Threading.Thread::SpinWait_nop()

# internal call
+SC-M: System.Void System.Threading.ThreadPool::pool_queue(System.Runtime.Remoting.Messaging.AsyncResult)

# internal call
+SC-M: System.Void System.Type::GetInterfaceMapData(System.Type,System.Type,System.Reflection.MethodInfo[]&,System.Reflection.MethodInfo[]&)

