/*
 * layout.cpp: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "moon-path.h"
#include "layout.h"


TextRun::TextRun (const char *utf8, int len, TextDecorations deco, TextFontDescription *font, Brush **fg)
{
	register gunichar *s, *d;
	
	d = this->text = g_utf8_to_ucs4_fast (utf8, len, NULL);
	
	// drop all non-printable characters and convert all ascii
	// lwsp into a SPACE, conserving only \n's
	for (s = this->text; *s; s++) {
		if (g_unichar_isspace (*s)) {
			if (*s == '\n')
				*d++ = *s;
			else if (*s < 128)
				*d++ = ' ';
			else
				*d++ = *s;
		} else if (g_unichar_isprint (*s)) {
			*d++ = *s;
		}
	}
	
	*d = 0;
	
	this->font = font->GetFont ();
	this->deco = deco;
	this->fg = fg;
}

TextRun::TextRun (TextFontDescription *font)
{
	// This TextRun will represent a LineBreak
	this->deco = TextDecorationsNone;
	this->font = font->GetFont ();
	this->text = NULL;
	this->fg = NULL;
}

TextRun::~TextRun ()
{
	font->unref ();
	g_free (text);
}



class TextSegment : public List::Node {
public:
	cairo_path_t *path;
	double advance;
	double width;
	TextRun *run;
	int start;
	int end;
	
	TextSegment (TextRun *run, int start);
	~TextSegment ();
};

TextSegment::TextSegment (TextRun *run, int start)
{
	this->advance = 0.0;
	this->width = 0.0;
	this->start = start;
	this->end = start;
	this->path = NULL;
	this->run = run;
}

TextSegment::~TextSegment ()
{
	if (path)
		cairo_path_destroy (path);
}




class TextLine : public List::Node {
public:
	List *segments;
	double descend;
	double height;
	
	TextLine ();
	~TextLine ();
};

TextLine::TextLine ()
{
	segments = new List ();
	descend = 0.0;
	height = -1.0;
}

TextLine::~TextLine ()
{
	segments->Clear (true);
	delete segments;
}




TextLayout::TextLayout ()
{
	wrapping = TextWrappingNoWrap;
	max_height = -1.0;
	max_width = -1.0;
	
	runs = NULL;
	
	lines = new List ();
	
	actual_height = -1.0;
	actual_width = -1.0;
	bbox_height = -1.0;
	bbox_width = -1.0;
}

TextLayout::~TextLayout ()
{
	if (runs) {
		runs->Clear (true);
		delete runs;
	}
	
	lines->Clear (true);
	delete lines;
}

double
TextLayout::GetMaxWidth ()
{
	return max_width;
}

void
TextLayout::SetMaxWidth (double max)
{
	if (max_width == max)
		return;
	
	max_width = max;
	
	actual_height = -1.0;
	actual_width = -1.0;
	bbox_height = -1.0;
	bbox_width = -1.0;
}

double
TextLayout::GetMaxHeight ()
{
	return max_height;
}

void
TextLayout::SetMaxHeight (double max)
{
	if (max_height == max)
		return;
	
	max_height = max;
	
	actual_height = -1.0;
	actual_width = -1.0;
	bbox_height = -1.0;
	bbox_width = -1.0;
}

TextWrapping
TextLayout::GetWrapping ()
{
	return wrapping;
}

void
TextLayout::SetWrapping (TextWrapping wrapping)
{
	if (this->wrapping == wrapping)
		return;
	
	this->wrapping = wrapping;
	
	actual_height = -1.0;
	actual_width = -1.0;
	bbox_height = -1.0;
	bbox_width = -1.0;
}

List *
TextLayout::GetTextRuns ()
{
	return runs;
}

void
TextLayout::SetTextRuns (List *runs)
{
	if (this->runs) {
		this->runs->Clear (true);
		delete this->runs;
	}
	
	this->runs = runs;
	
	actual_height = -1.0;
	actual_width = -1.0;
	bbox_height = -1.0;
	bbox_width = -1.0;
}

/**
 * TextLayout::GetActualExtents:
 * @width:
 * @height:
 *
 * Gets the actual width and height extents required for rendering the
 * full text.
 **/
void
TextLayout::GetActualExtents (double *width, double *height)
{
	*height = actual_height;
	*width = actual_width;
}

/**
 * TextLayout::GetLayoutExtents:
 * @width:
 * @height:
 *
 * Gets the width and height extents suitable for rendering the text
 * w/ the current wrapping model.
 **/
void
TextLayout::GetLayoutExtents (double *width, double *height)
{
	*height = bbox_height;
	*width = bbox_width;
}


struct Space {
	double width;
	int index;
};

#define BBOX_MARGIN 1.0
#define BBOX_PADDING 2.0

static int
IndexOfLastWord (gunichar *text)
{
	gunichar *inptr = text;
	gunichar *word = NULL;
	
	while (g_unichar_isspace (*inptr))
		inptr++;
	
	while (*inptr) {
		// skip over the word
		word = inptr++;
		while (*inptr && !g_unichar_isspace (*inptr))
			inptr++;
		
		if (*inptr == '\n') {
			word = NULL;
			inptr++;
		}
		
		// skip over lwsp
		while (g_unichar_isspace (*inptr))
			inptr++;
	}
	
	if (word != NULL)
		return (word - text);
	
	return -1;
}


static void
print_run_text (const char *msg, gunichar *start, gunichar *end)
{
	GString *str = g_string_new ("");
	
	while (*start && (end ? start < end : true)) {
		g_string_append_unichar (str, *start);
		start++;
	}
	
	printf ("%s = \"%s\"\n", msg, str->str);
	
	g_string_free (str, true);
}

void
TextLayout::LayoutWrapWithOverflow ()
{
	double x0 = 0.0, x1 = 0.0, wx = 0.0, dy = 0.0;
	register gunichar *start, *word, *inptr;
	bool underlined = false;
	TextSegment *segment;
	double descend = 0.0;
	double height = 0.0;
	bool blank = true;
	GlyphInfo *glyph;
	TextLine *line;
	double advance;
	uint32_t prev;
	TextRun *run;
	
	line = new TextLine ();
	for (run = (TextRun *) runs->First (); run; run = (TextRun *) run->next) {
		if (run->text == NULL) {
			// LineBreak
			if (height == 0.0) {
				descend = run->font->Descender ();
				height = run->font->Height ();
			}
			
			line->descend = descend;
			line->height = height;
			lines->Append (line);
			
			dy += height;
			
			// blank lines do not count toward 'ActualHeight' extents
			// unless underlined
			if (!blank || underlined)
				actual_height = dy;
			
			if (run->next)
				line = new TextLine ();
			else
				line = NULL;
			
			underlined = false;
			blank = true;
			
			descend = 0.0;
			height = 0.0;
			x0 = 0.0;
			
			continue;
		}
		
		if (!underlined)
			underlined = run->deco == TextDecorationsUnderline;
		descend = MIN (descend, run->font->Descender ());
		height = MAX (height, run->font->Height ());
		
		segment = new TextSegment (run, 0);
		inptr = start = run->text;
		prev = 0;
		x1 = x0;
		
		do {
			// always include the lwsp, it is allowed to go past max_width
			while (*inptr == ' ') {
				if ((glyph = run->font->GetGlyphInfo (*inptr))) {
					advance = glyph->metrics.horiAdvance;
					
					if (prev != 0)
						advance += run->font->Kerning (prev, glyph->index);
					else if (glyph->metrics.horiBearingX < 0)
						advance -= glyph->metrics.horiBearingX;
					
					prev = glyph->index;
					x1 += advance;
				}
				
				inptr++;
			}
			
			// trailing lwsp only counts toward 'ActualWidth' extents if underlined
			if (run->deco == TextDecorationsUnderline) {
				actual_width = MAX (actual_width, x1);
				segment->width = x1 - x0;
			}
			
			if (*inptr == 0)
				break;
			
			segment->advance = x1 - x0;
			word = inptr;
			wx = x1;
			
			if (max_width > 0.0 && x1 >= max_width) {
			linebreak:
				if (segment->start < (word - run->text)) {
					line->segments->Append (segment);
					
					segment = new TextSegment (run, word - run->text);
					start = word;
				} else {
					// reuse the segment
				}
				
				line->descend = descend;
				line->height = height;
				lines->Append (line);
				
				dy += height;
				
				// blank lines do not count toward 'ActualHeight' extents
				// unless underlined
				if (!blank || underlined)
					actual_height = dy;
				
				line = new TextLine ();
				blank = true;
				
				underlined = run->deco == TextDecorationsUnderline;
				descend = run->font->Descender ();
				height = run->font->Height ();
				
				prev = 0;
				x0 = 0.0;
				x1 = 0.0;
				wx = 0.0;
			}
			
			// append this word onto the line
			inptr = word;
			while (*inptr && *inptr != ' ') {
				if ((glyph = run->font->GetGlyphInfo (*inptr))) {
					advance = glyph->metrics.horiAdvance;
					
					if (prev != 0)
						advance += run->font->Kerning (prev, glyph->index);
					else if (glyph->metrics.horiBearingX < 0)
						advance -= glyph->metrics.horiBearingX;
					
					prev = glyph->index;
					x1 += advance;
					
					if (max_width > 0.0 && x1 >= max_width && wx > 0.0)
						goto linebreak;
				}
				
				inptr++;
			}
			
			actual_width = MAX (actual_width, x1);
			segment->end = inptr - run->text;
			segment->width = x1 - x0;
			blank = false;
		} while (*inptr);
		
		segment->advance = x1 - x0;
		line->segments->Append (segment);
		
		x0 = x1;
	}
	
	if (line) {
		line->descend = descend;
		line->height = height;
		lines->Append (line);
		
		dy += height;
		
		// blank lines do not count toward 'ActualHeight' extents
		// unless underlined
		if (!blank || underlined)
			actual_height = dy;
	}
}

void
TextLayout::LayoutNoWrap ()
{
	double x0 = 0.0, x1 = 0.0, dy = 0.0;
	register gunichar *inptr;
	bool underlined = false;
	bool clipped = false;
	TextSegment *segment;
	double descend = 0.0;
	double height = 0.0;
	bool blank = true;
	GlyphInfo *glyph;
	TextLine *line;
	double advance;
	uint32_t prev;
	TextRun *run;
	
	line = new TextLine ();
	for (run = (TextRun *) runs->First (); run; run = (TextRun *) run->next) {
		if (run->text == NULL) {
			// LineBreak
			if (height == 0.0) {
				descend = run->font->Descender ();
				height = run->font->Height ();
			}
			
			line->descend = descend;
			line->height = height;
			lines->Append (line);
			
			dy += height;
			
			// blank lines do not count toward 'ActualHeight' extents
			// unless underlined
			if (!blank || underlined)
				actual_height = dy;
			
			if (run->next)
				line = new TextLine ();
			else
				line = NULL;
			
			underlined = false;
			clipped = false;
			blank = true;
			
			descend = 0.0;
			height = 0.0;
			x0 = 0.0;
			
			continue;
		} else if (clipped) {
			// once we've clipped, we cannot append anymore text to the line
			continue;
		}
		
		if (!underlined)
			underlined = run->deco == TextDecorationsUnderline;
		descend = MIN (descend, run->font->Descender ());
		height = MAX (height, run->font->Height ());
		
		segment = new TextSegment (run, 0);
		inptr = run->text;
		prev = 0;
		x1 = x0;
		
		do {
			// always include the lwsp, it is allowed to go past max_width
			while (*inptr == ' ') {
				if ((glyph = run->font->GetGlyphInfo (*inptr))) {
					advance = glyph->metrics.horiAdvance;
					
					if (prev != 0)
						advance += run->font->Kerning (prev, glyph->index);
					else if (glyph->metrics.horiBearingX < 0)
						advance -= glyph->metrics.horiBearingX;
					
					prev = glyph->index;
					x1 += advance;
				}
				
				inptr++;
			}
			
			// trailing lwsp only counts toward 'ActualWidth' extents if underlined
			if (run->deco == TextDecorationsUnderline) {
				actual_width = MAX (actual_width, x1);
				segment->width = x1 - x0;
			}
			
			if (*inptr == 0)
				break;
			
			// only the first 'word' on a line may start past max_width
			if (blank || max_width <= 0.0 || x1 < max_width) {
				// append this word onto the line
				while (*inptr && *inptr != ' ') {
					if ((glyph = run->font->GetGlyphInfo (*inptr))) {
						advance = glyph->metrics.horiAdvance;
						
						if (prev != 0)
							advance += run->font->Kerning (prev, glyph->index);
						else if (glyph->metrics.horiBearingX < 0)
							advance -= glyph->metrics.horiBearingX;
						
						prev = glyph->index;
						x1 += advance;
					}
					
					inptr++;
				}
				
				actual_width = MAX (actual_width, x1);
				segment->end = inptr - run->text;
				segment->width = x1 - x0;
				blank = false;
				
				if (max_width > 0.0 && x1 >= max_width) {
					// cut the remainder of the run unless it is underlined
					// (in which case we need to underline trailing lwsp).
					if (run->deco != TextDecorationsUnderline) {
						clipped = true;
						break;
					}
				}
			} else {
				// next word would overflow max specified width
				clipped = true;
				break;
			}
		} while (*inptr);
		
		segment->advance = x1 - x0;
		line->segments->Append (segment);
		
		x0 = x1;
	}
	
	if (line) {
		line->descend = descend;
		line->height = height;
		lines->Append (line);
		
		dy += height;
		
		// blank lines do not count toward 'ActualHeight' extents
		// unless underlined
		if (!blank || underlined)
			actual_height = dy;
	}
}


struct LastWord {
	TextRun *run;
	gunichar *word;
};


static bool
isLastWord (TextRun *run, gunichar *word, bool *include_lwsp)
{
	register gunichar *inptr = word;
	
	// skip to the end of this word
	while (*inptr && *inptr != ' ')
		inptr++;
	
	// skip over trailing lwsp
	while (*inptr == ' ')
		inptr++;
	
	if (*inptr != 0)
		return false;
	
	// now we need to check following Runs
	while (run->next) {
		run = (TextRun *) run->next;
		
		if (!run->text)
			return true;
		
		inptr = run->text;
		
		// skip over lwsp
		while (*inptr == ' ')
			inptr++;
		
		if (*inptr != 0)
			return false;
	}
	
	*include_lwsp = true;
	
	return true;
}


/**
 * Notes: The last 'word' of any line must not be broken (bug in
 * Silverlight's text layout)
 **/
void
TextLayout::LayoutWrap ()
{
	double x0 = 0.0, x1 = 0.0, wx = 0.0, dy = 0.0;
	register gunichar *start, *word, *inptr;
	bool include_lwsp = false;
	bool underlined = false;
	bool last_word = false;
	TextSegment *segment;
	double descend = 0.0;
	double height = 0.0;
	bool blank = true;
	GlyphInfo *glyph;
	TextLine *line;
	double advance;
	uint32_t prev;
	TextRun *run;
	
	line = new TextLine ();
	for (run = (TextRun *) runs->First (); run; run = (TextRun *) run->next) {
		if (run->text == NULL) {
			// LineBreak
			if (height == 0.0) {
				descend = run->font->Descender ();
				height = run->font->Height ();
			}
			
			line->descend = descend;
			line->height = height;
			lines->Append (line);
			
			dy += height;
			
			// blank lines do not count toward 'ActualHeight' extents
			// unless underlined
			if (!blank || underlined)
				actual_height = dy;
			
			if (run->next)
				line = new TextLine ();
			else
				line = NULL;
			
			underlined = false;
			last_word = false;
			blank = true;
			
			descend = 0.0;
			height = 0.0;
			x0 = 0.0;
			
			continue;
		}
		
		if (!underlined)
			underlined = run->deco == TextDecorationsUnderline;
		descend = MIN (descend, run->font->Descender ());
		height = MAX (height, run->font->Height ());
		
		segment = new TextSegment (run, 0);
		print_run_text ("Laying out Run.Text", run->text, NULL);
		inptr = run->text;
		prev = 0;
		x1 = x0;
		
		do {
			// always include the lwsp, it is allowed to go past max_width
			start = inptr;
			while (*inptr == ' ') {
				if ((glyph = run->font->GetGlyphInfo (*inptr))) {
					advance = glyph->metrics.horiAdvance;
					
					if (prev != 0)
						advance += run->font->Kerning (prev, glyph->index);
					else if (glyph->metrics.horiBearingX < 0)
						advance -= glyph->metrics.horiBearingX;
					
					prev = glyph->index;
					x1 += advance;
				}
				
				inptr++;
			}
			
			// trailing lwsp only counts toward 'ActualWidth' extents if underlined
			if (run->deco == TextDecorationsUnderline || include_lwsp) {
				actual_width = MAX (actual_width, x1);
				segment->width = x1 - x0;
			}
			
			if (*inptr == 0)
				break;
			
			segment->advance = x1 - x0;
			word = inptr;
			wx = x1;
			
			// check to see if this is the last word of the line
			last_word = isLastWord (run, word, &include_lwsp);
			
			if (max_width > 0.0 && x1 >= max_width) {
			linebreak:
				if (segment->advance > 0.0) {
					line->segments->Append (segment);
					
					segment = new TextSegment (run, word - run->text);
					start = word;
				} else {
					// reuse the segment
				}
				
				line->descend = descend;
				line->height = height;
				lines->Append (line);
				
				dy += height;
				
				// blank lines do not count toward 'ActualHeight' extents
				// unless underlined
				if (!blank || underlined)
					actual_height = dy;
				
				line = new TextLine ();
				blank = true;
				
				underlined = run->deco == TextDecorationsUnderline;
				descend = run->font->Descender ();
				height = run->font->Height ();
				
				prev = 0;
				x0 = 0.0;
				x1 = 0.0;
				wx = 0.0;
			}
			
			// append this word onto the line
			inptr = word;
			while (*inptr && *inptr != ' ') {
				if ((glyph = run->font->GetGlyphInfo (*inptr))) {
					advance = glyph->metrics.horiAdvance;
					
					if (prev != 0)
						advance += run->font->Kerning (prev, glyph->index);
					else if (glyph->metrics.horiBearingX < 0)
						advance -= glyph->metrics.horiBearingX;
					
					if (max_width > 0.0 && (x1 + advance) > (max_width + 1.0)) {
						if (wx == 0.0 && inptr > word && !last_word) {
							// break in the middle of a word
							actual_width = MAX (actual_width, x1);
							segment->end = inptr - run->text;
							segment->advance = x1 - x0;
							segment->width = x1 - x0;
							blank = false;
							word = inptr;
							goto linebreak;
						} else if (wx > 0.0) {
							// break before this word
							segment->advance = wx - x0;
							segment->width = wx - x0;
							goto linebreak;
						}
					}
					
					prev = glyph->index;
					x1 += advance;
				}
				
				inptr++;
			}
			
			actual_width = MAX (actual_width, x1);
			segment->end = inptr - run->text;
			segment->width = x1 - x0;
			blank = false;
		} while (*inptr);
		
		segment->advance = x1 - x0;
		line->segments->Append (segment);
		
		x0 = x1;
	}
	
	if (line) {
		line->descend = descend;
		line->height = height;
		lines->Append (line);
		
		dy += height;
		
		// blank lines do not count toward 'ActualHeight' extents
		// unless underlined
		if (!blank || underlined)
			actual_height = dy;
	}
}


static void
print_lines (List *lines)
{
	TextSegment *segment;
	TextLine *line;
	GString *str;
	int ln = 0;
	int i;
	
	printf ("layout results:\n");
	
	str = g_string_new ("");
	line = (TextLine *) lines->First ();
	
	while (line) {
		printf ("\tline #%d: ", ln);
		
		segment = (TextSegment *) line->segments->First ();
		
		while (segment) {
			for (i = segment->start; i < segment->end; i++)
				g_string_append_unichar (str, segment->run->text[i]);
			
			printf ("\"%s\", ", str->str);
			g_string_truncate (str, 0);
			
			segment = (TextSegment *) segment->next;
		}
		
		printf ("\n");
		
		line = (TextLine *) line->next;
		ln++;
	}
}

void
TextLayout::Layout ()
{
	if (actual_width != -1.0)
		return;
	
	lines->Clear (true);
	actual_height = 0.0;
	actual_width = 0.0;
	
	if (!runs || runs->IsEmpty ())
		return;
	
	switch (wrapping) {
	case TextWrappingWrapWithOverflow:
		if (max_width > 0.0)
			printf ("TextLayout::LayoutWrapWithOverflow(%f)\n", max_width);
		else
			printf ("TextLayout::LayoutWrapWithOverflow()\n");
		LayoutWrapWithOverflow ();
		break;
	case TextWrappingNoWrap:
	default:
		if (max_width > 0.0)
			printf ("TextLayout::LayoutWrapNoWrap(%f)\n", max_width);
		else
			printf ("TextLayout::LayoutNoWrap()\n");
		LayoutNoWrap ();
		break;
	case TextWrappingWrap:
		if (max_width > 0.0)
			printf ("TextLayout::LayoutWrap(%f)\n", max_width);
		else
			printf ("TextLayout::LayoutWrap()\n");
		LayoutWrap ();
		break;
	}
	
	print_lines (lines);
	
	printf ("actualWidth = %f, actualHeight = %f\n", actual_width, actual_height);
	
	bbox_height = actual_height;
	bbox_width = actual_width;
}

static inline void
RenderLine (cairo_t *cr, UIElement *element, TextLine *line, Brush *default_fg, double x, double y)
{
	TextFont *font = NULL;
	TextDecorations deco;
	TextSegment *segment;
	const gunichar *text;
	uint32_t prev = 0;
	GlyphInfo *glyph;
	double x1, y1;
	double x0, y0;
	Brush *fg;
	int i;
	
	// set y0 to the line's baseline (descend is a negative value)
	y0 = y + line->height + line->descend;
	
	x0 = x;
	
	segment = (TextSegment *) line->segments->First ();
	
	while (segment) {
		text = segment->run->text;
		deco = segment->run->deco;
		font = segment->run->font;
		
		cairo_save (cr);
		cairo_translate (cr, x0, y0 - font->Ascender ());
		
		// set y1 to the baseline relative to the translation matrix
		y1 = font->Ascender ();
		x1 = 0.0;
		
		if (segment->run->fg && *segment->run->fg)
			fg = *segment->run->fg;
		else
			fg = default_fg;
		
		fg->SetupBrush (cr, element, segment->advance, font->Height ());
		
		if (!segment->path) {
			if (font->IsScalable () && segment->start < segment->end)
				cairo_new_path (cr);
			
			for (i = segment->start, prev = 0; i < segment->end; i++) {
				if (!(glyph = font->GetGlyphInfo (text[i])))
					continue;
				
				if (prev != 0)
					x1 += font->Kerning (prev, glyph->index);
				else if (glyph->metrics.horiBearingX < 0)
					x1 -= glyph->metrics.horiBearingX;
				
				prev = glyph->index;
				
				if (!font->IsScalable ())
					font->Render (cr, glyph, x1, y1);
				else
					font->Path (cr, glyph, x1, y1);
				
				x1 += glyph->metrics.horiAdvance;
			}
			
			if (font->IsScalable () && segment->start < segment->end) {
				cairo_close_path (cr);
				segment->path = cairo_copy_path (cr);
				cairo_fill (cr);
			}
		} else {
			// it is an error to append a path with no data
			if (segment->path->data)
				cairo_append_path (cr, segment->path);
			
			cairo_fill (cr);
		}
		
		if (deco == TextDecorationsUnderline && segment->width > 0.0) {
			cairo_antialias_t aa = cairo_get_antialias (cr);
			double thickness = font->UnderlineThickness ();
			double pos = y1 + font->UnderlinePosition ();
			
			cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
			cairo_set_line_width (cr, thickness);
			x1 = segment->width;
			
			cairo_new_path (cr);
			cairo_move_to (cr, 0.0, pos);
			cairo_line_to (cr, x1, pos);
			cairo_stroke (cr);
			
			cairo_set_antialias (cr, aa);
		}
		
		x0 += segment->advance;
		
		segment = (TextSegment *) segment->next;
		cairo_restore (cr);
	}
}

void
TextLayout::Render (cairo_t *cr, UIElement *element, Brush *default_fg, double x, double y)
{
	TextLine *line;
	double y1 = y;
	
	Layout ();
	
	line = (TextLine *) lines->First ();
	
	while (line) {
		RenderLine (cr, element, line, default_fg, x, y1);
		y1 += (double) line->height;
		
		line = (TextLine *) line->next;
	}
}
