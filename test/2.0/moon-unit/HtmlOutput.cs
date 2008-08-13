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
			string call = string.Format ("AddTest ('{0}', '{1}', '{2}', '{3}');",
						     result.name,
						     result.success ? "success" : "failure",
						     result.reason == null ? "" : Encode (result.reason),
						     result.output == null ? "" : Encode (result.output));
			HtmlPage.Window.Eval (call);
		}

		public string Encode (string text)
		{
			return HttpUtility.HtmlEncode (text).Replace ("\n", "<br/>").Replace ("\"", "&#34;").Replace("\'", "&#39;");
		}

		public void Write (string text)
		{
#if false
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
#endif
		}


		public void StartReport ()
		{
			builder = new StringBuilder ();
		}

		public void EndReport ()
		{
			HtmlPage.Window.Eval ("EndReport();");

#if false
			string start_html = @"
<table border='1'>
<tr>
<th>Name</th>
<th>Result</th>
<th>Exception</th>
<th>Reason</th>
<th>Output</th>
</tr>
";
	
			string end_html = @"
</table>
";

			StringBuilder tmp = builder;
			builder = new StringBuilder ();
		
			builder.Append (start_html);
			Report (final_result);
			builder.Append (tmp.ToString ());
			builder.Append (end_html);

			Write (builder.ToString ());
#endif
		}

	}
}
