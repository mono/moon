/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-utils.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */


#include <config.h>

#include "plugin-class.h"

enum MethodArgType {
	MethodArgTypeNone   = (0),
	MethodArgTypeVoid   = (1 << NPVariantType_Void),
	MethodArgTypeNull   = (1 << NPVariantType_Null),
	MethodArgTypeBool   = (1 << NPVariantType_Bool),
	MethodArgTypeInt32  = (1 << NPVariantType_Int32),
	MethodArgTypeDouble = (1 << NPVariantType_Double),
	MethodArgTypeString = (1 << NPVariantType_String),
	MethodArgTypeObject = (1 << NPVariantType_Object),
	MethodArgTypeAny    = (0xff)
};

static MethodArgType
decode_arg_ctype (char c)
{
	switch (c) {
	case 'v': return MethodArgTypeVoid;
	case 'n': return MethodArgTypeNull;
	case 'b': return MethodArgTypeBool;
	case 'i': return MethodArgTypeInt32;
	case 'd': return MethodArgTypeDouble;
	case 's': return MethodArgTypeString;
	case 'o': return MethodArgTypeObject;
	case '*': return MethodArgTypeAny;
	default:
		return MethodArgTypeNone;
	}
}

static MethodArgType
decode_arg_type (const char **in)
{
	MethodArgType t, type = MethodArgTypeNone;
	register const char *inptr = *in;
	
	if (*inptr == '(') {
		inptr++;
		while (*inptr && *inptr != ')') {
			t = decode_arg_ctype (*inptr);
			type = (MethodArgType) ((int) type | (int) t);
			inptr++;
		}
	} else {
		type = decode_arg_ctype (*inptr);
	}
	
	inptr++;
	*in = inptr;
	
	return type;
}

/**
 * check_arg_list:
 * @arglist: a string representing an arg-list token (see grammar below)
 * @args: NPVariant argument count
 * @argv: NPVariant argument vector
 *
 * Checks that the NPVariant arguments satisfy the argument count and
 * types expected (provided via @typestr).
 *
 * The @typestr argument should follow the following syntax:
 *
 * simple-arg-type ::= "v" / "n" / "b" / "i" / "d" / "s" / "o" / "*"
 *                     ; each char represents one of the following
 *                     ; NPVariant types: Void, Null, Bool, Int32,
 *                     ; Double, String, Object and wildcard
 *
 * arg-type        ::= simple-arg-type / "(" 1*(simple-arg-type) ")"
 *
 * optional-args   ::= "[" *(arg-type) "]"
 *
 * arg-list        ::= *(arg-type) (optional-args)
 *
 *
 * Returns: %true if @argv matches the arg-list criteria specified in
 * @arglist or %false otherwise.
 **/
bool
check_arg_list (const char *arglist, guint32 argc, const NPVariant *argv)
{
	const char *inptr = arglist;
	MethodArgType mask;
	guint32 i = 0;
	
	// check all of the required arguments
	while (*inptr && *inptr != '[' && i < argc) {
		mask = decode_arg_type (&inptr);
		if (!(mask & (1 << argv[i].type))) {
			// argv[i] does not match any of the expected types
			return false;
		}
		
		i++;
	}
	
	if (*inptr && *inptr != '[' && i < argc) {
		// we were not provided enough arguments
		return false;
	}
	
	// now check all of the optional arguments
	inptr++;
	while (*inptr && *inptr != ']' && i < argc) {
		mask = decode_arg_type (&inptr);
		if (!(mask & (1 << argv[i].type))) {
			// argv[i] does not match any of the expected types
			return false;
		}
		
		i++;
	}
	
	if (i < argc) {
		// we were provided too many arguments
		return false;
	}
	
	return true;
}
