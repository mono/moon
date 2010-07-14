/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * glyphs.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __GLYPHS_H__
#define __GLYPHS_H__

#include <glib.h>
#include <cairo.h>

#include "frameworkelement.h"
#include "downloader.h"
#include "fontfamily.h"
#include "fontsource.h"
#include "fontstretch.h"
#include "fontstyle.h"
#include "fontweight.h"
#include "moon-path.h"
#include "thickness.h"
#include "layout.h"
#include "brush.h"
#include "fonts.h"

namespace Moonlight {

/* @Namespace=System.Windows.Documents */
class Glyphs : public FrameworkElement {
	Downloader *downloader;
	char *part_name;
	int index;
	
	moon_path *path;
	TextFont *font;
	gunichar *text;
	List *attrs;
	Brush *fill;
	
	double height;
	double width;
	double left;
	double top;
	
	int uri_changed:1;
	int invalid:1;
	int dirty:1;
	
	void Layout ();
	void SetIndicesInternal (const char *in);
	
	void CleanupDownloader ();
	void DownloaderComplete ();
	
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	
	void LoadFont (const Uri *uri, const char *path);
	void DownloadFont (Uri *uri, MoonError *error);
	bool SetFontResource (const Uri *uri);
	
 protected:
	virtual ~Glyphs ();
	
 public:
 	/* @PropertyType=Brush,GenerateAccessors */
	const static int FillProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int FontRenderingEmSizeProperty;
	/* @PropertyType=FontSource,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int FontSourceProperty;
 	/* @PropertyType=Uri,GenerateAccessors,DefaultValue=Uri(),Validator=NonNullValidator */
	const static int FontUriProperty;
 	/* @PropertyType=string,DefaultValue=\"\",GenerateAccessors */
	const static int IndicesProperty;
 	/* @PropertyType=double,DefaultValue=-HUGE,Validator=FloatValidator,GenerateAccessors */
	const static int OriginXProperty;
 	/* @PropertyType=double,DefaultValue=-HUGE,Validator=FloatValidator,GenerateAccessors */
	const static int OriginYProperty;
 	/* @PropertyType=StyleSimulations,DefaultValue=StyleSimulationsNone,GenerateAccessors */
	const static int StyleSimulationsProperty;
 	/* @PropertyType=string,DefaultValue=\"\",GenerateAccessors */
	const static int UnicodeStringProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Glyphs ();
	
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);
	virtual Size ComputeActualSize ();
	virtual void ComputeBounds ();
	virtual Size MeasureOverrideWithError (Size availableSize, MoonError *error);
	virtual Size ArrangeOverrideWithError (Size finalSize, MoonError *error);
	virtual Point GetTransformOrigin ();
	virtual Point GetOriginPoint ();
	virtual void OnIsAttachedChanged (bool attached);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	void SetFontSource (Downloader *downloader, const char *part_name);
	void SetFontSource (ManagedStreamCallbacks *stream);
	void SetFontSource (GlyphTypeface *typeface);
	
	//
	// Property Accessors
	//
	void SetFill (Brush *fill);
	Brush *GetFill ();
	
	void SetFontRenderingEmSize (double size);
	double GetFontRenderingEmSize ();
	
	void SetFontSource (FontSource *source);
	FontSource *GetFontSource ();
	
	void SetFontUri (Uri *uri);
	Uri *GetFontUri ();
	
	void SetIndices (const char *indices);
	const char *GetIndices ();
	
	void SetOriginX (double origin);
	double GetOriginX ();
	
	void SetOriginY (double origin);
	double GetOriginY ();
	
	void SetStyleSimulations (StyleSimulations style);
	StyleSimulations GetStyleSimulations ();
	
	void SetUnicodeString (const char *unicode);
	const char *GetUnicodeString ();
};

};
#endif /* __GLYPHS_H__ */
