/*
 * Typeface.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

namespace System.Windows.Media {
	public class Typeface {
		internal Typeface ()
		{
			// This ctor is here to prevent the compiler from adding a default ctor
		}

		public bool TryGetGlyphTypeface (out GlyphTypeface glyphTypeface)
		{
			Console.WriteLine ("NIEX: System.Windows.Media.Typeface:.TryGetGlyphTypeface");
			throw new NotImplementedException ();
		}
	}
}