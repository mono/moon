/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  Alleyoop
 *  Copyright (C) 2003-2006 Jeffrey Stedfast
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "getopts.h"

struct _GetOptsContext {
	GetOptsOption *options;
	GetOptsFlags flags;
	char **argv;
	int argc;
	
	/* unused args */
	char **args;
	int nargs;
	
	int index;
	int subindex;
	int argindex;
	int lastopt;
	int err;
	
	const char *arg;
};


static const char *
base_name (const char *path)
{
	const char *p;
	
	if ((p = strrchr (path, '/')))
		return p + 1;
	
	return path;
}


GetOptsContext *
getopts_context_new (int argc, char **argv, GetOptsOption *options, GetOptsFlags flags)
{
	GetOptsContext *ctx;
	
	if (!(ctx = (GetOptsContext *) malloc (sizeof (GetOptsContext))))
		return NULL;
	
	if (!(ctx->args = (char **) malloc (sizeof (char *) * (argc + 1)))) {
		free (ctx);
		return NULL;
	}
	
	ctx->options = options;
	ctx->flags = flags;
	ctx->argv = argv;
	ctx->argc = argc;
	ctx->nargs = 0;
	
	ctx->index = 0;
	ctx->subindex = 0;
	ctx->argindex = 0;
	ctx->lastopt = 0;
	ctx->err = 0;
	
	ctx->arg = NULL;
	
	return ctx;
}


void
getopts_context_free (GetOptsContext *ctx, int freeargs)
{
	if (freeargs)
		free (ctx->args);
	free (ctx);
}


void
getopts_context_reset (GetOptsContext *ctx)
{
	ctx->nargs = 0;
	ctx->index = 0;
	ctx->subindex = 0;
	ctx->argindex = 0;
	ctx->lastopt = 0;
	ctx->err = 0;
	
	ctx->arg = NULL;
}

enum {
	HAS_NO_OPTS    = 0x0,
	HAS_SHORT_OPTS = 0x1,
	HAS_LONG_OPTS  = 0x2,
	HAS_BOTH_OPTS  = 0x3,
};

static void
get_widths (GetOptsOption *options, uint32_t *hasp, size_t *opts_width, size_t *desc_width)
{
	uint32_t has = HAS_NO_OPTS;
	size_t n, max = 0;
	uint32_t flags;
	int i;
	
	for (i = 0; options[i].id != 0; i++) {
		flags = options[i].flags & GETOPTS_FLAG_MASK;
		
		if (options[i].short_name) {
			if (options[i].arg_descrip && !options[i].long_name) {
				n = strlen (options[i].arg_descrip);
				if (options[i].flags & GETOPTS_OPTIONAL_ARG)
					n += 2;
				if (!(options[i].flags & GETOPTS_ARG_IN_SHORT))
					n++;
				
				/* compensate for the fact that the
				 * short_name column ends 2 chars
				 * before long_name column begins */
				n = n >= 4 ? n - 4 : 0;
				
				max = n > max ? n : max;
			}
			
			has |= HAS_SHORT_OPTS;
		}
		
		if (options[i].long_name) {
			n = strlen (options[i].long_name);
			if (options[i].arg_descrip && flags && !(flags & GETOPTS_ARG_IN_SHORT)) {
				if (flags & GETOPTS_REQUIRED_ARG)
					n += strlen (options[i].arg_descrip) + 1;
				else
					n += strlen (options[i].arg_descrip) + 3;
			}
			
			max = n > max ? n : max;
			has |= HAS_LONG_OPTS;
		}
	}
	
	n = 0;
	if (has != 0) {
		n = 2; /* left margin */
		
		if (has & HAS_SHORT_OPTS)
			n += 2 /* "-%c" */ + ((has & HAS_LONG_OPTS) ? 2 /* ", " */ : 0);
		
		if (has & HAS_LONG_OPTS)
			n += 2 + max; /* "--%s" */
		
		n += 2; /* right margin */
	}
	
	*hasp = has;
	*opts_width = n;
	*desc_width = 80 - n;
}


void
getopts_print_help (GetOptsContext *ctx)
{
	size_t opts_width, desc_width;
	register const char *inptr;
	const char *start, *lwsp;
	size_t width, n;
	uint32_t flags;
	uint32_t has;
	int i;
	
	get_widths (ctx->options, &has, &opts_width, &desc_width);
	
	for (i = 0; ctx->options[i].id != 0; i++) {
		if (ctx->options[i].short_name) {
			if (ctx->options[i].arg_descrip && !ctx->options[i].long_name) {
				/* we don't have a long_name so we have room to put an arg_descrip */
				if (ctx->options[i].flags & GETOPTS_ARG_IN_SHORT) {
					if (ctx->options[i].flags & GETOPTS_REQUIRED_ARG) {
						width = printf ("  -%c%s", ctx->options[i].short_name,
								ctx->options[i].arg_descrip);
					} else {
						width = printf ("  -%c[%s]", ctx->options[i].short_name,
								ctx->options[i].arg_descrip);
					}
				} else {
					if (ctx->options[i].flags & GETOPTS_REQUIRED_ARG) {
						width = printf ("  -%c %s", ctx->options[i].short_name,
								ctx->options[i].arg_descrip);
					} else {
						width = printf ("  -%c [%s]", ctx->options[i].short_name,
								ctx->options[i].arg_descrip);
					}
				}
			} else {
				printf ("  -%c%s ", ctx->options[i].short_name,
					ctx->options[i].long_name ? "," : " ");
				width = 6;
			}
		} else if (has == HAS_BOTH_OPTS) {
			fputs ("      ", stdout);
			width = 6;
		} else {
			fputs ("  ", stdout);
			width = 2;
		}
		
		if (ctx->options[i].long_name) {
			flags = ctx->options[i].flags & GETOPTS_FLAG_MASK;
			width += printf ("--%s", ctx->options[i].long_name);
			if (ctx->options[i].arg_descrip && flags && !(flags & GETOPTS_ARG_IN_SHORT)) {
				if (flags & GETOPTS_REQUIRED_ARG)
					width += printf ("=%s", ctx->options[i].arg_descrip);
				else
					width += printf ("[=%s]", ctx->options[i].arg_descrip);
			}
		}
		
		inptr = ctx->options[i].description;
		while (*inptr != '\0') {
			while (width < opts_width) {
				fputc (' ', stdout);
				width++;
			}
			
			lwsp = NULL;
			start = inptr;
			while (*inptr != '\0' && (size_t) (inptr - start) < desc_width) {
				if (*inptr == ' ')
					lwsp = inptr;
				inptr++;
			}
			
			if (!lwsp)
				lwsp = inptr;
			
			if (*inptr != '\0') {
				n = lwsp - start;
				inptr = lwsp + 1;
			} else {
				n = inptr - start;
			}
			
			fwrite (start, 1, n, stdout);
			fputc ('\n', stdout);
			width = 0;
		}
	}
	
	fflush (stdout);
}


static int
decode_bool (GetOptsContext *ctx, GetOptsOption *opt, const char *arg, void *valuep)
{
	if (!arg || !strcmp (arg, "yes") || !strcmp (arg, "true")) {
		*((int *) valuep) = 1;
		return 0;
	}
	
	if (!strcmp (arg, "no") || !strcmp (arg, "false")) {
		*((int *) valuep) = 0;
		return 0;
	}
	
	ctx->err = EINVAL;
	
	return -1;
}

#define STRTOINT(bits, max)                                             \
static int                                                              \
strtoint##bits (const char *in, char **inend, int##bits##_t *retval, int *err)    \
{                                                                       \
	register const char *inptr = in;                                \
	register size_t digit;                                          \
	int##bits##_t val = 0;                                          \
	int sign = 1;                                                   \
	                                                                \
	while (*inptr == ' ')                                           \
		inptr++;                                                \
	                                                                \
	if (*inptr == '-') {                                            \
		sign = -1;                                              \
		inptr++;                                                \
	}                                                               \
	                                                                \
	in = inptr;                                                     \
	while (*inptr >= '0' && *inptr <= '9') {                        \
		digit = (*inptr - '0');                                 \
		if (val > (max / 10)) {                                 \
			*err = EOVERFLOW;                               \
			return -1;                                      \
		} else if (val == (max / 10)) {                         \
			if (digit > (max % 10) &&                       \
			    (sign > 0 || digit > ((max % 10) + 1))) {   \
				*err = EOVERFLOW;                       \
				return -1;                              \
			}                                               \
			                                                \
			if (sign < 0)                                   \
				val = (val * sign * 10) - digit;        \
			else                                            \
				val = (val * 10) + digit;               \
			                                                \
			inptr++;                                        \
			if (*inptr >= '0' && *inptr <= '9') {           \
				*err = EOVERFLOW;                       \
				return -1;                              \
			}                                               \
 			                                                \
			goto ret;                                       \
		} else {                                                \
			val = (val * 10) + digit;                       \
		}                                                       \
		                                                        \
		inptr++;                                                \
	}                                                               \
	                                                                \
	if (inptr == in) {                                              \
		*err = EINVAL;                                          \
		return -1;                                              \
	}                                                               \
	                                                                \
	val *= sign;                                                    \
	                                                                \
 ret:                                                                   \
	*inend = (char *) inptr;                                        \
	*retval = val;                                                  \
	                                                                \
	return 0;                                                       \
}

STRTOINT(8, 127)
STRTOINT(16, 32767)
STRTOINT(32, 2147483647L)
STRTOINT(64, 9223372036854775807LL)

#define STRTOUINT(bits, max)                                            \
static int                                                              \
strtouint##bits (const char *in, char **inend, uint##bits##_t *retval, int *err)  \
{                                                                       \
	register const char *inptr = in;                                \
	register size_t digit;                                          \
	uint##bits##_t val = 0;                                         \
	                                                                \
	while (*inptr == ' ')                                           \
		inptr++;                                                \
	                                                                \
	in = inptr;                                                     \
	while (*inptr >= '0' && *inptr <= '9') {                        \
		digit = (*inptr - '0');                                 \
		if (val > (max / 10)) {                                 \
			*err = EOVERFLOW;                               \
			return -1;                                      \
		} else if (val == (max / 10) && digit > (max % 10)) {   \
		       	*err = EOVERFLOW;                               \
			return -1;                                      \
		} else {                                                \
			val = (val * 10) + digit;                       \
		}                                                       \
		                                                        \
		inptr++;                                                \
	}                                                               \
	                                                                \
	if (inptr == in) {                                              \
		*err = EINVAL;                                          \
		return -1;                                              \
	}                                                               \
	                                                                \
	*inend = (char *) inptr;                                        \
	*retval = val;                                                  \
	                                                                \
	return 0;                                                       \
}

STRTOUINT(8, 255)
STRTOUINT(16, 65535)
STRTOUINT(32, 4294967295UL)
STRTOUINT(64, 18446744073709551615ULL)

#define DECODE_INT(bits)                                                                   \
static int                                                                                 \
decode_int##bits (GetOptsContext *ctx, GetOptsOption *opt, const char *arg, void *valuep)  \
{                                                                                          \
	char *endptr;                                                                      \
	                                                                                   \
	if ((strtoint##bits (arg, &endptr, (int##bits##_t *) valuep, &ctx->err) == -1))    \
		return -1;                                                                 \
	                                                                                   \
	if (*endptr != '\0') {                                                             \
		ctx->err = EINVAL;                                                         \
		return -1;                                                                 \
	}                                                                                  \
	                                                                                   \
	return 0;                                                                          \
}

DECODE_INT(8)
DECODE_INT(16)
DECODE_INT(32)
DECODE_INT(64)

#define DECODE_UINT(bits)                                                                  \
static int                                                                                 \
decode_uint##bits (GetOptsContext *ctx, GetOptsOption *opt, const char *arg, void *valuep) \
{                                                                                          \
	char *endptr;                                                                      \
	                                                                                   \
	if ((strtouint##bits (arg, &endptr, (uint##bits##_t *) valuep, &ctx->err) == -1))  \
		return -1;                                                                 \
	                                                                                   \
	if (*endptr != '\0') {                                                             \
		ctx->err = EINVAL;                                                         \
		return -1;                                                                 \
	}                                                                                  \
	                                                                                   \
	return 0;                                                                          \
}

DECODE_UINT(8)
DECODE_UINT(16)
DECODE_UINT(32)
DECODE_UINT(64)

static int
decode_float (GetOptsContext *ctx, GetOptsOption *opt, const char *arg, void *valuep)
{
	char *endptr;
	float val;
	
	val = (float) strtod (arg, &endptr);
	if (endptr == (char *) arg || *endptr != '\0')
		return -1;
	
	*((float *) valuep) = val;
	
	return 0;
}

static int
decode_double (GetOptsContext *ctx, GetOptsOption *opt, const char *arg, void *valuep)
{
	char *endptr;
	double val;
	
	val = strtod (arg, &endptr);
	if (endptr == (char *) arg || *endptr != '\0')
		return -1;
	
	*((double *) valuep) = val;
	
	return 0;
}

static int
decode_string (GetOptsContext *ctx, GetOptsOption *opt, const char *arg, void *valuep)
{
	*((char **) valuep) = (char *) arg;
	
	return 0;
}

static int
decode_arg (GetOptsContext *ctx, GetOptsOption *opt, const char *arg)
{
	void *valuep = opt->value;
	int rc;
	
	if (opt->value == NULL && !(opt->flags & GETOPTS_ARG_CUSTOM)) {
		ctx->arg = arg;
		return opt->id;
	}
	
	switch (opt->flags & GETOPTS_ARG_MASK) {
	case GETOPTS_ARG_NONE:
	case GETOPTS_ARG_BOOL:
		rc = decode_bool (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_INT8:
		rc = decode_int8 (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_UINT8:
		rc = decode_uint8 (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_INT16:
		rc = decode_int16 (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_UINT16:
		rc = decode_uint16 (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_INT32:
		rc = decode_int32 (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_UINT32:
		rc = decode_uint32 (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_INT64:
		rc = decode_int64 (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_UINT64:
		rc = decode_uint64 (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_FLOAT:
		rc = decode_float (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_DOUBLE:
		rc = decode_double (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_STRING:
		rc = decode_string (ctx, opt, arg, valuep);
		break;
	case GETOPTS_ARG_CUSTOM:
		rc = opt->custom (ctx, opt, arg, valuep);
		break;
	default:
		return GETOPTS_ERROR_INVALID_ARG;
	}
	
	if (rc == -1)
		return GETOPTS_ERROR_INVALID_ARG;
	
	return opt->id;
}


#define GETOPTS_CAN_HAVE_ARG(x) ((x) & (GETOPTS_OPTIONAL_ARG | GETOPTS_REQUIRED_ARG))

int
getopts_get_next_opt (GetOptsContext *ctx)
{
	const char *long_opt, *arg, *eq;
	size_t n, len;
	int i;
	
	ctx->err = 0;
	
	if (ctx->subindex == 0) {
		if (ctx->argindex > ctx->index)
			ctx->index = ctx->argindex + 1;
		else
			ctx->index++;
	}
	
	while (ctx->index < ctx->argc) {
		if (!strncmp (ctx->argv[ctx->index], "--", 2)) {
			ctx->lastopt = ctx->index;
			if (ctx->argv[ctx->index][2] == '\0')
				break;
			
			eq = long_opt = ctx->argv[ctx->index] + 2;
			while (*eq && *eq != '=')
				eq++;
			
			n = eq - long_opt;
			
			for (i = 0; ctx->options[i].id != 0; i++) {
				len = ctx->options[i].long_name ? strlen (ctx->options[i].long_name) : 0;
				if (len == n && !strncmp (long_opt, ctx->options[i].long_name, n))
					goto get_long_arg;
			}
			
			return GETOPTS_ERROR_BAD_OPTION;
		} else if (ctx->argv[ctx->index][0] == '-') {
			ctx->lastopt = ctx->index;
			long_opt = ctx->argv[ctx->index] + 1;
			if (ctx->subindex == 0 && (ctx->flags & GETOPTS_FLAG_ALLOW_SINGLE_DASH) && *long_opt) {
				eq = long_opt;
				while (*eq && *eq != '=')
					eq++;
				
				n = eq - long_opt;
				
				for (i = 0; ctx->options[i].id != 0; i++) {
					len = ctx->options[i].long_name ? strlen (ctx->options[i].long_name) : 0;
					if (len == n && !strncmp (long_opt, ctx->options[i].long_name, n))
						goto get_long_arg;
				}
				
				/* fall thru */
			}
			
			ctx->subindex++;
			
			if (ctx->argv[ctx->index][ctx->subindex] != '\0') {
				for (i = 0; ctx->options[i].id != 0; i++) {
					if (ctx->argv[ctx->index][ctx->subindex] == ctx->options[i].short_name)
						goto get_arg;
				}
				
				return GETOPTS_ERROR_BAD_OPTION;
			}
		} else if (ctx->flags & GETOPTS_FLAG_BREAK_ON_FIRST_NONARG) {
			break;
		} else {
			/* keep track of unused args */
			ctx->args[ctx->nargs++] = ctx->argv[ctx->index];
		}
		
		ctx->index++;
		ctx->subindex = 0;
	}
	
	return -1;
	
 get_long_arg:
	
	if (!GETOPTS_CAN_HAVE_ARG (ctx->options[i].flags))
		return decode_arg (ctx, &ctx->options[i], NULL);
	
	if (*eq != '\0') {
		return decode_arg (ctx, &ctx->options[i], eq + 1);
	} else if (ctx->index + 1 < ctx->argc && ctx->argv[ctx->index + 1][0] != '-') {
		ctx->argindex = ctx->index + 1;
		arg = ctx->argv[ctx->argindex];
		
		return decode_arg (ctx, &ctx->options[i], arg);
	}
	
	if (ctx->options[i].flags & GETOPTS_REQUIRED_ARG)
		return GETOPTS_ERROR_MISSING_ARG;
	
	return decode_arg (ctx, &ctx->options[i], NULL);
	
 get_arg:
	
	if (!GETOPTS_CAN_HAVE_ARG (ctx->options[i].flags))
		return decode_arg (ctx, &ctx->options[i], NULL);
	
	if (ctx->argindex < ctx->index)
		ctx->argindex = ctx->index;
	
	if (ctx->options[i].flags & GETOPTS_ARG_IN_SHORT) {
		arg = ctx->argv[ctx->index] + ctx->subindex + 1;
		ctx->subindex = 0;
		
		if (arg[0] == '\0' && (ctx->options[i].flags & GETOPTS_REQUIRED_ARG))
			return GETOPTS_ERROR_MISSING_ARG;
		
		return decode_arg (ctx, &ctx->options[i], arg);
	}
	
	if (ctx->argindex + 1 < ctx->argc && ctx->argv[ctx->argindex + 1][0] != '-') {
		ctx->argindex++;
		arg = ctx->argv[ctx->argindex];
		
		return decode_arg (ctx, &ctx->options[i], arg);
	}
	
	if (ctx->options[i].flags & GETOPTS_REQUIRED_ARG)
		return GETOPTS_ERROR_MISSING_ARG;
	
	return decode_arg (ctx, &ctx->options[i], NULL);
}


const char *
getopts_get_opt_arg (GetOptsContext *ctx)
{
	const char *arg;
	
	arg = ctx->arg;
	ctx->arg = NULL;
	
	return arg;
}


void
getopts_parse_args (GetOptsContext *ctx)
{
	int rc;
	
	while ((rc = getopts_get_next_opt (ctx)) != -1) {
		switch (rc) {
		case GETOPTS_ERROR_BAD_OPTION:
		case GETOPTS_ERROR_MISSING_ARG:
		case GETOPTS_ERROR_INVALID_ARG:
			getopts_perror (ctx, rc);
			exit (1);
			break;
		default:
			break;
		}
	}
}


int
getopts_get_next_index (GetOptsContext *ctx)
{
	return ctx->index;
}


const char **
getopts_get_args (GetOptsContext *ctx, int *nargs)
{
	int i, n;
	
	n = ctx->nargs;
	i = getopts_get_next_index (ctx);
	while (i < ctx->argc)
		ctx->args[n++] = ctx->argv[i++];
	ctx->args[n] = NULL;
	
	if (nargs)
		*nargs = n;
	
	return (const char **) ctx->args;
}


void
getopts_perror (GetOptsContext *ctx, int err)
{
	const char *long_opt, *eq;
	const char *reason;
	
	if (ctx->index >= ctx->argc || ctx->argv[ctx->index][0] != '-')
		return;
	
	if (ctx->subindex == 0) {
		if (!strncmp (ctx->argv[ctx->index], "--", 2))
			long_opt = ctx->argv[ctx->index] + 2;
		else
			long_opt = ctx->argv[ctx->index] + 1;
		
		eq = long_opt;
		while (*eq && *eq != '=')
			eq++;
	}
	
	switch (err) {
	case GETOPTS_ERROR_BAD_OPTION:
		if (ctx->subindex == 0) {
			fprintf (stderr, "%s: unrecognized option `--%.*s'\n",
				 base_name (ctx->argv[0]), (int) (eq - long_opt), long_opt);
		} else {
			fprintf (stderr, "%s: unrecognized option -- %c\n",
				 base_name (ctx->argv[0]), ctx->argv[ctx->index][ctx->subindex]);
		}
		break;
	case GETOPTS_ERROR_MISSING_ARG:
		if (ctx->subindex == 0) {
			fprintf (stderr, "%s: option `--%.*s' requires an argument\n",
				 base_name (ctx->argv[0]), (int) (eq - long_opt), long_opt);
		} else {
			fprintf (stderr, "%s: option requires an argument -- %c\n",
				 base_name (ctx->argv[0]), ctx->argv[ctx->index][ctx->subindex]);
		}
		break;
	case GETOPTS_ERROR_INVALID_ARG:
		switch (ctx->err) {
		case EOVERFLOW:
			reason = "overflow error";
			break;
		default:
			reason = NULL;
		}
		
		if (ctx->subindex == 0) {
			fprintf (stderr, "%s: invalid argument passed to option `--%.*s'%s%s\n",
				 base_name (ctx->argv[0]), (int) (eq - long_opt), long_opt,
				 reason ? ": " : "", reason ? reason : "");
		} else {
			fprintf (stderr, "%s: invalid argument passed to option -- %c%s%s\n",
				 base_name (ctx->argv[0]), ctx->argv[ctx->index][ctx->subindex],
				 reason ? ": " : "", reason ? reason : "");
		}
		break;
	default:
		break;
	}
}


#ifdef TEST_SUITE
static int
display_help (GetOptsContext *ctx, GetOptsOption *opt, const char *arg, void *valuep)
{
	printf ("Usage: %s [OPTIONS...]\n\n", base_name (ctx->argv[0]));
	getopts_print_help (ctx);
	exit (0);
	
	return 0;
}

static int
display_version (GetOptsContext *ctx, GetOptsOption *opt, const char *arg, void *valuep)
{
	printf ("version 1.0.0\n");
	exit (0);
	
	return 0;
}

static int really_long_arg = 0;
static const char *required_arg = NULL;
static const char *optional_arg = NULL;
static const char *a = NULL;
static const char *b = NULL;
static const char *c = NULL;
static int O = 0;
static const char *W = NULL;
static int bool = 0;
static int64_t bignum = 0;
static uint64_t ubignum = 0;
static float fpnum = 0.0;
static double dpnum = 0.0;


static GetOptsOption options[] = {
	{ "help",    '?', GETOPTS_NO_ARG|GETOPTS_ARG_CUSTOM, "display this help and quit", NULL, 'h', display_help,    NULL },
	{ "version", 'v', GETOPTS_NO_ARG|GETOPTS_ARG_CUSTOM, "display version and quit",   NULL, 'v', display_version, NULL },
	{ "really-really-long-option-arg", 'l', GETOPTS_NO_ARG, "a really really really really long option that can take an optional argument and stuff", NULL, 'l', NULL, &really_long_arg },
	{ "required-arg", 'r', GETOPTS_REQUIRED_STRING_ARG, "a required argument",  "<value>", 'r', NULL, &required_arg },
	{ "optional-arg", 'o', GETOPTS_OPTIONAL_STRING_ARG, "an optional argument", "<value>", 'o', NULL, &optional_arg },
	{ NULL, 'a', GETOPTS_REQUIRED_STRING_ARG, "required", "<string>", 'a', NULL, &a },
	{ NULL, 'b', GETOPTS_OPTIONAL_STRING_ARG, "optional", "<string>", 'b', NULL, &b },
	{ NULL, 'c', GETOPTS_REQUIRED_STRING_ARG, "required", "<string>", 'c', NULL, &c },
	{ NULL, 'O', GETOPTS_OPTIONAL_INT_ARG | GETOPTS_ARG_IN_SHORT, "set optimisation level", "<level>", 'O', NULL, &O },
	{ NULL, 'W', GETOPTS_REQUIRED_STRING_ARG | GETOPTS_ARG_IN_SHORT, "enable a particular type of warning, e.g. -Wall", "<warning>", 'W', NULL, &W },
	{ "bool",  '\0', GETOPTS_REQUIRED_BOOL_ARG, "bool arg", "<bool>", 't', NULL, &bool },
	{ "int64", '\0', GETOPTS_REQUIRED_INT64_ARG, "int64 arg", "<int>", 'i', NULL, &bignum },
	{ "uint64", '\0', GETOPTS_REQUIRED_UINT64_ARG, "uint64 arg", "<int>", 'u', NULL, &ubignum },
	{ "float", '\0', GETOPTS_REQUIRED_FLOAT_ARG, "float arg", "<float>", 'f', NULL, &fpnum },
	{ "double", '\0', GETOPTS_REQUIRED_DOUBLE_ARG, "double arg", "<double>", 'd', NULL, &dpnum },
	GETOPTS_TABLE_END
};

int main (int argc, char **argv)
{
	GetOptsContext *ctx;
	int i;
	
	ctx = getopts_context_new (argc, argv, options, GETOPTS_FLAG_ALLOW_SINGLE_DASH | GETOPTS_FLAG_BREAK_ON_FIRST_NONARG);
	getopts_parse_args (ctx);
	fprintf (stdout, "remaining args: ");
	for (i = getopts_get_next_index (ctx); i < argc; i++) {
		fputs (argv[i], stdout);
		fputc (' ', stdout);
	}
	fputc ('\n', stdout);
	
	if (really_long_arg)
		printf ("- we got the really really long arg\n");
	
	if (required_arg)
		printf ("- --required-arg was passed with a value of %s\n", required_arg);
	
	if (optional_arg)
		printf ("- --optional-arg's value was %s\n", optional_arg);
	
	printf ("- a = '%s'; b = '%s'; c = '%s'\n", a ? a : "(null)", b ? b : "(null)", c ? c : "(null)");
	
	printf ("- optimisation level is %d\n", O);
	printf ("- showing %s warnings\n", W ? W : "no");
	
	printf ("- bool arg was %d\n", bool);
	printf ("- int64 value was %lld\n", bignum);
	printf ("- uint64 value was %llu\n", ubignum);
	printf ("- float value was %f\n", fpnum);
	printf ("- double value was %f\n", dpnum);
	
	getopts_context_free (ctx);
	
	return 0;
}

#endif /* TEST_SUITE */
