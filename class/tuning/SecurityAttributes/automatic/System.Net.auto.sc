# [SecurityCritical] needed to execute code inside 'System.Net, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e'.
# 16 methods needs to be decorated.

# internal call
+SC-M: System.Boolean System.Net.Dns::GetHostByName_internal(System.String,System.String&,System.String[]&,System.String[]&)

# p/invoke declaration
+SC-M: System.Boolean System.Net.NetworkInformation.NetworkChange::moon_network_service_get_is_network_available(System.IntPtr)

# internal call
+SC-M: System.Boolean System.Net.Sockets.Socket::Poll_internal(System.IntPtr,System.Net.Sockets.SelectMode,System.Int32,System.Int32&)

# internal call
+SC-M: System.Int32 System.Net.Sockets.Socket::Receive_internal(System.IntPtr,System.Net.Sockets.Socket/WSABUF[],System.Net.Sockets.SocketFlags,System.Int32&)

# internal call
+SC-M: System.Int32 System.Net.Sockets.Socket::Send_internal(System.IntPtr,System.Net.Sockets.Socket/WSABUF[],System.Net.Sockets.SocketFlags,System.Int32&)

# internal call
+SC-M: System.Int32 System.Net.Sockets.SocketException::WSAGetLastError_internal()

# p/invoke declaration
+SC-M: System.IntPtr System.Net.NetworkInformation.NetworkChange::runtime_get_network_service()

# internal call
+SC-M: System.IntPtr System.Net.Sockets.Socket::Socket_internal(System.Net.Sockets.AddressFamily,System.Net.Sockets.SocketType,System.Net.Sockets.ProtocolType,System.Int32&)

# internal call
+SC-M: System.Net.SocketAddress System.Net.Sockets.Socket::RemoteEndPoint_internal(System.IntPtr,System.Int32,System.Int32&)

# p/invoke declaration
+SC-M: System.Void System.Net.NetworkInformation.NetworkChange::moon_network_service_set_network_state_changed_callback(System.IntPtr,System.Net.NetworkInformation.NetworkChange/NetworkStateChangedCallback,System.IntPtr)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::Close_internal(System.IntPtr,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::Connect_internal(System.IntPtr,System.Net.SocketAddress,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::GetSocketOption_obj_internal(System.IntPtr,System.Net.Sockets.SocketOptionLevel,System.Net.Sockets.SocketOptionName,System.Object&,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::SetSocketOption_internal(System.IntPtr,System.Net.Sockets.SocketOptionLevel,System.Net.Sockets.SocketOptionName,System.Object,System.Byte[],System.Int32,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::Shutdown_internal(System.IntPtr,System.Net.Sockets.SocketShutdown,System.Int32&)

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::socket_pool_queue(System.Net.Sockets.Socket/SocketAsyncCall,System.Net.Sockets.Socket/SocketAsyncResult)

