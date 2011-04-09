/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal-fontconfig.cpp: different types of collections
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "pal.h"

namespace Moonlight {

class MoonFontServiceFontconfig : public MoonFontService {
	double dpi;
public:
	MoonFontServiceFontconfig ();
	~MoonFontServiceFontconfig ();
	
	virtual void ForeachFont (MoonForeachFontCallback foreach, gpointer user_data);
	virtual MoonFont *FindFont (const FontStyleInfo *pattern);
	
	virtual guint32 GetCharIndex (FT_Face face, gunichar unichar);
};

};
