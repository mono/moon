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
#include <cairo.h>
#include "enums.h"
#include "dependencyobject.h"

struct st_context;
struct pipe_buffer;

/* @Namespace=System.Windows.Media.Effects */
class Effect : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Effect ();

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	//
	// Padding
	//
	virtual double GetPaddingTop ();
	virtual double GetPaddingBottom ();
	virtual double GetPaddingLeft ();
	virtual double GetPaddingRight ();

	//
	// Composite
	//
	virtual bool Composite (cairo_t         *cr,
				cairo_surface_t *dst,
				cairo_surface_t *src,
				int             src_x,
				int             src_y,
				int             x,
				int             y,
				unsigned int    width,
				unsigned int    height);

	static void Initialize ();
	static void Shutdown ();

protected:
	virtual ~Effect ();

	virtual void UpdateShader ();
	void MaybeUpdateShader ();

	bool need_update;

	void *vs;
	void *fs;
	struct pipe_buffer *constants;

	static struct st_context *st_context;
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

	//
	// Padding
	//
	double GetPaddingTop ();
	double GetPaddingBottom ();
	double GetPaddingLeft ();
	double GetPaddingRight ();

	//
	// Shader
	//
	void UpdateShader ();

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
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	ShaderEffect ();

	/* @PropertyType=PixelShader,ManagedFieldAccess=Protected,ManagedAccess=Protected,GenerateAccessors */
	const static int PixelShaderProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal,ManagedAccess=Protected,GenerateAccessors */
	const static int PaddingBottomProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal,ManagedAccess=Protected,GenerateAccessors */
	const static int PaddingTopProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal,ManagedAccess=Protected,GenerateAccessors */
	const static int PaddingLeftProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal,ManagedAccess=Protected,GenerateAccessors */
	const static int PaddingRightProperty;
	/* @PropertyType=gint32,ManagedFieldAccess=Internal,ManagedAccess=Protected,GenerateAccessors */
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
