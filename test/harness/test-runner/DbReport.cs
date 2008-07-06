using System;
using System.IO;
using System.Data;
using System.Collections.Generic;
using Mono.Data.SqliteClient;
using System.Diagnostics;


namespace MoonlightTests {
	
	public class DbReport : IReport {
		private IDbConnection dbcon = null;
		private IDbCommand dbcmd = null;
		private bool inited = false;
		private string revision = string.Empty;
		private string runtime = string.Empty;
		private string masters = "masters";
		
		private string test_run_dir;
		
		public string Revision
		{
			get { return revision; }
			set { revision = value; }
		}
		
		
		public DbReport()
		{
		}
#region IReport members
		public void BeginRun(TestRun run)
		{
			
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
			
			string connectionString = string.Format("URI=file:{0}",Path.Combine(dir,filename));
			
			dbcon = (IDbConnection) new SqliteConnection(connectionString);
			dbcon.Open();
			dbcmd = dbcon.CreateCommand();
			
			AddRunToDB();
		}
		
		public void EndRun()
		{
		}
		public void Executing(Test test)
		{
		}
		
		public void AddResult(Test test, TestResult result)
		{
			
			//Console.WriteLine(test);
			
			// copy result file to test run dir
			// copy master file to test run dir
			
			string masterfile = Path.Combine(masters, Path.GetFileName(test.MasterFile));
			string renderfile = Path.Combine(runtime, Path.GetFileName(test.ResultFile));
			//string build = string.Empty;
			
			
			string result_file = XmlReport.GetFilePath (test.ResultFile);
			string master_file = XmlReport.GetFilePath (test.MasterFile);

			/*
			Console.WriteLine("masterfile = " + masterfile);
			Console.WriteLine("renderfile = " + renderfile);
			
			Console.WriteLine("result_file = " + result_file);
			Console.WriteLine("master_file = " + master_file);
			*/
			
			
			XmlReport.CopyImageToRunDirectory(test_run_dir,result_file);
			XmlReport.CopyImageToRunDirectory(Path.Combine("test-run-data",masters), master_file);
			
			if (masterfile.EndsWith("tif") || masterfile.EndsWith("tiff")) {
				masterfile += ".png";
				renderfile += ".png";
			}
			
			
			string query = string.Format("INSERT INTO testcases VALUES ('{0}','{1}','{2}');",test.Id, string.Empty, masterfile);
			execnonquery(query);
			
			query = string.Format("INSERT INTO results VALUES ('{0}','{1}','{2}','{3}');",test.Id, runtime, result.ToString(), renderfile);
			execnonquery(query);
			
		}
#endregion
		
		public void Close()
		{
			dbcmd.Dispose();
			dbcmd = null;
			dbcon.Close();
			dbcon = null;
		}

		public void AddBuild(string revision, string time)
		{
			string query = string.Format("INSERT INTO builds VALUES ('{0}','{1}');",revision,time);
			//execnonquery(query);
			dbcmd.CommandText = query;
			try 			{
				dbcmd.ExecuteNonQuery();
			}
			catch(Exception ex) {
				Console.WriteLine(ex);
			}
		}
		private void execnonquery(string query)
		{
			try {
				dbcmd.CommandText = query;
				dbcmd.ExecuteNonQuery();
			}
			catch(Exception ex) {
				Console.WriteLine("{0}: {1}",ex.GetType().ToString(), query);
			}
		}
		
		private void AddRunToDB()
		{
			ProcessStartInfo info = new ProcessStartInfo();
			info.FileName = "svn";
			info.Arguments = "info";			
			info.UseShellExecute = false;		
			info.RedirectStandardOutput = true;			
			Process p = new Process();
			p.StartInfo = info;
			p.Start();
			
			string output = p.StandardOutput.ReadToEnd();
			//string revision = string.Empty;
			string[] lines = output.Split('\n');
			//string time = string.Empty;
			
			foreach (string line in lines) {
				if (line.StartsWith("Revision")) {
					revision = line.Split(':')[1].Trim();
				}				
			}
			AddBuild(revision,runtime);
			
			//Console.ReadLine();
				
		
		}
	}
}
