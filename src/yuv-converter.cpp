/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * yuv-converter.cpp: YUV2RGB converters for the pipeline
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib.h>

#include <stdlib.h>

#include "yuv-converter.h"

/* R = 1.164 * (Y - 16)		+ 1.596 * (V - 128)
 * G = 1.164 * (Y - 16)		- 0.813 * (V - 128)	- 0.391 * (U - 128)	
 * B = 1.164 * (Y - 16)					+ 2.018 * (U - 128)
 *
 * R V coefficient = 1.596*64 = 102 = 0x66
 * G V coefficient = 0.813*64 =  52 = 0x34 (-ve) == 0xFFCC
 * G U coefficient = 0.391*64 =  25 = 0x19 (-ve) == 0xFFE7
 * B U coefficient = 2.018*64 = 129 = 0x81
 * Y coefficient   = 1.164*64 =  74 = 0x4a
 */

#define RED_V_C 0x0066006600660066ULL
#define GREEN_V_C 0xffccffccffccffccULL
#define GREEN_U_C 0xffe7ffe7ffe7ffe7ULL
#define BLUE_U_C 0x0081008100810081ULL
#define Y_C 0x004a004a004a004aULL
#define UV_128 0x0080008000800080ULL
#define Y_16 0x1010101010101010ULL
#define ALPHA_MASK 0xFFFFFFFFFFFFFFFFULL

#if HAVE_SSE2 || HAVE_MMX
static const guint64 simd_table [16] __attribute__ ((aligned (16))) = {
									RED_V_C, RED_V_C,
									GREEN_V_C, GREEN_V_C,
									GREEN_U_C, GREEN_U_C,
									BLUE_U_C, BLUE_U_C,
									Y_C, Y_C,
									UV_128, UV_128,
									Y_16, Y_16,
									ALPHA_MASK, ALPHA_MASK,
};

#define PREFETCH(memory) do {			\
		__asm__ __volatile__ (		\
			"prefetchnta (%0);"	\
		: : "r" (memory));		\
	} while (0);

#if defined(__x86_64__)
#define ALIGN_CMP_REG "rax"
#else
#define ALIGN_CMP_REG "eax"
#endif
	
#define CALC_COLOR_MODIFIERS(mov_instr, reg_type, alignment, align_reg, u, v, coeff_storage) do {					\
			__asm__ __volatile__ (												\
				"mov %0, %%"align_reg";"										\
				"and $"alignment", %%"align_reg";"									\
				"test %%"align_reg", %%"align_reg";"									\
				"je 1f;"												\
																	\
				mov_instr " 48(%2), %%"reg_type"2;"			/* restore Dred */				\
				mov_instr " 64(%2), %%"reg_type"3;"			/* restore Dgreen */				\
				mov_instr " 80(%2), %%"reg_type"1;"			/* restore Dblue */				\
																	\
				mov_instr " %%"reg_type"2, (%2);"			/* backup Dred */				\
				mov_instr " %%"reg_type"3, 16(%2);"			/* backup Dgreen */				\
				mov_instr " %%"reg_type"1, 32(%2);"			/* backup Dblue */				\
																	\
				"jmp 2f;"												\
																	\
				"1:"													\
				"pxor %%"reg_type"7, %%"reg_type"7;"									\
																	\
				mov_instr " (%0), %%"reg_type"1;"									\
				mov_instr " (%1), %%"reg_type"2;"									\
																	\
				mov_instr " %%"reg_type"1, %%"reg_type"5;"								\
				mov_instr " %%"reg_type"2, %%"reg_type"6;"								\
																	\
				"punpckhbw %%"reg_type"7, %%"reg_type"5;"								\
				"punpckhbw %%"reg_type"7, %%"reg_type"6;"								\
																	\
				"punpcklbw %%"reg_type"7, %%"reg_type"1;"								\
				"punpcklbw %%"reg_type"7, %%"reg_type"2;"								\
																	\
				mov_instr " 80(%3), %%"reg_type"7;"									\
																	\
				"psubsw %%"reg_type"7, %%"reg_type"5;"			/* U[hi] = U[hi] - 128 */			\
				"psubsw %%"reg_type"7, %%"reg_type"6;"			/* V[hi] = V[hi] - 128 */			\
																	\
				"psubsw %%"reg_type"7, %%"reg_type"1;"			/* U[lo] = U[lo] - 128 */			\
				"psubsw %%"reg_type"7, %%"reg_type"2;"			/* V[lo] = V[lo] - 128 */			\
																	\
				mov_instr " %%"reg_type"5, %%"reg_type"3;"								\
				mov_instr " %%"reg_type"6, %%"reg_type"4;"								\
																	\
				mov_instr " 32(%3), %%"reg_type"7;"									\
				"pmullw %%"reg_type"7, %%"reg_type"3;"			/* calculate Ugreen[hi] */			\
				"psraw $6, %%"reg_type"3;"				/* Ugreen[hi] = Ugreen[hi] / 64 */		\
				mov_instr " 16(%3), %%"reg_type"7;"									\
				"pmullw %%"reg_type"7, %%"reg_type"4;"			/* calculate Vgreen[hi] */			\
				"psraw $6, %%"reg_type"4;"				/* Vgreen[hi] = Vgreen[hi] / 64 */		\
				"paddsw %%"reg_type"4, %%"reg_type"3;"			/* Dgreen[hi] = Ugreen[hi] + Vgreen[hi] */	\
																	\
				mov_instr " %%"reg_type"3, 64(%2);"			/* backup Dgreen[hi] (clobbered) */		\
																	\
				mov_instr " %%"reg_type"1, %%"reg_type"3;"								\
				mov_instr " %%"reg_type"2, %%"reg_type"4;"								\
																	\
				mov_instr " 32(%3), %%"reg_type"7;"									\
				"pmullw %%"reg_type"7, %%"reg_type"3;"			/* calculate Ugreen[lo] */			\
				"psraw $6, %%"reg_type"3;"				/* Ugreen[lo] = Ugreen[lo] / 64 */		\
				mov_instr " 16(%3), %%"reg_type"7;"									\
				"pmullw %%"reg_type"7, %%"reg_type"4;"			/* calculate Vgreen[lo] */			\
				"psraw $6, %%"reg_type"4;"				/* Vgreen[lo] = Vgreen[lo] / 64 */		\
				"paddsw %%"reg_type"4, %%"reg_type"3;"			/* Dgreen[lo] = Ugreen[lo] + Vgreen[lo] */	\
																	\
				mov_instr " 48(%3), %%"reg_type"7;"									\
				"pmullw %%"reg_type"7, %%"reg_type"5;"			/* calculate Dblue[hi] */			\
				"psraw $6, %%"reg_type"5;"				/* Dblue[hi] = Dblue[hi] / 64 */		\
				"pmullw %%"reg_type"7, %%"reg_type"1;"			/* calculate Dblue[lo] */			\
				"psraw $6, %%"reg_type"1;"				/* Dblue[lo] = Dblue[lo] / 64 */		\
																	\
				mov_instr " (%3), %%"reg_type"7;"									\
				"pmullw %%"reg_type"7, %%"reg_type"6;"			/* calculate Dred[hi] */			\
				"psraw $6, %%"reg_type"6;"				/* Dred[hi] = Dred[hi] / 64 */			\
				"pmullw %%"reg_type"7, %%"reg_type"2;"			/* calculate Dred[lo] */			\
				"psraw $6, %%"reg_type"2;"				/* Dred[lo] = Dred[lo] / 64 */			\
																	\
				mov_instr " %%"reg_type"6, 48(%2);"			/* backup Dred[hi] */				\
				mov_instr " %%"reg_type"5, 80(%2);"			/* backup Dblue[hi] */				\
																	\
				mov_instr " %%"reg_type"2, 0(%2);"			/* backup Dred[lo] */				\
				mov_instr " %%"reg_type"3, 16(%2);"			/* backup Dgreen[lo] */				\
				mov_instr " %%"reg_type"1, 32(%2);"			/* backup Dblue[lo] */				\
				"2:"													\
				: : "r" (u), "r" (v), "r" (coeff_storage), "r" (&simd_table) : "%"align_reg);				\
	} while (0);

#define RESTORE_COLOR_MODIFIERS(mov_instr, reg_type, coeff_storage) do {				\
		__asm__ __volatile__ (									\
			mov_instr " (%0), %%"reg_type"2;"			/* restore Dred */	\
			mov_instr " 16(%0), %%"reg_type"3;"			/* restore Dgreen */	\
			mov_instr " 32(%0), %%"reg_type"1;"			/* restore Dblue */	\
			: : "r" (coeff_storage));							\
	} while (0);

#define YUV2RGB_INTEL_SIMD(mov_instr, reg_type, output_offset1, output_offset2, output_offset3, y_plane, dest) do { 	\
		__asm__ __volatile__ ( 											\
			mov_instr " (%0), %%"reg_type"0;"		/* Load Y plane into r0 */			\
			mov_instr " 96(%2), %%"reg_type"7;"		/* Load 16 into r7 */				\
			"psubusb %%"reg_type"7, %%"reg_type"0;"		/* Y = Y - 16 */				\
															\
			mov_instr " %%"reg_type"0, %%"reg_type"4;"	/* r4 == r0 */					\
															\
			"psllw $8, %%"reg_type"0;"			/* r0 [00 Y0 00 Y2 ...] */			\
			"psrlw $8, %%"reg_type"0;"			/* r0 [Y0 00 Y2 00 ...] */			\
			"psrlw $8, %%"reg_type"4;"			/* r4 [Y1 00 Y3 00 ...] */			\
															\
			mov_instr " 64(%2), %%"reg_type"7;"								\
			"pmullw %%"reg_type"7, %%"reg_type"0;"		/* calculate Y*Yc[even] */			\
			"pmullw %%"reg_type"7, %%"reg_type"4;"		/* calculate Y*Yc[odd] */			\
			"psraw $6, %%"reg_type"0;"			/* Yyc[even] = Yyc[even] / 64 */		\
			"psraw $6, %%"reg_type"4;"			/* Yyc[odd] = Yyc[odd] / 64 */			\
															\
			mov_instr " %%"reg_type"2, %%"reg_type"6;"							\
			mov_instr " %%"reg_type"3, %%"reg_type"7;"							\
			mov_instr " %%"reg_type"1, %%"reg_type"5;"							\
															\
			"paddsw %%"reg_type"0, %%"reg_type"2;"		/* CY[even] + DR */				\
			"paddsw %%"reg_type"0, %%"reg_type"3;"		/* CY[even] + DG */				\
			"paddsw %%"reg_type"0, %%"reg_type"1;"		/* CY[even] + DB */				\
															\
			"paddsw %%"reg_type"4, %%"reg_type"6;"		/* CY[odd] + DR */				\
			"paddsw %%"reg_type"4, %%"reg_type"7;"		/* CY[odd] + DG */				\
			"paddsw %%"reg_type"4, %%"reg_type"5;"		/* CY[odd] + DB */				\
															\
			"packuswb %%"reg_type"2, %%"reg_type"2;"	/* Clamp RGB to [0-255] */			\
			"packuswb %%"reg_type"3, %%"reg_type"3;"							\
			"packuswb %%"reg_type"1, %%"reg_type"1;"							\
															\
			"packuswb %%"reg_type"6, %%"reg_type"6;"							\
			"packuswb %%"reg_type"7, %%"reg_type"7;"							\
			"packuswb %%"reg_type"5, %%"reg_type"5;"							\
															\
			"punpcklbw %%"reg_type"6, %%"reg_type"2;"	/* r2 [R0 R1 R2 R3 ...] */			\
			"punpcklbw %%"reg_type"7, %%"reg_type"3;"	/* r3 [G0 G1 G2 G3 ...] */			\
			"punpcklbw %%"reg_type"5, %%"reg_type"1;"	/* r1 [B0 B1 B2 B3 ...] */			\
															\
			mov_instr " %%"reg_type"2, %%"reg_type"5;"	/* copy RGB */					\
			mov_instr " %%"reg_type"3, %%"reg_type"7;"							\
			mov_instr " %%"reg_type"1, %%"reg_type"6;"							\
															\
			mov_instr " 112(%2), %%"reg_type"4;"								\
			"punpcklbw %%"reg_type"2, %%"reg_type"1;"	/* r1 [B0 R0 B1 R1 ...] */			\
			"punpcklbw %%"reg_type"4, %%"reg_type"3;"	/* r4 [G0 FF G1 FF ...] */			\
															\
			mov_instr " %%"reg_type"1, %%"reg_type"0;"	/* r3 [G0 FF G1 FF ...] */			\
															\
			"punpcklbw %%"reg_type"3, %%"reg_type"1;"	/* r2 [B0 G0 R0 FF B1 G1 R1 FF ...] */		\
			"punpckhbw %%"reg_type"3, %%"reg_type"0;"	/* r3 [B2 G2 R2 FF B3 G3 R3 FF ...] */		\
															\
			mov_instr " %%"reg_type"1, (%1);"		/* output BGRA */	 			\
			mov_instr " %%"reg_type"0, "output_offset1"(%1);"						\
															\
			"punpckhbw %%"reg_type"5, %%"reg_type"6;"							\
			"punpckhbw %%"reg_type"4, %%"reg_type"7;"							\
															\
			mov_instr " %%"reg_type"6, %%"reg_type"0;"							\
															\
			"punpcklbw %%"reg_type"7, %%"reg_type"6;"							\
			"punpckhbw %%"reg_type"7, %%"reg_type"0;"							\
															\
			mov_instr " %%"reg_type"6, "output_offset2"(%1);"						\
			mov_instr " %%"reg_type"0, "output_offset3"(%1);"						\
			: : "r" (y_plane), "r" (dest), "r" (&simd_table));						\
	} while (0);
#endif

#if HAVE_SSE2
#define YUV2RGB_SSE(y_plane, dest) YUV2RGB_INTEL_SIMD("movdqa", "xmm", "16", "32", "48", y_plane, dest)
#endif

#if HAVE_MMX
#define YUV2RGB_MMX(y_plane, dest) YUV2RGB_INTEL_SIMD("movq", "mm", "8", "16", "24", y_plane, dest)
#endif

static inline void YUV444ToBGRA(guint8 Y, guint8 U, guint8 V, guint8 *dst)
{
	dst[2] = CLAMP((298 * (Y - 16) + 409 * (V - 128) + 128) >> 8, 0, 255);
	dst[1] = CLAMP((298 * (Y - 16) - 100 * (U - 128) - 208 * (V - 128) + 128) >> 8, 0, 255);
	dst[0] = CLAMP((298 * (Y - 16) + 516 * (U - 128) + 128) >> 8, 0, 255);
	dst[3] = 0xFF;
}

/*
 * YUVConverterInfo
 */

bool
YUVConverterInfo::Supports (MoonPixelFormat input, MoonPixelFormat output)
{
	return input != MoonPixelFormatNone && output != MoonPixelFormatNone;
}

IImageConverter*
YUVConverterInfo::Create (Media* media, VideoStream* stream)
{
	return new YUVConverter (media, stream);
}

/*
 * YUVConverter
 */

YUVConverter::YUVConverter (Media* media, VideoStream* stream) : IImageConverter (Type::YUVCONVERTER, media, stream)
{
#if defined(__amd64__) && defined(__x86_64__)
	have_mmx = true;
	have_sse2 = true;
#else
#  if HAVE_MMX
	int have_cpuid = 0;
	int features = 0;

	have_mmx = false;
	have_sse2 = false;

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
			: "%eax"
		);

		have_mmx = features & 0x00800000;
		have_sse2 = features & 0x04000000;
	}
#  else
	have_mmx = false;
	have_sse2 = false;
#  endif
#endif
	if (posix_memalign ((void **)(&rgb_uv), 16, 96))
		rgb_uv = NULL;
}

YUVConverter::~YUVConverter ()
{
	free(rgb_uv);
}

MediaResult
YUVConverter::Open ()
{
	if (input_format == MoonPixelFormatNone) {
		Media::Warning (MEDIA_CONVERTER_ERROR, "Invalid input format.");
		return MEDIA_CONVERTER_ERROR;
	}
	
	if (output_format == MoonPixelFormatNone) {
		Media::Warning (MEDIA_CONVERTER_ERROR, "Invalid output format.");
		return MEDIA_CONVERTER_ERROR;
	}
	
	return MEDIA_SUCCESS;
}

MediaResult
YUVConverter::Convert (guint8 *src[], int srcStride[], int srcSlideY, int srcSlideH, guint8* dest[], int dstStride [])
{
	guint8 *y_row1 = src[0];
	guint8 *y_row2 = src[0]+srcStride[0];

	guint8 *u_plane = src[1];
	guint8 *v_plane = src[2];
	
	guint8 *dest_row1 = dest[0];
	guint8 *dest_row2 = dest[0]+dstStride[0];

	int i, j;

	int width = dstStride[0] >> 2;
	int height = srcSlideH;
	int pad = 0;
	bool aligned = true;
	
	if (width != srcStride[0]) {
		pad = (srcStride[0] - width);
		if (pad % 16) {
			g_warning ("This video has padding that prevents us from doing aligned SIMD operations on it.");
			aligned = false;
		}
	}
	
	if (rgb_uv == NULL && posix_memalign ((void **)(&rgb_uv), 16, 96) != 0) {
		g_warning ("Could not allocate memory for YUVConverter");
		return MEDIA_OUT_OF_MEMORY;
	}
	
#if HAVE_SSE2
	if (have_sse2 && aligned) {
		for (i = 0; i < height >> 1; i ++, y_row1 += srcStride[0], y_row2 += srcStride[0], dest_row1 += dstStride[0], dest_row2 += dstStride[0]) {
			for (j = 0; j < width >> 4; j ++, y_row1 += 16, y_row2 += 16, u_plane += 8, v_plane += 8, dest_row1 += 64, dest_row2 += 64) {
				PREFETCH(y_row1);
				CALC_COLOR_MODIFIERS("movdqa", "xmm", "15", ALIGN_CMP_REG, u_plane, v_plane, rgb_uv);
			
				YUV2RGB_SSE(y_row1, dest_row1);
			
				PREFETCH(y_row2);
				RESTORE_COLOR_MODIFIERS("movdqa", "xmm", rgb_uv);

				YUV2RGB_SSE(y_row2, dest_row2);
			}
			y_row1 += pad;
			y_row2 += pad;
			u_plane += pad >> 1;
			v_plane += pad >> 1;
		}
	} else {
#endif
#if HAVE_MMX
		if (have_mmx && aligned) {
			for (i = 0; i < height >> 1; i ++, y_row1 += srcStride[0], y_row2 += srcStride[0], dest_row1 += dstStride[0], dest_row2 += dstStride[0]) {
				for (j = 0; j <  width >> 3; j ++, y_row1 += 8, y_row2 += 8, u_plane += 4, v_plane += 4, dest_row1 += 32, dest_row2 += 32) {
					PREFETCH(y_row1);
					CALC_COLOR_MODIFIERS("movq", "mm", "7", ALIGN_CMP_REG, u_plane, v_plane, rgb_uv);

					YUV2RGB_MMX(y_row1, dest_row1);

					PREFETCH(y_row2);
					RESTORE_COLOR_MODIFIERS("movq", "mm", rgb_uv);

					YUV2RGB_MMX(y_row2, dest_row2);
				}
				y_row1 += pad;
				y_row2 += pad;
				u_plane += pad >> 1;
				v_plane += pad >> 1;
			}
			__asm__ __volatile__ ("emms");
		} else {
#endif
			for (i = 0; i < height >> 1; i ++, y_row1 += srcStride[0], y_row2 += srcStride[0], dest_row1 += dstStride[0], dest_row2 += dstStride[0]) {
				for (j = 0; j < width >> 1; j ++, dest_row1 += 8, dest_row2 += 8, y_row1 += 2, y_row2 += 2, u_plane += 1, v_plane += 1) {
					YUV444ToBGRA (*y_row1, *u_plane, *v_plane, dest_row1);
					YUV444ToBGRA (y_row1[1], *u_plane, *v_plane, (dest_row1+4));
			
					YUV444ToBGRA (*y_row2, *u_plane, *v_plane, dest_row2);
					YUV444ToBGRA (y_row2[1], *u_plane, *v_plane, (dest_row2+4));
				}
				y_row1 += pad;
				y_row2 += pad;
				u_plane += pad >> 1;
				v_plane += pad >> 1;
			}
#if HAVE_MMX
		}
#endif
#if HAVE_SSE2
	}
#endif
	return MEDIA_SUCCESS;
}
