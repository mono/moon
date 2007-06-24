/*
 * control.h:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_CONTROL_H__
#define __MOON_CONTROL_H__

#include "runtime.h"

//
// Control Class
//
class Control : public FrameworkElement {
 public:
	FrameworkElement *real_object;

	Control () : real_object (NULL) { };
	~Control ();
	
	virtual Type::Kind GetObjectType () { return Type::CONTROL; }
	virtual void update_xform ();
	virtual void render (Surface *surface, int x, int y, int width, int height);
	virtual void getbounds ();
	virtual void get_xform_for (UIElement *item, cairo_matrix_t *result);
	virtual Point getxformorigin ();
	virtual bool inside_object (Surface *s, double x, double y);
	virtual bool handle_motion (Surface *s, int state, double x, double y);
	virtual bool handle_button (Surface *s, callback_mouse_event cb, int state, double x, double y);
	virtual void enter (Surface *s, int state, double x, double y);
	virtual void leave (Surface *s);

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop);
	virtual bool OnChildPropertyChanged (DependencyProperty *prop, DependencyObject *child);

	virtual void OnLoaded ();

	UIElement* InitializeFromXaml (const char *xaml,
				       xaml_create_custom_element_callback *cecb,
				       xaml_set_custom_attribute_callback *sca,
				       xaml_hookup_event_callback *hue,
				       Type::Kind *element_type);
};

G_BEGIN_DECLS

Control *control_new (void);
UIElement* control_initialize_from_xaml (Control *control, const char *xaml,
					 xaml_create_custom_element_callback *cecb,
					 xaml_set_custom_attribute_callback *sca,
					 xaml_hookup_event_callback *hue,
					 Type::Kind *element_type);

G_END_DECLS

#endif /* __MOON_CONTROL_H__ */
