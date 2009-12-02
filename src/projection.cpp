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

