using System;
using System.Windows;

namespace Mono {

	class DependencyObjectHandle {

		public IntPtr Handle {
			get; set;
		}

		public INativeEventObjectWrapper Object {
			get; set;
		}

		~DependencyObjectHandle ()
		{
			NativeDependencyObjectHelper.FreeNativeMapping (Handle);
			Handle = IntPtr.Zero;
		}
	}
}

