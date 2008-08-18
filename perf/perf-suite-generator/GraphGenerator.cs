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

	public static class GraphGenerator {

		public static void GenerateGraph (List <ResultDbEntry> resultList, string filename)
		{
			// FIXME Exception if more than 50 results...

			ImageSurface surface = new ImageSurface (Format.Rgb24, 103, 52);
			Context context = new Context (surface);

			// Fill with grad
			LinearGradient grad = new LinearGradient (0.0, 0.0, 0.0, 52.0);
			grad.AddColorStopRgb (0.0, new Color  (1.0, 1.0, 1.0));
			grad.AddColorStopRgb (1.0, new Color  (0.8, 0.8, 0.9));
			context.Pattern = grad;
			context.Paint ();

			// Frame
			context.SetSourceRGB (0, 0, 0);
			context.LineWidth = 1.0;
			context.Rectangle (0.5, 0.5, 102.0, 51.0);
			context.Stroke ();

			long denominator = (long) (FindBiggestResult (resultList) * 1.2);

			context.LineWidth = 1.5;

			// FIXME Reverse to copy
			resultList.Reverse ();

			double x = 100.5 - ((resultList.Count - 1) * 2.0);
			bool hasPrevResult = false;
			long prevResult = 0;

			foreach (ResultDbEntry entry in resultList) {

				if (entry.Failure) {
					x += 2.0;
					continue;
				}

				double sz = ((double) entry.Time / denominator) * 50.0;

				if (hasPrevResult && UtilFu.GetValueDifference (prevResult, entry.Time) > 0.1)
					context.SetSourceRGB (1.0, 0.0, 0.0);
				else if (hasPrevResult && UtilFu.GetValueDifference (prevResult, entry.Time) < -0.1)
					context.SetSourceRGB (0.0, 1.0, 0.0);
				else
					context.SetSourceRGB (0.4, 0.4, 0.4);
 
				context.MoveTo (x, 51);
				context.LineTo (x, 51 - sz);
				context.Stroke ();

				x += 2.0;

				hasPrevResult = true;
				prevResult = entry.Time;
			}

			surface.WriteToPng (filename);

			resultList.Reverse ();
			((IDisposable) context).Dispose ();
			((IDisposable) surface).Dispose ();
		}

		private static long FindBiggestResult (List <ResultDbEntry> resultList)
		{
			long biggest = 0;
			foreach (ResultDbEntry entry in resultList) 
				biggest = Math.Max (biggest, entry.Time);

			return biggest;
		}

	}
}


