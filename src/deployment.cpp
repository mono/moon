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


DependencyProperty *AssemblyPart::SourceProperty;

AssemblyPart *
assembly_part_new (void)
{
	return new AssemblyPart ();
}

AssemblyPartCollection *
assembly_part_collection_new (void)
{
	return new AssemblyPartCollection ();
}

SupportedCultureCollection *
supported_culture_collection_new (void)
{
	return new SupportedCultureCollection ();
}

DependencyProperty *Deployment::AllowInboundCallsFromXDomainProperty;
DependencyProperty *Deployment::EntryPointAssemblyProperty;
DependencyProperty *Deployment::EntryPointTypeProperty;
DependencyProperty *Deployment::NeutralResourcesLanguageProperty;
DependencyProperty *Deployment::PartsProperty;
DependencyProperty *Deployment::SupportedCulturesProperty;
DependencyProperty *Deployment::RuntimeVersion;

Deployment *
deployment_new (void)
{
	return new Deployment ();
}



void
deployment_init (void)
{
	AssemblyPart::SourceProperty = DependencyObject::Register (Type::ASSEMBLYPART, "Source", Type::STRING);
	
	Deployment::AllowInboundCallsFromXDomainProperty = DependencyObject::Register (Type::DEPLOYMENT, "AllowInboundCallsFromXDomain", new Value (false));
	Deployment::EntryPointAssemblyProperty = DependencyObject::Register (Type::DEPLOYMENT, "EntryPointAssembly", Type::STRING);
	Deployment::EntryPointTypeProperty = DependencyObject::Register (Type::DEPLOYMENT, "EntryPointType", Type::STRING);
	Deployment::NeutralResourcesLanguageProperty = DependencyObject::Register (Type::DEPLOYMENT, "NeutralResourcesLanguage", Type::STRING);
	Deployment::PartsProperty = DependencyObject::Register (Type::DEPLOYMENT, "Parts", Type::ASSEMBLYPART_COLLECTION);
	Deployment::SupportedCulturesProperty = DependencyObject::Register (Type::DEPLOYMENT, "SupportedCultures", Type::SUPPORTEDCULTURE_COLLECTION);
	Deployment::RuntimeVersion = DependencyObject::Register (Type::DEPLOYMENT, "RuntimeVersion", Type::STRING);
}
