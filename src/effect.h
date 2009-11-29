/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifndef __MOONLIGHT_EFFECT_H__
#define __MOONLIGHT_EFFECT_H__

#include <glib.h>
#include "enums.h"
#include "dependencyobject.h"

/* @Namespace=System.Windows.Media.Effects */
class Effect : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Effect ();

protected:
	virtual ~Effect () {}
};

/* @Namespace=System.Windows.Media.Effects */
class BlurEffect : public Effect {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	BlurEffect ();

	/* @PropertyType=double,DefaultValue=5.0,GenerateAccessors */
	const static int RadiusProperty;

	//
	// Property Accessors
	//
	void SetRadius (double radius);
	double GetRadius ();

protected:
	virtual ~BlurEffect () {}
};

/* @Namespace=System.Windows.Media.Effects */
class DropShadowEffect : public Effect {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	DropShadowEffect ();

	/* @PropertyType=double,DefaultValue=5.0,GenerateAccessors */
	const static int BlurRadiusProperty;
	/* @PropertyType=Color,DefaultValue=Color(0xFF000000),GenerateAccessors */
	const static int ColorProperty;
	/* @PropertyType=double,DefaultValue=315,GenerateAccessors */
	const static int DirectionProperty;
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int OpacityProperty;
	/* @PropertyType=double,DefaultValue=5,GenerateAccessors */
	const static int ShadowDepthProperty;

	//
	// Property Accessors
	//
	void SetBlurRadius (double radius);
	double GetBlurRadius ();

	void SetColor (Color* color);
	Color* GetColor ();

	void SetDirection (double direction);
	double GetDirection ();

	void SetOpacity (double opacity);
	double GetOpacity ();

	void SetShadowDepth (double shadowDepth);
	double GetShadowDepth ();

protected:
	virtual ~DropShadowEffect () {}
};

/* @Namespace=System.Windows.Media.Effects */
class PixelShader : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	PixelShader ();

	/* @PropertyType=Uri,GenerateAccessors */
	const static int UriSourceProperty;

	//
	// Property Accessors
	//
	Uri* GetUriSource ();
	void SetUriSource (Uri *uri);

protected:
	virtual ~PixelShader () {}
};

/* @Namespace=System.Windows.Media.Effects */
class ShaderEffect : public Effect {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	ShaderEffect ();

	/* @PropertyType=PixelShader,GenerateAccessors */
	const static int PixelShaderProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int PaddingBottomProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int PaddingTopProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int PaddingLeftProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int PaddingRightProperty;

	/* @PropertyType=gint32,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int DdxUvDdyUvRegisterIndexProperty;

	// Property Accessors
	PixelShader *GetPixelShader();
	void SetPixelShader (PixelShader *shader);

	double GetPaddingTop ();
	void SetPaddingTop (double pad);

	double GetPaddingBottom ();
	void SetPaddingBottom (double pad);

	double GetPaddingLeft ();
	void SetPaddingLeft (double pad);

	double GetPaddingRight ();
	void SetPaddingRight (double pad);

	int GetDdxUvDdyUvRegisterIndex ();
	void SetDdxUvDdyUvRegisterIndex (gint32 index);

protected:
	virtual ~ShaderEffect () {}
};


#endif /* __MOONLIGHT_EFFECT_H__ */
