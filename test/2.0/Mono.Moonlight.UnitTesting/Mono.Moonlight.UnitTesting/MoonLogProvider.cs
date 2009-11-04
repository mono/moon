//
// MoonLogProvider.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.Diagnostics;
using System.Globalization;
using System.Net;
using System.IO;
using System.Reflection;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Browser;
using System.Xml;

using Microsoft.Silverlight.Testing;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata.VisualStudio;
using Microsoft.Silverlight.Testing.UnitTesting.Harness;

namespace Mono.Moonlight.UnitTesting
{
	public class MoonLogProvider : LogProvider
	{
		private XmlWriter writer;
		private StringBuilder text = new StringBuilder ();

		private string filename;
		private string baseuri;
		private Action on_completed;
		private int[] totals;
		private readonly int KnownIssueIndex;
		private bool known_issue;

		public MoonLogProvider ()
		{
			XmlWriterSettings settings;

			DateTime now = DateTime.Now;
			Uri docuri = HtmlPage.Document.DocumentUri;

			if (docuri.Scheme != "http" && docuri.Scheme != "https") {
				Console.WriteLine ("MoonLogProvider: Disabled since we're not loading the test from http(s).");
				return;
			}

			baseuri = string.Concat (docuri.Scheme, "://", docuri.Host, ":", docuri.Port.ToString (), docuri.LocalPath.Substring (0, docuri.LocalPath.Length - System.IO.Path.GetFileName (docuri.LocalPath).Length));

			// Create a unique filename
			filename = string.Format ("moon-unit-log-{0}-{1}.xml",
				now.ToUniversalTime ().ToString ("yyyy-MM-dd-HH-mm-ss", CultureInfo.InvariantCulture.DateTimeFormat),
				DateTime.Now.Ticks.ToString (CultureInfo.InvariantCulture.NumberFormat));

			settings = new XmlWriterSettings ();
			settings.Indent = true;
			settings.Encoding = Encoding.UTF8;
			writer = XmlWriter.Create (text, settings);
			writer.WriteStartDocument ();
			writer.WriteProcessingInstruction ("xml-stylesheet", "type='text/xsl' href='moon-unit-log.xsl'");

			totals = new int [typeof (TestOutcome).GetFields (BindingFlags.Static | BindingFlags.Public).Length + 1];
			KnownIssueIndex = totals.Length - 1;
		}

		/// <summary>
		/// Returns true to indicate that the process can be shut down.
		/// </summary>
		/// <param name="e"></param>
		/// <returns></returns>
		public bool ProcessCompleted (TestHarnessCompletedEventArgs e, Action onCompleted)
		{
			string msg;

			try {
				if (filename == null)
					return true;

				if (writer != null)
					writer.Flush ();

				msg = text.ToString ();

				if (string.IsNullOrEmpty (msg))
					return true;

				LogRequest request = new LogRequest ();
				request.provider = this;
				request.uri = new Uri (baseuri + "MoonLogProvider.aspx?filename=" + filename, UriKind.Absolute);
				request.httprequest = (HttpWebRequest) WebRequest.Create (request.uri);
				request.httprequest.Method = "POST";
				request.httprequest.ContentType = "text/xml";// "application/x-www-form-urlencoded";
				request.message = msg + Environment.NewLine;
				
				// This cwl is required, the test harness looks for it in stdout to find
				// the filename we write to.
				Console.WriteLine ("MoonLogProvider: sending request to: {0}", request.uri);
				
				request.httprequest.BeginGetRequestStream (RequestCallback, request);

				on_completed = onCompleted;

				return false;
			} catch (Exception ex) {
				Console.WriteLine ("Exception while trying to send result: {0}", ex.ToString ());
				return true;
			}
		}

		public override void Process (LogMessage logMessage)
		{
			try {
				ProcessMessage (logMessage);
			} catch (Exception ex) {
				Console.WriteLine (ex.ToString ());
			}
		}

		private void ProcessMessage (LogMessage logMessage) {
			TestMethod m;
			TestOutcome outcome;
			TestGranularity granularity;
			TestStage stage;
			ScenarioResult result;

			if (filename == null)
				return;

			Debug.WriteLine (logMessage);

			try {
				switch (logMessage.MessageType) {
				case LogMessageType.Error:
					// Ignore this
					break;
				case LogMessageType.KnownIssue:
					known_issue = true;
					break;
				case LogMessageType.TestResult:
					result = (ScenarioResult) logMessage.Decorators [UnitTestLogDecorator.ScenarioResult];
					outcome = result.Result;

					if (known_issue) {
						writer.WriteAttributeString ("Result", "KnownIssue");
						totals [KnownIssueIndex]++;
					} else {
						writer.WriteAttributeString ("Result", outcome.ToString ());
						totals [(int) outcome]++;
					}
					known_issue = false;
					writer.WriteAttributeString ("StartTime", result.Started.ToUniversalTime ().ToString ("yyyy/MM/dd HH:mm:ss.fffffff"));
					writer.WriteAttributeString ("EndTime", result.Finished.ToUniversalTime ().ToString ("yyyy/MM/dd HH:mm:ss.fffffff"));
					if (result.Exception != null) {
						writer.WriteAttributeString ("Message", result.Exception.Message);
						writer.WriteStartElement ("Exception");
						writer.WriteAttributeString ("Type", result.Exception.GetType ().FullName);
						writer.WriteAttributeString ("Message", result.Exception.Message);
						writer.WriteElementString ("StackTrace", result.Exception.StackTrace);
						writer.WriteEndElement ();
					}
					break;
				case LogMessageType.TestExecution:
					known_issue = false;
					granularity = (TestGranularity) logMessage.Decorators [LogDecorator.TestGranularity];

					switch (granularity) {
					case TestGranularity.TestGroup:
						stage = (TestStage) logMessage.Decorators [LogDecorator.TestStage];
						switch (stage) {
						case TestStage.Starting:
							writer.WriteStartElement ("MoonLog");
							break;
						case TestStage.Finishing:
						case TestStage.Canceling:

							writer.WriteStartElement ("Totals");
							for (int i = 0; i < totals.Length; i++) {
								if (i == KnownIssueIndex) {
									writer.WriteAttributeString ("KnownIssue", totals [i].ToString ());
								} else {
									writer.WriteAttributeString (((TestOutcome) i).ToString (), totals [i].ToString ());
								}									
							}
							writer.WriteEndElement ();

							writer.WriteEndElement ();
							break;
						case TestStage.Running:
							break;
						}
						break;
					case TestGranularity.TestScenario:
						if (logMessage.Decorators.HasDecorator (UnitTestLogDecorator.IgnoreMessage) && (bool) logMessage.Decorators [UnitTestLogDecorator.IgnoreMessage]) {
							m = (TestMethod) logMessage.Decorators [UnitTestLogDecorator.TestMethodMetadata];
							writer.WriteStartElement ("Test");
							writer.WriteAttributeString ("Class", m.Method.DeclaringType.FullName);
							writer.WriteAttributeString ("Name", m.Method.Name);
							writer.WriteAttributeString ("FullName", m.Method.DeclaringType.FullName + "." + m.Method.Name);
							writer.WriteAttributeString ("Result", "NotExecuted");
							writer.WriteAttributeString ("Ignored", "True");
							writer.WriteEndElement ();
							totals [(int) TestOutcome.NotExecuted]++;
						} else {
							stage = (TestStage) logMessage.Decorators [LogDecorator.TestStage];
							switch (stage) {
							case TestStage.Starting:
								m = (TestMethod) logMessage.Decorators [UnitTestLogDecorator.TestMethodMetadata];
								writer.WriteStartElement ("Test");
								writer.WriteAttributeString ("Class", m.Method.DeclaringType.FullName);
								writer.WriteAttributeString ("Name", m.Method.Name);
								writer.WriteAttributeString ("FullName", m.Method.DeclaringType.FullName + "." + m.Method.Name);
								break;
							case TestStage.Finishing:
							case TestStage.Canceling:
								writer.WriteEndElement ();
								break;
							}
						}
						break;
					case TestGranularity.Test:
					case TestGranularity.Harness:
						break;
					default:
						writer.WriteComment (string.Format ("Unknown test execution granularity: {0}", granularity));
						break;
					}
					break;
				case LogMessageType.TestInfrastructure:
					break;
				default:
					Console.WriteLine (string.Format ("Unknown log type '{1}': {0}", logMessage.ToString (), logMessage.MessageType));
					break;
				}
			} catch (Exception ex) {
				writer.WriteComment (string.Format ("Exception while converting message to xml ({0})", ex.ToString ()));
			}
		}

		private void RequestCallback (IAsyncResult ar)
		{
			try {
				Console.WriteLine ("MoonLogProvider: RequestCallback on_completed: {0}", on_completed == null ? "null" : "not null");
				LogRequest request = (LogRequest) ar.AsyncState;
				Stream stream = request.httprequest.EndGetRequestStream (ar);
				StreamWriter writer = new StreamWriter (stream, this.writer.Settings.Encoding);
				writer.Write (request.message);
				writer.Close ();
				stream.Close ();
				request.httprequest.BeginGetResponse (ResponseCallback, request);
			} catch (Exception ex) {
				Console.WriteLine ("Exception in ResponseCallback: {0}", ex.ToString ());
			}
		}

		private void ResponseCallback (IAsyncResult ar)
		{
			try {
				Console.WriteLine ("MoonLogProvider: ResponseCallback on_completed: {0}", on_completed == null ? "null" : "not null");
				LogRequest request = (LogRequest) ar.AsyncState;
				request.httprequest.EndGetResponse (ar);
				// We don't care about the response
			} catch (Exception ex) {
				Console.WriteLine ("Exception in ResponseCallback: {0}", ex.Message);
			} finally {
				if (on_completed != null)
					on_completed ();
			}
		}

		class LogRequest
		{
			public HttpWebRequest httprequest;
			public MoonLogProvider provider;
			public Uri uri;
			public string message;
		}
	}
}
