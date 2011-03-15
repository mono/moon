/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * cpu.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOONLIGHT_CPU_H__
#define __MOONLIGHT_CPU_H__

class CPU {
private:
	static bool have_sse2;
	static bool have_mmx;
	static bool fetched;

	static void Fetch ();

public:
	static bool HaveMMX () { if (!fetched) Fetch (); return have_mmx; }
	static bool HaveSSE2 () { if (!fetched) Fetch (); return have_sse2; }
};

#endif /* __MOONLIGHT_CPU_H__ */
