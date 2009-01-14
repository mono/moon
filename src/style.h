/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * style.h:
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_STYLE_H__
#define __MOON_STYLE_H__

#include <glib.h>

#include "dependencyobject.h"
#include "collection.h"

//
// Style
//
/* @SilverlightVersion="2" */
/* @ContentProperty="Setters" */
/* @Namespace=System.Windows */
class Style : public DependencyObject {
public:
  	/* @PropertyType=bool,DefaultValue=false,ManagedSetterAccess=Private,GenerateAccessors */
	static DependencyProperty *IsSealedProperty;
 	/* @PropertyType=SetterBaseCollection,Access=Internal,ManagedFieldAccess=Private,ManagedAccess=Public,ManagedSetterAccess=Private,GenerateAccessors */
	static DependencyProperty *SettersProperty;
 	/* @PropertyType=ManagedTypeInfo,ManagedPropertyType=System.Type,Access=Internal,ManagedAccess=Public,ManagedFieldAccess=Internal */
	static DependencyProperty *TargetTypeProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Style ();
	
	virtual Type::Kind GetObjectType () { return Type::STYLE; }

	SetterBaseCollection* GetSetters();
	void SetSetters (SetterBaseCollection* value);

	bool GetIsSealed();
	void SetIsSealed(bool value);

	/* @GenerateCBinding,GeneratePInvoke */
	void Seal ();

protected:
	virtual ~Style ();
};

//
// SetterBaseCollection
//
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class SetterBaseCollection : public DependencyObjectCollection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	SetterBaseCollection ();
	
  	/* @PropertyType=bool,DefaultValue=false,ManagedSetterAccess=Private,GenerateAccessors */
	static DependencyProperty *IsSealedProperty;

	virtual bool AddedToCollection (Value *value, MoonError *error);
	virtual void RemovedFromCollection (Value *value);
	
	virtual Type::Kind GetObjectType () { return Type::SETTERBASE_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::SETTERBASE; }

	bool GetIsSealed();
	void SetIsSealed(bool value);
	
	void Seal ();

	void SetStyle (Style *style);

protected:
	virtual ~SetterBaseCollection () { }

private:
	Style *style;
	bool ValidateSetter (Value *value, MoonError *error);
};

//
// SetterBase
//
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class SetterBase : public DependencyObject {
 protected:
	virtual ~SetterBase () { }
	
 public:
   	/* @PropertyType=bool,DefaultValue=false,ManagedSetterAccess=Private,GenerateAccessors */
	static DependencyProperty *IsSealedProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	SetterBase ();

	virtual bool SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error);
	virtual Type::Kind GetObjectType () { return Type::SETTERBASE; }
	
	bool GetAttached ();
	void SetAttached (bool value);
	
	bool GetIsSealed();
	void SetIsSealed(bool value);
	
	void Seal ();
 private:
	bool attached;
};

//
// Setter
//
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
/* @ManagedDependencyProperties=Manual */
class Setter : public SetterBase {
 protected:
	virtual ~Setter () { }
	
 public:
	/* @PropertyType=DependencyProperty */
	static DependencyProperty *PropertyProperty;
	/* @PropertyType=object */
	static DependencyProperty *ValueProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Setter ();

	virtual Type::Kind GetObjectType () { return Type::SETTER; }
};

#endif /* __MOON_STYLE_H__ */
