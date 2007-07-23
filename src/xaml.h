/*
 * xaml.h: xaml parser
 *
 * Author:
 *   Jackson Harper (jackson@ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_XAML_H__
#define __MOON_XAML_H__

#include "enums.h"
#include "uielement.h"


typedef DependencyObject *xaml_create_custom_element_callback (const char *xmlns, const char *name);
typedef void xaml_set_custom_attribute_callback (void *target, const char *name, const char *value);
typedef void xaml_hookup_event_callback (void *target, const char *ename, const char *evalue);

G_BEGIN_DECLS

UIElement  *xaml_create_from_file (const char *xaml, bool create_namescope, Type::Kind *element_type);
UIElement  *xaml_create_from_str  (const char *xaml, bool create_namescope, Type::Kind *element_type);
void        xaml_set_property_from_str (DependencyObject *obj, DependencyProperty *prop, const char *value);

void        xaml_set_parser_callbacks (xaml_create_custom_element_callback *cecb,
				       xaml_set_custom_attribute_callback *sca, xaml_hookup_event_callback *hue);

void        xaml_init (void);

G_END_DECLS

#endif /* __MOON_XAML_H__ */
