#ifndef __MEDIA_H__
#define __MEDIA_H__

G_BEGIN_DECLS

#include "runtime.h"
#include "geometry.h"

class MediaBase : public FrameworkElement {
public:
	static DependencyProperty *SourceProperty;
	static DependencyProperty *StretchProperty;
	
	MediaBase () { }
	Value::Kind GetObjectType () { return Value::MEDIABASE; };
};

//Url    *media_base_get_source (MediaBase *media_base);
//void    media_base_set_source (MediaBase *media_base, Url *source);

Stretch media_base_get_stretch (MediaBase *media_base);
void    media_base_set_stretch (MediaBase *media_base, Stretch value);



class MediaElement : public MediaBase {
public:
	static DependencyProperty *IsMutedProperty;
	static DependencyProperty *AutoPlayProperty;
	static DependencyProperty *VolumeProperty;
	static DependencyProperty *BalanceProperty;
	static DependencyProperty *NaturalVideoHeightProperty;
	static DependencyProperty *NaturalVideoWidthProperty;
	static DependencyProperty *NaturalDurationProperty;
	static DependencyProperty *PositionProperty;
	static DependencyProperty *DownloadProgressProperty;
	static DependencyProperty *BufferingProgressProperty;
	static DependencyProperty *CurrentStateProperty;
	static DependencyProperty *BufferingTimeProperty;
	static DependencyProperty *MarkersProperty;
	static DependencyProperty *CanSeekProperty;
	static DependencyProperty *AttributesProperty;
	
	MediaElement ();
	Value::Kind GetObjectType () { return Value::MEDIAELEMENT; };
	
	void SetSource (DependencyObject *downloader, char *name);
	
	void Pause ();
	void Play ();
	void Stop ();
};


bool media_element_get_auto_play (MediaElement *media);
void media_element_set_auto_play (MediaElement *media, bool value);

double media_element_get_balance (MediaElement *media);
void media_element_set_balance (MediaElement *media, double value);

double media_element_get_buffering_progress (MediaElement *media);
void media_element_set_buffering_progress (MediaElement *media, double value);

//TimeSpan media_element_get_buffering_time (MediaElement *media);
//void media_element_set_buffering_time (MediaElement *media, TimeSpan value);

bool media_element_get_can_seek (MediaElement *media);

char *media_element_get_current_state (MediaElement *media);

double media_element_get_download_progress (MediaElement *media);
void media_element_set_download_progress (MediaElement *media, double value);

bool media_element_get_is_muted (MediaElement *media);
void media_element_set_is_muted (MediaElement *media, bool value);

//TimelineMarkerCollection *media_element_get_markers (MediaElement *media);
//void media_element_set_markers (MediaElement *media, TimelineMarkerCollection *value);

//Duration media_element_get_natural_duration (MediaElement *media);
//void media_element_set_natural_duration (MediaElement *media, Duration value);
double media_element_get_natural_video_height (MediaElement *media);
void media_element_set_natural_video_height (MediaElement *media, double value);

double media_element_get_natural_video_width (MediaElement *media);
void media_element_set_natural_video_width (MediaElement *media, double value);

//TimeSpan media_element_get_position (MediaElement *media);
//void media_element_set_position (MediaElement *media, TimeSpan value);

double media_element_get_volume (MediaElement *media);
void media_element_set_volume (MediaElement *media, double value);

