/*
 * value.cpp: Implementation of for Value.
 *
 * Author:
 *   Rolf Bjarne Kvinge (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <malloc.h>
#include <stdlib.h>
#include "value.h"
#include "rect.h"
#include "color.h"
#include "clock.h"
#include "animation.h"
#include "array.h"

/**
 * Value implementation
 */

Type::Kind
Value::GetKind ()
{
	return k;
}

void
Value::Init ()
{
	memset (&u, 0, sizeof (u));
}

Value::Value()
  : k (Type::INVALID)
{
	Init ();
}

/* this is needed for the TimeSpan handling in the parser (since
   timespans are stored as gint64's, we need this to "cast" it as far
   as the value goes) */
Value::Value (const Value& v, Type::Kind as)
{
	k = as;
	u = v.u;
}

Value::Value (const Value& v)
{
	k = v.k;
	u = v.u;

	/* make a copy of the string instead of just the pointer */
	switch (k) {
	case Type::STRING:
		u.s = g_strdup (v.u.s);
		break;
	case Type::POINT_ARRAY:
		u.point_array->basic.refcount++;
		break;
	case Type::DOUBLE_ARRAY:
		u.double_array->basic.refcount++;
		break;
	case Type::MATRIX:
		u.matrix = (Matrix*) g_malloc (sizeof (Matrix));
		memcpy (u.matrix, v.u.matrix, sizeof(Matrix));
		break;
	case Type::COLOR:
		u.color = g_new (Color, 1);
		*u.color = Color (*v.u.color);
		break;
	case Type::POINT:
		u.point = g_new (Point, 1);
		*u.point = Point (*v.u.point);
		break;
	case Type::RECT:
		u.rect = g_new (Rect, 1);
		*u.rect = Rect (*v.u.rect);
		break;
	case Type::REPEATBEHAVIOR:
		u.repeat = g_new (RepeatBehavior, 1);
		*u.repeat = RepeatBehavior (*v.u.repeat);
		break;
	case Type::DURATION:
		u.duration = g_new (Duration, 1);
		*u.duration = Duration (*v.u.duration);
		break;
	case Type::KEYTIME:
		u.keytime = g_new (KeyTime, 1);
		*u.keytime = KeyTime (*v.u.keytime);
		break;
	default:
		if (k >= Type::DEPENDENCY_OBJECT)
			u.dependency_object->ref ();
		break;
	}
}

Value::Value (Type::Kind k)
{
	Init();
	this->k = k;
}

Value::Value(bool z)
{
	Init ();
	k = Type::BOOL;
	u.i32 = z;
}

Value::Value (double d)
{
	Init ();
	k = Type::DOUBLE;
	u.d = d;
}

Value::Value (guint64 i)
{
	Init ();
	k = Type::UINT64;
	u.ui64 = i;
}

Value::Value (gint64 i)
{
	Init ();
	k = Type::INT64;
	u.i64 = i;
}

Value::Value (gint32 i)
{
	Init ();
	k = Type::INT32;
	u.i32 = i;
}

Value::Value (Color c)
{
	Init ();
	k = Type::COLOR;
	u.color = g_new (Color, 1);
	*u.color = Color (c);
}

Value::Value (DependencyObject *obj)
{
	Init ();
	if (obj == NULL) {
		k = Type::DEPENDENCY_OBJECT;
	} else {
		g_assert (obj->GetObjectType () >= Type::DEPENDENCY_OBJECT);
		k = obj->GetObjectType ();
		obj->ref ();
	}
	u.dependency_object = obj;
}

Value::Value (Point pt)
{
	Init ();
	k = Type::POINT;
	u.point = g_new (Point, 1);
	*u.point = Point (pt);
}

Value::Value (Rect rect)
{
	Init ();
	k = Type::RECT;
	u.rect = g_new (Rect, 1);
	*u.rect = Rect (rect);
}

Value::Value (RepeatBehavior repeat)
{
	Init();
	k = Type::REPEATBEHAVIOR;
	u.repeat = g_new (RepeatBehavior, 1);
	*u.repeat = RepeatBehavior (repeat);
}

Value::Value (Duration duration)
{
	Init();
	k = Type::DURATION;
	u.duration = g_new (Duration, 1);
	*u.duration = Duration (duration);
}

Value::Value (KeyTime keytime)
{
	Init ();
	k = Type::KEYTIME;
	u.keytime = g_new (KeyTime, 1);
	*u.keytime = KeyTime (keytime);
}

Value::Value (const char* s)
{
	Init ();
	k = Type::STRING;
	u.s= g_strdup (s);
}

Value::Value (Point *points, int count)
{
	Init ();
	k = Type::POINT_ARRAY;
	u.point_array = point_array_new (count, points);
}

Value::Value (double *values, int count)
{
	Init ();
	k = Type::DOUBLE_ARRAY;
	u.double_array = double_array_new (count, values);
}

Value::Value (Matrix *matrix)
{
	Init ();
	k = Type::MATRIX;
	u.matrix = (Matrix*) g_malloc (sizeof (Matrix));
	memcpy (u.matrix, matrix, sizeof (Matrix));
}

void
Value::FreeValue ()
{
	switch (GetKind ()) {
	case Type::STRING:
		g_free (u.s);
		break;
	case Type::POINT_ARRAY:
		if (u.point_array != NULL &&--u.point_array->basic.refcount == 0)
			g_free (u.point_array);
		break;
	case Type::DOUBLE_ARRAY:
		if (u.double_array != NULL && --u.double_array->basic.refcount == 0)
			g_free (u.double_array);
		break;
	case Type::MATRIX:
		g_free (u.matrix);
		break;
	case Type::COLOR:
		g_free (u.color);
		break;
	case Type::POINT:
		g_free (u.point);
		break;
	case Type::RECT:
		g_free (u.rect);
		break;
	case Type::REPEATBEHAVIOR:
		g_free (u.repeat);
		break;
	case Type::DURATION:
		g_free (u.duration);
		break;
	case Type::KEYTIME:
		g_free (u.keytime);
		break;
	default:
		if (GetKind () >= Type::DEPENDENCY_OBJECT && u.dependency_object)
			u.dependency_object->unref ();
	}
}

//
// This is invoked by managed code to free the contents of the value
//
void 
value_free_value (Value *value)
{
	value->FreeValue ();
}

Value::~Value ()
{
	FreeValue ();
}

