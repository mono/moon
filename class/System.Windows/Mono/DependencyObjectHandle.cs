using System;
using System.Windows;

namespace Mono {

	class DependencyObjectHandle {

		public IntPtr Handle {
			get; set;
		}

		public DependencyObject Object {
			get; set;
		}

		~DependencyObjectHandle ()
		{
			Object.Free ();
			Handle = IntPtr.Zero;
		}
	}
}

