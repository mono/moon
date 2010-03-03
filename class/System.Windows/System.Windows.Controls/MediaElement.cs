//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2007, 2009 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using Mono;
using System.IO;
using System.Windows.Media;
using System.Windows.Automation.Peers;

namespace System.Windows.Controls {

	public sealed partial class MediaElement : FrameworkElement {
		private StreamWrapper wrapper;
		private LicenseAcquirer license_acquirer;

		private MediaStreamSource media_stream_source;
		
		public LicenseAcquirer LicenseAcquirer {
			get {
				if (license_acquirer == null)
					license_acquirer = new LicenseAcquirer ();
				return license_acquirer;
			}
			set {
				license_acquirer = value;
			}
		}
		
		public void Pause ()
		{
			NativeMethods.media_element_pause (native);
		}
		
		public void Play ()
		{
			NativeMethods.media_element_play (native);
		}
		
		public void SetSource (Stream stream)
		{
			if (stream == null)
				throw new ArgumentNullException ("stream");

			Source = null;
			wrapper = new StreamWrapper (stream);
			ManagedStreamCallbacks callbacks = wrapper.GetCallbacks ();
			NativeMethods.media_element_set_stream_source (this.native, ref callbacks);
		}
		
		public void SetSource (MediaStreamSource mediaStreamSource)
		{
			if (mediaStreamSource == null)
				throw new ArgumentNullException ("mediaStreamSource");

			Source = null;

			if (media_stream_source != null) {
				media_stream_source.CloseMediaInternal ();
				media_stream_source = null;
			}

			if (mediaStreamSource.Closed)
				throw new InvalidOperationException ();
			
			media_stream_source = mediaStreamSource;
			media_stream_source.SetMediaElement (this);
		}
		
		public void Stop ()
		{
			NativeMethods.media_element_stop (native);
		}
		
		public void RequestLog ()
		{
			Console.WriteLine ("MediaElement.RequestLog (): Not implemented.");
		}		

		protected override AutomationPeer OnCreateAutomationPeer ()
		{
			return new MediaElementAutomationPeer (this);
		}
	}
}
