# [SecurityCritical] needed to execute code inside 'System, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e'.
# 29 methods needs to be decorated.

# internal call
+SC-M: System.Boolean System.Net.Dns::GetHostByAddr_internal(System.String,System.String&,System.String[]&,System.String[]&)

# internal call
+SC-M: System.Boolean System.Net.Dns::GetHostByName_internal(System.String,System.String&,System.String[]&,System.String[]&)

# internal call
+SC-M: System.Boolean System.Net.Dns::GetHostName_internal(System.String&)

# internal call
+SC-M: System.Boolean System.Net.Sockets.Socket::Poll_internal(System.IntPtr,System.Net.Sockets.SelectMode,System.Int32,System.Int32&)

# internal call
+SC-M: System.Boolean System.Net.Sockets.Socket::SendFile(System.IntPtr,System.String,System.Byte[],System.Byte[],System.Net.Sockets.TransmitFileOptions)

# internal call
+SC-M: System.Int32 System.Net.Sockets.Socket::Available_internal(System.IntPtr,System.Int32&)

# internal call
+SC-M: System.Int32 System.Net.Sockets.Socket::Receive_internal(System.IntPtr,System.Byte[],System.Int32,System.Int32,System.Net.Sockets.SocketFlags,System.Int32&)

# internal call
+SC-M: System.Int32 System.Net.Sockets.Socket::Receive_internal(System.IntPtr,System.Net.Sockets.Socket/WSABUF[],System.Net.Sockets.SocketFlags,System.Int32&)

# internal call
+SC-M: System.Int32 System.Net.Sockets.Socket::RecvFrom_internal(System.IntPtr,System.Byte[],System.Int32,System.Int32,System.Net.Sockets.SocketFlags,System.Net.SocketAddress&,System.Int32&)

# internal call
+SC-M: System.Int32 System.Net.Sockets.Socket::Send_internal(System.IntPtr,System.Byte[],System.Int32,System.Int32,System.Net.Sockets.SocketFlags,System.Int32&)

# internal call
+SC-M: System.Int32 System.Net.Sockets.Socket::Send_internal(System.IntPtr,System.Net.Sockets.Socket/WSABUF[],System.Net.Sockets.SocketFlags,System.Int32&)

# internal call
+SC-M: System.Int32 System.Net.Sockets.Socket::SendTo_internal(System.IntPtr,System.Byte[],System.Int32,System.Int32,System.Net.Sockets.SocketFlags,System.Net.SocketAddress,System.Int32&)

# internal call
+SC-M: System.Int32 System.Net.Sockets.Socket::WSAIoctl(System.IntPtr,System.Int32,System.Byte[],System.Byte[],System.Int32&)

# internal call
+SC-M: System.Int32 System.Net.Sockets.SocketException::WSAGetLastError_internal()

# internal call
+SC-M: System.IntPtr System.Net.Sockets.Socket::Accept_internal(System.IntPtr,System.Int32&,System.Boolean)

# internal call
+SC-M: System.IntPtr System.Net.Sockets.Socket::Socket_internal(System.Net.Sockets.AddressFamily,System.Net.Sockets.SocketType,System.Net.Sockets.ProtocolType,System.Int32&)

# internal call
+SC-M: System.Net.SocketAddress System.Net.Sockets.Socket::LocalEndPoint_internal(System.IntPtr,System.Int32&)

# internal call
+SC-M: System.Net.SocketAddress System.Net.Sockets.Socket::RemoteEndPoint_internal(System.IntPtr,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::Bind_internal(System.IntPtr,System.Net.SocketAddress,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::Blocking_internal(System.IntPtr,System.Boolean,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::Close_internal(System.IntPtr,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::Connect_internal(System.IntPtr,System.Net.SocketAddress,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::Disconnect_internal(System.IntPtr,System.Boolean,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::GetSocketOption_arr_internal(System.IntPtr,System.Net.Sockets.SocketOptionLevel,System.Net.Sockets.SocketOptionName,System.Byte[]&,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::GetSocketOption_obj_internal(System.IntPtr,System.Net.Sockets.SocketOptionLevel,System.Net.Sockets.SocketOptionName,System.Object&,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::Listen_internal(System.IntPtr,System.Int32,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::Select_internal(System.Net.Sockets.Socket[]&,System.Int32,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::SetSocketOption_internal(System.IntPtr,System.Net.Sockets.SocketOptionLevel,System.Net.Sockets.SocketOptionName,System.Object,System.Byte[],System.Int32,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::Shutdown_internal(System.IntPtr,System.Net.Sockets.SocketShutdown,System.Int32&)

