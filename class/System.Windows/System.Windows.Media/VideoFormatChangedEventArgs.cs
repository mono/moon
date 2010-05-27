/*
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Media
{
        internal class VideoFormatChangedEventArgs : EventArgs
        {
                internal VideoFormatChangedEventArgs (IntPtr raw)
                {
			IntPtr f = NativeMethods.video_format_changed_event_args_get_new_format (raw);

			unsafe {
				UnmanagedVideoFormat *uf = (UnmanagedVideoFormat*)f;
				NewFormat = uf->ToVideoFormat ();
			}
                }

		public VideoFormat NewFormat {
			get; private set;
		}
        }
}
