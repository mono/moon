#ifndef __CUTIL_H__
#define __CUTIL_H__

G_BEGIN_DECLS

void
x_cairo_matrix_transform_bounding_box (const cairo_matrix_t *matrix,
				       double *x1, double *y1,
				       double *x2, double *y2);

G_END_DECLS

#endif
