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
//>-Rusty Howell (rhowell@novell.com)
//


using System;
using System.IO;
using System.Data;
using System.Collections.Generic;
using System.Diagnostics;
using MySql.Data.MySqlClient;


namespace MoonlightTests {
	
	public class DbReport : IReport {
		
		private string connectionString = string.Empty;
		private IDbConnection dbcon = null;
		private IDbCommand dbcmd = null;
		
		private string runtime;
		private int runtimeid;
		private string masters = "masters";
		
		private string test_run_dir;
		
		public bool HasConnection
		{
			get {return (connectionString != string.Empty);}
		}
		
		public DbReport()
		{
			try
			{
				StreamReader reader = new StreamReader(".dbconnection.txt");
				connectionString = reader.ReadLine();
				reader.Close();
				
				//test out the 
				dbcon =  new MySqlConnection(connectionString);
				dbcon.Open();
				dbcmd = dbcon.CreateCommand();
				string query = "select id from testcases;";
				
				dbcmd.CommandText = query;
				IDataReader dbreader = dbcmd.ExecuteReader();
				while(dbreader.Read())
				{
					int id = dbreader.GetInt32(0);
					Console.WriteLine("reading testcase {0}",id);
				}
				dbreader.Close();
				dbreader = null;
				
				Console.WriteLine("MySql connection successful");
			
			}
			catch(Exception ex)
			{
				Console.WriteLine(ex.Message);
				WriteHelp();
				connectionString = string.Empty;
			}
			
		}
		private void WriteHelp()
		{
			
			string str = "\nDbReport is disabled because of missing file or invalid connection string\n";
			str += "\nCreate file .dbconnection.txt with the first line being the connection string to use.";
			str += "\nServer=mysqlserver.company.com;Database=moonlight;User ID=root;Password=password;Pooling=false;\n";
			
			Console.WriteLine(str);
				
		}
#region IReport members
		public void BeginRun(TestRun run)
		{
			
			if (!HasConnection)
			{
				WriteHelp();
				return;
			}
				
			
			string dir = "test-run-data";
			string filename = "moonTestSuite.db";
			
			runtime = run.StartTime.ToString("yyyy-MM-dd-hh-mm");
			test_run_dir = Path.Combine(dir,runtime);
			
			if (!Directory.Exists(test_run_dir)) {
				Directory.CreateDirectory(test_run_dir);
			}
			if (!Directory.Exists(Path.Combine(dir,masters))) {
				Directory.CreateDirectory(Path.Combine(dir,masters));
			}
			
			AddBuild(runtime);
		}
		
		public void EndRun()
		{
		}
		public void Executing(Test test)
		{
		}
		
		public void AddResult(Test test, TestResult result)
		{
			if (!HasConnection)
			{
				return;
			}
				
			AddTags(test);
			string info = string.Empty;
			
			string testname = test.InputFileName.Split('.')[0];
			string masterfile = Path.Combine(masters, Path.GetFileName(test.MasterFile));
			string renderfile = Path.Combine(runtime, Path.GetFileName(test.ResultFile));
			
			string result_file = XmlReport.GetFilePath (test.ResultFile);
			string master_file = XmlReport.GetFilePath (test.MasterFile);

			/*
			Console.WriteLine("masterfile = " + masterfile);
			Console.WriteLine("renderfile = " + renderfile);
			
			Console.WriteLine("result_file = " + result_file);
			Console.WriteLine("master_file = " + master_file);
			//*/
			
			
			XmlReport.CopyImageToRunDirectory(test_run_dir,result_file);
			XmlReport.CopyImageToRunDirectory(Path.Combine("test-run-data",masters), master_file);
			
			if (masterfile.EndsWith("tif") || masterfile.EndsWith("tiff")) {
				masterfile += ".png";
				renderfile += ".png";
			}
			
			AddTestCase(test.Id,testname,masterfile);
			
			switch(result)
			{
			case TestResult.Fail:
				info = test.FailedReason;
				break;
			case TestResult.Ignore:
				info = test.IgnoreReason;
				break;
			case TestResult.KnownFailure:
				info = test.KnownFailureReason;
				break;
			default:
				info = string.Empty;
				break;
				
			}		
			
			string query = string.Format("INSERT INTO results VALUES ('{0}','{1}','{2}','{3}', '{4}');",test.Id, runtimeid, result.ToString(), renderfile, info);
			execnonquery(query);
			
		}
		private void AddTestCase(string id, string testname, string masterfile)
		{
			int intid = Convert.ToInt32(id);
			string query = string.Format("Select id from testcases where id={0}",intid);
			
			dbcmd.CommandText = query;
			IDataReader reader = dbcmd.ExecuteReader();
			
			// If the test case is not found, add it
			if(!reader.Read())
			{		
				reader.Close();
				query = string.Format("INSERT INTO testcases VALUES ({0},'{1}','{2}');",intid, testname, masterfile);
				execnonquery(query);
			}
			reader.Close();
			reader = null;
			
		}
#endregion
		
		public void Close()
		{
			dbcmd.Dispose();
			dbcmd = null;
			dbcon.Close();
			dbcon = null;
		}

		private void AddBuild(string time)
		{
			
			string revision = GetSubersionRevision();
			
			string query = string.Format("INSERT INTO builds VALUES ('','{0}','{1}');",revision,time);
			//execnonquery(query);
			dbcmd.CommandText = query;
			try 			{
				dbcmd.ExecuteNonQuery();
			}
			catch(Exception ex) {
				Console.WriteLine(ex);
			}
			IDataReader reader = execreader(string.Format("select id from builds where revision='{0}' and runtime='{1}';",revision,time));
			if (reader.Read())
			{
				this.runtimeid = reader.GetInt32(0);
			}
			reader.Close();
			
			
			                                
		}
		private void AddTags(Test test)
		{
			foreach(string tag in test.Categories)
			{
				string query = string.Format("SELECT id FROM tags WHERE name = '{0}';",tag.ToLower());
				dbcmd.CommandText = query;
				IDataReader reader = null;
				int tagid = int.MinValue;
				try
				{
					reader = dbcmd.ExecuteReader();
					reader.Read();
					tagid = reader.GetInt32(0);
					
				}
				catch(Exception ex) {
					Console.WriteLine("Tag exists:" + tag);
					tagid = int.MinValue;
				}
				finally {
					if (reader != null)
					{
						reader.Close();
						reader = null;
					}
				}
					
				if (tagid == int.MinValue) {
					try {
						query = string.Format("INSERT INTO tags VALUES ('','{0}');",tag.ToLower());
						execnonquery(query);					
						
						query = string.Format("SELECT id FROM tags WHERE name = '{0}';",tag.ToLower());
						dbcmd.CommandText = query;
						reader = dbcmd.ExecuteReader();
						reader.Read();
						tagid = reader.GetInt32(0);
						reader.Close();
					}
					catch (Exception ex) {
						
						Console.WriteLine("ADDTAG " +ex.Message);
					}
					finally{
						if (reader != null) {
							reader.Close();
							reader = null;
						}
					}
				}
				
				AddTaggedCase(test.Id,tagid);
			}
		}
		private void AddTaggedCase(string testid, int tagid)
		{
			string query = string.Format("select testcaseid,tagid from taggedcases where testcaseid={0} and tagid={1};",testid,tagid);
			IDataReader reader = execreader(query);
			if (reader.Read()) {
				reader.Close();
				Console.WriteLine("tagged case EXISTS for testcaseid={0} and tagid={1};",testid,tagid);
			}
			else {
				reader.Close();
				query = string.Format("INSERT INTO taggedcases values ({0},'{1}');",testid,tagid);				
				execnonquery(query);
			}
			reader = null;
				
		}
		private void execnonquery(string query)
		{
			try {
				dbcmd.CommandText = query;
				dbcmd.ExecuteNonQuery();
			}
			catch(Exception ex) {
				Console.WriteLine("EXECQUERY {0}: {1}",ex.GetType().ToString(), ex.Message);
				Console.WriteLine("{0}",query);
			}
		}
		private IDataReader execreader(string query)
		{
			try {
				dbcmd.CommandText = query;
				IDataReader reader = dbcmd.ExecuteReader();
				return reader;
			}
			catch(Exception ex) {
				Console.WriteLine("NONEXEC {0}: {1}",ex.GetType().ToString(), query);
				return null;
			}
		}
		
		private string GetSubersionRevision()
		{
			
			ProcessStartInfo info = new ProcessStartInfo();
			info.FileName = "svn";
			info.Arguments = "info";			
			info.UseShellExecute = false;		
			info.RedirectStandardOutput = true;			
			Process p = new Process();
			p.StartInfo = info;
			p.Start();

			string revision = string.Empty;
			string output = p.StandardOutput.ReadToEnd();
			//string revision = string.Empty;
			string[] lines = output.Split('\n');
			//string time = string.Empty;
			
			foreach (string line in lines) {
				if (line.StartsWith("Revision")) {
					revision = line.Split(':')[1].Trim();
				}				
			}
			return revision;
			//Console.ReadLine();
				
		
		}
	}
}
