/*
 * media.cpp: 
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

#include "media.h"

// MediaBase

DependencyProperty *MediaBase::SourceProperty;
DependencyProperty *MediaBase::StretchProperty;

char *
media_base_get_source (MediaBase *media)
{
	return (char *) media->GetValue (MediaBase::SourceProperty)->AsString ();
}

void
media_base_set_source (MediaBase *media, char *value)
{
	media->SetValue (MediaBase::SourceProperty, Value (value));
}

Stretch
media_base_get_stretch (MediaBase *media)
{
	return (Stretch) media->GetValue (MediaBase::StretchProperty)->AsInt32 ();
}

void
media_base_set_stretch (MediaBase *media, Stretch value)
{
	media->SetValue (MediaBase::StretchProperty, Value (value));
}



// MediaElement

DependencyProperty *MediaElement::AutoPlayProperty;
DependencyProperty *MediaElement::BalanceProperty;
DependencyProperty *MediaElement::BufferingProgressProperty;
DependencyProperty *MediaElement::BufferingTimeProperty;
DependencyProperty *MediaElement::CanSeekProperty;
DependencyProperty *MediaElement::CurrentStateProperty;
DependencyProperty *MediaElement::DownloadProgressProperty;
DependencyProperty *MediaElement::IsMutedProperty;
DependencyProperty *MediaElement::MarkersProperty;
DependencyProperty *MediaElement::NaturalDurationProperty;
DependencyProperty *MediaElement::NaturalVideoHeightProperty;
DependencyProperty *MediaElement::NaturalVideoWidthProperty;
DependencyProperty *MediaElement::PositionProperty;
DependencyProperty *MediaElement::VolumeProperty;

DependencyProperty *MediaElement::AttributesProperty;


void
MediaElement::SetSource (DependencyObject *downloader, char *name)
{
	;
}

void
MediaElement::Pause ()
{
	;
}

void
MediaElement::Play ()
{
	;
}

void
MediaElement::Stop ()
{
	;
}

bool
media_element_get_auto_play (MediaElement *media)
{
	return (bool) media->GetValue (MediaElement::AutoPlayProperty)->AsBool ();
}

void
media_element_set_auto_play (MediaElement *media, bool value)
{
	media->SetValue (MediaElement::AutoPlayProperty, Value (value));
}

double
media_element_get_balance (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::BalanceProperty)->AsDouble ();
}

void
media_element_set_balance (MediaElement *media, double value)
{
	media->SetValue (MediaElement::BalanceProperty, Value (value));
}

double
media_element_get_buffering_progress (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::BufferingProgressProperty)->AsDouble ();
}

void
media_element_set_buffering_progress (MediaElement *media, double value)
{
	media->SetValue (MediaElement::BufferingProgressProperty, Value (value));
}

TimeSpan
media_element_get_buffering_time (MediaElement *media)
{
	return (TimeSpan) media->GetValue (MediaElement::BufferingTimeProperty)->AsInt64 ();
}

void
media_element_set_buffering_time (MediaElement *media, TimeSpan value)
{
	media->SetValue (MediaElement::BufferingTimeProperty, Value (value));
}

bool
media_element_get_can_seek (MediaElement *media)
{
	return (bool) media->GetValue (MediaElement::CanSeekProperty)->AsBool ();
}

char *
media_element_get_current_state (MediaElement *media)
{
	return (char *) media->GetValue (MediaElement::CurrentStateProperty)->AsString ();
}

double
media_element_get_download_progress (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::DownloadProgressProperty)->AsDouble ();
}

void
media_element_set_download_progress (MediaElement *media, double value)
{
	media->SetValue (MediaElement::DownloadProgressProperty, Value (value));
}

bool
media_element_get_is_muted (MediaElement *media)
{
	return (bool) media->GetValue (MediaElement::IsMutedProperty)->AsBool ();
}

void
media_element_set_is_muted (MediaElement *media, bool value)
{
	media->SetValue (MediaElement::IsMutedProperty, Value (value));
}

TimelineMarkerCollection *
media_element_get_markers (MediaElement *media)
{
	return (TimelineMarkerCollection *) media->GetValue (MediaElement::MarkersProperty)->AsTimelineMarkerCollection ();
}

void
media_element_set_markers (MediaElement *media, TimelineMarkerCollection *value)
{
	media->SetValue (MediaElement::MarkersProperty, Value (value));
}

Duration *
media_element_get_natural_duration (MediaElement *media)
{
	return (Duration *) media->GetValue (MediaElement::NaturalDurationProperty)->AsDuration ();
}

void
media_element_set_natural_duration (MediaElement *media, Duration value)
{
	media->SetValue (MediaElement::NaturalDurationProperty, Value (value));
}

double
media_element_get_natural_video_height (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::NaturalVideoHeightProperty)->AsDouble ();
}

void
media_element_set_natural_video_height (MediaElement *media, double value)
{
	media->SetValue (MediaElement::NaturalVideoHeightProperty, Value (value));
}

double
media_element_get_natural_video_width (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::NaturalVideoWidthProperty)->AsDouble ();
}

void
media_element_set_natural_video_width (MediaElement *media, double value)
{
	media->SetValue (MediaElement::NaturalVideoWidthProperty, Value (value));
}

TimeSpan
media_element_get_position (MediaElement *media)
{
	return (TimeSpan) media->GetValue (MediaElement::PositionProperty)->AsInt64 ();
}

void
media_element_set_position (MediaElement *media, TimeSpan value)
{
	media->SetValue (MediaElement::PositionProperty, Value (value));
}

double media_element_get_volume (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::VolumeProperty)->AsDouble ();
}

void
media_element_set_volume (MediaElement *media, double value)
{
	media->SetValue (MediaElement::VolumeProperty, Value (value));
}
