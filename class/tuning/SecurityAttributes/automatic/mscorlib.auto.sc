# [SecurityCritical] needed to execute code inside 'mscorlib, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e'.
# 107 methods needs to be decorated.

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

# using 'System.Security.RuntimeDeclSecurityActions*' as a parameter type
+SC-M: System.Boolean System.Security.SecurityManager::GetLinkDemandSecurity(System.Reflection.MethodBase,System.Security.RuntimeDeclSecurityActions*,System.Security.RuntimeDeclSecurityActions*)

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

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegCloseKey(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegConnectRegistry(System.String,System.IntPtr,System.IntPtr&)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegCreateKey(System.IntPtr,System.String,System.IntPtr&)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegDeleteKey(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegDeleteValue(System.IntPtr,System.String)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegEnumKey(System.IntPtr,System.Int32,System.Text.StringBuilder,System.Int32)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegEnumValue(System.IntPtr,System.Int32,System.Text.StringBuilder,System.Int32&,System.IntPtr,Microsoft.Win32.RegistryValueKind&,System.IntPtr,System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegFlushKey(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegOpenKeyEx(System.IntPtr,System.String,System.IntPtr,System.Int32,System.IntPtr&)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegQueryValueEx(System.IntPtr,System.String,System.IntPtr,Microsoft.Win32.RegistryValueKind&,System.Byte[],System.Int32&)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegQueryValueEx(System.IntPtr,System.String,System.IntPtr,Microsoft.Win32.RegistryValueKind&,System.Int32&,System.Int32&)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegQueryValueEx(System.IntPtr,System.String,System.IntPtr,Microsoft.Win32.RegistryValueKind&,System.IntPtr,System.Int32&)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegSetValueEx(System.IntPtr,System.String,System.IntPtr,Microsoft.Win32.RegistryValueKind,System.Byte[],System.Int32)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegSetValueEx(System.IntPtr,System.String,System.IntPtr,Microsoft.Win32.RegistryValueKind,System.Int32&,System.Int32)

# p/invoke declaration
+SC-M: System.Int32 Microsoft.Win32.Win32RegistryApi::RegSetValueEx(System.IntPtr,System.String,System.IntPtr,Microsoft.Win32.RegistryValueKind,System.String,System.Int32)

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

# p/invoke declaration
+SC-M: System.Int32 System.Console/WindowsConsole::GetConsoleCP()

# p/invoke declaration
+SC-M: System.Int32 System.Console/WindowsConsole::GetConsoleOutputCP()

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

