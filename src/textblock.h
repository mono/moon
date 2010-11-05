/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * textblock.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __TEXTBLOCK_H__
#define __TEXTBLOCK_H__

#include <glib.h>
#include <cairo.h>

#include "frameworkelement.h"
#include "textelement.h"
#include "downloader.h"
#include "moon-path.h"

#define TEXTBLOCK_FONT_FAMILY  "Portable User Interface"
#define TEXTBLOCK_FONT_STRETCH FontStretchesNormal
#define TEXTBLOCK_FONT_WEIGHT  FontWeightsNormal
#define TEXTBLOCK_FONT_STYLE   FontStylesNormal

namespace Moonlight {

class TextBlockDynamicPropertyValueProvider;

/* @ContentProperty="Inlines" */
/* @Namespace=System.Windows.Controls */
class TextBlock : public FrameworkElement, public ITextLayoutContainer {
	friend class TextBlockDynamicPropertyValueProvider;
	
	TextFontDescription *font;
	GPtrArray *downloaders;
	Downloader *source;
	TextLayout *layout;
	
	double actual_height;
	double actual_width;
	
	int setvalue:1;
	int was_set:1;
	int dirty:1;
	
	void Layout (Size constraint);
	void Paint (cairo_t *cr);
	
	char *GetTextInternal (InlineCollection *inlines);
	void SetTextInternal (const char *text);
	
	void AddFontResource (const char *resource);
	void AddFontSource (Downloader *downloader);
	
	void UpdateLayoutAttributes ();
	bool UpdateFontDescription (bool force);
	bool UpdateFontDescriptions (bool force);
	
	void CleanupDownloaders (bool all);
	
	void DownloaderComplete (Downloader *downloader);
	
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	
 protected:
	virtual ~TextBlock ();
	
 public:
	/* @PropertyType=FontFamily,DefaultValue=FontFamily(TEXTBLOCK_FONT_FAMILY),GenerateAccessors,Validator=NonNullValidator */
	const static int FontFamilyProperty;
	/* @PropertyType=double,AutoCreator=CreateDefaultFontSize,GenerateAccessors,Validator=DoubleGreaterThanZeroValidator */
	const static int FontSizeProperty;
	/* @PropertyType=FontStretch,DefaultValue=FontStretch(TEXTBLOCK_FONT_STRETCH),GenerateAccessors */
	const static int FontStretchProperty;
	/* @PropertyType=FontStyle,DefaultValue=FontStyle(TEXTBLOCK_FONT_STYLE),GenerateAccessors */
	const static int FontStyleProperty;
	/* @PropertyType=FontWeight,DefaultValue=FontWeight(TEXTBLOCK_FONT_WEIGHT),GenerateAccessors */
	const static int FontWeightProperty;
	/* @PropertyType=FontSource,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int FontSourceProperty;
	/* @PropertyType=Brush,AutoCreator=CreateBlackBrush,GenerateAccessors */
	const static int ForegroundProperty;
	/* @PropertyType=InlineCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int InlinesProperty;
	/* @PropertyType=string,DefaultValue=\"\",GenerateAccessors */
	const static int TextProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int LineHeightProperty;
	/* @PropertyType=LineStackingStrategy,DefaultValue=LineStackingStrategyMaxHeight,GenerateAccessors */
	const static int LineStackingStrategyProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness (0),GenerateAccessors */
	const static int PaddingProperty;
	/* @PropertyType=TextAlignment,DefaultValue=TextAlignmentLeft,GenerateAccessors */
	const static int TextAlignmentProperty;
	/* @PropertyType=TextDecorations,DefaultValue=TextDecorationsNone,ManagedPropertyType=TextDecorationCollection,GenerateAccessors,Coercer=TextElement::CoerceTextDecorations */
	const static int TextDecorationsProperty;
	/* @PropertyType=TextWrapping,DefaultValue=TextWrappingNoWrap,GenerateAccessors */
	const static int TextWrappingProperty;
	/* @PropertyType=TextTrimming,DefaultValue=TextTrimmingNone */
	const static int TextTrimmingProperty;	

	/* @PropertyType=FontResource,GenerateManagedDP=false,GenerateManagedAccessors=false,GenerateManagedField=false,GenerateAccessors */
	const static int FontResourceProperty;
	
	/* @GeneratePInvoke */
	TextBlock ();
	
	void SetFontSource (Downloader *downloader);
	
	//
	// Overrides
	//
	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);
	virtual Size MeasureOverrideWithError (Size availableSize, MoonError *error);
	virtual Size ArrangeOverrideWithError (Size finalSize, MoonError *error);
	virtual Size ComputeActualSize ();
	virtual void ComputeBounds ();
	virtual Point GetTransformOrigin ();
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual bool CanFindElement () { return true; }
	virtual bool InsideObject (cairo_t *cr, double x, double y);

	// ITextLayoutContainer interface
	virtual void DocumentPropertyChanged (TextElement *onElement, PropertyChangedEventArgs *args);
	virtual void DocumentCollectionChanged (TextElement *onElement, Collection *col, CollectionChangedEventArgs *args);
	
	void SetFontFamily (FontFamily *family);
	FontFamily *GetFontFamily ();
	
	void SetFontSize (double size);
	double GetFontSize ();
	
	void SetFontStretch (FontStretch *stretch);
	FontStretch *GetFontStretch ();
	
	void SetFontStyle (FontStyle *style);
	FontStyle *GetFontStyle ();
	
	void SetFontWeight (FontWeight *weight);
	FontWeight *GetFontWeight ();
	
	void SetFontSource (FontSource *source);
	FontSource *GetFontSource ();

	void SetFontResource (FontResource *resource);
	FontResource *GetFontResource ();
	
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
};

};
#endif /* __TEXTBLOCK_H__ */
