# [SecurityCritical] needed to execute code inside 'mscorlib, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e'.
# 130 methods needs to be decorated.

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

# using 'System.Byte*' as a parameter type
+SC-M: System.Boolean System.Double::ParseImpl(System.Byte*,System.Double&)

# p/invoke declaration
+SC-M: System.Boolean System.IO.IsolatedStorage.IsolatedStorageFile::isolated_storage_increase_quota_to(System.String,System.String)

# using 'System.Security.RuntimeDeclSecurityActions*' as a parameter type
+SC-M: System.Boolean System.Security.SecurityManager::InheritanceDemand(System.AppDomain,System.Reflection.Assembly,System.Security.RuntimeDeclSecurityActions*)

# using 'System.Security.RuntimeDeclSecurityActions*' as a parameter type
+SC-M: System.Boolean System.Security.SecurityManager::LinkDemand(System.Reflection.Assembly,System.Security.RuntimeDeclSecurityActions*,System.Security.RuntimeDeclSecurityActions*)

# using 'System.Threading.NativeOverlapped*' as a parameter type
+SC-M: System.Boolean System.Threading.ThreadPool::UnsafeQueueNativeOverlapped(System.Threading.NativeOverlapped*)

# using 'System.Byte*' as a parameter type
+SC-M: System.Byte[] Mono.Security.BitConverterLE::GetUIntBytes(System.Byte*)

# using 'System.Byte*' as a parameter type
+SC-M: System.Byte[] Mono.Security.BitConverterLE::GetULongBytes(System.Byte*)

# using 'System.Byte*' as a parameter type
+SC-M: System.Byte[] Mono.Security.BitConverterLE::GetUShortBytes(System.Byte*)

# using 'System.Byte*' as a parameter type
+SC-M: System.Byte[] System.BitConverter::GetBytes(System.Byte*,System.Int32)

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

# p/invoke declaration
+SC-M: System.Int32 System.__ComObject::CoCreateInstance(System.Guid,System.IntPtr,System.UInt32,System.Guid,System.IntPtr&)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.ASCIIEncoding::GetByteCount(System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.ASCIIEncoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.ASCIIEncoding::GetCharCount(System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.ASCIIEncoding::GetChars(System.Byte*,System.Int32,System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.Encoding::GetByteCount(System.Char*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.Encoding::GetCharCount(System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.Encoding::GetChars(System.Byte*,System.Int32,System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UnicodeEncoding::GetByteCount(System.Char*,System.Int32)

# [VISIBLE] overrides 'System.Int32 System.Text.Encoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)'.
+SC-M: System.Int32 System.Text.UnicodeEncoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UnicodeEncoding::GetBytesInternal(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UnicodeEncoding::GetCharCount(System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UnicodeEncoding::GetChars(System.Byte*,System.Int32,System.Char*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UnicodeEncoding::GetCharsInternal(System.Byte*,System.Int32,System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF32Encoding::GetByteCount(System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF32Encoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UTF32Encoding::GetCharCount(System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UTF32Encoding::GetChars(System.Byte*,System.Int32,System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF7Encoding::GetByteCount(System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF7Encoding::GetBytes(System.Char*,System.Int32,System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UTF7Encoding::GetCharCount(System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UTF7Encoding::GetChars(System.Byte*,System.Int32,System.Char*,System.Int32)

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

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::GetCharCount(System.Byte*,System.Int32)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::GetChars(System.Byte*,System.Int32,System.Char*,System.Int32)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::InternalGetByteCount(System.Char*,System.Int32,System.Char&,System.Boolean)

# using 'System.Char*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::InternalGetBytes(System.Char*,System.Int32,System.Byte*,System.Int32,System.Char&,System.Boolean)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::InternalGetCharCount(System.Byte*,System.Int32,System.UInt32,System.UInt32,System.Object,System.Text.DecoderFallbackBuffer&,System.Byte[]&,System.Boolean)

# using 'System.Byte*' as a parameter type
+SC-M: System.Int32 System.Text.UTF8Encoding::InternalGetChars(System.Byte*,System.Int32,System.Char*,System.Int32,System.UInt32&,System.UInt32&,System.Object,System.Text.DecoderFallbackBuffer&,System.Byte[]&,System.Boolean)

# p/invoke declaration
+SC-M: System.Int64 System.IO.IsolatedStorage.IsolatedStorage::isolated_storage_get_current_usage(System.String)

# [VISIBLE] implements 'System.IO.FileStream System.Runtime.InteropServices._Assembly::GetFile(System.String)'.
+SC-M: System.IO.FileStream System.Reflection.Assembly::GetFile(System.String)

# [VISIBLE] overrides 'System.IO.FileStream System.Reflection.Assembly::GetFile(System.String)'.
+SC-M: System.IO.FileStream System.Reflection.Emit.AssemblyBuilder::GetFile(System.String)

# Promoting interface member to [SecurityCritical] because of 'System.IO.FileStream System.Reflection.Assembly::GetFile(System.String)'.
+SC-M: System.IO.FileStream System.Runtime.InteropServices._Assembly::GetFile(System.String)

# implements 'System.IO.FileStream[] System.Runtime.InteropServices._Assembly::GetFiles(System.Boolean)'.
+SC-M: System.IO.FileStream[] System.Reflection.Assembly::GetFiles()

# [VISIBLE] implements 'System.IO.FileStream[] System.Runtime.InteropServices._Assembly::GetFiles(System.Boolean)'.
+SC-M: System.IO.FileStream[] System.Reflection.Assembly::GetFiles(System.Boolean)

# [VISIBLE] overrides 'System.IO.FileStream[] System.Reflection.Assembly::GetFiles()'.
+SC-M: System.IO.FileStream[] System.Reflection.Emit.AssemblyBuilder::GetFiles(System.Boolean)

# Promoting interface member to [SecurityCritical] because of 'System.IO.FileStream[] System.Reflection.Assembly::GetFiles()'.
+SC-M: System.IO.FileStream[] System.Runtime.InteropServices._Assembly::GetFiles()

# Promoting interface member to [SecurityCritical] because of 'System.IO.FileStream[] System.Reflection.Assembly::GetFiles()'.
+SC-M: System.IO.FileStream[] System.Runtime.InteropServices._Assembly::GetFiles(System.Boolean)

# Promoting interface member to [SecurityCritical] because of 'System.Object System.AppDomain::GetData(System.String)'.
+SC-M: System.Object System._AppDomain::GetData(System.String)

# [VISIBLE] implements 'System.Object System._AppDomain::GetData(System.String)'.
+SC-M: System.Object System.AppDomain::GetData(System.String)

# [VISIBLE] overrides 'System.Object System.Delegate::DynamicInvokeImpl(System.Object[])'.
+SC-M: System.Object System.MulticastDelegate::DynamicInvokeImpl(System.Object[])

# [VISIBLE] implements 'System.Reflection.AssemblyName System.Runtime.InteropServices._Assembly::GetName(System.Boolean)'.
+SC-M: System.Reflection.AssemblyName System.Reflection.Assembly::GetName()

# [VISIBLE] implements 'System.Reflection.AssemblyName System.Runtime.InteropServices._Assembly::GetName(System.Boolean)'.
+SC-M: System.Reflection.AssemblyName System.Reflection.Assembly::GetName(System.Boolean)

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.AssemblyName System.Reflection.Assembly::GetName(System.Boolean)'.
+SC-M: System.Reflection.AssemblyName System.Runtime.InteropServices._Assembly::GetName()

# Promoting interface member to [SecurityCritical] because of 'System.Reflection.AssemblyName System.Reflection.Assembly::GetName(System.Boolean)'.
+SC-M: System.Reflection.AssemblyName System.Runtime.InteropServices._Assembly::GetName(System.Boolean)

# [VISIBLE] implements 'System.String System.Runtime.InteropServices._Assembly::get_CodeBase()'.
+SC-M: System.String System.Reflection.Assembly::get_CodeBase()

# [VISIBLE] implements 'System.String System.Runtime.InteropServices._Assembly::get_Location()'.
+SC-M: System.String System.Reflection.Assembly::get_Location()

# [VISIBLE] overrides 'System.String System.Reflection.Assembly::get_CodeBase()'.
+SC-M: System.String System.Reflection.Emit.AssemblyBuilder::get_CodeBase()

# [VISIBLE] overrides 'System.String System.Reflection.Assembly::get_Location()'.
+SC-M: System.String System.Reflection.Emit.AssemblyBuilder::get_Location()

# [VISIBLE] overrides 'System.String System.Reflection.Module::get_FullyQualifiedName()'.
+SC-M: System.String System.Reflection.Emit.ModuleBuilder::get_FullyQualifiedName()

# Promoting interface member to [SecurityCritical] because of 'System.String System.Reflection.Assembly::get_CodeBase()'.
+SC-M: System.String System.Runtime.InteropServices._Assembly::get_CodeBase()

# Promoting interface member to [SecurityCritical] because of 'System.String System.Reflection.Assembly::get_Location()'.
+SC-M: System.String System.Runtime.InteropServices._Assembly::get_Location()

# arglist
+SC-M: System.String System.String::Concat(System.Object,System.Object,System.Object,System.Object)

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

# localloc
+SC-M: System.String System.String::ReplaceUnchecked(System.String,System.String)

# using 'System.Byte*' as a parameter type
+SC-M: System.UInt32 Mono.Globalization.Unicode.MSCompatUnicodeTable::UInt32FromBytePtr(System.Byte*,System.UInt32)

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

# [VISIBLE] implements 'System.Void System._AppDomain::remove_AssemblyResolve(System.ResolveEventHandler)'.
+SC-M: System.Void System.AppDomain::remove_AssemblyResolve(System.ResolveEventHandler)

# [VISIBLE] implements 'System.Void System._AppDomain::remove_UnhandledException(System.UnhandledExceptionEventHandler)'.
+SC-M: System.Void System.AppDomain::remove_UnhandledException(System.UnhandledExceptionEventHandler)

# [VISIBLE] implements 'System.Void System._AppDomain::SetData(System.String,System.Object)'.
+SC-M: System.Void System.AppDomain::SetData(System.String,System.Object)

# using 'System.Void*' as a parameter type
+SC-M: System.Void System.ArgIterator::.ctor(System.RuntimeArgumentHandle,System.Void*)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.BitConverter::PutBytes(System.Byte*,System.Byte[],System.Int32,System.Int32)

# arglist
+SC-M: System.Void System.Console::Write(System.String,System.Object,System.Object,System.Object,System.Object)

# arglist
+SC-M: System.Void System.Console::WriteLine(System.String,System.Object,System.Object,System.Object,System.Object)

# localloc
+SC-M: System.Void System.DateTimeUtils::ZeroPad(System.Text.StringBuilder,System.Int32,System.Int32)

# using 'System.Void*' as a parameter type
+SC-M: System.Void System.Globalization.TextInfo::.ctor(System.Globalization.CultureInfo,System.Int32,System.Void*,System.Boolean)

# [VISIBLE] overrides 'System.Void System.IO.FileSystemInfo::Delete()'.
+SC-M: System.Void System.IO.DirectoryInfo::Delete()

# [VISIBLE] overrides 'System.Void System.IO.FileSystemInfo::Delete()'.
+SC-M: System.Void System.IO.FileInfo::Delete()

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.IO.UnmanagedMemoryStream::.ctor(System.Byte*,System.Int64)

# using 'System.Byte*' as a parameter type
+SC-M: System.Void System.IO.UnmanagedMemoryStream::.ctor(System.Byte*,System.Int64,System.Int64,System.IO.FileAccess)

# using 'System.SByte*' as a parameter type
+SC-M: System.Void System.String::.ctor(System.SByte*,System.Int32,System.Int32)

# using 'System.SByte*' as a parameter type
+SC-M: System.Void System.String::.ctor(System.SByte*,System.Int32,System.Int32,System.Text.Encoding)

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

