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
		GlyphTypeface typeface;
		
		internal Typeface (GlyphTypeface gtf)
		{
			typeface = gtf;
		}
		
		public bool TryGetGlyphTypeface (out GlyphTypeface glyphTypeface)
		{
			glyphTypeface = typeface;
			return typeface != null;
		}
	}
}
