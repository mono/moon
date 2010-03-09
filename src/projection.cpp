/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * projection.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <math.h>
#include <string.h>

#include "projection.h"
#include "eventargs.h"

//
// Matrix3D
//

Matrix3D::Matrix3D ()
{
	SetObjectType (Type::MATRIX3D);

	// initialize the matrix as the identity
	memset (matrix, 0, sizeof (double) * 16);
	matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0;
}

Matrix3D::Matrix3D (double *m)
{
	SetObjectType (Type::MATRIX3D);

	memcpy (matrix, m, sizeof (double) * 16);
	SetM11 (m[0]);
	SetM12 (m[1]);
	SetM13 (m[2]);
	SetM14 (m[3]);
	SetM21 (m[4]);
	SetM22 (m[5]);
	SetM23 (m[6]);
	SetM24 (m[7]);
	SetM31 (m[8]);
	SetM32 (m[9]);
	SetM33 (m[10]);
	SetM34 (m[11]);
	SetOffsetX (m[12]);
	SetOffsetY (m[13]);
	SetOffsetZ (m[14]);
	SetM44 (m[15]);
}

void
Matrix3D::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::MATRIX3D) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == Matrix3D::M11Property)
		matrix[0] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M12Property)
		matrix[1] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M13Property)
		matrix[2] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M14Property)
		matrix[3] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M21Property)
		matrix[4] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M22Property)
		matrix[5] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M23Property)
		matrix[6] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M24Property)
		matrix[7] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M31Property)
		matrix[8] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M32Property)
		matrix[9] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M33Property)
		matrix[10] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M34Property)
		matrix[11] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::OffsetXProperty)
		matrix[12] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::OffsetYProperty)
		matrix[13] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::OffsetZProperty)
		matrix[14] = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix3D::M44Property)
		matrix[15] = args->GetNewValue()->AsDouble ();

	NotifyListenersOfPropertyChange (args, error);
}

void
Matrix3D::TransformPoint (double *out, const double *m, const double *in)
{
	double tmp[4];

#define M(row, col) m[col * 4 + row]
	tmp[0] = M (0, 0) * in[0] + M (0, 1) * in[1] + M (0, 2) * in[2] + M (0, 3) * in[3];
	tmp[1] = M (1, 0) * in[0] + M (1, 1) * in[1] + M (1, 2) * in[2] + M (1, 3) * in[3];
	tmp[2] = M (2, 0) * in[0] + M (2, 1) * in[1] + M (2, 2) * in[2] + M (2, 3) * in[3];
	tmp[3] = M (3, 0) * in[0] + M (3, 1) * in[1] + M (3, 2) * in[2] + M (3, 3) * in[3];
#undef M

	memcpy (out, tmp, 4 * sizeof (double));
}

Projection::Projection ()
{
	SetObjectType (Type::PROJECTION);
	need_update = false;
}

Matrix3D *
Projection::GetProjectionMatrix ()
{
	g_warning ("Projection::GetProjectionMatrix has been called. "
		   "The derived class should have overridden it.");
	return NULL;
}

void
Projection::UpdateProjection ()
{
	g_warning ("Projection::UpdateProjection has been called. "
		   "The derived class should have overridden it.");
}

void
Projection::MaybeUpdateProjection ()
{
	if (need_update) {
		UpdateProjection ();
		need_update = false;
	}
}

Matrix3D *
Projection::GetMatrix3D ()
{
	MaybeUpdateProjection ();
	return GetProjectionMatrix ();
}

Rect
Projection::ProjectBounds (Rect bounds)
{
	Matrix3D *matrix = GetMatrix3D ();
	double   *m;
	double   x = bounds.x;
	double   y = bounds.y;
	double   p1[4] = { 0.0, 0.0, 1.0, 1.0 };
	double   p2[4] = { bounds.width, 0.0, 1.0, 1.0 };
	double   p3[4] = { bounds.width, bounds.height, 1.0, 1.0 };
	double   p4[4] = { 0.0, bounds.height, 1.0, 1.0 };

	if (!matrix)
		return bounds;

	m = (double *) matrix->GetMatrixValues ();

	Matrix3D::TransformPoint (p1, m, p1);
	Matrix3D::TransformPoint (p2, m, p2);
	Matrix3D::TransformPoint (p3, m, p3);
	Matrix3D::TransformPoint (p4, m, p4);

	if (p1[3] == 0.0 || p2[3] == 0.0 || p3[3] == 0.0 || p4[3] == 0.0)
		return bounds;

	p1[0] /= p1[3];
	p1[1] /= p1[3];

	p2[0] /= p2[3];
	p2[1] /= p2[3];

	p3[0] /= p3[3];
	p3[1] /= p3[3];

	p4[0] /= p4[3];
	p4[1] /= p4[3];

	bounds = Rect (p1[0], p1[1], 0, 0);
	bounds = bounds.ExtendTo (p2[0], p2[1]);
	bounds = bounds.ExtendTo (p3[0], p3[1]);
	bounds = bounds.ExtendTo (p4[0], p4[1]);

	bounds.x += x;
	bounds.y += y;

	return bounds;
}

void
PlaneProjection::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::PLANEPROJECTION) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}

	need_update = true;

	NotifyListenersOfPropertyChange (args, error);
}

void
PlaneProjection::UpdateProjection ()
{
	double matrix[16];

	memset (matrix, 0, sizeof (matrix));
	matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0;

	SetProjectionMatrix (new Matrix3D (matrix));
}
