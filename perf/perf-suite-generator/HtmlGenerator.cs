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

		static readonly string DetailRowTemplate = "<div class=\"@@DETAIL_CLASS@@\">" + 
							   "<div class=\"left\">@@PASS_DESCRIPTION@@ @@DATE@@</div>" + 
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
						      "<script type=\"text/javascript\" src=\"helpers.js\"></script>" +
						      "</head><body>" + 
						      "<div class=\"container\">\n@@HEADER@@\n@@CONTENT@@</div></body></html>";

		static readonly string HeaderTemplate = "<div class=\"header\">" + 
							"<img src=\"logo.png\" id=\"logo\" />" +
							"<h1>Moonlight performance suite results</h1>" + 
							"<h2>Generated on: @@GENERATED_DATE@@</h2>" +
							"<h2>Lass pass on: @@LAST_PASS_DATE@@</h2>" +
							"</div>";

		public static string GenerateDetailRows (List <ResultWithDateDbEntry> resultList)
		{
			string output = String.Empty;

			// FIXME Exception if more than 50 results...

			resultList.Reverse ();
			bool hasPrevResult = false;
			long prevResult = 0;

			foreach (ResultWithDateDbEntry entry in resultList) {

				string cls;
				if (hasPrevResult && PercentDifference (prevResult, entry.Time) > 0.1)
					cls = "red-detail";
				else if (hasPrevResult && PercentDifference (prevResult, entry.Time) < -0.1)
					cls = "green-detail";
				else
					cls = "detail";

				string html = DetailRowTemplate;
				html = html.Replace ("@@DETAIL_CLASS@@", cls);
				html = html.Replace ("@@PASS_DESCRIPTION@@", entry.Description);
				html = html.Replace ("@@DATE@@", entry.Date.ToString ());
				html = html.Replace ("@@RESULT@@", (entry.Time / (float) 1000000).ToString ());

				output = html + output;
				
				hasPrevResult = true;
				prevResult = entry.Time;
			}

			return output;
		}

		public static string GenerateHeaderTemplate ()
		{
			string html = HeaderTemplate;

			PassDbEntry pass = Database.GetLastPass ();

			html = html.Replace ("@@GENERATED_DATE@@", DateTime.Now.ToString ());
			html = html.Replace ("@@LAST_PASS_DATE@@", String.Format ("{0} ({1})", pass.Date.ToString (), pass.Description));
			return html;
		}

		public static string GenerateItemRows (List <ItemDbEntry> itemList)
		{
			string output = String.Empty;

			foreach (ItemDbEntry entry in itemList) {

				string graphFilename = String.Format ("graph-{0}.png", entry.UniqueId);
				List <ResultWithDateDbEntry> list = Database.GetResultEntriesForItemEntry (entry, 50);
				GraphGenerator.GenerateGraph (list, graphFilename);

				string html = ItemRowTemplate;
				html = html.Replace ("@@ITEM_UNIQUE_ID@@", entry.UniqueId);
				html = html.Replace ("@@ITEM_NAME@@", entry.Name);
				html = html.Replace ("@@ITEM_INPUT_FILE@@", entry.InputFile);
				html = html.Replace ("@@IMG_SRC@@", graphFilename);
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

		private static double PercentDifference (long t1, long t2)
		{
			return (t2 - t1) / (double) t1;
		}

	}

}


