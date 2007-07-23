/*
 * error.h: ErrorEventArgs and its subclasses
 *
 * Authors:
 *   Jackson Harper (jackson@ximian.com)
 *   Chris Toshok (toshok@ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_ERROR_H__
#define __MOON_ERROR_H__

#include "enums.h"

struct ErrorEventArgs /* : public EventArgs */ {
 public:
	int error_code;;
	const char *error_message;
	ErrorType error_type;
};

struct ParserErrorEventArgs : public ErrorEventArgs {
 public:

	ParserErrorEventArgs () : char_position (0), line_number (0), xaml_file (NULL),
	xml_element (NULL), xml_attribute (NULL)
	{
		error_type = ParserError;
	}
	
	
	int char_position;
	int line_number;
	const char *xaml_file;
	const char *xml_element;
	const char *xml_attribute;
};

#endif /* __MOON_ERROR_H__ */
