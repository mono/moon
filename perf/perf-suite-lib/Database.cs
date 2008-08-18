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
using System.Collections.Generic;
using Mono.Data.SqliteClient;
using System.Data;

namespace PerfSuiteLib {

	public static class Database {
	
		static SqliteConnection connection;

		public static void Initialize ()
		{
			connection = new SqliteConnection ("URI=file:results.db"); 
			connection.Open ();

			if (CheckDatabaseVersion ())
				CreateTables ();
		}

		public static void Put (DbEntry entry)
		{
			if (entry.IsValid () == false)
				throw new Exception ("Can't put invalid entry in the database!");

			if (entry.IsInTheDatabase)
				throw new Exception ("Entry already in the database!");

			IDbCommand cmd = connection.CreateCommand ();
			entry.CreateCommand (ref cmd);

			cmd.ExecuteNonQuery ();
			entry.GiveId (connection.LastInsertRowId);
		}

		private static void CreateTables ()
		{
			Console.WriteLine ("*** Creating database tables...");
			ExecuteCreateCommand ("CREATE TABLE meta (version INTEGER)");
			ExecuteCreateCommand ("INSERT INTO meta VALUES ('1')");
		
			ExecuteCreateCommand ("CREATE TABLE passes (id INTEGER PRIMARY KEY, description TEXT, date TEXT)");
		}

		private static void ExecuteCreateCommand (string cmdString)
		{
			IDbCommand cmd = connection.CreateCommand ();
			cmd.CommandText = cmdString;
			cmd.ExecuteNonQuery ();
		}

		private static bool CheckDatabaseVersion ()
		{
			// FIXME Man, that's ugly
			IDbCommand cmd = connection.CreateCommand ();
			cmd.CommandText = "SELECT * from meta";	
			IDataReader reader;
			try {
				reader = cmd.ExecuteReader ();
				if (! reader.Read ())
					return true;
			} catch { 
				return true;
			}

			if (reader [0].ToString () != "1") 
				throw new Exception ("Incompatible database version!");

			return false;
		}

	}
}


