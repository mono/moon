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

namespace Moonlight {

/* @CBindingRequisite */
typedef double (*EasingFunction) (double normalizedTime);

/* @Namespace=System.Windows.Media.Animation */
/* @CallInitialize */
class EasingFunctionBase : public DependencyObject {
public:
	/* @PropertyType=EasingMode,DefaultValue=EasingModeOut,GenerateAccessors */
	const static int EasingModeProperty;

	double Ease (double normalizedTime);
	
	/* @GenerateCBinding,GeneratePInvoke */
	virtual double EaseInCore (double normalizedTime) { return normalizedTime; }

	EasingMode GetEasingMode ();
	void SetEasingMode (EasingMode easingMode);

	/* @GenerateCBinding,GeneratePInvoke */
	void SetEasingFunction (EasingFunction value);

protected:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	EasingFunctionBase ();

	virtual ~EasingFunctionBase ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	EasingFunction easing_function_callback;
};

/* @Namespace=System.Windows.Media.Animation */
class BackEase : public EasingFunctionBase
{
public:
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int AmplitudeProperty;
	
	virtual double EaseInCore (double normalizedTime);

	double GetAmplitude ();
	void SetAmplitude (double value);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	BackEase ();

	virtual ~BackEase();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class BounceEase : public EasingFunctionBase
{
public:
	/* @PropertyType=gint32,DefaultValue=3,GenerateAccessors */
	const static int BouncesProperty;
	/* @PropertyType=double,DefaultValue=2.0,GenerateAccessors */
	const static int BouncinessProperty;
	
	virtual double EaseInCore (double normalizedTime);

	int GetBounces ();
	void SetBounces (int value);

	double GetBounciness ();
	void SetBounciness (double value);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	BounceEase ();

	virtual ~BounceEase();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class CircleEase : public EasingFunctionBase
{
public:
	virtual double EaseInCore (double normalizedTime);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	CircleEase ();
	
	virtual ~CircleEase ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class CubicEase : public EasingFunctionBase
{
public:
	virtual double EaseInCore (double normalizedTime);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	CubicEase ();
	
	virtual ~CubicEase ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class ElasticEase : public EasingFunctionBase
{
public:
	/* @PropertyType=gint32,DefaultValue=3,GenerateAccessors */
	const static int OscillationsProperty;
	/* @PropertyType=double,DefaultValue=3.0,GenerateAccessors */
	const static int SpringinessProperty;
	
	virtual double EaseInCore (double normalizedTime);

	int GetOscillations ();
	void SetOscillations (int value);

	double GetSpringiness ();
	void SetSpringiness (double value);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	ElasticEase ();

	virtual ~ElasticEase();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class ExponentialEase : public EasingFunctionBase
{
public:
	/* @PropertyType=double,DefaultValue=2.0,GenerateAccessors */
	const static int ExponentProperty;
	
	virtual double EaseInCore (double normalizedTime);

	double GetExponent ();
	void SetExponent (double value);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	ExponentialEase ();

	virtual ~ExponentialEase();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class PowerEase : public EasingFunctionBase
{
public:
	/* @PropertyType=double,DefaultValue=2.0,GenerateAccessors */
	const static int PowerProperty;
	
	virtual double EaseInCore (double normalizedTime);

	double GetPower ();
	void SetPower (double value);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	PowerEase ();

	virtual ~PowerEase();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class QuadraticEase : public EasingFunctionBase
{
public:
	virtual double EaseInCore (double normalizedTime);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	QuadraticEase ();
	
	virtual ~QuadraticEase ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class QuarticEase : public EasingFunctionBase
{
public:
	virtual double EaseInCore (double normalizedTime);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	QuarticEase ();
	
	virtual ~QuarticEase ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class QuinticEase : public EasingFunctionBase
{
public:
	virtual double EaseInCore (double normalizedTime);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	QuinticEase ();
	
	virtual ~QuinticEase ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class SineEase : public EasingFunctionBase
{
public:
	virtual double EaseInCore (double normalizedTime);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	SineEase ();
	
	virtual ~SineEase ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

};
#endif /* __MOON_EASING_H__ */
