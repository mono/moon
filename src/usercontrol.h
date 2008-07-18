/*
 * usercontrol.h:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_USERCONTROL_H__
#define __MOON_USERCONTROL_H__

#include <glib.h>

G_BEGIN_DECLS

#include "control.h"

//
// UserControl
//
/* @SilverlightVersion="2" */
/* @ContentProperty="ContentProperty" */
class UserControl : public Control {
protected:
	virtual ~UserControl ();

public:
	UserControl ();

	virtual Type::Kind GetObjectType () { return Type::USERCONTROL; }

	static DependencyProperty *ContentProperty;

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
};

UserControl*   user_control_new (void);
UIElement*     user_control_get_content (UserControl *user_control);

void user_control_init (void);

G_END_DECLS

#endif /* __MOON_USERCONTROL_H__ */
