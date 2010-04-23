/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * getopts.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */


#ifndef __GETOPTS_H__
#define __GETOPTS_H__

typedef struct _GetOptsContext GetOptsContext;

enum GetOptsArgFlags {
	GETOPTS_NO_ARG          = 0,
	GETOPTS_OPTIONAL_ARG    = (1 << 0),
	GETOPTS_REQUIRED_ARG    = (1 << 1),
	
	GETOPTS_ARG_IN_SHORT    = (1 << 4),  /* for e.g. -Wall "all" is in the short option, -W */
#define GETOPTS_FLAG_MASK         0x000000ff
	
	GETOPTS_ARG_NONE        = 0,
	GETOPTS_ARG_BOOL        = (1 << 8),
	GETOPTS_ARG_INT8        = (1 << 9),
	GETOPTS_ARG_UINT8       = (1 << 10),
	GETOPTS_ARG_INT16       = (1 << 11),
	GETOPTS_ARG_UINT16      = (1 << 12),
	GETOPTS_ARG_INT32       = (1 << 13),
	GETOPTS_ARG_UINT32      = (1 << 14),
	GETOPTS_ARG_INT64       = (1 << 15),
	GETOPTS_ARG_UINT64      = (1 << 16),
	GETOPTS_ARG_FLOAT       = (1 << 17),
	GETOPTS_ARG_DOUBLE      = (1 << 18),
	GETOPTS_ARG_STRING      = (1 << 19),
	GETOPTS_ARG_CUSTOM      = (1 << 20),
#define GETOPTS_ARG_MASK          0xffffff00
};

/* convenience macros */
#define GETOPTS_ARG_CHAR              GETOPTS_ARG_INT8
#define GETOPTS_ARG_INT               GETOPTS_ARG_INT32
#define GETOPTS_OPTIONAL_BOOL_ARG     (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_BOOL)
#define GETOPTS_OPTIONAL_CHAR_ARG     (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_CHAR)
#define GETOPTS_OPTIONAL_INT8_ARG     (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_INT8)
#define GETOPTS_OPTIONAL_UINT8_ARG    (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_UINT8)
#define GETOPTS_OPTIONAL_INT16_ARG    (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_INT16)
#define GETOPTS_OPTIONAL_UINT16_ARG   (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_UINT16)
#define GETOPTS_OPTIONAL_INT_ARG      (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_INT)
#define GETOPTS_OPTIONAL_INT32_ARG    (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_INT32)
#define GETOPTS_OPTIONAL_UINT32_ARG   (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_UINT32)
#define GETOPTS_OPTIONAL_INT64_ARG    (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_INT64)
#define GETOPTS_OPTIONAL_UINT64_ARG   (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_UINT64)
#define GETOPTS_OPTIONAL_FLOAT_ARG    (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_FLOAT)
#define GETOPTS_OPTIONAL_DOUBLE_ARG   (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_DOUBLE)
#define GETOPTS_OPTIONAL_STRING_ARG   (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_STRING)
#define GETOPTS_OPTIONAL_CUSTOM_ARG   (GetOptsArgFlags) (GETOPTS_OPTIONAL_ARG | GETOPTS_ARG_CUSTOM)
#define GETOPTS_REQUIRED_BOOL_ARG     (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_BOOL)
#define GETOPTS_REQUIRED_CHAR_ARG     (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_CHAR)
#define GETOPTS_REQUIRED_INT8_ARG     (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_INT8)
#define GETOPTS_REQUIRED_UINT8_ARG    (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_UINT8)
#define GETOPTS_REQUIRED_INT16_ARG    (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_INT16)
#define GETOPTS_REQUIRED_UINT16_ARG   (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_UINT16)
#define GETOPTS_REQUIRED_INT_ARG      (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_INT)
#define GETOPTS_REQUIRED_INT32_ARG    (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_INT32)
#define GETOPTS_REQUIRED_UINT32_ARG   (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_UINT32)
#define GETOPTS_REQUIRED_INT64_ARG    (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_INT64)
#define GETOPTS_REQUIRED_UINT64_ARG   (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_UINT64)
#define GETOPTS_REQUIRED_FLOAT_ARG    (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_FLOAT)
#define GETOPTS_REQUIRED_DOUBLE_ARG   (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_DOUBLE)
#define GETOPTS_REQUIRED_STRING_ARG   (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_STRING)
#define GETOPTS_REQUIRED_CUSTOM_ARG   (GetOptsArgFlags) (GETOPTS_REQUIRED_ARG | GETOPTS_ARG_CUSTOM)

enum GetOptsFlags {
	GETOPTS_FLAG_DEFAULT               = 0,
	GETOPTS_FLAG_ALLOW_SINGLE_DASH     = (1 << 0),
	GETOPTS_FLAG_BREAK_ON_FIRST_NONARG = (1 << 1),
};

typedef struct _GetOptsOption GetOptsOption;

/* return 0 on success or -1 on error */
typedef int (* GetOptsCustomArgFunc) (GetOptsContext *ctx, GetOptsOption *opt, const char *arg, void *user_data);

#define GETOPTS_COMPLETE          -1
#define GETOPTS_ERROR_BAD_OPTION  -2
#define GETOPTS_ERROR_MISSING_ARG -3
#define GETOPTS_ERROR_INVALID_ARG -4

struct _GetOptsOption {
	const char *long_name;
	char short_name;
	GetOptsArgFlags flags;
	const char *description;
	const char *arg_descrip;
	int id;
	
	GetOptsCustomArgFunc custom;
	void *value;
};

#define GETOPTS_TABLE_END { NULL, '\0', GETOPTS_NO_ARG, NULL, NULL, 0, NULL, NULL }

GetOptsContext *getopts_context_new (int argc, char **argv, GetOptsOption *options, GetOptsFlags flags);
void getopts_context_free (GetOptsContext *ctx, int freeargs);
void getopts_context_reset (GetOptsContext *ctx);

void getopts_print_help (GetOptsContext *ctx);

int getopts_get_next_opt (GetOptsContext *ctx);
const char *getopts_get_opt_arg (GetOptsContext *ctx);

void getopts_parse_args (GetOptsContext *ctx);

int getopts_get_next_index (GetOptsContext *ctx);

const char **getopts_get_args (GetOptsContext *ctx, int *nargs);

void getopts_perror (GetOptsContext *ctx, int err);

#endif /* __GETOPTS_H__ */
