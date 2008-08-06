/*
 * size.h
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_SIZE_H__
#define __MOON_SIZE_H__

#include <glib.h>
#include "eventargs.h"

/* @SilverlightVersion="2" */
struct Size {
public:
       double width, height;

       Size () : width(0), height(0) {}

       Size (double w, double h)
       {
               this->width = w;
               this->height = h;
       }

};

/* @SilverlightVersion="2" */
class SizeChangedEventArgs : public RoutedEventArgs {
public:
	SizeChangedEventArgs (Size prev_size, Size new_size);

	virtual Type::Kind GetObjectType () { return Type::SIZECHANGEDEVENTARGS; };

	Size GetPrevSize () { return prev_size; }
	Size GetNewSize () { return new_size; }

private:
	Size prev_size;
	Size new_size;
};

G_BEGIN_DECLS

SizeChangedEventArgs* size_changed_event_args_new (Size prev_size, Size new_size);
void                  size_changed_event_args_get_prev_size (SizeChangedEventArgs *args, Size *prev_size);
void                  size_changed_event_args_get_new_size  (SizeChangedEventArgs *args, Size *new_size);

G_END_DECLS

#endif /* __MOON_POINT_H__ */
