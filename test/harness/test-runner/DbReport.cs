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
using Npgsql;


namespace MoonlightTests {
	
	public class DbReport : IReport {
		
		private string connectionString = string.Empty;
		private IDbConnection dbcon = null;
		private IDbCommand dbcmd = null;
		
		private string runtime;
		private int runtimeid;
		private string masters = "masters";
		
		private string test_run_dir;
		private string test_suite = string.Empty;
		private bool debug = false;
		
		public bool HasConnection
		{
			get {return (connectionString != string.Empty);}
		}
		
		public DbReport()
		{

			try {
				test_suite = Environment.GetEnvironmentVariable ("MS_TEST_SUITE");
			
				if ((test_suite != null) && (test_suite.Trim().ToLower() == "true")) {
					test_suite = "ms";
					throw new Exception("DbReport not enabled for MS test suite");// We should not log MS test suite results
				}
				else {
					test_suite = "moon";
				}
				StreamReader reader = new StreamReader(".dbconnection.txt");
				connectionString = reader.ReadLine(); // Read the first line of the file
				reader.Close();
				
				//Log(string.Format("Connecting to Postgresql server: '{0}'",connectionString));
				dbcon =  new NpgsqlConnection(connectionString);
				dbcon.Open();
				dbcmd = dbcon.CreateCommand();
				string query = "select id from testcases;";
				
				dbcmd.CommandText = query;
				IDataReader dbreader = dbcmd.ExecuteReader();
				int count = 0;
				while(dbreader.Read()) {
					int id = dbreader.GetInt32(0);
					++count;
				}
				Log(string.Format("{0} test cases read", count));
				dbreader.Close();
				dbreader = null;
				
				Console.WriteLine("\nDbReport enabled\n");
			
			}
			catch(Exception ex) {
				Console.WriteLine("\nDbReport disabled\n"); // Use Console.WriteLine() so that this line always appears
				WriteHelp();
				Log(string.Format("\n{1} {0}",ex.GetType(), ex.Message));
				connectionString = string.Empty;
			}
			
		}
		private void WriteHelp()
		{
			
			string str = "\nDbReport is disabled because of missing file or invalid connection string\n";
			str += "\nCreate file moon/test/.dbconnection.txt with the first line being the connection string to use.";
			str += "\nServer=mysqlserver.company.com;Database=db;User ID=user;Password=password;Pooling=false;\n";
			
			Log(str);
				
		}
#region IReport members
		public void BeginRun(TestRun run)
		{
			
			if (!HasConnection) {
				return;
			}
				
			
			string dir = "test-run-data";
			string filename = "moonTestSuite.db";
			
			runtime = run.StartTime.ToString("yyyy-MM-dd HH:mm");//Postgres timestamp format
			test_run_dir = Path.Combine(dir,runtime);
			
			if (!Directory.Exists(test_run_dir)) {
				Directory.CreateDirectory(test_run_dir);
			}
			if (!Directory.Exists(Path.Combine(dir,masters))) {
				Directory.CreateDirectory(Path.Combine(dir,masters));
			}


			Log(string.Format("Runtime: {0}", runtime));
			
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
			if (!HasConnection) {
				return;
			}
				
			AddTags(test);
			string info = string.Empty;
			
			string testname = test.InputFileName.Split('.')[0];
			string masterfile = Path.Combine(masters, Path.GetFileName(test.MasterFile));
			string renderfile = Path.Combine(runtime, Path.GetFileName(test.ResultFile));
			
			string result_file = XmlReport.GetFilePath (test.ResultFile);
			string master_file = XmlReport.GetFilePath (test.MasterFile);

			//Log("masterfile = " + masterfile);
			//Log("renderfile = " + renderfile);
			
			//Log("result_file = " + result_file);
			//Log("master_file = " + master_file);
			
			
			XmlReport.CopyImageToRunDirectory(test_run_dir,result_file);
			XmlReport.CopyImageToRunDirectory(Path.Combine("test-run-data",masters), master_file);
			
			if (masterfile.EndsWith("tif") || masterfile.EndsWith("tiff")) {
				masterfile += ".png";
				renderfile += ".png";
			}
			
			int internal_id = AddTestCase(test.Id,testname,masterfile,test_suite);
			
			switch(result) {

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

			string query = string.Format("INSERT INTO results (internal_id,runtimeid,result,renderfile,info) VALUES ('{0}','{1}','{2}','{3}', '{4}');",internal_id, runtimeid, result.ToString(), renderfile, info);
			execnonquery(query);

		}
		private int AddTestCase(string id, string testname, string masterfile, string suite)
		{
			int intid = Convert.ToInt32(id);
			int internal_id = -1;

			string query = string.Format("Select internal_id from testcases where id={0} and suite='{1}'",intid,suite);
			
			dbcmd.CommandText = query;
			IDataReader reader = dbcmd.ExecuteReader();
			
			// If the test case is not found, add it
			if(!reader.Read()) {
				reader.Close();
				query = string.Format("INSERT INTO testcases (id, suite, name, masterfile) VALUES ({0},'{1}','{2}','{3}');",intid, suite, testname, masterfile);
				Log(query);
				execnonquery(query);
			}
			else {
				internal_id = reader.GetInt32(0);
				Log(string.Format("Testcase exists with internal_id={0}",internal_id));
			}
			reader.Close();
			reader = null;

			if (internal_id != -1)
				return internal_id;

			dbcmd.CommandText = string.Format("SELECT internal_id FROM testcases WHERE id={0} AND suite='{1}';",intid,suite);
			reader = dbcmd.ExecuteReader();
			reader.Read();
			
			internal_id = reader.GetInt32(0);
			reader.Close();
			reader = null;


			return internal_id;
			
		}
#endregion
		
		public void Close()
		{
			if (dbcmd != null)
				dbcmd.Dispose();
			dbcmd = null;

			if (dbcon != null)
				dbcon.Close();
			dbcon = null;
		}

		private void AddBuild(string time)
		{
			
			string revision = GetSubersionRevision();
			string arch = GetArch();
			
			string query = string.Format("INSERT INTO builds (revision, runtime, arch) VALUES ('{0}','{1}','{2}');",revision,time,arch);
			//execnonquery(query);
			dbcmd.CommandText = query;
			try {
				dbcmd.ExecuteNonQuery();
			}
			catch(Exception ex) {
				Log(ex.ToString());
				Log(query);
			}
			IDataReader reader = execreader(string.Format("select id from builds where revision={0} and runtime='{1}' and arch='{2}';",revision,time,arch));
			if ((reader != null) && reader.Read())
			{
				this.runtimeid = reader.GetInt32(0);
			}
			if (reader != null)
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
					Log("Tag exists:" + tag);
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
						query = string.Format("INSERT INTO tags (name) VALUES ('{0}');",tag.ToLower());
						execnonquery(query);					
						
						query = string.Format("SELECT id FROM tags WHERE name = '{0}';",tag.ToLower());
						dbcmd.CommandText = query;
						reader = dbcmd.ExecuteReader();
						reader.Read();
						tagid = reader.GetInt32(0);
						reader.Close();
					}
					catch (Exception ex) {
						
						Log("ADDTAG " +ex.Message);
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
			if ((reader != null) && reader.Read()) {
				reader.Close();
				//Log(string.Format("tagged case EXISTS for testcaseid={0} and tagid={1};",testid,tagid));
			}
			else {
				if (reader != null)
					reader.Close();
				query = string.Format("INSERT INTO taggedcases values ({0},'{1}');",tagid, testid);
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
				Log(string.Format("EXECQUERY {0}: {1}",ex.GetType().ToString(), ex.Message));
				Log(query);
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
				Log(string.Format("NONEXEC {0}: {1}",ex.GetType().ToString(), query));
				return null;
			}
		}

		private string GetArch()
		{
			ProcessStartInfo info = new ProcessStartInfo();
			info.FileName = "uname";
			info.Arguments = "-i";
			
			info.UseShellExecute = false;
			info.RedirectStandardOutput = true;
			Process p = new Process();
			p.StartInfo = info;
			p.Start();

			string output = p.StandardOutput.ReadToEnd();
			
			return output.Trim();
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
			string[] lines = output.Split('\n');

			try {
			
				foreach (string line in lines) {
					if (line.StartsWith("Revision")) {
						revision = line.Split(':')[1].Trim();
					}
				}
			}
			catch (Exception ex) {
				
				Log("Cannot get revision number via svn info: " + ex.Message);
				revision = string.Empty;
			}

			if (revision == string.Empty) {
				revision = Environment.GetEnvironmentVariable ("SVN_REVISION");

			}
			return revision;

		}
		private void Log(string msg)
		{
			if (debug)
				Console.WriteLine(msg);
		}
	}
}
