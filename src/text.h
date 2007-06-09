#ifndef __TEXT_H__
#define __TEXT_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>
#include "runtime.h"

class Inline : public DependencyObject {
 public:
	static DependencyProperty* FontFamily;
	static DependencyProperty* FontSize;
	static DependencyProperty* FontStrech;
	static DependencyProperty* FontStyle;
	static DependencyProperty* FontWeight;
	static DependencyProperty* Foreground;
	static DependencyProperty* TextDecorations;
	
};



G_END_DECLS

#endif
