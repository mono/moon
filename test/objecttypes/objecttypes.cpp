#include "glib.h"
#include "gtk/gtk.h"
#include "libmoon.h"

int
main()
{
	runtime_init (0);

	for (int i = 0; i < Type::LASTTYPE; i ++) {
		Type *t = Type::Find((Type::Kind)i);
		if (t == NULL)
			continue;

		DependencyObject *obj = type_create_instance (t);

		if (!obj)
			continue;

		if (obj->GetObjectType() != i) {
			fprintf (stderr, "type %s doesn't correctly call SetObjectType\n",
				 t->GetName());
		}
		  
	}
}
