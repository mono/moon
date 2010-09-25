
/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * asxparser.cpp: ASX parser
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <glib.h>

#include "asxparser.h"

namespace Moonlight {

#define MAX_ELEMENT_LEN 15


#define OP_OPEN_ELEMENT '<'
#define OP_CLOSE_ELEMENT '>'
#define OP_ASSIGNMENT '='
#define OP_SLASH '/'

enum TokenType {
	TOKEN_INVALID,
	TOKEN_OPEN_ELEMENT,
	TOKEN_CLOSE_ELEMENT,
	TOKEN_ASSIGNMENT,
	TOKEN_SLASH,
	TOKEN_DATA,
	TOKEN_NAME,
	TOKEN_QUOTED_STRING,
	TOKEN_WHITESPACE,
	TOKEN_EOF
};


const char* token_type_to_string (TokenType t)
{
	switch (t) {
	case TOKEN_INVALID:
		return "INVALID";
	case TOKEN_OPEN_ELEMENT:
		return "OPEN_ELEMENT";
	case TOKEN_CLOSE_ELEMENT:
		return "CLOSE_ELEMENT";
	case TOKEN_ASSIGNMENT:
		return "ASSIGNMENT";
	case TOKEN_SLASH:
		return "SLASH";
	case TOKEN_DATA:
		return "DATA";
	case TOKEN_NAME:
		return "NAME";
	case TOKEN_QUOTED_STRING:
		return "QUOTED_STRING";
	case TOKEN_WHITESPACE:
		return "WHITESPACE";		
	case TOKEN_EOF:
		return "EOF";
	}

	return "";
}




class AsxToken {

public:
	AsxToken (TokenType type, const char *value)
	{
		this->type = type;
		this->value = value;

		entirely_whitespace = false;
	}

	TokenType get_type ()
	{
		return type;
	}

	void set_type (TokenType type)
	{
		this->type = type;
	}

	const char* get_value ()
	{
		return value;
	}

	void set_value (const char* value)
	{
		this->value = value;
	}

	bool get_entirely_whitespace ()
	{
		return entirely_whitespace;
	}

	void set_entirely_whitespace (bool value)
	{
		entirely_whitespace = value;
	}

	char* to_string ()
	{
		return g_strdup_printf ("%s: %s", token_type_to_string (type), value);
	}

private:
	TokenType type;
	const char* value;
	bool entirely_whitespace;
};



class AsxTokenizer {

public:
	AsxTokenizer (TextStream *stream)
	{
		this->stream = stream;

		current_token = new AsxToken (TOKEN_INVALID, NULL);

		this->error_handler = NULL;
		col = line = position = 0;

		buffer = g_string_new ("");

		in_data = false;

		putback_chars = g_string_new ("");
	}

	~AsxTokenizer ()
	{
		delete stream;

		delete current_token;
		g_string_free (buffer, true);
	}

	AsxToken* get_current ()
	{
		return current_token;
	}

	const char* get_buffer_str ()
	{
		return buffer->str;
	}

	void clear_buffer_str ()
	{
		g_string_set_size (buffer, 0);
	}

	void move_next ();

	void set_error_handler (asx_error_handler *handler)
	{
		error_handler = handler;
	}

	int get_current_line_number ()
	{
		return line;
	}

	int get_current_column_number ()
	{
		return col;
	}
private:
	TextStream *stream;
	asx_error_handler *error_handler;

	AsxToken *current_token;

	int col;
	int line;
	int position;

	GString *buffer;
	GString *putback_chars;

	bool in_data;

	char read_char ();
	void putback (char c);

	bool is_name_char (char c);
	bool is_quote_char (char c);
	bool is_comment_open (char c);

	const char* read_name (char start);
	const char* read_quoted_string (char start);
	const char* read_data (char start, bool &entirely_whitespace);
	void read_comment ();
};


class AsxParserInternal {

public:
	AsxParserInternal (AsxParser *parser)
	{
		this->parser = parser;

		tokenizer = NULL;

		element_stack = g_queue_new ();
		current_attributes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

		error_code = ASXPARSER_ERROR_NONE;
		error_message = NULL;

		stop_parsing = false;
		start_element_handler = NULL;
		end_element_handler = NULL;
		text_handler = NULL;
		error_handler = NULL;
	}

	~AsxParserInternal ();

	void set_error_handler (asx_error_handler *handler)
	{
		tokenizer->set_error_handler (handler);
		error_handler = handler;
	}

	void set_text_handler (asx_text_handler *handler)
	{
		text_handler = handler;
	}

	void set_element_start_handler (asx_element_start_handler *handler)
	{
		start_element_handler = handler;
	}

	void set_element_end_handler (asx_element_end_handler *handler)
	{
		end_element_handler = handler;
	}

	AsxParserError get_error_code ()
	{
		return error_code;
	}

	const char* get_error_message ()
	{
		return error_message;
	}

	int get_current_line_number ()
	{
		if (!tokenizer)
			return -1;
		return tokenizer->get_current_line_number ();
	}

	int get_current_column_number ()
	{
		if (!tokenizer)
			return -1;
		return tokenizer->get_current_column_number ();
	}

	bool parse_stream (TextStream *stream);
	void stop ();

private:
	AsxTokenizer *tokenizer;
	AsxParser *parser;

	GQueue *element_stack;

	asx_error_handler *error_handler;
	asx_element_start_handler *start_element_handler;
	asx_element_end_handler *end_element_handler;
	asx_text_handler *text_handler;

	bool stop_parsing;

	char current_element [MAX_ELEMENT_LEN];
	GHashTable *current_attributes;

	AsxParserError error_code;
	const char* error_message;

	void raise_error (AsxParserError code, const char* error);

	void clear_current_element ();
	void set_current_element (const char* name);

	bool move_next ();
	bool parse_attribute ();
	bool handle_element_open ();
	bool handle_element_close (const char* name);
	bool handle_char_data ();

	AsxToken *next_non_whitespace_token ();

	void print_current_state ();
};


void
AsxTokenizer::move_next ()
{
	char c;

	while ((c = read_char ()) != -1) {

		if (in_data) {
			bool entirely_whitespace;
			current_token->set_type (TOKEN_DATA);
			current_token->set_value (read_data (c, entirely_whitespace));
			current_token->set_entirely_whitespace (entirely_whitespace);
			return;
		}

		if (c == OP_OPEN_ELEMENT) {
			if (is_comment_open (c)) {
				read_comment ();
				continue;
			}
			current_token->set_type (TOKEN_OPEN_ELEMENT);
			current_token->set_value (NULL);
			in_data = false;
			return;
		}

		if (c == OP_CLOSE_ELEMENT) {
			current_token->set_type (TOKEN_CLOSE_ELEMENT);
			current_token->set_value (NULL);
			in_data = true;
			return;
		}

		if (c == OP_ASSIGNMENT) {
			current_token->set_type (TOKEN_ASSIGNMENT);
			current_token->set_value (NULL);
			return;
		}

		if (c == OP_SLASH) {
			current_token->set_type (TOKEN_SLASH);
			current_token->set_value (NULL);
			return;
		}

		if (g_ascii_isspace (c)) {
			current_token->set_type (TOKEN_WHITESPACE);
			current_token->set_value (NULL);
			return;
		}

		if (is_quote_char (c)) {
			current_token->set_type (TOKEN_QUOTED_STRING);
			current_token->set_value (read_quoted_string (c));
			return;
		}

		if (is_name_char (c)) {
			current_token->set_type (TOKEN_NAME);
			current_token->set_value (read_name (c));
			return;
		}
	}

	current_token->set_type (TOKEN_EOF);
	current_token->set_value (NULL);
}

bool
AsxTokenizer::is_name_char (char c)
{
	return g_ascii_isalpha (c);
}

bool
AsxTokenizer::is_quote_char (char c)
{
	return c == '\'' || c == '"';
}

bool
AsxTokenizer::is_comment_open (char c)
{
	if (c != '<')
		return false;

	char d = read_char ();
	if (d != '!') {
		putback (d);
		return false;
	}
		
	char e = read_char ();
	if (e != '-') {
		putback (e);
		putback (d);
		return false;
	}

	char f = read_char ();
	if (f != '-') {
		putback (f);
		putback (e);
		putback (d);
		return false;
	}

	return true;
}

const char*
AsxTokenizer::read_name (char start)
{
	clear_buffer_str ();

	char c = start;
	while (is_name_char (c)) {
		g_string_append_c (buffer, c);
		c = read_char ();
	}

	putback (c);

	return buffer->str;
}

const char*
AsxTokenizer::read_data (char start, bool &entirely_whitespace)
{
	clear_buffer_str ();

	entirely_whitespace = true;

	char c = start;
	while (c != -1) {

		if (c == OP_OPEN_ELEMENT) {
			in_data = false;
			putback (c);
			break;
		}

		if (!g_ascii_isspace (c))
			entirely_whitespace = false;
		g_string_append_c (buffer, c);

		c = read_char ();
	}

	return buffer->str;
}

const char*
AsxTokenizer::read_quoted_string (char start)
{
	clear_buffer_str ();

	char c = read_char ();
	while (!stream->Eof () && c != start) {
		g_string_append_c (buffer, c);
		c = read_char ();
	} 

	if (stream->Eof ()) {
		
	}

	return buffer->str;
}

void
AsxTokenizer::read_comment ()
{
	static char close [] = "-->";

	char c;
	int match_ind = 0;

	while (c != TOKEN_EOF) {

		if (close [match_ind] == c) {
			match_ind++;

			if (match_ind == 3)
				break;
		} else
			match_ind = 0;

		c = read_char ();
	}
}

char
AsxTokenizer::read_char ()
{
	char c;

	if (putback_chars->len > 0) {
		c = putback_chars->str [putback_chars->len - 1];
		g_string_truncate (putback_chars, putback_chars->len - 1);
		return c;
	}

	if (stream->Read (&c, 1) != 1)
		return -1;

	col++;
	position++;

	if (c == '\r') {
		char d;
		if (stream->Read (&d, 1) != 1)
			return -1;
		if (d == '\n') {
			line++;
			col = 0;
			return d;
		}
		putback (d);
	}

	if (c == '\n') {
		line++;
		col = 0;
	}

	return c;
}

void
AsxTokenizer::putback (char c)
{
	g_string_append_c (putback_chars, c);
}

AsxToken *
AsxParserInternal::next_non_whitespace_token ()
{
	do {
		tokenizer->move_next ();
	} while (tokenizer->get_current ()->get_type () == TOKEN_WHITESPACE);

	return tokenizer->get_current ();
}

bool
AsxParserInternal::move_next ()
{
	clear_current_element ();

	bool is_element_close = false;

	AsxToken *tok = next_non_whitespace_token ();
	if (tok->get_type () == TOKEN_EOF)
		return false;

	if (tok->get_type () == TOKEN_DATA)
		return handle_char_data ();

	if (tok->get_type () != TOKEN_OPEN_ELEMENT) {
		raise_error (ASXPARSER_ERROR_INVALID_TOKEN, "Invalid char data found.");
		return false;
	}

	tok = next_non_whitespace_token ();
	if (tok->get_type () == TOKEN_SLASH) {
		is_element_close = true;
		tok = next_non_whitespace_token ();
	}

	if (tok->get_type () != TOKEN_NAME) {
		raise_error (ASXPARSER_ERROR_INVALID_TOKEN, "Invalid element formation, no name found.");
		return false;
	}

	if (is_element_close) {
		char *close = g_strdup (tokenizer->get_current ()->get_value ());
		tok = next_non_whitespace_token ();
		if (tok->get_type () != TOKEN_CLOSE_ELEMENT) {
			raise_error (ASXPARSER_ERROR_INVALID_TOKEN, "Invalid closing element found.");
			return false;
		}

		bool res = handle_element_close (close);
		g_free (close);
		return res;
	}

	set_current_element (tok->get_value ());

	bool is_self_closing = false;
	tok = next_non_whitespace_token ();
	while (tok->get_type () != TOKEN_EOF && tok->get_type () != TOKEN_CLOSE_ELEMENT) {

		if (tok->get_type () == TOKEN_SLASH) {
			tok = next_non_whitespace_token ();
			if (tok->get_type () != TOKEN_CLOSE_ELEMENT) {
				raise_error (ASXPARSER_ERROR_INVALID_TOKEN, "Invalid / character found in element.\n");
				return false;
			}
			is_self_closing = true;
			break;
		}

		if (tok->get_type () == TOKEN_NAME) {
			if (!parse_attribute ())
				return false;
		}

		tok = next_non_whitespace_token ();
	}

	handle_element_open ();

	if (is_self_closing)
		handle_element_close (current_element);

	return true;
}

bool
AsxParserInternal::parse_attribute ()
{
	char *name;
	char *value;

	AsxToken *tok = tokenizer->get_current ();
	if (tok->get_type () != TOKEN_NAME) {
		raise_error (ASXPARSER_ERROR_INVALID_TOKEN, "Invalid attribute, no name specified.");
		return false;
	}

	name = g_strdup (tok->get_value ());
	
	if (g_hash_table_lookup (current_attributes, name)) {
		raise_error (ASXPARSER_ERROR_DUPLICATE_ATTRIBUTE, "Invalid element, attribute specified twice.\n");
		return false;
	}

	tok = next_non_whitespace_token ();
	if (tok->get_type () != TOKEN_ASSIGNMENT) {
		raise_error (ASXPARSER_ERROR_INVALID_TOKEN, "Invalid attribute, = character not found.\n");
		return false;
	}

	tok = next_non_whitespace_token ();
	if (tok->get_type () != TOKEN_QUOTED_STRING) {
		raise_error (ASXPARSER_ERROR_QUOTE_EXPECTED, "Invalid attribute, value not found.\n");
		return false;
	}

	value = g_strdup (tok->get_value ());
	g_hash_table_insert (current_attributes, name, g_strdup (tok->get_value ()));

	return true;
}

bool
AsxParserInternal::handle_element_open ()
{
	g_queue_push_head (element_stack, g_strdup (current_element));

	if (start_element_handler)
		start_element_handler (parser, current_element, current_attributes);
	return true;
}

bool
AsxParserInternal::handle_element_close (const char* name)
{
	char *expected = (char *) g_queue_pop_head (element_stack);

	if (g_ascii_strcasecmp (expected, name)) {
		raise_error (ASXPARSER_ERROR_UNBALANCED_ELEMENTS, "Invalid closing element found.");
		return false;
	}

	g_free (expected);

	if (end_element_handler)
		end_element_handler (parser, current_element);
	return true;
}

bool
AsxParserInternal::handle_char_data ()
{
	AsxToken *tok = tokenizer->get_current ();

	if (tok->get_entirely_whitespace ())
		return true;

	if (text_handler)
		text_handler (parser, tokenizer->get_current ()->get_value ());
	return true;
}

void
AsxParserInternal::raise_error (AsxParserError code, const char* error)
{
	this->error_code = code;
	this->error_message = error;

	if (error_handler)
		error_handler (parser, code, error);
}

void
AsxParserInternal::clear_current_element ()
{
	g_hash_table_remove_all (current_attributes);
}

void
AsxParserInternal::set_current_element (const char* name)
{
	strncpy (current_element, name, MAX_ELEMENT_LEN);
}

bool
AsxParserInternal::parse_stream (TextStream *stream)
{
	tokenizer = new AsxTokenizer (stream);

	while (move_next ()) {
		if (stop_parsing)
			return false;
	}

	if (error_code != ASXPARSER_ERROR_NONE)
		return false;

	if (!g_queue_is_empty (element_stack)) {
		raise_error (ASXPARSER_ERROR_NO_ELEMENTS, "Unexpected end of file found.");
		return false;
	}

	return true;
}

void
AsxParserInternal::stop ()
{
	stop_parsing = true;
}

void
print_attribute (gpointer key, gpointer value, gpointer user)
{
	printf ("%s=%s ", (char *) key, (char *) value);
}

void
AsxParserInternal::print_current_state ()
{
	printf ("current element: %s ", current_element);

	g_hash_table_foreach (current_attributes, print_attribute, NULL);

	printf ("\n");
}

void
free_element (gpointer data, gpointer user)
{
	g_free (data);
}

AsxParserInternal::~AsxParserInternal ()
{
	if (tokenizer)
		delete tokenizer;

	g_queue_foreach (element_stack, free_element, NULL);
	g_queue_free (element_stack);

	g_hash_table_destroy (current_attributes);
}


AsxParser::AsxParser ()
{	
        this->parser = new AsxParserInternal (this);

	this->buffer = NULL;
	this->user_data = NULL;
}

AsxParser::~AsxParser ()
{
	delete parser;
}

void
AsxParser::SetErrorHandler (asx_error_handler *handler)
{
	parser->set_error_handler (handler);
}

void
AsxParser::SetTextHandler (asx_text_handler *handler)
{
	parser->set_text_handler (handler);
}

void
AsxParser::SetElementStartHandler (asx_element_start_handler *handler)
{
	parser->set_element_start_handler (handler);
}

void
AsxParser::SetElementEndHandler (asx_element_end_handler *handler)
{
	parser->set_element_end_handler (handler);
}

bool
AsxParser::ParseBuffer (MemoryBuffer *buffer)
{
	this->buffer = buffer;
	TextStream *stream = new TextStream ();
	buffer->SeekSet (0);
	stream->OpenBuffer ((char *) buffer->GetCurrentPtr (), buffer->GetSize ());

	return parser->parse_stream (stream);
}

void
AsxParser::Stop ()
{
	parser->stop ();
}


AsxParserError
AsxParser::GetErrorCode ()
{
	return parser->get_error_code ();
}


const char*
AsxParser::GetErrorMessage ()
{
	return parser->get_error_message ();
}


int
AsxParser::GetCurrentLineNumber ()
{
	return parser->get_current_line_number ();
}

int
AsxParser::GetCurrentColumnNumber ()
{
	return parser->get_current_column_number ();
}



#ifdef ASX_TEST

int main (int argc, const char* argv[])
{
	if (argc != 2) {
		printf ("pass in an ASX file to parse.\n");
		return -1;
	}

	TextStream *ts = new TextStream ();

	printf ("opening the file: %s\n", argv [1]);
	ts->OpenFile (argv [1], true);

	AsxParserInternal *asx = new AsxParserInternal (ts);
	asx->run ();
}

#endif

};