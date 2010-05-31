//
// Unit tests for CaptureDeviceConfiguration
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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

using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Media;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Media {

	[TestClass]
	public partial class CaptureDeviceConfigurationTest {

		[TestMethod]
		public void DeviceAccess ()
		{
			Assert.IsFalse (CaptureDeviceConfiguration.AllowedDeviceAccess, "AllowedDeviceAccess");
			// non-user initiated, non previously allowed, return false
			Assert.IsFalse (CaptureDeviceConfiguration.RequestDeviceAccess (), "RequestDeviceAccess");
		}

		[TestMethod]
		public void AudioCaptureDevice ()
		{
			ReadOnlyCollection<AudioCaptureDevice> roc = CaptureDeviceConfiguration.GetAvailableAudioCaptureDevices ();
			Assert.IsNotNull (roc, "GetAvailableAudioCaptureDevices");
			// results are not cached
			Assert.AreNotSame (roc, CaptureDeviceConfiguration.GetAvailableAudioCaptureDevices (), "!same-collection");

			AudioCaptureDevice acd = CaptureDeviceConfiguration.GetDefaultAudioCaptureDevice ();
			if (acd != null) {
				// a new instance of AudioCaptureDevice is returned
				Assert.IsFalse (roc.Contains (acd), "Contains(Default)");
				Assert.AreNotSame (acd, CaptureDeviceConfiguration.GetDefaultAudioCaptureDevice (), "!same");
			} else {
				// no default then collection should be empty
				Assert.AreEqual (0, roc.Count, "Empty");
			}
		}

		[TestMethod]
		public void VideoCaptureDevice ()
		{
			ReadOnlyCollection<VideoCaptureDevice> roc = CaptureDeviceConfiguration.GetAvailableVideoCaptureDevices ();
			Assert.IsNotNull (roc, "GetAvailableVideoCaptureDevices");
			// results are not cached
			Assert.AreNotSame (roc, CaptureDeviceConfiguration.GetAvailableVideoCaptureDevices (), "!same-collection");

			VideoCaptureDevice vcd = CaptureDeviceConfiguration.GetDefaultVideoCaptureDevice ();
			if (vcd != null) {
				// a new instance of VideoCaptureDevice is returned
				Assert.IsFalse (roc.Contains (vcd), "Contains(Default)");
				Assert.AreNotSame (vcd, CaptureDeviceConfiguration.GetDefaultVideoCaptureDevice (), "!same");
			} else {
				// no default then collection should be empty
				Assert.AreEqual (0, roc.Count, "Empty");
			}
		}
	}
}

