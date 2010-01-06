/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * layout.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <ctype.h>
#include <math.h>

#include "moon-path.h"
#include "layout.h"
#include "debug.h"


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


/*
 * Silverlight does not apply any kerning on a DOT, so we exclude them
 * 	U+002E FULL STOP
 * 	U+06D4 ARABIC FULL STOP
 *	U+3002 IDEOGRAPHIC FULL STOP
 * Note: this is different than using the "sliding dot" algorithm from
 * http://www.freetype.org/freetype2/docs/glyphs/glyphs-4.html
 */
#define APPLY_KERNING(uc)	((uc != 0x002E) && (uc != 0x06D4) && (uc != 3002))


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


//
// TextLayoutGlyphCluster
//

TextLayoutGlyphCluster::TextLayoutGlyphCluster (int _start, int _length)
{
	length = _length;
	start = _start;
	selected = false;
	advance = 0.0;
	path = NULL;
}

TextLayoutGlyphCluster::~TextLayoutGlyphCluster ()
{
	if (path)
		moon_path_destroy (path);
}


//
// TextLayoutRun
//

TextLayoutRun::TextLayoutRun (TextLayoutLine *_line, TextLayoutAttributes *_attrs, int _start)
{
	clusters = g_ptr_array_new ();
	attrs = _attrs;
	start = _start;
	line = _line;
	advance = 0.0;
	length = 0;
	count = 0;
}

TextLayoutRun::~TextLayoutRun ()
{
	for (guint i = 0; i < clusters->len; i++)
		delete (TextLayoutGlyphCluster *) clusters->pdata[i];
	
	g_ptr_array_free (clusters, true);
}

void
TextLayoutRun::ClearCache ()
{
	for (guint i = 0; i < clusters->len; i++)
		delete (TextLayoutGlyphCluster *) clusters->pdata[i];
	
	g_ptr_array_set_size (clusters, 0);
}


//
// TextLayoutLine
//

TextLayoutLine::TextLayoutLine (TextLayout *_layout, int _start, int _offset)
{
	runs = g_ptr_array_new ();
	layout = _layout;
	offset = _offset;
	start = _start;
	advance = 0.0;
	descend = 0.0;
	height = 0.0;
	width = 0.0;
	length = 0;
	count = 0;
}

TextLayoutLine::~TextLayoutLine ()
{
	for (guint i = 0; i < runs->len; i++)
		delete (TextLayoutRun *) runs->pdata[i];
	
	g_ptr_array_free (runs, true);
}



//
// TextLayout
//

TextLayout::TextLayout ()
{
	// Note: TextBlock and TextBox assume their default values match these
	strategy = LineStackingStrategyMaxHeight;
	alignment = TextAlignmentLeft;
	wrapping = TextWrappingNoWrap;
	selection_length = 0;
	selection_start = 0;
	avail_width = INFINITY;
	max_height = INFINITY;
	max_width = INFINITY;
	base_descent = 0.0;
	base_height = 0.0;
	actual_height = NAN;
	actual_width = NAN;
	line_height = NAN;
	attributes = NULL;
	lines = g_ptr_array_new ();
	is_wrapped = true;
	text = NULL;
	length = 0;
	count = 0;
}

TextLayout::~TextLayout ()
{
	if (attributes) {
		attributes->Clear (true);
		delete attributes;
	}
	
	ClearLines ();
	g_ptr_array_free (lines, true);
	
	g_free (text);
}

void
TextLayout::ClearLines ()
{
	for (guint i = 0; i < lines->len; i++)
		delete (TextLayoutLine *) lines->pdata[i];
	
	g_ptr_array_set_size (lines, 0);
}

void
TextLayout::ResetState ()
{
	actual_height = NAN;
	actual_width = NAN;
}

void
TextLayout::SetBaseFont (const TextFont *font)
{
	if (font) {
		base_descent = font->Descender ();
		base_height = font->Height ();
	} else {
		base_descent = 0.0;
		base_height = 0.0;
	}
}

bool
TextLayout::SetLineStackingStrategy (LineStackingStrategy mode)
{
	if (strategy == mode)
		return false;
	
	strategy = mode;
	
	ResetState ();
	
	return true;
}

bool
TextLayout::SetTextAlignment (TextAlignment align)
{
	if (alignment == align)
		return false;
	
	alignment = align;
	
	return false;
}

bool
TextLayout::SetTextWrapping (TextWrapping mode)
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
	
	ResetState ();
	
	return true;
}

bool
TextLayout::SetLineHeight (double height)
{
	if (line_height == height)
		return false;
	
	line_height = height;
	
	ResetState ();
	
	return true;
}

bool
TextLayout::SetMaxHeight (double height)
{
	if (max_height == height)
		return false;
	
	max_height = height;
	
	ResetState ();
	
	return true;
}

bool
TextLayout::SetMaxWidth (double width)
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
	
	ResetState ();
	
	return true;
}

bool
TextLayout::SetTextAttributes (List *attrs)
{
	if (attributes) {
		attributes->Clear (true);
		delete attributes;
	}
	
	attributes = attrs;
	
	ResetState ();
	
	return true;
}

bool
TextLayout::SetText (const char *str, int len)
{
	g_free (text);
	
	if (str) {
		length = len == -1 ? strlen (str) : len;
		text = (char *) g_malloc (length + 1);
		memcpy (text, str, length);
		text[length] = '\0';
	} else {
		text = NULL;
		length = 0;
	}
	
	count = -1;
	
	ResetState ();
	
	return true;
}

void
TextLayout::ClearCache ()
{
	TextLayoutLine *line;
	TextLayoutRun *run;
	
	for (guint i = 0; i < lines->len; i++) {
		line = (TextLayoutLine *) lines->pdata[i];
		
		for (guint j = 0; j < line->runs->len; j++) {
			run = (TextLayoutRun *) line->runs->pdata[j];
			run->ClearCache ();
		}
	}
}

struct TextRegion {
	int start, length;
	bool select;
};

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
		line = (TextLayoutLine *) lines->pdata[i];
		
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

void
TextLayout::Select (int start, int length, bool byte_offsets)
{
	int new_selection_length;
	int new_selection_start;
	int new_selection_end;
	int selection_end;
	TextRegion pre, post;
	const char *inptr;
	const char *inend;
	
	if (!text) {
		selection_length = 0;
		selection_start = 0;
		return;
	}
	
	if (!byte_offsets) {
		inptr = g_utf8_offset_to_pointer (text, start);
		new_selection_start = inptr - text;
		
		inend = g_utf8_offset_to_pointer (inptr, length);
		new_selection_length = inend - inptr;
	} else {
		new_selection_length = length;
		new_selection_start = start;
	}
	
	if (selection_start == new_selection_start &&
	    selection_length == new_selection_length) {
		// no change in selection...
		return;
	}
	
#if 1
	// compute the region between the 2 starts
	pre.length = abs (new_selection_start - selection_start);
	pre.start = MIN (selection_start, new_selection_start);
	pre.select = (new_selection_start < selection_start) && (new_selection_length > 0);
	
	// compute the region between the 2 ends
	new_selection_end = new_selection_start + new_selection_length;
	selection_end = selection_start + selection_length;
	post.length = abs (new_selection_end - selection_end);
	post.start = MIN (selection_end, new_selection_end);
	post.select = (new_selection_end > selection_end) && (new_selection_length > 0);
	
	UpdateSelection (lines, &pre, &post);
	
	selection_length = new_selection_length;
	selection_start = new_selection_start;
#else
	if (selection_length || new_selection_length)
		ClearCache ();
	
	selection_length = new_selection_length;
	selection_start = new_selection_start;
#endif
}

void
TextLayout::GetActualExtents (double *width, double *height)
{
	*height = actual_height;
	*width = actual_width;
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

enum WordType {
	WORD_TYPE_UNKNOWN,
	WORD_TYPE_ALPHABETIC,
	WORD_TYPE_IDEOGRAPHIC,
	WORD_TYPE_INSEPARABLE,
	WORD_TYPE_NUMERIC,
	WORD_TYPE_HANGUL,
};

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
	
	d(printf ("layout_lwsp():\n"));
	
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
	
	d(printf ("layout_word_wrap():\n"));
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

#if DEBUG
static const char *wrap_modes[3] = {
	"WrapWithOverflow",
	"NoWrap",
	"Wrap"
};

static void
print_lines (GPtrArray *lines)
{
	TextLayoutLine *line;
	TextLayoutRun *run;
	const char *text;
	double y = 0.0;
	
	for (guint i = 0; i < lines->len; i++) {
		line = (TextLayoutLine *) lines->pdata[i];
		
		printf ("Line (top=%f, height=%f, advance=%f, offset=%d, count=%d):\n", y, line->height, line->advance, line->offset, line->count);
		for (guint j = 0; j < line->runs->len; j++) {
			run = (TextLayoutRun *) line->runs->pdata[j];
			
			text = line->layout->GetText () + run->start;
			
			printf ("\tRun (advance=%f, start=%d, length=%d): \"", run->advance, run->start, run->length);
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
			printf ("\"\n");
		}
		
		y += line->height;
	}
}
#endif

double
TextLayout::LineHeightOverride ()
{
	if (isnan (line_height))
		return base_height;
	else
		return line_height;
}

double
TextLayout::DescendOverride ()
{
	if (isnan (line_height))
		return base_descent;
	
	if (base_height == 0.0)
		return 0.0;
	
	return line_height * (base_descent / base_height);
}

static LayoutWordCallback layout_word_behavior[] = {
	layout_word_wrap,
	layout_word_nowrap,
	layout_word_wrap
};

static bool
validate_attrs (List *attributes)
{
	TextLayoutAttributes *attrs;
	
	// if no attributes or first attribute doesn't start at 0, we can't layout any text
	if (!(attrs = (TextLayoutAttributes *) attributes->First ()) || attrs->start != 0)
		return false;
	
	while (attrs != NULL) {
		if (!attrs->Font ()) {
			// we can't layout any text if any of the attributes
			// weren't able to load their font
			return false;
		}
		
		attrs = (TextLayoutAttributes *) attrs->next;
	}
	
	return true;
}

void
TextLayout::Layout ()
{
	TextLayoutAttributes *attrs, *nattrs;
	LayoutWordCallback layout_word;
	const char *inptr, *inend;
	size_t n_bytes, n_chars;
	TextLayoutLine *line;
	TextLayoutRun *run;
	GlyphInfo *prev;
	LayoutWord word;
	TextFont *font;
	bool linebreak;
	int offset = 0;
	bool wrapped;
	
	if (!isnan (actual_width))
		return;
	
	actual_height = 0.0;
	actual_width = 0.0;
	is_wrapped = false;
	ClearLines ();
	count = 0;
	
	if (!text || !validate_attrs (attributes))
		return;
	
	d(printf ("TextLayout::Layout(): wrap mode = %s, wrapping to %f pixels\n", wrap_modes[wrapping], max_width));
	
	if (wrapping == TextWrappingWrap)
		word.break_ops = g_array_new (false, false, sizeof (WordBreakOpportunity));
	else
		word.break_ops = NULL;
	
	layout_word = layout_word_behavior[wrapping];
	
	attrs = (TextLayoutAttributes *) attributes->First ();
	line = new TextLayoutLine (this, 0, 0);
	if (OverrideLineHeight ()) {
		line->descend = DescendOverride ();
		line->height = LineHeightOverride ();
	}
	
	g_ptr_array_add (lines, line);
	inptr = text;
	
	do {
		nattrs = (TextLayoutAttributes *) attrs->next;
		inend = text + (nattrs ? nattrs->start : length);
		run = new TextLayoutRun (line, attrs, inptr - text);
		g_ptr_array_add (line->runs, run);
		
		word.font = font = attrs->Font ();
		
		//if (!OverrideLineHeight ()) {
		//	line->descend = MIN (line->descend, font->Descender ());
		//	line->height = MAX (line->height, font->Height ());
		//}
		
		if (*inptr == '\0') {
			if (!OverrideLineHeight ()) {
				line->descend = MIN (line->descend, font->Descender ());
				line->height = MAX (line->height, font->Height ());
			}
			
			actual_height += line->height;
			break;
		}
		
		// layout until attrs change
		while (inptr < inend) {
			linebreak = false;
			wrapped = false;
			prev = NULL;
			
			// layout until eoln or until we reach max_width
			while (inptr < inend) {
				// check for line-breaks
				if (IsLineBreak (inptr, inend - inptr, &n_bytes, &n_chars)) {
					if (line->length == 0 && !OverrideLineHeight ()) {
						line->descend = font->Descender ();
						line->height = font->Height ();
					}
					
					line->length += n_bytes;
					run->length += n_bytes;
					line->count += n_chars;
					run->count += n_chars;
					offset += n_chars;
					inptr += n_bytes;
					linebreak = true;
					break;
				}
				
				layout_word_init (&word, line->advance, prev);
				
				// lay out the next word
				if (layout_word (&word, inptr, inend, max_width)) {
					// force a line wrap...
					is_wrapped = true;
					wrapped = true;
				}
				
				if (word.length > 0) {
					// append the word to the run/line
					if (!OverrideLineHeight ()) {
						line->descend = MIN (line->descend, font->Descender ());
						line->height = MAX (line->height, font->Height ());
					}
					
					line->advance += word.advance;
					run->advance += word.advance;
					line->width = line->advance;
					line->length += word.length;
					run->length += word.length;
					line->count += word.count;
					run->count += word.count;
					
					offset += word.count;
					inptr += word.length;
					prev = word.prev;
				}
				
				if (wrapped)
					break;
				
				// now append any trailing lwsp
				layout_word_init (&word, line->advance, prev);
				
				layout_lwsp (&word, inptr, inend);
				
				if (word.length > 0) {
					if (!OverrideLineHeight ()) {
						line->descend = MIN (line->descend, font->Descender ());
						line->height = MAX (line->height, font->Height ());
					}
					
					line->advance += word.advance;
					run->advance += word.advance;
					line->length += word.length;
					run->length += word.length;
					line->count += word.count;
					run->count += word.count;
					
					offset += word.count;
					inptr += word.length;
					prev = word.prev;
				}
			}
			
			if (linebreak || wrapped || *inptr == '\0') {
				// update actual width extents
				if (*inptr == '\0') {
					// ActualWidth extents only include trailing lwsp on the last line
					actual_width = MAX (actual_width, line->advance);
				} else {
					// not the last line, so don't include trailing lwsp
					actual_width = MAX (actual_width, line->width);
				}
				
				// update actual height extents
				actual_height += line->height;
				
				if (linebreak || wrapped) {
					// more text to layout... which means we'll need a new line
					line = new TextLayoutLine (this, inptr - text, offset);
					
					if (!OverrideLineHeight ()) {
						if (*inptr == '\0') {
							line->descend = font->Descender ();
							line->height = font->Height ();
						}
					} else {
						line->descend = DescendOverride ();
						line->height = LineHeightOverride ();
					}
					
					if (linebreak && *inptr == '\0')
						actual_height += line->height;
					
					g_ptr_array_add (lines, line);
					prev = NULL;
				}
				
				if (inptr < inend) {
					// more text to layout with the current attrs...
					run = new TextLayoutRun (line, attrs, inptr - text);
					g_ptr_array_add (line->runs, run);
				}
			}
		}
		
		attrs = nattrs;
	} while (*inptr != '\0');
	
	if (word.break_ops != NULL)
		g_array_free (word.break_ops, true);
	
	count = offset;
	
#if DEBUG
	if (debug_flags & RUNTIME_DEBUG_LAYOUT) {
		print_lines (lines);
		printf ("actualWidth = %f, actualHeight = %f\n\n", actual_width, actual_height);
	}
#endif
}

static inline TextLayoutGlyphCluster *
GenerateGlyphCluster (TextFont *font, GlyphInfo **pglyph, const char *text, int start, int length)
{
	TextLayoutGlyphCluster *cluster = new TextLayoutGlyphCluster (start, length);
	const char *inend = text + start + length;
	const char *inptr = text + start;
	GlyphInfo *prev = *pglyph;
	double x0, x1, y0;
	GlyphInfo *glyph;
	int size = 0;
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
			size += glyph->path->cairo.num_data + 1;
	}
	
	if (size > 0) {
		// generate the cached path for the cluster
		cluster->path = moon_path_new (size);
		inptr = text + start;
		
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
			
			font->AppendPath (cluster->path, glyph, x0, y0);
			x0 += glyph->metrics.horiAdvance;
			prev = glyph;
			
			if (!g_unichar_isspace (c))
				x1 = x0;
		}
		
		moon_close_path (cluster->path);
	}
	
	cluster->uadvance = x1;	
	cluster->advance = x0;
	
	*pglyph = prev;
	
	return cluster;
}

void
TextLayoutRun::GenerateCache ()
{
	int selection_length = line->layout->GetSelectionLength ();
	int selection_start = line->layout->GetSelectionStart ();
	const char *text = line->layout->GetText ();
	const char *inend = text + start + length;
	const char *inptr = text + start;
	TextFont *font = attrs->Font ();
	TextLayoutGlyphCluster *cluster;
	const char *selection_end;
	GlyphInfo *prev = NULL;
	int len;
	
	// cache the glyph cluster leading up to the selection
	if (selection_length == 0 || start < selection_start) {
		if (selection_length > 0)
			len = MIN (selection_start - start, length);
		else
			len = length;
		
		cluster = GenerateGlyphCluster (font, &prev, text, start, len);
		g_ptr_array_add (clusters, cluster);
		inptr += len;
	}
	
	// cache the selected glyph cluster
	selection_end = text + selection_start + selection_length;
	if (inptr < inend && inptr < selection_end) {
		len = MIN (inend, selection_end) - inptr;
		cluster = GenerateGlyphCluster (font, &prev, text, inptr - text, len);
		g_ptr_array_add (clusters, cluster);
		cluster->selected = true;
		inptr += len;
	}
	
	// cache the glyph cluster following the selection
	if (inptr < inend) {
		cluster = GenerateGlyphCluster (font, &prev, text, inptr - text, inend - inptr);
		g_ptr_array_add (clusters, cluster);
		inptr = inend;
	}
}

void
TextLayoutGlyphCluster::Render (cairo_t *cr, const Point &origin, TextLayoutAttributes *attrs, const char *text, double x, double y, bool uline_full)
{
	TextFont *font = attrs->Font ();
	const char *inend, *prev;
	GlyphInfo *glyph;
	Brush *brush;
	gunichar c;
	double y0;
	Rect area;
	
	if (length == 0 || advance == 0.0)
		return;
	
	// y is the baseline, set the origin to the top-left
	cairo_translate (cr, x, y - font->Ascender ());
	
	// set y0 to the baseline relative to the translation matrix
	y0 = font->Ascender ();
	
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
	
	// setup the foreground brush
	if (!(brush = attrs->Foreground (selected)))
		return;
	
	area = Rect (origin.x, origin.y, advance, font->Height ());
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
		Rect underline = Rect (0.0, pos - thickness * 0.5, uline_full ? advance : uadvance, thickness);
		underline.Draw (cr);
		
		brush->Fill (cr);
	}
}

void
TextLayoutRun::Render (cairo_t *cr, const Point &origin, double x, double y, bool is_last_run)
{
	const char *text = line->layout->GetText ();
	TextLayoutGlyphCluster *cluster;
	double x0 = x;
	
	if (clusters->len == 0)
		GenerateCache ();
	
	for (guint i = 0; i < clusters->len; i++) {
		cluster = (TextLayoutGlyphCluster *) clusters->pdata[i];
		
		cairo_save (cr);
		cluster->Render (cr, origin, attrs, text, x0, y, is_last_run && ((i + 1) < clusters->len));
		cairo_restore (cr);
		
		x0 += cluster->advance;
	}
}

void
TextLayoutLine::Render (cairo_t *cr, const Point &origin, double left, double top)
{
	TextLayoutRun *run;
	double x0, y0;
	
	// set y0 to the line's baseline (descend is a negative value)
	y0 = top + height + descend;
	x0 = left;
	
	for (guint i = 0; i < runs->len; i++) {
		run = (TextLayoutRun *) runs->pdata[i];
		run->Render (cr, origin, x0, y0, (i + 1) < runs->len);
		x0 += run->advance;
	}
}

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

double
TextLayout::HorizontalAlignment (double line_width)
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

void
TextLayout::Render (cairo_t *cr, const Point &origin, const Point &offset)
{
	TextLayoutLine *line;
	double x, y;
	
	y = offset.y;
	
	Layout ();
	
	for (guint i = 0; i < lines->len; i++) {
		line = (TextLayoutLine *) lines->pdata[i];
		
		x = offset.x + HorizontalAlignment (line->advance);
		line->Render (cr, origin, x, y);
		y += line->height;
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
TextLayout::GetRenderExtents ()
{
	Layout ();
	
	return Rect (HorizontalAlignment (actual_width), 0.0, actual_width, actual_height);
}

#ifdef USE_BINARY_SEARCH
/**
 * MID:
 * @lo: the low bound
 * @hi: the high bound
 *
 * Finds the midpoint between positive integer values, @lo and @hi.
 *
 * Notes: Typically expressed as '(@lo + @hi) / 2', this is incorrect
 * when @lo and @hi are sufficiently large enough that combining them
 * would overflow their integer type. To work around this, we use the
 * formula, '@lo + ((@hi - @lo) / 2)', thus preventing this problem
 * from occuring.
 *
 * Returns the midpoint between @lo and @hi (rounded down).
 **/
#define MID(lo, hi) (lo + ((hi - lo) >> 1))

TextLayoutLine *
TextLayout::GetLineFromY (const Point &offset, double y, int *index)
{
	register guint lo, hi;
	TextLayoutLine *line;
	double y0;
	guint m;
	
	if (lines->len == 0)
		return NULL;
	
	lo = 0, hi = lines->len;
	y0 = y - offset.y;
	
	do {
		m = MID (lo, hi);
		
		line = (TextLayoutLine *) lines->pdata[m];
		
		if (m > 0 && y0 < line->top) {
			// y is on some line above us
			hi = m;
		} else if (y0 > line->top + line->height) {
			// y is on some line below us
			lo = m + 1;
			m = lo;
		} else {
			// y is on this line
			break;
		}
		
		line = NULL;
	} while (lo < hi);
	
	if (line && index)
		*index = m;
	
	return line;
}
#else /* linear search */
TextLayoutLine *
TextLayout::GetLineFromY (const Point &offset, double y, int *index)
{
	TextLayoutLine *line = NULL;
	double y0, y1;
	
	//printf ("TextLayout::GetLineFromY (%.2g)\n", y);
	
	y0 = offset.y;
	
	for (guint i = 0; i < lines->len; i++) {
		line = (TextLayoutLine *) lines->pdata[i];
		
		// set y1 the top of the next line
		y1 = y0 + line->height;
		
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
#endif

TextLayoutLine *
TextLayout::GetLineFromIndex (int index)
{
	if (index >= (int) lines->len || index < 0)
		return NULL;
	
	return (TextLayoutLine *) lines->pdata[index];
}

int
TextLayoutLine::GetCursorFromX (const Point &offset, double x)
{
	const char *text, *inend, *ch, *inptr;
	TextLayoutRun *run = NULL;
	GlyphInfo *prev, *glyph;
	TextFont *font;
	int cursor;
	gunichar c;
	double x0;
	double m;
	guint i;
	
	// adjust x0 for horizontal alignment
	x0 = offset.x + layout->HorizontalAlignment (advance);
	
	text = layout->GetText ();
	inptr = text + start;
	cursor = this->offset;
	prev = NULL;
	
	for (i = 0; i < runs->len; i++) {
		run = (TextLayoutRun *) runs->pdata[i];
		
		if (x < x0 + run->advance) {
			// x is in somewhere inside this run
			break;
		}
		
		// x is beyond this run
		cursor += run->count;
		inptr += run->length;
		x0 += run->advance;
		run = NULL;
	}
	
	if (run != NULL) {
		inptr = text + run->start;
		inend = inptr + run->length;
		font = run->attrs->Font ();
		
		while (inptr < inend) {
			ch = inptr;
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
			if (x <= x0 + m) {
				inptr = ch;
				cursor--;
				break;
			}
			
			x0 += glyph->metrics.horiAdvance;
			prev = glyph;
		}
	} else if (i > 0) {
		// x is beyond the end of the last run
		run = (TextLayoutRun *) runs->pdata[i - 1];
		inend = text + run->start + run->length;
		inptr = text + run->start;
		
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
	
	return cursor;
}

int
TextLayout::GetCursorFromXY (const Point &offset, double x, double y)
{
	TextLayoutLine *line;
	
	//printf ("TextLayout::GetCursorFromXY (%.2g, %.2g)\n", x, y);
	
	if (y < offset.y)
		return 0;
	
	if (!(line = GetLineFromY (offset, y)))
		return count;
	
	return line->GetCursorFromX (offset, x);
}

Rect
TextLayout::GetCursor (const Point &offset, int index)
{
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
		y1 = y0 + line->height + line->descend;
		height = line->height;
		
		//printf ("\tline: left=%.2f, top=%.2f, baseline=%.2f, start index=%d\n", x0, y0, y1, line->offset);
		
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
					y0 += line->height;
				} else {
					// cursor at the end of the last line
					x0 += line->advance;
				}
				
				break;
			}
			
			cursor += line->count;
			y0 += line->height;
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
}
