

/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * asxparser.h: ASX parser
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_ASXPARSER_H__
#define __MOON_ASXPARSER_H__

#include <glib.h>
#include "pipeline.h"

namespace Moonlight {

class AsxParser;
class AsxParserInternal;

typedef void asx_text_handler (AsxParser *parser, const char* text);
typedef void asx_error_handler (AsxParser *parser, int error_code, const char* error);
typedef void asx_element_start_handler (AsxParser *parser, const char* element, GHashTable *atts);
typedef void asx_element_end_handler (AsxParser *parser, const char* element); 

enum AsxParserError {
	ASXPARSER_ERROR_NONE,
	ASXPARSER_ERROR_INVALID_TOKEN,	
	ASXPARSER_ERROR_DUPLICATE_ATTRIBUTE,
	ASXPARSER_ERROR_NO_ELEMENTS,
	ASXPARSER_ERROR_UNBALANCED_ELEMENTS,
	ASXPARSER_ERROR_QUOTE_EXPECTED
};

class AsxParser {

public:
	AsxParser ();
	~AsxParser ();

	void SetErrorHandler (asx_error_handler *handler);
	void SetTextHandler (asx_text_handler *handler);
	void SetElementStartHandler (asx_element_start_handler *handler);
	void SetElementEndHandler (asx_element_end_handler *handler);

	bool ParseBuffer (MemoryBuffer *buffer);
	void Stop ();

	void SetUserData (void *data)
	{
		user_data = data;
	}

	void* GetUserData ()
	{
		return user_data;
	}

	AsxParserError GetErrorCode ();
	const char* GetErrorMessage ();

	int GetCurrentLineNumber ();
	int GetCurrentColumnNumber ();
private:
	AsxParserInternal *parser;

	MemoryBuffer *buffer;
	void *user_data;
};

};
#endif /*  __MOON_ASXPARSER_H__ */
