using System;
using System.Runtime.InteropServices;


namespace Mono {

	class EventObjectSafeHandle : SafeHandle {

		public override bool IsInvalid {
			get { return handle == IntPtr.Zero; }
		}

		public EventObjectSafeHandle (IntPtr handle)
			: base (IntPtr.Zero, true)
		{
			this.handle = handle;
		}


		protected override bool ReleaseHandle ()
		{
			NativeDependencyObjectHelper.RemoveNativeMapping (handle);
			return true;
		}
	}
 }
