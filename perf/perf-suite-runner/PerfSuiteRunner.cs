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

	public static class PerfSuiteRunner {

		public static int AllTestsMode (Options opts)
		{
			Console.WriteLine ("*** Pass name is '{0}'...", opts.ShortName);

			Database.Initialize (opts.DatabaseFile);
			Database.BeginTransaction ();

			PassDbEntry passEntry = new PassDbEntry ();
			passEntry.ShortName = opts.ShortName;
			passEntry.Author = opts.Author;
			passEntry.ChangeLog = opts.ChangeLog;
			passEntry.Date = DateTime.Now;

			Database.Put (passEntry);

			DrtStore store = new DrtStore ("perf-suite-set/drtlist.xml");
			foreach (DrtItem item in store.Items) {
				Console.WriteLine ("*** Running [{0}]", item);

				ItemDbEntry itemEntry = Database.GetItemEntryByUniqueId (item.UniqueId);
				if (itemEntry == null) {
					Console.WriteLine ("*** [{0}] not yet in the database, adding...", item);
					itemEntry = new ItemDbEntry ();
					itemEntry.UniqueId = item.UniqueId;
					itemEntry.Name = item.Name;
					itemEntry.InputFile = item.InputFile;
					Database.Put (itemEntry);
				}
			
				DrtResult r = item.Run ();
				
				ResultDbEntry resultEntry = new ResultDbEntry ();
				resultEntry.Pass = passEntry;
				resultEntry.Item = itemEntry;

				if (r == null) {
					resultEntry.Time = 0;
					Console.WriteLine ("*** Averaged result: 0 (FAILURE)");
				} else {
					resultEntry.Time = r.AveragedTime;
					Console.WriteLine ("*** Averaged result: {0}usec", r.AveragedTime);
				}
				
				Database.Put (resultEntry);
			}

			Database.CommitTransaction ();

			return 0;

		}

		public static int SingleTestMode (Options opts)
		{
			Console.WriteLine ("*** Running single test with id '{0}' not storing results in database...", opts.TestId);

			DrtStore store = new DrtStore ("perf-suite-set/drtlist.xml");
			DrtItem item = store.GetDrtItemForId (opts.TestId);
			
			if (item == null) {
				Console.WriteLine ("*** Test '{0}' not found!", opts.TestId);
				return 128;
			}

			Console.WriteLine ("*** Running [{0}]", item);

			DrtResult r = item.Run ();
				
			ResultDbEntry resultEntry = new ResultDbEntry ();

			if (r == null) {
				resultEntry.Time = 0;
				Console.WriteLine ("*** Averaged result: 0 (FAILURE)");
			} else {
				resultEntry.Time = r.AveragedTime;
				Console.WriteLine ("*** Averaged result: {0}usec", r.AveragedTime);
			}
				
			return 0;

		}

		public static int Main (string [] args)
		{
			Options opts = new Options ();
			opts.ProcessArgs (args);

			if (opts.TestId != String.Empty)
				return SingleTestMode (opts);
			else
				return AllTestsMode (opts);

		}

	}

}


