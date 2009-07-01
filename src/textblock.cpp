/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * textblock.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <cairo.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "file-downloader.h"
#include "textblock.h"
#include "control.h"
#include "runtime.h"
#include "color.h"
#include "utils.h"
#include "debug.h"
#include "uri.h"


// Unicode Line Separator (\u2028)
static const char utf8_linebreak[3] = { 0xe2, 0x80, 0xa8 };
#define utf8_linebreak_len 3


//
// Inline
//

Inline::Inline ()
{
	SetObjectType (Type::INLINE);
	font = new TextFontDescription ();
	downloaders = g_ptr_array_new ();
	autogen = false;
}

Inline::~Inline ()
{
	CleanupDownloaders ();
	g_ptr_array_free (downloaders, true);
	delete font;
}

void
Inline::CleanupDownloaders ()
{
	Downloader *downloader;
	guint i;
	
	for (i = 0; i < downloaders->len; i++) {
		downloader = (Downloader *) downloaders->pdata[i];
		downloader->RemoveHandler (Downloader::CompletedEvent, downloader_complete, this);
		downloader->Abort ();
		downloader->unref ();
	}
	
	g_ptr_array_set_size (downloaders, 0);
}

void
Inline::AddFontSource (Downloader *downloader)
{
	downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);
	g_ptr_array_add (downloaders, downloader);
	downloader->ref ();
	
	if (downloader->Started () || downloader->Completed ()) {
		if (downloader->Completed ())
			DownloaderComplete (downloader);
	} else {
		// This is what actually triggers the download
		downloader->Send ();
	}
}

void
Inline::AddFontResource (const char *resource)
{
	FontManager *manager = Deployment::GetCurrent ()->GetFontManager ();
	Application *application = Application::GetCurrent ();
	Downloader *downloader;
	Surface *surface;
	char *path;
	Uri *uri;
	
	uri = new Uri ();
	
	if (!application || !uri->Parse (resource) || !(path = application->GetResourceAsPath (uri))) {
		if ((surface = GetSurface ()) && (downloader = surface->CreateDownloader ())) {
			downloader->Open ("GET", resource, XamlPolicy);
			AddFontSource (downloader);
			downloader->unref ();
		}
		
		delete uri;
		
		return;
	}
	
	manager->AddResource (resource, path);
	g_free (path);
	delete uri;
}

void
Inline::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType () != Type::INLINE) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == Inline::FontFamilyProperty) {
		FontFamily *family = args->GetNewValue () ? args->GetNewValue ()->AsFontFamily () : NULL;
		char **families, *fragment;
		int i;
		
		CleanupDownloaders ();
		
		if (family && family->source) {
			families = g_strsplit (family->source, ",", -1);
			for (i = 0; families[i]; i++) {
				g_strstrip (families[i]);
				if ((fragment = strchr (families[i], '#'))) {
					// the first portion of this string is the resource name...
					*fragment = '\0';
					AddFontResource (families[i]);
				}
			}
			g_strfreev (families);
		}
	}
	
	NotifyListenersOfPropertyChange (args, error);
}

void
Inline::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop && prop->GetId () == Inline::ForegroundProperty) {
		// this isn't exactly what we want, I don't
		// think... but it'll have to do.
		NotifyListenersOfPropertyChange (prop, NULL);
	} else {
		DependencyObject::OnSubPropertyChanged (prop, obj, subobj_args);
	}
}

bool
Inline::Equals (Inline *item)
{
	const char *lang0, *lang1;
	
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
	
	if (item->GetTextDecorations () != GetTextDecorations ())
		return false;
	
	lang0 = item->GetLanguage ();
	lang1 = GetLanguage ();
	
	if ((lang0 && !lang1) || (!lang0 && lang1))
		return false;
	
	if (lang0 && lang1 && strcmp (lang0, lang1) != 0)
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
Inline::UpdateFontDescription (const char *source, bool force)
{
	FontFamily *family = GetFontFamily ();
	bool changed = false;
	
	if (font->SetSource (source))
		changed = true;
	
	if (font->SetFamily (family ? family->source : NULL))
		changed = true;
	
	if (font->SetStretch (GetFontStretch ()->stretch))
		changed = true;
	
	if (font->SetWeight (GetFontWeight ()->weight))
		changed = true;
	
	if (font->SetStyle (GetFontStyle ()->style))
		changed = true;
	
	if (font->SetSize (GetFontSize ()))
		changed = true;
	
	//if (font->SetLanguage (GetLanguage ()))
	//	changed = true;
	
	if (force) {
		font->Reload ();
		return true;
	}
	
	return changed;
}

void
Inline::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((Inline *) closure)->DownloaderComplete ((Downloader *) sender);
}

void
Inline::DownloaderComplete (Downloader *downloader)
{
	FontManager *manager = Deployment::GetCurrent ()->GetFontManager ();
	char *resource, *filename;
	InternalDownloader *idl;
	const char *path;
	Uri *uri;
	
	// get the downloaded file path (enforces a mozilla workaround for files smaller than 64k)
	if (!(filename = downloader->GetDownloadedFilename (NULL)))
		return;
	
	g_free (filename);
	
	if (!(idl = downloader->GetInternalDownloader ()))
		return;
	
	if (!(idl->GetObjectType () == Type::FILEDOWNLOADER))
		return;
	
	uri = downloader->GetUri ();
	
	// If the downloaded file was a zip file, this'll get the path to the
	// extracted zip directory, else it will simply be the path to the
	// downloaded file.
	if (!(path = ((FileDownloader *) idl)->GetUnzippedPath ()))
		return;
	
	resource = uri->ToString ((UriToStringFlags) (UriHidePasswd | UriHideQuery | UriHideFragment));
	manager->AddResource (resource, path);
	g_free (resource);
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
// TextBlock
//

TextBlock::TextBlock ()
{
	SetObjectType (Type::TEXTBLOCK);
	
	downloaders = g_ptr_array_new ();
	layout = new TextLayout ();
	source = NULL;
	
	actual_height = 0.0;
	actual_width = 0.0;
	setvalue = true;
	was_set = false;
	dirty = true;
}

TextBlock::~TextBlock ()
{
	CleanupDownloaders (true);
	g_ptr_array_free (downloaders, true);
	
	delete layout;
}

void
TextBlock::CleanupDownloaders (bool all)
{
	Downloader *downloader;
	guint i;
	
	for (i = 0; i < downloaders->len; i++) {
		downloader = (Downloader *) downloaders->pdata[i];
		
		if (all || downloader != source) {
			downloader->RemoveHandler (Downloader::CompletedEvent, downloader_complete, this);
			downloader->Abort ();
			downloader->unref ();
		}
	}
	
	g_ptr_array_set_size (downloaders, 0);
	
	if (source && !all) {
		g_ptr_array_add (downloaders, source);
	} else {
		source = NULL;
	}
}

void
TextBlock::AddFontSource (Downloader *downloader)
{
	downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);
	g_ptr_array_add (downloaders, downloader);
	downloader->ref ();
	
	if (downloader->Started () || downloader->Completed ()) {
		if (downloader->Completed ())
			DownloaderComplete (downloader);
	} else {
		// This is what actually triggers the download
		downloader->Send ();
	}
}

void
TextBlock::SetFontSource (Downloader *downloader)
{
	CleanupDownloaders (true);
	source = downloader;
	
	if (downloader) {
		AddFontSource (downloader);
		return;
	}
	
	UpdateFontDescriptions (true);
	UpdateBounds (true);
	Invalidate ();
	dirty = true;
}

void
TextBlock::AddFontResource (const char *resource)
{
	FontManager *manager = Deployment::GetCurrent ()->GetFontManager ();
	Application *application = Application::GetCurrent ();
	Downloader *downloader;
	Surface *surface;
	char *path;
	Uri *uri;
	
	uri = new Uri ();
	
	if (!application || !uri->Parse (resource) || !(path = application->GetResourceAsPath (uri))) {
		if ((surface = GetSurface ()) && (downloader = surface->CreateDownloader ())) {
			downloader->Open ("GET", resource, XamlPolicy);
			AddFontSource (downloader);
			downloader->unref ();
		}
		
		delete uri;
		
		return;
	}
	
	manager->AddResource (resource, path);
	g_free (path);
	delete uri;
}

void
TextBlock::Render (cairo_t *cr, Region *region, bool path_only)
{
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

Point
TextBlock::GetTransformOrigin ()
{
	Point *user_xform_origin = GetRenderTransformOrigin ();
	return Point (actual_width * user_xform_origin->x, 
		      actual_height * user_xform_origin->y);
}

Size
TextBlock::ComputeActualSize ()
{
	Thickness padding = *GetPadding ();
	Size result = FrameworkElement::ComputeActualSize ();
	
	//if (dirty) {
	if (!LayoutInformation::GetLastMeasure (this)) {
		Size constraint = Size (INFINITY, INFINITY).Min (GetWidth (), GetHeight ());
		
		constraint = constraint.GrowBy (-padding);
		Layout (constraint);
	}
	//}
	
	result = Size (actual_width, actual_height);
	result = result.GrowBy (padding);
	
	return result;
};

Size
TextBlock::MeasureOverride (Size availableSize)
{
	//const char *text = layout->GetText ();
	Thickness padding = *GetPadding ();
	Size constraint;
	Size desired;
	
	//if (text && (!strcmp (text, "751 items") || !strncmp (text, "Use your mouse wheel to", 23))) {
	//	printf ("\nTextBlock::MeasureOverride(availableSize = { %f, %f })\n", availableSize.width, availableSize.height);
	//	printf ("\tText = \"%s\";\n", text);
	//}
	
	constraint = availableSize.GrowBy (-padding);
	Layout (constraint);
	
	desired = Size (actual_width, actual_height).GrowBy (padding);
	
	SetActualHeight (desired.height);
	SetActualWidth (desired.width);
	
	desired = desired.Min (availableSize);
	
	//if (text && (!strcmp (text, "751 items") || !strncmp (text, "Use your mouse wheel", 20)))
	//	printf ("\treturn { %f, %f };\n", desired.width, desired.height);
	
	return desired;
}

Size
TextBlock::ArrangeOverride (Size finalSize)
{
	//const char *text = layout->GetText ();
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
	HorizontalAlignment horiz = GetHorizontalAlignment ();
	
	if (!isnan (GetWidth ())) {
		arranged.width = GetWidth ();
	} else if (horiz == HorizontalAlignmentStretch) {
		arranged.width = finalSize.width;
	}

	if (!isnan (GetHeight ()))
		arranged.height = GetHeight ();

	arranged = arranged.Max (GetMinWidth (), GetMinHeight ());
	arranged = arranged.Min (GetMaxWidth (), GetMaxHeight ());

	layout->SetAvailableWidth (arranged.GrowBy (-padding).width);


	//arranged = arranged.Max (finalSize);
	
	//if (text && (!strcmp (text, "751 items") || !strncmp (text, "Use your mouse wheel", 20)))
	//	printf ("\treturn { %f, %f };\n", arranged.width, arranged.height);
	
	layout->SetAvailableWidth (arranged.GrowBy (-padding).width);
	
	return arranged;
}

void
TextBlock::UpdateLayoutAttributes ()
{
	InlineCollection *inlines = GetInlines ();
	TextLayoutAttributes *attrs;
	char *font_source = NULL;
	const char *text;
	int length = 0;
	Inline *item;
	List *runs;
	
	InvalidateMeasure ();
	InvalidateArrange ();
	runs = new List ();
	
	if (inlines != NULL) {
		if (source)
			font_source = source->GetUri ()->ToString ((UriToStringFlags) (UriHidePasswd | UriHideQuery | UriHideFragment));
		
		for (int i = 0; i < inlines->GetCount (); i++) {
			item = inlines->GetValueAt (i)->AsInline ();
			item->UpdateFontDescription (font_source, false);
			
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
				
				length += utf8_linebreak_len;
				break;
			default:
				break;
			}
		}
		
		if (inlines->GetCount () > 0)
			was_set = true;
		
		g_free (font_source);
	}
	
	layout->SetText (GetText (), length);
	layout->SetTextAttributes (runs);
}

bool
TextBlock::UpdateFontDescriptions (bool force)
{
	InlineCollection *inlines = GetInlines ();
	char *font_source = NULL;
	bool changed = false;
	Inline *item;
	
	if (inlines != NULL) {
		if (source)
			font_source = source->GetUri ()->ToString ((UriToStringFlags) (UriHidePasswd | UriHideQuery | UriHideFragment));
		
		for (int i = 0; i < inlines->GetCount (); i++) {
			item = inlines->GetValueAt (i)->AsInline ();
			if (item->UpdateFontDescription (font_source, force))
				changed = true;
		}
		
		if (changed)
			layout->ResetState ();
		
		g_free (font_source);
	}
	
	//ClearValue (TextBlock::ActualWidthProperty);
	//ClearValue (TextBlock::ActualHeightProperty);
	InvalidateMeasure ();
	InvalidateArrange ();
	UpdateBounds (true);
	dirty = true;
	
	return changed;
}

void
TextBlock::Layout (Size constraint)
{
	//const char *text = layout->GetText ();
	
	if (was_set && !GetValueNoDefault (TextBlock::TextProperty)) {
		// If the Text property had been set once upon a time,
		// but is currently empty, Silverlight seems to set
		// the ActualHeight property to the font height. See
		// bug #405514 for details.
		TextFontDescription *desc = new TextFontDescription ();
		FontFamily *family = GetFontFamily ();
		TextFont *font;
		
		desc->SetFamily (family ? family->source : NULL);
		desc->SetStretch (GetFontStretch ()->stretch);
		desc->SetWeight (GetFontWeight ()->weight);
		desc->SetStyle (GetFontStyle ()->style);
		desc->SetSize (GetFontSize ());
		
		font = desc->GetFont ();
		actual_height = font->Height ();
		actual_width = 0.0;
		delete desc;
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
	
	cairo_set_matrix (cr, &absolute_xform);
	layout->Render (cr, GetOriginPoint (), offset);
	
	if (moonlight_flags & RUNTIME_INIT_SHOW_TEXTBOXES) {
		cairo_set_source_rgba (cr, 0.0, 1.0, 0.0, 1.0);
		cairo_set_line_width (cr, 1);
		cairo_rectangle (cr, padding->left, padding->top, actual_width, actual_height);
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
			g_string_append_len (block, utf8_linebreak, utf8_linebreak_len);
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
	InlineCollection *inlines;
	Value *value;
	Run *run;
	
	// Note: calling GetValue() may cause the InlineCollection to be
	// autocreated, so we need to prevent reentrancy here.
	setvalue = false;
	
	value = GetValue (TextBlock::InlinesProperty);
	inlines = value->AsInlineCollection ();
	inlines->Clear ();
	
	if (text) {
		run = new Run ();
		run->SetAutogenerated (true);
		run->SetText (text);
		inlines->Add (run);
	} else {
		// setting text to null results in String.Empty
		SetValue (TextBlock::TextProperty, Value (""));
	}
	
	setvalue = true;
	
	return true;
}

void
TextBlock::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	//if (dirty)
	//	g_error ("entering changed while dirty");

	bool invalidate = true;
	if (args->GetProperty ()->GetOwnerType () != Type::TEXTBLOCK) {
		FrameworkElement::OnPropertyChanged (args, error);
		/*
		if (args->GetId () == FrameworkElement::WidthProperty) {
			//if (layout->SetMaxWidth (args->GetNewValue()->AsDouble ()))
			//	dirty = true;
			
			UpdateBounds (true);
		}
		*/
		return;
	}

	if (args->GetId () == TextBlock::FontFamilyProperty) {
		FontFamily *family = args->GetNewValue () ? args->GetNewValue ()->AsFontFamily () : NULL;
		char **families, *fragment;
		int i;
		
		CleanupDownloaders (false);
		
		if (family && family->source) {
			families = g_strsplit (family->source, ",", -1);
			for (i = 0; families[i]; i++) {
				g_strstrip (families[i]);
				if ((fragment = strchr (families[i], '#'))) {
					// the first portion of this string is the resource name...
					*fragment = '\0';
					AddFontResource (families[i]);
				}
			}
			g_strfreev (families);
		}
		
		if (UpdateFontDescriptions (false))
			dirty = true;
	} else if (args->GetId () == TextBlock::FontSizeProperty) {
		if (UpdateFontDescriptions (false))
			dirty = true;
	} else if (args->GetId () == TextBlock::FontStretchProperty) {
		if (UpdateFontDescriptions (false))
			dirty = true;
	} else if (args->GetId () == TextBlock::FontStyleProperty) {
		if (UpdateFontDescriptions (false))
			dirty = true;
	} else if (args->GetId () == TextBlock::FontWeightProperty) {
		if (UpdateFontDescriptions (false))
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
	} else if (args->GetId () == TextBlock::FontSourceProperty) {
		// FIXME: avoid crashing until this gets implemented (see DRT426-428)
		g_warning ("TextBlock::FontSourceProperty is not implemented");
	}
	
	if (invalidate) {
		if (dirty) {
			InvalidateMeasure ();
			InvalidateArrange ();
			UpdateBounds (true);
		}

		Invalidate ();
	}
	
	NotifyListenersOfPropertyChange (args, error);
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
	
	if (col != inlines) {
		FrameworkElement::OnCollectionChanged (col, args);
		return;
	}
	
	if (args->GetChangedAction () == CollectionChangedActionClearing)
		return;
	
	if (!setvalue) {
		// changes being handled elsewhere...
		return;
	}
	
	setvalue = false;
	// Note: this will cause UpdateLayoutAttributes() to be called in the TextProperty changed logic above
	SetValue (TextBlock::TextProperty, Value (GetTextInternal (inlines), true));
	setvalue = true;
	
	InvalidateMeasure ();
	InvalidateArrange ();
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
			char *font_source = NULL;
			
			if (source)
				font_source = source->GetUri ()->ToString ((UriToStringFlags) (UriHidePasswd | UriHideQuery | UriHideFragment));
			
			((Inline *) obj)->UpdateFontDescription (font_source, true);
			
			g_free (font_source);
		}
		
		// All non-Foreground property changes require
		// recalculating layout which can change the bounds.
		InvalidateMeasure ();
		InvalidateArrange ();
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
	((TextBlock *) closure)->DownloaderComplete ((Downloader *) sender);
}

void
TextBlock::DownloaderComplete (Downloader *downloader)
{
	FontManager *manager = Deployment::GetCurrent ()->GetFontManager ();
	char *resource, *filename;
	InternalDownloader *idl;
	const char *path;
	Uri *uri;
	
	dirty = true;
	InvalidateMeasure ();
	InvalidateArrange ();
	
	// get the downloaded file path (enforces a mozilla workaround for files smaller than 64k)
	if (!(filename = downloader->GetDownloadedFilename (NULL)))
		return;
	
	g_free (filename);
	
	if (!(idl = downloader->GetInternalDownloader ()))
		return;
	
	if (!(idl->GetObjectType () == Type::FILEDOWNLOADER))
		return;
	
	uri = downloader->GetUri ();
	
	// If the downloaded file was a zip file, this'll get the path to the
	// extracted zip directory, else it will simply be the path to the
	// downloaded file.
	if (!(path = ((FileDownloader *) idl)->GetUnzippedPath ()))
		return;
	
	resource = uri->ToString ((UriToStringFlags) (UriHidePasswd | UriHideQuery | UriHideFragment));
	manager->AddResource (resource, path);
	g_free (resource);
	
	if (UpdateFontDescriptions (true)) {
		dirty = true;
		
		UpdateBounds (true);
		Invalidate ();
	}
}
