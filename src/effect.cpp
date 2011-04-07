/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <cairo.h>
#include <glib.h>
#include <string.h>

#include "effect.h"
#include "eventargs.h"
#include "application.h"
#include "uri.h"

namespace Moonlight {

Effect::Effect ()
{
	SetObjectType (Type::EFFECT);
}

void
Effect::Render (Context     *ctx,
		MoonSurface *src,
		double      x,
		double      y)
{
	g_warning ("Effect::Render has been called. The derived class should have overridden it.");
}

BlurEffect::BlurEffect ()
{
	SetObjectType (Type::BLUREFFECT);
}

Thickness
BlurEffect::Padding ()
{
	double radius = MIN (GetRadius (), MAX_BLUR_RADIUS);
	int    width = (int) ceil (radius);

	return Thickness (width);
}

void
BlurEffect::Render (Context     *ctx,
		    MoonSurface *src,
		    double      x,
		    double      y)
{
	double radius = MIN (GetRadius (), MAX_BLUR_RADIUS);

	ctx->Blur (src, radius, x, y);
}

DropShadowEffect::DropShadowEffect ()
{
	SetObjectType (Type::DROPSHADOWEFFECT);
}

Thickness
DropShadowEffect::Padding ()
{
	double direction = GetDirection () * (M_PI / 180.0);
	double depth = CLAMP (GetShadowDepth (), 0.0, MAX_SHADOW_DEPTH);
	double radius = MIN (GetBlurRadius (), MAX_BLUR_RADIUS);
	int    width = (int) ceil (radius);
	double left;
	double top;
	double right;
	double bottom;

	left   = -cos (direction) * depth + width;
	top    =  sin (direction) * depth + width;
	right  =  cos (direction) * depth + width;
	bottom = -sin (direction) * depth + width;

	return Thickness (left   < 1.0 ? 1.0 : ceil (left),
			  top    < 1.0 ? 1.0 : ceil (top),
			  right  < 1.0 ? 1.0 : ceil (right),
			  bottom < 1.0 ? 1.0 : ceil (bottom));
}

void
DropShadowEffect::Render (Context     *ctx,
			  MoonSurface *src,
			  double      x,
			  double      y)
{
	double radius = MIN (GetBlurRadius (), MAX_BLUR_RADIUS);
	double direction = GetDirection () * (M_PI / 180.0);
 	double depth = CLAMP (GetShadowDepth (), 0.0, MAX_SHADOW_DEPTH);
	double dx = -cos (direction) * depth;
	double dy = sin (direction) * depth;
	double opacity = CLAMP (GetOpacity (), 0.0, 1.0);
	Color  *color = GetColor ();
	Color  rgba = Color (color->r * opacity,
			     color->g * opacity,
			     color->b * opacity,
			     opacity);

	ctx->DropShadow (src, dx, dy, radius, &rgba, x, y);
}

PixelShader::PixelShader ()
{
	SetObjectType (Type::PIXELSHADER);

	tokens = NULL;
}

PixelShader::~PixelShader ()
{
	g_free (tokens);
}

void
PixelShader::OnPropertyChanged (PropertyChangedEventArgs *args,
				MoonError                *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::PIXELSHADER) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == PixelShader::UriSourceProperty) {
		Application *application = Application::GetCurrent ();
		const Uri *uri = GetUriSource ();
		char *path;

		g_free (tokens);
		tokens = NULL;

		if (!Uri::IsNullOrEmpty (uri) && application &&
		    (path = application->GetResourceAsPath (GetResourceBase (),
							    uri))) {
			SetTokensFromPath (path);
			g_free (path);
		}
		else if (application) {
			g_warning ("invalid uri: %s", uri ? uri->ToString () :
				   "null");
		}
	}

	NotifyListenersOfPropertyChange (args, error);
}

void
PixelShader::SetTokensFromPath (const char *path)
{
	GError *error = NULL;
	gchar  *bytes;
	gsize  nbytes;

	if (!g_file_get_contents (path,
				  (char **) &bytes,
				  &nbytes,
				  &error)) {
		g_warning ("%s", error->message);
		g_error_free (error);
		return;
	}

	g_free (tokens);
	tokens = (guint32 *) bytes;
	ntokens = nbytes / sizeof (guint32);
}

int
PixelShader::GetToken (int     index,
		       guint32 *token)
{
	if (!tokens || index < 0 || index >= (int) ntokens) {
		if (index >= 0)
			g_warning ("incomplete pixel shader");

		return -1;
	}

	if (token)
		*token = *(tokens + index);

	return index + 1;
}

int
PixelShader::GetToken (int   index,
		       float *token)
{
	return GetToken (index, (guint32 *) token);
}

/* major version */
#define D3D_VERSION_MAJOR_SHIFT 8
#define D3D_VERSION_MAJOR_MASK  0xff

/* minor version */
#define D3D_VERSION_MINOR_SHIFT 0
#define D3D_VERSION_MINOR_MASK  0xff

/* shader type */
#define D3D_VERSION_TYPE_SHIFT 16
#define D3D_VERSION_TYPE_MASK  0xffff

int
PixelShader::GetVersion (int	       index,
			 d3d_version_t *value)
{
	guint32 token;

	if ((index = GetToken (index, &token)) < 0)
		return -1;

	value->major = (token >> D3D_VERSION_MAJOR_SHIFT) &
		D3D_VERSION_MAJOR_MASK;
	value->minor = (token >> D3D_VERSION_MINOR_SHIFT) &
		D3D_VERSION_MINOR_MASK;
	value->type  = (token >> D3D_VERSION_TYPE_SHIFT) &
		D3D_VERSION_TYPE_MASK;

	return index;
}

/* instruction type */
#define D3D_OP_TYPE_SHIFT 0
#define D3D_OP_TYPE_MASK  0xffff

/* instruction length */
#define D3D_OP_LENGTH_SHIFT 24
#define D3D_OP_LENGTH_MASK  0xf

/* comment length */
#define D3D_OP_COMMENT_LENGTH_SHIFT 16
#define D3D_OP_COMMENT_LENGTH_MASK  0xffff

int
PixelShader::GetOp (int      index,
		    d3d_op_t *value)
{
	const d3d_op_metadata_t metadata[] = {
		{ "NOP", 0, 0 }, /* D3DSIO_NOP 0 */
		{ "MOV", 1, 1 }, /* D3DSIO_MOV 1 */
		{ "ADD", 1, 2 }, /* D3DSIO_ADD 2 */
		{ "SUB", 1, 2 }, /* D3DSIO_SUB 3 */
		{ "MAD", 1, 3 }, /* D3DSIO_MAD 4 */
		{ "MUL", 1, 2 }, /* D3DSIO_MUL 5 */
		{ "RCP", 1, 1 }, /* D3DSIO_RCP 6 */
		{ "RSQ", 1, 1 }, /* D3DSIO_RSQ 7 */
		{ "DP3", 1, 2 }, /* D3DSIO_DP3 8 */
		{ "DP4", 1, 2 }, /* D3DSIO_DP4 9 */
		{ "MIN", 1, 2 }, /* D3DSIO_MIN 10 */
		{ "MAX", 1, 2 }, /* D3DSIO_MAX 11 */
		{ "SLT", 1, 2 }, /* D3DSIO_SLT 12 */
		{ "SGE", 1, 2 }, /* D3DSIO_SGE 13 */
		{ "EXP", 1, 1 }, /* D3DSIO_EXP 14 */
		{ "LOG", 1, 1 }, /* D3DSIO_LOG 15 */
		{ "LIT", 1, 1 }, /* D3DSIO_LIT 16 */
		{ "DST", 1, 2 }, /* D3DSIO_DST 17 */
		{ "LRP", 1, 3 }, /* D3DSIO_LRP 18 */
		{ "FRC", 1, 1 }, /* D3DSIO_FRC 19 */
		{  NULL, 0, 0 }, /* D3DSIO_M4x4 20 */
		{  NULL, 0, 0 }, /* D3DSIO_M4x3 21 */
		{  NULL, 0, 0 }, /* D3DSIO_M3x4 22 */
		{  NULL, 0, 0 }, /* D3DSIO_M3x3 23 */
		{  NULL, 0, 0 }, /* D3DSIO_M3x2 24 */
		{  NULL, 0, 0 }, /* D3DSIO_CALL 25 */
		{  NULL, 0, 0 }, /* D3DSIO_CALLNZ 26 */
		{  NULL, 0, 0 }, /* D3DSIO_LOOP 27 */
		{  NULL, 0, 0 }, /* D3DSIO_RET 28 */
		{  NULL, 0, 0 }, /* D3DSIO_ENDLOOP 29 */
		{  NULL, 0, 0 }, /* D3DSIO_LABEL 30 */
		{ "DCL", 0, 0 }, /* D3DSIO_DCL 31 */
		{ "POW", 1, 2 }, /* D3DSIO_POW 32 */
		{  NULL, 0, 0 }, /* D3DSIO_CRS 33 */
		{  NULL, 0, 0 }, /* D3DSIO_SGN 34 */
		{ "ABS", 1, 1 }, /* D3DSIO_ABS 35 */
		{ "NRM", 1, 1 }, /* D3DSIO_NRM 36 */
		{ "SIN", 1, 3 }, /* D3DSIO_SINCOS 37 */
		{  NULL, 0, 0 }, /* D3DSIO_REP 38 */
		{  NULL, 0, 0 }, /* D3DSIO_ENDREP 39 */
		{  NULL, 0, 0 }, /* D3DSIO_IF 40 */
		{  NULL, 0, 0 }, /* D3DSIO_IFC 41 */
		{  NULL, 0, 0 }, /* D3DSIO_ELSE 42 */
		{  NULL, 0, 0 }, /* D3DSIO_ENDIF 43 */
		{  NULL, 0, 0 }, /* D3DSIO_BREAK 44 */
		{  NULL, 0, 0 }, /* D3DSIO_BREAKC 45 */
		{  NULL, 0, 0 }, /* D3DSIO_MOVA 46 */
		{  NULL, 0, 0 }, /* D3DSIO_DEFB 47 */
		{  NULL, 0, 0 }, /* D3DSIO_DEFI 48 */
		{  NULL, 0, 0 }, /* 49 */
		{  NULL, 0, 0 }, /* 50 */
		{  NULL, 0, 0 }, /* 51 */
		{  NULL, 0, 0 }, /* 52 */
		{  NULL, 0, 0 }, /* 53 */
		{  NULL, 0, 0 }, /* 54 */
		{  NULL, 0, 0 }, /* 55 */
		{  NULL, 0, 0 }, /* 56 */
		{  NULL, 0, 0 }, /* 57 */
		{  NULL, 0, 0 }, /* 58 */
		{  NULL, 0, 0 }, /* 59 */
		{  NULL, 0, 0 }, /* 60 */
		{  NULL, 0, 0 }, /* 61 */
		{  NULL, 0, 0 }, /* 62 */
		{  NULL, 0, 0 }, /* 63 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXCOORD 64 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXKILL 65 */
		{ "TEX", 1, 2 }, /* D3DSIO_TEX 66 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXBEM 67 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXBEML 68 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXREG2AR 69 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXREG2GB 70 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x2PAD 71 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x2TEX 72 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x3PAD 73 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x3TEX 74 */
		{  NULL, 0, 0 }, /* D3DSIO_RESERVED0 75 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x3SPEC 76 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x3VSPEC 77 */
		{  NULL, 0, 0 }, /* D3DSIO_EXPP 78 */
		{  NULL, 0, 0 }, /* D3DSIO_LOGP 79 */
		{ "CND", 1, 3 }, /* D3DSIO_CND 80 */
		{ "DEF", 0, 0 }, /* D3DSIO_DEF 81 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXREG2RGB 82 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXDP3TEX 83 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x2DEPTH 84 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXDP3 85 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x3 86 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXDEPTH 87 */
		{ "CMP", 1, 3 }, /* D3DSIO_CMP 88 */
		{  NULL, 0, 0 }, /* D3DSIO_BEM 89 */
		{ "D2A", 1, 3 }, /* D3DSIO_DP2ADD 90 */
		{  NULL, 0, 0 }, /* D3DSIO_DSX 91 */
		{  NULL, 0, 0 }, /* D3DSIO_DSY 92 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXLDD 93 */
		{  NULL, 0, 0 }, /* D3DSIO_SETP 94 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXLDL 95 */
		{  NULL, 0, 0 }, /* D3DSIO_BREAKP 96 */
		{  NULL, 0, 0 }  /* 97 */
	};
	guint32 token;

	if ((index = GetToken (index, &token)) < 0)
		return -1;

	value->type = (token >> D3D_OP_TYPE_SHIFT) & D3D_OP_TYPE_MASK;
	value->length = (token >> D3D_OP_LENGTH_SHIFT) & D3D_OP_LENGTH_MASK;
	value->comment_length = (token >> D3D_OP_COMMENT_LENGTH_SHIFT) &
		D3D_OP_COMMENT_LENGTH_MASK;

	if (value->type < G_N_ELEMENTS (metadata))
		value->meta = metadata[value->type];
	else
		value->meta = metadata[G_N_ELEMENTS (metadata) - 1];

	return index;
}

/* register number */
#define D3D_DP_REGNUM_MASK 0x7ff

/* register type */
#define D3D_DP_REGTYPE_SHIFT1 28
#define D3D_DP_REGTYPE_MASK1  0x7
#define D3D_DP_REGTYPE_SHIFT2 8
#define D3D_DP_REGTYPE_MASK2  0x18

/* write mask */
#define D3D_DP_WRITEMASK_SHIFT 16
#define D3D_DP_WRITEMASK_MASK  0xf

/* destination modifier */
#define D3D_DP_DSTMOD_SHIFT 20
#define D3D_DP_DSTMOD_MASK  0x7

int
PixelShader::GetDestinationParameter (int                         index,
				      d3d_destination_parameter_t *value)
{
	guint32 token;

	if ((index = GetToken (index, &token)) < 0)
		return -1;

	if (!value)
		return index;

	value->regnum = token & D3D_DP_REGNUM_MASK;
	value->regtype =
		((token >> D3D_DP_REGTYPE_SHIFT1) & D3D_DP_REGTYPE_MASK1) |
		((token >> D3D_DP_REGTYPE_SHIFT2) & D3D_DP_REGTYPE_MASK2);
	value->writemask = (token >> D3D_DP_WRITEMASK_SHIFT) &
		D3D_DP_WRITEMASK_MASK;
	value->dstmod = (token >> D3D_DP_DSTMOD_SHIFT) & D3D_DP_DSTMOD_MASK;

	return index;
}

/* register number */
#define D3D_SP_REGNUM_MASK 0x7ff

/* register type */
#define D3D_SP_REGTYPE_SHIFT1 28
#define D3D_SP_REGTYPE_MASK1  0x7
#define D3D_SP_REGTYPE_SHIFT2 8
#define D3D_SP_REGTYPE_MASK2  0x18

/* swizzle */
#define D3D_SP_SWIZZLE_X_SHIFT 16
#define D3D_SP_SWIZZLE_X_MASK  0x3
#define D3D_SP_SWIZZLE_Y_SHIFT 18
#define D3D_SP_SWIZZLE_Y_MASK  0x3
#define D3D_SP_SWIZZLE_Z_SHIFT 20
#define D3D_SP_SWIZZLE_Z_MASK  0x3
#define D3D_SP_SWIZZLE_W_SHIFT 22
#define D3D_SP_SWIZZLE_W_MASK  0x3

/* source modifier */
#define D3D_SP_SRCMOD_SHIFT 24
#define D3D_SP_SRCMOD_MASK  0x7

int
PixelShader::GetSourceParameter (int                    index,
				 d3d_source_parameter_t *value)
{
	guint32 token;

	if ((index = GetToken (index, &token)) < 0)
		return -1;

	if (!value)
		return index;

	value->regnum = token & D3D_SP_REGNUM_MASK;
	value->regtype =
		((token >> D3D_SP_REGTYPE_SHIFT1) & D3D_SP_REGTYPE_MASK1) |
		((token >> D3D_SP_REGTYPE_SHIFT2) & D3D_SP_REGTYPE_MASK2);
	value->swizzle.x = (token >> D3D_SP_SWIZZLE_X_SHIFT) &
		D3D_SP_SWIZZLE_X_MASK;
	value->swizzle.y = (token >> D3D_SP_SWIZZLE_Y_SHIFT) &
		D3D_SP_SWIZZLE_Y_MASK;
	value->swizzle.z = (token >> D3D_SP_SWIZZLE_Z_SHIFT) &
		D3D_SP_SWIZZLE_Z_MASK;
	value->swizzle.w = (token >> D3D_SP_SWIZZLE_W_SHIFT) &
		D3D_SP_SWIZZLE_W_MASK;
	value->srcmod = (token >> D3D_SP_SRCMOD_SHIFT) & D3D_SP_SRCMOD_MASK;

	return index;
}

int
PixelShader::GetInstruction (int                   index,
			     d3d_def_instruction_t *value)
{
	index = GetDestinationParameter (index, &value->reg);
	index = GetToken (index, &value->v[0]);
	index = GetToken (index, &value->v[1]);
	index = GetToken (index, &value->v[2]);
	index = GetToken (index, &value->v[3]);

	return index;
}

/* DCL usage */
#define D3D_DCL_USAGE_SHIFT 0
#define D3D_DCL_USAGE_MASK  0xf

/* DCL usage index */
#define D3D_DCL_USAGEINDEX_SHIFT 16
#define D3D_DCL_USAGEINDEX_MASK  0xf

int
PixelShader::GetInstruction (int                   index,
			     d3d_dcl_instruction_t *value)
{
	guint32 token;

	if ((index = GetToken (index, &token)) < 0)
		return -1;

	index = GetDestinationParameter (index, &value->reg);

	if (!value)
		return index;

	value->usage = (token >> D3D_DCL_USAGE_SHIFT) & D3D_DCL_USAGE_MASK;
	value->usageindex = (token >> D3D_DCL_USAGEINDEX_SHIFT) &
		D3D_DCL_USAGEINDEX_MASK;

	return index;
}

ShaderEffect::ShaderEffect ()
{
	int i;

	SetObjectType (Type::SHADEREFFECT);

	for (i = 0; i < MAX_SAMPLERS; i++) {
		sampler_input[i] = NULL;
		sampler_mode[i] = 0;
	}

	for (i = 0; i < MAX_CONSTANTS; i++)
		constant[i] = Color ();
}

Thickness
ShaderEffect::Padding ()
{
	Value *left   = GetValue (ShaderEffect::PaddingLeftProperty);
	Value *top    = GetValue (ShaderEffect::PaddingTopProperty);
	Value *right  = GetValue (ShaderEffect::PaddingRightProperty);
	Value *bottom = GetValue (ShaderEffect::PaddingBottomProperty);

	return Thickness (left   ? ceil (left->AsDouble ())   : 0.0,
			  top    ? ceil (top->AsDouble ())    : 0.0,
			  right  ? ceil (right->AsDouble ())  : 0.0,
			  bottom ? ceil (bottom->AsDouble ()) : 0.0);
}

void
ShaderEffect::UpdateShaderConstant (int reg, double x, double y, double z, double w)
{
	if (reg >= MAX_CONSTANTS) {
		g_warning ("UpdateShaderConstant: invalid register number %d",
			   reg);
		return;
	}

	constant[reg].r = x;
	constant[reg].g = y;
	constant[reg].b = z;
	constant[reg].a = w;
}

void
ShaderEffect::UpdateShaderSampler (int reg, int mode, Brush *input)
{
	if (reg >= MAX_SAMPLERS) {
		g_warning ("UpdateShaderSampler: invalid register number %d",
			   reg);
		return;
	}

	sampler_input[reg] = input;
	sampler_mode[reg]  = mode;
}

void
ShaderEffect::Render (Context     *ctx,
		      MoonSurface *src,
		      double      x,
		      double      y)
{
	Value *ddxDdyReg;

	ddxDdyReg = GetValue (ShaderEffect::DdxUvDdyUvRegisterIndexProperty);
	if (ddxDdyReg) {
		int ddxDdy = ddxDdyReg->AsInt32 ();

		ctx->ShaderEffect (src,
				   GetPixelShader (),
				   sampler_input,
				   sampler_mode,
				   MAX_SAMPLERS,
				   constant,
				   MAX_CONSTANTS,
				   &ddxDdy,
				   x, y);
	}
	else {
		ctx->ShaderEffect (src,
				   GetPixelShader (),
				   sampler_input,
				   sampler_mode,
				   MAX_SAMPLERS,
				   constant,
				   MAX_CONSTANTS,
				   NULL,
				   x, y);
	}
}

static inline void
d3d_print_regtype (unsigned int type)
{
	const char *type_str[] = {
		"TEMP",
		"INPUT",
		"CONST",
		"TEX",
		"ROUT",
		"AOUT",
		"OUT",
		"CTINT",
		"COUT",
		"DOUT",
		"SAMP",
		"CONS2",
		"CONS3",
		"CONS4",
		"CBOOL",
		"LOOP",
		"TF16",
		"MISC",
		"LABEL",
		"PRED"
	};

	if (type >= G_N_ELEMENTS (type_str))
		printf ("0x%x", type);

	printf ("%s", type_str[type]);
}

static void
d3d_print_srcmod (unsigned int mod)
{
	const char *srcmod_str[] = {
		"",
		"-",
		"bias ",
		"-bias ",
		"sign ",
		"-sign ",
		"comp ",
		"pow ",
		"npow ",
		"dz ",
		"dw ",
		"abs ",
		"-abs ",
		"not "
	};

	if (mod >= G_N_ELEMENTS (srcmod_str))
		printf ("0x%x ", (int) mod);
	else
		printf ("%s", srcmod_str[mod]);
}

static void
d3d_print_src_param (d3d_source_parameter_t *src)
{
	const char *swizzle_str[] = { "x", "y", "z", "w" };

	d3d_print_srcmod (src->srcmod);
	d3d_print_regtype (src->regtype);
	printf ("[%d]", src->regnum);
	if (src->swizzle.x != 0 ||
	    src->swizzle.y != 1 ||
	    src->swizzle.z != 2 ||
	    src->swizzle.w != 3)
		printf (".%s%s%s%s",
			swizzle_str[src->swizzle.x],
			swizzle_str[src->swizzle.y],
			swizzle_str[src->swizzle.z],
			swizzle_str[src->swizzle.w]);
}

static void
d3d_print_dstmod (unsigned int mod)
{
	const char *dstmod_str[] = {
		"",
		"_SAT",
		"_PRT",
		"_CNT"
	};

	if (mod >= G_N_ELEMENTS (dstmod_str))
		printf ("_0x%x ", mod);
	else
		printf ("%s ", dstmod_str[mod]);
}

static void
d3d_print_dst_param (d3d_destination_parameter_t *dst)
{
	d3d_print_dstmod (dst->dstmod);
	d3d_print_regtype (dst->regtype);
	printf ("[%d]", dst->regnum);
	if (dst->writemask != 0xf)
		printf (".%s%s%s%s",
			dst->writemask & 0x1 ? "x" : "",
			dst->writemask & 0x2 ? "y" : "",
			dst->writemask & 0x4 ? "z" : "",
			dst->writemask & 0x8 ? "w" : "");
}

void
ShaderEffect::ShaderError (PixelShader *ps, const char *format, ...)
{
	d3d_version_t version;
	d3d_op_t      op;
	int           i;
	int           n = 0;

	if (format) {
		va_list ap;

		printf ("Moonlight: ");
		va_start (ap, format);
		vprintf (format, ap);
		va_end (ap);
		printf (":\n");
	}

	if (!ps)
		return;

	if ((i = ps->GetVersion (0, &version)) < 0)
		return;

	if (version.type != 0xffff) {
		printf ("0x%x %d.%d\n", version.type, version.major,
			version.minor);
		return;
	}
	else if (version.major < 2) {
		printf ("PS %d.%d\n", version.major, version.minor);
		return;
	}

	printf ("PS %d.%d\n", version.major, version.minor);

	while ((i = ps->GetOp (i, &op)) > 0) {
		d3d_destination_parameter_t reg;
		d3d_source_parameter_t      src;

		if (op.type == D3DSIO_COMMENT) {
			i += op.comment_length;
			continue;
		}

		switch (op.type) {
			case D3DSIO_DEF: {
				d3d_def_instruction_t def;

				if (ps->GetInstruction (i, &def) != 1) {
					printf ("%s ", op.meta.name);
					d3d_print_dst_param (&def.reg);
					printf (" { %10.4f, %10.4f, %10.4f, %10.4f }\n",
						def.v[0], def.v[1], def.v[2],
						def.v[3]);
				}
			} break;
			case D3DSIO_DCL: {
				d3d_dcl_instruction_t dcl;

				if (ps->GetInstruction (i, &dcl) != -1) {
					printf ("%s", op.meta.name);
					d3d_print_dst_param (&dcl.reg);
					printf ("\n");
				}
			} break;
			case D3DSIO_END:
				printf ("%3d: END\n", n + 1);
				return;
			default: {
				unsigned ndstparam = op.meta.ndstparam;
				unsigned nsrcparam = op.meta.nsrcparam;
				int      j = i;

				n++;

				if (op.meta.name)
					printf ("%3d: %s", n, op.meta.name);
				else
					printf ("%3d: %d", n, op.type);

				while (ndstparam--) {
					j = ps->GetDestinationParameter (j, &reg);
					d3d_print_dst_param (&reg);
				}

				if (nsrcparam--) {
					printf (", ");
					j = ps->GetSourceParameter (j, &src);
					d3d_print_src_param (&src);
				}

				while (nsrcparam--) {
					j = ps->GetSourceParameter (j, &src);
					printf (", ");
					d3d_print_src_param (&src);
				}

				printf ("\n");
			} break;
		}

		i += op.length;
	}
}

};
