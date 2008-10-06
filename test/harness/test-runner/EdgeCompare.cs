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
// Copyright (c) 2007-2008 Novell, Inc.
//
// Authors:
//	Jackson Harper (jackson@ximian.com)
//


using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections.Generic;


namespace MoonlightTests {

	public static class EdgeCompare {

		private static readonly int MinDiff = 10;
		private static readonly double Tolerance = 0.10;
		private static readonly int MaxMissingPoints = 150;

		private static readonly int KeyHeight = 50;

		public static TestResult CompareBitmaps (Test test, Bitmap result, Bitmap master)
		{
			using (Bitmap result_edges = BuildEdges (result)) {
				using (Bitmap master_edges = BuildEdges (master)) {
					using (Bitmap diff = new Bitmap (result.Width, result.Height + KeyHeight)) {
						double point_count = 0;
						double diff_score = 0;
						int missing_points = 0;
						
						using (Graphics g = Graphics.FromImage (diff)) {
							g.DrawImage (master_edges, 0, 0);
						}
						
						for (int x = 0; x < result.Width; x++) {
							
							// Cut off the top/bottom because the blur doesn't work correctly on these rows
							for (int y = 5; y < result.Height - 5; y++) {
								Color rc = result_edges.GetPixel (x, y);
								Color mc = master_edges.GetPixel (x, y);
								
								if (mc.ToArgb () == Color.Black.ToArgb ())
									point_count++;							
								
								if (rc.ToArgb () != mc.ToArgb ()) {
									int severity = PixelDiffSeverity (x, y, result_edges, master_edges);
									
									diff_score += severity;
									if (severity == 20)
										missing_points++;
									
									int red = Math.Min (255, (int) (severity / 10.0 * 255) + 100);
									diff.SetPixel (x, y, Color.FromArgb (red, 255, 0, 0));
								}
							}
						}
						
						AddDiffKey (diff);

						diff.Save (String.Concat (test.LocalFilePath, "-edge-diff.png"), ImageFormat.Png);
						
						if (diff_score / (point_count * 10) > Tolerance) {
							test.SetFailedReason (String.Format ("Edge difference was too great ({0})",
									diff_score / (point_count * 10)));
							return TestResult.Fail;
						}
	
						if (missing_points > MaxMissingPoints) {
							test.SetFailedReason (String.Format ("Too many missing points in edge compare ({0}).", missing_points));
							return TestResult.Fail;
						}
						
					}
				}
			}

			return TestResult.Pass;
		}

		private static Bitmap BuildEdges (Bitmap source)
		{
			Bitmap res = new Bitmap (source.Width, source.Height);
			
			using (Graphics g = Graphics.FromImage (res)) {
				g.Clear (Color.White);
	
				for (int x = 0; x < source.Width - 1; x++) {
					for (int y = 0; y < source.Height - 1; y++) {
						Color c = source.GetPixel (x, y);
						Color nx = source.GetPixel (x + 1, y);
						Color ny = source.GetPixel (x, y + 1);
	
						if ((Math.Sqrt ((c.R - nx.R) * (c.R - nx.R) + (c.G - nx.G) * (c.G - nx.G) + (c.B - nx.B) * (c.B - nx.B)) >= MinDiff) ||
						    (Math.Sqrt ((c.R - ny.R) * (c.R - ny.R) + (c.G - ny.G) * (c.G - ny.G) + (c.B - ny.B) * (c.B - ny.B)) >= MinDiff)) {
	
							res.SetPixel (x, y, Color.Black);
						}
					}
				}
			}
			return res;
		}
/*
		private static Bitmap BlurImage (Bitmap source)
		{
			Bitmap temp = new Bitmap (source.Width, source.Height);
			Bitmap res = new Bitmap (source.Width, source.Height);
			int gauss_width = 7;
 
			int sumr = 0;
			int sumg = 0;
			int sumb = 0;
    
			int [] gauss_fact = new int [] { 1, 6, 15, 20, 15, 6, 1 };
			int gauss_sum = 64;

			for (int i = 1; i < source.Width - 1; i++) {
				for (int j = 1; j < source.Height - 1; j++) {
					sumr = 0;
					sumg = 0;
					sumb = 0;
					for (int k = 0; k < gauss_width; k++) {

						int x = Math.Min (source.Width - 1, Math.Max (0, i - ((gauss_width - 1) >> 1) + k));

						Color color = source.GetPixel (x, j);
						sumr += color.R * gauss_fact [k];
						sumg += color.G * gauss_fact [k];
						sumb += color.B * gauss_fact [k];

					}
					temp.SetPixel (i, j, Color.FromArgb (Math.Min (255, sumr / gauss_sum),
							Math.Min (255, sumg / gauss_sum), Math.Min (255, sumb / gauss_sum)));
				} 
			}

			using (Graphics g = Graphics.FromImage (res)) {
				g.DrawImage (source, 0, 0);
			}

			for (int i = 1; i < source.Width - 1; i++) {
				for (int j = 1; j < source.Height - 1; j++) {
					for (int k = 0; k < gauss_width; k++) {

						int y = Math.Min (source.Height - 1, Math.Max (0, j - ((gauss_width - 1) >> 1) + k));

						Color color = temp.GetPixel (i, y);

						sumr += color.R * gauss_fact [k];
						sumg += color.G * gauss_fact [k];
						sumb += color.B * gauss_fact [k];
					}

					sumr /= gauss_sum;
					sumg /= gauss_sum;
					sumb /= gauss_sum;

					res.SetPixel (i, j, Color.FromArgb (Math.Min (255, sumr), Math.Min (255, sumg), Math.Min (255, sumb)));
				}
			}
			temp.Dispose ();
			return res;
		}
*/
		private static int PixelDiffSeverity (int x, int y, Bitmap result_edges, Bitmap master_edges)
		{
			Bitmap check = (master_edges.GetPixel (x, y).ToArgb () == Color.Black.ToArgb () ? result_edges : master_edges);

			int res = 20;
			for (int r = y - 5; r < y + 5; r++) {
				for (int c = x - 5; c < x + 5; c++) {
					Color color = check.GetPixel (Math.Max (0, Math.Min (check.Width - 1, c)), Math.Max (0, Math.Min (check.Height - 1, r)));

					if (color.ToArgb () == Color.Black.ToArgb ()) {
						int t = Math.Abs (c - x) + Math.Abs (r - y);
						if (t < res)
							res = t;
					}
				}
			}

			return res;
		}

		private static void AddDiffKey (Bitmap diff)
		{
			int y = diff.Height - KeyHeight;

			Font font = new Font ("Arial", 10);
			Brush black = new SolidBrush (Color.Black);
			Brush red = new SolidBrush (Color.Red);

			using (Graphics g = Graphics.FromImage (diff)) {
				g.FillRectangle (black, 10, y + 7, 10, 10);
				g.DrawString ("Edges", font, black, new Point (25, y));

				g.FillRectangle (red, 10, y + 32, 10, 10);
				g.DrawString ("Differences", font, red, new Point (25, y + 25));
			}
			black.Dispose ();
			red.Dispose ();
			font.Dispose ();
		}
	}

}

