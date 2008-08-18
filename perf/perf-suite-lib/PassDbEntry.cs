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
using System.Data;

namespace PerfSuiteLib {

	public class PassDbEntry : DbEntry {

		public string ShortName = String.Empty;
		public string Author = String.Empty;
		public string ChangeLog = String.Empty;
		public DateTime Date;

		public PassDbEntry (IDataReader reader, int index)
		{
			id = Convert.ToInt32 (reader [0 + index]);
			ShortName = (string) reader [1 + index];
			Author = (string) reader [2 + index];
			ChangeLog = (string) reader [3 + index];
			Date = new DateTime (Convert.ToInt64 ((string) reader [4 + index]));
		}

		public PassDbEntry ()
		{
		}

		public override void CreateCommand (ref IDbCommand command)
		{
			AddParameter (command, ":sn", ShortName);
			AddParameter (command, ":a", Author);
			AddParameter (command, ":cl", ChangeLog);
			AddParameter (command, ":d", Date.Ticks);

			command.CommandText = ("INSERT INTO passes VALUES " +
					       "(null, :sn, :a, :cl, :d)");
		}

		public override bool IsValid ()
		{
			if (ShortName == String.Empty)
				return false;

			return true;
		}

	}
}


