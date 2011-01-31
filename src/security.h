/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * security.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __SECURITY_H__
#define __SECURITY_H__

#include <string.h>
#include <glib.h>
#include <sys/stat.h>

#include "dependencyobject.h"

namespace Moonlight {

/* @Namespace=System.Windows */
class SecuritySettings : public DependencyObject {
protected:
	virtual ~SecuritySettings () {}

public:
	/* @GeneratePInvoke */
	SecuritySettings () : DependencyObject (Type::SECURITYSETTINGS) {}

	/* @PropertyType=ElevatedPermissions,DefaultValue=ElevatedPermissionsNone,GenerateAccessors,ManagedSetterAccess=Private,Validator=OnlyDuringInitializationValidator */
	const static int ElevatedPermissionsProperty;

	ElevatedPermissions GetElevatedPermissions ();
	void SetElevatedPermissions (ElevatedPermissions value);
};

G_BEGIN_DECLS

void security_enable_coreclr (const char *platform_dir);

G_END_DECLS

};
#endif

