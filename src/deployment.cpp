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

DependencyProperty *Deployment::ExternalCallersFromCrossDomainProperty;
DependencyProperty *Deployment::EntryPointAssemblyProperty;
DependencyProperty *Deployment::EntryPointTypeProperty;
DependencyProperty *Deployment::PartsProperty;
DependencyProperty *Deployment::RuntimeVersionProperty;

Deployment *
deployment_new (void)
{
	return new Deployment ();
}

Application *
application_new (void)
{
	return new Application ();
}

void
deployment_init (void)
{
	AssemblyPart::SourceProperty = DependencyProperty::Register (Type::ASSEMBLYPART, "Source", Type::STRING);
	
	Deployment::ExternalCallersFromCrossDomainProperty = DependencyProperty::Register (Type::DEPLOYMENT, "ExternalCallersFromCrossDomain", new Value (CrossDomainAccessNoAccess));
	Deployment::EntryPointAssemblyProperty = DependencyProperty::Register (Type::DEPLOYMENT, "EntryPointAssembly", Type::STRING);
	Deployment::EntryPointTypeProperty = DependencyProperty::Register (Type::DEPLOYMENT, "EntryPointType", Type::STRING);
	Deployment::PartsProperty = DependencyProperty::Register (Type::DEPLOYMENT, "Parts", Type::ASSEMBLYPART_COLLECTION);
	Deployment::RuntimeVersionProperty = DependencyProperty::Register (Type::DEPLOYMENT, "RuntimeVersion", Type::STRING);
}
