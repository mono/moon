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

#include <frameworkelement.h>
#include <downloader.h>
#include <moon-path.h>
#include <thickness.h>
#include <layout.h>
#include <brush.h>
#include <font.h>
#include <fontfamily.h>

/* @Namespace=System.Windows.Documents */
class Glyphs : public FrameworkElement {
	TextFontDescription *desc;
	Downloader *downloader;
	
	moon_path *path;
	gunichar *text;
	List *attrs;
	Brush *fill;
	int index;
	
	double origin_x;
	double origin_y;
	double height;
	double width;
	double left;
	double top;
	
	int origin_y_specified:1;
	int simulation_none:1;
	int uri_changed:1;
	int invalid:1;
	int dirty:1;
	
	void Layout ();
	void SetIndicesInternal (const char *in);
	
	void DownloaderComplete ();
	
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	
	void DownloadFont (Surface *surface, const char *url);
	
 protected:
	virtual ~Glyphs ();
	
 public:
 	/* @PropertyType=Brush,GenerateAccessors */
	const static int FillProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int FontRenderingEmSizeProperty;
 	/* @PropertyType=string,ManagedPropertyType=Uri,DefaultValue=\"\",GenerateAccessors,Validator=NonNullStringValidator */
	const static int FontUriProperty;
 	/* @PropertyType=string,DefaultValue=\"\",GenerateAccessors */
	const static int IndicesProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int OriginXProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int OriginYProperty;
 	/* @PropertyType=StyleSimulations,DefaultValue=StyleSimulationsNone,GenerateAccessors */
	const static int StyleSimulationsProperty;
 	/* @PropertyType=string,DefaultValue=\"\",GenerateAccessors */
	const static int UnicodeStringProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Glyphs ();
	
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);
	virtual void ComputeBounds ();
	virtual bool InsideObject (cairo_t *cr, double x, double y);
	virtual Point GetTransformOrigin ();
	virtual Point GetOriginPoint ();
	virtual void SetSurface (Surface *surface);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	//
	// Property Accessors
	//
	void SetFill (Brush *fill);
	Brush *GetFill ();
	
	void SetFontRenderingEmSize (double size);
	double GetFontRenderingEmSize ();
	
	void SetFontUri (const char *uri);
	const char *GetFontUri ();
	
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

#endif /* __GLYPHS_H__ */
