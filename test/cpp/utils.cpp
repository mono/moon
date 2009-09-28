/*
 * Native unit tests
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "config.h"
#include "main.h"

#include "utils.h"

TEST(Utils, parse_rfc_1945_token_test)
{
	char *input, *free;

	char *result;
	char c;
	char *end;

	input = g_strdup_printf ("asd");
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input);
	EXPECT_EQ (c, '\0');
	EXPECT_EQ (end, (void*) NULL);
	g_free (input);

	result = parse_rfc_1945_token (NULL, NULL, NULL);
	EXPECT_EQ (result, (void*) NULL);

	free = input = g_strdup_printf ("LI(H\"ñoIH\"");
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input);
	EXPECT_STREQ (result, "LI");
	EXPECT_EQ (c, '(');
	EXPECT_EQ (end, result + 3);
	input = end;
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input);
	EXPECT_STREQ (result, "H");
	EXPECT_EQ (c, '\"');
	EXPECT_EQ (end, result + 2);
	input = end;
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input);
	EXPECT_STREQ (result, "ñoIH");
	EXPECT_EQ (c, '\"');
	EXPECT_EQ (end, result + 6);
	g_free (free);

	free = input = g_strdup_printf ("packet-pair-experiment=1, no-cache, client-id=1089364828, xResetStrm=1, features=\"seekable,stridable,playlist,skipforward\", timeout=60000");
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input);
	EXPECT_STREQ (result, "packet-pair-experiment");
	EXPECT_EQ (c, '=');
	EXPECT_EQ (end, result + 23);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input);
	EXPECT_STREQ (result, "1");
	EXPECT_EQ (c, ',');
	EXPECT_EQ (end, result + 2);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input + 1);
	EXPECT_STREQ (result, "no-cache");
	EXPECT_EQ (c, ',');
	EXPECT_EQ (end, result + 9);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input + 1);
	EXPECT_STREQ (result, "client-id");
	EXPECT_EQ (c, '=');
	EXPECT_EQ (end, result + 10);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input );
	EXPECT_STREQ (result, "1089364828");
	EXPECT_EQ (c, ',');
	EXPECT_EQ (end, result + 11);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input + 1);
	EXPECT_STREQ (result, "xResetStrm");
	EXPECT_EQ (c, '=');
	EXPECT_EQ (end, result + 11);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input);
	EXPECT_STREQ (result, "1");
	EXPECT_EQ (c, ',');
	EXPECT_EQ (end, result + 2);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input + 1);
	EXPECT_STREQ (result, "features");
	EXPECT_EQ (c, '=');
	EXPECT_EQ (end, result + 9);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input + 1);
	EXPECT_STREQ (result, "seekable");
	EXPECT_EQ (c, ',');
	EXPECT_EQ (end, result + 9);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input);
	EXPECT_STREQ (result, "stridable");
	EXPECT_EQ (c, ',');
	EXPECT_EQ (end, result + 10);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input);
	EXPECT_STREQ (result, "playlist");
	EXPECT_EQ (c, ',');
	EXPECT_EQ (end, result + 9);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input);
	EXPECT_STREQ (result, "skipforward");
	EXPECT_EQ (c, '\"');
	EXPECT_EQ (end, result + 12);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input + 2);
	EXPECT_STREQ (result, "timeout");
	EXPECT_EQ (c, '=');
	EXPECT_EQ (end, result + 8);
	input = end; // ----
	result = parse_rfc_1945_token (input, &c, &end);
	EXPECT_EQ (result, input);
	EXPECT_STREQ (result, "60000");
	EXPECT_EQ (c, '\0');
	EXPECT_EQ (end, (void*) NULL);
	g_free (free);


}

TEST(Utils, parse_rfc_1945_quoted_string_test)
{
	char *input, *free;

	char *result;
	char c;
	char *end;

	input = g_strdup_printf ("\"asd");
	result = parse_rfc_1945_quoted_string (input, &c, &end);
	EXPECT_EQ (result, input + 1);
	EXPECT_STREQ (result, "asd");
	EXPECT_EQ (c, '\0');
	EXPECT_EQ (end, (void*) NULL);
	g_free (input);

	result = parse_rfc_1945_quoted_string (NULL, NULL, NULL);
	EXPECT_EQ (result, (void*) NULL);

	free = input = g_strdup_printf ("LI(H\"ñoIH\"");
	result = parse_rfc_1945_quoted_string (input, &c, &end);
	EXPECT_EQ (result, (void*) NULL);
	g_free (free);

	free = input = g_strdup_printf ("\"seekable,stridable,playlist,skipforward\"");
	result = parse_rfc_1945_quoted_string (input, &c, &end);
	EXPECT_EQ (result, input + 1);
	EXPECT_STREQ (result, "seekable,stridable,playlist,skipforward");
	EXPECT_EQ (c, '\"');
	EXPECT_EQ (end, result + 40);
	g_free (free);

}