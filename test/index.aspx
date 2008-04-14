<%@ page language="C#" %>
<%@ import namespace="System.IO" %>
<html>
	<head>
		<script runat="server">
	protected override void OnLoad (EventArgs args)
	{
		base.OnLoad (args);
		DirectoryInfo dir = new DirectoryInfo (Path.GetDirectoryName (Request.PhysicalPath));
		StringBuilder sb = new StringBuilder ();
		sb.Append ("<ul>\n");
		sb.Append (ReadDirectory (Path.Combine (Path.GetDirectoryName (Request.PhysicalPath), String.Empty), String.Empty));
		sb.Append ("</ul>");
		fileList.InnerHtml = sb.ToString ();
	}
	
	public string ReadDirectory (string path, string basePath)
	{
		StringBuilder sb = new StringBuilder ();
		foreach (string sdir in Directory.GetDirectories (path)) {
			string s = ReadDirectory (sdir, basePath + Path.GetFileName (sdir) + "/");
			if (s != "")
				sb.AppendFormat ("<li><b>{0}</b><ul>{1}</ul></li>", Path.GetFileName (sdir), s);
		}
		foreach (string file in Directory.GetFiles (path)) {
			string fileName = basePath + Path.GetFileName (file);
			string extension = Path.GetExtension (file);
			if (extension == ".xaml") {
				sb.AppendFormat ("<li><a class=\"{2}\" href=\"xamlize.aspx?xaml={1}\">{0}</a></li>\n",
						Path.GetFileName (fileName), fileName, extension.Substring (1));
			}
			if (extension == ".htm" || extension == ".html") {
			        sb.AppendFormat ("<li><a class=\"{2}\" href=\"{1}\">{0}</a></li>\n",
						Path.GetFileName (fileName), fileName, extension.Substring (1));
  	   
			}
		}
		return sb.ToString ();
	}
	
		</script>
	</head>

	<body>
		<span id="fileList" runat="server" />
		<hr />
		<div>
			<div style="text-align: left; font-size: small;">Generated: <%= DateTime.Now %></div>
		</div>
	</body>
</html>

