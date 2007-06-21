/*
 * stylys.cpp
 *
 * Author:
 *   Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <math.h>
#include "stylus.h"


StylusInfo*
stylus_info_new ()
{
	return new StylusInfo ();
}

StylusPoint*
stylus_point_new ()
{
	return new StylusPoint ();
}

DependencyProperty* StylusInfo::DeviceTypeProperty;
DependencyProperty* StylusInfo::IsInvertedProperty;

DependencyProperty* StylusPoint::PressureFactorProperty;
DependencyProperty* StylusPoint::XProperty;
DependencyProperty* StylusPoint::YProperty;

void stylus_init ()
{
	StylusInfo::DeviceTypeProperty = DependencyObject::Register (Type::STYLUSINFO, "DeviceType", Type::INT32);
	StylusInfo::IsInvertedProperty = DependencyObject::Register (Type::STYLUSINFO, "IsInverted", Type::BOOL);

	StylusPoint::PressureFactorProperty = DependencyObject::Register (Type::STYLUSPOINT, "PressureFactor", Type::DOUBLE);
	StylusPoint::XProperty = DependencyObject::Register (Type::STYLUSPOINT, "X", Type::DOUBLE);
	StylusPoint::YProperty = DependencyObject::Register (Type::STYLUSPOINT, "Y", Type::DOUBLE);
}

