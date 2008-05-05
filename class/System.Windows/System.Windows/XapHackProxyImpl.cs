
//
// This is a gross hack: it is here because Mono.Moonlight is friends
// with mscorlib and hence, we can derive from MarshalByRefObject
// which does not exist in 2.1.
//
// Here we derive from `XapHack' that derives from the hidden
// MarshalByRefObject and provide the one method whose only role
// is to call into a static method in Application.
//
// Just great
//

using System;
using Mono;

namespace System.Windows {
	internal class XapHackProxyImpl : XapHackProxy {

		public XapHackProxyImpl () {}
		
		public override bool Setup (IntPtr plugin, IntPtr surface, string xapFile)
		{
			return Application.LaunchFromXap (plugin, surface, xapFile);
		}

		public override void Terminate ()
		{
			Application.Current.Terminate ();
		}
		
	}
}