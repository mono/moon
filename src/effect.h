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
#include "rect.h"

#define MAX_SAMPLERS  16
#define MAX_CONSTANTS 32
#define MAX_REGS      MAX_CONSTANTS

#if MAX_REGS < MAX_SAMPLERS || MAX_REGS < MAX_CONSTANTS
#error MAX_REGS is not great enough
#endif

typedef struct st_context st_context_t;
typedef struct pipe_resource pipe_resource_t;
typedef struct pipe_surface pipe_surface_t;
typedef struct pipe_transfer pipe_transfer_t;

typedef struct _d3d_swizzle {
	unsigned int x;
	unsigned int y;
	unsigned int z;
	unsigned int w;
} d3d_swizzle_t;

typedef struct _d3d_destination_parameter {
	unsigned int regtype;
	unsigned int regnum;
	unsigned int writemask;
	unsigned int dstmod;
} d3d_destination_parameter_t;

typedef struct _d3d_source_parameter {
	unsigned int regtype;
	unsigned int regnum;
	d3d_swizzle_t swizzle;
	unsigned int srcmod;
} d3d_source_parameter_t;

typedef struct _d3d_version {
	unsigned int major;
	unsigned int minor;
	unsigned int type;
} d3d_version_t;

typedef struct _d3d_op_metadata_t {
	const char *name;
	unsigned int ndstparam;
	unsigned int nsrcparam;
} d3d_op_metadata_t;

typedef struct _d3d_op {
	unsigned int type;
	unsigned int length;
	unsigned int comment_length;
	d3d_op_metadata_t meta;
} d3d_op_t;

typedef struct _d3d_def_instruction {
	d3d_destination_parameter_t reg;
	float v[4];
} d3d_def_instruction_t;

typedef struct _d3d_dcl_instruction {
	unsigned int usage;
	unsigned int usageindex;
	d3d_destination_parameter_t reg;
} d3d_dcl_instruction_t;

namespace Moonlight {

/* @Namespace=System.Windows.Media.Effects */
class Effect : public DependencyObject {
public:
	//
	// Padding
	//
	virtual Thickness Padding () { return Thickness (); }

	//
	// Render
	//
	virtual bool Render (cairo_t         *cr,
			     cairo_surface_t *src,
			     const double    *matrix,
			     double          x,
			     double          y,
			     double          width,
			     double          height);

	static void Initialize ();
	static void Shutdown ();

	static Effect *GetProjectionEffect ();

protected:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Effect ();

	virtual ~Effect () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

	pipe_resource_t *GetShaderTexture (cairo_surface_t *surface);
	pipe_surface_t  *GetShaderSurface (cairo_surface_t *surface);
	pipe_resource_t *CreateVertexBuffer (pipe_resource_t *texture,
					     const double    *matrix,
					     double          x,
					     double          y,
					     double          width,
					     double          height,
					     double          s,
					     double          t);
	pipe_resource_t *CreateVertexBuffer (pipe_resource_t *texture,
					     const double    *matrix,
					     double          x,
					     double          y,
					     double          width,
					     double          height)
	{
		return CreateVertexBuffer (texture, matrix,
					   x, y, width, height,
					   1.0, 1.0);
	}

	void DrawVertexBuffer (pipe_surface_t  *dst,
			       pipe_resource_t *vertices,
			       double          dstX,
			       double          dstY,
			       const Rect      *clip);
	void DrawVertexBuffer (pipe_surface_t  *dst,
			       pipe_resource_t *vertices,
			       double          dstX,
			       double          dstY)
	{
		DrawVertexBuffer (dst, vertices, dstX, dstY, NULL);
	}

	virtual bool Composite (pipe_surface_t  *dst,
				pipe_resource_t *src,
				const double    *matrix,
				double          dstX,
				double          dstY,
				const Rect      *clip,
				double          x,
				double          y,
				double          width,
				double          height);

	virtual void UpdateShader ();
	void MaybeUpdateShader ();

	static int CalculateGaussianSamples (double radius,
					     double precision,
					     double *row);
	static void UpdateFilterValues (double radius,
					double *values,
					int    ***table,
					int    *size);

	bool need_update;

	static st_context_t *st_context;

	static cairo_user_data_key_t textureKey;
	static cairo_user_data_key_t surfaceKey;
	static cairo_user_data_key_t matrixKey;
	static cairo_user_data_key_t offsetXKey;
	static cairo_user_data_key_t offsetYKey;

	static int filtertable0[256];

	static Effect *projection;
};

#define MAX_BLUR_RADIUS 20

/* @Namespace=System.Windows.Media.Effects */
class BlurEffect : public Effect {
public:
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

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
	Thickness Padding ();

	//
	// Render
	//
	bool Render (cairo_t         *cr,
		     cairo_surface_t *src,
		     const double    *matrix,
		     double          x,
		     double          y,
		     double          width,
		     double          height);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	BlurEffect ();

	virtual ~BlurEffect ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

	void Clear ();
	void MaybeUpdateFilter ();

	bool Composite (pipe_surface_t  *dst,
			pipe_resource_t *src,
			const double    *matrix,
			double          dstX,
			double          dstY,
			const Rect      *clip,
			double          x,
			double          y,
			double          width,
			double          height);

	void UpdateShader ();

	void *fs;

	pipe_resource_t *horz_pass_constant_buffer;
	pipe_resource_t *vert_pass_constant_buffer;

	int filter_size;

	int    nfiltervalues;
	double filtervalues[MAX_BLUR_RADIUS + 1];

	int **filtertable;
	bool need_filter_update;
};

#define MAX_SHADOW_DEPTH 300

/* @Namespace=System.Windows.Media.Effects */
class DropShadowEffect : public Effect {
public:
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	/* @PropertyType=double,DefaultValue=5.0,GenerateAccessors */
	const static int BlurRadiusProperty;
	/* @PropertyType=Color,DefaultValue=Color(0xFF000000),GenerateAccessors */
	const static int ColorProperty;
	/* @PropertyType=double,DefaultValue=315.0,GenerateAccessors */
	const static int DirectionProperty;
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int OpacityProperty;
	/* @PropertyType=double,DefaultValue=5.0,GenerateAccessors */
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

	//
	// Padding
	//
	Thickness Padding ();

	//
	// Render
	//
	bool Render (cairo_t         *cr,
		     cairo_surface_t *src,
		     const double    *matrix,
		     double          x,
		     double          y,
		     double          width,
		     double          height);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	DropShadowEffect ();

	virtual ~DropShadowEffect ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

	void Clear ();
	void MaybeUpdateFilter ();

	bool Composite (pipe_surface_t  *dst,
			pipe_resource_t *src,
			const double    *matrix,
			double          dstX,
			double          dstY,
			const Rect      *clip,
			double          x,
			double          y,
			double          width,
			double          height);

	void UpdateShader ();

	void *horz_fs;
	void *vert_fs;

	pipe_resource_t *horz_pass_constant_buffer;
	pipe_resource_t *vert_pass_constant_buffer;

	int filter_size;

	int    nfiltervalues;
	double filtervalues[MAX_BLUR_RADIUS + 1];

	int **filtertable;
	bool need_filter_update;
};

/* @Namespace=System.Windows.Media.Effects */
class PixelShader : public DependencyObject {
public:
	/* @PropertyType=Uri,DefaultValue=Uri(),GenerateAccessors */
	const static int UriSourceProperty;

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	//
	// Property Accessors
	//
	Uri* GetUriSource ();
	void SetUriSource (Uri *uri);

	void SetTokensFromPath (const char *path);

	int GetToken (int     index,
		      guint32 *token);
	int GetToken (int   index,
		      float *token);

	int GetVersion (int           index,
			d3d_version_t *value);

	int GetOp (int      index,
		   d3d_op_t *value);

	int GetDestinationParameter (int                         index,
				     d3d_destination_parameter_t *value);
	int GetSourceParameter (int                    index,
				d3d_source_parameter_t *value);

	int GetInstruction (int                   index,
			    d3d_def_instruction_t *value);
	int GetInstruction (int                   index,
			    d3d_dcl_instruction_t *value);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	PixelShader ();

	virtual ~PixelShader ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	guint32 *tokens;
	unsigned int ntokens;
};

/* @Namespace=System.Windows.Media.Effects */
class ShaderEffect : public Effect {
public:
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	/* @PropertyType=PixelShader,ManagedFieldAccess=Protected,ManagedAccess=Protected,GenerateAccessors */
	const static int PixelShaderProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal,ManagedAccess=Protected,GenerateAccessors */
	const static int PaddingTopProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal,ManagedAccess=Protected,GenerateAccessors */
	const static int PaddingBottomProperty;
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

	/* @GenerateCBinding,GeneratePInvoke */
	void UpdateShaderConstant (int reg, double x, double y, double z, double w);

	/* @GenerateCBinding,GeneratePInvoke */
	void UpdateShaderSampler (int reg, int mode, Brush *input);

	//
	// Padding
	//
	Thickness Padding ();

	void ShaderError (const char *format, ...);

protected:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	ShaderEffect ();

	virtual ~ShaderEffect () { Clear (); }

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

	void Clear ();

	pipe_resource_t *GetShaderConstantBuffer (float           **ptr,
						  pipe_transfer_t **ptr_transfer);

	bool Composite (pipe_surface_t  *dst,
			pipe_resource_t *src,
			const double    *matrix,
			double          dstX,
			double          dstY,
			const Rect      *clip,
			double          x,
			double          y,
			double          width,
			double          height);

	void UpdateShader ();


	pipe_resource_t *constant_buffer;
	Brush *sampler_input[MAX_SAMPLERS];
	unsigned int sampler_filter[MAX_SAMPLERS];
	unsigned int sampler_last;

	void *fs;
};

/* @Namespace=None,ManagedDependencyProperties=None */
class ProjectionEffect : public Effect {
public:
	ProjectionEffect ();

	//
	// Render
	//
	bool Render (cairo_t         *cr,
		     cairo_surface_t *src,
		     const double    *matrix,
		     double          x,
		     double          y,
		     double          width,
		     double          height);

protected:
	virtual ~ProjectionEffect ();

	bool Composite (pipe_surface_t  *dst,
			pipe_resource_t *src,
			const double    *matrix,
			double          dstX,
			double          dstY,
			const Rect      *clip,
			double          x,
			double          y,
			double          width,
			double          height);

	void UpdateShader ();
};

};
#endif /* __MOONLIGHT_EFFECT_H__ */
