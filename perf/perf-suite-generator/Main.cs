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
using System.IO;
using System.Xml;
using PerfSuiteLib;
using System.Collections.Generic;

namespace PerfSuiteGenerator {

	public static class PerfSuiteGenerator {

		public static int Main (string [] args)
		{
			double p = 0.5;
			List <ResultDbEntry> list = new List <ResultDbEntry> ();
			for (int i = 0; i < 50; i++) {
				ResultDbEntry result = new ResultDbEntry ();
				if (i < 25) 
					result.Time = (long) (Math.Sin (p) * 100 + 100);
				else
					result.Time = (long) (Math.Cos (p) * 100 + 100);

				list.Add (result);
				p += 0.03;
			}

			GraphGenerator.GenerateGraph (list, "test.png");

			return 0;
		}

	}

}


