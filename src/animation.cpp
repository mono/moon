#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>

//#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
//#endif

#include "anim.h"

/*
 * Magic number to convert a time which is relative to
 * Jan 1, 1970 into a value which is relative to Jan 1, 0001.
 */
#define EPOCH_ADJUST    ((guint64)62135596800LL)

static gint64
get_now ()
{
        struct timeval tv;
        gint64 res;

        if (gettimeofday (&tv, NULL) == 0) {
                res = (((gint64)tv.tv_sec + EPOCH_ADJUST)* 1000000 + tv.tv_usec)*10;
                return res;
        }

	// XXX error
	return 0;
}




Clock::Clock (Timeline *tl)
  : timeline (tl), parent_clock (NULL), child_clocks (NULL)
{
}

Clock::Clock (Timeline *tl, Clock *clock)
  : timeline (tl), parent_clock (clock), child_clocks (NULL)
{
  parent_clock->child_clocks = g_list_prepend (parent_clock->child_clocks, clock);
}

void
Clock::Begin ()
{
  if (parent_clock != NULL) {
    /* XXX error */
    return;
  }
    
  start_time = get_now();
  tick_id = gtk_timeout_add (50 /* something suitably small */, Clock::StaticTick, this);

  for (GList *l = child_clocks; l; l = l->next) {
    ((Clock*)l->data)->Begin();
  }
}

gboolean
Clock::StaticTick (gpointer data)
{
  ((Clock*)data)->Tick ();
  return true;
}

void
Clock::Tick ()
{
  for (GList *l = child_clocks; l; l = l->next) {
    ((Clock*)l->data)->Tick();
  }
}

void
Clock::Pause ()
{
  if (parent_clock != NULL) {
    /* XXX error */
    return;
  }

  g_source_remove (tick_id);
}

void
Clock::Remove ()
{
}

void
Clock::Resume ()
{
  if (parent_clock != NULL) {
    /* XXX error */
    return;
  }

  tick_id = gtk_timeout_add (50 /* something suitably small */, Clock::StaticTick, this);
}

void
Clock::Seek (/*Timespan*/gint64 timespan)
{
  if (parent_clock != NULL) {
    /* XXX error */
    return;
  }

}

void
Clock::SeekAlignedToLastTick ()
{
  if (parent_clock != NULL) {
    /* XXX error */
    return;
  }

}

void
Clock::SkipToFill ()
{
  if (parent_clock != NULL) {
    /* XXX error */
    return;
  }

}

void
Clock::Stop ()
{
  if (parent_clock != NULL) {
    /* XXX error */
    return;
  }

  g_source_remove (tick_id);
}

double
Clock::GetCurrentProgress ()
{
  gint64/*TimeSpan*/ current_time = GetCurrentTime ();
  gint64 duration = do_get_value (timeline, "Timeline.Duration");

  return (double)current_time / duration;
}

gint64
Clock::GetCurrentTime ()
{
  return get_now () - start_time;
}




/* timeline group */

TimelineGroup::TimelineGroup ()
  : child_timelines (NULL)
{
}

void
TimelineGroup::AddChild (Timeline *child)
{
  child_timelines = g_list_prepend (child_timelines, child);

  if (child->clock)
    delete child->clock;

  child->clock = new Clock (child, this->clock);
}

void
TimelineGroup::RemoveChild (Timeline *child)
{
  /* nada for now */
}

void
timeline_group_add_child (TimelineGroup *group, Timeline *child)
{
  group->AddChild (child);
}

void
timeline_group_remove_child (TimelineGroup *group, Timeline *child)
{
  group->RemoveChild (child);
}


/* storyboard */

Storyboard::Storyboard ()
{
}

void
Storyboard::Begin ()
{
  clock->Begin ();
}

void
Storyboard::Pause ()
{
  clock->Pause ();
}

void
Storyboard::Resume ()
{
  clock->Resume ();
}

void
Storyboard::Seek (/*Timespan*/gint64 timespan)
{
  clock->Seek (timespan);
}

void
Storyboard::Stop ()
{
  clock->Stop ();
}


void
storyboard_begin (Storyboard *sb)
{
  sb->Begin ();
}

void
storyboard_pause (Storyboard *sb)
{
  sb->Pause ();
}

void
storyboard_resume (Storyboard *sb)
{
  sb->Resume ();
}

void
storyboard_seek (Storyboard *sb, /*Timespan*/gint64 timespan)
{
  sb->Seek (timespan);
}

void
storyboard_stop (Storyboard *sb)
{
  sb->Stop ();
}

void
storyboard_child_set_target_property (Storyboard *sb,
				      DependencyObject *o,
				      char *targetProperty)
{
  do_set_value (o, "Storyboard.targetProperty", targetProperty);
}

void
storyboard_child_set_target_name (Storyboard *sb,
				  DependencyObject *o,
				  char *targetName)
{
  do_set_value (o, "Storyboard.targetName", targetName);
}
