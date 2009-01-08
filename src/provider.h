/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * provider.h: an api for PropertyValue providers (for property inheritance)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_PROVIDER_H__
#define __MOON_PROVIDER_H__

#include <glib.h>

class DependencyObject;
class DependencyProperty;
struct Value;

enum PropertyPrecedence {
	PropertyPrecedence_Animation,
	PropertyPrecedence_LocalValue,
	PropertyPrecedence_Style,
	PropertyPrecedence_Inherited,
	PropertyPrecedence_NaturalMediaSize,
	PropertyPrecedence_DefaultValue,

	PropertyPrecedence_Count,
};

class PropertyValueProvider {
public:
	PropertyValueProvider (DependencyObject *obj);
	virtual ~PropertyValueProvider ();

	virtual Value* GetPropertyValue (DependencyProperty *property) = 0;

protected:
	DependencyObject *obj;
};


class AnimationPropertyValueProvider : public PropertyValueProvider {
public:
	AnimationPropertyValueProvider (DependencyObject *obj);
	virtual ~AnimationPropertyValueProvider ();

	virtual Value* GetPropertyValue (DependencyProperty *property);
};

class LocalPropertyValueProvider : public PropertyValueProvider {
public:
	LocalPropertyValueProvider (DependencyObject *obj);
	virtual ~LocalPropertyValueProvider ();

	virtual Value* GetPropertyValue (DependencyProperty *property);
};

class StylePropertyValueProvider : public PropertyValueProvider {
public:
	StylePropertyValueProvider (DependencyObject *obj);
	virtual ~StylePropertyValueProvider ();

	virtual Value* GetPropertyValue (DependencyProperty *property);
};

class InheritedPropertyValueProvider : public PropertyValueProvider {
public:
	InheritedPropertyValueProvider (DependencyObject *obj);
	virtual ~InheritedPropertyValueProvider ();

	virtual Value* GetPropertyValue (DependencyProperty *property);
};

class NaturalMediaSizePropertyValueProvider : public PropertyValueProvider {
public:
	NaturalMediaSizePropertyValueProvider (DependencyObject *obj);
	virtual ~NaturalMediaSizePropertyValueProvider ();

	virtual Value* GetPropertyValue (DependencyProperty *property);
};

class DefaultValuePropertyValueProvider : public PropertyValueProvider {
public:
	DefaultValuePropertyValueProvider (DependencyObject *obj);
	virtual ~DefaultValuePropertyValueProvider ();

	virtual Value* GetPropertyValue (DependencyProperty *property);
};


#endif /* __MOON_PROVIDER_H__ */
