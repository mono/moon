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

#include "effect.h"

Effect::Effect ()
{
	SetObjectType (Type::EFFECT);
}

BlurEffect::BlurEffect ()
{
	SetObjectType (Type::BLUREFFECT);
}

DropShadowEffect::DropShadowEffect ()
{
	SetObjectType (Type::DROPSHADOWEFFECT);
}

ShaderEffect::ShaderEffect ()
{
	SetObjectType (Type::SHADEREFFECT);
}

PixelShader::PixelShader ()
{
	SetObjectType (Type::PIXELSHADER);
}
