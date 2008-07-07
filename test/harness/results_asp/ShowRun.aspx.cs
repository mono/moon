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
	
	
	public partial class ShowRun : System.Web.UI.Page
	{
		string runtime = string.Empty;
		
		private void Page_Load(object sender, EventArgs e)
		{
			runtime = this.Request.QueryString["runtime"];
			
			if (runtime == null) runtime = string.Empty;		
			
			if (runtime.Trim() == string.Empty)
			{
				lblStatus.Text = "Please set build and testid";
				return;
			}
			
			lblStatus.Text = string.Format("Results for test run \'{0}\'", runtime);
			
			string connectionString = "URI=file:moonTestSuite.db";
			IDbConnection dbcon = (IDbConnection) new SqliteConnection(connectionString);
					
					
			dbcon.Open();
			IDbCommand dbcmd = dbcon.CreateCommand();
			// select runs.status, testcases.masterfile, runs.renderedfile from runs,testcases where testcases.id=1 and runs.revision=1001 and runs.testcaseid=testcases.id;
			dbcmd.CommandText = string.Format("select results.status, testcases.masterfile, results.renderedfile, results.info from results,testcases where results.testcaseid=testcases.id and testcases.id={0} and results.runtime='{0}';",runtime);
			
			//Console.WriteLine(dbcmd.CommandText);
			
			IDataReader reader = dbcmd.ExecuteReader();
			reader.Read();
			
			//tcStatus.Text = string.Format("<img src=\"images/{0}.png\">&nbsp;<b>{1}</b>",status.ToLower(),status);
			//tcMasterImg.Text = string.Format("<img src=\"{0}\" alt=\"{0}\">",master);
			//tcRenderedImg.VerticalAlign = WebControls.VerticalAlign.Top;
			//tcRenderedImg.Text = string.Format("<img src=\"{0}\" alt=\"{0}\">",rendered);
			
			
			
			reader.Close();
			reader = null;
			
			dbcmd.Dispose();
			dbcmd = null;
			dbcon.Close();
			dbcon = null;
			
		}
	}
}
