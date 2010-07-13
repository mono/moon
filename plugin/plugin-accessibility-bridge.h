/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-accessibility-bridge.h: Glue between Moonlight.AtkBridge and NPAPI.
 *
 * Contact:
 *   Moonlight Accessibility List (mono-a11y@forge.novell.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef MOON_PLUGIN_ACCESSIBILITY_BRIDGE
#define MOON_PLUGIN_ACCESSIBILITY_BRIDGE

#if !INCLUDED_MONO_HEADERS
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoClass MonoClass;
typedef struct _MonoDomain MonoDomain;
typedef struct _MonoImage MonoImage;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoObject MonoObject;
typedef struct _MonoProperty MonoProperty;
#endif

#include <config.h>

#if PAL_GTK_WINDOWING
#include <atk/atk.h>
/* Requires AtkPlug introduced in Atk 1.30 */
#ifdef ATK_TYPE_PLUG
#define PAL_GTK_A11Y 1
#endif
#endif /* PAL_GTK_WINDOWING */

namespace Moonlight {

class AccessibilityBridge {
public:
	AccessibilityBridge ();

	void Initialize ();
	void Shutdown ();

	/* @GenerateCBinding,GeneratePInvoke */
	bool IsAccessibilityEnabled ();

#if PAL_GTK_A11Y
	AtkObject* GetRootAccessible ();
	char* GetPlugId (AtkObject *rootAccessible);
#endif

private:
#if PAL_GTK_A11Y
	static void StartAtkBridge ();
	static void ShutdownAtkBridge ();
#endif

	bool is_accessibility_enabled;
	MonoAssembly* bridge_asm;
	MonoClass* automation_bridge_class;
	MonoObject* automation_bridge;
};

#if PAL_GTK_A11Y

G_BEGIN_DECLS

#define MOON_TYPE_ATK_ROOT		(moon_atk_root_get_type ())
#define MOON_ATK_ROOT(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), MOON_TYPE_ATK_ROOT, MoonAtkRoot))
#define MOON_IS_ATK_ROOT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOON_TYPE_ATK_ROOT))
#define MOON_ATK_ROOT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), MOON_TYPE_ATK_ROOT, MoonAtkRootClass))
#define MOON_IS_ATK_ROOT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), MOON_TYPE_ATK_ROOT))
#define MOON_ATK_ROOT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), MOON_TYPE_ATK_ROOT, MoonAtkRootClass))

typedef struct _MoonAtkRoot		MoonAtkRoot;
typedef struct _MoonAtkRootClass	MoonAtkRootClass;

struct _MoonAtkRoot
{
	AtkPlug parent;
};

struct _MoonAtkRootClass
{
	AtkPlugClass parent_class;
};

GType moon_atk_root_get_type (void);

AtkObject* moon_atk_root_new ();

G_END_DECLS

#endif /* PAL_GTK_A11Y  */
};
#endif /* MOON_PLUGIN_ACCESSIBILITY_BRIDGE */
