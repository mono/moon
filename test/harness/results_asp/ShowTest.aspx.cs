// ShowTest.aspx.cs created with MonoDevelop
// User: rhowell at 10:33 PM 6/23/2008
//
// To change standard headers go to Edit->Preferences->Coding->Standard Headers
//

using System;
using System.Web;
using System.Web.UI;
using System.Data;
using Mono.Data.SqliteClient;

namespace moonlight
{
	
	
	public partial class ShowTest : System.Web.UI.Page
	{
		string runtime = string.Empty;
		string testid = string.Empty;
		string status = string.Empty;
		string master = string.Empty;
		string rendered = string.Empty;
		
		private void Page_Load(object sender, EventArgs e)
		{
			runtime = this.Request.QueryString["runtime"];
			testid = this.Request.QueryString["testid"];
			
			if (runtime == null) runtime = string.Empty;
			if (testid == null) testid = string.Empty;			
			
			if ((runtime.Trim() == string.Empty) || (testid.Trim() == string.Empty))
			{
				tcBuild.Text = "Please set build and testid";
				//tcTestid.Text = "Unknown";
				return;
			}
			lblStatus.Text = string.Format("Results for test \'{0}\' on r{1}", testid, runtime);
			
			string connectionString = "URI=file:moonTestSuite.db";
			IDbConnection dbcon = (IDbConnection) new SqliteConnection(connectionString);
					
			Console.WriteLine("runtime = " + runtime);
			Console.WriteLine("testid = " + testid);
			
			
			dbcon.Open();
			IDbCommand dbcmd = dbcon.CreateCommand();
			// select runs.status, testcases.masterfile, runs.renderedfile from runs,testcases where testcases.id=1 and runs.revision=1001 and runs.testcaseid=testcases.id;
			dbcmd.CommandText = string.Format("select results.status, testcases.masterfile, results.renderedfile from results,testcases where results.testcaseid=testcases.id and testcases.id={0} and results.runtime='{1}';",testid, runtime);
			
			//Console.WriteLine(dbcmd.CommandText);
			
			IDataReader reader = dbcmd.ExecuteReader();
			reader.Read();
			
			status = reader.GetString(0);
			master = reader.GetString(1);
			rendered = reader.GetString(2);
						
			tcBuild.Text = runtime;
			tcTestid.Text = testid;
			tcStatus.Text = string.Format("<img src=\"images/{0}.png\">&nbsp;<b>{1}</b>",status.ToLower(),status);
			tcMasterImg.Text = string.Format("<img src=\"{0}\" alt=\"{0}\">",master);
			tcRenderedImg.Text = string.Format("<img src=\"{0}\" alt=\"{0}\">",rendered);
			
			reader.Close();
			reader = null;
			
			dbcmd.Dispose();
			dbcmd = null;
			dbcon.Close();
			dbcon = null;
			
		}
	}
}
