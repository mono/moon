/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * text.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __TEXT_H__
#define __TEXT_H__

#include <glib.h>
#include <cairo.h>

#include <frameworkelement.h>
#include <downloader.h>
#include <moon-path.h>
#include <thickness.h>
#include <layout.h>
#include <brush.h>
#include <font.h>

#define TEXTBLOCK_FONT_FAMILY  "Portable User Interface"
#define TEXTBLOCK_FONT_STRETCH FontStretchesNormal
#define TEXTBLOCK_FONT_WEIGHT  FontWeightsNormal
#define TEXTBLOCK_FONT_STYLE   FontStylesNormal
#define TEXTBLOCK_FONT_SIZE    14.666666984558105

G_BEGIN_DECLS

void text_shutdown (void);

G_END_DECLS


/* @Namespace=System.Windows.Documents */
class Inline : public DependencyObject {
 protected:
	virtual ~Inline ();
	
 public:
 	/* @PropertyType=string,DefaultValue=TEXTBLOCK_FONT_FAMILY,ManagedPropertyType=FontFamily,GenerateAccessors */
	static DependencyProperty *FontFamilyProperty;
 	/* @PropertyType=double,DefaultValue=TEXTBLOCK_FONT_SIZE,GenerateAccessors */
	static DependencyProperty *FontSizeProperty;
 	/* @PropertyType=FontStretch,DefaultValue=TEXTBLOCK_FONT_STRETCH,GenerateAccessors */
	static DependencyProperty *FontStretchProperty;
 	/* @PropertyType=FontStyle,DefaultValue=TEXTBLOCK_FONT_STYLE,GenerateAccessors */
	static DependencyProperty *FontStyleProperty;
 	/* @PropertyType=FontWeight,DefaultValue=TEXTBLOCK_FONT_WEIGHT,GenerateAccessors */
	static DependencyProperty *FontWeightProperty;
 	/* @PropertyType=Brush,DefaultValue=new SolidColorBrush("black"),GenerateAccessors */
	static DependencyProperty *ForegroundProperty;
 	/* @PropertyType=TextDecorations,DefaultValue=TextDecorationsNone,ManagedPropertyType=TextDecorationCollection,GenerateAccessors */
	static DependencyProperty *TextDecorationsProperty;
	/* @PropertyType=string,DefaultValue=\"en-US\",Version=2.0,ManagedPropertyType=XmlLanguage,Validator=NonNullStringValidator */
	static DependencyProperty *LanguageProperty;

	// internal property to inherit the font filename between inlines and textblocks
 	/* @PropertyType=string,GenerateManagedDP,GenerateAccessors */
	static DependencyProperty *FontFilenameProperty;
	
	/* Member variables should be considered private, for use only with the parent TextBlock */
	TextFontDescription *font;
	Brush *foreground;
	bool autogen;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Inline ();
	
	virtual Type::Kind GetObjectType () { return Type::INLINE; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);

	// property accessors
	void SetFontFamily (const char *family);
	const char *GetFontFamily ();

	double GetFontSize ();
	void SetFontSize (double value);

	FontStretches GetFontStretch ();
	void SetFontStretch (FontStretches value);

	FontStyles GetFontStyle ();
	void SetFontStyle (FontStyles value);

	FontWeights GetFontWeight ();
	void SetFontWeight (FontWeights value);

	void SetTextDecorations (TextDecorations decorations);
	TextDecorations GetTextDecorations ();

	Brush* GetForeground ();
	void SetForeground (Brush* value);

	const char* GetFontFilename ();
	void SetFontFilename (const char *value);
};


/* @Namespace=System.Windows.Documents */
class LineBreak : public Inline {
 protected:
	virtual ~LineBreak () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	LineBreak () { }
	
	virtual Type::Kind GetObjectType () { return Type::LINEBREAK; }
};


/* @ContentProperty="Text" */
/* @Namespace=System.Windows.Documents */
class Run : public Inline {
 protected:
	virtual ~Run () {}
	
 public:
 	/* @PropertyType=string,ManagedFieldAccess=Internal,GenerateAccessors */
	static DependencyProperty *TextProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Run () { }
	
	virtual Type::Kind GetObjectType () { return Type::RUN; }
	
	//
	// Property Accessors
	//
	void SetText (const char *text);
	const char *GetText ();
};


class TextBlockDynamicPropertyValueProvider;

/* @ContentProperty="Inlines" */
/* @Namespace=System.Windows.Controls */
class TextBlock : public FrameworkElement {
	friend class TextBlockDynamicPropertyValueProvider;
	
	TextFontDescription *font;
	Downloader *downloader;
	TextLayout *layout;
	
	double actual_height;
	double actual_width;
	bool setvalue;
	bool dirty;
	
	void CalcActualWidthHeight (cairo_t *cr);
	void Layout (cairo_t *cr);
	void Paint (cairo_t *cr);
	
	char *GetTextInternal ();
	bool SetTextInternal (const char *text);
	
	double GetBoundingWidth ()
	{
		double actual = GetActualWidth ();
		double width = GetWidth ();
		
		if (isnan (width))
			return actual;
		
		if (width> actual)
			return width;
		
		return actual;
	}
	
	double GetBoundingHeight ()
	{
		double actual = GetActualHeight ();
		double height = GetHeight();
		
		if (isnan (height))
			return actual;
		
		if (height > actual)
			return height;
		
		return actual;
	}
	
	void DownloaderComplete ();
	
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	
 protected:
	virtual ~TextBlock ();

 public:
 	/* @PropertyType=double,ReadOnly */
	static DependencyProperty *ActualHeightProperty;
 	/* @PropertyType=double,ReadOnly */
	static DependencyProperty *ActualWidthProperty;
 	/* @PropertyType=string,DefaultValue=TEXTBLOCK_FONT_FAMILY,ManagedPropertyType=FontFamily,GenerateAccessors */
	static DependencyProperty *FontFamilyProperty;
 	/* @PropertyType=double,DefaultValue=TEXTBLOCK_FONT_SIZE,GenerateAccessors */
	static DependencyProperty *FontSizeProperty;
 	/* @PropertyType=FontStretch,DefaultValue=TEXTBLOCK_FONT_STRETCH,GenerateAccessors */
	static DependencyProperty *FontStretchProperty;
 	/* @PropertyType=FontStyle,DefaultValue=TEXTBLOCK_FONT_STYLE,GenerateAccessors */
	static DependencyProperty *FontStyleProperty;
 	/* @PropertyType=FontWeight,DefaultValue=TEXTBLOCK_FONT_WEIGHT,GenerateAccessors */
	static DependencyProperty *FontWeightProperty;
 	/* @PropertyType=Brush,GenerateAccessors */
	static DependencyProperty *ForegroundProperty;
 	/* @PropertyType=InlineCollection,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	static DependencyProperty *InlinesProperty;
 	/* @PropertyType=string,GenerateAccessors */
	static DependencyProperty *TextProperty;
	/* @PropertyType=double,DefaultValue=NAN,Version=2.0,GenerateAccessors */
	static DependencyProperty *LineHeightProperty;
	/* @PropertyType=LineStackingStrategy,DefaultValue=LineStackingStrategyMaxHeight,Version=2.0,GenerateAccessors */
	static DependencyProperty *LineStackingStrategyProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness (0),Version=2.0,GenerateAccessors */
	static DependencyProperty *PaddingProperty;
	/* @PropertyType=TextAlignment,DefaultValue=TextAlignmentLeft,Version=2.0,GenerateAccessors */
	static DependencyProperty *TextAlignmentProperty;
 	/* @PropertyType=TextDecorations,DefaultValue=TextDecorationsNone,ManagedPropertyType=TextDecorationCollection,GenerateAccessors */
	static DependencyProperty *TextDecorationsProperty;
 	/* @PropertyType=TextWrapping,DefaultValue=TextWrappingNoWrap,GenerateAccessors */
	static DependencyProperty *TextWrappingProperty;

	// internal property to inherit the font filename between inlines and textblocks
 	/* @PropertyType=string,GenerateManagedDP,GenerateAccessors */
	static DependencyProperty *FontFilenameProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	TextBlock ();
	
	virtual Type::Kind GetObjectType () { return Type::TEXTBLOCK; }
	
	void SetFontSource (Downloader *downloader);
	
	//
	// Overrides
	//
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void ComputeBounds ();
	virtual bool InsideObject (cairo_t *cr, double x, double y);
	virtual Point GetTransformOrigin ();
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	
	//
	// Property Accessors
	//
	double GetActualWidth ()
	{
		if (dirty)
			CalcActualWidthHeight (NULL);
		return actual_width;
	}
	
	double GetActualHeight ()
	{
		if (dirty)
			CalcActualWidthHeight (NULL);
		return actual_height;
	}
	
	void SetFontFamily (const char *family);
	const char *GetFontFamily ();
	
	void SetFontSize (double size);
	double GetFontSize ();
	
	void SetFontStretch (FontStretches stretch);
	FontStretches GetFontStretch ();
	
	void SetFontStyle (FontStyles style);
	FontStyles GetFontStyle ();
	
	void SetFontWeight (FontWeights weight);
	FontWeights GetFontWeight ();
	
	void SetForeground (Brush *fg);
	Brush *GetForeground ();
	
	void SetInlines (InlineCollection *inlines);
	InlineCollection *GetInlines ();
	
	void SetLineHeight (double height);
	double GetLineHeight ();
	
	void SetLineStackingStrategy (LineStackingStrategy strategy);
	LineStackingStrategy GetLineStackingStrategy ();
	
	void SetPadding (Thickness *padding);
	Thickness *GetPadding ();
	
	void SetText (const char *text);
	const char *GetText ();
	
	void SetTextAlignment (TextAlignment alignment);
	TextAlignment GetTextAlignment ();
	
	void SetTextDecorations (TextDecorations decorations);
	TextDecorations GetTextDecorations ();
	
	void SetTextWrapping (TextWrapping wrapping);
	TextWrapping GetTextWrapping ();

	const char* GetFontFilename ();
	void SetFontFilename (const char *value);
};


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
	static DependencyProperty *FillProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *FontRenderingEmSizeProperty;
 	/* @PropertyType=string,ManagedPropertyType=Uri,GenerateAccessors,Validator=NonNullStringValidator */
	static DependencyProperty *FontUriProperty;
 	/* @PropertyType=string,GenerateAccessors */
	static DependencyProperty *IndicesProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *OriginXProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *OriginYProperty;
 	/* @PropertyType=StyleSimulations,DefaultValue=StyleSimulationsNone,GenerateAccessors */
	static DependencyProperty *StyleSimulationsProperty;
 	/* @PropertyType=string,GenerateAccessors */
	static DependencyProperty *UnicodeStringProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Glyphs ();
	
	virtual Type::Kind GetObjectType () { return Type::GLYPHS; }
	
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);
	virtual void ComputeBounds ();
	virtual bool InsideObject (cairo_t *cr, double x, double y);
	virtual Point GetTransformOrigin ();
	virtual Point GetOriginPoint ();
	virtual void SetSurface (Surface *surface);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
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

/* @Namespace=System.Windows.Input */
class InputMethod : public DependencyObject {
	
 protected:
	virtual ~InputMethod () {}
	
 public:
	/* @PropertyType=bool,Attached */
	static DependencyProperty *IsInputMethodEnabledProperty;
	
 	/* @ManagedAccess=Internal,GeneratePInvoke,GenerateCBinding */
	InputMethod () {}
	
	virtual Type::Kind GetObjectType () { return Type::INPUTMETHOD; }
};

#endif /* __TEXT_H__ */
