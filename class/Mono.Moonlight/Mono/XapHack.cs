
//
// This is a gross hack: it is here because Mono.Moonlight is friends
// with mscorlib and hence, we can derive from MarshalByRefObject
// which does not exist in 2.1.
//
// Then in System.Windows we derive from this
//
using System;

namespace Mono {
#if !NET_2_1
	public
#else
	internal
#endif
	abstract class XapHackProxy : MarshalByRefObject {

		public abstract bool Setup (IntPtr value, IntPtr surface, string xapFile);

		public abstract void Terminate ();
	}
}