using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Browser;
using System.Text;

namespace dependency_properties
{
	public class HtmlOutput : IOutput
	{
		StringBuilder builder;

		public void Report (TestResult result)
		{
			string html;

			html = @"
<tr style='color: {4}'>
<td>{0}</td>
<td>{1}</td>
<td>{2}</td>
<td>{5}</td>
<td>{3}</td>
</tr>
";
			html = string.Format (html, HttpUtility.HtmlEncode (result.name), result.success ? "SUCCESS" : "FAIL", 
				result.ex != null ? Encode (result.ex.ToString ()) : null, Encode (result.output),
				result.success ? "Green" : "Red", HttpUtility.HtmlEncode (result.reason));

			builder.Append (html);
		}

		public string Encode (string text)
		{
			return HttpUtility.HtmlEncode (text).Replace ("\n", "<br/>");
		}

		public void Write (string text)
		{
			HtmlElement TestResult;
			string innerHTML;

			if (false && HtmlPage.BrowserInformation.Platform.Contains ("Win")) {
				TestResult = HtmlPage.Document.GetElementById ("TestResult");
				innerHTML = (string) TestResult.GetProperty ("innerHTML");
				innerHTML += text;
				HtmlPage.Document.GetElementById ("TestResult").SetProperty ("innerHTML", innerHTML);
			} else {
				HtmlPage.Window.Eval ("AppendTestResult ('" + text.Replace ("'", "\\'").Replace ("\n", "\\n").Replace ("\r", "\\r") + "');");
			}
		}


		public void StartReport ()
		{
			string html = @"
<table border='1'>
<tr>
<th>Name</th>
<th>Result</th>
<th>Exception</th>
<th>Reason</th>
<th>Output</th>
</tr>
";

			builder = new StringBuilder ();
			builder.Append (html);
		}

		public void EndReport ()
		{
			string html = @"
</table>
";
			builder.Append (html); 
			Write (builder.ToString ());
		}

	}
}
