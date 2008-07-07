<%@ Page Language="C#" Inherits="moonlight.ShowRun" %>
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

<h1>&nbsp;&nbsp;&nbsp;Test Run</h1>
<h2>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<asp:Label id="lblStatus" runat="server">
				</asp:Label></h2>
</td></tr></table>
</div>
<div>
		<asp:Table id="tblsummary" runat="server">
			<asp:TableRow>
				<asp:TableCell Text="Passed Tests:" ></asp:TableCell>
				<asp:TableCell id="tcPassed" runat="server">
				</asp:TableCell>
			</asp:TableRow>
			<asp:TableRow>
				<asp:TableCell Text="Failed Tests:"></asp:TableCell>
				<asp:TableCell id="tcFailed" runat="server">
				</asp:TableCell>
			</asp:TableRow>
			<asp:TableRow>
				<asp:TableCell Text="Known Failures:"></asp:TableCell>
				<asp:TableCell id="tcKnownFailures" runat="server">
				</asp:TableCell>
			</asp:TableRow>
			<asp:TableRow>
				<asp:TableCell Text="Ignored Tests:"></asp:TableCell>
				<asp:TableCell id="tcIgnored" runat="server">
				</asp:TableCell>
			</asp:TableRow>
		</asp:Table>

</div> <!-- col2 -->
</div> <!-- page -->
</body>
</html>
