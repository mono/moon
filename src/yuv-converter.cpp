/*
 * yuv-converter.cpp: YUV2RGB converters for the pipeline
 *
 * Author:
 *   Geoff Norton (gnorton@novell.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib.h>
#include <unistd.h>

G_BEGIN_DECLS
#include <stdint.h>
#include <limits.h>
G_END_DECLS

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
static const uint64_t simd_table [16] __attribute__ ((aligned (32))) = {
									RED_V_C, RED_V_C,
									GREEN_V_C, GREEN_V_C,
									GREEN_U_C, GREEN_U_C,
									BLUE_U_C, BLUE_U_C,
									Y_C, Y_C,
									UV_128, UV_128,
									Y_16, Y_16,
									ALPHA_MASK, ALPHA_MASK,
};

#define CALC_COLOR_MODIFIERS(mov_instr, movu_instr, shift_instr, reg_type, zeros, output_offset1, output_offset2, u, v, coeff_storage) do {	\
			__asm__ __volatile__ (										\
				shift_instr " $" zeros ", %%"reg_type"7;"						\
															\
				movu_instr " (%0), %%"reg_type"1;"							\
				movu_instr " (%1), %%"reg_type"2;"							\
															\
				"punpcklbw %%"reg_type"7, %%"reg_type"1;"						\
				"punpcklbw %%"reg_type"7, %%"reg_type"2;"						\
															\
				mov_instr " 80(%3), %%"reg_type"7;"							\
				"psubsw %%"reg_type"7, %%"reg_type"1;"			/* U = U - 128 */		\
				"psubsw %%"reg_type"7, %%"reg_type"2;"			/* V = V - 128 */		\
															\
				mov_instr " %%"reg_type"1, %%"reg_type"3;"						\
				mov_instr " %%"reg_type"2, %%"reg_type"4;"						\
															\
				mov_instr " (%3), %%"reg_type"7;"							\
				"pmullw %%"reg_type"7, %%"reg_type"2;"			/* calculate Dred */		\
				"psraw $6, %%"reg_type"2;"				/* Dred = Dred / 64 */		\
															\
				mov_instr " 32(%3), %%"reg_type"7;"							\
				"pmullw %%"reg_type"7, %%"reg_type"3;"			/* calculate Ugreen */		\
				"psraw $6, %%"reg_type"3;"				/* Ugreen = Ugreen / 64 */	\
				mov_instr " 16(%3), %%"reg_type"7;"							\
				"pmullw %%"reg_type"7, %%"reg_type"4;"			/* calculate Vgreen */		\
				"psraw $6, %%"reg_type"4;"				/* Vgreen = Vgreen / 64 */	\
				"paddsw %%"reg_type"4, %%"reg_type"3;"			/* Dgreen = Ugreen + Vgreen */	\
															\
				mov_instr " 48(%3), %%"reg_type"7;"							\
				"pmullw %%"reg_type"7, %%"reg_type"1;"			/* calculate Dblue */		\
				"psraw $6, %%"reg_type"1;"				/* Dblue = Dblue / 64 */	\
															\
				mov_instr " %%"reg_type"2, (%2);"			/* backup Dred */		\
				mov_instr " %%"reg_type"3, "output_offset1"(%2);"	/* backup Dgreen */		\
				mov_instr " %%"reg_type"1, "output_offset2"(%2);"	/* backup Dblue */		\
				: : "r" (u), "r" (v), "r" (coeff_storage), "r" (&simd_table));				\
	} while (0);

#define RESTORE_COLOR_MODIFIERS(mov_instr, reg_type, coeff_storage, input_offset1, input_offset2) do {			\
			__asm__ __volatile__ (										\
				mov_instr " (%0), %%"reg_type"2;"				/* restore Dred */	\
				mov_instr " "input_offset1"(%0), %%"reg_type"3;"		/* restore Dgreen */	\
				mov_instr " "input_offset2"(%0), %%"reg_type"1;"		/* restore Dblue */	\
				: : "r" (coeff_storage));								\
	} while (0);

#define YUV2RGB_INTEL_SIMD(mov_instr, movu_instr, reg_type, output_offset1, output_offset2, output_offset3, y_plane, dest) do { 	\
			__asm__ __volatile__ ( 										\
				movu_instr " (%0), %%"reg_type"0;"		/* Load Y plane into r0 */		\
				mov_instr " 96(%2), %%"reg_type"7;"		/* Load 16 into r7 */			\
				"psubusb %%"reg_type"7, %%"reg_type"0;"		/* Y = Y - 16 */			\
															\
				mov_instr " %%"reg_type"0, %%"reg_type"4;"	/* r4 == r0 */				\
															\
				"psllw $8, %%"reg_type"0;"			/* r0 [00 Y0 00 Y2 ...] */		\
				"psrlw $8, %%"reg_type"0;"			/* r0 [Y0 00 Y2 00 ...] */		\
				"psrlw $8, %%"reg_type"4;"			/* r4 [Y1 00 Y3 00 ...] */		\
															\
				mov_instr " 64(%2), %%"reg_type"7;"							\
				"pmullw %%"reg_type"7, %%"reg_type"0;"		/* calculate Y*Yc[even] */		\
				"pmullw %%"reg_type"7, %%"reg_type"4;"		/* calculate Y*Yc[odd] */		\
				"psraw $6, %%"reg_type"0;"			/* Yyc[even] = Yyc[even] / 64 */	\
				"psraw $6, %%"reg_type"4;"			/* Yyc[odd] = Yyc[odd] / 64 */		\
															\
				mov_instr " %%"reg_type"2, %%"reg_type"6;"						\
				mov_instr " %%"reg_type"3, %%"reg_type"7;"						\
				mov_instr " %%"reg_type"1, %%"reg_type"5;"						\
															\
				"paddsw %%"reg_type"0, %%"reg_type"2;"		/* CY[even] + DR */			\
				"paddsw %%"reg_type"0, %%"reg_type"3;"		/* CY[even] + DG */			\
				"paddsw %%"reg_type"0, %%"reg_type"1;"		/* CY[even] + DB */			\
															\
				"paddsw %%"reg_type"4, %%"reg_type"6;"		/* CY[odd] + DR */			\
				"paddsw %%"reg_type"4, %%"reg_type"7;"		/* CY[odd] + DG */			\
				"paddsw %%"reg_type"4, %%"reg_type"5;"		/* CY[odd] + DB */			\
															\
				"packuswb %%"reg_type"2, %%"reg_type"2;"	/* Clamp RGB to [0-255] */		\
				"packuswb %%"reg_type"3, %%"reg_type"3;"						\
				"packuswb %%"reg_type"1, %%"reg_type"1;"						\
															\
				"packuswb %%"reg_type"6, %%"reg_type"6;"						\
				"packuswb %%"reg_type"7, %%"reg_type"7;"						\
				"packuswb %%"reg_type"5, %%"reg_type"5;"						\
															\
				"punpcklbw %%"reg_type"6, %%"reg_type"2;"	/* r2 [R0 R1 R2 R3 ...] */		\
				"punpcklbw %%"reg_type"7, %%"reg_type"3;"	/* r3 [G0 G1 G2 G3 ...] */		\
				"punpcklbw %%"reg_type"5, %%"reg_type"1;"	/* r1 [B0 B1 B2 B3 ...] */		\
															\
				mov_instr " %%"reg_type"2, %%"reg_type"5;"	/* copy RGB */				\
				mov_instr " %%"reg_type"3, %%"reg_type"7;"						\
				mov_instr " %%"reg_type"1, %%"reg_type"6;"						\
															\
				mov_instr " 112(%2), %%"reg_type"4;"							\
				"punpcklbw %%"reg_type"2, %%"reg_type"1;"	/* r1 [B0 R0 B1 R1 ...] */		\
				"punpcklbw %%"reg_type"4, %%"reg_type"3;"	/* r4 [G0 FF G1 FF ...] */		\
															\
				mov_instr " %%"reg_type"1, %%"reg_type"4;"	/* r3 [G0 FF G1 FF ...] */		\
															\
				"punpcklbw %%"reg_type"3, %%"reg_type"1;"	/* r2 [B0 G0 R0 FF B1 G1 R1 FF ...] */	\
				"punpckhbw %%"reg_type"3, %%"reg_type"4;"	/* r3 [B2 G2 R2 FF B3 G3 R3 FF ...] */	\
															\
				movu_instr " %%"reg_type"1, (%1);"		/* output BGRA */	 		\
				movu_instr " %%"reg_type"4, "output_offset1"(%1);"					\
															\
				mov_instr " 112(%2), %%"reg_type"4;"							\
				"punpckhbw %%"reg_type"5, %%"reg_type"6;"						\
				"punpckhbw %%"reg_type"4, %%"reg_type"7;"						\
															\
				mov_instr " %%"reg_type"6, %%"reg_type"4;"						\
															\
				"punpcklbw %%"reg_type"7, %%"reg_type"6;"						\
				"punpckhbw %%"reg_type"7, %%"reg_type"4;"						\
															\
				movu_instr " %%"reg_type"6, "output_offset2"(%1);"					\
				movu_instr " %%"reg_type"4, "output_offset3"(%1);"					\
				: : "r" (y_plane), "r" (dest), "r" (&simd_table));					\
		} while (0);
#endif

#if HAVE_SSE2
#define YUV2RGB_SSE(y_plane, dest) YUV2RGB_INTEL_SIMD("movdqa", "movdqu", "xmm", "16", "32", "48", y_plane, dest)
#endif

#if HAVE_MMX
#define YUV2RGB_MMX(y_plane, dest) YUV2RGB_INTEL_SIMD("movq", "movq", "mm", "8", "16", "24", y_plane, dest)
#endif

static inline void YUV444ToBGRA(uint8_t Y, uint8_t U, uint8_t V, uint8_t *dst)
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

YUVConverter::YUVConverter (Media* media, VideoStream* stream) : IImageConverter (media, stream)
{
#if !defined(__amd64__) && !defined(__x86_64__)
	have_mmx = true;
	have_sse2 = true;
#else
	// FIXME: We need to detect this at runtime
#  if HAVE_MMX
	have_mmx = true;
#  else
	have_mmx = false;
#  endif

#  if HAVE_SSE2
	have_sse2 = true;
#  else
	have_sse2 = false;
#  endif

#endif
}

YUVConverter::~YUVConverter ()
{
}

MediaResult
YUVConverter::Open ()
{
	if (input_format == MoonPixelFormatNone) {
		media->AddMessage (MEDIA_CONVERTER_ERROR, "Invalid input format.");
		return MEDIA_CONVERTER_ERROR;
	}
	
	if (output_format == MoonPixelFormatNone) {
		media->AddMessage (MEDIA_CONVERTER_ERROR, "Invalid output format.");
		return MEDIA_CONVERTER_ERROR;
	}
	
	return MEDIA_SUCCESS;
}

/* 
 * FIXME: We need to use the have_* runtime detection code so we can distribute a MMX/SSE2 build that will gracefully
 * fall back on ancient processors / processors without those features
 */
MediaResult
YUVConverter::Convert (uint8_t *src[], int srcStride[], int srcSlideY, int srcSlideH, uint8_t* dest[], int dstStride [])
{
	uint8_t *y_row1 = src[0];
	uint8_t *y_row2 = src[0]+srcStride[0];

	uint8_t *u_plane = src[1];
	uint8_t *v_plane = src[2];
	
	uint8_t *dest_row1 = dest[0];
	uint8_t *dest_row2 = dest[0]+dstStride[0];

	int i, j;

	int width = dstStride[0]/4;
	int height = srcSlideH;
	int planar_delta = srcStride[0] - (dstStride[0]/4);

#if HAVE_SSE2
	uint64_t rgb_uv [6];

	/* YUV420p processes 2 lines at a time */
	for (i = 0; i < height/2; i ++, y_row1 += srcStride[0], y_row2 += srcStride[0], dest_row1 += dstStride[0], dest_row2 += dstStride[0]) {
		for (j = 0; j < width/16; j ++, y_row1 += 16, y_row2 += 16, u_plane += 8, v_plane += 8, dest_row1 += 64, dest_row2 += 64) {
			CALC_COLOR_MODIFIERS("movdqa", "movdqu", "pslldq", "xmm", "128", "16", "32", u_plane, v_plane, &rgb_uv);
			
			YUV2RGB_SSE(y_row1, dest_row1);
			
			RESTORE_COLOR_MODIFIERS("movdqa", "xmm", &rgb_uv, "16", "32");

			YUV2RGB_SSE(y_row2, dest_row2);
		}
		/* We currently dont SSE convert these pixel
		 * the srcStride is padded enough that we could; but we'd have to track and clamp output
		 * we should measure the metric for that check on output for the extra 2/4/6 pixels on the
		 * end.  I presume that its faster to do it this way
		 */
		for (j = 0; j < (width-(width/16)*16); j+= 2, dest_row1 += 8, dest_row2 += 8, y_row1 += 2, y_row2 += 2, u_plane += 1, v_plane += 1) {
			YUV444ToBGRA (*y_row1, *u_plane, *v_plane, dest_row1);
			YUV444ToBGRA (y_row1[1], *u_plane, *v_plane, (dest_row1+4));
			
			YUV444ToBGRA (*y_row2, *u_plane, *v_plane, dest_row2);
			YUV444ToBGRA (y_row2[1], *u_plane, *v_plane, (dest_row2+4));
		}
		y_row1 += planar_delta;
		y_row2 += planar_delta;
		u_plane += planar_delta >> 1;
		v_plane += planar_delta >> 1;
	}
#elif HAVE_MMX
	uint64_t rgb_uv [3];

	/* YUV420p processes 2 lines at a time */
	for (i = 0; i < height/2; i ++, y_row1 += srcStride[0], y_row2 += srcStride[0], dest_row1 += dstStride[0], dest_row2 += dstStride[0]) {
		for (j = 0; j < width/8; j ++, y_row1 += 8, y_row2 += 8, u_plane += 4, v_plane += 4, dest_row1 += 32, dest_row2 += 32) {
			CALC_COLOR_MODIFIERS("movq", "movd", "psllq", "mm", "64", "8", "16", u_plane, v_plane, &rgb_uv);

			YUV2RGB_MMX(y_row1, dest_row1);

			RESTORE_COLOR_MODIFIERS("movq", "mm", &rgb_uv, "8", "16");

			YUV2RGB_MMX(y_row2, dest_row2);
		}
		/* We currently dont MMX convert these pixel
		 * the srcStride is padded enough that we could; but we'd have to track and clamp output
		 * we should measure the metric for that check on output for the extra 2/4/6 pixels on the
		 * end.  I presume that its faster to do it this way
		 */
		for (j = 0; j < (width-(width/8)*8); j+= 2, dest_row1 += 8, dest_row2 += 8, y_row1 += 2, y_row2 += 2, u_plane += 1, v_plane += 1) {
			YUV444ToBGRA (*y_row1, *u_plane, *v_plane, dest_row1);
			YUV444ToBGRA (y_row1[1], *u_plane, *v_plane, (dest_row1+4));
			
			YUV444ToBGRA (*y_row2, *u_plane, *v_plane, dest_row2);
			YUV444ToBGRA (y_row2[1], *u_plane, *v_plane, (dest_row2+4));
		}
		y_row1 += planar_delta;
		y_row2 += planar_delta;
		u_plane += planar_delta >> 1;
		v_plane += planar_delta >> 1;
	}

	__asm__ __volatile__ ("emms");
#else
	for (i = 0; i < height; i += 2, y_row1 += srcStride[0], y_row2 += srcStride[0], dest_row1 += dstStride[0], dest_row2 += dstStride[0]) {
		for (j = 0; j < width; j += 2, dest_row1 += 8, dest_row2 += 8, y_row1 += 2, y_row2 += 2, u_plane += 1, v_plane += 1) {
			YUV444ToBGRA (*y_row1, *u_plane, *v_plane, dest_row1);
			YUV444ToBGRA (y_row1[1], *u_plane, *v_plane, (dest_row1+4));
			
			YUV444ToBGRA (*y_row2, *u_plane, *v_plane, dest_row2);
			YUV444ToBGRA (y_row2[1], *u_plane, *v_plane, (dest_row2+4));
		}
		y_row1 += planar_delta;
		y_row2 += planar_delta;
		u_plane += planar_delta >> 1;
		v_plane += planar_delta >> 1;
	}
#endif

	return MEDIA_SUCCESS;
}
