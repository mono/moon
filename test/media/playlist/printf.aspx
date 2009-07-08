<%@ page language="C#" %>
<%@ import namespace="System.IO" %>
<%@ import namespace="System" %>
<html>
	<head>
		<script runat="server">
	protected override void OnLoad (EventArgs args)
	{
		base.OnLoad (args);

		string action = Request ["action"];
		string file = Request ["file"];
		string contents = (new StreamReader (Request.InputStream)).ReadToEnd ();

		Console.WriteLine ("");
		Console.WriteLine ("Date:     " + DateTime.Now.ToString ());
		Console.WriteLine ("Url:      " + Request.Url);
		Console.WriteLine ("Action:   " + action);
		Console.WriteLine ("File:     " + file);
		Console.WriteLine ("Contents: " + contents);

		if (action == null || action == string.Empty) {
			Console.WriteLine ("Empty action");
			return;
		} else if (file == null || file == string.Empty) {
			Console.WriteLine ("Empty file");
			return;
		}

		file = Path.Combine ("media/playlist/", file);

		switch (action) {
		case "create":
			File.WriteAllText (file, "");
			Console.WriteLine ("Created: " + file);
			break;
		case "append":
			File.AppendAllText (file, contents);
			Console.WriteLine ("Wrote " + contents.Length + " characters to " + file);
			break;
		case "printf":
			Console.WriteLine (contents);
			break;
		default:
			Console.WriteLine ("Unknown action: " + action);
			break;
		}
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

