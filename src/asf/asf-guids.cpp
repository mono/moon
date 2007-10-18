/*
 * asf-guids.cpp: 
 *
 * Author: Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include "asf.h"


bool
asf_guid_compare (const asf_guid* a, const asf_guid* b)
{
	if (a == b)
		return true;
		
	if (a == NULL || b == NULL)
		return false;
		
	return memcmp (a, b, sizeof (asf_guid)) == 0;
}

ASFTypes
asf_get_guid_type (const asf_guid* guid)
{
	int i = 0;
	while (asf_types [i].type != ASF_LAST_TYPE) {
		if (asf_guid_compare (&asf_types [i].guid, guid)) { 
			return asf_types [i].type;
		}
		
		i++;
	}
	
	return ASF_NONE;
}

const char*
asf_type_get_name (ASFTypes type)
{
	int i = 0;
	while (asf_types [i].type != ASF_LAST_TYPE) {
		if (asf_types [i].type == type) {
			return asf_types [i].name;
		}
		
		i++;
	}
	
	return "<unknown type>";
}

const char*
asf_guid_get_name (const asf_guid* guid)
{
	return asf_type_get_name (asf_get_guid_type (guid));
}

char* 
asf_guid_tostring (const asf_guid* g)
{
	return g_strdup_printf ("GUID: %s = (%X, %X, %X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X)", 
		asf_guid_get_name (g), g->a, g->b, g->c, g->d[0], g->d[1], g->d[2], g->d[3], g->d[4], g->d[5], g->d[6], g->d[7]);
}

bool
asf_guid_validate (const asf_guid* guid_actual, const asf_guid* guid_expected, ASFParser* parser)
{
	if (!asf_guid_compare (guid_actual, guid_expected)) {
		char* expected = asf_guid_tostring (guid_expected);
		char* actual = asf_guid_tostring (guid_actual);
		parser->AddError (g_strdup_printf ("Invalid id (expected: %s, got: %s).\n", expected, actual));
		g_free (actual);
		g_free (expected);
		return false;
	}
	
	return true;
}
