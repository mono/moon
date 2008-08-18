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
		static IDbTransaction transaction;

		public static void Initialize (string file)
		{
			Console.WriteLine ("*** Initializing database from '{0}' ...", file);
			
			string uri = String.Format ("URI=file:{0}", file);
			
			connection = new SqliteConnection (uri); 
			connection.Open ();

			if (CheckDatabaseVersion ())
				CreateTables ();
		}

		public static void BeginTransaction ()
		{
			transaction = connection.BeginTransaction ();
		}

		public static void CommitTransaction ()
		{
			transaction.Commit ();
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

		public static List <ItemDbEntry> GetAllItemEntries ()
		{
			IDbCommand cmd = connection.CreateCommand ();
			cmd.CommandText = ("SELECT * FROM items");

			IDataReader reader = cmd.ExecuteReader ();
			List <ItemDbEntry> list = new List <ItemDbEntry> ();

			while (reader.Read ())
				list.Add (new ItemDbEntry (reader, 0));

			return list;
		}

		public static PassDbEntry GetLastPass ()
		{
			IDbCommand cmd = connection.CreateCommand ();
			cmd.CommandText = ("SELECT * " + 
					   "FROM passes " + 
					   "ORDER BY passes.date DESC " + 
					   "LIMIT 1");

			IDataReader reader = cmd.ExecuteReader ();
			if (! reader.Read ())
				return null;

			return new PassDbEntry (reader, 0);
		}

		public static List <ResultDbEntry> GetResultEntriesForItemEntry (ItemDbEntry item, int limit)
		{
			IDbCommand cmd = connection.CreateCommand ();
			
			cmd.CommandText = (String.Format ("SELECT results.id, results.item_id, results.pass_id, results.time, passes.id, passes.short_name, passes.author, passes.changelog, passes.date " + 
					   "FROM results, passes " + 
					   "WHERE results.pass_id = passes.id AND results.item_id = :it " + 
					   "ORDER BY passes.date DESC " + 
					   "LIMIT {0}", limit));

			IDataParameter p1 = cmd.CreateParameter ();
			p1.ParameterName = ":it";
			p1.Value = item.Id.ToString ();
			cmd.Parameters.Add (p1);

			IDataReader reader = cmd.ExecuteReader ();
			List <ResultDbEntry> list = new List <ResultDbEntry> ();

			while (reader.Read ()) {
				PassDbEntry pass = new PassDbEntry (reader, 4);
				list.Add (new ResultDbEntry (reader, 0, item, pass));
			}

			return list;
		}

		public static ItemDbEntry GetItemEntryByUniqueId (string uniqueId)
		{
			IDbCommand cmd = connection.CreateCommand ();
			cmd.CommandText = ("SELECT * FROM items " +
					   "WHERE unique_id = :uq");

			IDataParameter p = cmd.CreateParameter ();
			p.ParameterName = ":uq";
			p.Value = uniqueId;
			cmd.Parameters.Add (p);

			IDataReader reader = cmd.ExecuteReader ();
			if (! reader.Read ())
				return null;
			else {
				return new ItemDbEntry (reader, 0);
			}
		}

		private static void CreateTables ()
		{
			Console.WriteLine ("*** Creating database tables...");
			ExecuteCreateCommand ("CREATE TABLE meta (version INTEGER)");
			ExecuteCreateCommand ("INSERT INTO meta VALUES ('1')");
		
			ExecuteCreateCommand ("CREATE TABLE passes (id INTEGER PRIMARY KEY, short_name TEXT, author TEXT, changelog TEXT, date TEXT)");
			ExecuteCreateCommand ("CREATE TABLE items (id INTEGER PRIMARY KEY, unique_id TEXT, name TEXT, input_file TEXT)");
			ExecuteCreateCommand ("CREATE TABLE results (id INTEGER PRIMARY KEY, item_id INTEGER, pass_id INTEGER, time INTEGER)");
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


