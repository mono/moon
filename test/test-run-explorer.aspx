<%@ Page Language="C#" %>
<%@ import namespace="System.IO" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">

<script runat="server">

      private void Page_Load (object sender, EventArgs e)
      {
		if (!IsPostBack) {
			foreach (string dir in Directory.GetDirectories ("test-run-data/")) {
			        string run_data = Path.Combine (dir, "run.xml");
				if (!System.IO.File.Exists (run_data))
				    continue;
				test_runs.Add (Path.GetDirectoryName (run_data));
			}
			test_run_data.DataFile = Path.Combine (test_runs [test_runs.Count - 1].ToString (), "run.xml");
			test_runs_list.DataSource = test_runs;
			test_runs_list.DataBind ();
	        }
      }

      private void test_runs_SelectedIndexChanged (object sender, EventArgs e)
      {
	  test_run_data.DataFile = Path.Combine (test_runs_list.SelectedValue, "run.xml");
	  GridView1.DataBind ();
      }
      
</script>

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
    <title>Moonlight Test Runs</title>
</head>
<body>
    <form id="form1" runat="server">

        <object id="test_runs" runat="server" class="System.Collections.ArrayList" />
        <asp:DropDownList id="test_runs_list" OnSelectedIndexChanged="test_runs_SelectedIndexChanged" runat="server" AutoPostBack="true" />
    
        <asp:GridView AutoGenerateColumns="False"
		        ID="GridView1"
			DataSourceID="test_run_data"
			AllowPaging="True" PageSize="5"
			AllowSorting="True"
			EnableSortingAndPagingCallbacks="false"
			DataKeyNames="InputFile"
                        runat="server">

            <Columns>
	        
		<asp:BoundField DataField="InputFile" HeaderText="InputFile" SortExpression="InputFile" />
		<asp:BoundField DataField="TestResult" HeaderText="Result" SortExpression="Result" />

                <asp:ImageField DataImageUrlField="ResultFile" HeaderText="Result File" SortExpression="ResultFile" />
		<asp:ImageField DataImageUrlField="MasterFile" HeaderText="Master File" SortExpression="MasterFile" />
		<asp:ImageField DataImageUrlField="EdgeDiffFile" HeaderText="Edge Differences File" SortExpression="EdgeDiffFile" />
		
            </Columns>
			<AlternatingRowStyle BackColor="#FFFFc0"/>
			<SelectedRowStyle BackColor="red"/>
			<PagerSettings Mode="Numeric"/>
        </asp:GridView>

	<asp:XmlDataSource ID="test_run_data" EnableCaching="false" runat="server" />
    </form>
</body>
</html>

