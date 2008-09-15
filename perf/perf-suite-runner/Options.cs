/*
 * Copyright (c) 2008 Novell, Inc. (http://www.novell.com)
 *
 * Contact:
 *  Moonlight List (moonlight-list@lists.ximian.com)
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

using System;
using System.IO;
using System.Xml;
using PerfSuiteLib;
using Mono.GetOptions;

namespace PerfSuiteRunner {

	public class Options : Mono.GetOptions.Options {

		[Option ("A test run short name (ie. revision number)", 'n', "short-name")]
		public string ShortName = "Unknown";

		[Option ("Author of the commit (or last change)", 'a', "author")]
		public string Author = String.Empty;

		[Option ("Changelog entry related to this pass", 'c', "changelog")]
		public string ChangeLog = String.Empty;

		[Option ("Location of the file with the database", 'd', "database")]
		public string DatabaseFile = "perf-results.db";

		[Option ("Test id to run (forces single-test mode)", 'i', "id")]
		public string TestId = "";

		public Options ()
		{
			base.ParsingMode = OptionsParsingMode.Both;

			/* Try getting defaults from env vars */
			ShortName = GetEnvVarIfPresentOrDefault ("PERF_SHORT_NAME", ShortName);
			Author = GetEnvVarIfPresentOrDefault ("PERF_AUTHOR", Author);
			ChangeLog = GetEnvVarIfPresentOrDefault ("PERF_CHANGE_LOG", ChangeLog);
			DatabaseFile = GetEnvVarIfPresentOrDefault ("PERF_DATABASE_FILE", DatabaseFile);
			TestId = GetEnvVarIfPresentOrDefault ("PERF_TEST_ID", TestId);
		}

		private string GetEnvVarIfPresentOrDefault (string var, string def)
		{
			string val = Environment.GetEnvironmentVariable (var);

			if (val != null)
				return val;
			else
				return def;
		}

	}

}


