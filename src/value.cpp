/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * value.cpp: Implementation of for Value.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <cairo.h>

#include <string.h>
#include <stdlib.h>

#include "value.h"
#include "rect.h"
#include "size.h"
#include "uri.h"
#include "color.h"
#include "clock.h"
#include "animation.h"
#include "control.h"
#include "point.h"
#include "grid.h"
#include "cornerradius.h"
#include "capture.h"
#include "fontmanager.h"
#include "fontsource.h"
#include "transform.h"
#include "utils.h"
#include "debug.h"
#include "deployment.h"
#include "managedtypeinfo.h"

namespace Moonlight {

/**
 * Value implementation
 */

static const int NullFlag = 1;
static const int GCHandleFlag = 1 << 1;
static const int NoUnrefFlag = 1 << 2;

static const int EqualityMask = NullFlag | GCHandleFlag;

Value*
Value::CreateUnrefPtr (EventObject* dob)
{
	Value *result = new Value (dob);
	LOG_VALUE ("unref Value [%p] %s\n", result, result->GetName());
	if (dob)
		dob->unref ();
	return result;
}

Value
Value::CreateUnref (EventObject* dob)
{
	Value result (dob);
	LOG_VALUE ("unref Value [%p] %s\n", &result, result.GetName());
	dob->unref ();
	return result;
}

Value *
Value::CreateUnrefPtr (Uri *uri)
{
	Value *result = new Value ();
	/* We could clone 'uri' and delete it, but instead we just stuff it into our fields to save a malloc/free.
	 * This method is somewhat badly named for 'Uri', but it's the name the generator generates */
	result->k = Type::URI;
	result->u.uri = uri;
	result->SetIsNull (false);
	LOG_VALUE ("unref [delete] Value [%p] %s\n", result, result->GetName());
	return result;
}

Value*
Value::Clone (Value *v, Types *types)
{
	if (!v)
		return NULL;

	if (!types)
		types = Deployment::GetCurrent()->GetTypes();

	if (!v->GetIsNull () && types->Find (v->k)->IsDependencyObject ()) {
		return Value::CreateUnrefPtr (v->AsDependencyObject()->Clone (types));
	}
	else {
		return new Value (*v);
	}
}

Type::Kind
Value::GetKind () const
{
	return k;
}

bool
Value::GetNeedUnref () const
{
	return (padding & NoUnrefFlag) == 0;
}

void
Value::SetNeedUnref (bool needUnref)
{
	if (needUnref)
		padding &= ~NoUnrefFlag;
	else
		padding |= NoUnrefFlag;
}

bool
Value::GetIsManaged () const
{
	return (padding & GCHandleFlag) == GCHandleFlag;
}

bool
Value::GetIsNull () const
{
	return (padding & NullFlag) == NullFlag;
}

void
Value::SetIsNull (bool isNull)
{
	if (isNull)
		padding |= NullFlag;
	else
		padding &= ~NullFlag;
}

void
Value::Init ()
{
	padding = 0;
	memset (&u, 0, sizeof (u));
	SetIsNull (true);
}

Value::Value()
{
	k = Type::INVALID;
	Init ();
}

Value::Value (const Value& v)
{
	Copy (v);
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
	SetIsNull (false);
}

Value::Value (double d)
{
	Init ();
	k = Type::DOUBLE;
	u.d = d;
	SetIsNull (false);
}

Value::Value (gint64 i, Type::Kind as)
{
	Init ();
	k = as;
	u.i64 = i;
	SetIsNull (false);
}

Value::Value (gint32 i)
{
	Init ();
	k = Type::INT32;
	u.i32 = i;
	SetIsNull (false);
}

Value::Value (gint32 i, Type::Kind as)
{
	Init ();
	k = as;
	u.i32 = i;
	SetIsNull (false);
}

Value::Value (guint32 i)
{
	Init ();
	k = Type::UINT32;
	u.ui32 = i;
	SetIsNull (false);
}

Value::Value (gunichar c, Type::Kind as)
{
	Init ();
	k = as;
	u.c = c;
	SetIsNull (false);
}

Value::Value (Color c)
{
	Init ();
	k = Type::COLOR;
	u.color = g_new (Color, 1);
	*u.color = Color (c);
	SetIsNull (false);
}

Value::Value (EventObject* obj)
{
	Init ();
	if (obj == NULL) {
		k = Type::EVENTOBJECT;
	}
	else {
		if (!obj->GetType ()->IsEventObject ()) {
			g_warning ("creating invalid dependency object Value");
			k = Type::INVALID;
			u.dependency_object = NULL;
			return;
		}
		k = obj->GetObjectType ();
		LOG_VALUE ("  ref Value [%p] %s\n", this, GetName());
		obj->ref ();
		SetIsNull (false);

		obj->AddHandler (EventObject::DestroyedEvent, EventObject::ClearWeakRef, &u.dependency_object);
	}
	u.dependency_object = obj;
}

Value::Value (FontFamily family)
{
	Init ();
	k = Type::FONTFAMILY;
	u.fontfamily = g_new (FontFamily, 1);
	u.fontfamily->source = g_strdup (family.source);
	SetIsNull (false);
}

Value::Value (const FontFamily *family)
{
	Init ();
	k = Type::FONTFAMILY;
	if (family) {
		u.fontfamily = g_new (FontFamily, 1);
		u.fontfamily->source = g_strdup (family->source);
	}
	SetIsNull (family == NULL);
}

Value::Value (FontWeight weight)
{
	Init ();
	k = Type::FONTWEIGHT;
	u.fontweight = g_new (FontWeight, 1);
	u.fontweight->weight = weight.weight;
	SetIsNull (false);
}

Value::Value (FontStretch stretch)
{
	Init ();
	k = Type::FONTSTRETCH;
	u.fontstretch = g_new (FontStretch, 1);
	u.fontstretch->stretch = stretch.stretch;
	SetIsNull (false);
}

Value::Value (FontStyle style)
{
	Init ();
	k = Type::FONTSTYLE;
	u.fontstyle = g_new (FontStyle, 1);
	u.fontstyle->style = style.style;
	SetIsNull (false);
}

Value::Value (FontSource source)
{
	Init ();
	k = Type::FONTSOURCE;
	u.fontsource = g_new (FontSource, 1);
	u.fontsource->type = source.type;
	
	switch (source.type) {
	case FontSourceTypeManagedStream:
		u.fontsource->source.stream = g_new (ManagedStreamCallbacks, 1);
		memcpy (u.fontsource->source.stream, source.source.stream, sizeof (ManagedStreamCallbacks));
		break;
	case FontSourceTypeGlyphTypeface:
		u.fontsource->source.typeface = new GlyphTypeface (source.source.typeface);
		break;
	}
	
	SetIsNull (false);
}

Value::Value (FontResource resource)
{
	Init ();
	k = Type::FONTRESOURCE;
	u.fontresource = new FontResource (resource);
	SetIsNull (false);
}

Value::Value (const PropertyPath *propertypath)
{
	Init ();
	k = Type::PROPERTYPATH;
	if (propertypath) {
		u.propertypath = g_new0 (PropertyPath, 1);
		*u.propertypath = *propertypath;
	}
	SetIsNull (propertypath == NULL);
}

Value::Value (Type::Kind kind, void *npobj)
{
	Init ();
	k = kind;
	u.managed_object = npobj;
	SetIsNull (npobj == NULL);
}

Value::Value (Point pt)
{
	Init ();
	k = Type::POINT;
	u.point = g_new (Point, 1);
	*u.point = Point (pt);
	SetIsNull (false);
}

Value::Value (const Uri *uri)
{
	Init ();
	k = Type::URI;
	u.uri = Uri::Clone (uri);
	SetIsNull (false);
}

Value::Value (Rect rect)
{
	Init ();
	k = Type::RECT;
	u.rect = g_new (Rect, 1);
	*u.rect = Rect (rect);
	SetIsNull (false);
}

Value::Value (Size size)
{
	Init ();
	k = Type::SIZE;
	u.size = g_new (Size, 1);
	*u.size = Size (size);
	SetIsNull (false);
}

Value::Value (RepeatBehavior repeat)
{
	Init();
	k = Type::REPEATBEHAVIOR;
	u.repeat = g_new (RepeatBehavior, 1);
	*u.repeat = RepeatBehavior (repeat);
	SetIsNull (false);
}

Value::Value (Duration duration)
{
	Init();
	k = Type::DURATION;
	u.duration = g_new (Duration, 1);
	*u.duration = Duration (duration);
	SetIsNull (false);
}

Value::Value (KeyTime keytime)
{
	Init ();
	k = Type::KEYTIME;
	u.keytime = g_new (KeyTime, 1);
	*u.keytime = KeyTime (keytime);
	SetIsNull (false);
}

Value::Value (const char *s, Type::Kind kind, bool take)
{
	Init ();
	k = kind;
	
	u.s = take ? (char *) s : g_strdup (s);
	SetIsNull (s == NULL);
}

Value::Value (GridLength grid_length)
{
	Init ();
	k = Type::GRIDLENGTH;
	u.grid_length = g_new (GridLength, 1);
	*u.grid_length = GridLength (grid_length);
	SetIsNull (false);
}

Value::Value (Thickness thickness)
{
	Init ();
	k = Type::THICKNESS;
	u.thickness = g_new (Thickness, 1);
	*u.thickness = Thickness (thickness);
	SetIsNull (false);
}

Value::Value (CornerRadius corner)
{
	Init ();
	k = Type::CORNERRADIUS;
	u.corner = g_new (CornerRadius, 1);
	*u.corner = CornerRadius (corner);
	SetIsNull (false);
}

Value::Value (AudioFormat format)
{
	Init ();
	k = Type::AUDIOFORMAT;
	u.audioformat = g_new (AudioFormat, 1);
	*u.audioformat = AudioFormat (format);
	SetIsNull (false);
}

Value::Value (VideoFormat format)
{
	Init ();
	k = Type::VIDEOFORMAT;
	u.videoformat = g_new (VideoFormat, 1);
	*u.videoformat = VideoFormat (format);
	SetIsNull (false);
}

Value::Value (AudioFormat *format)
{
	Init ();
	k = Type::AUDIOFORMAT;
	u.audioformat = g_new (AudioFormat, 1);
	*u.audioformat = AudioFormat (*format);
	SetIsNull (false);
}

Value::Value (VideoFormat *format)
{
	Init ();
	k = Type::VIDEOFORMAT;
	u.videoformat = g_new (VideoFormat, 1);
	*u.videoformat = VideoFormat (*format);
	SetIsNull (false);
}

Value::Value (GlyphTypeface *typeface)
{
	Init ();
	k = Type::GLYPHTYPEFACE;
	u.typeface = new GlyphTypeface (typeface);
	SetIsNull (false);
}

Value::Value (ManagedTypeInfo *type_info)
{
	Init ();
	k = Type::MANAGEDTYPEINFO;
	u.type_info = type_info ? new ManagedTypeInfo (*type_info) : NULL;
	SetIsNull (false);
}

bool
Value::IsEventObject (Deployment *d) const
{
	return d->GetTypes ()->Find (k)->IsEventObject ();
}

bool
Value::IsDependencyObject (Deployment *d) const
{
	return d->GetTypes ()->Find (k)->IsDependencyObject ();
}

GCHandle
Value::AsGCHandle () const
{
	if (GetIsManaged ())
		return (GCHandle) u.managed_object;
	if (u.dependency_object == NULL)
		return GCHandle::Zero;
#if DEBUG
	g_return_val_if_fail (IsEventObject (Deployment::GetCurrent ()), GCHandle::Zero);
#endif
	return u.dependency_object->GetManagedHandle ();
}

bool
Value::HoldManagedRef (Deployment *deployment)
{
	if (GetIsManaged ())
		return true;
	if (IsEventObject (deployment)) {
		// These types are DOs in native and structs in managed,
		// so it's impossible (currently) to hold a managed ref to them
		// As they can't reference other objects, there's no chance of
		// creating an immortal subtree, so it's simplest to just return
		// false here so we don't weaken the Value* where we store them.
		if (GetKind () == Type::UNMANAGEDMATRIX ||
			GetKind () == Type::UNMANAGEDMATRIX3D ||
			GetKind () == Type::UNMANAGEDSTYLUSPOINT)
			return false;
		return u.dependency_object && u.dependency_object->hadManagedPeer;
	}
	return false;
}

void
Value::Strengthen (Deployment *deployment)
{
	if (GetIsManaged ()) {
		if (GCHandle (this->u.managed_object).IsWeak ()) {
			void *handle = this->u.managed_object;
			u.managed_object = deployment->CreateGCHandle (deployment->GetGCHandleTarget (handle)).ToIntPtr ();
			deployment->FreeGCHandle (handle);
		}
	} else if (!GetNeedUnref () && IsEventObject (deployment)) {
		SetNeedUnref (true);
		if (u.dependency_object)
			u.dependency_object->ref ();
	}
}

void
Value::Weaken (Deployment *deployment)
{
	if (GetIsManaged ()) {
		if (GCHandle (this->u.managed_object).IsNormal ()) {
			void *handle = this->u.managed_object;
			u.managed_object = deployment->CreateWeakGCHandle (deployment->GetGCHandleTarget (handle)).ToIntPtr ();
			deployment->FreeGCHandle (handle);
		}
	} else if (GetNeedUnref () && IsEventObject (deployment)) {
		SetNeedUnref (false);
		if (u.dependency_object)
			u.dependency_object->unref ();
	}
}

void
Value::Copy (const Value& v)
{
	padding = v.padding;
	k = v.k;
	u = v.u;

	if ((padding & GCHandleFlag) == GCHandleFlag) {
		u.managed_object = Deployment::GetCurrent ()->CloneGCHandle (u.managed_object).ToIntPtr ();
		return;
	}

	// if we're null the u = v.u assignment above took care of everything we need
	if (GetIsNull ())
		return;

	/* make a copy of the string instead of just the pointer */
	switch (k) {
	case Type::XMLLANGUAGE:
	case Type::STRING:
		u.s = g_strdup (v.u.s);
		break;
	case Type::FONTFAMILY:
		if (v.u.fontfamily) {
			u.fontfamily = g_new (FontFamily, 1);
			u.fontfamily->source = g_strdup (v.u.fontfamily->source);
		}
		break;
	case Type::FONTSOURCE:
		if (v.u.fontsource) {
			u.fontsource = g_new (FontSource, 1);
			u.fontsource->type = v.u.fontsource->type;
			
			switch (u.fontsource->type) {
			case FontSourceTypeManagedStream:
				u.fontsource->source.stream = g_new (ManagedStreamCallbacks, 1);
				memcpy (u.fontsource->source.stream, v.u.fontsource->source.stream, sizeof (ManagedStreamCallbacks));
				break;
			case FontSourceTypeGlyphTypeface:
				u.fontsource->source.typeface = new GlyphTypeface (v.u.fontsource->source.typeface);
				break;
			}
		}
		break;
	case Type::FONTRESOURCE:
		if (v.u.fontresource) {
			u.fontresource = new FontResource (*v.u.fontresource);
		}
		break;
	case Type::FONTWEIGHT:
		if (v.u.fontweight) {
			u.fontweight = g_new (FontWeight, 1);
			*u.fontweight = *v.u.fontweight;
		}
		break;
	case Type::FONTSTRETCH:
		if (v.u.fontstretch) {
			u.fontstretch = g_new (FontStretch, 1);
			*u.fontstretch = *v.u.fontstretch;
		}
		break;
	case Type::FONTSTYLE:
		if (v.u.fontstyle) {
			u.fontstyle = g_new (FontStyle, 1);
			*u.fontstyle = *v.u.fontstyle;
		}
		break;
	case Type::PROPERTYPATH:
		if (v.u.propertypath) {
			u.propertypath = g_new (PropertyPath, 1);
			u.propertypath->path = g_strdup (v.u.propertypath->path);
			u.propertypath->expanded_path = g_strdup (v.u.propertypath->expanded_path);
			u.propertypath->property = v.u.propertypath->property;
		}
		break;
	case Type::COLOR:
		if (v.u.color) {
			u.color = g_new (Color, 1);
			*u.color = *v.u.color;
		}
		break;
	case Type::POINT:
		if (v.u.point) {
			u.point = g_new (Point, 1);
			*u.point = *v.u.point;
		}
		break;
	case Type::RECT:
		if (v.u.rect) {
			u.rect = g_new (Rect, 1);
			*u.rect = *v.u.rect;
		}
		break;
	case Type::SIZE:
		if (v.u.size) {
			u.size = g_new (Size, 1);
			*u.size = *v.u.size;
		}
		break;
	case Type::URI:
		if (v.u.uri) {
			u.uri = Uri::Clone (v.u.uri);
		} else {
			u.uri = NULL;
		}
		break;
	case Type::REPEATBEHAVIOR:
		if (v.u.repeat) {
			u.repeat = g_new (RepeatBehavior, 1);
			*u.repeat = *v.u.repeat;
		}
		break;
	case Type::DURATION:
		if (v.u.duration) {
			u.duration = g_new (Duration, 1);
			*u.duration = *v.u.duration;
		}
		break;
	case Type::KEYTIME:
		if (v.u.keytime) {
			u.keytime = g_new (KeyTime, 1);
			*u.keytime = *v.u.keytime;
		}
		break;
	case Type::GRIDLENGTH:
		if (v.u.grid_length) {
			u.grid_length = g_new (GridLength, 1);
			*u.grid_length = *v.u.grid_length;
		}
		break;
	case Type::THICKNESS:
		if (v.u.thickness) {
			u.thickness = g_new (Thickness, 1);
			*u.thickness = *v.u.thickness;
		}
		break;
	case Type::CORNERRADIUS:
		if (v.u.corner) {
			u.corner = g_new (CornerRadius, 1);
			*u.corner = *v.u.corner;
		}
		break;
	case Type::AUDIOFORMAT:
		if (v.u.audioformat) {
			u.audioformat = g_new (AudioFormat, 1);
			*u.audioformat = *v.u.audioformat;
		}
		break;
	case Type::VIDEOFORMAT:
		if (v.u.videoformat) {
			u.videoformat = g_new (VideoFormat, 1);
			*u.videoformat = *v.u.videoformat;
		}
		break;
	case Type::GLYPHTYPEFACE:
		if (v.u.typeface)
			u.typeface = new GlyphTypeface (v.u.typeface);
		else
			u.typeface = NULL;
		break;
	case Type::MANAGEDTYPEINFO:
		if (v.u.type_info) {
			u.type_info = new ManagedTypeInfo (*v.u.type_info);
		}
		break;
	default:
		if (IsEventObject (Deployment::GetCurrent ()) && u.dependency_object) {
			u.dependency_object->ref ();
			SetNeedUnref (true);
			u.dependency_object->AddHandler (EventObject::DestroyedEvent, EventObject::ClearWeakRef, &u.dependency_object);
		}
		break;
	}
}

void
Value::FreeValueInternal ()
{
	if ((padding & GCHandleFlag) == GCHandleFlag) {
		Deployment::GetCurrent ()->FreeGCHandle ((GCHandle) u.managed_object);
		return;
	}

	switch (GetKind ()) {
	case Type::XMLLANGUAGE:
	case Type::STRING:
		g_free (u.s);
		break;
	case Type::COLOR:
		g_free (u.color);
		break;
	case Type::FONTFAMILY:
		if (u.fontfamily) {
			g_free (u.fontfamily->source);
			g_free (u.fontfamily);
		}
		break;
	case Type::FONTWEIGHT:
		g_free (u.fontweight);
		break;
	case Type::FONTSTYLE:
		g_free (u.fontstyle);
		break;
	case Type::FONTSTRETCH:
		g_free (u.fontstretch);
		break;
	case Type::FONTSOURCE:
		if (u.fontsource) {
			switch (u.fontsource->type) {
			case FontSourceTypeManagedStream:
				g_free (u.fontsource->source.stream);
				break;
			case FontSourceTypeGlyphTypeface:
				delete u.fontsource->source.typeface;
			}
			
			g_free (u.fontsource);
		}
		break;
	case Type::FONTRESOURCE:
		delete u.fontresource;
		break;
	case Type::PROPERTYPATH:
		if (u.propertypath) {
			g_free (u.propertypath->path);
			g_free (u.propertypath->expanded_path);
			g_free (u.propertypath);
		}
		break;
	case Type::POINT:
		g_free (u.point);
		break;
	case Type::RECT:
		g_free (u.rect);
		break;
	case Type::SIZE:
		g_free (u.size);
		break;
	case Type::URI:
		delete u.uri;
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
	case Type::GRIDLENGTH:
		g_free (u.grid_length);
		break;
	case Type::THICKNESS:
		g_free (u.thickness);
		break;
	case Type::CORNERRADIUS:
		g_free (u.corner);
		break;
	case Type::AUDIOFORMAT:
		g_free (u.audioformat);
		break;
	case Type::VIDEOFORMAT:
		g_free (u.videoformat);
		break;
	case Type::GLYPHTYPEFACE:
		delete u.typeface;
		break;
	case Type::MANAGEDTYPEINFO:
		delete u.type_info;
		break;
	default: {
		Deployment *depl = Deployment::GetCurrent();
		if (IsEventObject (depl)) {
			if (u.dependency_object) {
				u.dependency_object->RemoveHandler (EventObject::DestroyedEvent, EventObject::ClearWeakRef, &u.dependency_object);
				if (GetNeedUnref ()) {
					LOG_VALUE ("unref Value [%p] %s\n", this, GetName());
					u.dependency_object->unref ();
				}
			}
		}
	}
	}
}

char *
Value::ToString () const
{
	GString *str = g_string_new ("");
	
	switch (k) {
	case Type::DOUBLE:
		g_string_append_printf (str, "{ %f }", u.d);
		break;
	case Type::XMLLANGUAGE:
	case Type::STRING:
		g_string_append (str, u.s);
		break;
	case Type::COLOR:
		g_string_append_printf (str, "{%g/%g/%g/%g}", u.color->r, u.color->g, u.color->b, u.color->a);
		break;
	case Type::POINT:
		g_string_append_printf (str, "{ %g, %g }", (u.point)->x, (u.point)->y);
		break;
	case Type::SIZE:
		g_string_append_printf (str, "{ %g, %g }", (u.size)->width, (u.size)->height);
		break;
	case Type::RECT:
		g_string_append_printf (str, "{ x=%g, y=%g, w=%g, h=%g }", (u.rect)->x, (u.rect)->y, (u.rect)->width, (u.rect)->height);
		break;
	case Type::REPEATBEHAVIOR:
		if (u.repeat->IsForever ())
			g_string_append (str, "{repeat=forever}");
		else if ((u.repeat)->HasDuration ())
			g_string_append_printf (str, "{repeat=duration}");
		else if ((u.repeat)->HasCount ())
			g_string_append_printf (str, "{repeat=count %g}", (u.repeat)->GetCount ());
		break;
	case Type::THICKNESS:
		g_string_append_printf (str, "{ l=%g, t=%g, r=%g, b=%g }", (u.thickness)->left, (u.thickness)->top, (u.thickness)->right, (u.thickness)->bottom);
		break;
	case Type::DURATION:
		g_string_append_printf (str, "{duration/TODO}");
		break;
	case Type::KEYTIME:
		g_string_append_printf (str, "{keytime/TODO}");
		break;
	case Type::GRIDLENGTH:
		g_string_append_printf (str, "{gridlength value:%.2f type:%d}", u.grid_length->val, u.grid_length->type);
		break;
	default:
		if (Is (Deployment::GetCurrent (), Type::EVENTOBJECT) && u.dependency_object)
			g_string_append_printf (str, "[%s <%s>]", u.dependency_object->GetTypeName (), Is (Deployment::GetCurrent (), Type::DEPENDENCY_OBJECT) ? AsDependencyObject ()->GetName () : "no name");
		else
			g_string_append_printf (str, "UnknownType");
		break;
	}
	
	return g_string_free (str, FALSE);
}

bool
Value::operator!= (const Value &v) const
{
	return !(*this == v);
}

bool 
Value::operator== (const Value &v) const
{
	if (k != v.k)
		return false;
	
	if ((padding & EqualityMask) != (v.padding & EqualityMask))
		return false;

	// If both values are equal and this is null, then both are null.
	// We can bail out early.
	if (GetIsNull ())
		return true;

	if ((padding & GCHandleFlag) == GCHandleFlag) {
		Deployment *deployment = Deployment::GetCurrent ();
		return deployment->GetGCHandleTarget (u.managed_object) == deployment->GetGCHandleTarget (v.u.managed_object);
	}

	switch (k) {
	case Type::XMLLANGUAGE:
	case Type::STRING:
		if (u.s == NULL){
			return v.u.s == NULL;
		} else if (v.u.s == NULL)
			return false;

		return !strcmp (u.s, v.u.s);
	/* don't use memcmp for comparing structures, structures may have uninitialized padding,
	 * which would cause random behaviour */
	case Type::FONTFAMILY:
		return *u.fontfamily == *v.u.fontfamily;
	case Type::FONTWEIGHT:
		return *u.fontweight == *v.u.fontweight;
	case Type::FONTSTYLE:
		return *u.fontstyle == *v.u.fontstyle;
	case Type::FONTSTRETCH:
		return *u.fontstretch == *v.u.fontstretch;
	case Type::FONTSOURCE:
		if (u.fontsource && v.u.fontsource) {
			if (u.fontsource->type != v.u.fontsource->type)
				return false;
			
			switch (u.fontsource->type) {
			case FontSourceTypeManagedStream:
				return u.fontsource->source.stream->handle == v.u.fontsource->source.stream->handle;
			case FontSourceTypeGlyphTypeface:
				return u.fontsource->source.typeface == v.u.fontsource->source.typeface;
			}
		}
		
		return u.fontsource == v.u.fontsource;
	case Type::FONTRESOURCE:
		if (u.fontresource && v.u.fontresource)
			return *u.fontresource == *v.u.fontresource;
		else
			return u.fontresource == v.u.fontresource;

	case Type::PROPERTYPATH:
		return *u.propertypath == *v.u.propertypath;
	case Type::COLOR:
		if (u.color == v.u.color)
			return true;
		if (u.color == NULL || v.u.color == NULL)
			return false;
		return *u.color == *v.u.color;
	case Type::POINT:
		return *u.point == *v.u.point;
	case Type::RECT:
		return *u.rect == *v.u.rect;
	case Type::SIZE:
		return *u.size == *v.u.size;
	case Type::REPEATBEHAVIOR:
		return *u.repeat == *v.u.repeat;
	case Type::DURATION:
		return *u.duration == *v.u.duration;
	case Type::KEYTIME:
		return *u.keytime == *v.u.keytime;
	case Type::GRIDLENGTH:
		return *u.grid_length == *v.u.grid_length;
	case Type::THICKNESS:
		return *u.thickness == *v.u.thickness;
	case Type::CORNERRADIUS:
		return *u.corner == *v.u.corner;
	case Type::AUDIOFORMAT:
		return *u.audioformat == *v.u.audioformat;
	case Type::VIDEOFORMAT:
		return *u.videoformat == *v.u.videoformat;
	case Type::MANAGEDTYPEINFO:
		return u.type_info == v.u.type_info;
	case Type::URI:
		if (!u.uri)
			return !v.u.uri;
		if (!v.u.uri)
			return false;
		return *u.uri == *v.u.uri;
	case Type::GLYPHTYPEFACE:
		if (!u.typeface)
			return !v.u.typeface;
		if (!v.u.typeface)
			return false;
		return u.typeface == v.u.typeface;
	case Type::DOUBLE:
		if (isnan (u.d) && isnan (v.u.d))
			return true;
		return fabs (u.d - v.u.d) < DBL_EPSILON;
	case Type::FLOAT:
		return fabs (u.f - v.u.f) < FLT_EPSILON;
	
	default:
		return !memcmp (&u, &v.u, sizeof (u));
	}

	return true;
}

bool
Value::AreEqual (const Value *v1, const Value *v2)
{
	// either one is null but not the other
	if (!!v1 != !!v2)
		return false;

	if (v1 && v2) {
		return *v1 == *v2;
	}
	else if (v1 && v1->GetIsNull ()) {
		// since we know v2 is null, return true here
		return true;
	}
	else if (v2 && v2->GetIsNull ()) {
		// since we know v1 is null, return true here
		return true;
	}

	return false;
}

Value&
Value::operator= (const Value& other)
{
	if (this != &other) {
		FreeValueInternal ();
		Copy (other);
	}
	return *this;
}

//
// This is invoked by managed code to free the contents of the value
//
void 
Value::FreeValue (Value* value)
{
	if (value == NULL)
		return;
	value->FreeValueInternal ();
}

// This is just a callback we can use in our GHashTable destructors
void
Value::DeleteValue (Value *value)
{
	delete value;
}

void
Value::FreeValue2 (Value* value)
{
	FreeValue (value);
}

void
Value::DeleteValue2 (Value *value)
{
	delete value;
}

Value::~Value ()
{
	FreeValueInternal ();
}

#if DEBUG || LOGGING
char *
Value::GetName ()
{
	GString *str = g_string_new ("");

	switch (k) {
	case Type::DOUBLE:
		g_string_append_printf (str, "DOUBLE");
		break;
	case Type::XMLLANGUAGE:
		g_string_append_printf (str, "XMLLANGUAGE");
		break;
	case Type::STRING:
		g_string_append_printf (str, "STRING");
		break;
	case Type::COLOR:
		g_string_append_printf (str, "COLOR");
		break;
	case Type::POINT:
		g_string_append_printf (str, "POINT");
		break;
	case Type::SIZE:
		g_string_append_printf (str, "SIZE");
		break;
	case Type::RECT:
		g_string_append_printf (str, "RECT");
		break;
	case Type::REPEATBEHAVIOR:
		g_string_append_printf (str, "REPEATBEHAVIOR");
		break;
	case Type::THICKNESS:
		g_string_append_printf (str, "THICKNESS");
		break;
	case Type::DURATION:
		g_string_append_printf (str, "DURATION");
		break;
	case Type::KEYTIME:
		g_string_append_printf (str, "KEYTIME");
		break;
	case Type::GRIDLENGTH:
		g_string_append_printf (str, "GRIDLENGTH");
		break;
	default:
		if (u.dependency_object)
			g_string_append_printf (str, "[%s] [%p] %d", u.dependency_object->GetTypeName (), u.dependency_object, u.dependency_object->GetRefCount ());
		else
			g_string_append_printf (str, "UnknownType");
		break;
	}

	return g_string_free (str, FALSE);
}
#endif

};
