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
#include "managedtypeinfo.h"

namespace Moonlight {

//
// Style
//
/* @ContentProperty="Setters" */
/* @Namespace=System.Windows */
class Style : public DependencyObject {
public:
	/* @PropertyType=Style,GenerateAccessors,ManagedFieldAccess=Internal,Validator=BasedOnValidator */
	const static int BasedOnProperty;
	/* @PropertyType=bool,DefaultValue=false,ManagedSetterAccess=Private,GenerateAccessors,ManagedFieldAccess=Internal */
	const static int IsSealedProperty;
	/* @PropertyType=SetterBaseCollection,AutoCreateValue,Access=Internal,ManagedFieldAccess=Private,ManagedAccess=Public,ManagedSetterAccess=Private,GenerateAccessors */
	const static int SettersProperty;
	/* @PropertyType=ManagedTypeInfo,ManagedPropertyType=System.Type,Access=Internal,ManagedAccess=Public,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int TargetTypeProperty;
	
	/* @GeneratePInvoke */
	Style ();
	
	/* @GeneratePInvoke */
	void Seal ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	virtual void OnIsAttachedChanged (bool value);

	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);

	//
	// Property Accessors
	//
	void SetBasedOn (Style *style);
	Style *GetBasedOn ();
	
	void SetSetters (SetterBaseCollection *setters);
	SetterBaseCollection *GetSetters ();
	
	void SetIsSealed (bool sealed);
	bool GetIsSealed ();
	
	void SetTargetType (ManagedTypeInfo *type_info);
	ManagedTypeInfo *GetTargetType ();

	const static int SettersChangedEvent;
	const static int DetachedEvent;

protected:
	virtual ~Style ();
};

//
// SetterBaseCollection
//
/* @Namespace=System.Windows */
class SetterBaseCollection : public DependencyObjectCollection {
public:
	/* @GeneratePInvoke */
	SetterBaseCollection ();
	
  	/* @PropertyType=bool,DefaultValue=false,ManagedFieldAccess=Internal,ManagedSetterAccess=Private,GenerateAccessors */
	const static int IsSealedProperty;

	virtual bool AddedToCollection (Value *value, MoonError *error);
	virtual void RemovedFromCollection (Value *value, bool is_value_safe);
	
	virtual Type::Kind GetElementType () { return Type::SETTERBASE; }
	
	void Seal ();
	
	//
	// Property Accessors
	//
	void SetIsSealed (bool sealed);
	bool GetIsSealed ();
	
protected:
	virtual ~SetterBaseCollection () { }

private:
	bool ValidateSetter (Value *value, MoonError *error);
};

//
// SetterBase
//
/* @Namespace=System.Windows */
class SetterBase : public DependencyObject {
 protected:
	virtual ~SetterBase () { }
	
 public:
   	/* @PropertyType=bool,DefaultValue=false,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int IsSealedProperty;
	
	/* @GeneratePInvoke,ManagedAccess=Internal */
	SetterBase ();
	
	void Seal ();
	
	//
	// Property Accessors
	//
	void SetAttached (bool attached);
	bool GetAttached ();
	
	void SetIsSealed (bool sealed);
	bool GetIsSealed ();
	
 private:
	bool attached;
};

//
// Setter
//
/* @Namespace=System.Windows */
/* @ManagedDependencyProperties=Manual */
class Setter : public SetterBase {
 protected:
	virtual ~Setter ();
	
 public:
	/* @PropertyType=DependencyProperty,Validator=IsSetterSealedValidator */
	const static int PropertyProperty;
	/* @PropertyType=object,Validator=IsSetterSealedValidator */
	const static int ValueProperty;

	/* @PropertyType=object */
	const static int ConvertedValueProperty;

	/* @GeneratePInvoke */
	Setter ();

	virtual bool PermitsMultipleParents () { return false; }
};


class DeepStyleWalker {
 public:
	DeepStyleWalker (Style *style, Types *types = NULL);
	DeepStyleWalker (Style **styles, Types *types = NULL);
	~DeepStyleWalker ();
	Setter *Step ();
 private:
	int offset;
	GPtrArray *setter_list;
	Types *types;
	static gint SetterComparer (gconstpointer left, gconstpointer right);
};

};
#endif /* __MOON_STYLE_H__ */
