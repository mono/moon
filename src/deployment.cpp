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

void
deployment_init (void)
{
	// Don't register DPs here.
	return;

	AssemblyPart::SourceProperty = DependencyProperty::Register (Type::ASSEMBLYPART, "Source", Type::STRING);
	
	Deployment::ExternalCallersFromCrossDomainProperty = DependencyProperty::Register (Type::DEPLOYMENT, "ExternalCallersFromCrossDomain", new Value (CrossDomainAccessNoAccess));
	Deployment::EntryPointAssemblyProperty = DependencyProperty::Register (Type::DEPLOYMENT, "EntryPointAssembly", Type::STRING);
	Deployment::EntryPointTypeProperty = DependencyProperty::Register (Type::DEPLOYMENT, "EntryPointType", Type::STRING);
	Deployment::PartsProperty = DependencyProperty::Register (Type::DEPLOYMENT, "Parts", Type::ASSEMBLYPART_COLLECTION);
	Deployment::RuntimeVersionProperty = DependencyProperty::Register (Type::DEPLOYMENT, "RuntimeVersion", Type::STRING);
}
