/*
 * xap.h: XAP loader
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_XAP_H__
#define __MOON_XAP_H__

#include <glib.h>

/* @Version=2 */
class Xap {
	char *xap_dir;
	XamlLoader *loader;
	DependencyObject *root;

 public:
	Xap (XamlLoader *loader, char *xap_dir, DependencyObject *root);
	virtual ~Xap ();

	DependencyObject *getRoot () { return root; } 

	/* @GenerateCBinding,GeneratePInvoke */
	static char *Unpack (const char *fname);
};

Xap *xap_create_from_file (XamlLoader *loader, const char *filename);

#endif /* __MOON_XAP_H__ */
