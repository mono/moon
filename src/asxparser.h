

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

class AsxParser;
class AsxParserInternal;

typedef void asx_text_handler (AsxParser *parser, const char* text);
typedef void asx_error_handler (AsxParser *parser, const char* error);
typedef void asx_element_start_handler (AsxParser *parser, const char* element, GHashTable *atts);
typedef void asx_element_end_handler (AsxParser *parser, const char* element); 



class AsxParser {

public:
	AsxParser (MemoryBuffer *buffer);
	~AsxParser ();

	void set_error_handler (asx_error_handler *handler);
	void set_text_handler (asx_text_handler *handler);
	void set_element_start_handler (asx_element_start_handler *handler);
	void set_element_end_handler (asx_element_end_handler *handler);

	void Run ();
	void Stop ();
	
private:
	AsxParserInternal *parser;

	MemoryBuffer *buffer;
	TextStream *stream;
};


#endif /*  __MOON_ASXPARSER_H__ */
