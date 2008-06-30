<%@ Page Language="C#" Inherits="moonlight.ShowTest" %>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
	<title>ShowTest</title>
	
	<link rel="stylesheet" type="text/css" href="moonlight.css"/>
</head>
<body>


<div id="page">
<div>
<table>
<tr><td>
<img src="images/moonlight_logo.png" alt="Moonlight Logo"/>
</td><td>

<h1>&nbsp;&nbsp;&nbsp;Automated Test Results</h1>
<h2>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<asp:Label id="lblStatus" runat="server">
				</asp:Label></h2>
</td></tr></table>
</div>
<div>
		<asp:Table runat="server">
			<asp:TableRow>
				<asp:TableCell>Test ID:&nbsp;&nbsp;&nbsp;
				</asp:TableCell>
				<asp:TableCell id="tcTestid" runat="server">
				</asp:TableCell>
			</asp:TableRow>
			<asp:TableRow>
				<asp:TableCell>Build:
				</asp:TableCell>
				<asp:TableCell id="tcBuild" runat="server">
				</asp:TableCell>
			</asp:TableRow>
			<asp:TableRow>
				<asp:TableCell>Status:
				</asp:TableCell>
				<asp:TableCell id="tcStatus" runat="server">
				</asp:TableCell>
			</asp:TableRow>
		</asp:Table>
		<asp:Table runat="server">
			<asp:TableRow>
				<asp:TableCell id="tcMasterImg" runat="server">
				</asp:TableCell>
				<asp:TableCell id="tcRenderedImg" runat="server">
				</asp:TableCell>
			</asp:TableRow>
		</asp:Table>

</div> <!-- col2 -->
</div> <!-- page -->
</body>
</html>