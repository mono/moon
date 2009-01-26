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
// TextBoxDynamicPropertyValueProvider
//

class TextBoxDynamicPropertyValueProvider : public PropertyValueProvider {
	Value *selection_value;
	
 public:
	TextBoxDynamicPropertyValueProvider (DependencyObject *obj) : PropertyValueProvider (obj)
	{
		selection_value = NULL;
	}
	
	virtual ~TextBoxDynamicPropertyValueProvider ()
	{
		delete selection_value;
	}
	
	virtual Value *GetPropertyValue (DependencyProperty *property)
	{
		if (property != TextBox::SelectedTextProperty)
			return NULL;
		
		TextBox *tb = (TextBox *) obj;
		char *selection;
		
		if (tb->selection_changed) {
			delete selection_value;
			selection = g_ucs4_to_utf8 (tb->buffer->text + tb->selection.start, tb->selection.length, NULL, NULL, NULL);
			selection_value = new Value (selection, true);
			tb->selection_changed = false;
		}
		
		return selection_value;
	}
};


//
// TextBox
//

TextBox::TextBox ()
{
	SetObjectType (Type::TEXTBOX);
	ManagedTypeInfo *type_info = new ManagedTypeInfo ();
	type_info->assembly_name = g_strdup ("System.Windows");
	type_info->full_name = g_strdup ("System.Windows.Controls.TextBox");

	SetDefaultStyleKey (type_info);

	providers[PropertyPrecedence_DynamicValue] = new TextBoxDynamicPropertyValueProvider (this);
	
	AddHandler (UIElement::KeyDownEvent, TextBox::key_down, this);
	AddHandler (UIElement::KeyUpEvent, TextBox::key_up, this);
	
	font = new TextFontDescription ();
	font->SetFamily (CONTROL_FONT_FAMILY);
	font->SetStretch (CONTROL_FONT_STRETCH);
	font->SetWeight (CONTROL_FONT_WEIGHT);
	font->SetStyle (CONTROL_FONT_STYLE);
	font->SetSize (CONTROL_FONT_SIZE);
	
	buffer = new TextBuffer ();
	
	selection_changed = false;
	selection.length = 0;
	selection.start = 0;
	emit = true;
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
	RemoveHandler (UIElement::KeyDownEvent, TextBox::key_down, this);
	RemoveHandler (UIElement::KeyUpEvent, TextBox::key_up, this);
	
	delete buffer;
	delete font;
}

#define CURSOR_POSITION_CHANGED (1 << 0)
#define SELECTION_CHANGED       (1 << 1)
#define CONTENT_CHANGED         (1 << 2)
#define NOTHING_CHANGED         (0)

#define MY_GDK_ALT_MASK         (GDK_MOD1_MASK | GDK_MOD2_MASK | GDK_MOD3_MASK | GDK_MOD4_MASK | GDK_MOD5_MASK)

static int
move_down (TextBuffer *buffer, int cursor, int n_lines)
{
	int offset, cur, n;
	
	// first find out what our character offset is in the current line
	cur = cursor;
	while (cur > 0 && buffer->text[cur - 1] != '\n')
		cur--;
	
	offset = cursor - cur;
	
	cur = cursor;
	n = 0;
	
	// skip ahead one page worth of lines
	while (n < n_lines) {
		while (cur < buffer->len && buffer->text[cur] != '\n')
			cur++;
		
		if (cur == buffer->len)
			break;
		
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
	while (cur > 0 && buffer->text[cur - 1] != '\n')
		cur--;
	
	offset = cursor - cur;
	n = 0;
	
	// go back one page worth of lines
	while (n < n_lines) {
		while (cur > 0 && buffer->text[cur - 1] != '\n')
			cur--;
		
		if (cur == 0)
			break;
		
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
	int i, eoln = cursor;
	CharClass cc;
	
	// find the end of the line
	while (eoln < buffer->len && buffer->text[eoln] != '\n')
		eoln++;
	
	// if the cursor is at the end of the line, return the starting offset of the next line
	if (cursor == eoln) {
		if (eoln < buffer->len)
			return eoln + 1;
		
		return cursor;
	}
	
	cc = char_class (buffer->text[cursor]);
	i = cursor;
	
	// skip over the word, punctuation, or run of whitespace
	while (i < eoln && char_class (buffer->text[i]) == cc)
		i++;
	
	// skip any whitespace after the word/punct
	while (i < eoln && char_class (buffer->text[i]) == CharClassWhitespace)
		i++;
	
	return i;
}

static int
prev_word (TextBuffer *buffer, int cursor)
{
	int i, boln = cursor;
	CharClass cc;
	
	// find the end of the line
	while (boln > 0 && buffer->text[boln - 1] != '\n')
		boln--;
	
	// if the cursor is at the beginning of the line, return the end of the prev line
	if (cursor == boln) {
		if (boln > 0)
			return boln - 1;
		
		return 0;
	}
	
	cc = char_class (buffer->text[cursor - 1]);
	i = cursor;
	
	// skip over the word, punctuation, or run of whitespace
	while (i > boln && char_class (buffer->text[i - 1]) == cc)
		i--;
	
	// if the cursor was at whitespace, skip back a word too
	if (cc == CharClassWhitespace && i > boln) {
		cc = char_class (buffer->text[i - 1]);
		while (i > boln && char_class (buffer->text[i - 1]) == cc)
			i--;
	}
	
	return i;
}

int
TextBox::CursorBackSpace (GdkModifierType modifiers)
{
	int changed = NOTHING_CHANGED;
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) != 0)
		return NOTHING_CHANGED;
	
	if (selection.length > 0) {
		// BackSpace w/ active selection: delete the selected text
		buffer->Cut (selection.start, selection.length);
		changed = CONTENT_CHANGED;
		
		if (cursor != selection.start) {
			changed |= CURSOR_POSITION_CHANGED;
			cursor = selection.start;
		}
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+BackSpace: delete the word ending at the cursor
		pos = prev_word (buffer, cursor);
		
		if (pos < cursor) {
			changed = CURSOR_POSITION_CHANGED | CONTENT_CHANGED;
			buffer->Cut (pos, cursor - pos);
			cursor = pos;
		}
	} else if (cursor > 0) {
		// BackSpace: delete the char before the cursor position
		changed = CURSOR_POSITION_CHANGED | CONTENT_CHANGED;
		buffer->Cut (cursor - 1, 1);
		cursor--;
	}
	
	// clear the selection if there was any
	length = start = 0;
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		changed |= SELECTION_CHANGED;
		
		if (length > 0) {
			SetSelectionLength (length);
			SetSelectionStart (start);
		} else {
			ClearSelection ();
		}
	}
	
	return changed;
}

int
TextBox::CursorDelete (GdkModifierType modifiers)
{
	int changed = NOTHING_CHANGED;
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) != 0)
		return NOTHING_CHANGED;
	
	if (selection.length > 0) {
		// Delete w/ active selection: delete the selected text
		buffer->Cut (selection.start, selection.length);
		changed = CONTENT_CHANGED;
		
		if (cursor != selection.start) {
			changed |= CURSOR_POSITION_CHANGED;
			cursor = selection.start;
		}
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+Delete: delete the word starting at the cursor
		pos = next_word (buffer, cursor);
		
		if (pos > cursor) {
			buffer->Cut (cursor, pos - cursor);
			changed = CONTENT_CHANGED;
		}
	} else if (cursor < buffer->len) {
		// Delete: delete the char after the cursor position
		changed = CONTENT_CHANGED;
		buffer->Cut (cursor, 1);
	}
	
	// clear the selection if there was any
	length = start = 0;
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		changed |= SELECTION_CHANGED;
		
		if (length > 0) {
			SetSelectionLength (length);
			SetSelectionStart (start);
		} else {
			ClearSelection ();
		}
	}
	
	return changed;
}

int
TextBox::CursorPageDown (GdkModifierType modifiers)
{
	int changed = NOTHING_CHANGED;
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK)) != 0)
		return NOTHING_CHANGED;
	
	if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Page_Down: grow selection by one page in the downward direction
		pos = move_down (buffer, cursor, 8);
		
		if (cursor == selection.start) {
			// cursor was at the beginning of the selection
			if (pos > selection.start + selection.length) {
				// flip selection over the current selection endpoint
				start += selection.length;
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
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else {
		// Page_Down: move cursor down one page and clear selection
		pos = move_down (buffer, cursor, 8);
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
		
		length = start = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		changed |= SELECTION_CHANGED;
		
		if (length > 0) {
			SetSelectionLength (length);
			SetSelectionStart (start);
		} else {
			ClearSelection ();
		}
	}
	
	return changed;
}

int
TextBox::CursorPageUp (GdkModifierType modifiers)
{
	int changed = NOTHING_CHANGED;
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK)) != 0)
		return NOTHING_CHANGED;
	
	if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Page_Up: grow selection by one page in the upward direction
		pos = move_up (buffer, cursor, 8);
		
		if (cursor > selection.start) {
			// cursor was at the end of the selection
			if (pos < selection.start) {
				// flip selection over the current selection starting point
				length = selection.start - pos;
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
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else {
		// Page_Up: move cursor up one page and clear selection
		pos = move_up (buffer, cursor, 8);
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
		
		length = start = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		changed |= SELECTION_CHANGED;
		
		if (length > 0) {
			SetSelectionLength (length);
			SetSelectionStart (start);
		} else {
			ClearSelection ();
		}
	}
	
	return changed;
}

int
TextBox::CursorHome (GdkModifierType modifiers)
{
	int changed = NOTHING_CHANGED;
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & MY_GDK_ALT_MASK) != 0)
		return NOTHING_CHANGED;
	
	if ((modifiers & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
		// Ctrl+Shift+Home: update selection to start at the beginning of the buffer
		pos = 0;
		
		if (cursor > selection.start) {
			// cursor was at the end of the selection
			if (pos < selection.start) {
				// flip selection over the current selection starting point
				length = selection.start - pos;
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
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+Home: move cursor to beginning of the buffer and clear selection
		if (cursor != 0) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = 0;
		}
		
		length = start = 0;
	} else if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Home: update selection to start at beginning of line
		pos = cursor;
		while (pos > 0 && buffer->text[pos - 1] != '\n')
			pos--;
		
		if (cursor > selection.start) {
			// cursor was at the end of the selection
			if (pos < selection.start) {
				// flip selection over the current selection starting point
				length = selection.start - pos;
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
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else {
		// Home: move cursor to beginning of line and clear selection
		pos = cursor;
		while (pos > 0 && buffer->text[pos - 1] != '\n')
			pos--;
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
		
		length = start = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		changed |= SELECTION_CHANGED;
		
		if (length > 0) {
			SetSelectionLength (length);
			SetSelectionStart (start);
		} else {
			ClearSelection ();
		}
	}
	
	return changed;
}

int
TextBox::CursorEnd (GdkModifierType modifiers)
{
	int changed = NOTHING_CHANGED;
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & MY_GDK_ALT_MASK) != 0)
		return NOTHING_CHANGED;
	
	if ((modifiers & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
		// Ctrl+Shift+End: update selection to end at the end of the buffer
		pos = buffer->len;
		
		if (cursor == selection.start) {
			// cursor was at the beginning of the selection
			if (pos > selection.start + selection.length) {
				// flip selection over the current selection endpoint
				start += selection.length;
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
		
		if (cursor != buffer->len) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = buffer->len;
		}
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+End: move cursor to end of the buffer and clear selection
		if (cursor != buffer->len) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = buffer->len;
		}
		
		length = start = 0;
	} else if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+End: update selection to end at the end of the current line
		pos = cursor;
		while (pos < buffer->len && buffer->text[pos] != '\n')
			pos++;
		
		if (cursor == selection.start) {
			// cursor was at the beginning of the selection
			if (pos > selection.start + selection.length) {
				// flip selection over the current selection endpoint
				start += selection.length;
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
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else {
		// End: move cursor to end of line and clear selection
		pos = cursor;
		while (pos < buffer->len && buffer->text[pos] != '\n')
			pos++;
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
		
		length = start = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		changed |= SELECTION_CHANGED;
		
		if (length > 0) {
			SetSelectionLength (length);
			SetSelectionStart (start);
		} else {
			ClearSelection ();
		}
	}
	
	return changed;
}

int
TextBox::CursorRight (GdkModifierType modifiers)
{
	int changed = NOTHING_CHANGED;
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & MY_GDK_ALT_MASK) != 0)
		return NOTHING_CHANGED;
	
	if ((modifiers & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
		// Ctrl+Shift+Right: grow selection to the right on word's worth of characters
		pos = next_word (buffer, cursor);
		
		if (cursor == selection.start) {
			// cursor was at the beginning of the selection
			if (pos > selection.start + selection.length) {
				// flip selection over the current selection endpoint
				start += selection.length;
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
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+Right: move cursor to right one word's worth of characters and clear selection
		pos = next_word (buffer, cursor);
		
		length = start = 0;
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Right: grow the selection to the right by one character
		if (cursor < buffer->len) {
			pos = cursor + 1;
			
			if (cursor == selection.start) {
				// cursor was at the beginning of the selection
				if (pos > selection.start + selection.length) {
					// flip selection over the current selection endpoint
					start += selection.length;
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
			
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else if (cursor < buffer->len) {
		// Right: move the cursor one character to the right and clear the selection
		changed = CURSOR_POSITION_CHANGED;
		length = start = 0;
		cursor++;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		changed |= SELECTION_CHANGED;
		
		if (length > 0) {
			SetSelectionLength (length);
			SetSelectionStart (start);
		} else {
			ClearSelection ();
		}
	}
	
	return changed;
}

int
TextBox::CursorLeft (GdkModifierType modifiers)
{
	int changed = NOTHING_CHANGED;
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & MY_GDK_ALT_MASK) != 0)
		return NOTHING_CHANGED;
	
	if ((modifiers & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
		// Ctrl+Shift+Left: grow selection to the left one word's worth of characters
		pos = prev_word (buffer, cursor);
		
		if (cursor > selection.start) {
			// cursor was at the end of the selection
			if (pos < selection.start) {
				// flip selection over the current selection starting point
				length = selection.start - pos;
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
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else if ((modifiers & GDK_CONTROL_MASK) != 0) {
		// Ctrl+Left: move cursor to left one word's worth of characters and clear selection
		pos = prev_word (buffer, cursor);
		
		length = start = 0;
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Left: grow the selection to the left by one character
		if (cursor < buffer->len) {
			pos = cursor + 1;
			
			if (cursor > selection.start) {
				// cursor was at the end of the selection
				if (pos < selection.start) {
					// flip selection over the current selection starting point
					length = selection.start - pos;
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
			
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else if (cursor > 0) {
		// Left: move the cursor one character to the right and clear the selection
		changed = CURSOR_POSITION_CHANGED;
		length = start = 0;
		cursor--;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		changed |= SELECTION_CHANGED;
		
		if (length > 0) {
			SetSelectionLength (length);
			SetSelectionStart (start);
		} else {
			ClearSelection ();
		}
	}
	
	return changed;
}

int
TextBox::CursorDown (GdkModifierType modifiers)
{
	int changed = NOTHING_CHANGED;
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK)) != 0)
		return NOTHING_CHANGED;
	
	if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Down: grow selection by one line in the downward direction
		pos = move_down (buffer, cursor, 1);
		
		if (cursor == selection.start) {
			// cursor was at the beginning of the selection
			if (pos > selection.start + selection.length) {
				// flip selection over the current selection endpoint
				start += selection.length;
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
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else {
		// Down: move cursor down one line and clear selection
		pos = move_down (buffer, cursor, 1);
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
		
		length = start = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		changed |= SELECTION_CHANGED;
		
		if (length > 0) {
			SetSelectionLength (length);
			SetSelectionStart (start);
		} else {
			ClearSelection ();
		}
	}
	
	return changed;
}

int
TextBox::CursorUp (GdkModifierType modifiers)
{
	int changed = NOTHING_CHANGED;
	int length = selection.length;
	int start = selection.start;
	int pos;
	
	if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK)) != 0)
		return NOTHING_CHANGED;
	
	if ((modifiers & GDK_SHIFT_MASK) != 0) {
		// Shift+Up: grow selection by one line in the upward direction
		pos = move_up (buffer, cursor, 1);
		
		if (cursor > selection.start) {
			// cursor was at the end of the selection
			if (pos < selection.start) {
				// flip selection over the current selection starting point
				length = selection.start - pos;
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
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
	} else {
		// Up: move cursor up one line and clear selection
		pos = move_up (buffer, cursor, 1);
		
		if (cursor != pos) {
			changed = CURSOR_POSITION_CHANGED;
			cursor = pos;
		}
		
		length = start = 0;
	}
	
	// check to see if selection has changed
	if (selection.start != start || selection.length != length) {
		changed |= SELECTION_CHANGED;
		
		if (length > 0) {
			SetSelectionLength (length);
			SetSelectionStart (start);
		} else {
			ClearSelection ();
		}
	}
	
	return changed;
}

void
TextBox::OnKeyDown (KeyEventArgs *args)
{
	GdkModifierType modifiers = (GdkModifierType) args->GetModifiers ();
	guint key = args->GetKeyVal ();
	int changed = NOTHING_CHANGED;
	gunichar c;
	
	if (args->IsModifier ())
		return;
	
	// FIXME: so... we'll need to do a few things here:
	// 1. interpret the key (insert a char or move the cursor, etc)
	// 2. update state being mindful of the current state (cursor, selection, etc)
	// 3. register some sort of repeat timeout that expires when OnKeyUp() is called
	//
	// Probably a good idea to lift a lot of the logic from jackson's MWF code.
	printf ("TextBox::OnKeyDown()\n");
	
	if ((c = args->GetUnicode ())) {
		// normal character key
		if ((maxlen > 0 && buffer->len >= maxlen) || ((c == '\n') && !GetAcceptsReturn ()))
			return;
		
		if (selection.length > 0) {
			// replace the currently selected text
			changed = CONTENT_CHANGED | CURSOR_POSITION_CHANGED | SELECTION_CHANGED;
			buffer->Replace (selection.start, selection.length, &c, 1);
			cursor = selection.start + 1;
			ClearSelection ();
		} else {
			// insert the text at the cursor position
			changed = CONTENT_CHANGED | CURSOR_POSITION_CHANGED;
			buffer->Insert (cursor, c);
			cursor++;
		}
	} else {
		switch (key) {
		case GDK_BackSpace:
			changed = CursorBackSpace (modifiers);
			break;
		case GDK_Delete:
			changed = CursorDelete (modifiers);
			break;
		case GDK_KP_Page_Down:
		case GDK_Page_Down:
			changed = CursorPageDown (modifiers);
			break;
		case GDK_KP_Page_Up:
		case GDK_Page_Up:
			changed = CursorPageUp (modifiers);
			break;
		case GDK_KP_Home:
		case GDK_Home:
			changed = CursorHome (modifiers);
			break;
		case GDK_KP_End:
		case GDK_End:
			changed = CursorEnd (modifiers);
			break;
		case GDK_KP_Right:
		case GDK_Right:
			changed = CursorRight (modifiers);
			break;
		case GDK_KP_Left:
		case GDK_Left:
			changed = CursorLeft (modifiers);
			break;
		case GDK_KP_Down:
		case GDK_Down:
			changed = CursorDown (modifiers);
			break;
		case GDK_KP_Up:
		case GDK_Up:
			changed = CursorUp (modifiers);
			break;
		case GDK_A:
		case GDK_a:
			if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) == GDK_CONTROL_MASK) {
				// select all and move cursor to end of buffer
				if (cursor != buffer->len) {
					changed = CURSOR_POSITION_CHANGED;
					cursor = buffer->len;
				}
				
				SelectAll ();
			}
			break;
		case GDK_C:
		case GDK_c:
			if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) == GDK_CONTROL_MASK) {
				// copy selection to the clipboard
				// FIXME: implement me
			}
			break;
		case GDK_X:
		case GDK_x:
			if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) == GDK_CONTROL_MASK) {
				// copy selection to the clipboard and then cut
				// FIXME: implement me
			}
			break;
		case GDK_V:
		case GDK_v:
			if ((modifiers & (GDK_CONTROL_MASK | MY_GDK_ALT_MASK | GDK_SHIFT_MASK)) == GDK_CONTROL_MASK) {
				// paste clipboard contents to the buffer
				// FIXME: implement me
			}
			break;
		case GDK_Y:
		case GDK_y:
			// Ctrl+Y := Redo
			break;
		case GDK_Z:
		case GDK_z:
			// Ctrl+Z := Undo
			break;
		default:
			// FIXME: what other keys do we need to handle?
			break;
		}
		
		// FIXME: some of these may also require updating scrollbars?
	}
	
	if (changed & CONTENT_CHANGED)
		Emit (TextChangedEvent, new TextChangedEventArgs ());
	
	if (changed & SELECTION_CHANGED)
		Emit (TextChangedEvent, new RoutedEventArgs ());
	
	// only bother emitting this event if the cursor position is
	// the only thing that changed. If either Text or Selection
	// changed, then TextBoxView has already been notified that it
	// needs to invalidate.
	if (changed == CURSOR_POSITION_CHANGED)
		Emit (ModelChangedEvent, new TextBoxModelChangedEventArgs (TextBoxModelChangedCursorPosition));
	
	// FIXME: register a key repeat timeout?
}

void
TextBox::OnKeyUp (KeyEventArgs *args)
{
	// FIXME: unregister the key repeat timeout?
}

void
TextBox::key_down (EventObject *sender, EventArgs *args, void *closure)
{
	((TextBox *) closure)->OnKeyDown ((KeyEventArgs *) args);
}

void
TextBox::key_up (EventObject *sender, EventArgs *args, void *closure)
{
	((TextBox *) closure)->OnKeyUp ((KeyEventArgs *) args);
}

void
TextBox::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	TextBoxModelChangeType changed = TextBoxModelChangedNothing;
	bool invalidate = false;
	
	if (args->property == Control::FontFamilyProperty) {
		char *family = args->new_value ? args->new_value->AsString () : NULL;
		changed = TextBoxModelChangedFont;
		font->SetFamily (family);
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
	} else if (args->property == TextBox::MaxLengthProperty) {
		maxlen = args->new_value->AsInt32 ();
	} else if (args->property == TextBox::SelectedTextProperty) {
		const char *str = args->new_value ? args->new_value->AsString () : "";
		gunichar *text;
		glong textlen;
		
		// FIXME: is the cursor/selection updating logic correct?
		
		text = g_utf8_to_ucs4_fast (str, -1, &textlen);
		if (selection.length > 0) {
			// replace the currently selected text
			buffer->Replace (selection.start, selection.length, text, textlen);
			cursor = selection.start + textlen;
			ClearSelection ();
		} else {
			// insert the text at the cursor position
			buffer->Insert (cursor, text, textlen);
			cursor += textlen;
		}
		
		Emit (TextBox::TextChangedEvent, new TextChangedEventArgs ());
		
		g_free (text);
	} else if (args->property == TextBox::SelectionStartProperty) {
		selection.start = args->new_value->AsInt32 ();
		selection_changed = true;
		
		if (emit)
			Emit (TextBox::SelectionChangedEvent, new RoutedEventArgs ());
	} else if (args->property == TextBox::SelectionLengthProperty) {
		selection.length = args->new_value->AsInt32 ();
		selection_changed = true;
		
		if (emit)
			Emit (TextBox::SelectionChangedEvent, new RoutedEventArgs ());
	} else if (args->property == TextBox::SelectionBackgroundProperty) {
		changed = TextBoxModelChangedBrush;
	} else if (args->property == TextBox::SelectionForegroundProperty) {
		changed = TextBoxModelChangedBrush;
	} else if (args->property == TextBox::TextProperty) {
		const char *str = args->new_value ? args->new_value->AsString () : "";
		gunichar *text;
		glong textlen;
		
		// FIXME: is the cursor/selection updating logic correct?
		
		text = g_utf8_to_ucs4_fast (str, -1, &textlen);
		buffer->Replace (0, buffer->len, text, textlen);
		ClearSelection ();
		cursor = textlen;
		g_free (text);
		
		Emit (TextBox::TextChangedEvent, new TextChangedEventArgs ());
	} else if (args->property == TextBox::TextAlignmentProperty) {
		changed = TextBoxModelChangedTextAlignment;
	} else if (args->property == TextBox::TextWrappingProperty) {
		changed = TextBoxModelChangedTextWrapping;
	} else if (args->property == TextBox::HorizontalScrollBarVisibilityProperty) {
		/* FIXME: need to figure out:
		 *
		 * 1. whether the scrollbar should be shown or not
		 * 2. whether the layout needs to be recalculated if the visibility changes
		 */
		invalidate = true;
	} else if (args->property == TextBox::VerticalScrollBarVisibilityProperty) {
		/* FIXME: need to figure out:
		 *
		 * 1. whether the scrollbar should be shown or not
		 * 2. whether the layout needs to be recalculated if the visibility changes
		 */
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
TextBox::ClearSelection ()
{
	emit = false;
	SetSelectionLength (0);
	SetSelectionStart (0);
	emit = true;
	
	Emit (TextBox::SelectionChangedEvent, new RoutedEventArgs ());
}

void
TextBox::Select (int start, int length)
{
	if (start < 0 || start > buffer->len || start + length > buffer->len)
		return;
	
	emit = false;
	SetSelectionLength (length);
	SetSelectionStart (start);
	emit = true;
	
	Emit (TextBox::SelectionChangedEvent, new RoutedEventArgs ());
}

void
TextBox::SelectAll ()
{
	Select (0, buffer->len);
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
	
	if (blink_timeout != 0)
		g_source_remove (blink_timeout);
	
	delete layout;
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
	TextBox *textbox = GetTextBox ();
	
	if (textbox->GetSelection ()->length == 0) {
		if (blink_timeout != 0)
			g_source_remove (blink_timeout);
		
		ConnectBlinkTimeout (CURSOR_DELAY_MULTIPLIER);
		ShowCursor ();
	}
}

void
TextBoxView::BeginCursorBlink ()
{
	TextBox *textbox = GetTextBox ();
	
	if (textbox->GetSelection ()->length == 0) {
		// no selection, proceed with blinking
		if (blink_timeout == 0) {
			ConnectBlinkTimeout (CURSOR_ON_MULTIPLIER);
			ShowCursor ();
		}
	} else {
		// temporarily disable blinking during selection
		if (blink_timeout != 0) {
			g_source_remove (blink_timeout);
			blink_timeout = 0;
		}
		
		cursor_visible = true;
	}
}

void
TextBoxView::EndCursorBlink ()
{
	if (blink_timeout != 0) {
		g_source_remove (blink_timeout);
		blink_timeout = 0;
	}
	
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
	TextBox *textbox = GetTextBox ();
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
TextBoxView::Render (cairo_t *cr, int x, int y, int width, int height)
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

void
TextBoxView::Layout (cairo_t *cr)
{
	TextBox *textbox = GetTextBox ();
	double width = GetWidth ();
	TextSelection *selection;
	const gunichar *text;
	TextBuffer *buffer;
	TextRun *run;
	List *runs;
	int left;
	
	if (width > 0.0f)
		layout->SetMaxWidth (width);
	else
		layout->SetMaxWidth (-1.0);
	
	buffer = textbox->GetBuffer ();
	text = buffer->text;
	left = buffer->len;
	
	runs = new List ();
	
	if ((selection = textbox->GetSelection ()) && selection->length > 0) {
		if (selection->start > 0) {
			// add run before the selected region
			run = new TextRun (text, selection->start, (ITextSource *) textbox);
			text += selection->start;
			left -= selection->start;
			runs->Append (run);
		}
		
		// add the selected text run
		run = new TextRun (text, selection->length, (ITextSource *) textbox, true);
		text += selection->length;
		left -= selection->length;
		runs->Append (run);
		
		// add the run after the selected region
		run = new TextRun (text, left, (ITextSource *) textbox, true);
		text += selection->length;
		left -= selection->length;
		runs->Append (run);
	} else {
		// nothing selected
		run = new TextRun (text, buffer->len, (ITextSource *) textbox);
		runs->Append (run);
	}
	
	layout->SetTextRuns (runs);
	layout->Layout ();
	
	UpdateCursor (false);
	
	dirty = false;
}

void
TextBoxView::Paint (cairo_t *cr)
{
	TextBox *textbox = GetTextBox ();
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
	// the selected region has changed, need to recalculate layout
	if (focused)
		BeginCursorBlink ();
	
	dirty = true;
	
	UpdateBounds (true);
	Invalidate ();
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
	if (focused)
		DelayCursorBlink ();
	
	dirty = true;
	
	UpdateBounds (true);
	Invalidate ();
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

Size
TextBoxView::ArrangeOverride (Size size)
{
	// FIXME: do we want to override this method? We probably do...
	return FrameworkElement::ArrangeOverride (size);
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
