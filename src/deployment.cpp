/* 
 * deployment.cpp: Deployment Class support
 *
 * Copyright 2008 Novell, Inc.  (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */
#include <config.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <stdint.h>
#include "deployment.h"

void 
assembly_part_init (void)
{
	AssemblyPart::Source = DependencyObject::Register (Type::ASSEMBLYPART, "Source", Type::STRING);
}

AssemblyPartCollection *
assembly_part_collection_new (void)
{
	return new AssemblyPartCollection ();
}

SupportedCulturesCollection *
supported_cultures_collection_new (void)
{
	return new SupportedCulturesCollection ();
}

void 
deployment_init (void)
{
	Deployment::AllowInboundCallsFromXDomain = DependencyObject::Register (Type::DEPLOYMENT, "AllowInboundCallsFromXDomain", new Value (false));
	Deployment::EntryPointAssembly = DependencyObject::Register (Type::DEPLOYMENT, "EntryPointAssembly", Type::STRING);
	Deployment::EntryPointType = DependencyObject::Register (Type::DEPLOYMENT, "EntryPointType", Type::STRING);
	Deployment::NeutralResourcesLanguage = DependencyObject::Register (Type::DEPLOYMENT, "NeutralResourcesLanguage", Type::STRING);
	Deployment::Parts = DependencyObject::Register (Type::DEPLOYMENT, "Parts", Type::ASSEMBLYPART_COLLECTION);
	Deployment::SupportedCultures = DependencyObject::Register (Type::DEPLOYMENT, "SupportedCultures", Type::SUPPORTEDCULTURES_COLLECTION);
}

Deployment *
deployment_new (void)
{
	return new Deployment ();
}
