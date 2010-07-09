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

/* @Namespace=System.Windows.Media.Effects */
class Effect : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Effect ();

	//
	// Padding
	//
	virtual Thickness Padding () { return Thickness (); }

	//
	// Composite
	//
	virtual bool Composite (cairo_surface_t *dst,
				cairo_surface_t *src,
				Rect            *bounds,
				double          x,
				double          y);

	//
	// ClipAndComposite
	//
	virtual bool ClipAndComposite (cairo_t         *cr,
				       cairo_surface_t *src,
				       double          x,
				       double          y);

	static void Initialize ();
	static void Shutdown ();

	static Effect *GetProjectionEffect ();
	static void SetShaderMatrix (cairo_surface_t *surface,
				     double          *matrix);
	static void SetShaderOffsetX (cairo_surface_t *surface,
				      double          x);
	static void SetShaderOffsetY (cairo_surface_t *surface,
				      double          y);

protected:
	virtual ~Effect () {}

	pipe_resource_t *GetShaderTexture (cairo_surface_t *surface);
	pipe_surface_t  *GetShaderSurface (cairo_surface_t *surface);
	double          *GetShaderMatrix (cairo_surface_t *surface);
	double          GetShaderOffsetX (cairo_surface_t *surface);
	double          GetShaderOffsetY (cairo_surface_t *surface);
	pipe_resource_t *GetShaderVertexBuffer (float           x1,
						float           y1,
						float           x2,
						float           y2,
						unsigned        stride,
						float           **ptr,
						pipe_transfer_t **ptr_transfer);
	void DrawVertices (pipe_surface_t  *surface,
			   pipe_resource_t *vertices,
			   int             nattrib,
			   int             blend_enable);

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
	/* @GenerateCBinding,GeneratePInvoke */
	BlurEffect ();

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
	// Composite
	//
	bool Composite (cairo_surface_t *dst,
			cairo_surface_t *src,
			Rect            *bounds,
			double          x,
			double          y);

	//
	// ClipAndComposite
	//
	bool ClipAndComposite (cairo_t         *cr,
			       cairo_surface_t *src,
			       double          x,
			       double          y);

	//
	// Shader
	//
	void UpdateShader ();

protected:
	virtual ~BlurEffect ();
	void Clear ();
	void MaybeUpdateFilter ();

	void *fs;

	pipe_resource_t *horz_pass_constant_buffer;
	pipe_resource_t *vert_pass_constant_buffer;

	int filter_size;

	int    nfiltervalues;
	double filtervalues[MAX_BLUR_RADIUS + 1];

	int **filtertable;
	bool need_filter_update;
};

/* @Namespace=System.Windows.Media.Effects */
class DropShadowEffect : public Effect {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	DropShadowEffect ();

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
	// Composite
	//
	bool Composite (cairo_surface_t *dst,
			cairo_surface_t *src,
			Rect            *bounds,
			double          x,
			double          y);

	//
	// ClipAndComposite
	//
	bool ClipAndComposite (cairo_t         *cr,
			       cairo_surface_t *src,
			       double          x,
			       double          y);

	//
	// Shader
	//
	void UpdateShader ();

protected:
	virtual ~DropShadowEffect ();
	void Clear ();
	void MaybeUpdateFilter ();

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
	/* @GenerateCBinding,GeneratePInvoke */
	PixelShader ();

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
	virtual ~PixelShader ();

private:
	guint32 *tokens;
	unsigned int ntokens;
};

/* @Namespace=System.Windows.Media.Effects */
class ShaderEffect : public Effect {
public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	ShaderEffect ();

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

	//
	// Composite
	//
	bool Composite (cairo_surface_t *dst,
			cairo_surface_t *src,
			Rect            *bounds,
			double          x,
			double          y);

	//
	// Shader
	//
	void UpdateShader ();

	void ShaderError (const char *format, ...);

protected:
	virtual ~ShaderEffect () { Clear (); }
	void Clear ();

	pipe_resource_t *GetShaderConstantBuffer (float           **ptr,
						  pipe_transfer_t **ptr_transfer);

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
	// Composite
	//
	bool Composite (cairo_surface_t *dst,
			cairo_surface_t *src,
			Rect            *bounds,
			double          x,
			double          y);

	//
	// Shader
	//
	void UpdateShader ();

protected:
	virtual ~ProjectionEffect ();
};

#endif /* __MOONLIGHT_EFFECT_H__ */
