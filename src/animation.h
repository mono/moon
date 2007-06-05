#include "runtime.h"

G_BEGIN_DECLS

class Timeline;
class DependencyObject;

/* this merges WPF's Clock and ClockController */
class Clock : public EventObject {
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
	static gboolean tick_timeout (gpointer data);


	Timeline *timeline;
	gint64/*TimeSpan*/ start_time; /* the time we actually started.  used for computing CurrentTime */
};

class Timeline : public DependencyObject {
 public:
	Timeline ();

	void SetClock (Clock *new_clock);

	static DependencyProperty* AutoReverseProperty;
	static DependencyProperty* BeginTimeProperty;
	static DependencyProperty* DurationProperty;
	static DependencyProperty* FillBehaviorProperty;
	static DependencyProperty* RepeatBehaviorProperty;
	static DependencyProperty* SpeedRatioProperty;

 protected:
	virtual void ClockTimeChanged () { }

	static void clock_time_changed (gpointer data);
	Clock *clock;
};


class Animation : public Timeline {
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

	static DependencyProperty* TargetNameProperty;
	static DependencyProperty* TargetPropertyProperty;

	// XXX event Completed

	virtual void ClockTimeChanged ();

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

char* storyboard_child_get_target_property (Storyboard *sb,
					    DependencyObject *o);

void storyboard_child_set_target_name (Storyboard *sb,
				       DependencyObject *o,
				       char *targetName);

char* storyboard_child_get_target_name (Storyboard *sb,
					DependencyObject *o);

class DoubleAnimation : public Animation {
 public:

	DoubleAnimation ();

	static DependencyProperty* ByProperty;
	static DependencyProperty* FromProperty;
	static DependencyProperty* ToProperty;
};

void   double_animation_set_by (DoubleAnimation *da, double by);
double double_animation_get_by (DoubleAnimation *da);

void   double_animation_set_from (DoubleAnimation *da, double from);
double double_animation_get_from (DoubleAnimation *da);

void   double_animation_set_to (DoubleAnimation *da, double to);
double double_animation_get_to (DoubleAnimation *da);

G_END_DECLS
