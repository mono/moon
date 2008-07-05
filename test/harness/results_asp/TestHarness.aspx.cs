// TestHarness2.aspx.cs created with MonoDevelop
// User: rhowell at 6:13 PM 6/18/2008
//
// To change standard headers go to Edit->Preferences->Coding->Standard Headers
//

using System;
using System.Web;
using System.Web.UI;
using System.Data;
using Mono.Data.SqliteClient;
using System.Collections.Generic;

using System.Web.UI.WebControls;

namespace moonlight
{
	
	
	public partial class TestHarness : System.Web.UI.Page
	{
		private IDbCommand dbcmd;
		private IDbConnection dbcon;
		
		protected void Page_Load(object sender, EventArgs e)
		{
			FillDataTable();
			
			
		}
		private void  OpenCommand()
		{
			string connectionString = "URI=file:moonTestSuite.db";
			dbcon = (IDbConnection) new SqliteConnection(connectionString);
					
			dbcon.Open();
			dbcmd = dbcon.CreateCommand();
				
		}
		private void CloseCommand()
		{
			try
			{
				dbcmd.Dispose();
				dbcmd = null;
				dbcon.Close();
				dbcon = null;
			}
			catch
			{
			}
		}
		
		private void FillDataTable()
		{
			tblData.CssClass = "maintable";
			TableRow tr;
			TableCell tc;
			try
			{
			
				List<string> runtimes = GetBuilds();
				List<int> ids = GetTestCases();
					
				OpenCommand();
				
				foreach(int testcase in ids)
				{
					tr = new TableRow();
					
					tc = new TableCell();
					tc.Text = string.Format("&nbsp;{0}&nbsp;",testcase.ToString());
					tc.CssClass = "rowheader";
					tc.HorizontalAlign = HorizontalAlign.Center;
					tr.Cells.Add(tc);
					
					foreach(string runtime in runtimes)
					{
						tc = GetTestRun(testcase,runtime);
						tr.Cells.Add(tc);
					}
					tblData.Rows.Add(tr);
				}
				
				//debug.Text = "CloseCommand()";
				
				CloseCommand();
			}
			catch(Exception ex)
			{
				debug1.Text = ex.ToString();
			}
			
		}//end FillUserTable
		private TableCell GetTestRun(int testid, string runtime)
		{
			
			IDataReader reader = null;
			if(dbcmd == null)
				throw new Exception("dbcmd is null");
			dbcmd.CommandText = string.Format("select status from results where testcaseid = {0} and runtime = {1};",testid,runtime);
				
			string status = string.Empty;
			
			try
			{
				//debug.Text = "dbcmd.ExecuteReader()";
				reader = dbcmd.ExecuteReader();
			
				//debug.Text = "reader.Read()";
				while(reader.Read())
				{
					//debug.Text = "reader.GetString(0)";
					status = reader.GetString(0);
					//debug.Text = "reader.Close()";
				}
				//reader.Close(); 
				//reader = null;
			}
			catch
			{
				status = "";
				//if (reader
				//reader.Close();
			}
			finally
			{
				reader.Close();
				reader = null;
			}
			status = status.ToLower();
			switch(status)
			{
				case "pass":
				case "fail":
				case "ignore":
					//status = string.Format("<a href=\"http://www.google.com\"><img src=\"images/{0}.png\"></a>", status.ToLower());
					status = string.Format("<a class=\'showtest\' href=\"ShowTest.aspx?runtime={1}&testid={2}\"><img src=\"images/{0}.png\"></a>", status.ToLower(),runtime,testid);
					break;
				default:
					status = " ";
				break;
			}
			
			TableCell tc = new TableCell();
			tc.Text = status;
			return tc;
		}
		
		private List<int> GetTestCases()
		{
			List<int> testcaseids = new List<int>();
			
			try
			{
				OpenCommand();
				
				dbcmd.CommandText = "select id from testcases;";
				
				IDataReader reader = dbcmd.ExecuteReader();
				
				while(reader.Read())
				{
					int id = reader.GetInt32(0);
					testcaseids.Add(id);
				}
				reader.Close();
				reader = null;
				CloseCommand();
				
				debug2.Text = "test cases = " + testcaseids.Count;
			}
			catch (Exception ex)
			{
				debug2.Text = ex.ToString();
			}
			
			return testcaseids;
		}
		
		private List<string> GetBuilds()
		{
			List<string> builds = new List<string>();
			
			try
			{
				TableRow tr = new TableRow();
				TableCell tc = new TableCell();
				tc.Text = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
				tc.CssClass = "colheader";
				tr.Cells.Add(tc);
				
				OpenCommand();
				dbcmd.CommandText = "select runtime from builds order by runtime;";
				
				IDataReader reader = dbcmd.ExecuteReader();
				
				while(reader.Read())
				{
					string id = reader.GetString(0);
					builds.Add(id);
					tc = new TableCell();
					tc.Text = id.ToString();					
					tc.CssClass = "colheader";
					tr.Cells.Add(tc);
				}
				
				tblData.Rows.Add(tr);
				debug3.Text = "Builds: " + builds.Count;
				
				reader.Close();
				reader = null;
				CloseCommand();
				
			}
			catch (Exception ex)
			{
				debug3.Text = ex.ToString();
			}
			return builds;
			
		}
		
	}
}
