/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * cpu.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include "cpu.h"

bool CPU::have_sse2 = false;
bool CPU::have_mmx = false;
bool CPU::fetched = false;

void
CPU::Fetch ()
{
	if (fetched)
		return;

	have_mmx = false;
	have_sse2 = false;

#if defined(__amd64__) && defined(__x86_64__)
	have_mmx = true;
	have_sse2 = true;
#elif HAVE_MMX
	int have_cpuid = 0;
	int features = 0;

	__asm__ __volatile__ (
		"pushfl;"
		"popl %%eax;"
		"movl %%eax, %%edx;"
		"xorl $0x200000, %%eax;"
		"pushl %%eax;"
		"popfl;"
		"pushfl;"
		"popl %%eax;"
		"xorl %%edx, %%eax;"
		"andl $0x200000, %%eax;"
		"movl %%eax, %0"
		: "=r" (have_cpuid)
		:
		: "%eax", "%edx"
	);

	if (have_cpuid) {
		__asm__ __volatile__ (
			"movl $0x0000001, %%eax;"
			"pushl %%ebx;"
			"cpuid;"
			"popl %%ebx;"
			"movl %%edx, %0;"
			: "=r" (features)
			:
			: "%eax", "%ecx"
		);

		have_mmx = features & 0x00800000;
		have_sse2 = features & 0x04000000;
	}
#endif

#if 0
	printf ("CPU::HaveMMX: %i\n", have_mmx);
	printf ("CPU::HaveSSE2: %i\n", have_sse2);
#endif

	fetched = true;
}
