using System;
using System.Windows;

namespace Mono {

	class DependencyObjectHandle {

		public IntPtr Handle {
			get; private set;
		}

		public INativeEventObjectWrapper Object {
			get; private set;
		}

		public DependencyObjectHandle (IntPtr handle, INativeEventObjectWrapper wrapper)
		{
			Object = wrapper;
			Handle = handle;
		}

		~DependencyObjectHandle ()
		{
			NativeDependencyObjectHelper.FreeNativeMapping (Handle);
			Handle = IntPtr.Zero;
		}
	}
}

