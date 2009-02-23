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
#include "runtime.h"
#include "textbox.h"
#include "clock.h"
#include "utils.h"



//
// TextBuffer
//

#define UNICODE_LEN(size) (sizeof (gunichar) * (size))
#define UNICODE_OFFSET(buf,offset) (((char *) buf) + UNICODE_LEN (offset))

class TextBuffer {
	int allocated;
	
	bool Resize (int needed)
	{
		bool resize = false;
		void *buf;
		
		if (allocated >= needed + 128) {
			while (allocated >= needed + 128)
				allocated -= 128;
			resize = true;
		} else if (allocated < needed) {
			while (allocated < needed)
				allocated += 128;
			resize = true;
		}
		
		if (resize) {
			if (!(buf = g_try_realloc (text, UNICODE_LEN (allocated))))
				return false;
			
			text = (gunichar *) buf;
		}
		
		return true;
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
	
	void Print ()
	{
		printf ("TextBuffer::text = \"");
		
		for (int i = 0; i < len; i++) {
			switch (text[i]) {
			case '\r':
				fputs ("\\r", stdout);
				break;
			case '\n':
				fputs ("\\n", stdout);
				break;
			case '\0':
				fputs ("\\0", stdout);
				break;
			case '\\':
				fputc ('\\', stdout);
				// fall thru
			default:
				fputc ((char) text[i], stdout);
				break;
			}
		}
		
		printf ("\";\n");
	}
	
	void Append (gunichar c)
	{
		if (!Resize (len + 2))
			return;
		
		text[len++] = c;
		text[len] = 0;
	}
	
	void Append (const gunichar *str, int count)
	{
		if (!Resize (len + count + 1))
			return;
		
		memcpy (UNICODE_OFFSET (text, len), str, UNICODE_LEN (count));
		len += count;
		text[len] = 0;
	}
	
	void Cut (int start, int length)
	{
		char *dest, *src;
		int beyond;
		
		if (length == 0 || start >= len)
			return;
		
		if (start + length > len)
			length = len - start;
		
		src = UNICODE_OFFSET (text, start + length);
		dest = UNICODE_OFFSET (text, start);
		beyond = len - (start + length);
		
		memmove (dest, src, UNICODE_LEN (beyond + 1));
		len -= length;
	}
	
	void Insert (int index, gunichar c)
	{
		if (!Resize (len + 2))
			return;
		
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
		if (!Resize (len + count + 1))
			return;
		
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
		if (!Resize (len + 2))
			return;
		
		// shift the entire buffer down by 1 char
		memmove (UNICODE_OFFSET (text, 1), text, UNICODE_LEN (len + 1));
		text[0] = c;
		len++;
	}
	
	void Prepend (const gunichar *str, int count)
	{
		if (!Resize (len + count + 1))
			return;
		
		// shift the endtire buffer down by @count chars
		memmove (UNICODE_OFFSET (text, count), text, UNICODE_LEN (len + 1));
		
		// copy @count chars of @str into the beginning of our buffer
		memcpy (text, str, UNICODE_LEN (count));
		len += count;
	}
	
	void Replace (int start, int length, const gunichar *str, int count)
	{
		char *dest, *src;
		int beyond;
		
		if (start > len)
			return;
		
		if (start + length > len)
			length = len - start;
		
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
		
		if (!Resize ((len - length) + count + 1))
			return;
		
		// calculate the number of chars beyond @start that won't be cut
		beyond = len - (start + length);
		
		// shift all chars beyond position (@start + length) into position...
		dest = UNICODE_OFFSET (text, start + count);
		src = UNICODE_OFFSET (text, start + length);
		memmove (dest, src, UNICODE_LEN (beyond + 1));
		
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
#define SELECTION_CHANGED       (1 << 0)
#define TEXT_CHANGED            (1 << 1)

TextBox::TextBox ()
{
	SetObjectType (Type::TEXTBOX);
	ManagedTypeInfo *type_info = new ManagedTypeInfo ();
	type_info->assembly_name = g_strdup ("System.Windows");
	type_info->full_name = g_strdup ("System.Windows.Controls.TextBox");
	
	SetDefaultStyleKey (type_info);
	
	AddHandler (UIElement::MouseLeftButtonDownEvent, TextBox::mouse_left_button_down, this);
	AddHandler (UIElement::MouseLeftButtonUpEvent, TextBox::mouse_left_button_up, this);
	AddHandler (UIElement::MouseEnterEvent, TextBox::mouse_enter, this);
	AddHandler (UIElement::MouseLeaveEvent, TextBox::mouse_leave, this);
	AddHandler (UIElement::MouseMoveEvent, TextBox::mouse_move, this);
	AddHandler (UIElement::LostFocusEvent, TextBox::focus_out, this);
	AddHandler (UIElement::GotFocusEvent, TextBox::focus_in, this);
	AddHandler (UIElement::KeyDownEvent, TextBox::key_down, this);
	AddHandler (UIElement::KeyUpEvent, TextBox::key_up, this);
	
	font = new TextFontDescription ();
	font->SetFamily (CONTROL_FONT_FAMILY);
	font->SetStretch (CONTROL_FONT_STRETCH);
	font->SetWeight (CONTROL_FONT_WEIGHT);
	font->SetStyle (CONTROL_FONT_STYLE);
	font->SetSize (CONTROL_FONT_SIZE);
	
	contentElement = NULL;
	
	buffer = new TextBuffer ();
	
	emit = NOTHING_CHANGED;
	selection_anchor = 0;
	selection_cursor = 0;
	cursor_column = 0;
	inkeypress = false;
	selecting = false;
	setvalue = true;
	captured = false;
	focused = false;
	view = NULL;
}

TextBox::~TextBox ()
{
	RemoveHandler (UIElement::MouseLeftButtonDownEvent, TextBox::mouse_left_button_down, this);
	RemoveHandler (UIElement::MouseLeftButtonUpEvent, TextBox::mouse_left_button_up, this);
	RemoveHandler (UIElement::MouseEnterEvent, TextBox::mouse_enter, this);
	RemoveHandler (UIElement::MouseLeaveEvent, TextBox::mouse_leave, this);
	RemoveHandler (UIElement::MouseMoveEvent, TextBox::mouse_move, this);
	RemoveHandler (UIElement::LostFocusEvent, TextBox::focus_out, this);
	RemoveHandler (UIElement::GotFocusEvent, TextBox::focus_in, this);
	RemoveHandler (UIElement::KeyDownEvent, TextBox::key_down, this);
	RemoveHandler (UIElement::KeyUpEvent, TextBox::key_up, this);
	
	delete buffer;
	delete font;
}

#define ALT_MASK (GDK_MOD1_MASK | GDK_MOD2_MASK | GDK_MOD3_MASK | GDK_MOD4_MASK | GDK_MOD5_MASK)
#define CONTROL_MASK GDK_CONTROL_MASK
#define SHIFT_MASK GDK_SHIFT_MASK

#define IsEOL(c) ((c) == '\r' || (c) == '\n')

int
TextBox::CursorDown (int cursor, int n_lines)
{
	int line_start = CursorLineBegin (cursor);
	int cur, n = 0;
	
	// first find out what our character offset is in the current line
	if (cursor_column == -1)
		cursor_column = cursor - line_start;
	
	cur = cursor;
	
	// skip ahead the number of requested lines
	while (n < n_lines) {
		while (cur < buffer->len && !IsEOL (buffer->text[cur]))
			cur++;
		
		if (cur == buffer->len)
			break;
		
		if (buffer->text[cur] == '\r' && buffer->text[cur + 1] == '\n')
			cur += 2;
		else
			cur++;
		
		line_start = cur;
		n++;
	}
	
	// if we are doing a PageDown and didn't manage to go down any
	// lines, go to the end of the buffer and update our
	// cursor_column as well.
	if (n == 0 && n_lines > 1) {
		cursor_column = cur - line_start;
		return cur;
	}
	
	// go forward until we're at the same character offset
	cur = line_start;
	for (n = 0; n < cursor_column && cur < buffer->len; n++) {
		if (IsEOL (buffer->text[cur]))
			break;
		cur++;
	}
	
	return cur;
}

int
TextBox::CursorUp (int cursor, int n_lines)
{
	int line_start = CursorLineBegin (cursor);
	int cur, n = 0;
	
	// first find out what our character offset is in the current line
	if (cursor_column == -1)
		cursor_column = cursor - line_start;
	
	cur = line_start;
	
	// go back the number of requested lines
	do {
		if (cur == 0)
			break;
		
		if (cur >= 2 && buffer->text[cur - 2] == '\r' && buffer->text[cur - 1] == '\n')
			cur -= 2;
		else
			cur--;
		
		n++;
		
		while (cur > 0 && !IsEOL (buffer->text[cur - 1]))
			cur--;
		
		line_start = cur;
	} while (n < n_lines);
	
	// if we are doing a PageUp and didn't manage to go up any
	// lines, go to position 0 and update our cursor_column as
	// well.
	if (n == 0 && n_lines > 1) {
		cursor_column = 0;
		return 0;
	}
	
	// go forward until we're at the same character column
	cur = line_start;
	for (n = 0; n < cursor_column && cur < buffer->len; n++) {
		if (IsEOL (buffer->text[cur]))
			break;
		cur++;
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

int
TextBox::CursorNextWord (int cursor)
{
	int i, lf, cr;
	CharClass cc;
	
	// find the end of the current line
	cr = CursorLineEnd (cursor);
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

int
TextBox::CursorPrevWord (int cursor)
{
	int begin, i, cr, lf;
	CharClass cc;
	
	// find the beginning of the current line
	lf = CursorLineBegin (cursor) - 1;
	
	if (lf > 0 && buffer->text[lf] == '\n' && buffer->text[lf - 1] == '\r')
		cr = lf - 1;
	else
		cr = lf;
	
	// if the cursor is at the beginning of the line, return the end of the prev line
	if (cursor - 1 == lf) {
		if (cr > 0)
			return cr;
		
		return 0;
	}
	
	cc = char_class (buffer->text[cursor - 1]);
	begin = lf + 1;
	i = cursor;
	
	// skip over the word, punctuation, or run of whitespace
	while (i > begin && char_class (buffer->text[i - 1]) == cc)
		i--;
	
	// if the cursor was at whitespace, skip back a word too
	if (cc == CharClassWhitespace && i > begin) {
		cc = char_class (buffer->text[i - 1]);
		while (i > begin && char_class (buffer->text[i - 1]) == cc)
			i--;
	}
	
	return i;
}

int
TextBox::CursorLineBegin (int cursor)
{
	int cur = cursor;
	
	// find the beginning of the line
	while (cur > 0 && !IsEOL (buffer->text[cur - 1]))
		cur--;
	
	return cur;
}

int
TextBox::CursorLineEnd (int cursor, bool include)
{
	int cur = cursor;
	
	// find the end of the line
	while (cur < buffer->len && !IsEOL (buffer->text[cur]))
		cur++;
	
	if (include && cur < buffer->len) {
		if (buffer->text[cur] == '\r' && buffer->text[cur + 1] == '\n')
			cur += 2;
		else
			cur++;
	}
	
	return cur;
}

void
TextBox::KeyPressBackSpace (GdkModifierType modifiers)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	int cur;
	
	if ((modifiers & (ALT_MASK | SHIFT_MASK)) != 0)
		return;
	
	if (cursor != anchor) {
		// BackSpace w/ active selection: delete the selected text
		int length = abs (cursor - anchor);
		int start = MIN (anchor, cursor);
		
		buffer->Cut (start, length);
		emit |= TEXT_CHANGED;
		anchor = start;
		cursor = start;
	} else if ((modifiers & CONTROL_MASK) != 0) {
		// Ctrl+BackSpace: delete the word ending at the cursor
		cur = CursorPrevWord (cursor);
		
		if (cur < cursor) {
			buffer->Cut (cur, cursor - cur);
			emit |= TEXT_CHANGED;
		        anchor = cur;
			cursor = cur;
		}
	} else if (cursor > 0) {
		// BackSpace: delete the char before the cursor position
		if (cursor >= 2 && buffer->text[cursor - 1] == '\n' && buffer->text[cursor - 2] == '\r') {
			buffer->Cut (cursor - 2, 2);
			cursor -= 2;
		} else {
			buffer->Cut (cursor - 1, 1);
			cursor--;
		}
		
		emit |= TEXT_CHANGED;
		anchor = cursor;
	}
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressDelete (GdkModifierType modifiers)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	int cur;
	
	if ((modifiers & (ALT_MASK | SHIFT_MASK)) != 0)
		return;
	
	if (cursor != anchor) {
		// Delete w/ active selection: delete the selected text
		int length = abs (cursor - anchor);
		int start = MIN (anchor, cursor);
		
		buffer->Cut (start, length);
		emit |= TEXT_CHANGED;
		anchor = start;
		cursor = start;
	} else if ((modifiers & CONTROL_MASK) != 0) {
		// Ctrl+Delete: delete the word starting at the cursor
		cur = CursorNextWord (cursor);
		
		if (cur > cursor) {
			buffer->Cut (cursor, cur - cursor);
			emit |= TEXT_CHANGED;
		}
	} else if (cursor < buffer->len) {
		// Delete: delete the char after the cursor position
		if (buffer->text[cursor] == '\r' && buffer->text[cursor + 1] == '\n')
			buffer->Cut (cursor, 2);
		else
			buffer->Cut (cursor, 1);
		
		emit |= TEXT_CHANGED;
	}
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressPageDown (GdkModifierType modifiers)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	int column;
	
	if ((modifiers & (CONTROL_MASK | ALT_MASK)) != 0)
		return;
	
	// move the cursor down one page from its current position
	cursor = CursorDown (cursor, 8);
	column = cursor_column;
	
	if ((modifiers & SHIFT_MASK) == 0) {
		// clobber the selection
		anchor = cursor;
	}
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
		cursor_column = column;
	}
}

void
TextBox::KeyPressPageUp (GdkModifierType modifiers)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	int column;
	
	if ((modifiers & (CONTROL_MASK | ALT_MASK)) != 0)
		return;
	
	// move the cursor up one page from its current position
	cursor = CursorUp (cursor, 8);
	column = cursor_column;
	
	if ((modifiers & SHIFT_MASK) == 0) {
		// clobber the selection
		anchor = cursor;
	}
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
		cursor_column = column;
	}
}

void
TextBox::KeyPressDown (GdkModifierType modifiers)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	int column;
	
	if ((modifiers & (CONTROL_MASK | ALT_MASK)) != 0)
		return;
	
	// move the cursor down by one line from its current position
	cursor = CursorDown (cursor, 1);
	column = cursor_column;
	
	if ((modifiers & SHIFT_MASK) == 0) {
		// clobber the selection
		anchor = cursor;
	}
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
		cursor_column = column;
	}
}

void
TextBox::KeyPressUp (GdkModifierType modifiers)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	int column;
	
	if ((modifiers & (CONTROL_MASK | ALT_MASK)) != 0)
		return;
	
	// move the cursor up by one line from its current position
	cursor = CursorUp (cursor, 1);
	column = cursor_column;
	
	if ((modifiers & SHIFT_MASK) == 0) {
		// clobber the selection
		anchor = cursor;
	}
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
		cursor_column = column;
	}
}

void
TextBox::KeyPressHome (GdkModifierType modifiers)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	
	if ((modifiers & ALT_MASK) != 0)
		return;
	
	if ((modifiers & CONTROL_MASK) != 0) {
		// move the cursor to the beginning of the buffer
		cursor = 0;
	} else {
		// move the cursor to the beginning of the line
		cursor = CursorLineBegin (cursor);
	}
	
	if ((modifiers & SHIFT_MASK) == 0) {
		// clobber the selection
		anchor = cursor;
	}
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
		cursor_column = 0;
	}
}

void
TextBox::KeyPressEnd (GdkModifierType modifiers)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	
	if ((modifiers & ALT_MASK) != 0)
		return;
	
	if ((modifiers & CONTROL_MASK) != 0) {
		// move the cursor to the end of the buffer
		cursor = buffer->len;
	} else {
		// move the cursor to the end of the line
		cursor = CursorLineEnd (cursor);
	}
	
	if ((modifiers & SHIFT_MASK) == 0) {
		// clobber the selection
		anchor = cursor;
	}
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressRight (GdkModifierType modifiers)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	
	if ((modifiers & ALT_MASK) != 0)
		return;
	
	if ((modifiers & CONTROL_MASK) != 0) {
		// move the cursor to beginning of the next word
		cursor = CursorNextWord (cursor);
	} else {
		// move the cursor forward one character
		if (buffer->text[cursor] == '\r' && buffer->text[cursor + 1] == '\n') 
			cursor += 2;
		else if (cursor < buffer->len)
			cursor++;
	}
	
	if ((modifiers & SHIFT_MASK) == 0) {
		// clobber the selection
		anchor = cursor;
	}
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressLeft (GdkModifierType modifiers)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	
	if ((modifiers & ALT_MASK) != 0)
		return;
	
	if ((modifiers & CONTROL_MASK) != 0) {
		// move the cursor to the beginning of the previous word
		cursor = CursorPrevWord (cursor);
	} else {
		// move the cursor backward one character
		if (cursor >= 2 && buffer->text[cursor - 2] == '\r' && buffer->text[cursor - 1] == '\n')
			cursor -= 2;
		else if (cursor > 0)
			cursor--;
	}
	
	if ((modifiers & SHIFT_MASK) == 0) {
		// clobber the selection
		anchor = cursor;
	}
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::KeyPressUnichar (gunichar c)
{
	int length = abs (selection_cursor - selection_anchor);
	int start = MIN (selection_anchor, selection_cursor);
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	int maxlen = GetMaxLength ();
	
	if ((maxlen > 0 && buffer->len >= maxlen) || ((c == '\r') && !GetAcceptsReturn ()))
		return;
	
	if (length > 0) {
		// replace the currently selected text
		buffer->Replace (start, length, &c, 1);
	} else {
		// insert the text at the cursor position
		buffer->Insert (start, c);
	}
	
	emit |= TEXT_CHANGED;
	cursor = start + 1;
	anchor = cursor;
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
	}
}

void
TextBox::SyncAndEmit ()
{
	if (emit & TEXT_CHANGED)
		SyncText ();
	
	if (emit & SELECTION_CHANGED)
		SyncSelectedText ();
	
	if (emit & TEXT_CHANGED)
		EmitTextChanged ();
	
	if (emit & SELECTION_CHANGED)
		EmitSelectionChanged ();
	
	emit = NOTHING_CHANGED;
}

static GtkClipboard *
GetClipboard (TextBox *textbox, GdkAtom atom)
{
	GdkDisplay *display;
	MoonWindow *window;
	GdkWindow *widget;
	Surface *surface;
	
	if (!(surface = textbox->GetSurface ()))
		return NULL;
	
	if (!(window = surface->GetWindow ()))
		return NULL;
	
	if (!(widget = window->GetGdkWindow ()))
		return NULL;
	
	if (!(display = gdk_drawable_get_display ((GdkDrawable *) widget)))
		return NULL;
	
	return gtk_clipboard_get_for_display (display, atom);
}

void
TextBox::paste (GtkClipboard *clipboard, const char *text, gpointer closure)
{
	TextBox *textbox = (TextBox *) closure;
	
	textbox->SetSelectedText (text);
}

void
TextBox::OnKeyDown (KeyEventArgs *args)
{
	GdkModifierType modifiers = (GdkModifierType) args->GetModifiers ();
	guint key = args->GetKeyVal ();
	GtkClipboard *clipboard;
	gunichar c;
	
	if (args->IsModifier ())
		return;
	
	// set 'emit' to NOTHING_CHANGED so that we can figure out
	// what has chanegd after applying the changes that this
	// keypress will cause.
	emit = NOTHING_CHANGED;
	inkeypress = true;
	
	switch (key) {
	case GDK_Return:
		if (!GetIsReadOnly ()) {
			args->SetHandled (true);
			KeyPressUnichar ('\r');
		}
		break;
	case GDK_BackSpace:
		if (!GetIsReadOnly ()) {
			KeyPressBackSpace (modifiers);
			args->SetHandled (true);
		}
		break;
	case GDK_Delete:
		if (!GetIsReadOnly ()) {
			KeyPressDelete (modifiers);
			args->SetHandled (true);
		}
		break;
	case GDK_KP_Page_Down:
	case GDK_Page_Down:
		KeyPressPageDown (modifiers);
		break;
	case GDK_KP_Page_Up:
	case GDK_Page_Up:
		KeyPressPageUp (modifiers);
		break;
	case GDK_KP_Home:
	case GDK_Home:
		KeyPressHome (modifiers);
		break;
	case GDK_KP_End:
	case GDK_End:
		KeyPressEnd (modifiers);
		break;
	case GDK_KP_Right:
	case GDK_Right:
		KeyPressRight (modifiers);
		break;
	case GDK_KP_Left:
	case GDK_Left:
		KeyPressLeft (modifiers);
		break;
	case GDK_KP_Down:
	case GDK_Down:
		KeyPressDown (modifiers);
		break;
	case GDK_KP_Up:
	case GDK_Up:
		KeyPressUp (modifiers);
		break;
	default:
		if ((modifiers & (CONTROL_MASK | ALT_MASK | SHIFT_MASK)) == CONTROL_MASK) {
			switch (key) {
			case GDK_A:
			case GDK_a:
				// Ctrl+A => Select All
				args->SetHandled (true);
				SelectAll ();
				break;
			case GDK_C:
			case GDK_c:
				// Ctrl+C => Copy
				if ((clipboard = GetClipboard (this, GDK_SELECTION_CLIPBOARD))) {
					// copy selection to the clipboard
					gtk_clipboard_set_text (clipboard, GetSelectedText (), -1);
					args->SetHandled (true);
				}
				break;
			case GDK_X:
			case GDK_x:
				// Ctrl+X => Cut
				if ((clipboard = GetClipboard (this, GDK_SELECTION_CLIPBOARD))) {
					// copy selection to the clipboard and then cut
					gtk_clipboard_set_text (clipboard, GetSelectedText (), -1);
					if (!GetIsReadOnly ())
						SetSelectedText ("");
					args->SetHandled (true);
				}
				break;
			case GDK_V:
			case GDK_v:
				// Ctrl+V => Paste
				if (!GetIsReadOnly () && (clipboard = GetClipboard (this, GDK_SELECTION_CLIPBOARD))) {
					// paste clipboard contents to the buffer
					gtk_clipboard_request_text (clipboard, TextBox::paste, this);
					args->SetHandled (true);
				}
				break;
			case GDK_Y:
			case GDK_y:
				// Ctrl+Y => Redo
				if (!GetIsReadOnly ()) {
					// FIXME: implement me
					args->SetHandled (true);
				}
				break;
			case GDK_Z:
			case GDK_z:
				// Ctrl+Z => Undo
				if (!GetIsReadOnly ()) {
					// FIXME: implement me
					args->SetHandled (true);
				}
				break;
			default:
				// unhandled Control commands
				break;
			}
		} else if ((modifiers & (CONTROL_MASK | ALT_MASK)) == 0) {
			// normal character input
			if ((c = args->GetUnicode ()) && !GetIsReadOnly ()) {
				args->SetHandled (true);
				KeyPressUnichar (c);
			}
		}
		break;
	}
	
	// FIXME: some of these may also require updating scrollbars?
	
	inkeypress = false;
	SyncAndEmit ();
	
	// FIXME: register a key repeat timeout?
}

void
TextBox::key_down (EventObject *sender, EventArgs *args, void *closure)
{
	((TextBox *) closure)->OnKeyDown ((KeyEventArgs *) args);
}

void
TextBox::OnKeyUp (KeyEventArgs *args)
{
	// FIXME: unregister the key repeat timeout?
}

void
TextBox::key_up (EventObject *sender, EventArgs *args, void *closure)
{
	((TextBox *) closure)->OnKeyUp ((KeyEventArgs *) args);
}

void
TextBox::OnMouseLeftButtonDown (MouseEventArgs *args)
{
	GdkEventButton *event = (GdkEventButton *) args->GetEvent ();
	int cursor, start, end;
	double x, y;
	
	if (view) {
		args->GetPosition (view, &x, &y);
		args->SetHandled (true);
		
		cursor = view->GetCursorFromXY (x, y);
		
		switch (event->type) {
		case GDK_3BUTTON_PRESS:
			// Note: Silverlight doesn't implement this, but to
			// be consistent with other TextEntry-type
			// widgets, we will.
			//
			// Triple-Click: select the line
			if (captured)
				ReleaseMouseCapture ();
			start = CursorLineBegin (cursor);
			end = CursorLineEnd (cursor, true);
			selecting = false;
			captured = false;
			break;
		case GDK_2BUTTON_PRESS:
			// Double-Click: select the word
			if (captured)
				ReleaseMouseCapture ();
			start = CursorPrevWord (cursor);
			end = CursorNextWord (cursor);
			selecting = false;
			captured = false;
			break;
		case GDK_BUTTON_PRESS:
		default:
			// Single-Click: cursor placement
			captured = CaptureMouse ();
			start = end = cursor;
			selecting = true;
			break;
		}
		
		emit = NOTHING_CHANGED;
		SetSelectionLength (end - start);
		SetSelectionStart (start);
		SyncAndEmit ();
	}
}

void
TextBox::mouse_left_button_down (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBox *) closure)->OnMouseLeftButtonDown ((MouseEventArgs *) args);
}

void
TextBox::OnMouseLeftButtonUp (MouseEventArgs *args)
{
	if (captured)
		ReleaseMouseCapture ();
	
	args->SetHandled (true);
	selecting = false;
	captured = false;
}

void
TextBox::mouse_left_button_up (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBox *) closure)->OnMouseLeftButtonUp ((MouseEventArgs *) args);
}

void
TextBox::OnMouseEnter (MouseEventArgs *args)
{
	//printf ("TextBox::OnMouseEnter()\n");
}

void
TextBox::mouse_enter (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBox *) closure)->OnMouseEnter ((MouseEventArgs *) args);
}

void
TextBox::OnMouseLeave (EventArgs *args)
{
	//printf ("TextBox::OnMouseLeave()\n");
}

void
TextBox::mouse_leave (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBox *) closure)->OnMouseLeave (args);
}

void
TextBox::OnMouseMove (MouseEventArgs *args)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	double x, y;
	
	if (selecting) {
		args->GetPosition (view, &x, &y);
		args->SetHandled (true);
		
		cursor = view->GetCursorFromXY (x, y);
		
		emit = NOTHING_CHANGED;
		SetSelectionLength (abs (cursor - anchor));
		SetSelectionStart (MIN (anchor, cursor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		SyncAndEmit ();
	}
}

void
TextBox::mouse_move (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBox *) closure)->OnMouseMove ((MouseEventArgs *) args);
}

void
TextBox::OnFocusOut (EventArgs *args)
{
	focused = false;
	
	if (view)
		view->OnFocusOut ();
}

void
TextBox::focus_out (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBox *) closure)->OnFocusOut (args);
}

void
TextBox::OnFocusIn (EventArgs *args)
{
	focused = true;
	
	if (view)
		view->OnFocusIn ();
}

void
TextBox::focus_in (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBox *) closure)->OnFocusIn (args);
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
	if (selection_cursor != selection_anchor) {
		int length = abs (selection_cursor - selection_anchor);
		int start = MIN (selection_anchor, selection_cursor);
		char *text;
		
		text = g_ucs4_to_utf8 (buffer->text + start, length, NULL, NULL, NULL);
		
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
	DependencyProperty *prop;
	bool invalidate = false;
	int start, length;
	
	if (args->GetId () == Control::FontFamilyProperty) {
		FontFamily *family = args->new_value ? args->new_value->AsFontFamily () : NULL;
		changed = TextBoxModelChangedFont;
		font->SetFamily (family ? family->source : NULL);
	} else if (args->GetId () == Control::FontSizeProperty) {
		double size = args->new_value->AsDouble ();
		changed = TextBoxModelChangedFont;
		font->SetSize (size);
	} else if (args->GetId () == Control::FontStretchProperty) {
		FontStretches stretch = (FontStretches) args->new_value->AsInt32 ();
		changed = TextBoxModelChangedFont;
		font->SetStretch (stretch);
	} else if (args->GetId () == Control::FontStyleProperty) {
		FontStyles style = (FontStyles) args->new_value->AsInt32 ();
		changed = TextBoxModelChangedFont;
		font->SetStyle (style);
	} else if (args->GetId () == Control::FontWeightProperty) {
		FontWeights weight = (FontWeights) args->new_value->AsInt32 ();
		changed = TextBoxModelChangedFont;
		font->SetWeight (weight);
	} else if (args->GetId () == TextBox::AcceptsReturnProperty) {
		// no state changes needed
	} else if (args->GetId () == TextBox::IsReadOnlyProperty) {
		// no state changes needed
	} else if (args->GetId () == TextBox::MaxLengthProperty) {
		// no state changes needed
	} else if (args->GetId () == TextBox::SelectedTextProperty) {
		if (setvalue) {
			const char *str = args->new_value && args->new_value->AsString () ? args->new_value->AsString () : "";
			gunichar *text;
			glong textlen;
			
			length = abs (selection_cursor - selection_anchor);
			start = MIN (selection_anchor, selection_cursor);
			
			// replace the currently selected text
			if ((text = g_utf8_to_ucs4_fast (str, -1, &textlen))) {
				buffer->Replace (start, length, text, textlen);
				emit |= TEXT_CHANGED;
				g_free (text);
				
				ClearSelection (start + textlen);
				
				if (!inkeypress)
					SyncText ();
			} else {
				g_warning ("g_utf8_to_ucs4_fast failed for string '%s'", str);
			}
		}
	} else if (args->GetId () == TextBox::SelectionStartProperty) {
		length = abs (selection_cursor - selection_anchor);
		start = args->new_value->AsInt32 ();
		
		// When set programatically, anchor is always the
		// start and cursor is always the end
		selection_cursor = start + length;
		selection_anchor = start;
		
		changed = TextBoxModelChangedSelection;
		emit |= SELECTION_CHANGED;
		cursor_column = -1;
		
		if (!inkeypress) {
			// update SelectedText
			SyncSelectedText ();
		}
	} else if (args->GetId () == TextBox::SelectionLengthProperty) {
		start = MIN (selection_anchor, selection_cursor);
		length = args->new_value->AsInt32 ();
		
		// When set programatically, anchor is always the
		// start and cursor is always the end
		selection_cursor = start + length;
		selection_anchor = start;
		
		changed = TextBoxModelChangedSelection;
		emit |= SELECTION_CHANGED;
		cursor_column = -1;
		
		// set some default selection brushes if unset
		if (selection_cursor != selection_anchor) {
			Brush *brush;
			
			if (!GetSelectionBackground ()) {
				brush = new SolidColorBrush ("#FF444444");
				SetSelectionBackground (brush);
				brush->unref ();
			}
			
			if (!GetSelectionForeground ()) {
				brush = new SolidColorBrush ("#FFFFFFFF");
				SetSelectionForeground (brush);
				brush->unref ();
			}
		}
		
		if (!inkeypress) {
			// update SelectedText
			SyncSelectedText ();
		}
	} else if (args->GetId () == TextBox::SelectionBackgroundProperty) {
		changed = TextBoxModelChangedBrush;
	} else if (args->GetId () == TextBox::SelectionForegroundProperty) {
		changed = TextBoxModelChangedBrush;
	} else if (args->GetId () == TextBox::TextProperty) {
		if (setvalue) {
			const char *str = args->new_value && args->new_value->AsString () ? args->new_value->AsString () : "";
			gunichar *text;
			glong textlen;
			
			if ((text = g_utf8_to_ucs4_fast (str, -1, &textlen))) {
				buffer->Replace (0, buffer->len, text, textlen);
				emit |= TEXT_CHANGED;
				g_free (text);
				
				ClearSelection (0);
			} else {
				g_warning ("g_utf8_to_ucs4_fast failed for string '%s'", str);
			}
		}
		
		changed = TextBoxModelChangedText;
	} else if (args->GetId () == TextBox::TextAlignmentProperty) {
		changed = TextBoxModelChangedTextAlignment;
	} else if (args->GetId () == TextBox::TextWrappingProperty) {
		changed = TextBoxModelChangedTextWrapping;
	} else if (args->GetId () == TextBox::HorizontalScrollBarVisibilityProperty) {
		invalidate = true;
		
		// XXX more crap because these aren't templatebound.
		if (contentElement) {
			if ((prop = contentElement->GetDependencyProperty ("VerticalScrollBarVisibility")))
				contentElement->SetValue (prop, GetValue (TextBox::VerticalScrollBarVisibilityProperty));
		}
	} else if (args->GetId () == TextBox::VerticalScrollBarVisibilityProperty) {
		invalidate = true;
		
		// XXX more crap because these aren't templatebound.
		if (contentElement) {
			if ((prop = contentElement->GetDependencyProperty ("HorizontalScrollBarVisibility")))
				contentElement->SetValue (prop, GetValue (TextBox::HorizontalScrollBarVisibilityProperty));
		}
	}
	
	if (invalidate)
		Invalidate ();
	
	if (changed != TextBoxModelChangedNothing)
		Emit (ModelChangedEvent, new TextBoxModelChangedEventArgs (changed, args));
	
	if (args->GetProperty ()->GetOwnerType () != Type::TEXTBOX) {
		Control::OnPropertyChanged (args);
		return;
	}
	
	NotifyListenersOfPropertyChange (args);
}

void
TextBox::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop && (prop->GetId () == TextBox::SelectionBackgroundProperty ||
	    prop->GetId () == TextBox::SelectionForegroundProperty ||
	    prop->GetId () == Control::BackgroundProperty ||
	    prop->GetId () == Control::ForegroundProperty)) {
		Emit (ModelChangedEvent, new TextBoxModelChangedEventArgs (TextBoxModelChangedBrush));
		Invalidate ();
	}
	
	if (prop->GetOwnerType () != Type::TEXTBOX)
		Control::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
TextBox::OnApplyTemplate ()
{
	DependencyProperty *prop;
	ContentControl *control;
	
	contentElement = GetTemplateChild ("ContentElement");
	
	if (contentElement->Is (Type::CONTENTCONTROL)) {
		// Insert our TextBoxView
		control = (ContentControl *) contentElement;
		view = new TextBoxView ();
		view->SetTextBox (this);
		control->SetValue (ContentControl::ContentProperty, Value (view));
	}
	
	// XXX LAME these should be template bindings in the textbox template.
	if ((prop = contentElement->GetDependencyProperty ("VerticalScrollBarVisibility")))
		contentElement->SetValue (prop, GetValue (TextBox::VerticalScrollBarVisibilityProperty));
	
	if ((prop = contentElement->GetDependencyProperty ("HorizontalScrollBarVisibility")))
		contentElement->SetValue (prop, GetValue (TextBox::HorizontalScrollBarVisibilityProperty));
	
	Control::OnApplyTemplate ();
}

void
TextBox::ClearSelection (int start)
{
	SetSelectionStart (start);
	SetSelectionLength (0);
	
	if (!inkeypress)
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
	
	SetSelectionStart (start);
	SetSelectionLength (length);
	
	if (!inkeypress)
		SyncSelectedText ();
}

void
TextBox::SelectAll ()
{
	Select (0, buffer->len);
}


//
// TextBoxView
//

#define CURSOR_BLINK_TIMEOUT_DEFAULT  900
#define CURSOR_BLINK_ON_MULTIPLIER    2
#define CURSOR_BLINK_OFF_MULTIPLIER   1
#define CURSOR_BLINK_DELAY_MULTIPLIER 3
#define CURSOR_BLINK_DIVIDER          3

TextBoxView::TextBoxView ()
{
	SetObjectType (Type::TEXTBOXVIEW);
	
	SetCursor (MouseCursorIBeam);
	
	cursor = Rect (0, 0, 0, 0);
	layout = new TextLayout ();
	had_selected_text = false;
	cursor_visible = false;
	blink_timeout = 0;
	textbox = NULL;
	dirty = false;
}

TextBoxView::~TextBoxView ()
{
	if (textbox) {
		textbox->RemoveHandler (TextBox::ModelChangedEvent, TextBoxView::model_changed, this);
		textbox->view = NULL;
	}
	
	DisconnectBlinkTimeout ();
	
	delete layout;
}

int
TextBoxView::GetCursorFromXY (double x, double y)
{
	return layout->GetCursorFromXY (Point (), x, y);
}

gboolean
TextBoxView::blink (void *user_data)
{
	return ((TextBoxView *) user_data)->Blink ();
}

static guint
GetCursorBlinkTimeout (TextBoxView *view)
{
	GtkSettings *settings;
	MoonWindow *window;
	GdkScreen *screen;
	GdkWindow *widget;
	Surface *surface;
	guint timeout;
	
	if (!(surface = view->GetSurface ()))
		return CURSOR_BLINK_TIMEOUT_DEFAULT;
	
	if (!(window = surface->GetWindow ()))
		return CURSOR_BLINK_TIMEOUT_DEFAULT;
	
	if (!(widget = window->GetGdkWindow ()))
		return CURSOR_BLINK_TIMEOUT_DEFAULT;
	
	if (!(screen = gdk_drawable_get_screen ((GdkDrawable *) widget)))
		return CURSOR_BLINK_TIMEOUT_DEFAULT;
	
	if (!(settings = gtk_settings_get_for_screen (screen)))
		return CURSOR_BLINK_TIMEOUT_DEFAULT;
	
	g_object_get (settings, "gtk-cursor-blink-time", &timeout, NULL);
	
	return timeout;
}

void
TextBoxView::ConnectBlinkTimeout (guint multiplier)
{
	guint timeout = GetCursorBlinkTimeout (this) * multiplier / CURSOR_BLINK_DIVIDER;
	Surface *surface = GetSurface ();
	TimeManager *manager;
	
	if (!surface || !(manager = surface->GetTimeManager ()))
		return;
	
	blink_timeout = manager->AddTimeout (G_PRIORITY_DEFAULT, timeout, TextBoxView::blink, this);
}

void
TextBoxView::DisconnectBlinkTimeout ()
{
	TimeManager *manager;
	Surface *surface;
	
	if (blink_timeout != 0) {
		if (!(surface = GetSurface ()) || !(manager = surface->GetTimeManager ()))
			return;
		
		manager->RemoveTimeout (blink_timeout);
		blink_timeout = 0;
	}
}

bool
TextBoxView::Blink ()
{
	guint multiplier;
	
	if (cursor_visible) {
		multiplier = CURSOR_BLINK_OFF_MULTIPLIER;
		HideCursor ();
	} else {
		multiplier = CURSOR_BLINK_ON_MULTIPLIER;
		ShowCursor ();
	}
	
	ConnectBlinkTimeout (multiplier);
	
	return false;
}

void
TextBoxView::DelayCursorBlink ()
{
	DisconnectBlinkTimeout ();
	ConnectBlinkTimeout (CURSOR_BLINK_DELAY_MULTIPLIER);
	UpdateCursor (true);
	ShowCursor ();
}

void
TextBoxView::BeginCursorBlink ()
{
	if (blink_timeout == 0) {
		ConnectBlinkTimeout (CURSOR_BLINK_ON_MULTIPLIER);
		UpdateCursor (true);
		ShowCursor ();
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
TextBoxView::ResetCursorBlink (bool delay)
{
	if (textbox->IsFocused () && !textbox->HasSelectedText ()) {
		// cursor is blinkable... proceed with blinkage
		if (delay)
			DelayCursorBlink ();
		else
			BeginCursorBlink ();
	} else {
		// cursor not blinkable... stop all blinkage
		EndCursorBlink ();
	}
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
		cursor = Rect (0.0, 0.0, 1.0, font->Height ());
		font->unref ();
	} else {
		cursor = layout->GetCursor (Point (), cur);
	}
	
	// invalidate the new cursor rect
	if (invalidate && cursor_visible)
		Invalidate (cursor);
}

void
TextBoxView::Render (cairo_t *cr, Region *region, bool path_only)
{
	if (dirty)
		Layout (cr, GetRenderSize ());
	
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
		
		if (n > 0) {
			// append the Run
			run = new TextRun (start, n, textbox, selected);
			runs->Append (run);
		}
		
		if (inptr == inend) {
			(*length) -= n;
			break;
		}
		
		// append the LineBreak
		if (inptr[0] == '\r' && inptr[1] == '\n') {
			run = new TextRun (inptr, 2, textbox, selected);
			inptr += 2;
			n += 2;
		} else {
			run = new TextRun (inptr, 1, textbox, selected);
			inptr++;
			n++;
		}
		
		runs->Append (run);
		
		(*length) -= n;
	}
	
	*text = inptr;
}

Size
TextBoxView::MeasureOverride (Size availableSize)
{
	Thickness padding = Thickness (0); //*GetPadding ();
	Size constraint = availableSize.GrowBy (-padding);
	
	cairo_t *cr = measuring_context_create ();
	Layout (cr, constraint);
	measuring_context_destroy (cr);

	Size desired = Size ();
	layout->GetActualExtents (&desired.width, &desired.height);
	
	return desired.Min (availableSize);
}

Size
TextBoxView::ArrangeOverride (Size finalSize)
{
        Thickness padding = Thickness (0); //*GetPadding ();
	Size constraint = finalSize.GrowBy (-padding);

	cairo_t *cr = measuring_context_create ();
	Layout (cr, constraint);
	measuring_context_destroy (cr);

	Size arranged = Size ();
	layout->GetActualExtents (&arranged.width, &arranged.height);
	
	arranged = arranged.GrowBy (padding);

	return arranged.Max (finalSize);
}

void
TextBoxView::Layout (cairo_t *cr, Size constraint)
{
	int selection_length = textbox->GetSelectionLength ();
	int selection_start = textbox->GetSelectionStart ();
	TextBuffer *buffer = textbox->GetBuffer ();
	const gunichar *text = buffer->text;
	double width = constraint.width;
	List *runs;
	int left;
	
	if (isinf (width))
		layout->SetMaxWidth (-1.0);
	else
		layout->SetMaxWidth (width);
	
	runs = new List ();
	
	if (selection_length > 0) {
		left = selection_start;
		
		if (left > 0) {
			// add text before the selected region
			append_runs ((ITextSource *) textbox, runs, &text, &left, false);
		}
		
		// add the selected region of text
		left += selection_length;
		append_runs ((ITextSource *) textbox, runs, &text, &left, true);
		
		left += buffer->len - (text - buffer->text);
	} else {
		left = buffer->len;
	}
	
	// add the text after the selected region
	append_runs ((ITextSource *) textbox, runs, &text, &left, false);
	
	layout->SetTextRuns (runs);
	layout->Layout ();
	dirty = false;
	
	UpdateCursor (false);
}

void
TextBoxView::Paint (cairo_t *cr)
{
	layout->Render (cr, GetOriginPoint (), Point ());
	
	if (cursor_visible) {
		cairo_antialias_t alias = cairo_get_antialias (cr);
		double h = round (cursor.height);
		double x = cursor.x;
		double y = cursor.y;
		
		// disable antialiasing
		cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
		
		// set the color to black
		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
		
		// snap 'x' to the half-pixel grid (to try and get a sharp 1-pixel-wide line)
		cairo_user_to_device (cr, &x, &y);
		x = trunc (x) + 0.5; y = trunc (y);
		cairo_device_to_user (cr, &x, &y);
		
		// draw the cursor
		cairo_set_line_width (cr, 1.0);
		cairo_move_to (cr, x, y);
		cairo_line_to (cr, x, y + h);
		cairo_stroke (cr);
		
		// restore antialiasing
		cairo_set_antialias (cr, alias);
	}
}

void
TextBoxView::OnModelChanged (TextBoxModelChangedEventArgs *args)
{
	switch (args->changed) {
	case TextBoxModelChangedTextAlignment:
		// text alignment changed, update our layout
		dirty = layout->SetTextAlignment ((TextAlignment) args->property->new_value->AsInt32 ());
		break;
	case TextBoxModelChangedTextWrapping:
		// text wrapping changed, update our layout
		dirty = layout->SetTextWrapping ((TextWrapping) args->property->new_value->AsInt32 ());
		break;
	case TextBoxModelChangedSelection:
		if (had_selected_text || textbox->HasSelectedText ()) {
			// the selection has changed, need to recalculate layout
			had_selected_text = textbox->HasSelectedText ();
			ResetCursorBlink (false);
			dirty = true;
		} else {
			// cursor position changed
			ResetCursorBlink (true);
			return;
		}
		break;
	case TextBoxModelChangedBrush:
		// a brush has changed, no layout updates needed, we just need to re-render
		break;
	case TextBoxModelChangedFont:
		// font changed, need to recalculate layout
		dirty = true;
		break;
	case TextBoxModelChangedText:
		// the text has changed, need to recalculate layout
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
TextBoxView::model_changed (EventObject *sender, EventArgs *args, gpointer closure)
{
	((TextBoxView *) closure)->OnModelChanged ((TextBoxModelChangedEventArgs *) args);
}

void
TextBoxView::OnFocusOut ()
{
	EndCursorBlink ();
}

void
TextBoxView::OnFocusIn ()
{
	ResetCursorBlink (false);
}

void
TextBoxView::SetTextBox (TextBox *textbox)
{
	if (this->textbox == textbox)
		return;
	
	if (this->textbox) {
		// remove the event handlers from the old textbox
		this->textbox->RemoveHandler (TextBox::ModelChangedEvent, TextBoxView::model_changed, this);
	}
	
	if (textbox) {
		textbox->AddHandler (TextBox::ModelChangedEvent, TextBoxView::model_changed, this);
		
		// sync our state with the textbox
		layout->SetTextAlignment (textbox->GetTextAlignment ());
		layout->SetTextWrapping (textbox->GetTextWrapping ());
		had_selected_text = textbox->HasSelectedText ();
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
	if (args->GetId () == PasswordBox::PasswordProperty)
		Emit (PasswordBox::PasswordChangedEvent);
	
	TextBox::OnPropertyChanged (args);	
}
