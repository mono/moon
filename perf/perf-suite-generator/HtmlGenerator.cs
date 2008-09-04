/*
 * Copyright (c) 2008 Novell, Inc. (http://www.novell.com)
 *
 * Contact:
 *  Moonlight List (moonlight-list@lists.ximian.com)
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

using System;
using System.Collections.Generic;
using PerfSuiteLib;
using Cairo;

namespace PerfSuiteGenerator {

	public static class HtmlGenerator {

		static readonly string ChangelogTemplate = "<div class=\"changelog\" id=\"changelog-@@DETAIL_ID@@\">" + 
							   "<pre>" + 
							   "@@DETAIL_CHANGELOG@@" +
							   "</pre>" +
							   "</div>";

		static readonly string DetailRowTemplate = "<div class=\"detail @@DETAIL_CLASS@@\" id=\"detail-@@DETAIL_ID@@\">" + 
							   "<div class=\"left\">@@PASS_SHORT_NAME@@ @@PASS_AUTHOR@@ @@DATE@@</div>" + 
							   "<div class=\"right\">@@RESULT@@</div>" +
							   "</div>";

		static readonly string ItemRowTemplate = "<div class=\"item\">" + 
							   "<div class=\"left\"><h3>#@@ITEM_UNIQUE_ID@@: @@ITEM_NAME@@</h3><p>File: @@ITEM_INPUT_FILE@@</p><p id=\"more-info-@@ITEM_UNIQUE_ID@@\" class=\"click\"><a href=\"empty\">More info</a></p></div>" + 
							   "<div class=\"right\"><img src=\"@@IMG_SRC@@\" /></div>" +
							   "<div class=\"details\" id=\"details-@@ITEM_UNIQUE_ID@@\">@@DETAILS@@</div></div>";

		static readonly string HtmlTemplate = "<html><head>" + 
						      "<title>Moonlight performance suite</title>" +
						      "<meta content='text/html; charset utf-8' http-equiv='Content-type' />" +
						      "<link type=\"text/css\" href=\"report.css\" charset=\"utf-8\" media=\"all\" rel=\"Stylesheet\" />" +
						      "<script type=\"text/javascript\" src=\"jquery.js\"></script>" + 
						      "<script type=\"text/javascript\" src=\"jquery.tooltip.js\"></script>" + 
						      "<script type=\"text/javascript\" src=\"helpers.js\"></script>" +
						      "</head><body>" + 
						      "<div class=\"container\">\n@@HEADER@@\n@@CONTENT@@</div></body></html>";

		static readonly string HeaderTemplate = "<div class=\"header\">" + 
							"<img src=\"logo.png\" id=\"logo\" />" +
							"<h1>Moonlight performance suite results</h1>" + 
							"<h2>Generated on: @@GENERATED_DATE@@</h2>" +
							"<h2>Lass pass on: @@LAST_PASS_DATE@@</h2>" +
							"</div>";

		public static string GenerateDetailRows (List <ResultDbEntry> resultList)
		{
			string output = String.Empty;
			string changelogsOutput = String.Empty;

			// FIXME Exception if more than 50 results...
			// FIXME Reverse the copy
			resultList.Reverse ();
			bool hasPrevResult = false;
			long prevResult = 0;
			int id = 1;

			foreach (ResultDbEntry entry in resultList) {

				string cls = "normal";
				
				if (! entry.Failure) {

					if (hasPrevResult && UtilFu.GetValueDifference (prevResult, entry.Time) > 0.1)
						cls = "red";
					else if (hasPrevResult && UtilFu.GetValueDifference (prevResult, entry.Time) < -0.1)
						cls = "green";
				
					hasPrevResult = true;
					prevResult = entry.Time;
				} else 
					cls = "black";

				string html = DetailRowTemplate;
				html = html.Replace ("@@DETAIL_CLASS@@", cls);
				html = html.Replace ("@@DETAIL_ID@@", id.ToString ());
				html = html.Replace ("@@PASS_SHORT_NAME@@", entry.Pass.ShortName);
				
				string author = String.Empty;
				if (entry.Pass.Author != String.Empty)
					author = String.Format ("[{0}]", entry.Pass.Author);

				html = html.Replace ("@@PASS_AUTHOR@@", author);
				
				html = html.Replace ("@@DATE@@", entry.Pass.Date.ToString ());

				if (! entry.Failure) 
					html = html.Replace ("@@RESULT@@", String.Format ("{0:0.000}", (entry.Time / (float) 1000000)));
				else
					html = html.Replace ("@@RESULT@@", "FAILURE");

				// Add also the detail changelog
				string changelog = ChangelogTemplate;
				changelog = changelog.Replace ("@@DETAIL_ID@@", id.ToString ());

				if (entry.Pass.ChangeLog != String.Empty) 
					changelog = changelog.Replace ("@@DETAIL_CHANGELOG@@", entry.Pass.ChangeLog);
				else
					changelog = changelog.Replace ("@@DETAIL_CHANGELOG@@", "No ChangeLog data");

				changelogsOutput += changelog;
				output = html + output;
				id += 1;
			}
				
			return output + changelogsOutput;
		}

		public static string GenerateHeaderTemplate ()
		{
			string html = HeaderTemplate;

			PassDbEntry pass = Database.GetLastPass ();

			html = html.Replace ("@@GENERATED_DATE@@", DateTime.Now.ToString ());
			
			if (pass != null)
				html = html.Replace ("@@LAST_PASS_DATE@@", String.Format ("{0} ({1})", pass.Date.ToString (), pass.ShortName));
			else
				html = html.Replace ("@@LAST_PASS_DATE@@", "NO DATA!");

			
			return html;
		}

		public static string GenerateItemRows (List <ItemDbEntry> itemList)
		{
			string output = String.Empty;

			foreach (ItemDbEntry entry in itemList) {

				string graphBaseName = String.Format ("graph-{0}.png", entry.UniqueId);
				string graphFilename = "perf-report/" + graphBaseName;

				List <ResultDbEntry> list = Database.GetResultEntriesForItemEntry (entry, 50);
				GraphGenerator.GenerateGraph (list, graphFilename);

				string html = ItemRowTemplate;
				html = html.Replace ("@@ITEM_UNIQUE_ID@@", entry.UniqueId);
				html = html.Replace ("@@ITEM_NAME@@", entry.Name);
				html = html.Replace ("@@ITEM_INPUT_FILE@@", entry.InputFile);
				html = html.Replace ("@@IMG_SRC@@", graphBaseName);
				html = html.Replace ("@@DETAILS@@", GenerateDetailRows (list));

				output += html;
			}

			return output;
		}

		public static string GenerateHTML ()
		{
			string html = HtmlTemplate;

			List <ItemDbEntry> itemList = Database.GetAllItemEntries ();
			string content = GenerateItemRows (itemList);
			string header = GenerateHeaderTemplate ();

			html = html.Replace ("@@CONTENT@@", content);
			html = html.Replace ("@@HEADER@@", header);

			return html;
		}

	}

}


