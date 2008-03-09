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

#if HAVE_MMX
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
static const uint64_t mmx_table [8] __attribute__ ((aligned (16))) = {
									0x0066006600660066ULL, /* Red V coefficient */
									0xffccffccffccffccULL, /* Green V coefficient */
									0xffe7ffe7ffe7ffe7ULL, /* Green U coefficient */
									0x0081008100810081ULL, /* Blue U coefficient */
									0x004a004a004a004aULL, /* Y coefficient */
									0x0080008000800080ULL, /* 128 */
									0x1010101010101010ULL, /* 16 */
									0xFFFFFFFFFFFFFFFFULL, /* Alpha channel */
};

/* We process 2 lines per so we fold this into a define:
 * Register usage:
 * 	%mm2: Rcoeff * V
 * 	%mm3: GUcoeff * U + GVcoeff * V
 * 	%mm1: Bcoeff * U 
 */
#define YUV2RGB_MMX(y_plane, dest) do { \
			__asm__ __volatile__ ( \
				"movq (%0), %%mm0;"		/* mm0 [Y0 Y1 Y2 Y3 Y4 Y5 Y6 Y7] */	\
				"movq 48(%2), %%mm7;"							\
				"psubusb %%mm7, %%mm0;"		/* Y = Y - 16 */			\
													\
				"movq %%mm0, %%mm4;"		/* mm4 == mm0 */			\
													\
				"psllw $8, %%mm0;"		/* mm0 [00 Y0 00 Y2 00 Y4 00 Y6] */	\
				"psrlw $8, %%mm0;"		/* mm0 [Y0 00 Y2 00 Y4 00 Y6 00] */	\
				"psrlw $8, %%mm4;"		/* mm4 [Y1 00 Y3 00 Y5 00 Y7 00] */	\
													\
				"movq 32(%2), %%mm7;"							\
				"pmullw %%mm7, %%mm0;"		/* calculate Y*Yc[even] */		\
				"pmullw %%mm7, %%mm4;"		/* calculate Y*Yc[odd] */		\
				"psraw $6, %%mm0;"		/* Yyc[even] = Yyc[even] / 64 */	\
				"psraw $6, %%mm4;"		/* Yyc[odd] = Yyc[odd] / 64 */		\
													\
				"movq %%mm2, %%mm6;"							\
				"movq %%mm3, %%mm7;"							\
				"movq %%mm1, %%mm5;"							\
													\
				"paddsw %%mm0, %%mm2;"		/* CY[even] + DR */			\
				"paddsw %%mm0, %%mm3;"		/* CY[even] + DG */			\
				"paddsw %%mm0, %%mm1;"		/* CY[even] + DB */			\
													\
				"paddsw %%mm4, %%mm6;"		/* CY[odd] + DR */			\
				"paddsw %%mm4, %%mm7;"		/* CY[odd] + DG */			\
				"paddsw %%mm4, %%mm5;"		/* CY[odd] + DB */			\
													\
				"packuswb %%mm2, %%mm2;"	/* Clamp RGB to [0-255] */		\
				"packuswb %%mm3, %%mm3;"						\
				"packuswb %%mm1, %%mm1;"						\
													\
				"packuswb %%mm6, %%mm6;"						\
				"packuswb %%mm7, %%mm7;"						\
				"packuswb %%mm5, %%mm5;"						\
													\
				"punpcklbw %%mm6, %%mm2;"	/* mm2 [R0 R1 R2 R3 R4 R5 R6 R7] */	\
				"punpcklbw %%mm7, %%mm3;"	/* mm3 [G0 G1 G2 G3 G4 G5 G6 G7] */	\
				"punpcklbw %%mm5, %%mm1;"	/* mm1 [B0 B1 B2 B3 B4 B5 B6 B7] */	\
													\
				"movq %%mm2, %%mm5;"		/* copy RGB */				\
				"movq %%mm3, %%mm7;"							\
				"movq %%mm1, %%mm6;"							\
													\
				"movq 56(%2), %%mm4;"							\
				"punpcklbw %%mm2, %%mm1;"	/* mm1 [B0 R0 B1 R1 B2 R2 B3 R3] */	\
				"punpcklbw %%mm4, %%mm3;"	/* mm4 [G0 FF G1 FF G2 FF G3 FF] */	\
													\
				"movq %%mm1, %%mm4;"		/* mm3 [G0 FF G1 FF G2 FF G3 FF] */	\
													\
				"punpcklbw %%mm3, %%mm1;"	/* mm2 [B0 G0 R0 FF B1 G1 R1 FF] */	\
				"punpckhbw %%mm3, %%mm4;"	/* mm3 [B2 G2 R2 FF B3 G3 R3 FF] */	\
													\
				"movq %%mm1, (%1);"		/* output BGRA[0] BGRA[1] */ 		\
				"movq %%mm4, 8(%1);"		/* output BGRA[2] BGRA[3] */ 		\
													\
				"movq 56(%2), %%mm4;"							\
				"punpckhbw %%mm5, %%mm6;"	/* mm5 [B4 R4 B5 R6 B7 R7 B8 R8] */	\
				"punpckhbw %%mm4, %%mm7;"	/* mm6 [G4 FF G5 FF G6 FF G7 FF] */	\
													\
				"movq %%mm6, %%mm4;"		/* mm7 [G4 FF G5 FF G6 FF G7 FF] */	\
													\
				"punpcklbw %%mm7, %%mm6;"	/* mm6 [B4 G4 R4 FF B5 G5 R5 FF] */	\
				"punpckhbw %%mm7, %%mm4;"	/* mm7 [B6 G6 R6 FF B7 G7 R7 FF] */	\
													\
				"movq %%mm6, 16(%1);"		/* output BGRA[4] BGRA[5] */ 		\
				"movq %%mm4, 24(%1);"		/* output BGRA[6] BGRA[7] */ 		\
				: : "r" (y_plane), "r" (dest), "r" (&mmx_table));			\
		} while (0);
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

#if HAVE_MMX
	uint64_t rgb_uv [3];

	/* YUV420p processes 2 lines at a time */
	for (i = 0; i < height/2; i ++, y_row1 += srcStride[0], y_row2 += srcStride[0], dest_row1 += dstStride[0], dest_row2 += dstStride[0]) {
		for (j = 0; j < width/8; j ++, y_row1 += 8, y_row2 += 8, u_plane += 4, v_plane += 4, dest_row1 += 32, dest_row2 += 32) {
			__asm__ __volatile__ (
				"psllq $64, %%mm7;"		/* mm7 [00 00 00 00 00 00 00 00] */ 

				"movd (%0), %%mm1;"		/* mm1 [U0 U1 U2 U3 00 00 00 00] */
				"movd (%1), %%mm2;"		/* mm2 [V0 V1 V2 V3 00 00 00 00] */

				"punpcklbw %%mm7, %%mm1;"	/* mm1 [U0 00 U1 00 U2 00 U3 00] */	
				"punpcklbw %%mm7, %%mm2;"	/* mm2 [V0 00 V1 00 V2 00 V3 00] */	

				"movq 40(%3), %%mm7;"
				"psubsw %%mm7, %%mm1;"		/* U = U - 128 */
				"psubsw %%mm7, %%mm2;"		/* V = V - 128 */

				"movq %%mm1, %%mm3;"		/* mm3 = mm1 */
				"movq %%mm2, %%mm4;"		/* mm4 = mm1 */

				"movq (%3), %%mm7;"
				"pmullw %%mm7, %%mm2;"		/* calculate Dred */
				"psraw $6, %%mm2;"		/* Dred = Dred / 64 */

				"movq 16(%3), %%mm7;"
				"pmullw %%mm7, %%mm3;"		/* calculate Ugreen */ 
				"psraw $6, %%mm3;"		/* Ugreen = Ugreen / 64 */
				"movq 8(%3), %%mm7;"
				"pmullw %%mm7, %%mm4;"		/* calculate Vgreen */ 
				"psraw $6, %%mm4;"		/* Vgreen = Vgreen / 64 */
				"paddsw %%mm4, %%mm3;"		/* Dgreen = Ugreen + Vgreen */

				"movq 24(%3), %%mm7;"
				"pmullw %%mm7, %%mm1;"		/* calculate Dblue */
				"psraw $6, %%mm1;"		/* Dblue = Dblue / 64 */

				"movq %%mm2, (%2);"		/* backup Dred */
				"movq %%mm3, 8(%2);"		/* backup Dgreen */
				"movq %%mm1, 16(%2);"		/* backup Dblue */
				: : "r" (u_plane), "r" (v_plane), "r" (&rgb_uv), "r" (&mmx_table));

			YUV2RGB_MMX(y_row1, dest_row1);

			__asm__ __volatile__ (
				"movq (%0), %%mm2;"		/* restore Dred */
				"movq 8(%0), %%mm3;"		/* restore Dgreen */
				"movq 16(%0), %%mm1;"		/* restore Dblue */
				: : "r" (&rgb_uv));

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
