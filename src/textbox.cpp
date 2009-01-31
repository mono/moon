/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * textbox.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk/gdkkeysyms.h>
#include <cairo.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "dependencyproperty.h"
#include "contentcontrol.h"
#include "textbox.h"
#include "utils.h"



//
// TextBuffer
//

#define UNICODE_LEN(size) (sizeof (gunichar) * (size))
#define UNICODE_OFFSET(buf,offset) (((char *) buf) + sizeof (gunichar) * (offset))

class TextBuffer {
	int allocated;
	
	void Resize (int needed)
	{
		bool resize = false;
		
		if (allocated >= needed + 128) {
			while (allocated >= needed + 128)
				allocated -= 128;
			resize = true;
		} else if (allocated < needed) {
			while (allocated < needed)
				allocated += 128;
			resize = true;
		}
		
		if (resize)
			text = (gunichar *) g_realloc (text, UNICODE_LEN (allocated));
	}
	
 public:
	gunichar *text;
	int len;
	
	TextBuffer ()
	{
		text = NULL;
		Reset ();
	}
	
	void Reset ()
	{
		text = (gunichar *) g_realloc (text, UNICODE_LEN (128));
		allocated = 128;
		text[0] = '\0';
		len = 0;
	}
	
	void Append (gunichar c)
	{
		Resize (len + 2);
		
		text[len++] = c;
		text[len] = 0;
	}
	
	void Append (const gunichar *str, int count)
	{
		Resize (len + count + 1);
		
		memcpy (UNICODE_OFFSET (text, len), str, UNICODE_LEN (count));
		len += count;
		text[len] = 0;
	}
	
	void Cut (int start, int length)
	{
		int offset;
		
		if (length == 0 || start >= len)
			return;
		
		offset = MAX (len, start + length);
		
		memmove (UNICODE_OFFSET (text, start), UNICODE_OFFSET (text, offset), UNICODE_LEN ((len - offset) + 1));
	}
	
	void Insert (int index, gunichar c)
	{
		Resize (len + 2);
		
		if (index < len) {
			// shift all chars beyond position @index down by 1 char
			memmove (UNICODE_OFFSET (text, index + 1), UNICODE_OFFSET (text, index), UNICODE_LEN ((len - index) + 1));
			text[index] = c;
			len++;
		} else {
			text[len++] = c;
			text[len] = 0;
		}
	}
	
	void Insert (int index, const gunichar *str, int count)
	{
		Resize (len + count + 1);
		
		if (index < len) {
			// shift all chars beyond position @index down by @count chars
			memmove (UNICODE_OFFSET (text, index + count), UNICODE_OFFSET (text, index), UNICODE_LEN ((len - index) + 1));
			
			// insert @count chars of @str into our buffer at position @index
			memcpy (UNICODE_OFFSET (text, index), str, UNICODE_LEN (count));
			len += count;
		} else {
			// simply append @count chars of @str onto the end of our buffer
			memcpy (UNICODE_OFFSET (text, len), str, UNICODE_LEN (count));
			len += count;
			text[len] = 0;
		}
	}
	
	void Prepend (gunichar c)
	{
		Resize (len + 2);
		
		// shift the entire buffer down by 1 char
		memmove (UNICODE_OFFSET (text, 1), text, UNICODE_LEN (len + 1));
		text[0] = c;
		len++;
	}
	
	void Prepend (const gunichar *str, int count)
	{
		Resize (len + count + 1);
		
		// shift the endtire buffer down by @count chars
		memmove (UNICODE_OFFSET (text, count), text, UNICODE_LEN (len + 1));
		
		// copy @count chars of @str into the beginning of our buffer
		memcpy (text, str, UNICODE_LEN (count));
		len += count;
	}
	
	void Replace (int start, int length, const gunichar *str, int count)
	{
		int beyond;
		
		if (start > len) {
			g_warning ("TextBuffer::Replace() start out of range");
			return;
		}
		
		if (start + length > len) {
			g_warning ("TextBuffer::Replace() length out of range");
			return;
		}
		
		// Check for the easy cases first...
		if (length == 0) {
			Insert (start, str, count);
			return;
		} else if (count == 0) {
			Cut (start, length);
			return;
		} else if (count == length) {
			memcpy (UNICODE_OFFSET (text, start), str, UNICODE_LEN (count));
			return;
		}
		
		Resize ((len - length) + count + 1);
		
		// calculate the number of chars beyond @start that won't be cut
		beyond = len - (start + length);
		
		// shift all chars beyond position (@start + length) into position...
		memmove (UNICODE_OFFSET (text, start + count), UNICODE_OFFSET (text, start + length), UNICODE_LEN (beyond + 1));
		
		// copy @count chars of @str into our buffer at position @start
		memcpy (UNICODE_OFFSET (text, start), str, UNICODE_LEN (count));
		
		len = (len - length) + count;
	}
};


//
// TextBox
//

// emit state
#define NOTHING_CHANGED         (0)
#define CURSOR_POSITION_CHANGED (1 << 0)
#define SELECTION_CHANGED       (1 << 1)
#define TEXT_CHANGED            (1 << 2)

// cursor state
#define SELECTION_BEGIN 0
#define SELECTION_END   1

TextBox::TextBox ()
{
	SetObjectType (Type::TEXTBOX);
	ManagedTypeInfo *type_info = new ManagedTypeInfo ();
	type_info->assembly_name = g_strdup ("System.Windows");
	type_info->full_name = g_strdup ("System.Windows.Controls.TextBox");
	
	SetDefaultStyleKey (type_info);
	
	font = new TextFontDescription ();
	font->SetFamily (CONTROL_FONT_FAMILY);
	font->SetStretch (CONTROL_FONT_STRETCH);
	font->SetWeight (CONTROL_FONT_WEIGHT);
	font->SetStyle (CONTROL_FONT_STYLE);
	font->SetSize (CONTROL_FONT_SIZE);
	
	buffer = new TextBuffer ();
	
	cursor = SELECTION_BEGIN;
	emit = NOTHING_CHANGED;
	selection.length = 0;
	selection.start = 0;
	setvalue = true;
	maxlen = 0;
	cursor = 0;
	
#if 0
	// Okay, so these values probably come from the theme because
	// by default they are supposed to be null according to the
	// unit tests.
	Brush *brush = new SolidColorBrush ("#FF444444");
	SetValue (TextBox::SelectionBackgroundProperty, Value (brush));
	brush->unref ();
	
	brush = new SolidColorBrush ("#FFFFFFFF");
	SetValue (TextBox::SelectionForegroundProperty, Value (brush));
	brush->unref ();
#endif
}

TextBox::~TextBox ()
{
	delete buffer;
	delete font;
}

#define MY_GDK_ALT_MASK         (GDK_MOD1_MASK | GDK_MOD2_MASK | GDK_MOD3_MASK | GDK_MOD4_MASK | GDK_MOD5_MASK)

#define IsEOL(c) ((c) == '\r' || (c) == '\n')

static int
move_down (TextBuffer *buffer, int cursor, int n_lines)
{
	int offset, cur, n;
	
	// first find out what our character offset is in the current line
	cur = cursor;
	while (cur > 0 && !IsEOL (buffer->text[cur - 1]))
		cur--;
	
	offset = cursor - cur;
	
	cur = cursor;
	n = 0;
	
	// skip ahead one page worth of lines
	while (n < n_lines) {
		while (cur < buffer->len && !IsEOL (buffer->text[cur]))
			cur++;
		
		if (cur == buffer->len)
			break;
		
		if (buffer->text[cur] == '\r' && buffer->text[cur + 1] == '\n')
			cur++;
		
		cur++;
		n++;
	}
	
	if (n == n_lines) {
		// go forward until we're at the same character offset
		if ((buffer->len - cur) < offset)
			cur += buffer->len;
		else
			cur += offset;
	}
	
	return cur;
}

static int
move_up (TextBuffer *buffer, int cursor, int n_lines)
{
	int offset, cur, n;
	
	// first find out what our character offset is in the current line
	cur = cursor;
	while (cur > 0 && !IsEOL (buffer->text[cur - 1]))
		cur--;
	
	offset = cursor - cur;
	n = 0;
	
	// go back one page worth of lines
	while (n < n_lines) {
		while (cur > 0 && !IsEOL (buffer->text[cur - 1]))
			cur--;
		
		if (cur == 0)
			break;
		
		if (cur >= 2 && buffer->text[cur - 2] == '\r' && buffer->text[cur - 1] == '\n')
			cur--;
		
		cur--;
		n++;
	}
	
	if (n == n_lines) {
		// go forward until we're at the same character offset
		if ((buffer->len - cur) < offset)
			cur += buffer->len;
		else
			cur += offset;
	}
	
	return cur;
}

enum CharClass {
	CharClassUnknown,
	CharClassWhitespace,
	CharClassAlphaNumeric
};

static inline CharClass
char_class (gunichar c)
{
	if (g_unichar_isspace (c))
		return CharClassWhitespace;
	
	if (g_unichar_isalnum (c))
		return CharClassAlphaNumeric;
	
	return CharClassUnknown;
}

static int
next_word (TextBuffer *buffer, int cursor)
{
	int i, lf, cr = cursor;
	CharClass cc;
	
	// find the end of the current line
	while (cr < buffer->len && !IsEOL (buffer->text[cr]))
		cr++;
	
	if (buffer->text[cr] == '\r' && buffer->text[cr + 1] == '\n')
		lf = cr + 1;
	else
		lf = cr;
	
	// if the cursor is at the end of the line, return the starting offset of the next line
	if (cursor == cr || cursor == lf) {
		if (lf < buffer->len)
			return lf + 1;
		
		return cursor;
	}
	
	cc = char_class (buffer->text[cursor]);
	i = cursor;
	
	// skip over the word, punctuation, or run of whitespace
	while (i < cr && char_class (buffer->text[i]) == cc)
		i++;
	
	// skip any whitespace after the word/punct
	while (i < cr && char_class (buffer->text[i]) == CharClassWhitespace)
		i++;
	
	return i;
}

static int
prev_word (TextBuffer *buffer, int cursor)
{
	int i, cr, lf = cursor;
	CharClass cc;
	
	// find the beginning of the current line
	while (lf > 0 && !IsEOL (buffer->text[lf - 1]))
		lf--;
	
	if (lf > 0 && buffer->text[lf] == '\n' && buffer->text[lf - 1] == '\r')
		cr = lf - 1;
	else
		cr = lf;
	
	// if the cursor is at the beginning of the line, return the end of the prev line
	if (cursor - 1 == lf) {
		if (cr > 0)
			return cr - 1;
		
		return 0;
	}
	
	cc = char_class (buffer->text[cursor - 1]);
	i = cursor;
	
	// skip over the word, punctuation, or run of whitespace
	while (i > lf && char_class (buffer->text[i - 1]) == cc)
		i--;
	
	// if the cursor was at whitespace, skip back a word too
	if (cc == CharClassWhitespace && i > lf) {
		cc = char_class (buffer->text[i - 1]);
		while (i > lf && char_class (buffer->text[i - 1]) == cc)
			i--;
	}
	
	return i;
}

void
TextBox::KeyPressBackSpace (GdkModifierType modifiers)
{
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) != 0)
		return;
	
	if (selection.length > 0) {
		// BackSpace w/ active selection: delete the selected text
		buffer->Cut (start, length);
		emit |= TEXT_CHANGED;
		length = 0;
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+BackSpace: delete the word ending at the cursor
		pos = prev_word (buffer, start);
		
		if (pos < start) {
			buffer->Cut (pos, start - pos);
			emit |= TEXT_CHANGED;
		        start = pos;
		}
	} else if (start > 0) {
		// BackSpace: delete the char before the cursor position
		if (start >= 2 && buffer->text[start - 1] == '\n' && buffer->text[start - 2] == '\r') {
			buffer->Cut (start - 2, 2);
			start -= 2;
		} else {
			buffer->Cut (start - 1, 1);
			start--;
		}
		
		emit |= TEXT_CHANGED;
	}
	
	cursor = SELECTION_BEGIN;
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		SetSelectionLength (length);
		SetSelectionStart (start);
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressDelete (GdkModifierType modifiers)
{
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) != 0)
		return;
	
	if (selection.length > 0) {
		// Delete w/ active selection: delete the selected text
		buffer->Cut (start, length);
		emit |= TEXT_CHANGED;
		length = 0;
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+Delete: delete the word starting at the cursor
		pos = next_word (buffer, start);
		
		if (pos > start) {
			buffer->Cut (cursor, pos - start);
			emit |= TEXT_CHANGED;
		}
	} else if (start < buffer->len) {
		// Delete: delete the char after the cursor position
		if (buffer->text[start] == '\r' && buffer->text[start + 1] == '\n')
			buffer->Cut (start, 2);
		else
			buffer->Cut (start, 1);
		
		emit |= TEXT_CHANGED;
	}
	
	cursor = SELECTION_BEGIN;
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		SetSelectionLength (length);
		SetSelectionStart (start);
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressPageDown (GdkModifierType modifiers)
{
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK)) != 0)
		return;
	
	if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Page_Down: grow selection by one page in the downward direction
		if (cursor == SELECTION_BEGIN) {
			// cursor is at the beginning of the selection
			pos = move_down (buffer, start, 8);
			
			if (pos > selection.start + selection.length) {
				// flip selection over the current selection endpoint
				start += selection.length;
				cursor = SELECTION_END;
				length = pos - start;
			} else {
				// shrink the selection from the start
				length -= pos - start;
				start = pos;
			}
		} else {
			// cursor is at the end of the selection
			pos = move_down (buffer, start + length, 8);
			
			// grow the selection beyond the current endpoint
			length = pos - start;
		}
	} else {
		// Page_Down: move cursor down one page and clear selection
		start = move_down (buffer, start, 8);
		cursor = SELECTION_BEGIN;
		length = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		SetSelectionLength (length);
		SetSelectionStart (start);
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressPageUp (GdkModifierType modifiers)
{
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK)) != 0)
		return;
	
	if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Page_Up: grow selection by one page in the upward direction
		if (cursor == SELECTION_END) {
			// cursor is at the end of the selection
			pos = move_up (buffer, start + length, 8);
			
			if (pos < start) {
				// flip selection over the current selection starting point
				length = selection.start - pos;
				cursor = SELECTION_BEGIN;
				start = pos;
			} else {
				// shrink the selection from the start
				length += start - pos;
				start = pos;
			}
		} else {
			// cursor is at the beginning of the selection
			pos = move_down (buffer, start, 8);
			
			// grow the selection to the left
			length += start - pos;
			start = pos;
		}
	} else {
		// Page_Up: move cursor up one page and clear selection
		start = move_up (buffer, start, 8);
		cursor = SELECTION_BEGIN;
		length = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		SetSelectionLength (length);
		SetSelectionStart (start);
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressHome (GdkModifierType modifiers)
{
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & MY_GDK_ALT_MASK) != 0)
		return;
	
	if ((modifiers & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
		// Ctrl+Shift+Home: update selection to start at the beginning of the buffer
		pos = 0;
		
		if (cursor == SELECTION_END) {
			// cursor was at the end of the selection
			if (pos < start) {
				// flip selection over the current selection starting point
				cursor = SELECTION_BEGIN;
				length = start - pos;
				start = pos;
			} else {
				// shrink the selection from the start
				length += start - pos;
				start = pos;
			}
		} else {
			// grow the selection to the left
			length += start - pos;
			start = pos;
		}
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+Home: move cursor to beginning of the buffer and clear selection
		cursor = SELECTION_BEGIN;
		length = start = 0;
	} else if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Home: update selection to start at beginning of line
		if (cursor == SELECTION_END)
			pos = start + length;
		else
			pos = start;
		
		while (pos > 0 && !IsEOL (buffer->text[pos - 1]))
			pos--;
		
		if (cursor == SELECTION_END) {
			// cursor was at the end of the selection
			if (pos < start) {
				// flip selection over the current selection starting point
				cursor = SELECTION_BEGIN;
				length = start - pos;
				start = pos;
			} else {
				// shrink the selection from the start
				length += start - pos;
				start = pos;
			}
		} else {
			// grow the selection to the left
			length += start - pos;
			start = pos;
		}
	} else {
		// Home: move cursor to beginning of line and clear selection
		pos = start;
		while (pos > 0 && !IsEOL (buffer->text[pos - 1]))
			pos--;
		
		cursor = SELECTION_BEGIN;
		start = pos;
		length = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		SetSelectionLength (length);
		SetSelectionStart (start);
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressEnd (GdkModifierType modifiers)
{
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & MY_GDK_ALT_MASK) != 0)
		return;
	
	if ((modifiers & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
		// Ctrl+Shift+End: update selection to end at the end of the buffer
		pos = buffer->len;
		
		if (cursor == SELECTION_BEGIN) {
			// cursor was at the beginning of the selection
			if (pos > start + length) {
				// flip selection over the current selection endpoint
				cursor = SELECTION_END;
				start += length;
				length = pos - start;
			} else {
				// shrink the selection from the start
				length -= pos - start;
				start = pos;
			}
		} else {
			// grow the selection beyond the current endpoint
			length = pos - start;
		}
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+End: move cursor to end of the buffer and clear selection
		cursor = SELECTION_BEGIN;
		start = buffer->len;
		length = 0;
	} else if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+End: update selection to end at the end of the current line
		pos = start;
		while (pos < buffer->len && !IsEOL (buffer->text[pos]))
			pos++;
		
		if (cursor == SELECTION_BEGIN) {
			// cursor was at the beginning of the selection
			if (pos > start + length) {
				// flip selection over the current selection endpoint
				cursor = SELECTION_END;
				start += length;
				length = pos - start;
			} else {
				// shrink the selection from the start
				length -= pos - start;
				start = pos;
			}
		} else {
			// grow the selection beyond the current endpoint
			length = pos - start;
		}
	} else {
		// End: move cursor to end of line and clear selection
		pos = start;
		while (pos < buffer->len && !IsEOL (buffer->text[pos]))
			pos++;
		
		cursor = SELECTION_BEGIN;
		start = pos;
		length = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		SetSelectionLength (length);
		SetSelectionStart (start);
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressRight (GdkModifierType modifiers)
{
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & MY_GDK_ALT_MASK) != 0)
		return;
	
	if ((modifiers & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
		// Ctrl+Shift+Right: grow selection to the right on word's worth of characters
		if (cursor == SELECTION_BEGIN) {
			// cursor is at the beginning of the selection
			pos = next_word (buffer, start);
			
			if (pos > selection.start + selection.length) {
				// flip selection over the current selection endpoint
				cursor = SELECTION_END;
				start += length;
				length = pos - start;
			} else {
				// shrink the selection from the start
				length -= pos - start;
				start = pos;
			}
		} else {
			// cursor is at the end of the selection
			pos = next_word (buffer, start + length);
			
			// grow the selection beyond the current endpoint
			length = pos - start;
		}
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+Right: move cursor to right one word's worth of characters and clear selection
		pos = next_word (buffer, start);
		cursor = SELECTION_BEGIN;
		start = pos;
		length = 0;
	} else if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Right: grow the selection to the right by one character
		if (cursor == SELECTION_END)
			pos = start + length;
		else
			pos = start;
		
		if (pos < buffer->len) {
			if (buffer->text[pos] == '\r' && buffer->text[pos + 1] == '\n') 
				pos += 2;
			else
				pos++;
			
			if (cursor == SELECTION_BEGIN) {
				// cursor was at the beginning of the selection
				if (pos > start + length) {
					// flip selection over the current selection endpoint
					cursor = SELECTION_END;
					start += length;
					length = pos - start;
				} else {
					// shrink the selection from the start
					length -= pos - start;
					start = pos;
				}
			} else {
				// grow the selection beyond the current endpoint
				length = pos - start;
			}
		}
	} else {
		// Right: grow the selection to the right by one character
		if (cursor == SELECTION_END)
			pos = start + length;
		else
			pos = start;
		
		if (pos < buffer->len) {
			// Right: move the cursor one character to the right and clear the selection
			if (buffer->text[pos] == '\r' && buffer->text[pos + 1] == '\n') 
				pos += 2;
			else
				pos++;
			
			cursor = SELECTION_BEGIN;
			start = pos;
			length = 0;
		}
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		SetSelectionLength (length);
		SetSelectionStart (start);
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressLeft (GdkModifierType modifiers)
{
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & MY_GDK_ALT_MASK) != 0)
		return;
	
	if ((modifiers & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
		// Ctrl+Shift+Left: grow selection to the left one word's worth of characters
		pos = prev_word (buffer, cursor);
		
		if (cursor == SELECTION_END) {
			// cursor is at the end of the selection
			pos = prev_word (buffer, start + length);
			
			if (pos < start) {
				// flip selection over the current selection starting point
				cursor = SELECTION_BEGIN;
				length = start - pos;
				start = pos;
			} else {
				// shrink the selection from the start
				length += start - pos;
				start = pos;
			}
		} else {
			// cursor is at the beginning of the selection
			pos = prev_word (buffer, start);
			
			// grow the selection to the left
			length += start - pos;
			start = pos;
		}
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+Left: move cursor to left one word's worth of characters and clear selection
		pos = prev_word (buffer, start);
		cursor = SELECTION_BEGIN;
		start = pos;
		length = 0;
	} else if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Left: grow the selection to the left by one character
		if (cursor == SELECTION_END)
			pos = start + length;
		else
			pos = start;
		
		if (pos > 0) {
			if (pos >= 2 && buffer->text[pos - 2] == '\r' && buffer->text[pos - 1] == '\n')
				pos -= 2;
			else
				pos--;
			
			if (cursor == SELECTION_END) {
				// cursor was at the end of the selection
				if (pos < start) {
					// flip selection over the current selection starting point
					cursor = SELECTION_BEGIN;
					length = start - pos;
					start = pos;
				} else {
					// shrink the selection from the start
					length += start - pos;
					start = pos;
				}
			} else {
				// grow the selection to the left
				length += start - pos;
				start = pos;
			}
		}
	} else {
		if (cursor == SELECTION_END)
			pos = start + length;
		else
			pos = start;
		
		if (pos > 0) {
			// Left: move the cursor one character to the right and clear the selection
			if (pos >= 2 && buffer->text[pos - 2] == '\r' && buffer->text[pos - 1] == '\n')
				pos -= 2;
			else
				pos--;
			
			cursor = SELECTION_BEGIN;
			start = pos;
			length = 0;
		}
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		SetSelectionLength (length);
		SetSelectionStart (start);
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressDown (GdkModifierType modifiers)
{
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK)) != 0)
		return;
	
	if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Down: grow selection by one line in the downward direction
		if (cursor == SELECTION_BEGIN) {
			// cursor is at the beginning of the selection
			pos = move_down (buffer, start, 1);
			
			if (pos > selection.start + selection.length) {
				// flip selection over the current selection endpoint
				start += selection.length;
				cursor = SELECTION_END;
				length = pos - start;
			} else {
				// shrink the selection from the start
				length -= pos - start;
				start = pos;
			}
		} else {
			// cursor is at the end of the selection
			pos = move_down (buffer, start + length, 1);
			
			// grow the selection beyond the current endpoint
			length = pos - start;
		}
	} else {
		// Down: move cursor down one line and clear selection
		start = move_down (buffer, start, 8);
		cursor = SELECTION_BEGIN;
		length = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		SetSelectionLength (length);
		SetSelectionStart (start);
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressUp (GdkModifierType modifiers)
{
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK)) != 0)
		return;
	
	if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Page_Up: grow selection by one line in the upward direction
		if (cursor == SELECTION_END) {
			// cursor is at the end of the selection
			pos = move_up (buffer, start + length, 1);
			
			if (pos < start) {
				// flip selection over the current selection starting point
				length = selection.start - pos;
				cursor = SELECTION_BEGIN;
				start = pos;
			} else {
				// shrink the selection from the start
				length += start - pos;
				start = pos;
			}
		} else {
			// cursor is at the beginning of the selection
			pos = move_down (buffer, start, 8);
			
			// grow the selection to the left
			length += start - pos;
			start = pos;
		}
	} else {
		// Page_Up: move cursor up one line and clear selection
		start = move_up (buffer, start, 1);
		cursor = SELECTION_BEGIN;
		length = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		SetSelectionLength (length);
		SetSelectionStart (start);
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressUnichar (gunichar c)
{
	int length = selection.length;
	int start = selection.start;
	
	if ((maxlen > 0 && buffer->len >= maxlen) || ((c == '\r') && !GetAcceptsReturn ()))
		return;
	
	if (length > 0) {
		// replace the currently selected text
		printf ("TextBox::KeyPressUnichar(): relacing selection with '%c'\n", (char) c);
		buffer->Replace (start, length, &c, 1);
	} else {
		// insert the text at the cursor position
		printf ("TextBox::KeyPressUnichar(): inserting '%c' @ %d\n", (char) c, start);
		buffer->Insert (start, c);
	}
	
	cursor = SELECTION_BEGIN;
	emit |= TEXT_CHANGED;
	length = 0;
	start++;
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		SetSelectionLength (length);
		SetSelectionStart (start);
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressFreeze ()
{
	emit = NOTHING_CHANGED;
}

void
TextBox::KeyPressThaw ()
{
	if (emit & TEXT_CHANGED)
		SyncText ();
	
	if (emit & SELECTION_CHANGED)
		SyncSelectedText ();
	
	if (emit & TEXT_CHANGED)
		EmitTextChanged ();
	
	if (emit & SELECTION_CHANGED)
		EmitSelectionChanged ();
	
	if (emit & CURSOR_POSITION_CHANGED)
		Emit (TextBox::ModelChangedEvent, new TextBoxModelChangedEventArgs (TextBoxModelChangedCursorPosition));
}

void
TextBox::EmitSelectionChanged ()
{
	Emit (TextBox::SelectionChangedEvent, new RoutedEventArgs ());
}

void
TextBox::EmitTextChanged ()
{
	Emit (TextChangedEvent, new TextChangedEventArgs ());
}

void
TextBox::SyncSelectedText ()
{
	if (selection.length > 0) {
		char *text = g_ucs4_to_utf8 (buffer->text + selection.start, selection.length, NULL, NULL, NULL);
		
		setvalue = false;
		SetValue (TextBox::SelectedTextProperty, Value (text, true));
		setvalue = true;
	} else {
		setvalue = false;
		SetValue (TextBox::SelectedTextProperty, Value (""));
		setvalue = true;
	}
}

void
TextBox::SyncText ()
{
	char *text = g_ucs4_to_utf8 (buffer->text, buffer->len, NULL, NULL, NULL);
	
	setvalue = false;
	SetValue (TextBox::TextProperty, Value (text, true));
	setvalue = true;
}

void
TextBox::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	TextBoxModelChangeType changed = TextBoxModelChangedNothing;
	bool invalidate = false;
	
	if (args->property == Control::FontFamilyProperty) {
		FontFamily *family = args->new_value ? args->new_value->AsFontFamily () : NULL;
		changed = TextBoxModelChangedFont;
		font->SetFamily (family ? family->source : NULL);
	} else if (args->property == Control::FontSizeProperty) {
		double size = args->new_value->AsDouble ();
		changed = TextBoxModelChangedFont;
		font->SetSize (size);
	} else if (args->property == Control::FontStretchProperty) {
		FontStretches stretch = (FontStretches) args->new_value->AsInt32 ();
		changed = TextBoxModelChangedFont;
		font->SetStretch (stretch);
	} else if (args->property == Control::FontStyleProperty) {
		FontStyles style = (FontStyles) args->new_value->AsInt32 ();
		changed = TextBoxModelChangedFont;
		font->SetStyle (style);
	} else if (args->property == Control::FontWeightProperty) {
		FontWeights weight = (FontWeights) args->new_value->AsInt32 ();
		changed = TextBoxModelChangedFont;
		font->SetWeight (weight);
	} else if (args->property == TextBox::AcceptsReturnProperty) {
		// no rendering changes required
	} else if (args->property == TextBox::IsReadOnlyProperty) {
		changed = TextBoxModelChangedReadOnly;
	} else if (args->property == TextBox::MaxLengthProperty) {
		maxlen = args->new_value->AsInt32 ();
	} else if (args->property == TextBox::SelectedTextProperty) {
		if (setvalue) {
			const char *str = args->new_value ? args->new_value->AsString () : "";
			gunichar *text;
			glong textlen;
			
			// replace the currently selected text
			text = g_utf8_to_ucs4_fast (str, -1, &textlen);
			buffer->Replace (selection.start, selection.length, text, textlen);
			g_free (text);
			
			ClearSelection (selection.start + textlen);
			SyncText ();
		}
		
		emit |= SELECTION_CHANGED;
	} else if (args->property == TextBox::SelectionStartProperty) {
		selection.start = args->new_value->AsInt32 ();
		emit |= CURSOR_POSITION_CHANGED;
		
		// update SelectedText
		SyncSelectedText ();
	} else if (args->property == TextBox::SelectionLengthProperty) {
		selection.length = args->new_value->AsInt32 ();
		
		// update SelectedText
		SyncSelectedText ();
	} else if (args->property == TextBox::SelectionBackgroundProperty) {
		changed = TextBoxModelChangedBrush;
	} else if (args->property == TextBox::SelectionForegroundProperty) {
		changed = TextBoxModelChangedBrush;
	} else if (args->property == TextBox::TextProperty) {
		if (setvalue) {
			const char *str = args->new_value ? args->new_value->AsString () : "";
			gunichar *text;
			glong textlen;
			
			text = g_utf8_to_ucs4_fast (str, -1, &textlen);
			buffer->Replace (0, buffer->len, text, textlen);
			g_free (text);
			
			ClearSelection (0);
		}
		
		emit |= TEXT_CHANGED;
	} else if (args->property == TextBox::TextAlignmentProperty) {
		changed = TextBoxModelChangedTextAlignment;
	} else if (args->property == TextBox::TextWrappingProperty) {
		changed = TextBoxModelChangedTextWrapping;
	} else if (args->property == TextBox::HorizontalScrollBarVisibilityProperty) {
		invalidate = true;
	} else if (args->property == TextBox::VerticalScrollBarVisibilityProperty) {
		invalidate = true;
	}
	
	if (invalidate)
		Invalidate ();
	
	if (changed != TextBoxModelChangedNothing)
		Emit (ModelChangedEvent, new TextBoxModelChangedEventArgs (changed, args));
	
	if (args->property->GetOwnerType () != Type::TEXTBOX) {
		Control::OnPropertyChanged (args);
		return;
	}
	
	NotifyListenersOfPropertyChange (args);
}

void
TextBox::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == TextBox::SelectionBackgroundProperty ||
	    prop == TextBox::SelectionForegroundProperty ||
	    prop == Control::BackgroundProperty ||
	    prop == Control::ForegroundProperty) {
		Emit (ModelChangedEvent, new TextBoxModelChangedEventArgs (TextBoxModelChangedBrush));
		Invalidate ();
	}
	
	if (prop->GetOwnerType () != Type::TEXTBOX)
		Control::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
TextBox::OnApplyTemplate ()
{
	DependencyObject *content = GetTemplateChild ("ContentElement");
	ContentControl *control;
	TextBoxView *view;
	
	if (content->Is (Type::CONTENTCONTROL)) {
		// Insert our TextBoxView
		control = (ContentControl *) content;
		view = new TextBoxView ();
		view->SetTextBox (this);
		control->SetValue (ContentControl::ContentProperty, Value (view));
	}
	
	Control::OnApplyTemplate ();
}

void
TextBox::ClearSelection (int start)
{
	cursor = SELECTION_BEGIN;
	SetSelectionStart (start);
	SetSelectionLength (0);
	SyncSelectedText ();
}

void
TextBox::Select (int start, int length)
{
	if ((start < 0) || (length < 0))
		return;
	
	if (start > buffer->len)
		start = buffer->len;
	
	if (length > (buffer->len - start))
		length = (buffer->len - start);
	
	cursor = SELECTION_BEGIN;
	SetSelectionStart (start);
	SetSelectionLength (length);
	SyncSelectedText ();
}

bool
TextBox::SelectAll ()
{
	if (selection.start == 0 && selection.length == buffer->len)
		return false;
	
	Select (0, buffer->len);
	
	return true;
}


//
// TextBoxView
//

#define CURSOR_ON_MULTIPLIER 2
#define CURSOR_OFF_MULTIPLIER 1
#define CURSOR_DELAY_MULTIPLIER 3
#define CURSOR_DIVIDER 3

TextBoxView::TextBoxView ()
{
	SetObjectType (Type::TEXTBOXVIEW);
	
	AddHandler (UIElement::LostFocusEvent, TextBoxView::focus_out, this);
	AddHandler (UIElement::GotFocusEvent, TextBoxView::focus_in, this);
	AddHandler (UIElement::KeyDownEvent, TextBoxView::key_down, this);
	AddHandler (UIElement::KeyUpEvent, TextBoxView::key_up, this);
	
	cursor = Rect (0, 0, 0, 0);
	layout = new TextLayout ();
	cursor_visible = false;
	blink_timeout = 0;
	textbox = NULL;
	focused = false;
	dirty = false;
}

TextBoxView::~TextBoxView ()
{
	if (textbox) {
		textbox->RemoveHandler (TextBox::SelectionChangedEvent, TextBoxView::selection_changed, this);
		textbox->RemoveHandler (TextBox::ModelChangedEvent, TextBoxView::model_changed, this);
		textbox->RemoveHandler (TextBox::TextChangedEvent, TextBoxView::text_changed, this);
	}
	
	RemoveHandler (UIElement::LostFocusEvent, TextBoxView::focus_out, this);
	RemoveHandler (UIElement::GotFocusEvent, TextBoxView::focus_in, this);
	RemoveHandler (UIElement::KeyDownEvent, TextBoxView::key_down, this);
	RemoveHandler (UIElement::KeyUpEvent, TextBoxView::key_up, this);
	
	DisconnectBlinkTimeout ();
	
	delete layout;
}

void
TextBoxView::OnKeyDown (KeyEventArgs *args)
{
	GdkModifierType modifiers = (GdkModifierType) args->GetModifiers ();
	guint key = args->GetKeyVal ();
	gunichar c;
	
	if (args->IsModifier ())
		return;
	
	printf ("TextBoxView::OnKeyDown()\n");
	
	// freeze TextBox event emission
	textbox->KeyPressFreeze ();
	
	if ((c = args->GetUnicode ())) {
		textbox->KeyPressUnichar (c);
		args->SetHandled (true);
	} else {
		// special key
		switch (key) {
		case GDK_Return:
			textbox->KeyPressUnichar ('\r');
			args->SetHandled (true);
			break;
		case GDK_BackSpace:
			textbox->KeyPressBackSpace (modifiers);
			args->SetHandled (true);
			break;
		case GDK_Delete:
			textbox->KeyPressDelete (modifiers);
			args->SetHandled (true);
			break;
		case GDK_KP_Page_Down:
		case GDK_Page_Down:
			textbox->KeyPressPageDown (modifiers);
			break;
		case GDK_KP_Page_Up:
		case GDK_Page_Up:
			textbox->KeyPressPageUp (modifiers);
			break;
		case GDK_KP_Home:
		case GDK_Home:
			textbox->KeyPressHome (modifiers);
			break;
		case GDK_KP_End:
		case GDK_End:
			textbox->KeyPressEnd (modifiers);
			break;
		case GDK_KP_Right:
		case GDK_Right:
			textbox->KeyPressRight (modifiers);
			break;
		case GDK_KP_Left:
		case GDK_Left:
			textbox->KeyPressLeft (modifiers);
			break;
		case GDK_KP_Down:
		case GDK_Down:
			textbox->KeyPressDown (modifiers);
			break;
		case GDK_KP_Up:
		case GDK_Up:
			textbox->KeyPressUp (modifiers);
			break;
		case GDK_A:
		case GDK_a:
			if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) == GDK_CONTROL_MASK) {
				// select all
				args->SetHandled (true);
				textbox->SelectAll ();
			}
			break;
		case GDK_C:
		case GDK_c:
			if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) == GDK_CONTROL_MASK) {
				// copy selection to the clipboard
				// FIXME: implement me
				args->SetHandled (true);
			}
			break;
		case GDK_X:
		case GDK_x:
			if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) == GDK_CONTROL_MASK) {
				// copy selection to the clipboard and then cut
				// FIXME: implement me
				args->SetHandled (true);
			}
			break;
		case GDK_V:
		case GDK_v:
			if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) == GDK_CONTROL_MASK) {
				// paste clipboard contents to the buffer
				// FIXME: implement me
				args->SetHandled (true);
			}
			break;
		case GDK_Y:
		case GDK_y:
			if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK)) == GDK_CONTROL_MASK) {
				// Ctrl+Y := Redo
				args->SetHandled (true);
			}
			break;
		case GDK_Z:
		case GDK_z:
			if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK)) == GDK_CONTROL_MASK) {
				// Ctrl+Z := Undo
				args->SetHandled (true);
			}
			break;
		default:
			// FIXME: what other keys do we need to handle?
			break;
		}
		
		// FIXME: some of these may also require updating scrollbars?
	}
	
	// thaw textbox keypress, causes Text and SelectedText to be
	// sync'd and events to be emitted
	textbox->KeyPressThaw ();
	
	// FIXME: register a key repeat timeout?
}

void
TextBoxView::OnKeyUp (KeyEventArgs *args)
{
	// FIXME: unregister the key repeat timeout?
}

void
TextBoxView::key_down (EventObject *sender, EventArgs *args, void *closure)
{
	((TextBoxView *) closure)->OnKeyDown ((KeyEventArgs *) args);
}

void
TextBoxView::key_up (EventObject *sender, EventArgs *args, void *closure)
{
	((TextBoxView *) closure)->OnKeyUp ((KeyEventArgs *) args);
}

gboolean
TextBoxView::blink (void *user_data)
{
	return ((TextBoxView *) user_data)->Blink ();
}

void
TextBoxView::ConnectBlinkTimeout (guint multiplier)
{
	blink_timeout = g_timeout_add (1000 * multiplier / CURSOR_DIVIDER, TextBoxView::blink, this);
}

void
TextBoxView::DisconnectBlinkTimeout ()
{
	if (blink_timeout != 0) {
		g_source_remove (blink_timeout);
		blink_timeout = 0;
	}
}

bool
TextBoxView::Blink ()
{
	guint multiplier;
	
	if (cursor_visible) {
		multiplier = CURSOR_OFF_MULTIPLIER;
		HideCursor ();
	} else {
		multiplier = CURSOR_ON_MULTIPLIER;
		ShowCursor ();
	}
	
	ConnectBlinkTimeout (multiplier);
	
	return false;
}

void
TextBoxView::DelayCursorBlink ()
{
	if (textbox->GetSelection ()->length == 0) {
		DisconnectBlinkTimeout ();
		ConnectBlinkTimeout (CURSOR_DELAY_MULTIPLIER);
		ShowCursor ();
	} else {
		// temporarily disable blinking during selection
		DisconnectBlinkTimeout ();
		cursor_visible = true;
	}
}

void
TextBoxView::BeginCursorBlink ()
{
	if (textbox->GetSelection ()->length == 0) {
		// no selection, proceed with blinking
		if (blink_timeout == 0) {
			ConnectBlinkTimeout (CURSOR_ON_MULTIPLIER);
			ShowCursor ();
		}
	} else {
		// temporarily disable blinking during selection
		DisconnectBlinkTimeout ();
		cursor_visible = true;
	}
}

void
TextBoxView::EndCursorBlink ()
{
	DisconnectBlinkTimeout ();
	
	if (cursor_visible)
		HideCursor ();
}

void
TextBoxView::ShowCursor ()
{
	cursor_visible = true;
	Invalidate (cursor);
}

void
TextBoxView::HideCursor ()
{
	cursor_visible = false;
	Invalidate (cursor);
}

void
TextBoxView::UpdateCursor (bool invalidate)
{
	int cur = textbox->GetCursor ();
	
	// invalidate current cursor rect
	if (invalidate && cursor_visible)
		Invalidate (cursor);
	
	// calculate the new cursor rect
	if (cur == 0) {
		// Manually set cursor rect if position is 0 because
		// we might not have any text runs (which the layout
		// code requires to get font metrics).
		TextFont *font = textbox->FontDescription ()->GetFont ();
		cursor = Rect (0.0, 0.0, 1.0, font->Ascender ());
		font->unref ();
	} else {
		cursor = layout->GetCursor (Point (), cur);
	}
	
	// invalidate the new cursor rect
	if (invalidate && cursor_visible)
		Invalidate (cursor);
}

void
TextBoxView::Render (cairo_t *cr, Region *region)
{
	if (dirty)
		Layout (cr);
	
	printf ("TextBoxView::Render()\n");
	
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	Paint (cr);
	cairo_restore (cr);
}

void
TextBoxView::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	*height = GetActualHeight ();
	*width = GetActualWidth ();
}

static void
append_runs (ITextSource *textbox, List *runs, const gunichar **text, int *length, bool selected)
{
	register const gunichar *inptr = *text;
	const gunichar *inend = inptr + *length;
	const gunichar *start = inptr;
	TextRun *run;
	int n;
	
	while (inptr < inend) {
		start = inptr;
		n = 0;
		
		while (inptr < inend && *inptr != '\r' && *inptr != '\n') {
			inptr++;
			n++;
		}
		
		// append the Run
		run = new TextRun (start, n, textbox, selected);
		runs->Append (run);
		
		if (inptr == inend) {
			(*length) -= n;
			break;
		}
		
		// append the LineBreak
		if (inptr[0] == '\r' && inptr[1] == '\n') {
			run = new TextRun (textbox, 2);
			inptr += 2;
			n += 2;
		} else {
			run = new TextRun (textbox, 1);
			inptr++;
			n++;
		}
		
		(*length) -= n;
	}
	
	*text = inptr;
}

void
TextBoxView::Layout (cairo_t *cr)
{
	double width = GetWidth ();
	TextSelection *selection;
	const gunichar *text;
	TextBuffer *buffer;
	List *runs;
	int left;
	
	if (width > 0.0f)
		layout->SetMaxWidth (width);
	else
		layout->SetMaxWidth (-1.0);
	
	selection = textbox->GetSelection ();
	buffer = textbox->GetBuffer ();
	text = buffer->text;
	
	runs = new List ();
	
	if (selection->length > 0) {
		left = selection->start;
		
		if (left > 0) {
			// add text before the selected region
			append_runs ((ITextSource *) textbox, runs, &text, &left, false);
		}
		
		// add the selected region of text
		left += selection->length;
		append_runs ((ITextSource *) textbox, runs, &text, &left, true);
		
		left += buffer->len - (text - buffer->text);
	} else {
		left = buffer->len;
	}
	
	// add the text after the selected region
	append_runs ((ITextSource *) textbox, runs, &text, &left, false);
	
	layout->SetTextRuns (runs);
	layout->Layout ();
	
	UpdateCursor (false);
	
	dirty = false;
}

void
TextBoxView::Paint (cairo_t *cr)
{
	Brush *fg;
	
	printf ("TextBoxView::Paint()\n");
	
	layout->Render (cr, GetOriginPoint (), Point ());
	
	if (cursor_visible && (fg = textbox->GetForeground ())) {
		fg->SetupBrush (cr, cursor);
		cairo_rectangle (cr, cursor.x, cursor.y, cursor.width, cursor.height);
		fg->Fill (cr);
	}
}

void
TextBoxView::selection_changed (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBoxView *) closure)->OnSelectionChanged ((RoutedEventArgs *) args);
}

void
TextBoxView::OnSelectionChanged (RoutedEventArgs *args)
{
	// the selection has changed, need to recalculate layout
	UpdateBounds (true);
	Invalidate ();
	dirty = true;
}

void
TextBoxView::text_changed (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBoxView *) closure)->OnTextChanged ((TextChangedEventArgs *) args);
}

void
TextBoxView::OnTextChanged (TextChangedEventArgs *args)
{
	// text has changed, need to recalculate layout
	UpdateBounds (true);
	Invalidate ();
	dirty = true;
}

void
TextBoxView::model_changed (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBoxView *) closure)->OnModelChanged ((TextBoxModelChangedEventArgs *) args);
}

void
TextBoxView::OnModelChanged (TextBoxModelChangedEventArgs *args)
{
	switch (args->changed) {
	case TextBoxModelChangedCursorPosition:
		// cursor position changed, just need to re-render
		if (focused)
			DelayCursorBlink ();
		
		UpdateCursor (true);
		return;
	case TextBoxModelChangedTextAlignment:
		// text alignment changed, update our layout
		dirty = layout->SetTextAlignment ((TextAlignment) args->property->new_value->AsInt32 ());
		break;
	case TextBoxModelChangedTextWrapping:
		// text wrapping changed, update our layout
		dirty = layout->SetTextWrapping ((TextWrapping) args->property->new_value->AsInt32 ());
		break;
	case TextBoxModelChangedReadOnly:
		if (focused) {
			if (args->property->new_value->AsBool ())
				BeginCursorBlink ();
			else
				EndCursorBlink ();
		}
		break;
	case TextBoxModelChangedBrush:
		// a brush has changed, no layout updates needed, we just need to re-render
		break;
	case TextBoxModelChangedFont:
		// font changed, need to recalculate layout
		dirty = true;
		break;
	default:
		// nothing changed??
		return;
	}
	
	if (dirty)
		UpdateBounds (true);
	
	Invalidate ();
}

void
TextBoxView::focus_out (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBoxView *) closure)->OnFocusOut (args);
}

void
TextBoxView::OnFocusOut (EventArgs *args)
{
	EndCursorBlink ();
	focused = false;
}

void
TextBoxView::focus_in (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBoxView *) closure)->OnFocusIn (args);
}

void
TextBoxView::OnFocusIn (EventArgs *args)
{
	BeginCursorBlink ();
	focused = true;
}

void
TextBoxView::SetTextBox (TextBox *textbox)
{
	if (this->textbox == textbox)
		return;
	
	if (this->textbox) {
		// remove the event handlers from the old textbox
		this->textbox->RemoveHandler (TextBox::SelectionChangedEvent, TextBoxView::selection_changed, this);
		this->textbox->RemoveHandler (TextBox::ModelChangedEvent, TextBoxView::model_changed, this);
		this->textbox->RemoveHandler (TextBox::TextChangedEvent, TextBoxView::text_changed, this);
	}
	
	if (textbox) {
		textbox->AddHandler (TextBox::SelectionChangedEvent, TextBoxView::selection_changed, this);
		textbox->AddHandler (TextBox::ModelChangedEvent, TextBoxView::model_changed, this);
		textbox->AddHandler (TextBox::TextChangedEvent, TextBoxView::text_changed, this);
		
		// sync our layout hints state
		layout->SetTextAlignment (textbox->GetTextAlignment ());
		layout->SetTextWrapping (textbox->GetTextWrapping ());
	}
	
	this->textbox = textbox;
	
	UpdateBounds (true);
	Invalidate ();
	dirty = true;
}


//
// PasswordBox
//

PasswordBox::PasswordBox ()
{
	SetObjectType (Type::PASSWORDBOX);
	ManagedTypeInfo *type_info = new ManagedTypeInfo ();
	type_info->assembly_name = g_strdup ("System.Windows");
	type_info->full_name = g_strdup ("System.Windows.Controls.PasswordBox");

	SetDefaultStyleKey (type_info);
}

void
PasswordBox::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property == PasswordBox::PasswordProperty)
		Emit (PasswordBox::PasswordChangedEvent);
	
	TextBox::OnPropertyChanged (args);	
}
