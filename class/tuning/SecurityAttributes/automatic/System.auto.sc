# [SecurityCritical] needed to execute code inside 'System, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e'.
# 63 methods needs to be decorated.

# internal call
+SC-M: System.Boolean System.Net.Dns::GetHostByAddr_internal(System.String,System.String&,System.String[]&,System.String[]&)

# internal call
+SC-M: System.Boolean System.Net.Dns::GetHostByName_internal(System.String,System.String&,System.String[]&,System.String[]&)

# internal call
+SC-M: System.Boolean System.Net.Dns::GetHostName_internal(System.String&)

# overrides 'System.Boolean System.Net.WebRequest::get_PreAuthenticate()'.
+SC-M: System.Boolean System.Net.HttpWebRequest::get_PreAuthenticate()

# overrides 'System.Boolean System.Net.WebRequest::get_UseDefaultCredentials()'.
+SC-M: System.Boolean System.Net.HttpWebRequest::get_UseDefaultCredentials()

# internal call
+SC-M: System.Boolean System.Net.Sockets.Socket::Poll_internal(System.IntPtr,System.Net.Sockets.SelectMode,System.Int32,System.Int32&)

# internal call
+SC-M: System.Boolean System.Net.Sockets.Socket::SendFile(System.IntPtr,System.String,System.Byte[],System.Byte[],System.Net.Sockets.TransmitFileOptions)

# overrides 'System.IAsyncResult System.Net.WebRequest::BeginGetRequestStream(System.AsyncCallback,System.Object)'.
+SC-M: System.IAsyncResult System.Net.HttpWebRequest::BeginGetRequestStream(System.AsyncCallback,System.Object)

# overrides 'System.IAsyncResult System.Net.WebRequest::BeginGetResponse(System.AsyncCallback,System.Object)'.
+SC-M: System.IAsyncResult System.Net.HttpWebRequest::BeginGetResponse(System.AsyncCallback,System.Object)

# p/invoke declaration
+SC-M: System.Int32 System.IO.Compression.DeflateStream::CloseZStream(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 System.IO.Compression.DeflateStream::Flush(System.IntPtr)

# p/invoke declaration
+SC-M: System.Int32 System.IO.Compression.DeflateStream::ReadZStream(System.IntPtr,System.IntPtr,System.Int32)

# p/invoke declaration
+SC-M: System.Int32 System.IO.Compression.DeflateStream::WriteZStream(System.IntPtr,System.IntPtr,System.Int32)

# overrides 'System.Int32 System.Net.WebRequest::get_Timeout()'.
+SC-M: System.Int32 System.Net.HttpWebRequest::get_Timeout()

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

# overrides 'System.Int64 System.Net.WebRequest::get_ContentLength()'.
+SC-M: System.Int64 System.Net.HttpWebRequest::get_ContentLength()

# p/invoke declaration
+SC-M: System.IntPtr System.IO.Compression.DeflateStream::CreateZStream(System.IO.Compression.CompressionMode,System.Boolean,System.IO.Compression.DeflateStream/UnmanagedReadOrWrite,System.IntPtr)

# internal call
+SC-M: System.IntPtr System.Net.Sockets.Socket::Accept_internal(System.IntPtr,System.Int32&,System.Boolean)

# internal call
+SC-M: System.IntPtr System.Net.Sockets.Socket::Socket_internal(System.Net.Sockets.AddressFamily,System.Net.Sockets.SocketType,System.Net.Sockets.ProtocolType,System.Int32&)

# overrides 'System.IO.Stream System.Net.WebRequest::EndGetRequestStream(System.IAsyncResult)'.
+SC-M: System.IO.Stream System.Net.HttpWebRequest::EndGetRequestStream(System.IAsyncResult)

# overrides 'System.IO.Stream System.Net.WebRequest::GetRequestStream()'.
+SC-M: System.IO.Stream System.Net.HttpWebRequest::GetRequestStream()

# overrides 'System.Net.ICredentials System.Net.WebRequest::get_Credentials()'.
+SC-M: System.Net.ICredentials System.Net.HttpWebRequest::get_Credentials()

# overrides 'System.Net.IWebProxy System.Net.WebRequest::get_Proxy()'.
+SC-M: System.Net.IWebProxy System.Net.HttpWebRequest::get_Proxy()

# internal call
+SC-M: System.Net.SocketAddress System.Net.Sockets.Socket::LocalEndPoint_internal(System.IntPtr,System.Int32,System.Int32&)

# internal call
+SC-M: System.Net.SocketAddress System.Net.Sockets.Socket::RemoteEndPoint_internal(System.IntPtr,System.Int32,System.Int32&)

# overrides 'System.Net.WebHeaderCollection System.Net.WebRequest::get_Headers()'.
+SC-M: System.Net.WebHeaderCollection System.Net.HttpWebRequest::get_Headers()

# overrides 'System.Net.WebResponse System.Net.WebRequest::EndGetResponse(System.IAsyncResult)'.
+SC-M: System.Net.WebResponse System.Net.HttpWebRequest::EndGetResponse(System.IAsyncResult)

# overrides 'System.Net.WebResponse System.Net.WebRequest::GetResponse()'.
+SC-M: System.Net.WebResponse System.Net.HttpWebRequest::GetResponse()

# overrides 'System.String System.Net.WebRequest::get_ConnectionGroupName()'.
+SC-M: System.String System.Net.HttpWebRequest::get_ConnectionGroupName()

# overrides 'System.String System.Net.WebRequest::get_ContentType()'.
+SC-M: System.String System.Net.HttpWebRequest::get_ContentType()

# overrides 'System.String System.Net.WebRequest::get_Method()'.
+SC-M: System.String System.Net.HttpWebRequest::get_Method()

# overrides 'System.Uri System.Net.WebRequest::get_RequestUri()'.
+SC-M: System.Uri System.Net.HttpWebRequest::get_RequestUri()

# overrides 'System.Void System.Net.WebRequest::Abort()'.
+SC-M: System.Void System.Net.HttpWebRequest::Abort()

# overrides 'System.Void System.Net.WebRequest::set_ConnectionGroupName(System.String)'.
+SC-M: System.Void System.Net.HttpWebRequest::set_ConnectionGroupName(System.String)

# overrides 'System.Void System.Net.WebRequest::set_ContentLength(System.Int64)'.
+SC-M: System.Void System.Net.HttpWebRequest::set_ContentLength(System.Int64)

# overrides 'System.Void System.Net.WebRequest::set_ContentType(System.String)'.
+SC-M: System.Void System.Net.HttpWebRequest::set_ContentType(System.String)

# overrides 'System.Void System.Net.WebRequest::set_Credentials(System.Net.ICredentials)'.
+SC-M: System.Void System.Net.HttpWebRequest::set_Credentials(System.Net.ICredentials)

# overrides 'System.Void System.Net.WebRequest::set_Headers(System.Net.WebHeaderCollection)'.
+SC-M: System.Void System.Net.HttpWebRequest::set_Headers(System.Net.WebHeaderCollection)

# overrides 'System.Void System.Net.WebRequest::set_Method(System.String)'.
+SC-M: System.Void System.Net.HttpWebRequest::set_Method(System.String)

# overrides 'System.Void System.Net.WebRequest::set_PreAuthenticate(System.Boolean)'.
+SC-M: System.Void System.Net.HttpWebRequest::set_PreAuthenticate(System.Boolean)

# overrides 'System.Void System.Net.WebRequest::set_Proxy(System.Net.IWebProxy)'.
+SC-M: System.Void System.Net.HttpWebRequest::set_Proxy(System.Net.IWebProxy)

# overrides 'System.Void System.Net.WebRequest::set_Timeout(System.Int32)'.
+SC-M: System.Void System.Net.HttpWebRequest::set_Timeout(System.Int32)

# overrides 'System.Void System.Net.WebRequest::set_UseDefaultCredentials(System.Boolean)'.
+SC-M: System.Void System.Net.HttpWebRequest::set_UseDefaultCredentials(System.Boolean)

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

# internal call
+SC-M: System.Void System.Net.Sockets.Socket::socket_pool_queue(System.Net.Sockets.Socket/SocketAsyncCall,System.Net.Sockets.Socket/SocketAsyncResult)

