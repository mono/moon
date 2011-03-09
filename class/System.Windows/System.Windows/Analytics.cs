//
// Copyright 2009 Novell, Inc.
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
using System.Windows;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows.Threading;

namespace System.Windows
{

	public class Analytics {
		System.Windows.Threading.DispatcherTimer timer;
		AnalyticsProvider provider;

		public Analytics ()
		{
			provider = new AnalyticsLinux ();
			timer = new System.Windows.Threading.DispatcherTimer ();
			timer.Interval = TimeSpan.FromSeconds (1);
			timer.Tick += HandleTimerTick;
			timer.Start ();
		}

		~Analytics ()
		{
			timer.Stop ();
		}

		void HandleTimerTick (object sender, EventArgs e)
		{
			try {
				Sample ();
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Analytics.HandleTimerTick (): unexpected exception: {0}", ex);
				} catch (Exception ex2) {
					// ignore
				}
			}
		}

		void Sample ()
		{
			provider.Sample ();
		}

		public float AverageProcessLoad {
			get {
				float result = provider.AverageProcessLoad * 100;
				result = Math.Min (result, 100);
				result = Math.Max (result, 0);
				return result;
			}
		}

		public float AverageProcessorLoad {
			get {
				float result = provider.AverageProcessorLoad * 100;
				result = Math.Min (result, 100);
				result = Math.Max (result, 0);
				return result;
			}
		}

		public ReadOnlyCollection<GpuInformation> GpuCollection {
			get {
				List<GpuInformation> list = new List<GpuInformation>();

				GpuInformation info = new GpuInformation ();
				info.DeviceId = 0;
				info.DriverVersion = "bogus";
				info.VendorId = 0;

				return new ReadOnlyCollection<GpuInformation>(list);
			}
		}
	}
}
