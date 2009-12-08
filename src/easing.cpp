/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <runtime.h>
#include <timemanager.h>
#include <timesource.h>

#include "easing.h"

// base

EasingFunctionBase::EasingFunctionBase ()
{
	SetObjectType (Type::EASINGFUNCTIONBASE);
	easing_function_callback = NULL;
}

EasingFunctionBase::~EasingFunctionBase ()
{
}


double
EasingFunctionBase::Ease (double normalizedTime)
{
	if (easing_function_callback)
		return easing_function_callback (normalizedTime);

	switch (GetEasingMode()) {
	case EasingModeIn:
		return EaseInCore (normalizedTime);
	case EasingModeOut:
		return 1.0 - EaseInCore (1 - normalizedTime);
	case EasingModeInOut:
		return (normalizedTime <= 0.5
			? EaseInCore (normalizedTime * 2) * 0.5
			: 1.0 - EaseInCore ((1 - normalizedTime) * 2) * 0.5);
	default:
		// XXX
		return 0.0;
	}
}

void
EasingFunctionBase::SetEasingFunction (EasingFunction easing_function)
{
	easing_function_callback = easing_function;
}

// Back

BackEase::BackEase ()
{
	SetObjectType (Type::BACKEASE);
}

BackEase::~BackEase ()
{
}

double
BackEase::EaseInCore (double normalizedTime)
{
	double t_cubed = normalizedTime * normalizedTime * normalizedTime;

	return t_cubed - normalizedTime * GetAmplitude () * sin (normalizedTime * M_PI);
}

// Bounce

BounceEase::BounceEase ()
{
	SetObjectType (Type::BOUNCEEASE);
}

BounceEase::~BounceEase ()
{
}

double
BounceEase::EaseInCore (double normalizedTime)
{
	g_warning ("BounceEase::EaseInCore not implemented");
	return EasingFunctionBase::EaseInCore (normalizedTime);
}

// Circle

CircleEase::CircleEase ()
{
	SetObjectType (Type::CIRCLEEASE);
}

CircleEase::~CircleEase ()
{
}

double
CircleEase::EaseInCore (double normalizedTime)
{
	return 1 - sqrt (1 - normalizedTime * normalizedTime);
}

// Cubic

CubicEase::CubicEase ()
{
	SetObjectType (Type::CUBICEASE);
}

CubicEase::~CubicEase ()
{
}

double
CubicEase::EaseInCore (double normalizedTime)
{
	return normalizedTime * normalizedTime * normalizedTime;
}

// Elastic

ElasticEase::ElasticEase ()
{
	SetObjectType (Type::ELASTICEASE);
}

ElasticEase::~ElasticEase ()
{
}

double
ElasticEase::EaseInCore (double normalizedTime)
{
	double period = 1.0 / (GetOscillations () + .25);
	double offset = period / 4;
	double spring = GetSpringiness ();
	double t = normalizedTime - 1;
       
	return normalizedTime * -(pow (2.0, spring * t)) * sin (((t - offset) * M_PI * 2) / period);
}

// Exponential

ExponentialEase::ExponentialEase ()
{
	SetObjectType (Type::EXPONENTIALEASE);
}

ExponentialEase::~ExponentialEase ()
{
}

double
ExponentialEase::EaseInCore (double normalizedTime)
{
	return (exp (GetExponent () * normalizedTime) - 1) /  (exp (GetExponent ()) - 1);
}

// Power

PowerEase::PowerEase ()
{
	SetObjectType (Type::POWEREASE);
}

PowerEase::~PowerEase ()
{
}

double
PowerEase::EaseInCore (double normalizedTime)
{
	return pow (normalizedTime, GetPower ());
}

// Quadratic

QuadraticEase::QuadraticEase ()
{
	SetObjectType (Type::QUADRATICEASE);
}

QuadraticEase::~QuadraticEase ()
{
}

double
QuadraticEase::EaseInCore (double normalizedTime)
{
	return normalizedTime * normalizedTime;
}


// Quartic

QuarticEase::QuarticEase ()
{
	SetObjectType (Type::QUARTICEASE);
}

QuarticEase::~QuarticEase ()
{
}

double
QuarticEase::EaseInCore (double normalizedTime)
{
	return normalizedTime * normalizedTime * normalizedTime * normalizedTime;
}


// Quintic

QuinticEase::QuinticEase ()
{
	SetObjectType (Type::QUINTICEASE);
}

QuinticEase::~QuinticEase ()
{
}

double
QuinticEase::EaseInCore (double normalizedTime)
{
	return normalizedTime * normalizedTime * normalizedTime * normalizedTime * normalizedTime;
}


// Sine

SineEase::SineEase ()
{
	SetObjectType (Type::SINEEASE);
}

SineEase::~SineEase ()
{
}

double
SineEase::EaseInCore (double normalizedTime)
{
	return 1.0 - sin ((1.0 - normalizedTime) * M_PI_2);
}

