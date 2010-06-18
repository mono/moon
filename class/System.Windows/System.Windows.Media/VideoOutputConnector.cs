/*
 * VideoOutputConnector.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

namespace System.Windows.Media {
	public class VideoOutputConnector {
		internal VideoOutputConnector ()
		{
			// This ctor is here to prevent the compiler from adding a default ctor
		}

		public bool CanEnableCgmsa {
			get {
				Console.WriteLine ("NIEX: System.Windows.Media.VideoOutputConnector:get_CanEnableCgmsa");
				throw new NotImplementedException ();
			}
		}

		public bool CanEnableHdcp {
			get {
				Console.WriteLine ("NIEX: System.Windows.Media.VideoOutputConnector:get_CanEnableHdcp");
				throw new NotImplementedException ();
			}
		}

		public VideoOutputConnectorType ConnectorType {
			get {
				Console.WriteLine ("NIEX: System.Windows.Media.VideoOutputConnector:get_ConnectorType");
				throw new NotImplementedException ();
			}
		}
	}
}