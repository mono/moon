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
#include "mms-downloader.h"

TEST(MmsTest, TestContentDescriptionList)
{
	ContentDescriptionList *cdl;
	ContentDescription *cd;
	const char *input;

	cdl = new ContentDescriptionList ();
	EXPECT_FALSE (cdl->Parse ("9,copyright,31,30,)) 1999 Microsoft Corporation", 47));
	delete cdl;

	cdl = new ContentDescriptionList ();
	EXPECT_TRUE (cdl->Parse ("9,copyright,31,30,)) 1999 Microsoft Corporation ", 48));
	EXPECT_TRUE (cdl->list.Length () == 1);
	if (cdl->list.Length () == 1) {
		cd = (ContentDescription *) cdl->list.First ();
		EXPECT_STREQ ("copyright", cd->name);
		EXPECT_EQ (ContentDescription::VT_LPWSTR, cd->value_type);
		EXPECT_EQ (30, cd->value_length);
		EXPECT_TRUE (memcmp (cd->value, ")) 1999 Microsoft Corporation ", cd->value_length) == 0);
	}
	delete cdl;

	input =
		"8,language,31,0,,5,title,31,13,Advertisement,6,author,31,21,Microsoft Corporation,9,copyright,31,57,Copyright (C) Microsoft Corporation. All "
		"rights reserved.,44,WMS_CONTENT_DESCRIPTION_SERVER_BRANDING_INFO,31,12,WMServer/9.1,51,WMS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_START_OFFSET,3"
		",4,3000,47,WMS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_DURATION,3,4,8000,58,WMS_CONTENT_DESCRIPTION_COPIED_METADATA_FROM_PLAYLIST_FILE,3,1,1,42,W"
		"MS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_URL,31,1,/\x013\x010$H�\x011\x000\x000\x000";

	cdl = new ContentDescriptionList ();
	EXPECT_TRUE (cdl->Parse (input, strlen (input)));
	EXPECT_EQ (9, cdl->list.Length ());
	if (cdl->list.Length () == 9) {
		cd = (ContentDescription *) cdl->list.First ();
		EXPECT_STREQ ("language", cd->name);
		EXPECT_EQ (ContentDescription::VT_LPWSTR, cd->value_type);
		EXPECT_EQ (0, cd->value_length);

		cd = (ContentDescription *) cd->next;
		EXPECT_STREQ ("title", cd->name);
		EXPECT_EQ (ContentDescription::VT_LPWSTR, cd->value_type);
		EXPECT_EQ (13, cd->value_length);
		EXPECT_TRUE (memcmp (cd->value, "Advertisement", cd->value_length) == 0);

		cd = (ContentDescription *) cd->next;
		EXPECT_STREQ ("author", cd->name);
		EXPECT_EQ (ContentDescription::VT_LPWSTR, cd->value_type);
		EXPECT_EQ (21, cd->value_length);
		EXPECT_TRUE (memcmp (cd->value, "Microsoft Corporation", cd->value_length) == 0);

		cd = (ContentDescription *) cd->next;
		EXPECT_STREQ ("copyright", cd->name);
		EXPECT_EQ (ContentDescription::VT_LPWSTR, cd->value_type);
		EXPECT_EQ (57, cd->value_length);
		EXPECT_TRUE (memcmp (cd->value, "Copyright (C) Microsoft Corporation. All rights reserved.", cd->value_length) == 0);

		cd = (ContentDescription *) cd->next;
		EXPECT_STREQ ("WMS_CONTENT_DESCRIPTION_SERVER_BRANDING_INFO", cd->name);
		EXPECT_EQ (ContentDescription::VT_LPWSTR, cd->value_type);
		EXPECT_EQ (12, cd->value_length);
		EXPECT_TRUE (memcmp (cd->value, "WMServer/9.1.", cd->value_length) == 0);

		cd = (ContentDescription *) cd->next;
		EXPECT_STREQ ("WMS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_START_OFFSET", cd->name);
		EXPECT_EQ (ContentDescription::VT_I4, cd->value_type);
		EXPECT_EQ (4, cd->value_length);
		EXPECT_TRUE (memcmp (cd->value, "3000", cd->value_length) == 0);

		cd = (ContentDescription *) cd->next;
		EXPECT_STREQ ("WMS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_DURATION", cd->name);
		EXPECT_EQ (ContentDescription::VT_I4, cd->value_type);
		EXPECT_EQ (4, cd->value_length);
		EXPECT_TRUE (memcmp (cd->value, "8000", cd->value_length) == 0);

		cd = (ContentDescription *) cd->next;
		EXPECT_STREQ ("WMS_CONTENT_DESCRIPTION_COPIED_METADATA_FROM_PLAYLIST_FILE", cd->name);
		EXPECT_EQ (ContentDescription::VT_I4, cd->value_type);
		EXPECT_EQ (1, cd->value_length);
		EXPECT_TRUE (memcmp (cd->value, "1", cd->value_length) == 0);

		cd = (ContentDescription *) cd->next;
		EXPECT_STREQ ("WMS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_URL", cd->name);
		EXPECT_EQ (ContentDescription::VT_LPWSTR, cd->value_type);
		EXPECT_EQ (1, cd->value_length);
		EXPECT_TRUE (memcmp (cd->value, "/", cd->value_length) == 0);
	}
	delete cdl;
}
