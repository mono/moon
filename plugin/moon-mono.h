G_BEGIN_DECLS
#include <mono/jit/jit.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/mono-config.h>

extern MonoDomain   *moon_domain;
extern MonoAssembly *moon_boot_assembly;

gboolean    vm_init ();

G_END_DECLS
