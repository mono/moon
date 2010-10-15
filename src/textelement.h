/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * textelement.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __TEXTELEMENT_H__
#define __TEXTELEMENT_H__

#include <glib.h>
#include <cairo.h>

#include "frameworkelement.h"
#include "downloader.h"
#include "thickness.h"
#include "fontfamily.h"
#include "fontstretch.h"
#include "fontstyle.h"
#include "fontweight.h"
#include "fontsource.h"
#include "layout.h"
#include "enums.h"
#include "brush.h"
#include "fonts.h"

#define TEXTBLOCK_FONT_FAMILY  "Portable User Interface"
#define TEXTBLOCK_FONT_STRETCH FontStretchesNormal
#define TEXTBLOCK_FONT_WEIGHT  FontWeightsNormal
#define TEXTBLOCK_FONT_STYLE   FontStylesNormal

namespace Moonlight {

/* @Namespace=System.Windows.Documents */
class TextElement : public DependencyObject, public ITextAttributes {
	TextFontDescription *font;
	GPtrArray *downloaders;
	
	void AddFontResource (const char *resource);
	void AddFontSource (Downloader *downloader);
	
	void CleanupDownloaders ();
	
	void DownloaderComplete (Downloader *downloader);
	
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	
 protected:
	/* @GeneratePInvoke,ManagedAccess=Protected */
	TextElement ();
	
	virtual ~TextElement ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=FontFamily,DefaultValue=FontFamily(TEXTBLOCK_FONT_FAMILY),GenerateAccessors */
	const static int FontFamilyProperty;
	/* @PropertyType=double,AutoCreator=CreateDefaultFontSize,GenerateAccessors */
	const static int FontSizeProperty;
	/* @PropertyType=FontStretch,DefaultValue=FontStretch(TEXTBLOCK_FONT_STRETCH),GenerateAccessors */
	const static int FontStretchProperty;
	/* @PropertyType=FontStyle,DefaultValue=FontStyle(TEXTBLOCK_FONT_STYLE),GenerateAccessors */
	const static int FontStyleProperty;
	/* @PropertyType=FontWeight,DefaultValue=FontWeight(TEXTBLOCK_FONT_WEIGHT),GenerateAccessors */
	const static int FontWeightProperty;
	/* @PropertyType=Brush,AutoCreator=CreateBlackBrush,GenerateAccessors */
	const static int ForegroundProperty;
	/* @PropertyType=string,DefaultValue=\"en-US\",ManagedPropertyType=XmlLanguage,Validator=LanguageValidator,GenerateAccessors */
	const static int LanguageProperty;
	/* @PropertyType=TextDecorations,DefaultValue=TextDecorationsNone,HiddenDefaultValue,ManagedPropertyType=TextDecorationCollection,GenerateAccessors,Coercer=TextElement::CoerceTextDecorations */
	const static int TextDecorationsProperty;

	static bool CoerceTextDecorations (DependencyObject *obj, DependencyProperty *p, Value *value, Value **coerced, MoonError *error);

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	//
	// ITextAttributes Interface Methods
	//
	virtual TextFontDescription *FontDescription () { return font; }
	virtual FlowDirection Direction () { return FlowDirectionLeftToRight; }
	virtual TextDecorations Decorations () { return GetTextDecorations (); }
	virtual Brush *Foreground (bool selected) { return GetForeground (); }
	virtual Brush *Background (bool selected) { return NULL; }

	/* @GeneratePInvoke */
	TextPointer *GetContentStart ();
	/* @GeneratePInvoke */
	TextPointer *GetContentEnd ();
	/* @GeneratePInvoke */
	TextPointer *GetElementStart ();
	/* @GeneratePInvoke */
	TextPointer *GetElementEnd ();
	
	//
	// Property Accessors
	//
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
	
	void SetForeground (Brush *fg);
	Brush *GetForeground ();
	
	void SetLanguage (const char *language);
	const char *GetLanguage ();
	
	void SetTextDecorations (TextDecorations decorations);
	TextDecorations GetTextDecorations ();
	
	//
	// Convenience Methods
	//
	bool UpdateFontDescription (const FontResource *resource, bool force);
};


//
// TextElement subclasses used by TextBlock
//


/* @Namespace=System.Windows.Documents */
class Inline : public TextElement {
	bool autogen;
	
 protected:
	/* @GeneratePInvoke,ManagedAccess=Protected */
	Inline ();

	virtual ~Inline () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	// internal properties to inherit the FontSource between inlines and textblocks
	/* @PropertyType=FontSource,GenerateManagedDP=false,GenerateAccessors */
	const static int FontSourceProperty;
	
	virtual bool PermitsMultipleParents () { return false; }
	
	void SetFontSource (FontSource *source);
	FontSource *GetFontSource ();
	
	//
	// Non-DependencyProperty Accessors
	//
	void SetAutogenerated (bool autogen) { this->autogen = autogen; }
	bool GetAutogenerated () { return autogen; }
	
	//
	// Convenience Methods
	//
	virtual bool Equals (Inline *item);
};

/* @Namespace=System.Windows.Documents */
class LineBreak : public Inline {
 protected:
	/* @GeneratePInvoke */
	LineBreak ();

	virtual ~LineBreak () {}
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @ContentProperty="Text" */
/* @Namespace=System.Windows.Documents */
class Run : public Inline {
 protected:
	/* @GeneratePInvoke */
	Run ();
	
	virtual ~Run () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=FlowDirection,DefaultValue=FlowDirectionLeftToRight,GenerateAccessors */
	const static int FlowDirectionProperty;
	/* @PropertyType=string,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int TextProperty;
	
	virtual bool Equals (Inline *item);
	
	//
	// ITextAttributes Interface Method Overrides
	//
	virtual FlowDirection Direction () { return GetFlowDirection (); }
	
	//
	// Property Accessors
	//
	void SetFlowDirection (FlowDirection direction);
	FlowDirection GetFlowDirection ();
	
	void SetText (const char *text);
	const char *GetText ();
};


//
// TextElement subclasses used by RichTextBox
//


/* @Namespace=System.Windows.Documents */
class Block : public TextElement {
 protected:
	/* @GeneratePInvoke,ManagedAccess=Protected */
	Block ();

	virtual ~Block () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=TextAlignment,DefaultValue=TextAlignmentLeft,GenerateAccessors */
	const static int TextAlignmentProperty;
	
	//
	// Property Accessors
	//
	void SetTextAlignment (TextAlignment alignment);
	TextAlignment GetTextAlignment ();
};

/* @Namespace=System.Windows.Documents */
/* @ContentProperty=Inlines */
class Paragraph : public Block {
 protected:
	/* @GeneratePInvoke */
	Paragraph ();
	
	virtual ~Paragraph () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=InlineCollection,AutoCreateValue,GenerateAccessors,ManagedSetterAccess=Private,ManagedFieldAccess=Internal */
	const static int InlinesProperty;
	
	//
	// Property Accessors
	//
	void SetInlines (InlineCollection *inlines);
	InlineCollection *GetInlines ();
};

/* @Namespace=System.Windows.Documents */
/* @ContentProperty=Blocks */
class Section : public Block {
 protected:
	/* @GeneratePInvoke */
	Section ();
	
	virtual ~Section () {}
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=BlockCollection,AutoCreateValue,GenerateAccessors,ManagedSetterAccess=Private,ManagedFieldAccess=Internal*/
	const static int BlocksProperty;
	/* @PropertyType=bool,DefaultValue=true,GenerateAccessors,ManagedFieldAccess=Internal */
	const static int HasTrailingParagraphBreakOnPasteProperty;
	
	//
	// Property Accessors
	//
	void SetBlocks (BlockCollection *value);
	BlockCollection *GetBlocks ();
	
	void SetHasTrailingParagraphBreakOnPaste (bool value);
	bool GetHasTrailingParagraphBreakOnPaste ();
};

/* @Namespace=System.Windows.Documents */
/* @ContentProperty=Inlines */
class Span : public Inline {
 protected:
	/* @GeneratePInvoke */
	Span ();

	virtual ~Span () {}
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=InlineCollection,AutoCreateValue,GenerateAccessors,ManagedFieldAccess=Internal */
	const static int InlinesProperty;
	
	//
	// Property Accessors
	//
	void SetInlines (InlineCollection *inlines);
	InlineCollection *GetInlines ();
};

/* @Namespace=System.Windows.Documents */
class Bold : public Span {
 protected:
	/* @GeneratePInvoke */
	Bold ();

	virtual ~Bold () {}
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Documents */
class Italic : public Span {
 protected:
	/* @GeneratePInvoke */
	Italic ();
	
	virtual ~Italic () {}
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Documents */
class Underline : public Span {
 protected:
	/* @GeneratePInvoke */
	Underline ();

	virtual ~Underline () {}
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Documents */
/* @CallInitialize */
class Hyperlink : public Span {
 protected:
	/* @GeneratePInvoke */
	Hyperlink ();
	
	virtual ~Hyperlink () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=object */
	const static int CommandParameterProperty;
	/* @PropertyType=object,ManagedPropertyType=ICommand */
	const static int CommandProperty;
	/* @PropertyType=Brush,AutoCreator=CreateBlackBrush,GenerateAccessors */
	const static int MouseOverForegroundProperty;
	/* @PropertyType=TextDecorations,DefaultValue=TextDecorationsUnderline,ManagedPropertyType=TextDecorationCollection */
	const static int MouseOverTextDecorationsProperty;
	/* @PropertyType=Uri,DefaultValue=new Uri () */
	const static int NavigateUriProperty;
	/* @PropertyType=string,DefaultValue=\"\" */
	const static int TargetNameProperty;
	
	//
	// Property Accessors
	//
	void SetMouseOverForeground (Brush *brush);
	Brush *GetMouseOverForeground ();

	/* @DelegateType=RoutedEventHandler */
	const static int ClickEvent;
};

/* @Namespace=System.Windows.Documents */
/* @ContentProperty=Child */
class InlineUIContainer : public Inline {
 protected:
	/* @GeneratePInvoke */
	InlineUIContainer ();
	
	virtual ~InlineUIContainer () {}
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=UIElement,GenerateAccessors,ManagedFieldAccess=Internal */
	const static int ChildProperty;
	
	//
	// Property Accessors
	//
	void SetChild (UIElement *child);
	UIElement *GetChild ();
};

};
#endif /* __TEXTELEMENT_H__ */
