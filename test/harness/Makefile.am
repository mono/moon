
INCLUDES=@MOON_CFLAGS@ -I$(srcdir)/../../src

noinst_PROGRAMS=xaml2png
xaml2png_SOURCES=xaml2png.cpp
xaml2png_LDADD=$(top_builddir)/src/libmoon.la $(MOON_PROG_LIBS)

MOON_TESTS= \
	../xaml/test-brush-linear.xaml \
	../xaml/test-brush-radial.xaml \
	../xaml/test-canvas-inside-canvas.xaml \
	../xaml/test-colors.xaml \
	../xaml/test-geometry-collection.xaml \
	../xaml/test-geometry-transform.xaml \
	../xaml/test-imagebrush-opacity.xaml \
	../xaml/test-imagebrush-transform.xaml \
	../xaml/test-imagebrush.xaml \
	../xaml/test-image.xaml \
	../xaml/test-mediaelement-clip.xaml \
	../xaml/test-monkey.xaml \
	../xaml/test-path-ellipse-renderxorigin.xaml \
	../xaml/test-path-ellipse-stretch.xaml \
	../xaml/test-path-ellipse-stroke.xaml \
	../xaml/test-path-ellipse-stroke-2.xaml \
	../xaml/test-path-ellipse-stroke-3.xaml \
	../xaml/test-path-ellipse.xaml \
	../xaml/test-path-figure-collection.xaml \
	../xaml/test-path-group-ellipse.xaml \
	../xaml/test-path-group-empty.xaml \
	../xaml/test-path-group-rectangle.xaml \
	../xaml/test-path-group-stretch.xaml \
	../xaml/test-path-group-torture.xaml \
	../xaml/test-path-group.xaml \
	../xaml/test-path-line-stretch.xaml \
	../xaml/test-path-line-stroke.xaml \
	../xaml/test-path-line.xaml \
	../xaml/test-path-rectangle-round-stroke.xaml \
	../xaml/test-path-rectangle-stretch.xaml \
	../xaml/test-path-rectangle-stroke.xaml \
	../xaml/test-path-rectangle.xaml \
	../xaml/test-path-segments-arc.xaml \
	../xaml/test-path-segments-collection.xaml \
	../xaml/test-path-segments-empty-2.xaml \
	../xaml/test-path-segments-empty.xaml \
	../xaml/test-path-segments-line-stroke.xaml \
	../xaml/test-path-segments-line.xaml \
	../xaml/test-path-smoothbezier-stroke.xaml \
	../xaml/test-path-smoothbezier.xaml \
	../xaml/test-path-smoothquadraticbezier-stroke.xaml \
	../xaml/test-path-smoothquadraticbezier.xaml \
	../xaml/test-path-stretch-fill-extents.xaml \
	../xaml/test-property-path.xaml \
	../xaml/test-render-xorigin.xaml \
	../xaml/test-rtol.xaml \
	../xaml/test-set-inherited-property-elements.xaml \
	../xaml/test-shape-ellipse-renderxorigin.xaml \
	../xaml/test-shape-ellipse-stretch.xaml \
	../xaml/test-shape-ellipse-stroke-2.xaml \
	../xaml/test-shape-ellipse-stroke.xaml \
	../xaml/test-shape-ellipse.xaml \
	../xaml/test-shape-line-renderxorigin.xaml \
	../xaml/test-shape-line-stretch.xaml \
	../xaml/test-shape-line-stroke.xaml \
	../xaml/test-shape-line.xaml \
	../xaml/test-shape-opacity.xaml \
	../xaml/test-shape-polygon-renderxorigin.xaml \
	../xaml/test-shape-polygon-stretch.xaml \
	../xaml/test-shape-polygon-stroke.xaml \
	../xaml/test-shape-polygon.xaml \
	../xaml/test-shape-polyline-renderxorigin.xaml \
	../xaml/test-shape-polyline-stretch.xaml \
	../xaml/test-shape-polyline-stroke.xaml \
	../xaml/test-shape-polyline.xaml \
	../xaml/test-shape-rectangle-matrix.xaml \
	../xaml/test-shape-rectangle-renderxorigin.xaml \
	../xaml/test-shape-rectangle-round-stroke.xaml \
	../xaml/test-shape-rectangle-stretch.xaml \
	../xaml/test-shape-rectangle-stroke.xaml \
	../xaml/test-shape-rectangle.xaml \
	../xaml/test-shape-styles.xaml \
	../xaml/test-sibling-opacity.xaml \
	../xaml/test-solidbrush-opacity.xaml \
	../xaml/test-textblock-content.xaml \
	../xaml/test-textblock.xaml \
	../xaml/test-text-runs.xaml \
	../xaml/test-transform-group-empty.xaml \
	../xaml/test-transform-group.xaml \
	../xaml/test-transform-matrix.xaml \
	../xaml/test-transform-skew.xaml \
	../xaml/test-trigger-action-collection.xaml \
	../xaml/test-trigger-collection.xaml \
	../xaml/test-zero-scale-animation.xaml

baseline: xaml2png
	@for t in $(MOON_TESTS); do \
		./run-test.sh baseline $$t `basename $$t`; \
	done	
check: xaml2png
	@for t in $(MOON_TESTS); do \
		./run-test.sh test $$t `basename $$t`; \
	done

clean-local:
	rm -f *.jpg
