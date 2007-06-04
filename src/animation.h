#include "runtime.h"

G_BEGIN_DECLS

class Timeline;
class DependencyObject;

/* this merges WPF's Clock and ClockController */
class Clock {
 public:
  Clock (Timeline *timeline);
  Clock (Timeline *timeline, Clock *clock);

  void Begin ();
  void Pause ();
  void Remove ();
  void Resume ();
  void Seek (gint64/*TimeSpan*/ timespan);
  void SeekAlignedToLastTick ();
  void SkipToFill ();
  void Stop ();

  double GetCurrentProgress ();
  gint64 GetCurrentTime ();

  static long current_global_time;


  Clock *parent_clock;
  GList *child_clocks;

  gint tick_id;
  void Tick ();
  static gboolean StaticTick (gpointer data);


  Timeline *timeline;
  gint64/*TimeSpan*/ start_time; /* the time we actually started.  used for computing CurrentTime */
};

class Timeline /*: public DependencyObject */ {
 public:
  Timeline () { }

  /* we use these dependency properties:
     bool AutoReverse
     TimeSpan? BeginTime
     Duration Duration
     FillBehavior FillBehavior
     RepeatBehavior RepeatBehavior
     double SpeedRatio
  */

  Clock *clock;
};

class Animation : Timeline {
};

class TimelineGroup : public Timeline {
 public:
  TimelineGroup ();

  /* we use these dependency properties:
     Timeline Children - XXX shouldn't that be TimelineCollection?
  */
  void AddChild (Timeline *child);
  void RemoveChild (Timeline *child);

  GList *child_timelines;
};

void timeline_group_add_child (TimelineGroup *group, Timeline *child);
void timeline_group_remove_child (TimelineGroup *group, Timeline *child);


class ParallelTimeline : public TimelineGroup {
 public:
  ParallelTimeline () { }
};


class Storyboard : public ParallelTimeline {
 public:
  Storyboard ();

  void Begin ();
  void Pause ();
  void Resume ();
  void Seek (/*Timespan*/gint64 timespan);
  void Stop ();

  /* we use these attached properties for our children
     string targetProperty
     string targetName
   */

  // XXX event Completed

 private:
  gboolean Tick ();
  static gboolean storyboard_tick (gpointer data);
};

void storyboard_begin (Storyboard *sb);
void storyboard_pause (Storyboard *sb);
void storyboard_resume (Storyboard *sb);
void storyboard_seek (Storyboard *sb, /*Timespan*/gint64 timespan);
void storyboard_stop (Storyboard *sb);

void storyboard_child_set_target_property (Storyboard *sb,
					   DependencyObject *o,
					   char *targetProperty);

void storyboard_child_set_target_name (Storyboard *sb,
				       DependencyObject *o,
				       char *targetName);

class DoubleAnimation : public Animation {
 public:

	DoubleAnimation () { }

	/* these are dependency properties:
	   double? By
	   double? From
	   double? To
	*/
};

void   double_animation_set_by (DoubleAnimation *da, double by);
double double_animation_get_by (DoubleAnimation *da);

void   double_animation_set_from (DoubleAnimation *da, double from);
double double_animation_get_from (DoubleAnimation *da);

void   double_animation_set_to (DoubleAnimation *da, double to);
double double_animation_get_to (DoubleAnimation *da);

G_END_DECLS
