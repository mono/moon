/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * text.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cairo.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "file-downloader.h"
#include "runtime.h"
#include "color.h"
#include "text.h"
#include "uri.h"
#include "utils.h"
#include "debug.h"


//
// Inline
//

Inline::Inline ()
{
	SetObjectType (Type::INLINE);
	font = new TextFontDescription ();
	autogen = false;
}

Inline::~Inline ()
{
	delete font;
}

void
Inline::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == Inline::ForegroundProperty) {
		// this isn't exactly what we want, I don't
		// think... but it'll have to do.
		NotifyListenersOfPropertyChange (prop);
	} else {
		DependencyObject::OnSubPropertyChanged (prop, obj, subobj_args);
	}
}

bool
Inline::Equals (Inline *item)
{
	if (item->GetObjectType () != GetObjectType ())
		return false;
	
	if (*item->GetFontFamily() == *GetFontFamily())
		return false;
	
	if (item->GetFontSize () != GetFontSize ())
		return false;
	
	if (item->GetFontStyle () != GetFontStyle ())
		return false;
	
	if (item->GetFontWeight () != GetFontWeight ())
		return false;
	
	if (item->GetFontStretch () != GetFontStretch ())
		return false;
	
	if (item->GetFontFilename () != GetFontFilename ())
		return false;
	
	if (item->GetTextDecorations () != GetTextDecorations ())
		return false;
	
	// this isn't really correct - we should be checking
	// the innards of the foreground brushes, but we're
	// guaranteed to never have a false positive here.
	if (item->GetForeground () != GetForeground ())
		return false;
	
	// OK, as best we can tell - they are equal
	return true;
}

void
Inline::UpdateFontDescription ()
{
	// FIXME: I hate having to do it this way, updating the font
	// in OnPropertyChanged() was a much much better way to do
	// things. *sigh*
	font->SetFilename (GetFontFilename ());
	FontFamily *family = GetFontFamily ();
	font->SetFamily (family ? family->source : NULL);
	font->SetStyle (GetFontStyle ());
	font->SetWeight (GetFontWeight ());
	font->SetSize (GetFontSize ());
	font->SetStretch (GetFontStretch ());
}


//
// Run
//
bool
Run::Equals (Inline *item)
{
	const char *text, *itext;
	
	if (!Inline::Equals (item))
		return false;
	
	itext = ((Run *) item)->GetText ();
	text = GetText ();
	
	// compare the text
	if (text && itext && strcmp (text, itext) != 0)
		return false;
	else if ((text && !itext) || (!text && itext))
		return false;
	
	return true;
}


//
// TextBlockDynamicPropertyValueProvider
//

class TextBlockDynamicPropertyValueProvider : public PropertyValueProvider {
	Value *actual_height_value;
	Value *actual_width_value;
	
 public:
	TextBlockDynamicPropertyValueProvider (DependencyObject *obj) : PropertyValueProvider (obj)
	{
		actual_height_value = NULL;
		actual_width_value = NULL;
	}
	
	virtual ~TextBlockDynamicPropertyValueProvider ()
	{
		delete actual_height_value;
		delete actual_width_value;
	}
	
	virtual Value *GetPropertyValue (DependencyProperty *property)
	{
		if (property != FrameworkElement::ActualHeightProperty && property != FrameworkElement::ActualWidthProperty)
			return NULL;
		
		TextBlock *tb = (TextBlock *) obj;
		if (tb->dirty) {
			delete actual_height_value;
			actual_height_value = NULL;
			delete actual_width_value;
			actual_width_value = NULL;
			tb->CalcActualWidthHeight (NULL);
		}
		
		if (property == FrameworkElement::ActualHeightProperty) {
			if (!actual_height_value)
				actual_height_value = new Value (tb->actual_height);
			return actual_height_value;
		} else {
			if (!actual_width_value)
				actual_width_value = new Value (tb->actual_width);
			return actual_width_value;
		}
	}
};


//
// TextBlock
//

TextBlock::TextBlock ()
{
	SetObjectType (Type::TEXTBLOCK);

	providers[PropertyPrecedence_DynamicValue] = new TextBlockDynamicPropertyValueProvider (this);

	downloader = NULL;
	
	layout = new TextLayout ();
	
	constraint = Size (INFINITY, INFINITY);
	actual_height = 0.0;
	actual_width = 0.0;
	setvalue = true;
	dirty = true;
	
	font = new TextFontDescription ();
	font->SetFamily (TEXTBLOCK_FONT_FAMILY);
	font->SetStretch (TEXTBLOCK_FONT_STRETCH);
	font->SetWeight (TEXTBLOCK_FONT_WEIGHT);
	font->SetStyle (TEXTBLOCK_FONT_STYLE);
	font->SetSize (TEXTBLOCK_FONT_SIZE);
	
	setvalue = false;
	SetValue (TextBlock::ForegroundProperty, Value::CreateUnref (new SolidColorBrush ("black")));
	setvalue = true;
}

TextBlock::~TextBlock ()
{
	delete layout;
	delete font;
	
	if (downloader != NULL) {
		downloader_abort (downloader);
		downloader->unref ();
	}
}

void
TextBlock::SetFontSource (Downloader *downloader)
{
	if (this->downloader == downloader)
		return;
	
	if (this->downloader) {
		this->downloader->Abort ();
		this->downloader->unref ();
		this->downloader = NULL;
	}
	
	if (downloader) {
		this->downloader = downloader;
		downloader->ref ();
		
		downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);
		if (downloader->Started () || downloader->Completed ()) {
			if (downloader->Completed ())
				DownloaderComplete ();
		} else {
			// This is what actually triggers the download
			downloader->Send ();
		}
	} else {
		ClearValue (TextBlock::FontFilenameProperty);
		font->SetFilename (NULL);
		
		dirty = true;
		
		UpdateBounds (true);
		Invalidate ();
	}
}

void
TextBlock::Render (cairo_t *cr, Region *region)
{
	if (dirty)
		Layout (cr);
	
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	Paint (cr);
	cairo_restore (cr);
}

void
TextBlock::CalcActualWidthHeight (cairo_t *cr)
{
	bool destroy = false;
	
	if (cr == NULL) {
		cr = measuring_context_create ();
		destroy = true;
	} else {
		cairo_save (cr);
	}
	
	cairo_identity_matrix (cr);
	
	Layout (cr);
	
	if (destroy) {
		measuring_context_destroy (cr);
	} else {
		cairo_new_path (cr);
		cairo_restore (cr);
	}
}

#define USE_FE 0
void
TextBlock::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
#if USE_FE
	FrameworkElement::GetSizeForBrush (cr, width, height);
#else
	*width = actual_width;
	*height = actual_height;
#endif	
}

void
TextBlock::ComputeBounds ()
{
#if USE_FE
	FrameworkElement::ComputeBounds ();
#else
	Size total = Size (actual_width, actual_height);
	total.GrowBy (*GetPadding ());
	extents = Rect (0,0,total.width, total.height);
	bounds = IntersectBoundsWithClipPath (extents, false).Transform (&absolute_xform);
	bounds_with_children = bounds;
#endif
}

Point
TextBlock::GetTransformOrigin ()
{
#if USE_FE
	return FrameworkElement::GetTransformOrigin ();
#else
	Point *user_xform_origin = GetRenderTransformOrigin ();
	return Point (actual_width * user_xform_origin->x, 
		      actual_height * user_xform_origin->y);
#endif
}

Size
TextBlock::MeasureOverride (Size availableSize)
{
	Thickness padding = *GetPadding ();
	dirty = true;
	
	constraint = availableSize.GrowBy (-padding);

	if (constraint.width == 0 && constraint.height == 0)
		constraint = Size (INFINITY, INFINITY);

	CalcActualWidthHeight (NULL);
	
	//printf ("measure actual = %g, %g\n", actual_width, actual_height);
	Size desired = Size (actual_width, actual_height).GrowBy (padding);

	return availableSize.Min (desired);
}

Size
TextBlock::ArrangeOverride (Size finalSize)
{
	Thickness padding = *GetPadding ();
	dirty = true;
	
	constraint = finalSize.GrowBy (-padding);
	
	// Hack around the funky canvas logic in layout //
	if (constraint.width <= 0 || constraint.height <= 0)
		constraint = Size (INFINITY, INFINITY);

	CalcActualWidthHeight (NULL);
	
	//printf ("arrange actual = %g, %g\n", actual_width, actual_height);

	return Size (actual_width, actual_height).GrowBy (padding);
}

void
TextBlock::Layout (cairo_t *cr)
{
	InlineCollection *inlines = GetInlines ();
	double width = constraint.width;
	const char *text;
	List *runs;

	/* 
	 *  in the layout case this should already be handled
	 *  but we do it anyway to help
	 */
	if (!isnan (width))
		width = GetWidth ();
	
	if (width == 0)
		width = INFINITY;
	
	if (!(text = GetText ())) {
		// If the TextBlock's Text property is not set,
		// then don't modify ActualHeight.
		// Fixes bug #435798
		// actual_height = 0.0;
		actual_width = 0.0;
		goto done;
	}
	
	runs = new List ();
	
	if (!isinf (width)) {
		Thickness *padding = GetPadding ();
		double pad = padding->left + padding->right;
		
		if (pad >= width) {
			layout->SetTextRuns (runs);
			actual_height = 0.0;
			actual_width = 0.0;
			goto done;
		}
		
		layout->SetMaxWidth (width);
	} else {
		layout->SetMaxWidth (-1.0);
	}
	
	if (inlines != NULL) {
		Inline *item;
		Run *run;
		
		for (int i = 0; i < inlines->GetCount (); i++) {
			item = inlines->GetValueAt (i)->AsInline ();
			item->UpdateFontDescription ();
			
 			switch (item->GetObjectType ()) {
			case Type::RUN:
				run = (Run *) item;
				
				text = run->GetText ();
				
				if (text && text[0]) {
					const char *inptr, *inend;
					
					inptr = text;
					
					do {
						inend = inptr;
						while (*inend && *inend != '\r' && *inend != '\n')
							inend++;
						
						if (inend > inptr)
							runs->Append (new TextRun (inptr, inend - inptr, (ITextSource *) item));
						
						if (*inend == '\0')
							break;
						
						runs->Append (new TextRun ((ITextSource *) item));
						
						if (inend[0] == '\r' && inend[1] == '\n')
							inptr = inend + 2;
						else
							inptr = inend + 1;
					} while (*inptr);
				}
				
				break;
			case Type::LINEBREAK:
				runs->Append (new TextRun ((ITextSource *) item));
				break;
			default:
				break;
			}
		}
	}
	
	layout->SetTextRuns (runs);
	layout->Layout ();
	
	layout->GetActualExtents (&actual_width, &actual_height);
	//layout->GetLayoutExtents (&bbox_width, &bbox_height);
	
	if (runs->IsEmpty ()) {
		// If the Text property had been set once upon a time,
		// but is currently empty, Silverlight seems to set
		// the ActualHeight property to the font height. See
		// bug #405514 for details.
		TextFont *font = this->font->GetFont ();
		actual_height = font->Height ();
		font->unref ();
	}
	
 done:
	
	dirty = false;
}

void
TextBlock::Paint (cairo_t *cr)
{
	Thickness *padding = GetPadding ();
	Point offset (padding->left, padding->top);
	
	layout->Render (cr, GetOriginPoint (), offset);
	
	if (moonlight_flags & RUNTIME_INIT_SHOW_TEXTBOXES) {
		cairo_set_source_rgba (cr, 0.0, 1.0, 0.0, 1.0);
		cairo_set_line_width (cr, 1);
		cairo_rectangle (cr, 0, 0, actual_width, actual_height);
		cairo_stroke (cr);
	}
}

char *
TextBlock::GetTextInternal ()
{
	InlineCollection *inlines = GetInlines ();
	GString *block;
	char *str;
	
	block = g_string_new ("");
	
	if (inlines != NULL) {
		const char *text;
		Inline *item;
		
		for (int i = 0; i < inlines->GetCount (); i++) {
			item = inlines->GetValueAt (i)->AsInline ();
			
			switch (item->GetObjectType ()) {
			case Type::RUN:
				text = ((Run *) item)->GetText ();
				
				if (text && text[0])
					g_string_append (block, text);
				break;
			case Type::LINEBREAK:
				g_string_append_c (block, '\n');
				break;
			default:
				break;
			}
		}
	}
	
	str = block->str;
	g_string_free (block, false);
	
	return str;
}

bool
TextBlock::SetTextInternal (const char *text)
{
	InlineCollection *curInlines = GetInlines ();
	InlineCollection *inlines = NULL;
	char *inptr, *buf, *d;
	const char *txt;
	Inline *run;
	
	if (text && text[0]) {
		inlines = new InlineCollection ();
		
		d = buf = (char *) g_malloc (strlen (text) + 1);
		txt = text;
		
		// canonicalize line endings
		while (*txt) {
			if (*txt == '\r') {
				if (txt[1] == '\n')
					txt++;
				*d++ = '\n';
			} else {
				*d++ = *txt;
			}
			
			txt++;
		}
		*d = '\n';
		
		inptr = buf;
		while (inptr < d) {
			txt = inptr;
			while (*inptr != '\n')
				inptr++;
			
			if (inptr > txt) {
				*inptr = '\0';
				run = new Run ();
				run->SetAutogenerated (true);
				run->SetValue (Run::TextProperty, Value (txt));
				inlines->Add (run);
				run->unref ();
			}
			
			if (inptr < d) {
				run = new LineBreak ();
				run->SetAutogenerated (true);
				inlines->Add (run);
				run->unref ();
				inptr++;
			}
		}
		
		g_free (buf);
		
		if (curInlines && inlines->Equals (curInlines)) {
			// old/new inlines are equal, don't set the new value
			inlines->unref ();
			return false;
		}
		
		setvalue = false;
		SetValue (TextBlock::InlinesProperty, Value (inlines));
		setvalue = true;
		
		inlines->unref ();
	} else if (curInlines) {
		curInlines->Clear ();
	}
	
	return true;
}

void
TextBlock::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	bool invalidate = true;
	
	if (args->property->GetOwnerType () != Type::TEXTBLOCK) {
		FrameworkElement::OnPropertyChanged (args);
		if (args->property == FrameworkElement::WidthProperty) {
			if (layout->GetTextWrapping () != TextWrappingNoWrap)
				dirty = true;
			
			UpdateBounds (true);
		}
		
		return;
	}
	
	if (args->property == TextBlock::FontFamilyProperty) {
		FontFamily *family = args->new_value ? args->new_value->AsFontFamily () : NULL;
		font->SetFamily (family ? family->source : NULL);
		dirty = true;
	} else if (args->property == TextBlock::FontSizeProperty) {
		double size = args->new_value->AsDouble ();
		font->SetSize (size);
		dirty = true;
	} else if (args->property == TextBlock::FontStretchProperty) {
		FontStretches stretch = (FontStretches) args->new_value->AsInt32 ();
		font->SetStretch (stretch);
		dirty = true;
	} else if (args->property == TextBlock::FontStyleProperty) {
		FontStyles style = (FontStyles) args->new_value->AsInt32 ();
		font->SetStyle (style);
		dirty = true;
	} else if (args->property == TextBlock::FontWeightProperty) {
		FontWeights weight = (FontWeights) args->new_value->AsInt32 ();
		font->SetWeight (weight);
		dirty = true;
	} else if (args->property == TextBlock::TextProperty) {
		if (setvalue) {
			// result of a change to the TextBlock.Text property
			char *text = args->new_value ? args->new_value->AsString () : NULL;
			
			if (!SetTextInternal (text)) {
				// no change so nothing to invalidate
				invalidate = false;
			} else {
				dirty = true;
			}
		} else {
			// result of a change to the TextBlock.Inlines property
			invalidate = false;
		}
	} else if (args->property == TextBlock::TextDecorationsProperty) {
		dirty = true;
	} else if (args->property == TextBlock::TextWrappingProperty) {
		dirty = layout->SetTextWrapping ((TextWrapping) args->new_value->AsInt32 ());
	} else if (args->property == TextBlock::InlinesProperty) {
		if (setvalue) {
			// result of a change to the TextBlock.Inlines property
			setvalue = false;
			SetValue (TextBlock::TextProperty, Value (GetTextInternal (), true));
			setvalue = true;
			dirty = true;
		} else {
			// result of a change to the TextBlock.Text property
			invalidate = false;
		}
	} else if (args->property == TextBlock::LineStackingStrategyProperty) {
		dirty = layout->SetLineStackingStrategy ((LineStackingStrategy) args->new_value->AsInt32 ());
	} else if (args->property == TextBlock::LineHeightProperty) {
		dirty = layout->SetLineHeight (args->new_value->AsDouble ());
	} else if (args->property == TextBlock::TextAlignmentProperty) {
		dirty = layout->SetTextAlignment ((TextAlignment) args->new_value->AsInt32 ());
	} else if (args->property == TextBlock::PaddingProperty) {
		dirty = true;
	}
	
	if (invalidate) {
		if (dirty)
			UpdateBounds (true);
		
		Invalidate ();
	}
	
	NotifyListenersOfPropertyChange (args);
}

void
TextBlock::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == TextBlock::ForegroundProperty) {
		Invalidate ();
	} else {
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
	}
}

void
TextBlock::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	bool update_bounds = false;
	bool update_text = false;
	
	if (col != GetInlines ()) {
		FrameworkElement::OnCollectionChanged (col, args);
		return;
	}
	
	switch (args->action) {
	case CollectionChangedActionAdd:
	case CollectionChangedActionRemove:
	case CollectionChangedActionReplace:
		// an Inline element has been added or removed, update our TextProperty
		update_bounds = true;
		update_text = true;
		dirty = true;
		break;
	case CollectionChangedActionCleared:
		// the collection has changed, only update our TextProperty if it was the result of a SetValue
		update_bounds = setvalue;
		update_text = setvalue;
		dirty = true;
		break;
	default:
		break;
	}
	
	if (update_text) {
		setvalue = false;
		SetValue (TextBlock::TextProperty, Value (GetTextInternal (), true));
		setvalue = true;
	}
	
	if (update_bounds)
		UpdateBounds (true);
	
	Invalidate ();
}

void
TextBlock::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col != GetInlines ()) {
		FrameworkElement::OnCollectionItemChanged (col, obj, args);
		return;
	}
	
	if (args->property != Inline::ForegroundProperty) {
		if (args->property == Run::TextProperty) {
			// update our TextProperty
			setvalue = false;
			SetValue (TextBlock::TextProperty, Value (GetTextInternal (), true));
			setvalue = true;
		}
		
		// All non-Foreground property changes require
		// recalculating layout which can change the bounds.
		UpdateBounds (true);
		dirty = true;
	} else {
		// A simple Foreground brush change does not require
		// recalculating layout. Invalidate() and we're done.
	}
	
	Invalidate ();
}

void
TextBlock::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((TextBlock *) closure)->DownloaderComplete ();
}

static const char *
deobfuscate_font (Downloader *downloader, const char *path)
{
	char *filename, guid[16];
	const char *str;
	GString *name;
	Uri *uri;
	int fd;
	
	if (!(str = downloader->GetUri ()))
		return NULL;
	
	uri = new Uri ();
	if (!uri->Parse (str) || !uri->path) {
		delete uri;
		return NULL;
	}
	
	if (!(str = strrchr (uri->path, '/')))
		str = uri->path;
	else
		str++;
	
	if (!DecodeObfuscatedFontGUID (str, guid)) {
		delete uri;
		return NULL;
	}
	
	name = g_string_new (str);
	g_string_append (name, ".XXXXXX");
	delete uri;
	
	filename = g_build_filename (g_get_tmp_dir (), name->str, NULL);
	g_string_free (name, true);
	
	if ((fd = g_mkstemp (filename)) == -1) {
		g_free (filename);
		return NULL;
	}
	
	if (CopyFileTo (path, fd) == -1 || !DeobfuscateFontFileWithGUID (filename, guid, NULL)) {
		unlink (filename);
		g_free (filename);
		return NULL;
	}
	
	downloader->getFileDownloader ()->SetDeobfuscatedFile (filename);
	g_free (filename);
	
	return downloader->getFileDownloader ()->GetDownloadedFile ();
}

void
TextBlock::DownloaderComplete ()
{
	const char *filename, *path;
	struct stat st;
	
	/* the download was aborted */
	if (!(path = downloader->getFileDownloader ()->GetUnzippedPath ()))
		return;
	
	if (stat (path, &st) == -1)
		return;
	
	// check for obfuscated fonts
	if (S_ISREG (st.st_mode) && !downloader->getFileDownloader ()->IsDeobfuscated ()) {
		if ((filename = deobfuscate_font (downloader, path)))
			path = filename;
		
		downloader->getFileDownloader ()->SetDeobfuscated (true);
	}
	
	font->SetFilename (path);
	SetValue (TextBlock::FontFilenameProperty, path);
	dirty = true;
	
	UpdateBounds (true);
	Invalidate ();
}

//
// Glyphs
//

enum GlyphAttrMask {
	Cluster = 1 << 1,
	Index   = 1 << 2,
	Advance = 1 << 3,
	uOffset = 1 << 4,
	vOffset = 1 << 5,
};

class GlyphAttr : public List::Node {
public:
	guint32 glyph_count;
	guint32 code_units;
	guint32 index;
	double advance;
	double uoffset;
	double voffset;
	guint8 set;
	
	GlyphAttr ();
};

GlyphAttr::GlyphAttr ()
{
	glyph_count = 1;
	code_units = 1;
	set = 0;
}

Glyphs::Glyphs ()
{
	SetObjectType (Type::GLYPHS);

	desc = new TextFontDescription ();
	desc->SetSize (0.0);
	downloader = NULL;
	
	fill = NULL;
	path = NULL;
	
	attrs = new List ();
	text = NULL;
	index = 0;
	
	origin_y_specified = false;
	origin_x = 0.0;
	origin_y = 0.0;
	
	height = 0.0;
	width = 0.0;
	left = 0.0;
	top = 0.0;
	
	simulation_none	= true;
	uri_changed = false;
	invalid = false;
	dirty = false;
}

Glyphs::~Glyphs ()
{
	if (downloader) {
		downloader_abort (downloader);
		downloader->unref ();
	}
	
	if (path)
		moon_path_destroy (path);
	
	attrs->Clear (true);
	delete attrs;
	
	g_free (text);
	
	delete desc;
}

void
Glyphs::Layout ()
{
	guint32 code_units, glyph_count, i;
	bool first_char = true;
	double x0, x1, y0, y1;
	double bottom, right;
	double bottom0, top0;
	GlyphInfo *glyph;
	GlyphAttr *attr;
	int nglyphs = 0;
	TextFont *font;
	double offset;
	bool cluster;
	double scale;
	int n = 0;
	
	invalid = false;
	dirty = false;
	
	height = 0.0;
	width = 0.0;
	left = 0.0;
	top = 0.0;
	
	if (path) {
		moon_path_destroy (path);
		path = NULL;
	}
	
	// Silverlight only renders for None (other, invalid, values do not render anything)
	if (!simulation_none) {
		invalid = true;
		return;
	}
	
	if (!desc->GetFilename () || desc->GetSize () == 0.0) {
		// required font fields have not been set
		return;
	}
	
	if (((!text || !text[0]) && attrs->IsEmpty ())) {
		// no glyphs to render
		return;
	}
	
	if (fill == NULL) {
		// no fill specified (unlike TextBlock, there is no default brush)
		return;
	}
	
	font = desc->GetFont ();
	
	// scale Advance, uOffset and vOffset units to pixels
	scale = desc->GetSize () * 20.0 / 2048.0;
	
	right = origin_x;
	left = origin_x;
	x0 = origin_x;
	
	// OriginY is the baseline if specified
	if (origin_y_specified) {
		top0 = origin_y - font->Ascender ();
		y0 = origin_y;
	} else {
		y0 = font->Ascender ();
		top0 = 0.0;
	}
	
	bottom0 = top0 + font->Height ();
	
	bottom = bottom0;
	top = top0;
	
	path = moon_path_new (16);
	
	attr = (GlyphAttr *) attrs->First ();
	
	if (text && text[0]) {
		gunichar *c = text;
		
		while (*c != 0) {
			if (attr && (attr->set & Cluster)) {
				// get the cluster's GlyphCount and CodeUnitCount
				glyph_count = attr->glyph_count;
				code_units = attr->code_units;
			} else {
				glyph_count = 1;
				code_units = 1;
			}
			
			if (glyph_count == 1 && code_units == 1)
				cluster = false;
			else
				cluster = true;
			
			// render the glyph cluster
			i = 0;
			do {
				if (attr && (attr->set & Index)) {
					if (!(glyph = font->GetGlyphInfoByIndex (attr->index)))
						goto next1;
				} else if (cluster) {
					// indexes MUST be specified for each glyph in a cluster
					moon_path_destroy (path);
					invalid = true;
					path = NULL;
					goto done;
				} else {
					glyph = font->GetGlyphInfo (*c);
					if (!glyph)
						goto next1;
				}
				
				y1 = y0;
				if (attr && (attr->set & vOffset)) {
					offset = -(attr->voffset * scale);
					bottom = MAX (bottom, bottom0 + offset);
					top = MIN (top, top0 + offset);
					y1 += offset;
				} 
				
				if (attr && (attr->set & uOffset)) {
					offset = (attr->uoffset * scale);
					left = MIN (left, x0 + offset);
					x1 = x0 + offset;
				} else if (first_char) {
					if (glyph->metrics.horiBearingX < 0)
						x0 -= glyph->metrics.horiBearingX;
					
					first_char = false;
					x1 = x0;
				} else {
					x1 = x0;
				}
				
				right = MAX (right, x1 + glyph->metrics.horiAdvance);
				
				font->AppendPath (path, glyph, x1, y1);
				nglyphs++;
				
				if (attr && (attr->set & Advance))
					x0 += attr->advance * scale;
				else
					x0 += glyph->metrics.horiAdvance;
				
			next1:
				
				attr = attr ? (GlyphAttr *) attr->next : NULL;
				i++;
				
				if (i == glyph_count)
					break;
				
				if (!attr) {
					// there MUST be an attr for each glyph in a cluster
					moon_path_destroy (path);
					invalid = true;
					path = NULL;
					goto done;
				}
				
				if ((attr->set & Cluster)) {
					// only the first glyph in a cluster may specify a cluster mapping
					moon_path_destroy (path);
					invalid = true;
					path = NULL;
					goto done;
				}
			} while (true);
			
			// consume the code units
			for (i = 0; i < code_units && *c != 0; i++)
				c++;
			
			n++;
		}
	}
	
	while (attr) {
		if (attr->set & Cluster) {
			LOG_TEXT (stderr, "Can't use clusters past the end of the UnicodeString\n");
			moon_path_destroy (path);
			invalid = true;
			path = NULL;
			goto done;
		}
		
		if (!(attr->set & Index)) {
			LOG_TEXT (stderr, "No index specified for glyph %d\n", n + 1);
			moon_path_destroy (path);
			invalid = true;
			path = NULL;
			goto done;
		}
		
		if (!(glyph = font->GetGlyphInfoByIndex (attr->index)))
			goto next;
		
		y1 = y0;
		if ((attr->set & vOffset)) {
			offset = -(attr->voffset * scale);
			bottom = MAX (bottom, bottom0 + offset);
			top = MIN (top, top0 + offset);
			y1 += offset;
		}
		
		if ((attr->set & uOffset)) {
			offset = (attr->uoffset * scale);
			left = MIN (left, x0 + offset);
			x1 = x0 + offset;
		} else if (first_char) {
			if (glyph->metrics.horiBearingX < 0)
				x0 -= glyph->metrics.horiBearingX;
			
			first_char = false;
			x1 = x0;
		} else {
			x1 = x0;
		}
		
		right = MAX (right, x1 + glyph->metrics.horiAdvance);
		
		font->AppendPath (path, glyph, x1, y1);
		nglyphs++;
		
		if ((attr->set & Advance))
			x0 += attr->advance * scale;
		else
			x0 += glyph->metrics.horiAdvance;
		
	next:
		
		attr = (GlyphAttr *) attr->next;
		n++;
	}
	
	if (nglyphs > 0) {
		height = bottom - top;
		width = right - left;
	} else {
		moon_path_destroy (path);
		path = NULL;
	}
	
done:
	
	font->unref ();
}

void
Glyphs::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	if (dirty)
		Layout ();
	
	*height = this->height;
	*width = this->width;
}

Point
Glyphs::GetOriginPoint () 
{
	if (origin_y_specified) {
		TextFont *font = desc->GetFont ();
		
		double d = font->Descender ();
		double h = font->Height ();
		
		font->unref ();
		
		return Point (origin_x, origin_y - d - h);
	} else {
		return Point (origin_x, 0);
	}
}

void
Glyphs::Render (cairo_t *cr, Region *region)
{
	if (width == 0.0 && height == 0.0)
		return;
	
	if (invalid) {
		// do not render anything if our state is invalid to keep with Silverlight's behavior.
		// (Note: rendering code also assumes everything is kosher)
		return;
	}
	
	if (path == NULL || path->cairo.num_data == 0) {
		// No glyphs to render
		return;
	}
	
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	
	Point p = GetOriginPoint ();
	Rect area = Rect (p.x, p.y, 0, 0);
	GetSizeForBrush (cr, &(area.width), &(area.height));
	fill->SetupBrush (cr, area);
	
	cairo_append_path (cr, &path->cairo);
	fill->Fill (cr);
	
	cairo_restore (cr);
}

void 
Glyphs::ComputeBounds ()
{
	if (dirty)
		Layout ();
	
	bounds = IntersectBoundsWithClipPath (Rect (left, top, width, height), false).Transform (&absolute_xform);
}

bool
Glyphs::InsideObject (cairo_t *cr, double x, double y)
{
	double nx = x;
	double ny = y;

	TransformPoint (&nx, &ny);
	
	return (nx >= left && ny >= top && nx < left + width && ny < top + height);
}

Point
Glyphs::GetTransformOrigin ()
{
	// Glyphs seems to always use 0,0 no matter what is specified in the RenderTransformOrigin nor the OriginX/Y points
	return Point (0.0, 0.0);
}

void
Glyphs::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == Glyphs::FillProperty) {
		Invalidate ();
	} else {
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
	}
}

void
Glyphs::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((Glyphs *) closure)->DownloaderComplete ();
}

void
Glyphs::DownloaderComplete ()
{
	const char *filename, *path;
	struct stat st;
	
	/* the download was aborted */
	if (!(filename = downloader->getFileDownloader ()->GetDownloadedFile ()))
		return;
	
	if (stat (filename, &st) == -1 || !S_ISREG (st.st_mode))
		return;
	
	if (!downloader->getFileDownloader ()->IsDeobfuscated ()) {
		if ((path = deobfuscate_font (downloader, filename)))
			filename = path;
		
		downloader->getFileDownloader ()->SetDeobfuscated (true);
	}
	
	desc->SetFilename (filename);
	desc->SetIndex (index);
	dirty = true;
	
	UpdateBounds (true);
	Invalidate ();
}

static void
print_parse_error (const char *in, const char *where, const char *reason)
{
#if DEBUG
	if (debug_flags & RUNTIME_DEBUG_TEXT) {
		int i;
	
		fprintf (stderr, "Glyph Indices parse error: \"%s\": %s\n", in, reason);
		fprintf (stderr, "                            ");
		for (i = 0; i < (where - in); i++)
			fputc (' ', stderr);
		fprintf (stderr, "^\n");
	}
#endif
}

void
Glyphs::SetIndicesInternal (const char *in)
{
	register const char *inptr = in;
	GlyphAttr *glyph;
	double value;
	char *end;
	uint bit;
	int n;
	
	attrs->Clear (true);
	
	if (in == NULL)
		return;
	
	while (g_ascii_isspace (*inptr))
		inptr++;
	
	while (*inptr) {
		glyph = new GlyphAttr ();
		
		while (g_ascii_isspace (*inptr))
			inptr++;
		
		// check for a cluster
		if (*inptr == '(') {
			inptr++;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			errno = 0;
			glyph->code_units = strtoul (inptr, &end, 10);
			if (glyph->code_units == 0 || (glyph->code_units == LONG_MAX && errno != 0)) {
				// invalid cluster
				print_parse_error (in, inptr, errno ? strerror (errno) : "invalid cluster mapping; CodeUnitCount cannot be 0");
				delete glyph;
				return;
			}
			
			inptr = end;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			if (*inptr != ':') {
				// invalid cluster
				print_parse_error (in, inptr, "expected ':'");
				delete glyph;
				return;
			}
			
			inptr++;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			errno = 0;
			glyph->glyph_count = strtoul (inptr, &end, 10);
			if (glyph->glyph_count == 0 || (glyph->glyph_count == LONG_MAX && errno != 0)) {
				// invalid cluster
				print_parse_error (in, inptr, errno ? strerror (errno) : "invalid cluster mapping; GlyphCount cannot be 0");
				delete glyph;
				return;
			}
			
			inptr = end;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			if (*inptr != ')') {
				// invalid cluster
				print_parse_error (in, inptr, "expected ')'");
				delete glyph;
				return;
			}
			
			glyph->set |= Cluster;
			inptr++;
			
			while (g_ascii_isspace (*inptr))
				inptr++;
		}
		
		if (*inptr >= '0' && *inptr <= '9') {
			errno = 0;
			glyph->index = strtoul (inptr, &end, 10);
			if ((glyph->index == 0 || glyph->index == LONG_MAX) && errno != 0) {
				// invalid glyph index
				print_parse_error (in, inptr, strerror (errno));
				delete glyph;
				return;
			}
			
			glyph->set |= Index;
			
			inptr = end;
			while (g_ascii_isspace (*inptr))
				inptr++;
		}
		
		bit = (uint) Advance;
		n = 0;
		
		while (*inptr == ',' && n < 3) {
			inptr++;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			if (*inptr != ',') {
				value = g_ascii_strtod (inptr, &end);
				if ((value == 0.0 || value == HUGE_VAL || value == -HUGE_VAL) && errno != 0) {
					// invalid advance or offset
					print_parse_error (in, inptr, strerror (errno));
					delete glyph;
					return;
				}
			} else {
				end = (char *) inptr;
			}
			
			if (end > inptr) {
				switch ((GlyphAttrMask) bit) {
				case Advance:
					glyph->advance = value;
					glyph->set |= Advance;
					break;
				case uOffset:
					glyph->uoffset = value;
					glyph->set |= uOffset;
					break;
				case vOffset:
					glyph->voffset = value;
					glyph->set |= vOffset;
					break;
				default:
					break;
				}
			}
			
			inptr = end;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			bit <<= 1;
			n++;
		}
		
		attrs->Append (glyph);
		
		while (g_ascii_isspace (*inptr))
			inptr++;
		
		if (*inptr && *inptr != ';') {
			print_parse_error (in, inptr, "expected ';'");
			return;
		}
		
		if (*inptr == '\0')
			break;
		
		inptr++;
	}
}

void
Glyphs::DownloadFont (Surface *surface, const char *url)
{
	Uri *uri = new Uri ();
	char *str;
	
	if (uri->Parse (url)) {
		if ((downloader = surface->CreateDownloader ())) {
			if (uri->fragment) {
				if ((index = strtol (uri->fragment, NULL, 10)) < 0 || index == LONG_MAX)
					index = 0;
			}
			
			str = uri->ToString (UriHideFragment);
			downloader->Open ("GET", str, XamlPolicy);
			g_free (str);
			
			downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);
			if (downloader->Started () || downloader->Completed ()) {
				if (downloader->Completed ())
					DownloaderComplete ();
			} else {
				// This is what actually triggers the download
				downloader->Send ();
			}
		} else {
			// we're shutting down
		}
	}
	
	delete uri;
}

void
Glyphs::SetSurface (Surface *surface)
{
	if (GetSurface() == surface)
		return;

	const char *uri;
	
	FrameworkElement::SetSurface (surface);
	
	if (!uri_changed || !surface)
		return;
	
	if ((uri = GetFontUri ()) && *uri)
		DownloadFont (surface, uri);
	
	uri_changed = false;
}

void
Glyphs::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	bool invalidate = true;
	
	if (args->property->GetOwnerType() != Type::GLYPHS) {
		FrameworkElement::OnPropertyChanged (args);
		return;
	}
	
	if (args->property == Glyphs::FontUriProperty) {
		const char *str = args->new_value ? args->new_value->AsString () : NULL;
		Surface *surface = GetSurface ();
		
		if (downloader) {
			downloader->Abort ();
			downloader->unref ();
			downloader = NULL;
			index = 0;
		}
		
		if (surface) {
			if (str && *str)
				DownloadFont (surface, str);
			uri_changed = false;
		} else {
			uri_changed = true;
		}
		
		invalidate = false;
	} else if (args->property == Glyphs::FillProperty) {
		fill = args->new_value ? args->new_value->AsBrush() : NULL;
	} else if (args->property == Glyphs::UnicodeStringProperty) {
		const char *str = args->new_value ? args->new_value->AsString () : NULL;
		g_free (text);
		
		if (str != NULL)
			text = g_utf8_to_ucs4_fast (str, -1, NULL);
		else
			text = NULL;
		
		dirty = true;
	} else if (args->property == Glyphs::IndicesProperty) {
		const char *str = args->new_value ? args->new_value->AsString () : NULL;
		SetIndicesInternal (str);
		dirty = true;
	} else if (args->property == Glyphs::FontRenderingEmSizeProperty) {
		double size = args->new_value->AsDouble();
		desc->SetSize (size);
		dirty = true;
	} else if (args->property == Glyphs::OriginXProperty) {
		origin_x = args->new_value->AsDouble ();
		dirty = true;
	} else if (args->property == Glyphs::OriginYProperty) {
		origin_y = args->new_value->AsDouble ();
		origin_y_specified = true;
		dirty = true;
	} else if (args->property == Glyphs::StyleSimulationsProperty) {
		// Silverlight 1.0 does not implement this property but, if present, 
		// requires it to be 0 (or else nothing is displayed)
		bool none = (args->new_value->AsInt32 () == StyleSimulationsNone);
		dirty = (none != simulation_none);
		simulation_none = none;
	}
	
	if (invalidate)
		Invalidate ();
	
	if (dirty)
		UpdateBounds (true);
	
	NotifyListenersOfPropertyChange (args);
}
