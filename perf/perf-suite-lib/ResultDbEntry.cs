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

	public class ResultDbEntry : DbEntry {

		public ItemDbEntry Item;
		public PassDbEntry Pass;
		public long Time;

		public bool Failure {
			get { return (Time == 0); }
		}

		public ResultDbEntry (IDataReader reader, int index, ItemDbEntry item, PassDbEntry pass)
		{
			id = Convert.ToInt32 (reader [0 + index]);
			Item = item;
			Pass = pass;
			Time = Convert.ToInt64 ((string) reader [3 + index]);
		}

		public ResultDbEntry ()
		{
		}

		public override void CreateCommand (ref IDbCommand command)
		{
			AddParameter (command, ":it", Item.Id.ToString ());
			AddParameter (command, ":pa", Pass.Id.ToString ());
			AddParameter (command, ":t", Time.ToString ());

			command.CommandText = ("INSERT INTO results VALUES " +
					       "(null, :it, :pa, :t)");
		}

		public override bool IsValid ()
		{
			if (Item == null)
				return false;

			if (Pass == null)
				return false;

			if (Time < 0)
				return false;

			return true;
		}

	}

}


