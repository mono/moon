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
#include "rect.h"
#include "context.h"
#include "pal.h"

#define MAX_SAMPLERS  16
#define MAX_CONSTANTS 32

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

typedef enum _shader_instruction_opcode_type {
	D3DSIO_NOP = 0,
	D3DSIO_MOV = 1,
	D3DSIO_ADD = 2,
	D3DSIO_SUB = 3,
	D3DSIO_MAD = 4,
	D3DSIO_MUL = 5,
	D3DSIO_RCP = 6,
	D3DSIO_RSQ = 7,
	D3DSIO_DP3 = 8,
	D3DSIO_DP4 = 9,
	D3DSIO_MIN = 10,
	D3DSIO_MAX = 11,
	D3DSIO_SLT = 12,
	D3DSIO_SGE = 13,
	D3DSIO_EXP = 14,
	D3DSIO_LOG = 15,
	D3DSIO_LIT = 16,
	D3DSIO_DST = 17,
	D3DSIO_LRP = 18,
	D3DSIO_FRC = 19,
	D3DSIO_M4x4 = 20,
	D3DSIO_M4x3 = 21,
	D3DSIO_M3x4 = 22,
	D3DSIO_M3x3 = 23,
	D3DSIO_M3x2 = 24,
	D3DSIO_CALL = 25,
	D3DSIO_CALLNZ = 26,
	D3DSIO_LOOP = 27,
	D3DSIO_RET = 28,
	D3DSIO_ENDLOOP = 29,
	D3DSIO_LABEL = 30,
	D3DSIO_DCL = 31,
	D3DSIO_POW = 32,
	D3DSIO_CRS = 33,
	D3DSIO_SGN = 34,
	D3DSIO_ABS = 35,
	D3DSIO_NRM = 36,
	D3DSIO_SINCOS = 37,
	D3DSIO_REP = 38,
	D3DSIO_ENDREP = 39,
	D3DSIO_IF = 40,
	D3DSIO_IFC = 41,
	D3DSIO_ELSE = 42,
	D3DSIO_ENDIF = 43,
	D3DSIO_BREAK = 44,
	D3DSIO_BREAKC = 45,
	D3DSIO_MOVA = 46,
	D3DSIO_DEFB = 47,
	D3DSIO_DEFI = 48,
	D3DSIO_TEXCOORD = 64,
	D3DSIO_TEXKILL = 65,
	D3DSIO_TEX = 66,
	D3DSIO_TEXBEM = 67,
	D3DSIO_TEXBEML = 68,
	D3DSIO_TEXREG2AR = 69,
	D3DSIO_TEXREG2GB = 70,
	D3DSIO_TEXM3x2PAD = 71,
	D3DSIO_TEXM3x2TEX = 72,
	D3DSIO_TEXM3x3PAD = 73,
	D3DSIO_TEXM3x3TEX = 74,
	D3DSIO_RESERVED0 = 75,
	D3DSIO_TEXM3x3SPEC = 76,
	D3DSIO_TEXM3x3VSPEC = 77,
	D3DSIO_EXPP = 78,
	D3DSIO_LOGP = 79,
	D3DSIO_CND = 80,
	D3DSIO_DEF = 81,
	D3DSIO_TEXREG2RGB = 82,
	D3DSIO_TEXDP3TEX = 83,
	D3DSIO_TEXM3x2DEPTH = 84,
	D3DSIO_TEXDP3 = 85,
	D3DSIO_TEXM3x3 = 86,
	D3DSIO_TEXDEPTH = 87,
	D3DSIO_CMP = 88,
	D3DSIO_BEM = 89,
	D3DSIO_DP2ADD = 90,
	D3DSIO_DSX = 91,
	D3DSIO_DSY = 92,
	D3DSIO_TEXLDD = 93,
	D3DSIO_SETP = 94,
	D3DSIO_TEXLDL = 95,
	D3DSIO_BREAKP = 96,
	D3DSIO_PHASE = 0xfffd,
	D3DSIO_COMMENT = 0xfffe,
	D3DSIO_END = 0xffff
} shader_instruction_opcode_type_t;

typedef enum _shader_param_register_type {
	D3DSPR_TEMP = 0,
	D3DSPR_INPUT = 1,
	D3DSPR_CONST = 2,
	D3DSPR_TEXTURE = 3,
	D3DSPR_RASTOUT = 4,
	D3DSPR_ATTROUT = 5,
	D3DSPR_OUTPUT = 6,
	D3DSPR_CONSTINT = 7,
	D3DSPR_COLOROUT = 8,
	D3DSPR_DEPTHOUT = 9,
	D3DSPR_SAMPLER = 10,
	D3DSPR_CONST2 = 11,
	D3DSPR_CONST3 = 12,
	D3DSPR_CONST4 = 13,
	D3DSPR_CONSTBOOL = 14,
	D3DSPR_LOOP = 15,
	D3DSPR_TEMPFLOAT16 = 16,
	D3DSPR_MISCTYPE = 17,
	D3DSPR_LABEL = 18,
	D3DSPR_PREDICATE = 19,
	D3DSPR_LAST = 20
} shader_param_register_type_t;

typedef enum _shader_param_dstmod_type {
	D3DSPD_NONE = 0,
	D3DSPD_SATURATE = 1,
	D3DSPD_PARTIAL_PRECISION = 2,
	D3DSPD_CENTRIOD = 4,
} shader_param_dstmod_type_t;

typedef enum _shader_param_srcmod_type {
	D3DSPS_NONE = 0,
	D3DSPS_NEGATE = 1,
	D3DSPS_BIAS = 2,
	D3DSPS_NEGATE_BIAS = 3,
	D3DSPS_SIGN = 4,
	D3DSPS_NEGATE_SIGN = 5,
	D3DSPS_COMP = 6,
	D3DSPS_X2 = 7,
	D3DSPS_NEGATE_X2 = 8,
	D3DSPS_DZ = 9,
	D3DSPS_DW = 10,
	D3DSPS_ABS = 11,
	D3DSPS_NEGATE_ABS = 12,
	D3DSPS_NOT = 13,
	D3DSPS_LAST = 14
} shader_param_srcmod_type_t;

namespace Moonlight {

class GalliumContext;

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
	virtual void Render (Context     *ctx,
			     MoonSurface *src,
			     double      x,
			     double      y);

	static int ComputeGaussianSamples (double radius,
					   double precision,
					   double *row);

	static void Blur (Context     *ctx,
			  MoonSurface *src,
			  double      radius,
			  double      x,
			  double      y,
			  double      width,
			  double      height);

	static void DropShadow (Context     *ctx,
				MoonSurface *src,
				double      dx,
				double      dy,
				double      radius,
				Color       *color,
				double      x,
				double      y,
				double      width,
				double      height);

protected:
	/* @GeneratePInvoke,ManagedAccess=Protected */
	Effect ();

	virtual ~Effect () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

	static void UpdateFilterValues (double radius,
					double *values,
					int    ***table,
					int    *size);
};

#define MAX_BLUR_RADIUS 20

/* @Namespace=System.Windows.Media.Effects */
class BlurEffect : public Effect {
public:
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
	void Render (Context     *ctx,
		     MoonSurface *src,
		     double      x,
		     double      y);

protected:
	/* @GeneratePInvoke */
	BlurEffect ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

#define MAX_SHADOW_DEPTH 300

/* @Namespace=System.Windows.Media.Effects */
class DropShadowEffect : public Effect {
public:
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
	void Render (Context     *ctx,
		     MoonSurface *src,
		     double      x,
		     double      y);

protected:
	/* @GeneratePInvoke */
	DropShadowEffect ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Effects */
class MOON_API PixelShader : public DependencyObject {
public:
	/* @PropertyType=Uri,DefaultValue=new Uri(),IsConstPropertyType,GenerateAccessors */
	const static int UriSourceProperty;

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	//
	// Property Accessors
	//
	const Uri* GetUriSource ();
	void SetUriSource (const Uri *uri);

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
	/* @GeneratePInvoke */
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

	/* @GeneratePInvoke */
	void UpdateShaderConstant (int reg, double x, double y, double z, double w);

	/* @GeneratePInvoke */
	void UpdateShaderSampler (int reg, int mode, Brush *input);

	//
	// Padding
	//
	Thickness Padding ();

	void Render (Context     *ctx,
		     MoonSurface *src,
		     double      x,
		     double      y);

	static void ShaderError (PixelShader *ps, const char *format, ...);

protected:
	/* @GeneratePInvoke,ManagedAccess=Protected */
	ShaderEffect ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

	Color constant[MAX_CONSTANTS];
	Brush *sampler_input[MAX_SAMPLERS];
	int sampler_mode[MAX_SAMPLERS];
};

};
#endif /* __MOONLIGHT_EFFECT_H__ */
