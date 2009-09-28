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

using System.Windows;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace System.Windows
{

	public class Analytics {

		~Analytics ()
		{
		}

		public float AverageProcessLoad {
			get { return 0; }
		}

		public float AverageProcessorLoad {
			get { return 0; }
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