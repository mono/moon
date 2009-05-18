/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_EASING_H__
#define __MOON_EASING_H__

#include "dependencyobject.h"
#include "enums.h"

/* @CBindingRequisite */
typedef double (*EasingFunction) (double normalizedTime);

/* @Namespace=System.Windows.Media.Animation */
/* @CallInitialize */
class EasingFunctionBase : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	EasingFunctionBase ();

	/* @PropertyType=EasingMode,DefaultValue=EasingModeOut,GenerateAccessors */
	const static int EasingModeProperty;

	double Ease (double normalizedTime);
	virtual double EaseInCore (double normalizedTime) { return normalizedTime; }

	EasingMode GetEasingMode ();
	void SetEasingMode (EasingMode easingMode);

	/* @GenerateCBinding,GeneratePInvoke */
	void SetEasingFunction (EasingFunction value);

protected:
	virtual ~EasingFunctionBase ();

private:
	EasingFunction easing_function_callback;
};

/* @Namespace=System.Windows.Media.Animation */
class BackEase : public EasingFunctionBase
{
public:
	/* @GenerateCBinding,GeneratePInvoke */
	BackEase ();

	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int AmplitudeProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime);

	double GetAmplitude ();
	void SetAmplitude (double value);

protected:
	virtual ~BackEase();
};

/* @Namespace=System.Windows.Media.Animation */
class BounceEase : public EasingFunctionBase
{
public:
	/* @GenerateCBinding,GeneratePInvoke */
	BounceEase ();

	/* @PropertyType=gint32,DefaultValue=3,GenerateAccessors */
	const static int BouncesProperty;
	/* @PropertyType=double,DefaultValue=2.0,GenerateAccessors */
	const static int BouncinessProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime);

	int GetBounces ();
	void SetBounces (int value);

	double GetBounciness ();
	void SetBounciness (double value);

protected:
	virtual ~BounceEase();
};

/* @Namespace=System.Windows.Media.Animation */
class CircleEase : public EasingFunctionBase
{
public:
	/* @GenerateCBinding,GeneratePInvoke */
	CircleEase ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime);

protected:
	virtual ~CircleEase ();
};

/* @Namespace=System.Windows.Media.Animation */
class CubicEase : public EasingFunctionBase
{
public:
	/* @GenerateCBinding,GeneratePInvoke */
	CubicEase ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime);

protected:
	virtual ~CubicEase ();
};

/* @Namespace=System.Windows.Media.Animation */
class ElasticEase : public EasingFunctionBase
{
public:
	/* @GenerateCBinding,GeneratePInvoke */
	ElasticEase ();

	/* @PropertyType=gint32,DefaultValue=3,GenerateAccessors */
	const static int OscillationsProperty;
	/* @PropertyType=double,DefaultValue=3.0,GenerateAccessors */
	const static int SpringinessProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime);

	int GetOscillations ();
	void SetOscillations (int value);

	double GetSpringiness ();
	void SetSpringiness (double value);

protected:
	virtual ~ElasticEase();
};

/* @Namespace=System.Windows.Media.Animation */
class ExponentialEase : public EasingFunctionBase
{
public:
	/* @GenerateCBinding,GeneratePInvoke */
	ExponentialEase ();

	/* @PropertyType=double,DefaultValue=2.0,GenerateAccessors */
	const static int ExponentProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime);

	double GetExponent ();
	void SetExponent (double value);

protected:
	virtual ~ExponentialEase();
};

/* @Namespace=System.Windows.Media.Animation */
class PowerEase : public EasingFunctionBase
{
public:
	/* @GenerateCBinding,GeneratePInvoke */
	PowerEase ();

	/* @PropertyType=double,DefaultValue=2.0,GenerateAccessors */
	const static int PowerProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime);

	double GetPower ();
	void SetPower (double value);

protected:
	virtual ~PowerEase();
};

/* @Namespace=System.Windows.Media.Animation */
class QuadraticEase : public EasingFunctionBase
{
public:
	/* @GenerateCBinding,GeneratePInvoke */
	QuadraticEase ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime);

protected:
	virtual ~QuadraticEase ();
};

/* @Namespace=System.Windows.Media.Animation */
class QuarticEase : public EasingFunctionBase
{
public:
	/* @GenerateCBinding,GeneratePInvoke */
	QuarticEase ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime);

protected:
	virtual ~QuarticEase ();
};

/* @Namespace=System.Windows.Media.Animation */
class QuinticEase : public EasingFunctionBase
{
public:
	/* @GenerateCBinding,GeneratePInvoke */
	QuinticEase ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime);

protected:
	virtual ~QuinticEase ();
};

/* @Namespace=System.Windows.Media.Animation */
class SineEase : public EasingFunctionBase
{
public:
	/* @GenerateCBinding,GeneratePInvoke */
	SineEase ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime);

protected:
	virtual ~SineEase ();
};

#endif /* __MOON_EASING_H__ */
