//
// System.Windows.Markup.XmlLanguage.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008, 2009 Novell, Inc (http://www.novell.com)
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

using Mono;
using System;
using System.Globalization;
using System.Windows;
using System.Windows.Media;

namespace System.Windows.Markup {
	public sealed class XmlLanguage {
		string lang;

		internal XmlLanguage (string ietfLanguageTag)
		{
			lang = ietfLanguageTag.ToLower (CultureInfo.InvariantCulture);
		}
		
		public static XmlLanguage GetLanguage (string ietfLanguageTag)
		{
			char c;
			int i;
			
			// verify language code is all us-ascii alphabetic
			for (i = 0; i < ietfLanguageTag.Length; i++) {
				if (ietfLanguageTag[i] == '-')
					break;
				
				c = Char.ToLower (ietfLanguageTag[i]);
				
				if (c < 'a' || c > 'z')
					throw new ArgumentException ("ietfLanguageTag");
			}
			
			// language code needs at least 2 characters (if non-empty)
			if (i > 0 && i < 2)
				throw new ArgumentException ("ietfLanguageTag");
			
			if (i < ietfLanguageTag.Length) {
				i++;
				
				// country code needs at least 2 characters
				if ((ietfLanguageTag.Length - i) < 2)
					throw new ArgumentException ("ietfLanguageTag");
				
				// verify country code is all us-ascii alpha-numeric
				for ( ; i < ietfLanguageTag.Length; i++) {
					c = Char.ToLower (ietfLanguageTag[i]);
					
					if (!(c >= 'a' && c <= 'z') && !(c >= '0' && c <= '9'))
						throw new ArgumentException ("ietfLanguageTag");
				}
			}
			
			return new XmlLanguage (ietfLanguageTag);
		}
		
		public string IetfLanguageTag {
			get { return lang; }
		}
		
		public override bool Equals (object obj)
		{
			XmlLanguage lang = obj as XmlLanguage;
			
			if (lang == null)
				return false;
			
			return IetfLanguageTag.Equals (lang.IetfLanguageTag);
		}
		
		public override int GetHashCode ()
		{
			return IetfLanguageTag.GetHashCode ();
		}
		
		public static bool operator ==(XmlLanguage xmlLanguage1, XmlLanguage xmlLanguage2)
		{
			if ( object.ReferenceEquals(xmlLanguage1, null) 
				|| object.ReferenceEquals(xmlLanguage2, null) )
				return false;

			return xmlLanguage1.IetfLanguageTag == xmlLanguage2.IetfLanguageTag;
		}
		
		public static bool operator !=(XmlLanguage xmlLanguage1, XmlLanguage xmlLanguage2)
		{
			return !(xmlLanguage1 == xmlLanguage2);
		}
	}
}
