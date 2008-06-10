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

#endif /* __MOON_POINT_H__ */
