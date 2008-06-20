using System;
using System.Data;
using Mono.Data.SqliteClient;
using System.IO;


namespace MoonlightTests
{
	public class SqliteStore
	{
		static string dbFileName = "db_MoonlightTests.db";
		IDbConnection dbcon;
		IDbCommand dbcmd;

		public SqliteStore()
		{
			string constring = "URI=file:" + dbFileName;
			dbcon = (IDbConnection) new SqliteConnection(constring);
			dbcmd = dbcon.CreateCommand();
			
			CreateNewDatabase();
			
		}


		public void CreateNewDatabase()
		{
			string sql = "CREATE TABLE testcase (id INTEGER PRIMARY KEY, description VARCHAR(256));";
			
		}


	}
}
