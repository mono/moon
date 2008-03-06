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
#include "eventargs.h"

class ErrorEventArgs  : public EventArgs  {
protected:
	virtual ~ErrorEventArgs ()
	{
		g_free (error_message);
	}


public:
	ErrorEventArgs (ErrorType type, int code, const char *msg)
	{
		error_type = type;
		error_code = code;
		error_message = g_strdup (msg);
	}
	
	virtual const char *GetTypeName () { return "ErrorEventArgs"; }

	int error_code;
	char *error_message;
	ErrorType error_type;
};

class ImageErrorEventArgs : public ErrorEventArgs {
protected:
	virtual ~ImageErrorEventArgs () {}

public:
	ImageErrorEventArgs (const char *msg)
	  : ErrorEventArgs (ImageError, 0, msg)
	{
	}

	virtual const char *GetTypeName () { return "ImageErrorEventArgs"; }
};

class ParserErrorEventArgs : public ErrorEventArgs {
protected:
	virtual ~ParserErrorEventArgs ()
	{
		g_free (xaml_file);
		g_free (xml_element);
		g_free (xml_attribute);
	}

public:
	ParserErrorEventArgs (const char *msg,
			      const char *file,
			      int line, int column,
			      int error_code, 
			      const char *element,
			      const char *attribute)
	  : ErrorEventArgs (ParserError, error_code, msg),
	  char_position (column),
	  line_number (line),
	  xaml_file (g_strdup(file)),
	  xml_element (g_strdup (element)),
	  xml_attribute (g_strdup (attribute))
	  {
	  }

	virtual const char *GetTypeName () { return "ParserErrorEventArgs"; }

	int char_position;
	int line_number;
	char *xaml_file;
	char *xml_element;
	char *xml_attribute;
};

#endif /* __MOON_ERROR_H__ */
