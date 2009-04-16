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
#include "textblock.h"
#include "runtime.h"
#include "color.h"
#include "utils.h"
#include "debug.h"
#include "uri.h"


//
// Inline
//

Inline::Inline ()
{
	SetObjectType (Type::INLINE);
	font = new TextFontDescription ();
	downloader = NULL;
	autogen = false;
}

Inline::~Inline ()
{
	CleanupDownloader ();
	
	delete font;
}

void
Inline::CleanupDownloader ()
{
	if (downloader) {
		downloader->RemoveHandler (Downloader::CompletedEvent, downloader_complete, this);
		downloader->Abort ();
		downloader->unref ();
		downloader = NULL;
	}
}

void
Inline::SetFontSource (Downloader *downloader)
{
	if (this->downloader == downloader)
		return;
	
	CleanupDownloader ();
	
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
		ClearValue (Inline::FontFilenameProperty);
		ClearValue (Inline::FontGUIDProperty);
	}
}

void
Inline::SetFontResource (const char *resource)
{
	Application *application = Application::GetCurrent ();
	const char *guid = NULL;
	Surface *surface;
	char *filename;
	size_t len;
	Uri *uri;
	
	CleanupDownloader ();
	
	uri = new Uri ();
	
	if (!application || !uri->Parse (resource) || !(filename = application->GetResourceAsPath (uri))) {
		if ((surface = GetSurface ()) && (downloader = surface->CreateDownloader ())) {
			downloader->Open ("GET", resource, XamlPolicy);
			SetFontSource (downloader);
			downloader->unref ();
		} else {
			ClearValue (Inline::FontFilenameProperty);
			ClearValue (Inline::FontGUIDProperty);
		}
		
		delete uri;
		
		return;
	}
	
	delete uri;
	
	// check if the resource is an obfuscated font
	len = strlen (resource);
	if (len > 6 && !g_ascii_strcasecmp (resource + len - 6, ".odttf"))
		guid = resource;
	
	SetValue (Inline::FontFilenameProperty, Value (filename));
	if (guid != NULL)
		SetValue (Inline::FontGUIDProperty, Value (guid));
	else
		ClearValue (Inline::FontGUIDProperty);
	
	g_free (filename);
}

void
Inline::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType () != Type::INLINE) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == Inline::FontFamilyProperty) {
		FontFamily *family = args->GetNewValue() ? args->GetNewValue()->AsFontFamily () : NULL;
		const char *family_name;
		char *resource;
		
		if (family && family->source && (family_name = strchr (family->source, '#'))) {
			// FontFamily can reference a font resource in the form "<resource>#<family name>"
			resource = g_strndup (family->source, family_name - family->source);
			SetFontResource (resource);
			g_free (resource);
		} else {
			ClearValue (Inline::FontFilenameProperty);
			ClearValue (Inline::FontGUIDProperty);
		}
	}
	
	NotifyListenersOfPropertyChange (args);
}

void
Inline::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop && prop->GetId () == Inline::ForegroundProperty) {
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
	
	if (*item->GetFontFamily () != *GetFontFamily ())
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

bool
Inline::UpdateFontDescription ()
{
	// FIXME: I hate having to do it this way, updating the font
	// in OnPropertyChanged() was a much much better way to do
	// things. *sigh*
	FontFamily *family = GetFontFamily ();
	const char *family_name = NULL;
	bool changed = false;
	
	if (family && family->source) {
		if (!(family_name = strchr (family->source, '#')))
			family_name = family->source;
		else
			family_name++;
	}
	
	if (font->SetFilename (GetFontFilename (), GetFontGUID ()))
		changed = true;
	
	if (font->SetFamily (family_name))
		changed = true;
	
	if (font->SetStyle (GetFontStyle ()))
		changed = true;
	
	if (font->SetWeight (GetFontWeight ()))
		changed = true;
	
	if (font->SetSize (GetFontSize ()))
		changed = true;
	
	if (font->SetStretch (GetFontStretch ()))
		changed = true;
	
	return changed;
}

void
Inline::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((Inline *) closure)->DownloaderComplete ();
}

void
Inline::DownloaderComplete ()
{
	const char *path, *guid = NULL;
	InternalDownloader *idl;
	char *filename;
	size_t len;
	Uri *uri;
	
	// get the downloaded file path (enforces a mozilla workaround for files smaller than 64k)
	if (!(filename = downloader->GetDownloadedFilename (NULL)))
		return;
	
	g_free (filename);
	
	if (!(idl = downloader->GetInternalDownloader ()))
		return;
	
	if (!(idl->GetType () == InternalDownloader::FileDownloader))
		return;
	
	uri = downloader->GetUri ();
	path = uri->GetPath ();
	
	len = path ? strlen (path) : 0;
	
	if (len > 6 && !g_ascii_strcasecmp (path + len - 6, ".odttf")) {
		// if the font file is obfuscated, use the basename of the path as the guid
		if (!(guid = strrchr (path, '/')))
			guid = path;
		else
			guid++;
	}
	
	// If the downloaded file was a zip file, this'll get the path to the
	// extracted zip directory, else it will simply be the path to the
	// downloaded file.
	if (!(path = ((FileDownloader *) idl)->GetUnzippedPath ()))
		return;
	
	SetValue (Inline::FontFilenameProperty, Value (path));
	if (guid != NULL)
		SetValue (Inline::FontGUIDProperty, Value (guid));
	else
		ClearValue (Inline::FontGUIDProperty);
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
		if (property->GetId () != FrameworkElement::ActualHeightProperty && property->GetId () != FrameworkElement::ActualWidthProperty)
			return NULL;
		
		TextBlock *tb = (TextBlock *) obj;
		Thickness padding = *tb->GetPadding ();
		
		if (tb->dirty) {
			Size constraint;
			
			constraint = tb->GetSize ().GrowBy (-padding);
			
			delete actual_height_value;
			actual_height_value = NULL;
			delete actual_width_value;
			actual_width_value = NULL;
			
			tb->Layout (constraint);
		}
		
		if (property->GetId () == FrameworkElement::ActualHeightProperty) {
			if (!actual_height_value)
				actual_height_value = new Value (tb->actual_height + padding.top + padding.bottom);
			return actual_height_value;
		} else {
			if (!actual_width_value)
				actual_width_value = new Value (tb->actual_width + padding.left + padding.right);
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
	
	actual_height = 0.0;
	actual_width = 0.0;
	setvalue = true;
	was_set = false;
	dirty = true;
	
	font = new TextFontDescription ();
	font->SetFamily (TEXTBLOCK_FONT_FAMILY);
	font->SetStretch (TEXTBLOCK_FONT_STRETCH);
	font->SetWeight (TEXTBLOCK_FONT_WEIGHT);
	font->SetStyle (TEXTBLOCK_FONT_STYLE);
	font->SetSize (TEXTBLOCK_FONT_SIZE);
}

TextBlock::~TextBlock ()
{
	CleanupDownloader ();
	
	delete layout;
	delete font;
}

void
TextBlock::CleanupDownloader ()
{
	if (downloader) {
		downloader->RemoveHandler (Downloader::CompletedEvent, downloader_complete, this);
		downloader->Abort ();
		downloader->unref ();
		downloader = NULL;
	}
}

void
TextBlock::SetFontSource (Downloader *downloader)
{
	if (this->downloader == downloader)
		return;
	
	ClearValue (TextBlock::FontSourceProperty);
	CleanupDownloader ();
	
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
		ClearValue (TextBlock::FontGUIDProperty);
		font->SetFilename (NULL, NULL);
		UpdateFontDescriptions ();
		
		dirty = true;
		
		UpdateBounds (true);
		Invalidate ();
	}
}

void
TextBlock::SetFontResource (const char *resource)
{
	Application *application = Application::GetCurrent ();
	Downloader *downloader;
	const char *guid = NULL;
	Surface *surface;
	char *filename;
	size_t len;
	Uri *uri;
	
	ClearValue (TextBlock::FontSourceProperty);
	CleanupDownloader ();
	
	uri = new Uri ();
	
	if (!application || !uri->Parse (resource) || !(filename = application->GetResourceAsPath (uri))) {
		if ((surface = GetSurface ()) && (downloader = surface->CreateDownloader ())) {
			downloader->Open ("GET", resource, XamlPolicy);
			SetFontSource (downloader);
			downloader->unref ();
		} else {
			ClearValue (TextBlock::FontFilenameProperty);
			ClearValue (TextBlock::FontGUIDProperty);
			font->SetFilename (NULL, NULL);
			UpdateFontDescriptions ();
		}
		
		delete uri;
		
		return;
	}
	
	delete uri;
	
	// check if the resource is an obfuscated font
	len = strlen (resource);
	if (len > 6 && !g_ascii_strcasecmp (resource + len - 6, ".odttf"))
		guid = resource;
	
	SetValue (TextBlock::FontFilenameProperty, Value (filename));
	if (guid != NULL)
		SetValue (TextBlock::FontGUIDProperty, Value (guid));
	else
		ClearValue (TextBlock::FontGUIDProperty);
	
	font->SetFilename (filename, guid);
	UpdateFontDescriptions ();
	
	g_free (filename);
}

Size
TextBlock::GetSize ()
{
	Size size = Size (GetWidth (), GetHeight ());
	
	if (isnan (size.height))
		size.height = INFINITY;
	
	if (isnan (size.width))
		size.width = INFINITY;
	
	return size;
}

void
TextBlock::Render (cairo_t *cr, Region *region, bool path_only)
{
	layout->SetAvailableWidth (GetActualWidth ());
	
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	Paint (cr);
	cairo_restore (cr);
}

void
TextBlock::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	*width = actual_width;
	*height = actual_height;
}

void
TextBlock::ComputeBounds ()
{
	Size total = Size (actual_width, actual_height);
	total.GrowBy (*GetPadding ());
	extents = Rect (0,0,total.width, total.height);
	bounds = IntersectBoundsWithClipPath (extents, false).Transform (&absolute_xform);
	bounds_with_children = bounds;
}

Point
TextBlock::GetTransformOrigin ()
{
	Point *user_xform_origin = GetRenderTransformOrigin ();
	return Point (actual_width * user_xform_origin->x, 
		      actual_height * user_xform_origin->y);
}

Size
TextBlock::MeasureOverride (Size availableSize)
{
	const char *text = layout->GetText ();
	Thickness padding = *GetPadding ();
	Size constraint;
	Size desired;
	
	//if (text && (!strcmp (text, "751 items") || !strncmp (text, "Use your mouse wheel to", 23))) {
	//	printf ("\nTextBlock::MeasureOverride(availableSize = { %f, %f })\n", availableSize.width, availableSize.height);
	//	printf ("\tText = \"%s\";\n", text);
	//}
	
	constraint = availableSize.GrowBy (-padding);
	Layout (constraint);
	
	SetActualHeight (actual_height);
	SetActualWidth (actual_width);
	
	desired = Size (actual_width, actual_height).GrowBy (padding);
	desired = desired.Min (availableSize);
	
	//if (text && (!strcmp (text, "751 items") || !strncmp (text, "Use your mouse wheel", 20)))
	//	printf ("\treturn { %f, %f };\n", desired.width, desired.height);
	
	return desired;
}

Size
TextBlock::ArrangeOverride (Size finalSize)
{
	const char *text = layout->GetText ();
	Thickness padding = *GetPadding ();
	Size constraint;
	Size arranged;
	
	//if (text && (!strcmp (text, "751 items") || !strncmp (text, "Use your mouse wheel to", 23))) {
	//	printf ("\nTextBlock::ArrangeOverride(finalSize = { %f, %f })\n", finalSize.width, finalSize.height);
	//	printf ("\tText = \"%s\";\n", text);
	//}
	
	constraint = finalSize.GrowBy (-padding);
	Layout (constraint);
	
	arranged = Size (actual_width, actual_height).GrowBy (padding);
	arranged = arranged.Max (finalSize);
	
	//if (text && (!strcmp (text, "751 items") || !strncmp (text, "Use your mouse wheel", 20)))
	//	printf ("\treturn { %f, %f };\n", arranged.width, arranged.height);
	
	return arranged;
}

void
TextBlock::UpdateLayoutAttributes ()
{
	InlineCollection *inlines = GetInlines ();
	TextLayoutAttributes *attrs;
	const char *text;
	int length = 0;
	Inline *item;
	List *runs;
	
	runs = new List ();
	
	if (inlines != NULL) {
		for (int i = 0; i < inlines->GetCount (); i++) {
			item = inlines->GetValueAt (i)->AsInline ();
			item->UpdateFontDescription ();
			
 			switch (item->GetObjectType ()) {
			case Type::RUN:
				text = ((Run *) item)->GetText ();
				
				if (text && text[0]) {
					attrs = new TextLayoutAttributes ((ITextAttributes *) item, length);
					runs->Append (attrs);
					
					length += strlen (text);
				}
				
				break;
			case Type::LINEBREAK:
				attrs = new TextLayoutAttributes ((ITextAttributes *) item, length);
				runs->Append (attrs);
				
				length++;
				break;
			default:
				break;
			}
		}
		
		if (inlines->GetCount () > 0)
			was_set = true;
	}
	
	layout->SetText (GetText (), length);
	layout->SetTextAttributes (runs);
}

bool
TextBlock::UpdateFontDescriptions ()
{
	InlineCollection *inlines = GetInlines ();
	bool changed = false;
	Inline *item;
	
	if (inlines != NULL) {
		for (int i = 0; i < inlines->GetCount (); i++) {
			item = inlines->GetValueAt (i)->AsInline ();
			if (item->UpdateFontDescription ())
				changed = true;
		}
		
		if (changed)
			layout->ResetState ();
	}
	
	return changed;
}

void
TextBlock::Layout (Size constraint)
{
	const char *text = layout->GetText ();
	
	if (was_set && !GetValueNoDefault (TextBlock::TextProperty)) {
		// FIXME: Does this only apply if the Text is set to
		// String.Empty?  If so, then TextLayout::Layout()
		// will handle this for us...
		
		// If the Text property had been set once upon a time,
		// but is currently empty, Silverlight seems to set
		// the ActualHeight property to the font height. See
		// bug #405514 for details.
		TextFont *font = this->font->GetFont ();
		actual_height = font->Height ();
		actual_width = 0.0;
		font->unref ();
	} else if (!was_set) {
		// If the Text property has never been set, then its
		// extents should both be 0.0. See bug #435798 for
		// details.
		actual_height = 0.0;
		actual_width = 0.0;
	} else {
		layout->SetMaxWidth (constraint.width);
		layout->Layout ();
		
		layout->GetActualExtents (&actual_width, &actual_height);
	}
	
	//if (text && (!strcmp (text, "751 items") || !strncmp (text, "Use your mouse wheel to", 23)))
	//	printf ("\tTextBlock::Layout(constraint = { %f, %f }) => %f, %f\n", constraint.width, constraint.height, actual_width, actual_height);
	
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
TextBlock::GetTextInternal (InlineCollection *inlines)
{
	const char *text;
	GString *block;
	Inline *item;
	char *str;
	
	if (!inlines)
		return g_strdup ("");
	
	block = g_string_new ("");
	
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
	
	str = block->str;
	g_string_free (block, false);
	
	return str;
}

bool
TextBlock::SetTextInternal (const char *text)
{
	Value *value = ReadLocalValue (TextBlock::InlinesProperty);
	InlineCollection *curInlines = value ? value->AsInlineCollection () : NULL;
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
TextBlock::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	bool invalidate = true;
	
	if (args->GetProperty ()->GetOwnerType () != Type::TEXTBLOCK) {
		FrameworkElement::OnPropertyChanged (args, error);
		if (args->GetId () == FrameworkElement::WidthProperty) {
			if (layout->SetMaxWidth (args->GetNewValue()->AsDouble ()))
				dirty = true;
			
			UpdateBounds (true);
		}
		
		return;
	}
	
	if (args->GetId () == TextBlock::FontFamilyProperty) {
		FontFamily *family = args->GetNewValue() ? args->GetNewValue()->AsFontFamily () : NULL;
		const char *family_name = NULL;
		char *resource;
		
		if (family && family->source && (family_name = strchr (family->source, '#'))) {
			// FontFamily can reference a font resource in the form "<resource>#<family name>"
			resource = g_strndup (family->source, family_name - family->source);
			SetFontResource (resource);
			g_free (resource);
			family_name++;
		} else if (family) {
			family_name = family->source;
		}
		
		font->SetFamily (family_name);
		UpdateFontDescriptions ();
		dirty = true;
	} else if (args->GetId () == TextBlock::FontSizeProperty) {
		double size = args->GetNewValue()->AsDouble ();
		font->SetSize (size);
		UpdateFontDescriptions ();
		dirty = true;
	} else if (args->GetId () == TextBlock::FontStretchProperty) {
		FontStretches stretch = (FontStretches) args->GetNewValue()->AsInt32 ();
		font->SetStretch (stretch);
		UpdateFontDescriptions ();
		dirty = true;
	} else if (args->GetId () == TextBlock::FontStyleProperty) {
		FontStyles style = (FontStyles) args->GetNewValue()->AsInt32 ();
		font->SetStyle (style);
		UpdateFontDescriptions ();
		dirty = true;
	} else if (args->GetId () == TextBlock::FontWeightProperty) {
		FontWeights weight = (FontWeights) args->GetNewValue()->AsInt32 ();
		font->SetWeight (weight);
		UpdateFontDescriptions ();
		dirty = true;
	} else if (args->GetId () == TextBlock::TextProperty) {
		if (setvalue) {
			// result of a change to the TextBlock.Text property
			const char *text = args->GetNewValue() ? args->GetNewValue()->AsString () : NULL;
			
			if (!SetTextInternal (text)) {
				// no change so nothing to invalidate
				invalidate = false;
			} else {
				UpdateLayoutAttributes ();
				dirty = true;
			}
		} else {
			// result of a change to the TextBlock.Inlines property
			UpdateLayoutAttributes ();
			invalidate = false;
		}
	} else if (args->GetId () == TextBlock::TextDecorationsProperty) {
		dirty = true;
	} else if (args->GetId () == TextBlock::TextWrappingProperty) {
		dirty = layout->SetTextWrapping ((TextWrapping) args->GetNewValue()->AsInt32 ());
	} else if (args->GetId () == TextBlock::InlinesProperty) {
		if (setvalue) {
			// result of a change to the TextBlock.Inlines property
			InlineCollection *inlines = args->GetNewValue() ? args->GetNewValue()->AsInlineCollection () : NULL;
			
			setvalue = false;
			// Note: this will cause UpdateLayoutAttributes() to be called in the TextProperty changed logic above
			SetValue (TextBlock::TextProperty, Value (GetTextInternal (inlines), true));
			setvalue = true;
			
			dirty = true;
		} else {
			// result of a change to the TextBlock.Text property
			invalidate = false;
		}
	} else if (args->GetId () == TextBlock::LineStackingStrategyProperty) {
		dirty = layout->SetLineStackingStrategy ((LineStackingStrategy) args->GetNewValue()->AsInt32 ());
	} else if (args->GetId () == TextBlock::LineHeightProperty) {
		dirty = layout->SetLineHeight (args->GetNewValue()->AsDouble ());
	} else if (args->GetId () == TextBlock::TextAlignmentProperty) {
		dirty = layout->SetTextAlignment ((TextAlignment) args->GetNewValue()->AsInt32 ());
	} else if (args->GetId () == TextBlock::PaddingProperty) {
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
	if (prop && prop->GetId () == TextBlock::ForegroundProperty) {
		Invalidate ();
	} else {
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
	}
}

void
TextBlock::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	InlineCollection *inlines = GetInlines ();
	bool update_bounds = false;
	bool update_text = false;
	
	if (col != inlines) {
		FrameworkElement::OnCollectionChanged (col, args);
		return;
	}
	
	switch (args->GetChangedAction()) {
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
		// Note: this will cause UpdateLayoutAttributes() to be called in the TextProperty changed logic above
		SetValue (TextBlock::TextProperty, Value (GetTextInternal (inlines), true));
		setvalue = true;
	}
	
	if (update_bounds)
		UpdateBounds (true);
	
	Invalidate ();
}

void
TextBlock::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	InlineCollection *inlines = GetInlines ();
	
	if (col != inlines) {
		FrameworkElement::OnCollectionItemChanged (col, obj, args);
		return;
	}
	
	if (args->GetId () != Inline::ForegroundProperty) {
		if (args->GetId () == Run::TextProperty) {
			// update our TextProperty
			setvalue = false;
			// Note: this will cause UpdateLayoutAttributes() to be called in the TextProperty changed logic above
			SetValue (TextBlock::TextProperty, Value (GetTextInternal (inlines), true));
			setvalue = true;
		} else {
			// likely a font property change...
			((Inline *) obj)->UpdateFontDescription ();
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

void
TextBlock::DownloaderComplete ()
{
	const char *path, *guid = NULL;
	InternalDownloader *idl;
	char *filename;
	size_t len;
	Uri *uri;
	
	// get the downloaded file path (enforces a mozilla workaround for files smaller than 64k)
	if (!(filename = downloader->GetDownloadedFilename (NULL)))
		return;
	
	g_free (filename);
	
	if (!(idl = downloader->GetInternalDownloader ()))
		return;
	
	if (!(idl->GetType () == InternalDownloader::FileDownloader))
		return;
	
	uri = downloader->GetUri ();
	path = uri->GetPath ();
	
	len = path ? strlen (path) : 0;
	
	if (len > 6 && !g_ascii_strcasecmp (path + len - 6, ".odttf")) {
		// if the font file is obfuscated, use the basename of the path as the guid
		if (!(guid = strrchr (path, '/')))
			guid = path;
		else
			guid++;
	}
	
	// If the downloaded file was a zip file, this'll get the path to the
	// extracted zip directory, else it will simply be the path to the
	// downloaded file.
	if (!(path = ((FileDownloader *) idl)->GetUnzippedPath ()))
		return;
	
	SetValue (TextBlock::FontFilenameProperty, Value (path));
	if (guid != NULL)
		SetValue (TextBlock::FontGUIDProperty, Value (guid));
	else
		ClearValue (TextBlock::FontGUIDProperty);
	
	font->SetFilename (path, guid);
	
	if (UpdateFontDescriptions ()) {
		dirty = true;
		
		UpdateBounds (true);
		Invalidate ();
	}
}
