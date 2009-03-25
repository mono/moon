# [SecurityCritical] needed to execute code inside 'System.Net, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e'.
# 9 methods needs to be decorated.

# p/invoke declaration
+SC-M: System.Int32 System.Net.NetworkInformation.LinuxNetworkInterface::getifaddrs(System.IntPtr&)

# p/invoke declaration
+SC-M: System.Int32 System.Net.NetworkInformation.LinuxNetworkInterface::if_nametoindex(System.String)

# p/invoke declaration
+SC-M: System.Int32 System.Net.NetworkInformation.Win32_FIXED_INFO::GetNetworkParams(System.Byte[],System.Int32&)

# p/invoke declaration
+SC-M: System.Int32 System.Net.NetworkInformation.Win32IPv4InterfaceProperties::GetPerAdapterInfo(System.Int32,System.Net.NetworkInformation.Win32_IP_PER_ADAPTER_INFO,System.Int32&)

# p/invoke declaration
+SC-M: System.Int32 System.Net.NetworkInformation.Win32NetworkInterface2::GetAdaptersAddresses(System.UInt32,System.UInt32,System.IntPtr,System.Byte[],System.Int32&)

# p/invoke declaration
+SC-M: System.Int32 System.Net.NetworkInformation.Win32NetworkInterface2::GetAdaptersInfo(System.Byte[],System.Int32&)

# p/invoke declaration
+SC-M: System.Int32 System.Net.NetworkInformation.Win32NetworkInterface2::GetIfEntry(System.Net.NetworkInformation.Win32_MIB_IFROW&)

# p/invoke declaration
+SC-M: System.UInt32 System.Net.WebClient::g_timeout_add(System.UInt32,System.Net.WebClient/GSourceFunc,System.IntPtr)

# p/invoke declaration
+SC-M: System.Void System.Net.NetworkInformation.LinuxNetworkInterface::freeifaddrs(System.IntPtr)

