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
#include <cairo.h>
#include <malloc.h>
#include <stdlib.h>
#include "value.h"
#include "rect.h"
#include "color.h"
#include "clock.h"
#include "animation.h"
#include "array.h"
#include "point.h"

/**
 * Value implementation
 */

Value*
Value::CreateUnrefPtr (DependencyObject* dob)
{
	Value* result = new Value (dob);
	dob->unref ();
	return result;
}

Value
Value::CreateUnref (DependencyObject* dob)
{
	Value result = Value (dob);
	dob->unref ();
	return result;
}

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
{
	k = Type::INVALID;
	Init ();
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
		if (Is (Type::EVENTOBJECT) && u.dependency_object)
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

Value::Value (gint64 i, Type::Kind as)
{
	Init ();
	k = as;
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

Value::Value (EventObject *obj)
{
	Init ();
	if (obj == NULL) {
		k = Type::EVENTOBJECT;
	}
	else {
		if (!Type::IsSubclassOf (obj->GetObjectType (), Type::EVENTOBJECT)) {
			g_warning ("creating invalid dependency object Value");
			k = Type::INVALID;
			u.dependency_object = NULL;
			return;
		}
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
		if (Is (Type::EVENTOBJECT) && u.dependency_object)
			u.dependency_object->unref ();
	}
}

char *
Value::ToString ()
{
	if (this == NULL)
		return g_strdup ("NULL");

	GString *str = g_string_new ("");
	char *t = NULL;
	
	switch (k){

	case Type::DOUBLE:
		g_string_append_printf (str, "{ %f }", u.d);
		break;

	case Type::STRING:
		g_string_append (str, u.s);
		break;
		
	case Type::POINT_ARRAY: {
		char *t = g_strdup_printf ("Points [] = { /* refcount = %d */ ", u.point_array->basic.refcount);
		g_string_append (str, t);
		g_free (t);
		
		for (uint i = 0; i < u.point_array->basic.refcount; i++)
			g_string_append_printf (str, "(%g, %g), ", u.point_array->points [i].x, u.point_array->points [i].y);

		g_string_append (str, "}");
		break;
	}
	case Type::DOUBLE_ARRAY: {
		t = g_strdup_printf ("double [] = { /* refcount = %d */ ", u.double_array->basic.refcount);
		g_string_append (str, t);
		g_free (t);
		
		for (uint i = 0; i < u.double_array->basic.refcount; i++)
			g_string_append_printf (str, "%g,  ", u.double_array->values [i]);

		g_string_append (str, "}");
		break;
	}
	case Type::COLOR:
		g_string_append_printf (str, "{%g/%g/%g/%g}", u.color->r, u.color->g, u.color->b, u.color->a);
		break;
	case Type::POINT:
		g_string_append_printf (str, "{ %g, %g }", (u.point)->x, (u.point)->y);
		break;
	case Type::RECT:
		g_string_append_printf (str, "{ x=%g, y=%g, w=%g, h=%g }", (u.rect)->x, (u.rect)->y, (u.rect)->w, (u.rect)->h);
		break;
	case Type::REPEATBEHAVIOR:
		if (u.repeat->IsForever ())
			g_string_append (str, "{repeat=forever}");
		else if ((u.repeat)->HasDuration ())
			g_string_append_printf (str, "{repeat=duration}");
		else if ((u.repeat)->HasCount ())
			g_string_append_printf (str, "{repeat=count %g}", (u.repeat)->GetCount ());
		break;
	case Type::DURATION:
		g_string_append_printf (str, "{duration/TODO}");
		break;
	case Type::KEYTIME:
		g_string_append_printf (str, "{keytime/TODO}");
		break;
	default:
		if (Is (Type::EVENTOBJECT) && u.dependency_object)
			g_string_append_printf (str, "[%s <%s>]", u.dependency_object->GetTypeName (), Is (Type::DEPENDENCY_OBJECT) ? AsDependencyObject ()->GetName () : "no name");
		else
			g_string_append_printf (str, "UnknownType");
		break;
	}
	return g_string_free (str, FALSE);
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

