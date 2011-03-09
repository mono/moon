//
// Copyright 201 Novell, Inc.
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
using System.IO;

namespace Mono {
	internal abstract class AnalyticsProvider
	{
		public float AverageProcessLoad { get; protected set; }
		public float AverageProcessorLoad { get; protected set; }
		public abstract void Sample ();
	}

	internal class AnalyticsLinux : AnalyticsProvider {
		long last_proc_idle;
		long last_proc_busy;
		long last_pid_busy;
		long last_pid_ticks;
		int cpu_count;

		public override void Sample ()
		{
			SampleCpu ();
			SampleProcess ();
		}

		void SampleProcess ()
		{
			string line = null;
			string [] fields;
			long cur_pid_busy = 0;
			long cur_pid_ticks = 0;
			
			using (FileStream fs = new FileStream ("/proc/self/stat", FileMode.Open, FileAccess.Read)) {
				using (StreamReader r = new StreamReader (fs)) {
					line = r.ReadLine ();
					fields = line.Split (new char [] {' '}, StringSplitOptions.RemoveEmptyEntries);
					cur_pid_busy = long.Parse (fields [13]) + long.Parse (fields [14]); // utime + stime according to man 5 proc
					cur_pid_ticks = DateTime.Now.Ticks;
				}
			}
	
			long busy = cur_pid_busy - last_pid_busy;
			long ticks = cur_pid_ticks - last_pid_ticks;
			if (ticks > 0 && last_pid_ticks != 0) {
				// busy = The amount of time, measured in units of USER_HZ (1/100ths of a second on most architectures, use sysconf(_SC_CLK_TCK) to obtain the right value)
				// ticks = one hundred nanoseconds or one ten-millionth of a second. There are 10,000 ticks in a millisecond.
				AverageProcessLoad = (float) ((double) (busy * 100000) / (double) (ticks * cpu_count));
			}

			/*
			Console.WriteLine ("pid load: {7} cur ticks: {0} cur busy: {1} last ticks: {2} last busy: {3} ticks: {4} busy: {5} line: {6}",
				cur_pid_ticks, cur_pid_busy, last_pid_ticks, last_pid_busy, ticks, busy, line, AverageProcessLoad);
			*/
			
			last_pid_ticks = cur_pid_ticks;
			last_pid_busy = cur_pid_busy;
		}
		
		void SampleCpu ()
		{
			string line = null;
			string [] fields;
			long cur_proc_busy = 0;
			long cur_proc_idle = 0;
			int cpu_count = 0;

			using (FileStream fs = new FileStream ("/proc/stat", FileMode.Open, FileAccess.Read)) {
				using (StreamReader r = new StreamReader (fs)) {
					while ((line = r.ReadLine ()) != null) {
						if (!line.StartsWith ("cpu ")) {
							if (line.StartsWith ("cpu"))
								cpu_count++;
							continue;
						}
						fields = line.Split (new char [] {' '}, StringSplitOptions.RemoveEmptyEntries);

						cur_proc_idle = long.Parse (fields [4]);
						cur_proc_busy = long.Parse (fields [1]) + long.Parse (fields [2]) + long.Parse (fields [3]);
					}
				}
			}

			this.cpu_count = System.Math.Max (1, cpu_count);

			long busy = cur_proc_busy - last_proc_busy;
			long idle = cur_proc_idle - last_proc_idle;
			if (busy + idle > 0 && last_proc_busy > 0) {
				AverageProcessorLoad = (float) ((double) (busy) / (double) (busy + idle));
			}

			/*
			Console.WriteLine ("load: {7} cpu count: {8} cur idle: {0} cur busy: {1} last idle: {2} last busy: {3} idle: {4} busy: {5} line: {6}",
				cur_proc_idle, cur_proc_busy, last_proc_idle, last_proc_busy, idle, busy, line, AverageProcessorLoad, cpu_count);
			*/

			last_proc_idle = cur_proc_idle;
			last_proc_busy = cur_proc_busy;
		}
	}
}

