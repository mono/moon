using System;
using System.IO;
using System.Data;
using System.Collections.Generic;
using Mono.Data.SqliteClient;


namespace MoonlightTests
{
	
	public class SqliteData
	{
		private static SqliteData _instance = null;
		private IDbConnection dbcon = null;
		private IDbCommand dbcmd = null;
		private bool inited = false;
		private string revision = string.Empty;
		
		public string Revision
		{
			get { return revision; }
			set { revision = value; }
		}
		
		private SqliteData()
		{
			Init();
		}
		public static SqliteData GetInstance()
		{
			if (_instance == null)
				_instance = new SqliteData();
			return _instance;
		}
		private void Init()
		{
			string dir = "/var/tmp";
			string filename = "moonlightTests.db";
			
			if (!Directory.Exists(dir))
			{
				Directory.CreateDirectory(dir);
			}
			string connectionString = string.Format("URI=file:{0}",Path.Combine(dir,filename));
			
			dbcon = (IDbConnection) new SqliteConnection(connectionString);
			dbcon.Open();
			dbcmd = dbcon.CreateCommand();

			ImportSchema();
			
		}
		public void Close()
		{
			dbcmd.Dispose();
			dbcmd = null;
			dbcon.Close();
			dbcon = null;
			_instance = null;
		}

		private void ImportSchema()
		{
			
			List<string> tables = new List<string>();
			tables.Add("CREATE TABLE builds ( revision INTEGER PRIMARY KEY, builddate DATE);");
			tables.Add("CREATE TABLE testcases ( id INTEGER PRIMARY KEY, desc VARCHAR(255), master VARCHAR(255));");
			tables.Add("CREATE TABLE runs ( testcaseid INTEGER, revision INTEGER, status VARCHAR(32), rendered VARCHAR(255));");
			tables.Add("CREATE TABLE taggedcases ( testcaseid INTEGER, tagid INTEGER );");
			tables.Add("CREATE TABLE tags ( id INTEGER PRIMARY KEY, name VARCHAR(32));");

			foreach(string sql in tables)
			{
				execnonquery(sql);
			}

			inited = true;
		}
		public void LogRun(string testid, TestResult result)
		{
			LogRun(testid, Revision, result);
		}
		public void LogRun(string testid, string build, TestResult result)
		{
			string query = string.Format("INSERT INTO runs VALUES ('{0}','{1}','{2}','');",testid, build, result.ToString());
			execnonquery(query);
			//dbcmd.CommandText = query;
			//dbcmd.ExecuteNonQuery();
		}
		public void AddTestCase(string testid, string desc, string masterfile)
		{
			string query = string.Format("INSERT INTO testcases VALUES ('{0}','{1}','{2}');",testid, desc, masterfile);
			execnonquery(query);
			//dbcmd.CommandText = query;
			//dbcmd.ExecuteNonQuery();
		}
		public void AddBuild(string revision, string time)
		{
			string query = string.Format("INSERT INTO builds VALUES ('{0}','{1}');",revision,time);
			//execnonquery(query);
			dbcmd.CommandText = query;
			try
			{
				dbcmd.ExecuteNonQuery();
			}
			catch(Exception ex)
			{
				Console.WriteLine(ex);
			}
		}
		private void execnonquery(string query)
		{
			try
			{
				dbcmd.CommandText = query;
				dbcmd.ExecuteNonQuery();
			}
			catch(SqliteExecutionException ex)
			{
				Console.WriteLine("Bad command: {0}",query);
				//Console.WriteLine(ex.ToString());
			}
		}
		


	}
}
