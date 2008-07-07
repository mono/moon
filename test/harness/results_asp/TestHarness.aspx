<%@ Page Language="C#" Inherits="moonlight.TestHarness" %>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
	<title>Test Harness</title>
	
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
</td></tr></table>
</div>
<div>
	<form id="form1" runat="server">
	
		<div>
			<table><tr><td></td><td>
			<h2><center>Builds</center></h2>
			</td></tr><tr>
			<td><h2>Test ID</h2></td><td>
	        <asp:Table ID="tblData" runat="server" class="maintable">
	        </asp:Table>
	        </td></td>
	        </table>
	    </div>
	    <div>
	    	<br><br><br>
		    <asp:Label id="debug1" runat="server">
		    </asp:Label>
		</div>
	    <div>
	    	<br><br><br>
		    <asp:Label id="debug2" runat="server">
		    </asp:Label>
		</div>    
	    <div>
	    	<br><br><br>
		    <asp:Label id="debug3" runat="server">
		    </asp:Label>
		</div>        
	</form>  
	

</div> <!-- col2 -->
</div> <!-- page -->
</body>
</html>