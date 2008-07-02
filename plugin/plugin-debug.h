/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-debug.h: debugging/inspection support in the plugin
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_PLUGIN_DEBUG_H__
#define __MOON_PLUGIN_DEBUG_H__

#include "plugin.h"

G_BEGIN_DECLS

void plugin_debug (PluginInstance *plugin);
void plugin_sources (PluginInstance *plugin);

G_END_DECLS

#endif /* __MOON_PLUGIN_DEBUG_H__ */
