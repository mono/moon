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
// Copyright (c) 2007-2008 Novell, Inc.
//
// Authors:
//	Jackson Harper (jackson@ximian.com)
//


using System;
using System.Collections;

namespace MoonlightTests {

	public class TestRun {

		private DateTime start_time;

		private VerboseLevel verbose_level;
		private LoggingServer logging_server;

		private ArrayList executed_tests = new ArrayList ();
		private ArrayList passed_tests = new ArrayList ();
		private ArrayList ignored_tests = new ArrayList ();
		private ArrayList failed_tests = new ArrayList ();
		private ArrayList known_failures = new ArrayList ();

		public TestRun (VerboseLevel verbose_level, LoggingServer logging_server)
		{
			start_time = DateTime.Now;

			this.verbose_level = verbose_level;
			this.logging_server = logging_server;
		}

		public VerboseLevel VerboseLevel {
			get { return verbose_level; }
		}

		public LoggingServer LoggingServer {
			get { return logging_server; }
		}

		public DateTime StartTime {
			get { return start_time; }
		}

		public IList ExecutedTests {
			get { return executed_tests; }
		}

		public IList PassedTests {
			get { return passed_tests; }
		}

		public IList IgnoredTests {
			get { return ignored_tests; }
		}

		public IList FailedTests {
			get { return failed_tests; }
		}

		public IList KnownFailures {
			get { return known_failures; }
		}
	}
}

