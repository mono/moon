/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * textelement.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <cairo.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "textelement.h"
#include "deployment.h"
#include "runtime.h"
#include "color.h"
#include "utils.h"
#include "debug.h"
#include "uri.h"
#include "factory.h"

namespace Moonlight {

//
// TextElement
//

TextElement::TextElement ()
{
	SetObjectType (Type::TEXTELEMENT);
	font = new TextFontDescription ();
	downloaders = g_ptr_array_new ();
}

TextElement::~TextElement ()
{
	CleanupDownloaders ();
	g_ptr_array_free (downloaders, true);
	delete font;
}

void
TextElement::CleanupDownloaders ()
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
TextElement::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((TextElement *) closure)->DownloaderComplete ((Downloader *) sender);
}

void
TextElement::DownloaderComplete (Downloader *downloader)
{
	FontManager *manager = Deployment::GetCurrent ()->GetFontManager ();
	char *resource;
	const char *path;
	Uri *uri;
	
	uri = downloader->GetUri ();
	
	// If the downloaded file was a zip file, this'll get the path to the
	// extracted zip directory, else it will simply be the path to the
	// downloaded file.
	if (!(path = downloader->GetUnzippedPath ()))
		return;
	
	resource = uri->ToString ((UriToStringFlags) (UriHidePasswd | UriHideQuery | UriHideFragment));
	manager->AddResource (resource, path);
	g_free (resource);
}

void
TextElement::AddFontSource (Downloader *downloader)
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
TextElement::AddFontResource (const char *resource)
{
	FontManager *manager = Deployment::GetCurrent ()->GetFontManager ();
	Application *application = Application::GetCurrent ();
	Downloader *downloader;
	char *path;
	Uri *uri;
	
	uri = new Uri ();
	
	if (!application || !uri->Parse (resource) || !(path = application->GetResourceAsPath (GetResourceBase(), uri))) {
		if (IsAttached () && (downloader = GetDeployment ()->CreateDownloader ())) {
			downloader->Open ("GET", resource, FontPolicy);
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
TextElement::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType () != Type::TEXTELEMENT) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == TextElement::FontFamilyProperty) {
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
TextElement::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop && prop->GetId () == TextElement::ForegroundProperty) {
		// this isn't exactly what we want, I don't
		// think... but it'll have to do.
		NotifyListenersOfPropertyChange (prop, NULL);
	} else {
		DependencyObject::OnSubPropertyChanged (prop, obj, subobj_args);
	}
}

bool
TextElement::UpdateFontDescription (const FontResource *resource, bool force)
{
	FontFamily *family = GetFontFamily ();
	bool changed = false;
	
	if (font->SetResource (resource))
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
	
	if (font->SetLanguage (GetLanguage ()))
		changed = true;
	
	if (force) {
		font->Reload ();
		return true;
	}
	
	return changed;
}

TextPointer*
TextElement::GetContentStart ()
{
	return MoonUnmanagedFactory::CreateTextPointer (0, LogicalDirectionBackward);
}

TextPointer*
TextElement::GetContentEnd ()
{
	return MoonUnmanagedFactory::CreateTextPointer (-1, LogicalDirectionForward);
}

TextPointer*
TextElement::GetElementStart ()
{
	printf ("NIEX TextElement::GetElementStart ()");
	// FIXME this likely requires getting the parent, then getting a TextPointer just before @this
	return NULL;
}

TextPointer*
TextElement::GetElementEnd ()
{
	printf ("NIEX TextElement::GetElementEnd ()");
	// FIXME this likely requires getting the parent, then getting a TextPointer just after @this
	return NULL;
}

//
// Inline
//

Inline::Inline ()
{
	SetObjectType (Type::INLINE);
	autogen = false;
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


//
// LineBreak
//

LineBreak::LineBreak ()
{
	SetObjectType (Type::LINEBREAK);
}


//
// Run
//

Run::Run ()
{
	SetObjectType (Type::RUN);
}
 
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
// Block
//

Block::Block ()
{
	SetObjectType (Type::BLOCK);
}


//
// Paragraph
//

Paragraph::Paragraph ()
{
	SetObjectType (Type::PARAGRAPH);
}


//
// Section
//

Section::Section ()
{
	SetObjectType (Type::SECTION);
}


//
// Span
//

Span::Span ()
{
	SetObjectType (Type::SPAN);
}


//
// Bold
//

Bold::Bold ()
{
	SetObjectType (Type::BOLD);
	
	// Yes, this is supposed to be locally set in the ctor.
	FontWeight weight (FontWeightsBold);
	SetFontWeight (&weight);
}


//
// Italic
//

Italic::Italic ()
{
	SetObjectType (Type::ITALIC);
	
	FontStyle style (FontStylesItalic);
	SetFontStyle (&style);
}


//
// Underline
//

Underline::Underline ()
{
	SetObjectType (Type::UNDERLINE);
	
	SetTextDecorations (TextDecorationsUnderline);
}


//
// Hyperlink
//

Hyperlink::Hyperlink ()
{
	SetObjectType (Type::HYPERLINK);
	SetTextDecorations (TextDecorationsUnderline);

	// XXX these should happen here, but since they're inherited
	// properties, they end up causing a boatload of problems with
	// our forcing of managed refs.  They've been moved to
	// Hyperlink.Initialize in managed code.
	//
	// SetMouseOverForeground (new SolidColorBrush ("black"));
	// SetForeground (new SolidColorBrush ("#FF337CBB"));
}


//
// InlineUIContainer
//

InlineUIContainer::InlineUIContainer ()
{
	SetObjectType (Type::INLINEUICONTAINER);
}

};
