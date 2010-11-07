/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * richtextlayout.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <ctype.h>
#include <math.h>

#include "moon-path.h"
#include "textlayout.h"
#include "richtextlayout.h"
#include "richtextbox.h"
#include "textelement.h"
#include "debug.h"
#include "textblock.h"
#include "textbox.h"

namespace Moonlight {

#if DEBUG
#define d(x) if (debug_flags & RUNTIME_DEBUG_LAYOUT) x
#else
#define d(x)
#endif

#if DEBUG
static const char *unicode_break_types[] = {
	"G_UNICODE_BREAK_MANDATORY",
	"G_UNICODE_BREAK_CARRIAGE_RETURN",
	"G_UNICODE_BREAK_LINE_FEED",
	"G_UNICODE_BREAK_COMBINING_MARK",
	"G_UNICODE_BREAK_SURROGATE",
	"G_UNICODE_BREAK_ZERO_WIDTH_SPACE",
	"G_UNICODE_BREAK_INSEPARABLE",
	"G_UNICODE_BREAK_NON_BREAKING_GLUE",
	"G_UNICODE_BREAK_CONTINGENT",
	"G_UNICODE_BREAK_SPACE",
	"G_UNICODE_BREAK_AFTER",
	"G_UNICODE_BREAK_BEFORE",
	"G_UNICODE_BREAK_BEFORE_AND_AFTER",
	"G_UNICODE_BREAK_HYPHEN",
	"G_UNICODE_BREAK_NON_STARTER",
	"G_UNICODE_BREAK_OPEN_PUNCTUATION",
	"G_UNICODE_BREAK_CLOSE_PUNCTUATION",
	"G_UNICODE_BREAK_QUOTATION",
	"G_UNICODE_BREAK_EXCLAMATION",
	"G_UNICODE_BREAK_IDEOGRAPHIC",
	"G_UNICODE_BREAK_NUMERIC",
	"G_UNICODE_BREAK_INFIX_SEPARATOR",
	"G_UNICODE_BREAK_SYMBOL",
	"G_UNICODE_BREAK_ALPHABETIC",
	"G_UNICODE_BREAK_PREFIX",
	"G_UNICODE_BREAK_POSTFIX",
	"G_UNICODE_BREAK_COMPLEX_CONTEXT",
	"G_UNICODE_BREAK_AMBIGUOUS",
	"G_UNICODE_BREAK_UNKNOWN",
	"G_UNICODE_BREAK_NEXT_LINE",
	"G_UNICODE_BREAK_WORD_JOINER",
	"G_UNICODE_BREAK_HANGUL_L_JAMO",
	"G_UNICODE_BREAK_HANGUL_V_JAMO",
	"G_UNICODE_BREAK_HANGUL_T_JAMO",
	"G_UNICODE_BREAK_HANGUL_LV_SYLLABLE",
	"G_UNICODE_BREAK_HANGUL_LVT_SYLLABLE"
};

static const char *unicode_char_types[] = {
	"G_UNICODE_CONTROL",
	"G_UNICODE_FORMAT",
	"G_UNICODE_UNASSIGNED",
	"G_UNICODE_PRIVATE_USE",
	"G_UNICODE_SURROGATE",
	"G_UNICODE_LOWERCASE_LETTER",
	"G_UNICODE_MODIFIER_LETTER",
	"G_UNICODE_OTHER_LETTER",
	"G_UNICODE_TITLECASE_LETTER",
	"G_UNICODE_UPPERCASE_LETTER",
	"G_UNICODE_COMBINING_MARK",
	"G_UNICODE_ENCLOSING_MARK",
	"G_UNICODE_NON_SPACING_MARK",
	"G_UNICODE_DECIMAL_NUMBER",
	"G_UNICODE_LETTER_NUMBER",
	"G_UNICODE_OTHER_NUMBER",
	"G_UNICODE_CONNECT_PUNCTUATION",
	"G_UNICODE_DASH_PUNCTUATION",
	"G_UNICODE_CLOSE_PUNCTUATION",
	"G_UNICODE_FINAL_PUNCTUATION",
	"G_UNICODE_INITIAL_PUNCTUATION",
	"G_UNICODE_OTHER_PUNCTUATION",
	"G_UNICODE_OPEN_PUNCTUATION",
	"G_UNICODE_CURRENCY_SYMBOL",
	"G_UNICODE_MODIFIER_SYMBOL",
	"G_UNICODE_MATH_SYMBOL",
	"G_UNICODE_OTHER_SYMBOL",
	"G_UNICODE_LINE_SEPARATOR",
	"G_UNICODE_PARAGRAPH_SEPARATOR",
	"G_UNICODE_SPACE_SEPARATOR"
};
#endif

#define UnicharIsLineBreak(c) ((c) == '\r' || (c) == '\n' || (c) == 0x2028)

#define BreakSpace(c, btype) (c == '\t' || btype == G_UNICODE_BREAK_SPACE || btype == G_UNICODE_BREAK_ZERO_WIDTH_SPACE)

enum WordType {
	WORD_TYPE_UNKNOWN,
	WORD_TYPE_ALPHABETIC,
	WORD_TYPE_IDEOGRAPHIC,
	WORD_TYPE_INSEPARABLE,
	WORD_TYPE_NUMERIC,
	WORD_TYPE_HANGUL,
};


/*
 * Silverlight does not apply any kerning on a DOT, so we exclude them
 * 	U+002E FULL STOP
 * 	U+06D4 ARABIC FULL STOP
 *	U+3002 IDEOGRAPHIC FULL STOP
 * Note: this is different than using the "sliding dot" algorithm from
 * http://www.freetype.org/freetype2/docs/glyphs/glyphs-4.html
 */
#define APPLY_KERNING(uc)	((uc != 0x002E) && (uc != 0x06D4) && (uc != 3002))

static WordType
word_type (GUnicodeType ctype, GUnicodeBreakType btype)
{
	switch (btype) {
	case G_UNICODE_BREAK_ALPHABETIC:
		return WORD_TYPE_ALPHABETIC;
	case G_UNICODE_BREAK_IDEOGRAPHIC:
		return WORD_TYPE_IDEOGRAPHIC;
	case G_UNICODE_BREAK_NUMERIC:
		if (ctype == G_UNICODE_OTHER_PUNCTUATION)
			return WORD_TYPE_UNKNOWN;
		return WORD_TYPE_NUMERIC;
	case G_UNICODE_BREAK_INSEPARABLE:
		return WORD_TYPE_INSEPARABLE;
#if GLIB_CHECK_VERSION (2,10,0)
	case G_UNICODE_BREAK_HANGUL_LVT_SYLLABLE:
	case G_UNICODE_BREAK_HANGUL_LV_SYLLABLE:
	case G_UNICODE_BREAK_HANGUL_L_JAMO:
	case G_UNICODE_BREAK_HANGUL_V_JAMO:
	case G_UNICODE_BREAK_HANGUL_T_JAMO:
		return WORD_TYPE_HANGUL;
#endif
	default:
		return WORD_TYPE_UNKNOWN;
	}
}

static bool
word_type_changed (WordType wtype, gunichar c, GUnicodeType ctype, GUnicodeBreakType btype)
{
	WordType type;
	
	// compare this character's word-type against the current word-type
	if ((type = word_type (ctype, btype)) == wtype)
		return false;
	
	if (type == WORD_TYPE_UNKNOWN)
		return false;
	
	// word-types not identical... check if they are compatible
	switch (wtype) {
	case WORD_TYPE_ALPHABETIC:
		return type != WORD_TYPE_NUMERIC;
#if 0
	case WORD_TYPE_IDEOGRAPHIC:
		// this fixes drt #411 but breaks drt #208. I can't win.
		return type != WORD_TYPE_ALPHABETIC;
#endif
	default:
		return true;
	}
}

static inline gunichar
utf8_getc (const char **in, size_t inlen)
{
	register const unsigned char *inptr = (const unsigned char *) *in;
	const unsigned char *inend = inptr + inlen;
	register unsigned char c, r;
	register gunichar m, u = 0;
	
	if (inlen == 0)
		return 0;
	
	r = *inptr++;
	if (r < 0x80) {
		*in = (const char *) inptr;
		u = r;
	} else if (r < 0xfe) { /* valid start char? */
		u = r;
		m = 0x7f80;    /* used to mask out the length bits */
		do {
			if (inptr >= inend) {
				*in = (const char *) inend;
				return 0;
			}
			
			c = *inptr++;
			if ((c & 0xc0) != 0x80)
				goto error;
			
			u = (u << 6) | (c & 0x3f);
			r <<= 1;
			m <<= 5;
		} while (r & 0x40);
		
		*in = (const char *) inptr;
		
		u &= ~m;
	} else {
	 error:
		u = (gunichar) -1;
		*in = (*in) + 1;
	}
	
	return u;
}

static int
unichar_combining_class (gunichar c)
{
#if GLIB_CHECK_VERSION (2,14,0)
	if (glib_check_version (2,14,0))
		return g_unichar_combining_class (c);
	else
#endif
	
	return 0;
}

struct WordBreakOpportunity {
	GUnicodeBreakType btype;
	const char *inptr;
	double advance;
	guint32 index;
	gunichar c;
	int count;
};

struct LayoutWord {
	// <internal use>
	GArray *break_ops;     // TextWrappingWrap only
	
	WordType type;
	
	// <input>
	double line_advance;
	TextFont *font;
	
	// <input/output>
	GlyphInfo *prev;       // previous glyph; used for kerning
	
	// <output>
	double advance;        // the advance-width of the 'word'
	int length;            // length of the word in bytes
	int count;             // length of the word in unichars
};

typedef bool (* LayoutWordCallback) (LayoutWord *word, const char *in, const char *inend, double max_width);

static inline bool
IsLineBreak (const char *text, size_t left, size_t *n_bytes, size_t *n_chars)
{
	const char *inptr = text;
	gunichar c;
	
	if ((c = utf8_getc (&inptr, left)) == (gunichar) -1)
		return false;
	
	if (!UnicharIsLineBreak (c))
		return false;
	
	if (c == '\r' && *inptr == '\n') {
		*n_bytes = 2;
		*n_chars = 2;
	} else {
		*n_bytes = (size_t) (inptr - text);
		*n_chars = 1;
	}
	
	return true;
}

static inline void
layout_word_init (LayoutWord *word, double line_advance, GlyphInfo *prev)
{
	word->line_advance = line_advance;
	word->prev = prev;
}

/**
 * layout_lwsp:
 * @word: #LayoutWord context
 * @in: input text
 * @inend: end of input text
 *
 * Measures a word containing nothing but LWSP.
 **/
static void
layout_lwsp (LayoutWord *word, const char *in, const char *inend)
{
	GlyphInfo *prev = word->prev;
	GUnicodeBreakType btype;
	const char *inptr = in;
	const char *start;
	GlyphInfo *glyph;
	double advance;
	gunichar c;
	
	d(printf ("RichTextLayout::layout_lwsp():\n"));
	
	word->advance = 0.0;
	word->count = 0;
	
	while (inptr < inend) {
		start = inptr;
		if ((c = utf8_getc (&inptr, inend - inptr)) == (gunichar) -1) {
			// ignore invalid chars
			continue;
		}
		
		if (UnicharIsLineBreak (c)) {
			inptr = start;
			break;
		}
		
		btype = g_unichar_break_type (c);
		if (!BreakSpace (c, btype)) {
			inptr = start;
			break;
		}
		
#if DEBUG
		if (debug_flags & RUNTIME_DEBUG_LAYOUT) {
			if (c < 128 && isprint ((int) c))
				printf ("\tunichar = %c; btype = %s, ctype = %s\n", (char) c,
					unicode_break_types[btype], unicode_char_types[g_unichar_type (c)]);
			else
				printf ("\tunichar = 0x%.4X; btype = %s, ctype = %s\n", c,
					unicode_break_types[btype], unicode_char_types[g_unichar_type (c)]);
		}
#endif
		
		word->count++;
		
		// treat tab as a single space
		if (c == '\t')
			c = ' ';
		
		// ignore glyphs the font doesn't contain...
		if (!(glyph = word->font->GetGlyphInfo (c)))
			continue;
		
		// calculate total glyph advance
		advance = glyph->metrics.horiAdvance;
		if ((prev != NULL) && APPLY_KERNING (c))
			advance += word->font->Kerning (prev, glyph);
		else if (glyph->metrics.horiBearingX < 0)
			advance -= glyph->metrics.horiBearingX;
		
		word->line_advance += advance;
		word->advance += advance;
		prev = glyph;
	}
	
	word->length = (inptr - in);
	word->prev = prev;
}

/**
 * layout_word_nowrap:
 * @word: #LayoutWord context
 * @in: input text
 * @inend = end of input text
 * @max_width: max allowable width for a line
 *
 * Calculates the advance of the current word.
 *
 * Returns: %true if the caller should create a new line for the
 * remainder of the word or %false otherwise.
 **/
static bool
layout_word_nowrap (LayoutWord *word, const char *in, const char *inend, double max_width)
{
	GUnicodeBreakType btype = G_UNICODE_BREAK_UNKNOWN;
	GlyphInfo *prev = word->prev;
	const char *inptr = in;
	const char *start;
	GlyphInfo *glyph;
	double advance;
	gunichar c;
	
	// Note: since we don't ever need to wrap, no need to keep track of word-type
	word->type = WORD_TYPE_UNKNOWN;
	word->advance = 0.0;
	word->count = 0;
	
	while (inptr < inend) {
		start = inptr;
		if ((c = utf8_getc (&inptr, inend - inptr)) == (gunichar) -1) {
			// ignore invalid chars
			continue;
		}
		
		if (UnicharIsLineBreak (c)) {
			inptr = start;
			break;
		}
		
		if (btype == G_UNICODE_BREAK_COMBINING_MARK) {
			// ignore zero-width spaces
			if ((btype = g_unichar_break_type (c)) == G_UNICODE_BREAK_ZERO_WIDTH_SPACE)
				btype = G_UNICODE_BREAK_COMBINING_MARK;
		} else {
			btype = g_unichar_break_type (c);
		}
		
		if (BreakSpace (c, btype)) {
			inptr = start;
			break;
		}
		
		word->count++;
		
		// ignore glyphs the font doesn't contain...
		if (!(glyph = word->font->GetGlyphInfo (c)))
			continue;
		
		// calculate total glyph advance
		advance = glyph->metrics.horiAdvance;
		if ((prev != NULL) && APPLY_KERNING (c))
			advance += word->font->Kerning (prev, glyph);
		else if (glyph->metrics.horiBearingX < 0)
			advance -= glyph->metrics.horiBearingX;
		
		word->line_advance += advance;
		word->advance += advance;
		prev = glyph;
	}
	
	word->length = (inptr - in);
	word->prev = prev;
	
	return false;
}

/**
 * layout_word_wrap:
 * @word: word state
 * @in: start of word
 * @inend = end of word
 * @max_width: max allowable width for a line
 *
 * Calculates the advance of the current word, breaking if needed.
 *
 * Returns: %true if the caller should create a new line for the
 * remainder of the word or %false otherwise.
 **/
static bool
layout_word_wrap (LayoutWord *word, const char *in, const char *inend, double max_width)
{
	GUnicodeBreakType btype = G_UNICODE_BREAK_UNKNOWN;
	bool line_start = word->line_advance == 0.0;
	GlyphInfo *prev = word->prev;
	WordBreakOpportunity op;
	const char *inptr = in;
	const char *start;
	GUnicodeType ctype;
	bool force = false;
	bool fixed = false;
	bool wrap = false;
	GlyphInfo *glyph;
#if DEBUG
	GString *debug;
#endif
	double advance;
	int glyphs = 0;
	bool new_glyph;
	gunichar c;
	
	g_array_set_size (word->break_ops, 0);
	word->type = WORD_TYPE_UNKNOWN;
	word->advance = 0.0;
	word->count = 0;
	
	d(printf ("RichTextLayout::layout_word_wrap():\n"));
	d(debug = g_string_new (""));
	
	while (inptr < inend) {
		start = inptr;
		if ((c = utf8_getc (&inptr, inend - inptr)) == (gunichar) -1) {
			// ignore invalid chars
			continue;
		}
		
		if (UnicharIsLineBreak (c)) {
			inptr = start;
			break;
		}
		
		// check the previous break-type
		if (btype == G_UNICODE_BREAK_CLOSE_PUNCTUATION) {
			// if anything other than an infix separator come after a close-punctuation, then the 'word' is done
			btype = g_unichar_break_type (c);
			if (btype != G_UNICODE_BREAK_INFIX_SEPARATOR) {
				inptr = start;
				break;
			}
		} else if (btype == G_UNICODE_BREAK_INFIX_SEPARATOR) {
			btype = g_unichar_break_type (c);
			if (word->type == WORD_TYPE_NUMERIC) {
				// only accept numbers after the infix
				if (btype != G_UNICODE_BREAK_NUMERIC) {
					inptr = start;
					break;
				}
			} else if (word->type == WORD_TYPE_UNKNOWN) {
				// only accept alphanumerics after the infix
				if (btype != G_UNICODE_BREAK_ALPHABETIC && btype != G_UNICODE_BREAK_NUMERIC) {
					inptr = start;
					break;
				}
				
				fixed = true;
			}
		} else if (btype == G_UNICODE_BREAK_WORD_JOINER) {
			btype = g_unichar_break_type (c);
			fixed = true;
		} else {
			btype = g_unichar_break_type (c);
		}
		
		if (BreakSpace (c, btype)) {
			inptr = start;
			break;
		}
		
		ctype = g_unichar_type (c);
		
		if (word->type == WORD_TYPE_UNKNOWN) {
			// record our word-type
			word->type = word_type (ctype, btype);
		} else if (btype == G_UNICODE_BREAK_OPEN_PUNCTUATION) {
			// this is a good place to break
			inptr = start;
			break;
		} else if (word_type_changed (word->type, c, ctype, btype)) {
			// changing word-types, don't continue
			inptr = start;
			break;
		}
		
		d(g_string_append_unichar (debug, c));
		word->count++;
		
		// a Combining Class of 0 means start of a new glyph
		if (glyphs > 0 && unichar_combining_class (c) != 0) {
			// this char gets combined with the previous glyph
			new_glyph = false;
		} else {
			new_glyph = true;
			glyphs++;
		}
		
#if DEBUG
		if (debug_flags & RUNTIME_DEBUG_LAYOUT) {
			if (c < 128 && isprint ((int) c))
				printf ("\tunichar = %c; btype = %s, new glyph = %s; cc = %d; ctype = %s\n", (char) c,
					unicode_break_types[btype], new_glyph ? "true" : "false", unichar_combining_class (c),
					unicode_char_types[ctype]);
			else
				printf ("\tunichar = 0x%.4X; btype = %s, new glyph = %s; cc = %d; ctype = %s\n", c,
					unicode_break_types[btype], new_glyph ? "true" : "false", unichar_combining_class (c),
					unicode_char_types[ctype]);
		}
#endif
		
		if ((glyph = word->font->GetGlyphInfo (c))) {
			// calculate total glyph advance
			advance = glyph->metrics.horiAdvance;
			if ((prev != NULL) && APPLY_KERNING (c))
				advance += word->font->Kerning (prev, glyph);
			else if (glyph->metrics.horiBearingX < 0)
				advance -= glyph->metrics.horiBearingX;
			
			word->line_advance += advance;
			word->advance += advance;
			prev = glyph;
		} else {
			advance = 0.0;
		}
		
		if (new_glyph) {
			op.index = glyph ? glyph->index : 0;
			op.advance = word->advance;
			op.count = word->count;
			op.inptr = inptr;
			op.btype = btype;
			op.c = c;
		} else {
			g_array_remove_index (word->break_ops, word->break_ops->len - 1);
			op.advance += advance;
			op.inptr = inptr;
			op.count++;
		}
		
		g_array_append_val (word->break_ops, op);
		
		if (!isinf (max_width) && word->line_advance > max_width) {
			d(printf ("\tjust exceeded max width (%fpx): %s\n", max_width, debug->str));
			wrap = true;
			break;
		}
	}
	
	if (!wrap) {
		d(g_string_free (debug, true));
		word->length = (inptr - in);
		word->prev = prev;
		return false;
	}
	
	d(printf ("\tcollecting any/all decomposed chars and figuring out the break-type for the next glyph\n"));
	
	// pretend btype is SPACE here in case inptr is at the end of the run
	if (inptr == inend)
		btype = G_UNICODE_BREAK_SPACE;
	
	// keep going until we reach a new distinct glyph. we also
	// need to know the btype of the char after the char that
	// exceeded the width limit.
	while (inptr < inend) {
		start = inptr;
		if ((c = utf8_getc (&inptr, inend - inptr)) == (gunichar) -1) {
			// ignore invalid chars
			continue;
		}
		
		if (UnicharIsLineBreak (c)) {
			btype = G_UNICODE_BREAK_SPACE;
			inptr = start;
			break;
		}
		
		btype = g_unichar_break_type (c);
		if (BreakSpace (c, btype) || unichar_combining_class (c) == 0) {
			inptr = start;
			break;
		}
		
		d(g_string_append_unichar (debug, c));
		word->count++;
		
		if ((glyph = word->font->GetGlyphInfo (c))) {
			// calculate total glyph advance
			advance = glyph->metrics.horiAdvance;
			if ((prev != NULL) && APPLY_KERNING (c))
				advance += word->font->Kerning (prev, glyph);
			else if (glyph->metrics.horiBearingX < 0)
				advance -= glyph->metrics.horiBearingX;
			
			word->line_advance += advance;
			word->advance += advance;
			prev = glyph;
		} else {
			advance = 0.0;
		}
		
		g_array_remove_index (word->break_ops, word->break_ops->len - 1);
		op.advance += advance;
		op.inptr = inptr;
		op.count++;
		g_array_append_val (word->break_ops, op);
	}
	
	d(printf ("\tok, at this point we have: %s\n", debug->str));
	d(printf ("\tnext break-type is %s\n", unicode_break_types[btype]));
	d(g_string_free (debug, true));
	
	// at this point, we're going to break the word so we can reset kerning
	word->prev = 0;
	
	// we can't break any smaller than a single glyph
	if (line_start && glyphs == 1) {
		d(printf ("\tsince this is the first glyph on the line, can't break any smaller than that...\n"));
		word->length = (inptr - in);
		word->prev = prev;
		return true;
	}
	
 retry:
	
	// search backwards for the best break point
	d(printf ("\tscanning over %d break opportunities...\n", word->break_ops->len));
	for (guint i = word->break_ops->len; i > 0; i--) {
		op = g_array_index (word->break_ops, WordBreakOpportunity, i - 1);
		
#if DEBUG
		if (debug_flags & RUNTIME_DEBUG_LAYOUT) {
			if (op.c < 128 && isprint ((int) op.c))
				printf ("\tunichar = %c; btype = %s; i = %d\n", (char) op.c, unicode_break_types[op.btype], i);
			else
				printf ("\tunichar = 0x%.4X; btype = %s; i = %d\n", op.c, unicode_break_types[op.btype], i);
		}
#endif
		
		switch (op.btype) {
		case G_UNICODE_BREAK_BEFORE_AND_AFTER:
			if (i > 1 && i == word->break_ops->len) {
				// break after the previous glyph
				op = g_array_index (word->break_ops, WordBreakOpportunity, i - 2);
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			} else if (i < word->break_ops->len) {
				// break after this glyph
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			}
		case G_UNICODE_BREAK_NON_BREAKING_GLUE:
		case G_UNICODE_BREAK_WORD_JOINER:
			// cannot break before or after this character (unless forced)
			if (force && i < word->break_ops->len) {
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			}
			
			if (i > 1) {
				// skip past previous glyph
				op = g_array_index (word->break_ops, WordBreakOpportunity, i - 2);
				i--;
			}
			break;
		case G_UNICODE_BREAK_INSEPARABLE:
			// only restriction is no breaking between inseparables unless we have to
			if (line_start && i < word->break_ops->len) {
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			}
			break;
		case G_UNICODE_BREAK_BEFORE:
			if (i > 1) {
				// break after the previous glyph
				op = g_array_index (word->break_ops, WordBreakOpportunity, i - 2);
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			}
			break;
		case G_UNICODE_BREAK_CLOSE_PUNCTUATION:
			if (i < word->break_ops->len && (force || btype != G_UNICODE_BREAK_INFIX_SEPARATOR)) {
				// we can safely break after this character
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			}
			
			if (i > 1 && !force) {
				// we can never break before a closing punctuation, so skip past prev char
				op = g_array_index (word->break_ops, WordBreakOpportunity, i - 2);
				i--;
			}
			break;
		case G_UNICODE_BREAK_INFIX_SEPARATOR:
			if (i < word->break_ops->len && (force || btype != G_UNICODE_BREAK_NUMERIC)) {
				// we can safely break after this character
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			}
			
			if (i > 1 && !force) {
				// we can never break before an infix, skip past prev char
				op = g_array_index (word->break_ops, WordBreakOpportunity, i - 2);
				if (op.btype == G_UNICODE_BREAK_INFIX_SEPARATOR ||
				    op.btype == G_UNICODE_BREAK_CLOSE_PUNCTUATION) {
					// unless previous char is one of these special types...
					op = g_array_index (word->break_ops, WordBreakOpportunity, i - 1);
				} else {
					i--;
				}
			}
			break;
		case G_UNICODE_BREAK_ALPHABETIC:
			// only break if we have no choice...
			if ((line_start || fixed || force) && i < word->break_ops->len) {
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			}
			break;
		case G_UNICODE_BREAK_IDEOGRAPHIC:
			if (i < word->break_ops->len && btype != G_UNICODE_BREAK_NON_STARTER) {
				// we can safely break after this character
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			}
			break;
		case G_UNICODE_BREAK_NUMERIC:
			// only break if we have no choice...
			if (line_start && i < word->break_ops->len && (force || btype != G_UNICODE_BREAK_INFIX_SEPARATOR)) {
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			}
			break;
		case G_UNICODE_BREAK_OPEN_PUNCTUATION:
		case G_UNICODE_BREAK_COMBINING_MARK:
		case G_UNICODE_BREAK_CONTINGENT:
		case G_UNICODE_BREAK_AMBIGUOUS:
		case G_UNICODE_BREAK_QUOTATION:
		case G_UNICODE_BREAK_PREFIX:
			// do not break after characters with these break-types (unless forced)
			if (force && i < word->break_ops->len) {
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			}
			break;
		default:
			d(printf ("Unhandled Unicode break-type: %s\n", unicode_break_types[op.btype]));
			// fall thru to the "default" behavior
			
#if GLIB_CHECK_VERSION (2,10,0)
		case G_UNICODE_BREAK_HANGUL_LVT_SYLLABLE:
		case G_UNICODE_BREAK_HANGUL_LV_SYLLABLE:
		case G_UNICODE_BREAK_HANGUL_L_JAMO:
		case G_UNICODE_BREAK_HANGUL_V_JAMO:
		case G_UNICODE_BREAK_HANGUL_T_JAMO:
#endif
		case G_UNICODE_BREAK_NON_STARTER:
		case G_UNICODE_BREAK_EXCLAMATION:
		case G_UNICODE_BREAK_MANDATORY:
		case G_UNICODE_BREAK_NEXT_LINE:
		case G_UNICODE_BREAK_UNKNOWN:
		case G_UNICODE_BREAK_POSTFIX:
		case G_UNICODE_BREAK_HYPHEN:
		case G_UNICODE_BREAK_AFTER:
			if (i < word->break_ops->len) {
				// we can safely break after this character
				word->prev = word->font->GetGlyphInfo (op.c);
				word->length = (op.inptr - in);
				word->advance = op.advance;
				word->count = op.count;
				
				return true;
			}
			break;
		}
		
		btype = op.btype;
		c = op.c;
	}
	
	if (line_start && !force) {
		d(printf ("\tcouldn't find a good place to break but we must force a break, retrying...\n"));
		force = true;
		goto retry;
	}
	
	d(printf ("\tcouldn't find a good place to break, defaulting to breaking before the word start\n"));
	
	word->advance = 0.0;
	word->length = 0;
	word->count = 0;
	
	return true;
}

static LayoutWordCallback layout_word_behavior[] = {
	layout_word_wrap,
	layout_word_nowrap,
	layout_word_wrap
};

#if DEBUG
static const char *wrap_modes[3] = {
	"WrapWithOverflow",
	"NoWrap",
	"Wrap"
};

static void
print_lines (GPtrArray *lines)
{
	double y = 0.0;
	
	for (guint i = 0; i < lines->len; i++) {
		RichTextLayoutLine *line = (RichTextLayoutLine *) lines->pdata[i];
		
		printf ("Line (top=%f, height=%f, width=%f):\n", y, line->size.height, line->size.width);
		for (guint j = 0; j < line->inlines->len; j++) {
			RichTextLayoutInline *inl = (RichTextLayoutInline *) line->inlines->pdata[j];
			
			printf ("\tInline (height=%f,width=%f): \"", inl->size.height, inl->size.width);
#if notyet
			for (const char *s = text; s < text + run->length; s++) {
				switch (*s) {
				case '\r':
					fputs ("\\r", stdout);
					break;
				case '\n':
					fputs ("\\n", stdout);
					break;
				case '\t':
					fputs ("\\t", stdout);
					break;
				case '"':
					fputs ("\\\"", stdout);
					break;
				default:
					fputc (*s, stdout);
					break;
				}
			}
#endif
			printf ("\"\n");
		}
		
		y += line->size.height;
	}
}
#endif

struct TextRegion {
	int start, length;
	bool select;
};

#if notyet
static void
UpdateSelection (GPtrArray *lines, TextRegion *pre, TextRegion *post)
{
	TextLayoutGlyphCluster *cluster;
	TextLayoutLine *line;
	TextLayoutRun *run;
	guint i, j;
	
	// first update pre-region
	for (i = 0; i < lines->len; i++) {
		line = (TextLayoutLine *) lines->pdata[i];
		
		if (pre->start >= line->start + line->length) {
			// pre-region not on this line...
			continue;
		}
		
		for (j = 0; j < line->runs->len; j++) {
			run = (TextLayoutRun *) line->runs->pdata[j];
			
			if (pre->start >= run->start + run->length) {
				// pre-region not in this run...
				continue;
			}
			
			if (pre->start <= run->start) {
				if (pre->start + pre->length >= run->start + run->length) {
					// run is fully contained within the pre-region
					if (run->clusters->len == 1) {
						cluster = (TextLayoutGlyphCluster *) run->clusters->pdata[0];
						cluster->selected = pre->select;
					} else {
						run->ClearCache ();
					}
				} else {
					run->ClearCache ();
				}
			} else {
				run->ClearCache ();
			}
			
			if (pre->start + pre->length <= run->start + run->length)
				break;
		}
	}
	
	// now update the post region...
	for ( ; i < lines->len; i++, j = 0) {
		line = (RichTextLayoutLine *) lines->pdata[i];
		
		if (post->start >= line->start + line->length) {
			// pre-region not on this line...
			continue;
		}
		
		for ( ; j < line->runs->len; j++) {
			run = (TextLayoutRun *) line->runs->pdata[j];
			
			if (post->start >= run->start + run->length) {
				// post-region not in this run...
				continue;
			}
			
			if (post->start <= run->start) {
				if (post->start + post->length >= run->start + run->length) {
					// run is fully contained within the pre-region
					if (run->clusters->len == 1) {
						cluster = (TextLayoutGlyphCluster *) run->clusters->pdata[0];
						cluster->selected = post->select;
					} else {
						run->ClearCache ();
					}
				} else {
					run->ClearCache ();
				}
			} else {
				run->ClearCache ();
			}
			
			if (post->start + post->length <= run->start + run->length)
				break;
		}
	}
}
#endif

static double
GetWidthConstraint (double avail_width, double max_width, double actual_width)
{
	if (isinf (avail_width)) {
		// find an upper width constraint
		if (isinf (max_width))
			return actual_width;
		else
			return MIN (actual_width, max_width);
	}
	
	return avail_width;
}

//
// RichTextLayout
//

RichTextLayout::RichTextLayout ()
{
	// Note: TextBlock and TextBox assume their default values match these
	alignment = TextAlignmentLeft;
	wrapping = TextWrappingNoWrap;
	avail_width = INFINITY;
	max_height = INFINITY;
	max_width = INFINITY;
	actual_height = NAN;
	actual_width = NAN;
	lines = g_ptr_array_new ();
	is_wrapped = true;
	blocks = NULL;
}

RichTextLayout::~RichTextLayout ()
{
	ClearLines ();
	g_ptr_array_free (lines, true);
}

void
RichTextLayout::ClearLines ()
{
	for (guint i = 0; i < lines->len; i++)
		delete (RichTextLayoutLine *) lines->pdata[i];
	
	g_ptr_array_set_size (lines, 0);
}

void
RichTextLayout::ResetState ()
{
	actual_height = NAN;
	actual_width = NAN;
}

bool
RichTextLayout::SetTextAlignment (TextAlignment align)
{
	if (alignment == align)
		return false;
	
	alignment = align;
	
	return true;
}

bool
RichTextLayout::SetTextWrapping (TextWrapping mode)
{
	switch (mode) {
	case TextWrappingNoWrap:
	case TextWrappingWrap:
		break;
	default:
		// Silverlight defaults to Wrap for unknown values
		mode = TextWrappingWrap;
		break;
	}
	
	if (wrapping == mode)
		return false;
	
	wrapping = mode;
	
	return true;
}

bool
RichTextLayout::SetMaxHeight (double height)
{
	if (max_height == height)
		return false;
	
	max_height = height;
	
	return true;
}

bool
RichTextLayout::SetMaxWidth (double width)
{
	if (width == 0.0)
		width = INFINITY;
	
	if (max_width == width)
		return false;
	
	if (!is_wrapped && (isinf (width) || width > actual_width)) {
		// the new max_width won't change layout
		max_width = width;
		return false;
	}
	
	max_width = width;
	
	return true;
}

bool
RichTextLayout::SetBlocks (BlockCollection *blocks)
{
	if (this->blocks == blocks)
		return false;

	this->blocks = blocks;

	if (!this->blocks) {
		selection_start = TextPointer();
		selection_end = TextPointer();
	}

	return true;
}

void
RichTextLayout::ClearCache ()
{
#if notyet
	for (guint i = 0; i < lines->len; i++) {
		RichTextLayoutLine *line = (RichTextLayoutLine *) lines->pdata[i];
		
		for (guint j = 0; j < line->inlines->len; j++) {
			RichTextLayoutInline *inl = (RichTextLayoutInline *) line->inlines->pdata[j];
			inl->ClearCache ();
		}
	}
#endif
}

void
RichTextLayout::Select (TextSelection *selection)
{
#if notyet
	if (!blocks)
		return;

	ClearCurrentSelection ();

	if (selection->IsEmpty())
		SetCurrentSelection (NULL, NULL);
	else
		SetCurrentSelection (selection->GetStart(), selection->GetEnd());
#endif
}

void
RichTextLayout::GetActualExtents (double *width, double *height)
{
	*height = actual_height;
	*width = actual_width;
}

double
RichTextLayout::GetBaselineOffset ()
{
	if (lines->len == 0)
		return 0;

	RichTextLayoutLine *line = (RichTextLayoutLine*)lines->pdata[0];

	return line->size.height + line->descend;
}

RichTextLayoutLine *
RichTextLayout::GetLineFromIndex (int index)
{
	if (index >= (int) lines->len || index < 0)
		return NULL;
	
	return (RichTextLayoutLine *) lines->pdata[index];
}

Rect
RichTextLayout::GetCharacterRect (TextPointer *tp, LogicalDirection direction)
{
	/* find the line that contains the textpointer */
	RichTextLayoutLine *line = NULL;
	
	for (guint i = 0; i < lines->len; i++) {
		RichTextLayoutLine *l = (RichTextLayoutLine*)lines->pdata[i];
		if (tp->CompareTo_np(l->start) < 0) {
			// tp comes before line->start
			if (i == 0)
				line = (RichTextLayoutLine*)lines->pdata[0];
			else
				line = (RichTextLayoutLine*)lines->pdata[i-i];
			break;
		}
	}

	RichTextLayoutInline *inline_ = NULL;

	for (guint i = 0; i < line->inlines->len; i ++) {
		RichTextLayoutInline *inl = (RichTextLayoutInline*)line->inlines->pdata[i];

		if (tp->CompareTo_np (inl->start) >= 0 && tp->CompareTo_np (inl->end) <= 0) {
			inline_ = inl;
			break;
		}
	}

	if (!inline_) {
		inline_ = (RichTextLayoutInline*)line->inlines->pdata[0];
		tp = &inline_->start;
	}

	printf ("NIEX GetCharacterRect()\n");
	return Rect();
}

Rect
RichTextLayout::GetRenderExtents ()
{
	return Rect (HorizontalAlignment (actual_width), 0.0, actual_width, actual_height);
}

double
RichTextLayout::HorizontalAlignment (double line_width)
{
	double deltax;
	double width;
	
	switch (alignment) {
	case TextAlignmentCenter:
		width = GetWidthConstraint (avail_width, max_width, actual_width);
		if (line_width < width)
			deltax = (width - line_width) / 2.0;
		else
			deltax = 0.0;
		break;
	case TextAlignmentRight:
		width = GetWidthConstraint (avail_width, max_width, actual_width);
		if (line_width < width)
			deltax = width - line_width;
		else
			deltax = 0.0;
		break;
	default:
		deltax = 0.0;
		break;
	}
	
	return deltax;
}

TextPointer
RichTextLayout::GetLocationFromXY (const Point &offset, double x, double y)
{
	RichTextLayoutLine *line;
	
	if (y < offset.y) {
		return TextPointer(); // XXX FIXME
	}
	
	if (!(line = GetLineFromY (offset, y))) {
		return TextPointer(); // XXX FIXME
	}
	
	return line->GetLocationFromX (offset, x);
}

void
RichTextLayout::Layout (RichTextBoxView *rtbview)
{
	if (!isnan (actual_width))
		return;
	
	actual_height = 0.0;
	actual_width = 0.0;
	is_wrapped = false;
	ClearLines ();
	
	d(printf ("RichTextLayout::Layout(): wrap mode = %s, wrapping to %f pixels\n", wrap_modes[wrapping], max_width));

	if (blocks) {
		for (int i = 0; i < blocks->GetCount(); i ++)
			LayoutBlock (rtbview, blocks->GetValueAt(i)->AsBlock());
	}
}

const char*
RichTextLayout::AddWordsToLine (RichTextLayoutLine *line, TextLayoutAttributes *attrs, const char *inptr, const char *inend, TextPointer start, TextPointer end, TextPointer *endpoint)
{
	RichTextLayoutInlineGlyphs *inline_ = new RichTextLayoutInlineGlyphs (line, attrs, start, end);
	LayoutWord word;
	TextFont *font;
	GlyphInfo *prev;
	bool linebreak;
	bool wrapped;
	LayoutWordCallback layout_word;
	size_t n_bytes, n_chars;
	bool empty = true;

	const char *text = inptr;

	linebreak = false;
	wrapped = false;
	prev = NULL;

	layout_word = layout_word_behavior[wrapping];

	word.break_ops = g_array_new (false, false, sizeof (WordBreakOpportunity));
		
	word.font = font = attrs->Font ();

	// layout until eoln or until we reach max_width
	while (inptr < inend) {
		// check for line-breaks
		if (IsLineBreak (inptr, inend - inptr, &n_bytes, &n_chars)) {
			if (line->inlines->len == 0) {
				line->descend = font->Descender ();
				line->size.height = font->Height ();
			}

			inptr += n_bytes;
			linebreak = true;
			break;
		}
				
		layout_word_init (&word, line->size.width, prev);
				
		// lay out the next word
		if (layout_word (&word, inptr, inend, max_width)) {
			// force a line wrap...
			is_wrapped = true;
			wrapped = true;
		}
				
		if (word.length > 0) {
			empty = false;

			inline_->size.width += word.advance;
					
			inptr += word.length;
			prev = word.prev;
		}
				
		if (wrapped)
			break;
			
		// now append any trailing lwsp
		layout_word_init (&word, line->size.width, prev);
				
		layout_lwsp (&word, inptr, inend);
				
		if (word.length > 0) {
			line->size.width += word.advance;
			inline_->size.width += word.advance;

			inptr += word.length;
			prev = word.prev;
		}
	}

	if (empty)
		delete inline_;
	else {
		char *s = g_strndup (text, inptr - text);

		line->AddInline (inline_);
	}

	g_array_free (word.break_ops, true);

	*endpoint = start.GetPositionAtOffset_np (inptr - text, LogicalDirectionForward);

	return inptr;
}
			
void
RichTextLayout::LayoutBlock (RichTextBoxView *rtbview, Block *b)
{
	if (!b->GetDocumentChildren() || b->GetDocumentChildren()->GetCount() == 0)
		return;

	// FIXME: take text trimming and flow direction into consideration...
	RichTextLayoutLine *line;

	TextPointer tp = b->GetContentStart_np();
	TextPointer b_end = b->GetContentEnd_np();

	line = new RichTextLayoutLine (this, tp, actual_height);
	
	do {
		while (!(tp.GetParent()->Is (Type::RUN) ||
			 tp.GetParent()->Is (Type::LINEBREAK) ||
			 tp.GetParent()->Is (Type::INLINEUICONTAINER))) {
			// the above types are the only ones that actually result
			// in "stuff appearing in the text view".  everything else
			// is just styling
			tp = tp.GetPositionAtOffset_np (1, LogicalDirectionForward);
			if (tp.CompareTo_np (b_end) >= 0)
				break;
			continue;
		}

		if (tp.GetParent()->Is (Type::RUN)) {
			Run *run = (Run*)tp.GetParent();
			// we know that we've got a run of similarly
			// styled text here, so we use the usual
			// layout routine for it
			TextLayoutAttributes *attrs;
			const char *text = run->GetText();

			if (text) {
				const char *inptr = text;
				const char *inend = text + strlen(text);
				attrs = new TextLayoutAttributes (run);

				const char *left = inptr;
				do {
					left = AddWordsToLine (line, attrs, left, inend, tp, run->GetContentEnd_np(), &tp);

					// if left != inend, we know we ran out of space and need to continue on a new line
					if (left != inend) {
						AddLine (line);

						line = new RichTextLayoutLine (this, tp, actual_height);
					}
				} while (left != inend);
			}
			
			tp = tp.GetPositionAtOffset_np (1, LogicalDirectionForward);
		}
		else if (tp.GetParent()->Is (Type::LINEBREAK)) {
			LineBreak *linebreak = (LineBreak*)tp.GetParent();
			TextLayoutAttributes attrs (linebreak);
			TextFont *font = attrs.Font();

			// since we don't create an actual
			// RichTextLayoutInline subclass for
			// LineBreaks, we need to deal with setting
			// the line height info for empty lines.
			// (normally this is handled in
			// RichTextLayoutLine::AddInline.)
			if (line->inlines->len == 0) {
				line->descend = font->Descender ();
				line->size.height = font->Height ();
			}

			AddLine (line);

			line = new RichTextLayoutLine (this, tp, actual_height);
					
			tp = linebreak->GetContentEnd_np ();
			tp = tp.GetPositionAtOffset_np (1, LogicalDirectionForward);
		}
		else if (tp.GetParent()->Is (Type::INLINEUICONTAINER)) {
			InlineUIContainer *container = (InlineUIContainer*)tp.GetParent();
			TextLayoutAttributes attrs (container);
			TextFont *font = attrs.Font();

			UIElement *ui = container->GetChild ();
			if (ui) {
				TextLayoutAttributes *attrs = new TextLayoutAttributes (container);
				RichTextLayoutInlineUIElement *inline_;

				if (ui->GetVisualParent() == NULL) {
					rtbview->GetChildren()->Add (Value (ui));
					rtbview->ElementAdded (ui);
				}

				inline_ = new RichTextLayoutInlineUIElement (line, attrs, tp, ui);

				MoonError error;
				ui->MeasureWithError (Size (INFINITY, INFINITY), &error);

				inline_->size = ui->GetDesiredSize ();

				double offset = 0;
				if (ui->Is (Type::TEXTBLOCK)) {
					offset = ((TextBlock*)ui)->GetBaselineOffset();
				}
				else if (ui->Is (Type::TEXTBOX)) {
					offset = ((TextBox*)ui)->GetBaselineOffset();
				}
				else if (ui->Is (Type::PASSWORDBOX)) {
					offset = ((PasswordBox*)ui)->GetBaselineOffset();
				}
				else if (ui->Is (Type::RICHTEXTBOX)) {
					offset = ((RichTextBox*)ui)->GetBaselineOffset();
				}

				// FIXME we need to deal with
				// ui->GetDesiredSize better here.  we
				// need to clip height by the
				// overridden line height, and we need
				// to wrap the line if the width of
				// the uielement causes us to, or else
				// limit the size of it if it's the
				// only thing on the line (i'm
				// guessing?)

			wrap_uielement:
				inline_->position = Point (line->size.width, offset == 0 ? line->y : line->y + (line->size.height - font->Descender() - offset) );

				if (inline_->size.width + line->size.width > max_width) {
					// the inline extends beyond
					// the end of the line.

					// if we're the only element
					// on the line, arrange with a
					// smaller size
					if (line->inlines->len == 0) {
						inline_->size = max_width;
					}
					else {
						AddLine (line);

						line = new RichTextLayoutLine (this, tp, actual_height);
					
						goto wrap_uielement;
					}
				}

				ui->ArrangeWithError (Rect (inline_->position, inline_->size), &error);

				line->AddInline (inline_);

				line->size.height = MAX (line->size.height, inline_->size.height - font->Descender ());
			}

			tp = container->GetContentEnd_np ();
			tp = tp.GetPositionAtOffset_np (1, LogicalDirectionForward);
		}

	} while (tp.CompareTo_np (b_end) < 0);

	// make sure we add at least one line, and make sure we add the final line.
	if (lines->len == 0 || line != lines->pdata[lines->len - 1])
		AddLine (line);
	
#if DEBUG
	if (debug_flags & RUNTIME_DEBUG_LAYOUT) {
		print_lines (lines);
		printf ("actualWidth = %f, actualHeight = %f\n\n", actual_width, actual_height);
	}
#endif
}

void
RichTextLayout::AddLine (RichTextLayoutLine *line)
{
	// update actual height extents
	actual_height += line->size.height;

	actual_width = MAX (actual_width, line->size.width);
				
	g_ptr_array_add (lines, line);
}


RichTextLayoutLine *
RichTextLayout::GetLineFromY (const Point &offset, double y, int *index)
{
	RichTextLayoutLine *line = NULL;
	double y0, y1;
	
	y0 = offset.y;

	for (guint i = 0; i < lines->len; i++) {
		line = (RichTextLayoutLine *) lines->pdata[i];

		// set y1 the top of the next line
		y1 = y0 + line->size.height;
		
		if (y < y1) {
			// we found the line that the point is located on
			if (index)
				*index = (int) i;
			
			return line;
		}
		
		y0 = y1;
	}
	
	return NULL;
}

void
RichTextLayout::Render (cairo_t *cr, const Point &origin, const Point &offset)
{
	RichTextLayoutLine *line;
	double x, y;
	
	y = offset.y;
	
	for (guint i = 0; i < lines->len; i++) {
		line = (RichTextLayoutLine *) lines->pdata[i];
		
		x = offset.x + HorizontalAlignment (line->size.width);
		line->Render (cr, origin, x, y);
		y += line->size.height;
	}
	
	if (moonlight_flags & RUNTIME_INIT_SHOW_TEXTBOXES) {
		Rect rect = GetRenderExtents ();
		
		rect.x += offset.x;
		rect.y += offset.y;
		
		cairo_set_source_rgba (cr, 0.0, 1.0, 0.0, 1.0);
		cairo_set_line_width (cr, 1);
		rect.Draw (cr);
		cairo_stroke (cr);
	}
}

Rect
RichTextLayout::GetCursor (const Point &offset, int index)
{
	return Rect();
#if 0
	const char *inptr, *inend, *pchar;
	double height, x0, y0, y1;
	GlyphInfo *prev, *glyph;
	TextLayoutLine *line;
	TextLayoutRun *run;
	TextFont *font;
	int cursor = 0;
	gunichar c;
	
	//printf ("TextLayout::GetCursor (%d)\n", index);
	
	x0 = offset.x;
	y0 = offset.y;
	height = 0.0;
	y1 = 0.0;
	
	for (guint i = 0; i < lines->len; i++) {
		line = (TextLayoutLine *) lines->pdata[i];
		inend = text + line->start + line->length;
		
		// adjust x0 for horizontal alignment
		x0 = offset.x + HorizontalAlignment (line->advance);
		
		// set y1 to the baseline (descend is a negative value)
		y1 = y0 + line->size.height + line->descend;
		height = line->size.height;
		
		//printf ("\tline: left=%.2f, top=%.2f, baseline=%.2f\n", x0, y0, y1);
		
		if (index >= cursor + line->count) {
			// maybe the cursor is on the next line...
			if ((i + 1) == lines->len) {
				// we are on the last line... get the previous unichar
				inptr = text + line->start;
				inend = inptr + line->length;
				
				if ((pchar = g_utf8_find_prev_char (text + line->start, inend)))
					c = utf8_getc (&pchar, inend - pchar);
				else
					c = (gunichar) -1;
				
				if (UnicharIsLineBreak (c)) {
					// cursor is on the next line by itself
					x0 = offset.x + HorizontalAlignment (0.0);
					y0 += line->size.height;
				} else {
					// cursor at the end of the last line
					x0 += line->advance;
				}
				
				break;
			}
			
			cursor += line->count;
			y0 += line->size.height;
			continue;
		}
		
		// cursor is on this line...
		for (guint j = 0; j < line->runs->len; j++) {
			run = (TextLayoutRun *) line->runs->pdata[j];
			inend = text + run->start + run->length;
			
			if (index >= cursor + run->count) {
				// maybe the cursor is in the next run...
				cursor += run->count;
				x0 += run->advance;
				continue;
			}
			
			// cursor is in this run...
			font = run->attrs->Font ();
			inptr = text + run->start;
			prev = NULL;
			
			while (cursor < index) {
				if ((c = utf8_getc (&inptr, inend - inptr)) == (gunichar) -1)
					continue;
				
				cursor++;
				
				// we treat tabs as a single space
				if (c == '\t')
					c = ' ';
				
				if (!(glyph = font->GetGlyphInfo (c)))
					continue;
				
				if ((prev != NULL) && APPLY_KERNING (c))
					x0 += font->Kerning (prev, glyph);
				else if (glyph->metrics.horiBearingX < 0)
					x0 += glyph->metrics.horiBearingX;
				
				x0 += glyph->metrics.horiAdvance;
				prev = glyph;
			}
			
			break;
		}
		
		break;
	}
	
	return Rect (x0, y0, 1.0, height);
#endif
}

//
// RichTextLayoutLine
//

RichTextLayoutLine::RichTextLayoutLine (RichTextLayout *_layout, const TextPointer& _start, double _y)
{
	inlines = g_ptr_array_new ();
	layout = _layout;
	start = _start;
	y = _y;
}

RichTextLayoutLine::~RichTextLayoutLine ()
{
	for (guint i = 0; i < inlines->len; i++)
		delete (RichTextLayoutInline *) inlines->pdata[i];
	
	g_ptr_array_free (inlines, true);
}

void
RichTextLayoutLine::Render (cairo_t *cr, const Point &origin, double left, double top)
{
	double x0, y0;
	
	// set y0 to the line's baseline (descend is a negative value)
	y0 = top + size.height + descend;
	x0 = left;
	
	for (guint i = 0; i < inlines->len; i++) {
		RichTextLayoutInline *inl = (RichTextLayoutInline *) inlines->pdata[i];
		inl->Render (cr, origin, x0, y0, (i + 1) < inlines->len);
		x0 += inl->size.width;
	}
}

TextPointer
RichTextLayoutLine::GetLocationFromX (const Point &offset, double x)
{
	RichTextLayoutInline *inline_;
	double x0;
	double m;
	guint i;

	// adjust x0 for horizontal alignment
	x0 = offset.x + layout->HorizontalAlignment (size.width);

	for (i = 0; i < inlines->len; i ++) {
		inline_= (RichTextLayoutInline*)inlines->pdata[i];
		if (x < x0 + inline_->size.width) {
			// x is in somewhere inside this run
			break;
		}

		// x is beyond this inline
		x0 += inline_->size.width;
		inline_ = NULL;
	}

	int cursor = 0;

	if (inline_ != NULL) {
		const char *text = inline_->GetText();

		// empty inlines just put the location at the end.
		if (!text || !*text)
			return inline_->end;

		const char *inptr = text + inline_->start.GetLocation();
		int end_loc = inline_->end.GetLocation() == (guint32)-1 ? strlen (text) : inline_->end.GetLocation();
		const char *inend = text + end_loc;
		GlyphInfo *prev, *glyph;
		TextFont *font;

		font = inline_->attrs->Font ();
		
		while (inptr < inend) {
			const char *ch = inptr;
			gunichar c;

			if ((c = utf8_getc (&inptr, inend - inptr)) == (gunichar) -1)
				continue;
			
			if (UnicharIsLineBreak (c)) {
				inptr = ch;
				break;
			}
			
			cursor++;
			
			// we treat tabs as a single space
			if (c == '\t')
				c = ' ';
			
			if (!(glyph = font->GetGlyphInfo (c)))
				continue;
			
			if ((prev != NULL) && APPLY_KERNING (c))
				x0 += font->Kerning (prev, glyph);
			else if (glyph->metrics.horiBearingX < 0)
				x0 += glyph->metrics.horiBearingX;
			
			// calculate midpoint of the character
			m = glyph->metrics.horiAdvance / 2.0;
			
			// if x is <= the midpoint, then the cursor is
			// considered to be at the start of this character.

			// FIXME should this alter the logical direction instead of the "cursor"?
			if (x <= x0 + m) {
				inptr = ch;
				cursor--;
				break;
			}
			
			x0 += glyph->metrics.horiAdvance;
			prev = glyph;
		}
	} else if (i > 0) {
		// FIXME:  this should just be
		// return end;

		// x is beyond the end of the last inline
		inline_= (RichTextLayoutInline*)inlines->pdata[i-1];

		const char *text = inline_->GetText();

		// empty inlines just put the location at the end.
		if (!text || !*text)
			return inline_->end;

		const char *inptr = text + inline_->start.GetLocation();
		int end_loc = inline_->end.GetLocation() == (guint32)-1 ? strlen (text) : inline_->end.GetLocation();
		const char *inend = text + end_loc;
		const char *ch = inptr;
		gunichar c;

		cursor = inend - inptr;

		if ((ch = g_utf8_find_prev_char (inptr, inend)))
			c = utf8_getc (&ch, inend - ch);
		else
			c = (gunichar) -1;
		
		if (c == '\n') {
			cursor--;
			inend--;
			
			if (inend > inptr && inend[-1] == '\r') {
				cursor--;
				inend--;
			}
		} else if (UnicharIsLineBreak (c)) {
			cursor--;
			inend--;
		}
	}

	return inline_->start.GetPositionAtOffset_np (cursor, LogicalDirectionForward);
}

void
RichTextLayoutLine::AddInline (RichTextLayoutInline *inline_)
{
	size.width += inline_->size.width;
	size.height = MAX (size.height, inline_->size.height);

	descend = MIN (descend, inline_->descend);

	g_ptr_array_add (inlines, inline_);
}

void
RichTextLayoutInlineGlyphs::GenerateCache ()
{
	Run *run = (Run*)start.GetParent();
	const char *inptr = run->GetText() + start.GetLocation();
	int end_loc = end.GetLocation() == (guint32)-1 ? strlen (inptr) : end.GetLocation();
	const char *inend = inptr + (end_loc - start.GetLocation());
	GlyphInfo *prev = NULL /* FIXME *pglyph */;
	TextFont *font = attrs->Font ();
	double x0, x1, y0;
	GlyphInfo *glyph;
	int path_size = 0;
	gunichar c;
	
	// set y0 to the baseline
	y0 = font->Ascender ();
	x0 = 0.0;
	x1 = 0.0;
	
	// count how many path data items we'll need to allocate
	while (inptr < inend) {
		if ((c = utf8_getc (&inptr, inend - inptr)) == (gunichar) -1)
			continue;
		
		if (UnicharIsLineBreak (c))
			break;
		
		// treat tab as a single space
		if (c == '\t')
			c = ' ';
		
		if (!(glyph = font->GetGlyphInfo (c)))
			continue;
		
		if (glyph->path)
			path_size += glyph->path->cairo.num_data + 1;
	}
	
	if (path_size > 0) {
		// generate the cached path for the cluster
		path = moon_path_new (path_size);
		inptr = run->GetText() + start.GetLocation();
		
		while (inptr < inend) {
			if ((c = utf8_getc (&inptr, inend - inptr)) == (gunichar) -1)
				continue;
			
			if (UnicharIsLineBreak (c))
				break;
			
			// treat tab as a single space
			if (c == '\t')
				c = ' ';
			
			if (!(glyph = font->GetGlyphInfo (c)))
				continue;
			
			if (prev != NULL) {
				if (APPLY_KERNING (c))
					x0 += font->Kerning (prev, glyph);
				else if (glyph->metrics.horiBearingX < 0)
					x0 += glyph->metrics.horiBearingX;
			}
			
			font->AppendPath (path, glyph, x0, y0);
			x0 += glyph->metrics.horiAdvance;
			prev = glyph;
			
			if (!g_unichar_isspace (c))
				x1 = x0;
		}
		
		moon_close_path (path);
	}
	
	uadvance = x1;	
	size.width = x0;
	
	/* FIXME *pglyph = prev; */
}

void
RichTextLayoutInlineGlyphs::Render (cairo_t *cr, const Point &origin, double x, double y, bool is_last_run)
{
	//	Run *run = (Run*)start.GetParent();
	//	const char *text = run->GetText();
	TextFont *font = attrs->Font ();
	//	const char *inend, *prev;
	//	GlyphInfo *glyph;
	Brush *brush;
	//	gunichar c;
	double y0;
	Rect area;
	
	if (size.width == 0.0)
		return;

	if (!path)
		GenerateCache ();

	cairo_save (cr);
	// y is the baseline, set the origin to the top-left
	cairo_translate (cr, x, y - font->Ascender ());
	
	// set y0 to the baseline relative to the translation matrix
	y0 = font->Ascender ();

#if notyet
	if (selected && (brush = attrs->Background (true))) {
		area = Rect (origin.x, origin.y, advance, font->Height ());
		
		// extend the selection background by the width of a SPACE if it includes CRLF
		inend = text + start + length;
		if ((prev = g_utf8_find_prev_char (text + start, inend)))
			c = utf8_getc (&prev, inend - prev);
		else
			c = (gunichar) -1;
		
		if (UnicharIsLineBreak (c)) {
			if ((glyph = font->GetGlyphInfo (' ')))
				area.width += glyph->metrics.horiAdvance;
		}
		
		// render the selection background
		brush->SetupBrush (cr, area);
		cairo_new_path (cr);
		cairo_rectangle (cr, area.x, area.y, area.width, area.height);
		brush->Fill (cr);
	}
#endif
	
	// setup the foreground brush
	if (!(brush = attrs->Foreground (false /*FIXME: selected*/))) {
		cairo_restore (cr);
		return;
	}
	
	area = Rect (origin.x, origin.y, size.width, font->Height ());
	brush->SetupBrush (cr, area);
	cairo_new_path (cr);
	
	if (path && path->cairo.data)
		cairo_append_path (cr, &path->cairo);
	
	brush->Fill (cr);
	
	if (attrs->IsUnderlined ()) {
		double thickness = font->UnderlineThickness ();
		double pos = y0 + font->UnderlinePosition ();
		
		cairo_set_line_width (cr, thickness);
		
		cairo_new_path (cr);
#if notyet
		Rect underline = Rect (0.0, pos - thickness * 0.5, uline_full ? advance : uadvance, thickness);
#else
		Rect underline = Rect (0.0, pos - thickness * 0.5, size.width, thickness);
#endif
		underline.Draw (cr);
		
		brush->Fill (cr);
	}

	cairo_restore (cr);
}

};
