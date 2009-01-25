/* 
 * deployment.cpp: Deployment Class support
 *
 * Copyright 2008 Novell, Inc.  (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

#include "deployment.h"

#include <mono/metadata/appdomain.h>


GHashTable* Deployment::current_hash = NULL;

Deployment*
Deployment::GetCurrent()
{
	if (!current_hash)
		current_hash = g_hash_table_new (g_direct_hash, g_direct_equal);

	MonoDomain *domain;
	if (!(domain = mono_domain_get ()))
		return NULL;

	return (Deployment*)g_hash_table_lookup (current_hash, domain);
}

void
Deployment::SetCurrent (Deployment* deployment)
{
	if (!current_hash)
		current_hash = g_hash_table_new (g_direct_hash, g_direct_equal);

	MonoDomain *domain;

	if (!(domain = mono_domain_get ()))
		return;
	
	if (deployment == NULL) {
		g_hash_table_remove (current_hash, domain);
		return;
	}

	return g_hash_table_insert (current_hash, domain, deployment);
}

Deployment::Deployment()
{
	types = new Types ();
	current_app = NULL;
}

Deployment::~Deployment()
{
	delete types;
}

Types*
Deployment::GetTypes ()
{
	return types;
}

Application*
Deployment::GetCurrentApplication ()
{
	return current_app;
}

void
Deployment::SetCurrentApplication (Application* value)
{
	if (current_app == value)
		return;

	if (current_app)
		current_app->unref ();

	current_app = value;

	if (current_app)
	  current_app->ref ();
}
