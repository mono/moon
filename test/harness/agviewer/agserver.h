

#include <glib-object.h>



typedef struct Agserver Agserver;
typedef struct AgserverClass AgserverClass;

GType agserver_get_type (void);


struct Agserver
{
	GObject parent;
};

struct AgserverClass
{
	GObjectClass parent;
};

#define AGSERVER_TYPE      (agserver_get_type ())

G_DEFINE_TYPE(Agserver, agserver, G_TYPE_OBJECT)


gboolean signal_shutdown (Agserver* server, GError** error);

