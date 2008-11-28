using System;
using System.Diagnostics;
using System.Globalization;
using System.Net;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Browser;

using Microsoft.Silverlight.Testing;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata.VisualStudio;
using Microsoft.Silverlight.Testing.UnitTesting.Harness;

namespace Mono.Moonlight.UnitTesting
{
	public class MoonLogProvider : LogProvider
	{
		private string filename;
		private int sequence;
		private string baseuri;

		public MoonLogProvider ()
		{
			DateTime now = DateTime.Now;
			Uri docuri = HtmlPage.Document.DocumentUri;

			if (docuri.Scheme != "http" && docuri.Scheme != "https") {
				Console.WriteLine ("MoonLogProvider: Disabled since we're not loading the test from http(s).");
				return;
			}

			baseuri = string.Concat (docuri.Scheme, "://", docuri.Host, ":", docuri.Port.ToString ());

			// Create a unique filename
			filename = string.Format ("moon-unit-log-{0}-{1}.xml",
				now.ToUniversalTime ().ToString ("yyyy-MM-dd-HH-mm-ss", CultureInfo.InvariantCulture.DateTimeFormat),
				DateTime.Now.Ticks.ToString (CultureInfo.InvariantCulture.NumberFormat));
		}

		private string MessageToXml (LogMessage logMessage)
		{
			TestClass c;
			TestMethod m;
			string name;
			TestOutcome outcome;
			TestGranularity granularity;
			TestStage stage;

			Debug.WriteLine (logMessage.ToString ());

			try {
				switch (logMessage.MessageType) {
				case LogMessageType.TestResult:
					c = (TestClass) logMessage.Decorators [UnitTestLogDecorator.TestClassMetadata];
					m = (TestMethod) logMessage.Decorators [UnitTestLogDecorator.TestMethodMetadata];
					name = (string) logMessage.Decorators [LogDecorator.NameProperty];
					outcome = (TestOutcome) logMessage.Decorators [LogDecorator.TestOutcome];

					return string.Format ("<Test Name='{0}.{1}' Result='{2}'/>", c.Name, name, outcome);
				case LogMessageType.TestExecution:
					granularity = (TestGranularity) logMessage.Decorators [LogDecorator.TestGranularity];
					stage = (TestStage) logMessage.Decorators [LogDecorator.TestStage];

					switch (granularity) {
					case TestGranularity.TestGroup:
						switch (stage) {
						case TestStage.Starting:
							return "<MoonLog>";
						case TestStage.Finishing:
						case TestStage.Canceling:
							return "</MoonLog>";
						case TestStage.Running:
							return null;
						}
						break;
					case TestGranularity.TestScenario:
					case TestGranularity.Test:
						return null;
					}
					return string.Format ("<!-- Unknown test execution granularity: {0} stage: {1} -->", granularity, stage);
				default:
					return string.Format ("<!-- Unknown log type '{1}': {0} -->", logMessage.ToString (), logMessage.MessageType);
				}
			} catch (Exception ex) {
				return string.Format ("<!-- Exception while converting message to xml ({0}) -->", ex.Message);
			}
		}

		public override void Process (LogMessage logMessage)
		{
			string msg;
			
			try {

				if (filename == null)
					return;
				
				Console.WriteLine ("Process {0}", logMessage);

				msg = MessageToXml (logMessage);

				if (string.IsNullOrEmpty (msg))
					return;

				LogRequest request = new LogRequest ();
				request.provider = this;
				request.sequence = sequence++;
				request.uri = new Uri (baseuri + "/MoonLogProvider.aspx?filename=" + filename + "&sequence=" + request.sequence.ToString (), UriKind.Absolute);
								request.httprequest = (HttpWebRequest) WebRequest.Create (request.uri);
				request.httprequest.Method = "POST";
				//request.httprequest.ContentType = "application/x-www-form-urlencoded";
				request.message = msg + Environment.NewLine;
				request.httprequest.BeginGetRequestStream (RequestCallback, request);
			} catch (Exception ex) {
				Console.WriteLine ("Exception while trying to send message: {0}", ex.ToString ());
			}
		}

		private static void RequestCallback (IAsyncResult ar)
		{
			try {
				// Console.WriteLine ("RequestCallback");
				LogRequest request = (LogRequest) ar.AsyncState;
				Stream stream = request.httprequest.EndGetRequestStream (ar);
				StreamWriter writer = new StreamWriter (stream);
				writer.Write (request.message);
				writer.Close ();
				stream.Close ();
				request.httprequest.BeginGetResponse (ResponseCallback, request);
			} catch (Exception ex) {
				Console.WriteLine ("Exception in ResponseCallback: {0}", ex.ToString ());
			}
		}

		private static void ResponseCallback (IAsyncResult ar)
		{
			try {
				LogRequest request = (LogRequest) ar.AsyncState;
				HttpWebResponse response = (HttpWebResponse) request.httprequest.EndGetResponse (ar);
				// Do nothing, we don't care about the response.
				// Console.WriteLine ("ResponseCallback");
			} catch (Exception ex) {
				Console.WriteLine ("Exception in ResponseCallback sequence: {1}: {0}", ex.Message, ((LogRequest) ar.AsyncState).sequence);
			}
		}

		class LogRequest
		{
			public HttpWebRequest httprequest;
			public MoonLogProvider provider;
			public Uri uri;
			public string message;
			public int sequence;
		}
	}
}
