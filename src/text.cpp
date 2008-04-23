/*
 * text.cpp: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <cairo.h>

#include <stdlib.h>
#include <string.h>

#include "runtime.h"
#include "color.h"
#include "text.h"
#include "uri.h"


#define d(x) x


#define TEXTBLOCK_FONT_FAMILY  "Portable User Interface"
#define TEXTBLOCK_FONT_STRETCH FontStretchesNormal
#define TEXTBLOCK_FONT_WEIGHT  FontWeightsNormal
#define TEXTBLOCK_FONT_STYLE   FontStylesNormal
#define TEXTBLOCK_FONT_SIZE    14.666666984558105


static SolidColorBrush *default_foreground_brush = NULL;

static Brush *
default_foreground (void)
{
	if (!default_foreground_brush)
		default_foreground_brush = new SolidColorBrush ("black");
	
	return (Brush *) default_foreground_brush;
}


// Inline

DependencyProperty *Inline::FontFamilyProperty;
DependencyProperty *Inline::FontSizeProperty;
DependencyProperty *Inline::FontStretchProperty;
DependencyProperty *Inline::FontStyleProperty;
DependencyProperty *Inline::FontWeightProperty;
DependencyProperty *Inline::ForegroundProperty;
DependencyProperty *Inline::TextDecorationsProperty;

Inline::Inline ()
{
	foreground = NULL;
	autogen = false;
	
	/* initialize the font description */
	font = new TextFontDescription ();
}

Inline::~Inline ()
{
	delete font;
}

static DependencyProperty *
textblock_property (DependencyProperty *prop)
{
	if (prop == Inline::FontFamilyProperty)
		return TextBlock::FontFamilyProperty;
	
	if (prop == Inline::FontStretchProperty)
		return TextBlock::FontStretchProperty;
	
	if (prop == Inline::FontWeightProperty)
		return TextBlock::FontWeightProperty;
	
	if (prop == Inline::FontStyleProperty)
		return TextBlock::FontStyleProperty;
	
	if (prop == Inline::FontSizeProperty)
		return TextBlock::FontSizeProperty;
	
	if (prop == Inline::ForegroundProperty)
		return TextBlock::ForegroundProperty;
	
	if (prop == Inline::TextDecorationsProperty)
		return TextBlock::TextDecorationsProperty;
	
	return NULL;
}

Value *
Inline::GetDefaultValue (DependencyProperty *prop)
{
	DependencyObject *parent = GetLogicalParent ();
	
	if (parent && parent->Is (Type::TEXTBLOCK)) {
		DependencyProperty *text_prop = textblock_property (prop);
		
		if (text_prop)
			return parent->GetValue (text_prop);
		
		return prop->default_value;
	}
	
	// not yet attached to a textblock
	
	if (prop == Inline::ForegroundProperty) {
		SolidColorBrush *brush = new SolidColorBrush ("black");
		
		SetValue (prop, Value (brush));
		brush->unref ();
		
		return GetValue (prop);
	}
	
	// all other properties have a default value
	return prop->default_value;
}

void
Inline::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::INLINE) {
		DependencyObject::OnPropertyChanged (args);
		return;
	}
	
	if (args->property == Inline::FontFamilyProperty) {
		if (args->new_value) {
			char *family = args->new_value->AsString ();
			font->SetFamily (family);
		} else {
			font->UnsetFields (FontMaskFamily);
		}
	} else if (args->property == Inline::FontSizeProperty) {
		if (args->new_value) {
			double size = args->new_value->AsDouble ();
			font->SetSize (size);
		} else {
			font->UnsetFields (FontMaskSize);
		}
	} else if (args->property == Inline::FontStretchProperty) {
		if (args->new_value) {
			FontStretches stretch = (FontStretches) args->new_value->AsInt32 ();
			font->SetStretch (stretch);
		} else {
			font->UnsetFields (FontMaskStretch);
		}
	} else if (args->property == Inline::FontStyleProperty) {
		if (args->new_value) {
			FontStyles style = (FontStyles) args->new_value->AsInt32 ();
			font->SetStyle (style);
		} else {
			font->UnsetFields (FontMaskStyle);
		}
	} else if (args->property == Inline::FontWeightProperty) {
		if (args->new_value) {
			FontWeights weight = (FontWeights) args->new_value->AsInt32 ();
			font->SetWeight (weight);
		} else {
			font->UnsetFields (FontMaskWeight);
		}
	} else if (args->property == Inline::ForegroundProperty) {
		foreground = args->new_value ? args->new_value->AsBrush () : NULL;
	}
	
	
	NotifyListenersOfPropertyChange (args);
}

void
Inline::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == Inline::ForegroundProperty) {
		// this isn't exactly what we want, I don't
		// think... but it'll have to do.
		NotifyListenersOfPropertyChange (prop);
	}
}

const char *
inline_get_font_family (Inline *inline_)
{
	Value *value = inline_->GetValue (Inline::FontFamilyProperty);
	
	return value ? value->AsString () : NULL;
}

void
inline_set_font_family (Inline *inline_, const char *value)
{
	inline_->SetValue (Inline::FontFamilyProperty, Value (value));
}

double
inline_get_font_size (Inline *inline_)
{
	return inline_->GetValue (Inline::FontSizeProperty)->AsDouble ();
}

void
inline_set_font_size (Inline *inline_, double value)
{
	inline_->SetValue (Inline::FontSizeProperty, Value (value));
}

FontStretches
inline_get_font_stretch (Inline *inline_)
{
	return (FontStretches) inline_->GetValue (Inline::FontStretchProperty)->AsInt32 ();
}

void
inline_set_font_stretch (Inline *inline_, FontStretches value)
{
	inline_->SetValue (Inline::FontStretchProperty, Value (value));
}

FontStyles
inline_get_font_style (Inline *inline_)
{
	return (FontStyles) inline_->GetValue (Inline::FontStyleProperty)->AsInt32 ();
}

void
inline_set_font_style (Inline *inline_, FontStyles value)
{
	inline_->SetValue (Inline::FontStyleProperty, Value (value));
}

FontWeights
inline_get_font_weight (Inline *inline_)
{
	return (FontWeights) inline_->GetValue (Inline::FontWeightProperty)->AsInt32 ();
}

void
inline_set_font_weight (Inline *inline_, FontWeights value)
{
	inline_->SetValue (Inline::FontWeightProperty, Value (value));
}

Brush *
inline_get_foreground (Inline *inline_)
{
	Value *value = inline_->GetValue (Inline::ForegroundProperty);
	
	return value ? value->AsBrush () : NULL;
}

void
inline_set_foreground (Inline *inline_, Brush *value)
{
	inline_->SetValue (Inline::ForegroundProperty, Value (value));
}

TextDecorations
inline_get_text_decorations (Inline *inline_)
{
	return (TextDecorations) inline_->GetValue (Inline::TextDecorationsProperty)->AsInt32 ();
}

void
inline_set_text_decorations (Inline *inline_, TextDecorations value)
{
	inline_->SetValue (Inline::TextDecorationsProperty, Value (value));
}


// LineBreak

LineBreak *
line_break_new (void)
{
	return new LineBreak ();
}


// Run

DependencyProperty *Run::TextProperty;

void
Run::SetText (const char *text)
{
	SetValue (Run::TextProperty, Value (text));
}

const char *
Run::GetText ()
{
	Value *value = GetValue (Run::TextProperty);
	
	return value ? value->AsString () : NULL;
}


Run *
run_new (void)
{
	return new Run ();
}

const char *
run_get_text (Run *run)
{
	return run->GetText ();
}

void
run_set_text (Run *run, const char *text)
{
	run->SetText (text);
}


// TextBlock

DependencyProperty *TextBlock::ActualHeightProperty;
DependencyProperty *TextBlock::ActualWidthProperty;
DependencyProperty *TextBlock::FontFamilyProperty;
DependencyProperty *TextBlock::FontSizeProperty;
DependencyProperty *TextBlock::FontStretchProperty;
DependencyProperty *TextBlock::FontStyleProperty;
DependencyProperty *TextBlock::FontWeightProperty;
DependencyProperty *TextBlock::ForegroundProperty;
DependencyProperty *TextBlock::InlinesProperty;
DependencyProperty *TextBlock::TextProperty;
DependencyProperty *TextBlock::TextDecorationsProperty;
DependencyProperty *TextBlock::TextWrappingProperty;


TextBlock::TextBlock ()
{
	downloader = NULL;
	
	setvalue = true;
	dirty = true;
	
	actual_height = 0.0;
	actual_width = 0.0;
	bbox_height = 0.0;
	bbox_width = 0.0;
	
	/* initialize the font description and layout */
	layout = new TextLayout ();
	
	font = new TextFontDescription ();
	font->SetFamily (TEXTBLOCK_FONT_FAMILY);
	font->SetStretch (TEXTBLOCK_FONT_STRETCH);
	font->SetWeight (TEXTBLOCK_FONT_WEIGHT);
	font->SetStyle (TEXTBLOCK_FONT_STYLE);
	font->SetSize (TEXTBLOCK_FONT_SIZE);
	
	Brush *brush = new SolidColorBrush ("black");
	
	SetValue (TextBlock::ForegroundProperty, Value (brush));
	SetValue (TextBlock::InlinesProperty, Value::CreateUnref (new Inlines ()));
	brush->unref();
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
			downloader->SetWriteFunc (data_write, size_notify, this);
			
			// This is what actually triggers the download
			downloader->Send ();
		}
	} else {
		font->SetFilename (NULL);
		dirty = true;
		
		UpdateBounds (true);
		Invalidate ();
	}
}

void
TextBlock::Render (cairo_t *cr, int x, int y, int width, int height)
{
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	Paint (cr);
	cairo_restore (cr);
}

void 
TextBlock::ComputeBounds ()
{
	bounds = IntersectBoundsWithClipPath (Rect (0, 0, GetBoundingWidth (), GetBoundingHeight ()), false).Transform (&absolute_xform);
}

bool
TextBlock::InsideObject (cairo_t *cr, double x, double y)
{
	bool ret = false;
	
	cairo_save (cr);
	
	double nx = x;
	double ny = y;

	uielement_transform_point (this, &nx, &ny);
	
	if (nx >= 0.0 && ny >= 0.0 && nx < GetBoundingWidth () && ny < GetBoundingHeight ())
		ret = true;
	
	cairo_restore (cr);
	return ret;
}

Point
TextBlock::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	
	return Point (user_xform_origin.x * GetBoundingWidth (), user_xform_origin.y * GetBoundingHeight ());
}

void
TextBlock::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	*height = GetActualHeight ();
	*width = GetActualWidth ();
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

void
TextBlock::Layout (cairo_t *cr)
{
	TextDecorations decorations;
	double height, width;
	uint8_t font_mask;
	const char *text;
	List *runs;
	
	layout->SetWrapping (GetTextWrapping ());
	
	height = GetValue (FrameworkElement::HeightProperty)->AsDouble ();
	width = GetValue (FrameworkElement::WidthProperty)->AsDouble ();
	
	if (width > 0.0f)
		layout->SetMaxWidth (width);
	else
		layout->SetMaxWidth (-1.0);
	
	runs = new List ();
	
	decorations = GetTextDecorations ();
	font_mask = font->GetFields ();
	
	Inlines *inlines = GetInlines ();
	
	if (inlines != NULL) {
		Collection::Node *node = (Collection::Node *) inlines->list->First ();
		uint8_t run_mask, inherited_mask;
		TextFontDescription *ifont;
		TextDecorations deco;
		Value *value;
		Inline *item;
		Run *run;
		
		while (node != NULL) {
			item = (Inline *) node->obj;
			
			ifont = item->font;
			
			// Inlines inherit their parent TextBlock's font properties if
			// they don't specify their own.
			run_mask = ifont->GetFields ();
			ifont->Merge (font, false);
			
			inherited_mask = (FontMask) (font_mask & ~run_mask);
			
			// Inherit the TextDecorations from the parent TextBlock if unset
			if ((value = item->GetValue (Inline::TextDecorationsProperty)))
				deco = (TextDecorations) value->AsInt32 ();
			else
				deco = decorations;
			
 			switch (item->GetObjectType ()) {
			case Type::RUN:
				run = (Run *) item;
				
				text = run->GetText ();
				
				if (text && text[0]) {
					const char *inptr, *inend;
					
					inptr = text;
					
					do {
						inend = inptr;
						while (*inend && *inend != '\n')
							inend++;
						
						if (inend > inptr)
							runs->Append (new TextRun (inptr, inend - inptr, deco,
										   ifont, &item->foreground));
						
						if (*inend == '\0')
							break;
						
						runs->Append (new TextRun (ifont));
						inptr = inend + 1;
					} while (*inptr);
				}
				
				break;
			case Type::LINEBREAK:
				runs->Append (new TextRun (ifont));
				break;
			default:
				break;
			}
			
			if (inherited_mask != 0)
				ifont->UnsetFields (inherited_mask);
			
			node = (Collection::Node *) node->next;
		}
	}
	
	layout->SetTextRuns (runs);
	layout->Layout ();
	
	layout->GetActualExtents (&actual_width, &actual_height);
	layout->GetLayoutExtents (&bbox_width, &bbox_height);
	
	SetActualHeight (actual_height);
	SetActualWidth (actual_width);
	
	dirty = false;
}

void
TextBlock::Paint (cairo_t *cr)
{
	Brush *fg;
	
	if (!(fg = GetForeground ()))
		fg = default_foreground ();
	
	layout->Render (cr, this, fg, 0.0, 0.0);
}

char *
TextBlock::GetTextInternal ()
{
	Inlines *inlines = GetInlines ();
	GString *block;
	char *str;
	
	block = g_string_new ("");
	
	if (inlines != NULL) {
		Collection::Node *node = (Collection::Node *) inlines->list->First ();
		const char *text;
		Inline *item;
		
		while (node != NULL) {
			item = (Inline *) node->obj;
			
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
			
			node = (Collection::Node *) node->next;
		}
	}
	
	str = block->str;
	g_string_free (block, false);
	
	return str;
}

static bool
inlines_simple_text_equal (Inlines *curInlines, Inlines *newInlines)
{
	Collection::Node *node1, *node2;
	const char *text1, *text2;
	Inline *run1, *run2;
	
	node1 = (Collection::Node *) curInlines->list->First ();
	node2 = (Collection::Node *) newInlines->list->First ();
	
	while (node1 && node2) {
		run1 = (Inline *) node1->obj;
		run2 = (Inline *) node2->obj;
		
		if (run1->GetObjectType () != run2->GetObjectType ())
			return false;
		
		if (run1->GetObjectType () == Type::RUN) {
			text1 = run1->GetText ();
			text2 = run2->GetText ();
			
			if (text1 && text2 && strcmp (text1, text2) != 0)
				return false;
			else if ((text1 && !text2) || (!text1 && text2))
				return false;
		}
		
		// newInlines uses TextBlock font/brush properties, so
		// if curInlines uses any non-default props then they
		// are not equal.
		
		if (run1->font->GetFields () != 0)
			return false;
		
		if (run1->GetValueNoDefault (Inline::TextDecorationsProperty) != NULL)
			return false;
		
		if (run1->foreground != NULL)
			return false;
		
		node1 = (Collection::Node *) node1->next;
		node2 = (Collection::Node *) node2->next;
	}
	
	if (node1 != NULL || node2 != NULL)
		return false;
	
	return true;
}

bool
TextBlock::SetTextInternal (const char *text)
{
	Inlines *curInlines = GetInlines ();
	Inlines *inlines = NULL;
	char *inptr, *buf, *d;
	const char *txt;
	Inline *run;
	
	if (text && text[0]) {
		inlines = new Inlines ();
		
		d = buf = (char *) g_malloc (strlen (text) + 1);
		txt = text;
		
		while (*txt) {
			if (*txt != '\r')
				*d++ = *txt;
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
				run->autogen = true;
				run->SetValue (Run::TextProperty, Value (txt));
				inlines->Add (run);
				run->unref ();
			}
			
			if (inptr < d) {
				run = new LineBreak ();
				run->autogen = true;
				inlines->Add (run);
				run->unref ();
				inptr++;
			}
		}
		
		g_free (buf);
		
		if (curInlines && inlines_simple_text_equal (curInlines, inlines)) {
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
	
	if (args->property->type != Type::TEXTBLOCK) {
		FrameworkElement::OnPropertyChanged (args);
		if (args->property == FrameworkElement::WidthProperty)
			UpdateBounds (true);
		
		return;
	}
	
	if (args->property == TextBlock::FontFamilyProperty) {
		char *family = args->new_value ? args->new_value->AsString () : NULL;
		font->SetFamily (family);
		
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
	} else if (args->property == TextBlock::InlinesProperty) {
		if (setvalue) {
			// result of a change to the TextBlock.Inlines property
			char *text = GetTextInternal ();
			
			setvalue = false;
			SetValue (TextBlock::TextProperty, Value (text));
			setvalue = true;
			g_free (text);
			dirty = true;
		} else {
			// result of a change to the TextBlock.Text property
			invalidate = false;
		}
	} else if (args->property == TextBlock::ActualHeightProperty) {
		invalidate = false;
	} else if (args->property == TextBlock::ActualWidthProperty) {
		invalidate = false;
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
	if (prop == TextBlock::ForegroundProperty)
		Invalidate ();
	else
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
TextBlock::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args)
{
	bool update_bounds = false;
	bool update_text = false;
	
	switch (type) {
	case CollectionChangeTypeItemAdded:
	case CollectionChangeTypeItemRemoved:
		// an Inline element has been added or removed, update our TextProperty
		update_bounds = true;
		update_text = true;
		dirty = true;
		break;
	case CollectionChangeTypeItemChanged:
		// only update bounds if a property other than the Foreground changed
		update_bounds = element_args->property != Inline::ForegroundProperty;
		
		// only update our TextProperty if change was in a Run's Text property
		update_text = element_args->property == Run::TextProperty;
		
		dirty = true;
		break;
	case CollectionChangeTypeChanged:
		// the collection has changed, only update our TextProperty if it was the result of a SetValue
		update_bounds = setvalue;
		update_text = setvalue;
		dirty = true;
		break;
	default:
		break;
	}
	
	if (update_text) {
		char *text = GetTextInternal ();
		
		setvalue = false;
		SetValue (TextBlock::TextProperty, Value (text));
		setvalue = true;
		g_free (text);
	}
	
	if (update_bounds)
		UpdateBounds (true);
	
	Invalidate ();
}

Value *
TextBlock::GetValue (DependencyProperty *property)
{
	if (dirty && ((property == TextBlock::ActualHeightProperty) || (property == TextBlock::ActualWidthProperty)))
		CalcActualWidthHeight (NULL);
	
	return DependencyObject::GetValue (property);
}

void
TextBlock::data_write (void *buf, int32_t offset, int32_t n, gpointer data)
{
	;
}

void
TextBlock::size_notify (int64_t size, gpointer data)
{
	;
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
	Value *value;
	Uri *uri;
	int fd;
	
	if (!(value = downloader->GetValue (Downloader::UriProperty)) || !(str = value->AsString ()))
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
	
	downloader->SetDeobfuscatedFile (filename);
	g_free (filename);
	
	return downloader->GetDownloadedFile ();
}

void
TextBlock::DownloaderComplete ()
{
	const char *filename, *path;
	struct stat st;
	
	/* the download was aborted */
	if (!(path = downloader->GetUnzippedPath ()))
		return;
	
	if (stat (path, &st) == -1)
		return;
	
	// check for obfuscated fonts
	if (S_ISREG (st.st_mode) && !downloader->IsDeobfuscated ()) {
		if ((filename = deobfuscate_font (downloader, path)))
			path = filename;
		
		downloader->SetDeobfuscated (true);
	}
	
	font->SetFilename (path);
	dirty = true;
	
	UpdateBounds (true);
	Invalidate ();
}

void
TextBlock::SetActualHeight (double height)
{
	SetValue (TextBlock::ActualHeightProperty, Value (height));
}

void
TextBlock::SetActualWidth (double width)
{
	SetValue (TextBlock::ActualWidthProperty, Value (width));
}

void
TextBlock::SetFontFamily (const char *family)
{
	SetValue (TextBlock::FontFamilyProperty, Value (family));
}

const char *
TextBlock::GetFontFamily ()
{
	return font->GetFamily ();
}

void
TextBlock::SetFontSize (double size)
{
	SetValue (TextBlock::FontSizeProperty, Value (size));
}

double
TextBlock::GetFontSize ()
{
	return font->GetSize ();
}

void
TextBlock::SetFontStretch (FontStretches stretch)
{
	SetValue (TextBlock::FontStretchProperty, Value (stretch));
}

FontStretches
TextBlock::GetFontStretch ()
{
	return font->GetStretch ();
}

void
TextBlock::SetFontStyle (FontStyles style)
{
	SetValue (TextBlock::FontStyleProperty, Value (style));
}

FontStyles
TextBlock::GetFontStyle ()
{
	return font->GetStyle ();
}

void
TextBlock::SetFontWeight (FontWeights weight)
{
	SetValue (TextBlock::FontWeightProperty, Value (weight));
}

FontWeights
TextBlock::GetFontWeight ()
{
	return font->GetWeight ();
}

void
TextBlock::SetForeground (Brush *foreground)
{
	SetValue (TextBlock::ForegroundProperty, Value (foreground));
}

Brush *
TextBlock::GetForeground ()
{
	Value *value = GetValue (TextBlock::ForegroundProperty);
	
	return value ? value->AsBrush () : NULL;
}

void
TextBlock::SetInlines (Inlines *inlines)
{
	SetValue (TextBlock::InlinesProperty, Value (inlines));
}

Inlines *
TextBlock::GetInlines ()
{
	Value *value = GetValue (TextBlock::InlinesProperty);
	
	return value ? value->AsInlines () : NULL;
}

void
TextBlock::SetText (const char *text)
{
	SetValue (TextBlock::TextProperty, Value (text));
}

const char *
TextBlock::GetText ()
{
	Value *value = GetValue (TextBlock::TextProperty);

	return value ? value->AsString () : NULL;
}

void
TextBlock::SetTextDecorations (TextDecorations decorations)
{
	SetValue (TextBlock::TextDecorationsProperty, Value (decorations));
}

TextDecorations
TextBlock::GetTextDecorations ()
{
	return (TextDecorations) GetValue (TextBlock::TextDecorationsProperty)->AsInt32 ();
}

void
TextBlock::SetTextWrapping (TextWrapping wrapping)
{
	SetValue (TextBlock::TextWrappingProperty, Value (wrapping));
}

TextWrapping
TextBlock::GetTextWrapping ()
{
	return (TextWrapping) GetValue (TextBlock::TextWrappingProperty)->AsInt32 ();
}



TextBlock *
text_block_new (void)
{
	return new TextBlock ();
}

double
text_block_get_actual_height (TextBlock *textblock)
{
	return textblock->GetActualHeight ();
}

double
text_block_get_actual_width (TextBlock *textblock)
{
	return textblock->GetActualWidth ();
}

const char *
text_block_get_font_family (TextBlock *textblock)
{
	return textblock->GetFontFamily ();
}

void
text_block_set_font_family (TextBlock *textblock, const char *family)
{
	textblock->SetFontFamily (family);
}

double
text_block_get_font_size (TextBlock *textblock)
{
	return textblock->GetFontSize ();
}

void
text_block_set_font_size (TextBlock *textblock, double size)
{
	textblock->SetFontSize (size);
}

FontStretches
text_block_get_font_stretch (TextBlock *textblock)
{
	return textblock->GetFontStretch ();
}

void
text_block_set_font_stretch (TextBlock *textblock, FontStretches stretch)
{
	textblock->SetFontStretch (stretch);
}

FontStyles
text_block_get_font_style (TextBlock *textblock)
{
	return textblock->GetFontStyle ();
}

void
text_block_set_font_style (TextBlock *textblock, FontStyles style)
{
	textblock->SetFontStyle (style);
}

FontWeights
text_block_get_font_weight (TextBlock *textblock)
{
	return textblock->GetFontWeight ();
}

void
text_block_set_font_weight (TextBlock *textblock, FontWeights weight)
{
	textblock->SetFontWeight (weight);
}

Brush *
text_block_get_foreground (TextBlock *textblock)
{
	return textblock->GetForeground ();
}

void
text_block_set_foreground (TextBlock *textblock, Brush *foreground)
{
	textblock->SetForeground (foreground);
}

Inlines *
text_block_get_inlines (TextBlock *textblock)
{
	return textblock->GetInlines ();
}

void
text_block_set_inlines (TextBlock *textblock, Inlines *inlines)
{
	textblock->SetInlines (inlines);
}

const char *
text_block_get_text (TextBlock *textblock)
{
	return textblock->GetText ();
}

void
text_block_set_text (TextBlock *textblock, const char *text)
{
	textblock->SetText (text);
}

TextDecorations
text_block_get_text_decorations (TextBlock *textblock)
{
	return textblock->GetTextDecorations ();
}

void
text_block_set_text_decorations (TextBlock *textblock, TextDecorations decorations)
{
	textblock->SetTextDecorations (decorations);
}

TextWrapping
text_block_get_text_wrapping (TextBlock *textblock)
{
	return textblock->GetTextWrapping ();
}

void
text_block_set_text_wrapping (TextBlock *textblock, TextWrapping wrapping)
{
	textblock->SetTextWrapping (wrapping);
}

void
text_block_set_font_source (TextBlock *textblock, Downloader *downloader)
{
	textblock->SetFontSource (downloader);
}


// Glyphs

DependencyProperty *Glyphs::FillProperty;
DependencyProperty *Glyphs::FontRenderingEmSizeProperty;
DependencyProperty *Glyphs::FontUriProperty;
DependencyProperty *Glyphs::IndicesProperty;
DependencyProperty *Glyphs::OriginXProperty;
DependencyProperty *Glyphs::OriginYProperty;
DependencyProperty *Glyphs::StyleSimulationsProperty;
DependencyProperty *Glyphs::UnicodeStringProperty;

enum GlyphAttrMask {
	Cluster = 1 << 1,
	Index   = 1 << 2,
	Advance = 1 << 3,
	uOffset = 1 << 4,
	vOffset = 1 << 5,
};

class GlyphAttr : public List::Node {
public:
	uint32_t glyph_count;
	uint32_t code_units;
	uint32_t index;
	double advance;
	double uoffset;
	double voffset;
	uint8_t set;
	
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
	
	invalid = false;
	dirty = false;
}

Glyphs::~Glyphs ()
{
	if (path)
		cairo_path_destroy (path);
	
	if (downloader) {
		downloader_abort (downloader);
		downloader->unref ();
	}
	
	attrs->Clear (true);
	delete attrs;
	
	g_free (text);
	
	delete desc;
}

void
Glyphs::Layout ()
{
	uint32_t code_units, glyph_count, i;
	bool first_char = true;
	double x, y, w, h, v;
	GlyphInfo *glyph;
	GlyphAttr *attr;
	TextFont *font;
	bool cluster;
	double scale;
	int n = 0;
	
	invalid = false;
	dirty = false;
	
	height = 0.0;
	width = 0.0;
	
	if (path) {
		cairo_path_destroy (path);
		path = NULL;
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
	
	scale = desc->GetSize () * 20.0 / 2048.0;
	
	x = origin_x;
	if (!origin_y_specified)
		y = font->Height ();
	else
		y = origin_y;
	
	h = y - font->Descender ();
	w = x;
	
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
					invalid = true;
					goto done;
				} else {
					glyph = font->GetGlyphInfo (*c);
				}
				
				if (attr && (attr->set & vOffset)) {
					v = y - (attr->voffset * scale);
					h = MAX (v, h);
				}
				
				if (attr && (attr->set & uOffset)) {
					v = x + (attr->uoffset * scale);
				} else {
					if (first_char) {
						if (glyph->metrics.horiBearingX < 0)
							x -= glyph->metrics.horiBearingX;
						
						first_char = false;
					}
					
					v = x;
				}
				
				v += glyph->metrics.horiAdvance;
				w = MAX (v, w);
				
				if (attr && (attr->set & Advance))
					x += attr->advance * scale;
				else
					x += glyph->metrics.horiAdvance;
				
			next1:
				
				attr = attr ? (GlyphAttr *) attr->next : NULL;
				i++;
				
				if (i == glyph_count)
					break;
				
				if (!attr) {
					// there MUST be an attr for each glyph in a cluster
					invalid = true;
					goto done;
				}
				
				if ((attr->set & Cluster)) {
					// only the first glyph in a cluster may specify a cluster mapping
					invalid = true;
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
			d(fprintf (stderr, "Can't use clusters past the end of the UnicodeString\n"));
			invalid = true;
			goto done;
		}
		
		if (!(attr->set & Index)) {
			d(fprintf (stderr, "No index specified for glyph %d\n", n + 1));
			invalid = true;
			goto done;
		}
		
		if (!(glyph = font->GetGlyphInfoByIndex (attr->index)))
			goto next;
		
		if ((attr->set & vOffset)) {
			v = y - (attr->voffset * scale);
			h = MAX (v, h);
		}
		
		if ((attr->set & uOffset)) {
			v = x + (attr->uoffset * scale);
		} else {
			if (first_char) {
				if (glyph->metrics.horiBearingX < 0)
					x -= glyph->metrics.horiBearingX;
				
				first_char = false;
			}
			
			v = x;
		}
		
		v += glyph->metrics.horiAdvance;
		w = MAX (v, w);
		
		if ((attr->set & Advance))
			x += attr->advance * scale;
		else
			x += glyph->metrics.horiAdvance;
		
	next:
		
		attr = (GlyphAttr *) attr->next;
		n++;
	}
	
	height = h > 0.0 ? h : 0.0;
	width = w;
	
done:
	
	font->unref ();
}

void
Glyphs::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	if (dirty)
		Layout ();
	
	*height = desc->GetFont ()->Height ();
	*width = this->width - origin_x;
}

Point 
Glyphs::GetOriginPoint () 
{
	if (origin_y_specified) {
		double d = desc->GetFont ()->Descender ();
		double h = desc->GetFont ()->Height ();
		return Point (origin_x, origin_y - d - h);
	} else {
		return Point (origin_x, 0);
	}
}

void
Glyphs::Render (cairo_t *cr, int x, int y, int width, int height)
{
	uint32_t code_units, glyph_count, i;
	bool first_char = true;
	GlyphInfo *glyph;
	GlyphAttr *attr;
	TextFont *font;
	double x0, y0;
	double x1, y1;
	double scale;
	
	if (this->width == 0.0 && this->height == 0.0)
		return;
	
	if (invalid) {
		// do not render anything if our state is invalid to keep with Silverlight's behavior.
		// (Note: rendering code also assumes everything is kosher)
		return;
	}
	
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	
	fill->SetupBrush (cr, this);
	
	if (path) {
		if (path->data) {
			cairo_append_path (cr, path);
			cairo_fill (cr);
		}
		
		cairo_restore (cr);
		return;
	}
	
	font = desc->GetFont ();
	
	scale = desc->GetSize () * 20.0 / 2048.0;
	
	x0 = origin_x;
	if (!origin_y_specified)
		y0 = font->Height () + font->Descender ();
	else
		y0 = origin_y;
	
	attr = (GlyphAttr *) attrs->First ();
	
	if (font->IsScalable ())
		cairo_new_path (cr);
	
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
			
			// render the glyph cluster
			for (i = 0; i < glyph_count; i++) {
				if (attr && (attr->set & Index)) {
					if (!(glyph = font->GetGlyphInfoByIndex (attr->index)))
						goto next1;
				} else {
					glyph = font->GetGlyphInfo (*c);
				}
				
				if (attr && (attr->set & vOffset))
					y1 = y0 - (attr->voffset * scale);
				else
					y1 = y0;
				
				if (attr && (attr->set & uOffset)) {
					x1 = x0 + (attr->uoffset * scale);
				} else {
					if (first_char) {
						if (glyph->metrics.horiBearingX < 0)
							x0 -= glyph->metrics.horiBearingX;
						
						first_char = false;
					}
					
					x1 = x0;
				}
				
				if (!font->IsScalable ())
					font->Render (cr, glyph, x1, y1);
				else
					font->Path (cr, glyph, x1, y1);
				
				if (attr && (attr->set & Advance))
					x0 += attr->advance * scale;
				else
					x0 += glyph->metrics.horiAdvance;
				
			next1:
				
				attr = attr ? (GlyphAttr *) attr->next : NULL;
			}
			
			// consume the code units
			for (i = 0; i < code_units && *c != 0; i++)
				c++;
		}
	}
	
	while (attr) {
		if (!(glyph = font->GetGlyphInfoByIndex (attr->index)))
			goto next;
		
		if ((attr->set & vOffset))
			y1 = y0 - (attr->voffset * scale);
		else
			y1 = y0;
		
		if ((attr->set & uOffset)) {
			x1 = x0 + (attr->uoffset * scale);
		} else {
			if (first_char) {
				if (glyph->metrics.horiBearingX < 0)
					x0 -= glyph->metrics.horiBearingX;
				
				first_char = false;
			}
			
			x1 = x0;
		}
		
		if (!font->IsScalable ())
			font->Render (cr, glyph, x1, y1);
		else
			font->Path (cr, glyph, x1, y1);
		
		if ((attr->set & Advance))
			x0 += attr->advance * scale;
		else
			x0 += glyph->metrics.horiAdvance;
		
	next:
		
		attr = (GlyphAttr *) attr->next;
	}
	
	if (font->IsScalable ()) {
		cairo_close_path (cr);
		
		if ((path = cairo_copy_path (cr)) && path->data) {
			cairo_fill (cr);
		} else if (path) {
			cairo_path_destroy (path);
			path = NULL;
		}
	}
	
	font->unref ();
	cairo_restore (cr);
}

void 
Glyphs::ComputeBounds ()
{
	if (dirty)
		Layout ();
	
	bounds = IntersectBoundsWithClipPath (Rect (0, 0, width, height), false).Transform (&absolute_xform);
}

Point
Glyphs::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	
	return Point (user_xform_origin.x * width, user_xform_origin.y * height);
}

void
Glyphs::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == Glyphs::FillProperty)
		Invalidate ();
	else
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
Glyphs::data_write (void *buf, int32_t offset, int32_t n, gpointer data)
{
	;
}

void
Glyphs::size_notify (int64_t size, gpointer data)
{
	;
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
	if (!(filename = downloader->GetDownloadedFile ()))
		return;
	
	if (stat (filename, &st) == -1 || !S_ISREG (st.st_mode))
		return;
	
	if (!downloader->IsDeobfuscated ()) {
		if ((path = deobfuscate_font (downloader, filename)))
			filename = path;
		
		downloader->SetDeobfuscated (true);
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
	int i;
	
	fprintf (stderr, "Glyph Indices parse error: \"%s\": %s\n", in, reason);
	fprintf (stderr, "                            ");
	for (i = 0; i < (where - in); i++)
		fputc (' ', stderr);
	fprintf (stderr, "^\n");
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
				d(print_parse_error (in, inptr, errno ? strerror (errno) : "invalid cluster mapping; CodeUnitCount cannot be 0"));
				delete glyph;
				return;
			}
			
			inptr = end;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			if (*inptr != ':') {
				// invalid cluster
				d(print_parse_error (in, inptr, "expected ':'"));
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
				d(print_parse_error (in, inptr, errno ? strerror (errno) : "invalid cluster mapping; GlyphCount cannot be 0"));
				delete glyph;
				return;
			}
			
			inptr = end;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			if (*inptr != ')') {
				// invalid cluster
				d(print_parse_error (in, inptr, "expected ')'"));
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
				d(print_parse_error (in, inptr, strerror (errno)));
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
					d(print_parse_error (in, inptr, strerror (errno)));
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
			d(print_parse_error (in, inptr, "expected ';'"));
			return;
		}
		
		if (*inptr == '\0')
			break;
		
		inptr++;
	}
}

void
Glyphs::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	bool invalidate = true;
	
	if (args->property->type != Type::GLYPHS) {
		FrameworkElement::OnPropertyChanged (args);
		return;
	}
	
	if (args->property == Glyphs::FontUriProperty) {
		char *str = args->new_value ? args->new_value->AsString() : NULL;
		Uri *uri = new Uri ();
		
		if (downloader) {
			downloader_abort (downloader);
			downloader->unref ();
			downloader = NULL;
			index = 0;
		}
		
		if (str && *str && uri->Parse (str)) {
			downloader = Surface::CreateDownloader (this);
			
			if (uri->fragment) {
				if ((index = strtol (uri->fragment, NULL, 10)) < 0 || index == LONG_MAX)
					index = 0;
			}
			
			str = uri->ToString (UriHideFragment);
			downloader_open (downloader, "GET", str);
			g_free (str);
			
			downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);
			if (downloader->Started () || downloader->Completed ()) {
				if (downloader->Completed ())
					DownloaderComplete ();
			} else {
				downloader->SetWriteFunc (data_write, size_notify, this);
				
				// This is what actually triggers the download
				downloader->Send ();
			}
		}
		
		delete uri;
		
		invalidate = false;
	} else if (args->property == Glyphs::FillProperty) {
		fill = args->new_value ? args->new_value->AsBrush() : NULL;
	} else if (args->property == Glyphs::UnicodeStringProperty) {
		char *str = args->new_value ? args->new_value->AsString() : NULL;
		g_free (text);
		
		if (str != NULL)
			text = g_utf8_to_ucs4_fast (str, -1, NULL);
		else
			text = NULL;
		
		dirty = true;
	} else if (args->property == Glyphs::IndicesProperty) {
		char *str = args->new_value ? args->new_value->AsString() : NULL;
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
		// Silverlight 1.0 does not implement this property
		invalidate = false;
	}
	
	if (invalidate)
		Invalidate ();
	
	if (dirty)
		UpdateBounds (true);
	
	NotifyListenersOfPropertyChange (args);
}


void
Glyphs::SetFill (Brush *fill)
{
	SetValue (Glyphs::FillProperty, Value (fill));
}

Brush *
Glyphs::GetFill ()
{
	Value *value = GetValue (Glyphs::FillProperty);
	
	return value ? value->AsBrush () : NULL;
}

void
Glyphs::SetFontRenderingEmSize (double size)
{
	SetValue (Glyphs::FontRenderingEmSizeProperty, Value (size));
}

double
Glyphs::GetFontRenderingEmSize ()
{
	return GetValue (Glyphs::FontRenderingEmSizeProperty)->AsDouble ();
}

void
Glyphs::SetFontUri (const char *uri)
{
	SetValue (Glyphs::FontUriProperty, Value (uri));
}

const char *
Glyphs::GetFontUri ()
{
	Value *value = GetValue (Glyphs::FontUriProperty);
	
	return value ? value->AsString () : NULL;
}

void
Glyphs::SetIndices (const char *indices)
{
	SetValue (Glyphs::IndicesProperty, Value (indices));
}

const char *
Glyphs::GetIndices ()
{
	Value *value = GetValue (Glyphs::IndicesProperty);
	
	return value ? value->AsString () : NULL;
}

void
Glyphs::SetOriginX (double origin)
{
	SetValue (Glyphs::OriginXProperty, Value (origin));
}

double
Glyphs::GetOriginX ()
{
	return GetValue (Glyphs::OriginXProperty)->AsDouble ();
}

void
Glyphs::SetOriginY (double origin)
{
	SetValue (Glyphs::OriginYProperty, Value (origin));
}

double
Glyphs::GetOriginY ()
{
	return GetValue (Glyphs::OriginYProperty)->AsDouble ();
}

void
Glyphs::SetStyleSimulations (StyleSimulations style)
{
	SetValue (Glyphs::StyleSimulationsProperty, Value (style));
}

StyleSimulations
Glyphs::GetStyleSimulations ()
{
	return (StyleSimulations) GetValue (Glyphs::StyleSimulationsProperty)->AsInt32 ();
}

void
Glyphs::SetUnicodeString (const char *unicode)
{
	SetValue (Glyphs::UnicodeStringProperty, Value (unicode));
}

const char *
Glyphs::GetUnicodeString ()
{
	Value *value = GetValue (Glyphs::UnicodeStringProperty);
	
	return value ? value->AsString () : NULL;
}


Glyphs *
glyphs_new (void)
{
	return new Glyphs ();
}

Brush *
glyphs_get_fill (Glyphs *glyphs)
{
	return glyphs->GetFill ();
}

void
glyphs_set_fill (Glyphs *glyphs, Brush *fill)
{
	glyphs->SetFill (fill);
}

double
glyphs_get_font_rendering_em_size (Glyphs *glyphs)
{
	return glyphs->GetFontRenderingEmSize ();
}

void
glyphs_set_font_rendering_em_size (Glyphs *glyphs, double size)
{
	glyphs->SetFontRenderingEmSize (size);
}

const char *
glyphs_get_font_uri (Glyphs *glyphs)
{
	return glyphs->GetFontUri ();
}

void
glyphs_set_font_uri (Glyphs *glyphs, const char *uri)
{
	glyphs->SetFontUri (uri);
}

const char *
glyphs_get_indices (Glyphs *glyphs)
{
	return glyphs->GetIndices ();
}

void
glyphs_set_indices (Glyphs *glyphs, const char *indices)
{
	glyphs->SetIndices (indices);
}

double
glyphs_get_origin_x (Glyphs *glyphs)
{
	return glyphs->GetOriginX ();
}

void
glyphs_set_origin_x (Glyphs *glyphs, double origin)
{
	glyphs->SetOriginX (origin);
}

double
glyphs_get_origin_y (Glyphs *glyphs)
{
	return glyphs->GetOriginY ();
}

void
glyphs_set_origin_y (Glyphs *glyphs, double origin)
{
	glyphs->SetOriginY (origin);
}

StyleSimulations
glyphs_get_style_simulations (Glyphs *glyphs)
{
	return glyphs->GetStyleSimulations ();
}

void
glyphs_set_style_simulations (Glyphs *glyphs, StyleSimulations style)
{
	glyphs->SetStyleSimulations (style);
}

const char *
glyphs_get_unicode_string (Glyphs *glyphs)
{
	return glyphs->GetUnicodeString ();
}

void
glyphs_set_unicode_string (Glyphs *glyphs, const char *unicode)
{
	glyphs->SetUnicodeString (unicode);
}


void
text_destroy (void)
{
	if (default_foreground_brush) {
		default_foreground_brush->unref ();
		default_foreground_brush = NULL;
	}
}


void
text_init (void)
{
	font_init ();
	
	// Inline
	Inline::FontFamilyProperty = DependencyObject::Register (Type::INLINE, "FontFamily", new Value (TEXTBLOCK_FONT_FAMILY));
	Inline::FontSizeProperty = DependencyObject::Register (Type::INLINE, "FontSize", new Value (TEXTBLOCK_FONT_SIZE));
	Inline::FontStretchProperty = DependencyObject::Register (Type::INLINE, "FontStretch", new Value (TEXTBLOCK_FONT_STRETCH));
	Inline::FontStyleProperty = DependencyObject::Register (Type::INLINE, "FontStyle", new Value (TEXTBLOCK_FONT_STYLE));
	Inline::FontWeightProperty = DependencyObject::Register (Type::INLINE, "FontWeight", new Value (TEXTBLOCK_FONT_WEIGHT));
	Inline::ForegroundProperty = DependencyObject::Register (Type::INLINE, "Foreground", Type::BRUSH);
	Inline::TextDecorationsProperty = DependencyObject::Register (Type::INLINE, "TextDecorations", new Value (TextDecorationsNone));
	
	
	// Run
	Run::TextProperty = DependencyObject::Register (Type::RUN, "Text", Type::STRING);
	
	
	// TextBlock
	TextBlock::ActualHeightProperty = DependencyObject::RegisterFull (Type::TEXTBLOCK, "ActualHeight", NULL, Type::DOUBLE, false, true);
	TextBlock::ActualWidthProperty = DependencyObject::RegisterFull (Type::TEXTBLOCK, "ActualWidth", NULL, Type::DOUBLE, false, true);
	TextBlock::FontFamilyProperty = DependencyObject::Register (Type::TEXTBLOCK, "FontFamily", new Value (TEXTBLOCK_FONT_FAMILY));
	TextBlock::FontSizeProperty = DependencyObject::Register (Type::TEXTBLOCK, "FontSize", new Value (TEXTBLOCK_FONT_SIZE));
	TextBlock::FontStretchProperty = DependencyObject::Register (Type::TEXTBLOCK, "FontStretch", new Value (TEXTBLOCK_FONT_STRETCH));
	TextBlock::FontStyleProperty = DependencyObject::Register (Type::TEXTBLOCK, "FontStyle", new Value (TEXTBLOCK_FONT_STYLE));
	TextBlock::FontWeightProperty = DependencyObject::Register (Type::TEXTBLOCK, "FontWeight", new Value (TEXTBLOCK_FONT_WEIGHT));
	TextBlock::ForegroundProperty = DependencyObject::Register (Type::TEXTBLOCK, "Foreground", Type::BRUSH);
	TextBlock::InlinesProperty = DependencyObject::Register (Type::TEXTBLOCK, "Inlines", Type::INLINES);
	TextBlock::TextProperty = DependencyObject::Register (Type::TEXTBLOCK, "Text", Type::STRING);
	TextBlock::TextDecorationsProperty = DependencyObject::Register (Type::TEXTBLOCK, "TextDecorations", new Value (TextDecorationsNone));
	TextBlock::TextWrappingProperty = DependencyObject::Register (Type::TEXTBLOCK, "TextWrapping", new Value (TextWrappingNoWrap));
	
	
	// Glyphs
	Glyphs::FillProperty = DependencyObject::Register (Type::GLYPHS, "Fill", Type::BRUSH);
	Glyphs::FontRenderingEmSizeProperty = DependencyObject::Register (Type::GLYPHS, "FontRenderingEmSize", new Value (0.0));
	Glyphs::FontUriProperty = DependencyObject::Register (Type::GLYPHS, "FontUri", Type::STRING);
	Glyphs::IndicesProperty = DependencyObject::Register (Type::GLYPHS, "Indices", Type::STRING);
	Glyphs::OriginXProperty = DependencyObject::Register (Type::GLYPHS, "OriginX", new Value (0.0));
	Glyphs::OriginYProperty = DependencyObject::Register (Type::GLYPHS, "OriginY", new Value (0.0));
	Glyphs::StyleSimulationsProperty = DependencyObject::Register (Type::GLYPHS, "StyleSimulations", new Value (StyleSimulationsNone));
	Glyphs::UnicodeStringProperty = DependencyObject::Register (Type::GLYPHS, "UnicodeString", Type::STRING);
}
