using System;
using System.Windows;
using System.Windows.Automation.Peers;
using System.Windows.Browser;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Resources;
using System.Windows.Shapes;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;
using System.Collections.Generic;

namespace MoonTest.System.Windows.Controls
{
    [TestClass]
    public class DefaultValueTests
    {
        [TestMethod]
        public void AssemblyPart_ReadLocalValue ()
        {
            AssemblyPart widget = new AssemblyPart ();
            object retval;

            retval = widget.ReadLocalValue (AssemblyPart.SourceProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SourceProperty) should not have a value by default");
        }

        [TestMethod]
        public void AssemblyPart_GetValue ()
        {
            AssemblyPart widget = new AssemblyPart ();
            object retval;

            retval = widget.GetValue (AssemblyPart.SourceProperty);
            Assert.IsNotNull (retval, "GetValue(SourceProperty) should not have returned null");
            Assert.IsTrue (retval is string, "GetValue(SourceProperty) is not of the correct type");
            Assert.AreEqual ("", retval, "GetValue(SourceProperty) does not match the default value");
        }

        [TestMethod]
        public void RotateTransform_ReadLocalValue ()
        {
            RotateTransform widget = new RotateTransform ();
            object retval;

            retval = widget.ReadLocalValue (RotateTransform.CenterXProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CenterXProperty) should not have a value by default");

            retval = widget.ReadLocalValue (RotateTransform.CenterYProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CenterYProperty) should not have a value by default");

            retval = widget.ReadLocalValue (RotateTransform.AngleProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(AngleProperty) should not have a value by default");
        }

        [TestMethod]
        public void RotateTransform_GetValue ()
        {
            RotateTransform widget = new RotateTransform ();
            object retval;

            retval = widget.GetValue (RotateTransform.CenterXProperty);
            Assert.IsNotNull (retval, "GetValue(CenterXProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(CenterXProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(CenterXProperty) does not match the default value");

            retval = widget.GetValue (RotateTransform.CenterYProperty);
            Assert.IsNotNull (retval, "GetValue(CenterYProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(CenterYProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(CenterYProperty) does not match the default value");

            retval = widget.GetValue (RotateTransform.AngleProperty);
            Assert.IsNotNull (retval, "GetValue(AngleProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(AngleProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(AngleProperty) does not match the default value");
        }

        [TestMethod]
        public void ScaleTransform_ReadLocalValue ()
        {
            ScaleTransform widget = new ScaleTransform ();
            object retval;

            retval = widget.ReadLocalValue (ScaleTransform.CenterXProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CenterXProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScaleTransform.CenterYProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CenterYProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScaleTransform.ScaleXProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ScaleXProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScaleTransform.ScaleYProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ScaleYProperty) should not have a value by default");
        }

        [TestMethod]
        public void ScaleTransform_GetValue ()
        {
            ScaleTransform widget = new ScaleTransform ();
            object retval;

            retval = widget.GetValue (ScaleTransform.CenterXProperty);
            Assert.IsNotNull (retval, "GetValue(CenterXProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(CenterXProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(CenterXProperty) does not match the default value");

            retval = widget.GetValue (ScaleTransform.CenterYProperty);
            Assert.IsNotNull (retval, "GetValue(CenterYProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(CenterYProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(CenterYProperty) does not match the default value");

            retval = widget.GetValue (ScaleTransform.ScaleXProperty);
            Assert.IsNotNull (retval, "GetValue(ScaleXProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(ScaleXProperty) is not of the correct type");
            Assert.AreEqual (1, retval, "GetValue(ScaleXProperty) does not match the default value");

            retval = widget.GetValue (ScaleTransform.ScaleYProperty);
            Assert.IsNotNull (retval, "GetValue(ScaleYProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(ScaleYProperty) is not of the correct type");
            Assert.AreEqual (1, retval, "GetValue(ScaleYProperty) does not match the default value");
        }

        [TestMethod]
        public void SkewTransform_ReadLocalValue ()
        {
            SkewTransform widget = new SkewTransform ();
            object retval;

            retval = widget.ReadLocalValue (SkewTransform.CenterXProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CenterXProperty) should not have a value by default");

            retval = widget.ReadLocalValue (SkewTransform.CenterYProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CenterYProperty) should not have a value by default");

            retval = widget.ReadLocalValue (SkewTransform.AngleXProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(AngleXProperty) should not have a value by default");

            retval = widget.ReadLocalValue (SkewTransform.AngleYProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(AngleYProperty) should not have a value by default");
        }

        [TestMethod]
        public void SkewTransform_GetValue ()
        {
            SkewTransform widget = new SkewTransform ();
            object retval;

            retval = widget.GetValue (SkewTransform.CenterXProperty);
            Assert.IsNotNull (retval, "GetValue(CenterXProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(CenterXProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(CenterXProperty) does not match the default value");

            retval = widget.GetValue (SkewTransform.CenterYProperty);
            Assert.IsNotNull (retval, "GetValue(CenterYProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(CenterYProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(CenterYProperty) does not match the default value");

            retval = widget.GetValue (SkewTransform.AngleXProperty);
            Assert.IsNotNull (retval, "GetValue(AngleXProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(AngleXProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(AngleXProperty) does not match the default value");

            retval = widget.GetValue (SkewTransform.AngleYProperty);
            Assert.IsNotNull (retval, "GetValue(AngleYProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(AngleYProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(AngleYProperty) does not match the default value");
        }

        [TestMethod]
        public void TranslateTransform_ReadLocalValue ()
        {
            TranslateTransform widget = new TranslateTransform ();
            object retval;

            retval = widget.ReadLocalValue (TranslateTransform.XProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(XProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TranslateTransform.YProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(YProperty) should not have a value by default");
        }

        [TestMethod]
        public void TranslateTransform_GetValue ()
        {
            TranslateTransform widget = new TranslateTransform ();
            object retval;

            retval = widget.GetValue (TranslateTransform.XProperty);
            Assert.IsNotNull (retval, "GetValue(XProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(XProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(XProperty) does not match the default value");

            retval = widget.GetValue (TranslateTransform.YProperty);
            Assert.IsNotNull (retval, "GetValue(YProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(YProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(YProperty) does not match the default value");
        }

        [TestMethod]
        public void TransformGroup_ReadLocalValue ()
        {
            TransformGroup widget = new TransformGroup ();
            object retval;

            retval = widget.ReadLocalValue (TransformGroup.ChildrenProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ChildrenProperty) should not have a value by default");
        }

        [TestMethod]
        public void TransformGroup_GetValue ()
        {
            TransformGroup widget = new TransformGroup ();
            object retval;

            retval = widget.GetValue (TransformGroup.ChildrenProperty);
            Assert.IsNotNull (retval, "GetValue(ChildrenProperty) should not have returned null");
            Assert.IsTrue (retval is TransformCollection, "GetValue(ChildrenProperty) is not of the correct type");
        }

        [TestMethod]
        public void MatrixTransform_ReadLocalValue ()
        {
            MatrixTransform widget = new MatrixTransform ();
            object retval;

            retval = widget.ReadLocalValue (MatrixTransform.MatrixProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(MatrixProperty) should not have a value by default");
        }

        [TestMethod]
        public void MatrixTransform_GetValue ()
        {
            MatrixTransform widget = new MatrixTransform ();
            object retval;

            retval = widget.GetValue (MatrixTransform.MatrixProperty);
            Assert.IsNotNull (retval, "GetValue(MatrixProperty) should not have returned null");
            Assert.IsTrue (retval is Matrix, "GetValue(MatrixProperty) is not of the correct type");
            Assert.AreEqual ("Identity", retval.ToString (), "GetValue(MatrixProperty) does not match the default value");
        }

        [TestMethod]
        public void LineSegment_ReadLocalValue ()
        {
            LineSegment widget = new LineSegment ();
            object retval;

            retval = widget.ReadLocalValue (LineSegment.PointProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(PointProperty) should not have a value by default");
        }

        [TestMethod]
        public void LineSegment_GetValue ()
        {
            LineSegment widget = new LineSegment ();
            object retval;

            retval = widget.GetValue (LineSegment.PointProperty);
            Assert.IsNotNull (retval, "GetValue(PointProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(PointProperty) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(PointProperty) does not match the default value");
        }

        [TestMethod]
        public void BezierSegment_ReadLocalValue ()
        {
            BezierSegment widget = new BezierSegment ();
            object retval;

            retval = widget.ReadLocalValue (BezierSegment.Point1Property);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(Point1Property) should not have a value by default");

            retval = widget.ReadLocalValue (BezierSegment.Point2Property);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(Point2Property) should not have a value by default");

            retval = widget.ReadLocalValue (BezierSegment.Point3Property);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(Point3Property) should not have a value by default");
        }

        [TestMethod]
        public void BezierSegment_GetValue ()
        {
            BezierSegment widget = new BezierSegment ();
            object retval;

            retval = widget.GetValue (BezierSegment.Point1Property);
            Assert.IsNotNull (retval, "GetValue(Point1Property) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(Point1Property) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(Point1Property) does not match the default value");

            retval = widget.GetValue (BezierSegment.Point2Property);
            Assert.IsNotNull (retval, "GetValue(Point2Property) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(Point2Property) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(Point2Property) does not match the default value");

            retval = widget.GetValue (BezierSegment.Point3Property);
            Assert.IsNotNull (retval, "GetValue(Point3Property) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(Point3Property) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(Point3Property) does not match the default value");
        }

        [TestMethod]
        public void QuadraticBezierSegment_ReadLocalValue ()
        {
            QuadraticBezierSegment widget = new QuadraticBezierSegment ();
            object retval;

            retval = widget.ReadLocalValue (QuadraticBezierSegment.Point1Property);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(Point1Property) should not have a value by default");

            retval = widget.ReadLocalValue (QuadraticBezierSegment.Point2Property);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(Point2Property) should not have a value by default");
        }

        [TestMethod]
        public void QuadraticBezierSegment_GetValue ()
        {
            QuadraticBezierSegment widget = new QuadraticBezierSegment ();
            object retval;

            retval = widget.GetValue (QuadraticBezierSegment.Point1Property);
            Assert.IsNotNull (retval, "GetValue(Point1Property) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(Point1Property) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(Point1Property) does not match the default value");

            retval = widget.GetValue (QuadraticBezierSegment.Point2Property);
            Assert.IsNotNull (retval, "GetValue(Point2Property) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(Point2Property) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(Point2Property) does not match the default value");
        }

        [TestMethod]
        public void ArcSegment_ReadLocalValue ()
        {
            ArcSegment widget = new ArcSegment ();
            object retval;

            retval = widget.ReadLocalValue (ArcSegment.PointProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(PointProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ArcSegment.SizeProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SizeProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ArcSegment.RotationAngleProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RotationAngleProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ArcSegment.IsLargeArcProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsLargeArcProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ArcSegment.SweepDirectionProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SweepDirectionProperty) should not have a value by default");
        }

        [TestMethod]
        public void ArcSegment_GetValue ()
        {
            ArcSegment widget = new ArcSegment ();
            object retval;

            retval = widget.GetValue (ArcSegment.PointProperty);
            Assert.IsNotNull (retval, "GetValue(PointProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(PointProperty) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(PointProperty) does not match the default value");

            retval = widget.GetValue (ArcSegment.SizeProperty);
            Assert.IsNotNull (retval, "GetValue(SizeProperty) should not have returned null");
            Assert.IsTrue (retval is Size, "GetValue(SizeProperty) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(SizeProperty) does not match the default value");

            retval = widget.GetValue (ArcSegment.RotationAngleProperty);
            Assert.IsNotNull (retval, "GetValue(RotationAngleProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(RotationAngleProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(RotationAngleProperty) does not match the default value");

            retval = widget.GetValue (ArcSegment.IsLargeArcProperty);
            Assert.IsNotNull (retval, "GetValue(IsLargeArcProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsLargeArcProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsLargeArcProperty) does not match the default value");

            retval = widget.GetValue (ArcSegment.SweepDirectionProperty);
            Assert.IsNotNull (retval, "GetValue(SweepDirectionProperty) should not have returned null");
            Assert.IsTrue (retval is SweepDirection, "GetValue(SweepDirectionProperty) is not of the correct type");
            Assert.AreEqual (SweepDirection.Counterclockwise, retval, "GetValue(SweepDirectionProperty) does not match the default value");
        }

        [TestMethod]
        public void PolyLineSegment_ReadLocalValue ()
        {
            PolyLineSegment widget = new PolyLineSegment ();
            object retval;

            retval = widget.ReadLocalValue (PolyLineSegment.PointsProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(PointsProperty) should not have a value by default");
        }

        [TestMethod]
        public void PolyLineSegment_GetValue ()
        {
            PolyLineSegment widget = new PolyLineSegment ();
            object retval;

            retval = widget.GetValue (PolyLineSegment.PointsProperty);
            Assert.IsNotNull (retval, "GetValue(PointsProperty) should not have returned null");
            Assert.IsTrue (retval is PointCollection, "GetValue(PointsProperty) is not of the correct type");
        }

        [TestMethod]
        public void PolyBezierSegment_ReadLocalValue ()
        {
            PolyBezierSegment widget = new PolyBezierSegment ();
            object retval;

            retval = widget.ReadLocalValue (PolyBezierSegment.PointsProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(PointsProperty) should not have a value by default");
        }

        [TestMethod]
        public void PolyBezierSegment_GetValue ()
        {
            PolyBezierSegment widget = new PolyBezierSegment ();
            object retval;

            retval = widget.GetValue (PolyBezierSegment.PointsProperty);
            Assert.IsNotNull (retval, "GetValue(PointsProperty) should not have returned null");
            Assert.IsTrue (retval is PointCollection, "GetValue(PointsProperty) is not of the correct type");
        }

        [TestMethod]
        public void PolyQuadraticBezierSegment_ReadLocalValue ()
        {
            PolyQuadraticBezierSegment widget = new PolyQuadraticBezierSegment ();
            object retval;

            retval = widget.ReadLocalValue (PolyQuadraticBezierSegment.PointsProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(PointsProperty) should not have a value by default");
        }

        [TestMethod]
        public void PolyQuadraticBezierSegment_GetValue ()
        {
            PolyQuadraticBezierSegment widget = new PolyQuadraticBezierSegment ();
            object retval;

            retval = widget.GetValue (PolyQuadraticBezierSegment.PointsProperty);
            Assert.IsNotNull (retval, "GetValue(PointsProperty) should not have returned null");
            Assert.IsTrue (retval is PointCollection, "GetValue(PointsProperty) is not of the correct type");
        }

        [TestMethod]
        public void PathFigure_ReadLocalValue ()
        {
            PathFigure widget = new PathFigure ();
            object retval;

            retval = widget.ReadLocalValue (PathFigure.SegmentsProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SegmentsProperty) should not have a value by default");

            retval = widget.ReadLocalValue (PathFigure.StartPointProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(StartPointProperty) should not have a value by default");

            retval = widget.ReadLocalValue (PathFigure.IsClosedProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsClosedProperty) should not have a value by default");

            retval = widget.ReadLocalValue (PathFigure.IsFilledProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsFilledProperty) should not have a value by default");
        }

        [TestMethod]
        public void PathFigure_GetValue ()
        {
            PathFigure widget = new PathFigure ();
            object retval;

            retval = widget.GetValue (PathFigure.SegmentsProperty);
            Assert.IsNotNull (retval, "GetValue(SegmentsProperty) should not have returned null");
            Assert.IsTrue (retval is PathSegmentCollection, "GetValue(SegmentsProperty) is not of the correct type");

            retval = widget.GetValue (PathFigure.StartPointProperty);
            Assert.IsNotNull (retval, "GetValue(StartPointProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(StartPointProperty) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(StartPointProperty) does not match the default value");

            retval = widget.GetValue (PathFigure.IsClosedProperty);
            Assert.IsNotNull (retval, "GetValue(IsClosedProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsClosedProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsClosedProperty) does not match the default value");

            retval = widget.GetValue (PathFigure.IsFilledProperty);
            Assert.IsNotNull (retval, "GetValue(IsFilledProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsFilledProperty) is not of the correct type");
            Assert.AreEqual (true, retval, "GetValue(IsFilledProperty) does not match the default value");
        }

        [TestMethod]
        public void PathGeometry_ReadLocalValue ()
        {
            PathGeometry widget = new PathGeometry ();
            object retval;

            retval = widget.ReadLocalValue (PathGeometry.FillRuleProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FillRuleProperty) should not have a value by default");

            retval = widget.ReadLocalValue (PathGeometry.FiguresProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FiguresProperty) should not have a value by default");
        }

        [TestMethod]
        public void PathGeometry_GetValue ()
        {
            PathGeometry widget = new PathGeometry ();
            object retval;

            retval = widget.GetValue (PathGeometry.FillRuleProperty);
            Assert.IsNotNull (retval, "GetValue(FillRuleProperty) should not have returned null");
            Assert.IsTrue (retval is FillRule, "GetValue(FillRuleProperty) is not of the correct type");
            Assert.AreEqual (FillRule.EvenOdd, retval, "GetValue(FillRuleProperty) does not match the default value");

            retval = widget.GetValue (PathGeometry.FiguresProperty);
            Assert.IsNotNull (retval, "GetValue(FiguresProperty) should not have returned null");
            Assert.IsTrue (retval is PathFigureCollection, "GetValue(FiguresProperty) is not of the correct type");
        }

        [TestMethod]
        public void EllipseGeometry_ReadLocalValue ()
        {
            EllipseGeometry widget = new EllipseGeometry ();
            object retval;

            retval = widget.ReadLocalValue (EllipseGeometry.CenterProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CenterProperty) should not have a value by default");

            retval = widget.ReadLocalValue (EllipseGeometry.RadiusXProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RadiusXProperty) should not have a value by default");

            retval = widget.ReadLocalValue (EllipseGeometry.RadiusYProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RadiusYProperty) should not have a value by default");
        }

        [TestMethod]
        public void EllipseGeometry_GetValue ()
        {
            EllipseGeometry widget = new EllipseGeometry ();
            object retval;

            retval = widget.GetValue (EllipseGeometry.CenterProperty);
            Assert.IsNotNull (retval, "GetValue(CenterProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(CenterProperty) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(CenterProperty) does not match the default value");

            retval = widget.GetValue (EllipseGeometry.RadiusXProperty);
            Assert.IsNotNull (retval, "GetValue(RadiusXProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(RadiusXProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(RadiusXProperty) does not match the default value");

            retval = widget.GetValue (EllipseGeometry.RadiusYProperty);
            Assert.IsNotNull (retval, "GetValue(RadiusYProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(RadiusYProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(RadiusYProperty) does not match the default value");
        }

        [TestMethod]
        public void RectangleGeometry_ReadLocalValue ()
        {
            RectangleGeometry widget = new RectangleGeometry ();
            object retval;

            retval = widget.ReadLocalValue (RectangleGeometry.RectProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RectProperty) should not have a value by default");

            retval = widget.ReadLocalValue (RectangleGeometry.RadiusXProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RadiusXProperty) should not have a value by default");

            retval = widget.ReadLocalValue (RectangleGeometry.RadiusYProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RadiusYProperty) should not have a value by default");
        }

        [TestMethod]
        public void RectangleGeometry_GetValue ()
        {
            RectangleGeometry widget = new RectangleGeometry ();
            object retval;

            retval = widget.GetValue (RectangleGeometry.RectProperty);
            Assert.IsNotNull (retval, "GetValue(RectProperty) should not have returned null");
            Assert.IsTrue (retval is Rect, "GetValue(RectProperty) is not of the correct type");
            Assert.AreEqual ("0,0,0,0", retval.ToString (), "GetValue(RectProperty) does not match the default value");

            retval = widget.GetValue (RectangleGeometry.RadiusXProperty);
            Assert.IsNotNull (retval, "GetValue(RadiusXProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(RadiusXProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(RadiusXProperty) does not match the default value");

            retval = widget.GetValue (RectangleGeometry.RadiusYProperty);
            Assert.IsNotNull (retval, "GetValue(RadiusYProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(RadiusYProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(RadiusYProperty) does not match the default value");
        }

        [TestMethod]
        public void LineGeometry_ReadLocalValue ()
        {
            LineGeometry widget = new LineGeometry ();
            object retval;

            retval = widget.ReadLocalValue (LineGeometry.StartPointProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(StartPointProperty) should not have a value by default");

            retval = widget.ReadLocalValue (LineGeometry.EndPointProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(EndPointProperty) should not have a value by default");
        }

        [TestMethod]
        public void LineGeometry_GetValue ()
        {
            LineGeometry widget = new LineGeometry ();
            object retval;

            retval = widget.GetValue (LineGeometry.StartPointProperty);
            Assert.IsNotNull (retval, "GetValue(StartPointProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(StartPointProperty) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(StartPointProperty) does not match the default value");

            retval = widget.GetValue (LineGeometry.EndPointProperty);
            Assert.IsNotNull (retval, "GetValue(EndPointProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(EndPointProperty) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(EndPointProperty) does not match the default value");
        }

        [TestMethod]
        public void GeometryGroup_ReadLocalValue ()
        {
            GeometryGroup widget = new GeometryGroup ();
            object retval;

            retval = widget.ReadLocalValue (GeometryGroup.FillRuleProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FillRuleProperty) should not have a value by default");

            retval = widget.ReadLocalValue (GeometryGroup.ChildrenProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ChildrenProperty) should not have a value by default");
        }

        [TestMethod]
        public void GeometryGroup_GetValue ()
        {
            GeometryGroup widget = new GeometryGroup ();
            object retval;

            retval = widget.GetValue (GeometryGroup.FillRuleProperty);
            Assert.IsNotNull (retval, "GetValue(FillRuleProperty) should not have returned null");
            Assert.IsTrue (retval is FillRule, "GetValue(FillRuleProperty) is not of the correct type");
            Assert.AreEqual (FillRule.EvenOdd, retval, "GetValue(FillRuleProperty) does not match the default value");

            retval = widget.GetValue (GeometryGroup.ChildrenProperty);
            Assert.IsNotNull (retval, "GetValue(ChildrenProperty) should not have returned null");
            Assert.IsTrue (retval is GeometryCollection, "GetValue(ChildrenProperty) is not of the correct type");
        }

        [TestMethod]
        public void SolidColorBrush_ReadLocalValue ()
        {
            SolidColorBrush widget = new SolidColorBrush ();
            object retval;

            retval = widget.ReadLocalValue (SolidColorBrush.ColorProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ColorProperty) should not have a value by default");
        }

        [TestMethod]
        public void SolidColorBrush_GetValue ()
        {
            SolidColorBrush widget = new SolidColorBrush ();
            object retval;

            retval = widget.GetValue (SolidColorBrush.ColorProperty);
            Assert.IsNotNull (retval, "GetValue(ColorProperty) should not have returned null");
            Assert.IsTrue (retval is Color, "GetValue(ColorProperty) is not of the correct type");
            Assert.AreEqual ("#00000000", retval.ToString (), "GetValue(ColorProperty) does not match the default value");
        }

        [TestMethod]
        public void GradientStop_ReadLocalValue ()
        {
            GradientStop widget = new GradientStop ();
            object retval;

            retval = widget.ReadLocalValue (GradientStop.ColorProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ColorProperty) should not have a value by default");

            retval = widget.ReadLocalValue (GradientStop.OffsetProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(OffsetProperty) should not have a value by default");
        }

        [TestMethod]
        public void GradientStop_GetValue ()
        {
            GradientStop widget = new GradientStop ();
            object retval;

            retval = widget.GetValue (GradientStop.ColorProperty);
            Assert.IsNotNull (retval, "GetValue(ColorProperty) should not have returned null");
            Assert.IsTrue (retval is Color, "GetValue(ColorProperty) is not of the correct type");
            Assert.AreEqual ("#00000000", retval.ToString (), "GetValue(ColorProperty) does not match the default value");

            retval = widget.GetValue (GradientStop.OffsetProperty);
            Assert.IsNotNull (retval, "GetValue(OffsetProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(OffsetProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(OffsetProperty) does not match the default value");
        }

        [TestMethod]
        public void LinearGradientBrush_ReadLocalValue ()
        {
            LinearGradientBrush widget = new LinearGradientBrush ();
            object retval;

            retval = widget.ReadLocalValue (LinearGradientBrush.StartPointProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(StartPointProperty) should not have a value by default");

            retval = widget.ReadLocalValue (LinearGradientBrush.EndPointProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(EndPointProperty) should not have a value by default");
        }

        [TestMethod]
        public void LinearGradientBrush_GetValue ()
        {
            LinearGradientBrush widget = new LinearGradientBrush ();
            object retval;

            retval = widget.GetValue (LinearGradientBrush.StartPointProperty);
            Assert.IsNotNull (retval, "GetValue(StartPointProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(StartPointProperty) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(StartPointProperty) does not match the default value");

            retval = widget.GetValue (LinearGradientBrush.EndPointProperty);
            Assert.IsNotNull (retval, "GetValue(EndPointProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(EndPointProperty) is not of the correct type");
            Assert.AreEqual ("1,1", retval.ToString (), "GetValue(EndPointProperty) does not match the default value");
        }

        [TestMethod]
        public void RadialGradientBrush_ReadLocalValue ()
        {
            RadialGradientBrush widget = new RadialGradientBrush ();
            object retval;

            retval = widget.ReadLocalValue (RadialGradientBrush.CenterProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CenterProperty) should not have a value by default");

            retval = widget.ReadLocalValue (RadialGradientBrush.GradientOriginProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(GradientOriginProperty) should not have a value by default");

            retval = widget.ReadLocalValue (RadialGradientBrush.RadiusXProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RadiusXProperty) should not have a value by default");

            retval = widget.ReadLocalValue (RadialGradientBrush.RadiusYProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RadiusYProperty) should not have a value by default");
        }

        [TestMethod]
        public void RadialGradientBrush_GetValue ()
        {
            RadialGradientBrush widget = new RadialGradientBrush ();
            object retval;

            retval = widget.GetValue (RadialGradientBrush.CenterProperty);
            Assert.IsNotNull (retval, "GetValue(CenterProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(CenterProperty) is not of the correct type");
            Assert.AreEqual ("0.5,0.5", retval.ToString (), "GetValue(CenterProperty) does not match the default value");

            retval = widget.GetValue (RadialGradientBrush.GradientOriginProperty);
            Assert.IsNotNull (retval, "GetValue(GradientOriginProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(GradientOriginProperty) is not of the correct type");
            Assert.AreEqual ("0.5,0.5", retval.ToString (), "GetValue(GradientOriginProperty) does not match the default value");

            retval = widget.GetValue (RadialGradientBrush.RadiusXProperty);
            Assert.IsNotNull (retval, "GetValue(RadiusXProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(RadiusXProperty) is not of the correct type");
            Assert.AreEqual (0.5, retval, "GetValue(RadiusXProperty) does not match the default value");

            retval = widget.GetValue (RadialGradientBrush.RadiusYProperty);
            Assert.IsNotNull (retval, "GetValue(RadiusYProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(RadiusYProperty) is not of the correct type");
            Assert.AreEqual (0.5, retval, "GetValue(RadiusYProperty) does not match the default value");
        }

        [TestMethod]
        public void ImageBrush_ReadLocalValue ()
        {
            ImageBrush widget = new ImageBrush ();
            object retval;

            retval = widget.ReadLocalValue (ImageBrush.ImageSourceProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ImageSourceProperty) should not have a value by default");
        }

        [TestMethod]
        public void ImageBrush_GetValue ()
        {
            ImageBrush widget = new ImageBrush ();
            object retval;

            retval = widget.GetValue (ImageBrush.ImageSourceProperty);
            Assert.IsNull (retval, "GetValue(ImageSourceProperty) should have returned null");
        }

        [TestMethod]
        public void VideoBrush_ReadLocalValue ()
        {
            VideoBrush widget = new VideoBrush ();
            object retval;

            retval = widget.ReadLocalValue (VideoBrush.SourceNameProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SourceNameProperty) should not have a value by default");
        }

        [TestMethod]
        public void VideoBrush_GetValue ()
        {
            VideoBrush widget = new VideoBrush ();
            object retval;

            retval = widget.GetValue (VideoBrush.SourceNameProperty);
            Assert.IsNotNull (retval, "GetValue(SourceNameProperty) should not have returned null");
            Assert.IsTrue (retval is string, "GetValue(SourceNameProperty) is not of the correct type");
            Assert.AreEqual ("", retval, "GetValue(SourceNameProperty) does not match the default value");
        }

        [TestMethod]
        public void TimelineMarker_ReadLocalValue ()
        {
            TimelineMarker widget = new TimelineMarker ();
            object retval;

            retval = widget.ReadLocalValue (TimelineMarker.TimeProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TimeProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TimelineMarker.TypeProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TypeProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TimelineMarker.TextProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TextProperty) should not have a value by default");
        }

        [TestMethod]
        public void TimelineMarker_GetValue ()
        {
            TimelineMarker widget = new TimelineMarker ();
            object retval;

            retval = widget.GetValue (TimelineMarker.TimeProperty);
            Assert.IsNotNull (retval, "GetValue(TimeProperty) should not have returned null");
            Assert.IsTrue (retval is TimeSpan, "GetValue(TimeProperty) is not of the correct type");
            Assert.AreEqual ("00:00:00", retval.ToString (), "GetValue(TimeProperty) does not match the default value");

            retval = widget.GetValue (TimelineMarker.TypeProperty);
            Assert.IsNull (retval, "GetValue(TypeProperty) should have returned null");

            retval = widget.GetValue (TimelineMarker.TextProperty);
            Assert.IsNull (retval, "GetValue(TextProperty) should have returned null");
        }

        [TestMethod]
        public void BeginStoryboard_ReadLocalValue ()
        {
            BeginStoryboard widget = new BeginStoryboard ();
            object retval;

            retval = widget.ReadLocalValue (BeginStoryboard.StoryboardProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(StoryboardProperty) should not have a value by default");
        }

        [TestMethod]
        public void BeginStoryboard_GetValue ()
        {
            BeginStoryboard widget = new BeginStoryboard ();
            object retval;

            retval = widget.GetValue (BeginStoryboard.StoryboardProperty);
            Assert.IsNull (retval, "GetValue(StoryboardProperty) should have returned null");
        }

        [TestMethod]
        public void Storyboard_ReadLocalValue ()
        {
            Storyboard widget = new Storyboard ();
            object retval;

            retval = widget.ReadLocalValue (Storyboard.TargetPropertyProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TargetPropertyProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Storyboard.TargetNameProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TargetNameProperty) should not have a value by default");
        }

        [TestMethod]
        public void Storyboard_GetValue ()
        {
            Storyboard widget = new Storyboard ();
            object retval;

            retval = widget.GetValue (Storyboard.TargetPropertyProperty);
            Assert.IsNull (retval, "GetValue(TargetPropertyProperty) should have returned null");

            retval = widget.GetValue (Storyboard.TargetNameProperty);
            Assert.IsNull (retval, "GetValue(TargetNameProperty) should have returned null");
        }

        [TestMethod]
        public void DoubleAnimation_ReadLocalValue ()
        {
            DoubleAnimation widget = new DoubleAnimation ();
            object retval;

            retval = widget.ReadLocalValue (DoubleAnimation.FromProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FromProperty) should not have a value by default");

            retval = widget.ReadLocalValue (DoubleAnimation.ToProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ToProperty) should not have a value by default");

            retval = widget.ReadLocalValue (DoubleAnimation.ByProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ByProperty) should not have a value by default");
        }

        [TestMethod]
        public void DoubleAnimation_GetValue ()
        {
            DoubleAnimation widget = new DoubleAnimation ();
            object retval;

            retval = widget.GetValue (DoubleAnimation.FromProperty);
            Assert.IsNull (retval, "GetValue(FromProperty) should have returned null");

            retval = widget.GetValue (DoubleAnimation.ToProperty);
            Assert.IsNull (retval, "GetValue(ToProperty) should have returned null");

            retval = widget.GetValue (DoubleAnimation.ByProperty);
            Assert.IsNull (retval, "GetValue(ByProperty) should have returned null");
        }

        [TestMethod]
        public void ColorAnimation_ReadLocalValue ()
        {
            ColorAnimation widget = new ColorAnimation ();
            object retval;

            retval = widget.ReadLocalValue (ColorAnimation.FromProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FromProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ColorAnimation.ToProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ToProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ColorAnimation.ByProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ByProperty) should not have a value by default");
        }

        [TestMethod]
        public void ColorAnimation_GetValue ()
        {
            ColorAnimation widget = new ColorAnimation ();
            object retval;

            retval = widget.GetValue (ColorAnimation.FromProperty);
            Assert.IsNull (retval, "GetValue(FromProperty) should have returned null");

            retval = widget.GetValue (ColorAnimation.ToProperty);
            Assert.IsNull (retval, "GetValue(ToProperty) should have returned null");

            retval = widget.GetValue (ColorAnimation.ByProperty);
            Assert.IsNull (retval, "GetValue(ByProperty) should have returned null");
        }

        [TestMethod]
        public void PointAnimation_ReadLocalValue ()
        {
            PointAnimation widget = new PointAnimation ();
            object retval;

            retval = widget.ReadLocalValue (PointAnimation.FromProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FromProperty) should not have a value by default");

            retval = widget.ReadLocalValue (PointAnimation.ToProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ToProperty) should not have a value by default");

            retval = widget.ReadLocalValue (PointAnimation.ByProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ByProperty) should not have a value by default");
        }

        [TestMethod]
        public void PointAnimation_GetValue ()
        {
            PointAnimation widget = new PointAnimation ();
            object retval;

            retval = widget.GetValue (PointAnimation.FromProperty);
            Assert.IsNull (retval, "GetValue(FromProperty) should have returned null");

            retval = widget.GetValue (PointAnimation.ToProperty);
            Assert.IsNull (retval, "GetValue(ToProperty) should have returned null");

            retval = widget.GetValue (PointAnimation.ByProperty);
            Assert.IsNull (retval, "GetValue(ByProperty) should have returned null");
        }

        [TestMethod]
        public void SplineDoubleKeyFrame_ReadLocalValue ()
        {
            SplineDoubleKeyFrame widget = new SplineDoubleKeyFrame ();
            object retval;

            retval = widget.ReadLocalValue (SplineDoubleKeyFrame.KeySplineProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(KeySplineProperty) should not have a value by default");
        }

        [TestMethod]
        public void SplineDoubleKeyFrame_GetValue ()
        {
            SplineDoubleKeyFrame widget = new SplineDoubleKeyFrame ();
            object retval;

            retval = widget.GetValue (SplineDoubleKeyFrame.KeySplineProperty);
            Assert.IsNotNull (retval, "GetValue(KeySplineProperty) should not have returned null");
            Assert.IsTrue (retval is KeySpline, "GetValue(KeySplineProperty) is not of the correct type");
        }

        [TestMethod]
        public void SplineColorKeyFrame_ReadLocalValue ()
        {
            SplineColorKeyFrame widget = new SplineColorKeyFrame ();
            object retval;

            retval = widget.ReadLocalValue (SplineColorKeyFrame.KeySplineProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(KeySplineProperty) should not have a value by default");
        }

        [TestMethod]
        public void SplineColorKeyFrame_GetValue ()
        {
            SplineColorKeyFrame widget = new SplineColorKeyFrame ();
            object retval;

            retval = widget.GetValue (SplineColorKeyFrame.KeySplineProperty);
            Assert.IsNotNull (retval, "GetValue(KeySplineProperty) should not have returned null");
            Assert.IsTrue (retval is KeySpline, "GetValue(KeySplineProperty) is not of the correct type");
        }

        [TestMethod]
        public void SplinePointKeyFrame_ReadLocalValue ()
        {
            SplinePointKeyFrame widget = new SplinePointKeyFrame ();
            object retval;

            retval = widget.ReadLocalValue (SplinePointKeyFrame.KeySplineProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(KeySplineProperty) should not have a value by default");
        }

        [TestMethod]
        public void SplinePointKeyFrame_GetValue ()
        {
            SplinePointKeyFrame widget = new SplinePointKeyFrame ();
            object retval;

            retval = widget.GetValue (SplinePointKeyFrame.KeySplineProperty);
            Assert.IsNotNull (retval, "GetValue(KeySplineProperty) should not have returned null");
            Assert.IsTrue (retval is KeySpline, "GetValue(KeySplineProperty) is not of the correct type");
        }

        [TestMethod]
        public void Path_ReadLocalValue ()
        {
            Path widget = new Path ();
            object retval;

            retval = widget.ReadLocalValue (Path.DataProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(DataProperty) should not have a value by default");
        }

        [TestMethod]
        public void Path_GetValue ()
        {
            Path widget = new Path ();
            object retval;

            retval = widget.GetValue (Path.DataProperty);
            Assert.IsNull (retval, "GetValue(DataProperty) should have returned null");
        }

        [TestMethod]
        public void Line_ReadLocalValue ()
        {
            Line widget = new Line ();
            object retval;

            retval = widget.ReadLocalValue (Line.X1Property);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(X1Property) should not have a value by default");

            retval = widget.ReadLocalValue (Line.Y1Property);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(Y1Property) should not have a value by default");

            retval = widget.ReadLocalValue (Line.X2Property);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(X2Property) should not have a value by default");

            retval = widget.ReadLocalValue (Line.Y2Property);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(Y2Property) should not have a value by default");
        }

        [TestMethod]
        public void Line_GetValue ()
        {
            Line widget = new Line ();
            object retval;

            retval = widget.GetValue (Line.X1Property);
            Assert.IsNotNull (retval, "GetValue(X1Property) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(X1Property) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(X1Property) does not match the default value");

            retval = widget.GetValue (Line.Y1Property);
            Assert.IsNotNull (retval, "GetValue(Y1Property) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(Y1Property) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(Y1Property) does not match the default value");

            retval = widget.GetValue (Line.X2Property);
            Assert.IsNotNull (retval, "GetValue(X2Property) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(X2Property) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(X2Property) does not match the default value");

            retval = widget.GetValue (Line.Y2Property);
            Assert.IsNotNull (retval, "GetValue(Y2Property) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(Y2Property) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(Y2Property) does not match the default value");
        }

        [TestMethod]
        public void Polygon_ReadLocalValue ()
        {
            Polygon widget = new Polygon ();
            object retval;

            retval = widget.ReadLocalValue (Polygon.FillRuleProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FillRuleProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Polygon.PointsProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(PointsProperty) should not have a value by default");
        }

        [TestMethod]
        public void Polygon_GetValue ()
        {
            Polygon widget = new Polygon ();
            object retval;

            retval = widget.GetValue (Polygon.FillRuleProperty);
            Assert.IsNotNull (retval, "GetValue(FillRuleProperty) should not have returned null");
            Assert.IsTrue (retval is FillRule, "GetValue(FillRuleProperty) is not of the correct type");
            Assert.AreEqual (FillRule.EvenOdd, retval, "GetValue(FillRuleProperty) does not match the default value");

            retval = widget.GetValue (Polygon.PointsProperty);
            Assert.IsNotNull (retval, "GetValue(PointsProperty) should not have returned null");
            Assert.IsTrue (retval is PointCollection, "GetValue(PointsProperty) is not of the correct type");
        }

        [TestMethod]
        public void Polyline_ReadLocalValue ()
        {
            Polyline widget = new Polyline ();
            object retval;

            retval = widget.ReadLocalValue (Polyline.FillRuleProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FillRuleProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Polyline.PointsProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(PointsProperty) should not have a value by default");
        }

        [TestMethod]
        public void Polyline_GetValue ()
        {
            Polyline widget = new Polyline ();
            object retval;

            retval = widget.GetValue (Polyline.FillRuleProperty);
            Assert.IsNotNull (retval, "GetValue(FillRuleProperty) should not have returned null");
            Assert.IsTrue (retval is FillRule, "GetValue(FillRuleProperty) is not of the correct type");
            Assert.AreEqual (FillRule.EvenOdd, retval, "GetValue(FillRuleProperty) does not match the default value");

            retval = widget.GetValue (Polyline.PointsProperty);
            Assert.IsNotNull (retval, "GetValue(PointsProperty) should not have returned null");
            Assert.IsTrue (retval is PointCollection, "GetValue(PointsProperty) is not of the correct type");
        }

        [TestMethod]
        public void Rectangle_ReadLocalValue ()
        {
            Rectangle widget = new Rectangle ();
            object retval;

            retval = widget.ReadLocalValue (Rectangle.RadiusXProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RadiusXProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Rectangle.RadiusYProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RadiusYProperty) should not have a value by default");
        }

        [TestMethod]
        public void Rectangle_GetValue ()
        {
            Rectangle widget = new Rectangle ();
            object retval;

            retval = widget.GetValue (Rectangle.RadiusXProperty);
            Assert.IsNotNull (retval, "GetValue(RadiusXProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(RadiusXProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(RadiusXProperty) does not match the default value");

            retval = widget.GetValue (Rectangle.RadiusYProperty);
            Assert.IsNotNull (retval, "GetValue(RadiusYProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(RadiusYProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(RadiusYProperty) does not match the default value");
        }

        [TestMethod]
        public void Glyphs_ReadLocalValue ()
        {
            Glyphs widget = new Glyphs ();
            object retval;

            retval = widget.ReadLocalValue (Glyphs.UnicodeStringProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(UnicodeStringProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Glyphs.IndicesProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IndicesProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Glyphs.FontUriProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FontUriProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Glyphs.StyleSimulationsProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(StyleSimulationsProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Glyphs.FontRenderingEmSizeProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FontRenderingEmSizeProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Glyphs.OriginXProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(OriginXProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Glyphs.OriginYProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(OriginYProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Glyphs.FillProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FillProperty) should not have a value by default");
        }

        [TestMethod]
        public void Glyphs_GetValue ()
        {
            Glyphs widget = new Glyphs ();
            object retval;

            retval = widget.GetValue (Glyphs.UnicodeStringProperty);
            Assert.IsNotNull (retval, "GetValue(UnicodeStringProperty) should not have returned null");
            Assert.IsTrue (retval is string, "GetValue(UnicodeStringProperty) is not of the correct type");
            Assert.AreEqual ("", retval, "GetValue(UnicodeStringProperty) does not match the default value");

            retval = widget.GetValue (Glyphs.IndicesProperty);
            Assert.IsNotNull (retval, "GetValue(IndicesProperty) should not have returned null");
            Assert.IsTrue (retval is string, "GetValue(IndicesProperty) is not of the correct type");
            Assert.AreEqual ("", retval, "GetValue(IndicesProperty) does not match the default value");

            retval = widget.GetValue (Glyphs.FontUriProperty);
            Assert.IsNotNull (retval, "GetValue(FontUriProperty) should not have returned null");
            Assert.IsTrue (retval is Uri, "GetValue(FontUriProperty) is not of the correct type");
            Assert.AreEqual ("", retval.ToString (), "GetValue(FontUriProperty) does not match the default value");

            retval = widget.GetValue (Glyphs.StyleSimulationsProperty);
            Assert.IsNotNull (retval, "GetValue(StyleSimulationsProperty) should not have returned null");
            Assert.IsTrue (retval is StyleSimulations, "GetValue(StyleSimulationsProperty) is not of the correct type");
            Assert.AreEqual (StyleSimulations.None, retval, "GetValue(StyleSimulationsProperty) does not match the default value");

            retval = widget.GetValue (Glyphs.FontRenderingEmSizeProperty);
            Assert.IsNotNull (retval, "GetValue(FontRenderingEmSizeProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(FontRenderingEmSizeProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(FontRenderingEmSizeProperty) does not match the default value");

            retval = widget.GetValue (Glyphs.OriginXProperty);
            Assert.IsNotNull (retval, "GetValue(OriginXProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(OriginXProperty) is not of the correct type");
            Assert.AreEqual (-3.40282346638529E+38, retval, "GetValue(OriginXProperty) does not match the default value");

            retval = widget.GetValue (Glyphs.OriginYProperty);
            Assert.IsNotNull (retval, "GetValue(OriginYProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(OriginYProperty) is not of the correct type");
            Assert.AreEqual (-3.40282346638529E+38, retval, "GetValue(OriginYProperty) does not match the default value");

            retval = widget.GetValue (Glyphs.FillProperty);
            Assert.IsNull (retval, "GetValue(FillProperty) should have returned null");
        }

        [TestMethod]
        public void Image_ReadLocalValue ()
        {
            Image widget = new Image ();
            object retval;

            retval = widget.ReadLocalValue (Image.SourceProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SourceProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Image.StretchProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(StretchProperty) should not have a value by default");
        }

        [TestMethod]
        public void Image_GetValue ()
        {
            Image widget = new Image ();
            object retval;

            retval = widget.GetValue (Image.SourceProperty);
            Assert.IsNotNull (retval, "GetValue(SourceProperty) should not have returned null");
            Assert.IsTrue (retval is BitmapImage, "GetValue(SourceProperty) is not of the correct type");

            retval = widget.GetValue (Image.StretchProperty);
            Assert.IsNotNull (retval, "GetValue(StretchProperty) should not have returned null");
            Assert.IsTrue (retval is Stretch, "GetValue(StretchProperty) is not of the correct type");
            Assert.AreEqual (Stretch.Uniform, retval, "GetValue(StretchProperty) does not match the default value");
        }

        [TestMethod]
        public void Canvas_ReadLocalValue ()
        {
            Canvas widget = new Canvas ();
            object retval;

            retval = widget.ReadLocalValue (Canvas.LeftProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(LeftProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Canvas.TopProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TopProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Canvas.ZIndexProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ZIndexProperty) should not have a value by default");
        }

        [TestMethod]
        public void Canvas_GetValue ()
        {
            Canvas widget = new Canvas ();
            object retval;

            retval = widget.GetValue (Canvas.LeftProperty);
            Assert.IsNotNull (retval, "GetValue(LeftProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(LeftProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(LeftProperty) does not match the default value");

            retval = widget.GetValue (Canvas.TopProperty);
            Assert.IsNotNull (retval, "GetValue(TopProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(TopProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(TopProperty) does not match the default value");

            retval = widget.GetValue (Canvas.ZIndexProperty);
            Assert.IsNotNull (retval, "GetValue(ZIndexProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(ZIndexProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(ZIndexProperty) does not match the default value");
        }

        [TestMethod]
        public void TextBlock_ReadLocalValue ()
        {
            TextBlock widget = new TextBlock ();
            object retval;

            retval = widget.ReadLocalValue (TextBlock.FontSizeProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FontSizeProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.FontFamilyProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FontFamilyProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.FontWeightProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FontWeightProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.FontStyleProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FontStyleProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.FontStretchProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(FontStretchProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.ForegroundProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ForegroundProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.TextDecorationsProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TextDecorationsProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.TextWrappingProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TextWrappingProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.TextAlignmentProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TextAlignmentProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.TextProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TextProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.PaddingProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(PaddingProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.LineHeightProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(LineHeightProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBlock.LineStackingStrategyProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(LineStackingStrategyProperty) should not have a value by default");
        }

        [TestMethod]
        public void TextBlock_GetValue ()
        {
            TextBlock widget = new TextBlock ();
            object retval;

            retval = widget.GetValue (TextBlock.FontSizeProperty);
            Assert.IsNotNull (retval, "GetValue(FontSizeProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(FontSizeProperty) is not of the correct type");
            Assert.AreEqual (11, retval, "GetValue(FontSizeProperty) does not match the default value");

            retval = widget.GetValue (TextBlock.FontFamilyProperty);
            Assert.IsNotNull (retval, "GetValue(FontFamilyProperty) should not have returned null");
            Assert.IsTrue (retval is FontFamily, "GetValue(FontFamilyProperty) is not of the correct type");
            Assert.AreEqual ("Portable User Interface", retval.ToString (), "GetValue(FontFamilyProperty) does not match the default value");

            retval = widget.GetValue (TextBlock.FontWeightProperty);
            Assert.IsNotNull (retval, "GetValue(FontWeightProperty) should not have returned null");
            Assert.IsTrue (retval is FontWeight, "GetValue(FontWeightProperty) is not of the correct type");
            Assert.AreEqual ("Normal", retval.ToString (), "GetValue(FontWeightProperty) does not match the default value");

            retval = widget.GetValue (TextBlock.FontStyleProperty);
            Assert.IsNotNull (retval, "GetValue(FontStyleProperty) should not have returned null");
            Assert.IsTrue (retval is FontStyle, "GetValue(FontStyleProperty) is not of the correct type");
            Assert.AreEqual ("Normal", retval.ToString (), "GetValue(FontStyleProperty) does not match the default value");

            retval = widget.GetValue (TextBlock.FontStretchProperty);
            Assert.IsNotNull (retval, "GetValue(FontStretchProperty) should not have returned null");
            Assert.IsTrue (retval is FontStretch, "GetValue(FontStretchProperty) is not of the correct type");
            Assert.AreEqual ("Normal", retval.ToString (), "GetValue(FontStretchProperty) does not match the default value");

            retval = widget.GetValue (TextBlock.ForegroundProperty);
            Assert.IsNotNull (retval, "GetValue(ForegroundProperty) should not have returned null");
            Assert.IsTrue (retval is SolidColorBrush, "GetValue(ForegroundProperty) is not of the correct type");

            retval = widget.GetValue (TextBlock.TextDecorationsProperty);
            Assert.IsNull (retval, "GetValue(TextDecorationsProperty) should have returned null");

            retval = widget.GetValue (TextBlock.TextWrappingProperty);
            Assert.IsNotNull (retval, "GetValue(TextWrappingProperty) should not have returned null");
            Assert.IsTrue (retval is TextWrapping, "GetValue(TextWrappingProperty) is not of the correct type");
            Assert.AreEqual (TextWrapping.NoWrap, retval, "GetValue(TextWrappingProperty) does not match the default value");

            retval = widget.GetValue (TextBlock.TextAlignmentProperty);
            Assert.IsNotNull (retval, "GetValue(TextAlignmentProperty) should not have returned null");
            Assert.IsTrue (retval is TextAlignment, "GetValue(TextAlignmentProperty) is not of the correct type");
            Assert.AreEqual (TextAlignment.Left, retval, "GetValue(TextAlignmentProperty) does not match the default value");

            retval = widget.GetValue (TextBlock.TextProperty);
            Assert.IsNotNull (retval, "GetValue(TextProperty) should not have returned null");
            Assert.IsTrue (retval is string, "GetValue(TextProperty) is not of the correct type");
            Assert.AreEqual ("", retval, "GetValue(TextProperty) does not match the default value");

            retval = widget.GetValue (TextBlock.PaddingProperty);
            Assert.IsNotNull (retval, "GetValue(PaddingProperty) should not have returned null");
            Assert.IsTrue (retval is Thickness, "GetValue(PaddingProperty) is not of the correct type");
            Assert.AreEqual ("0,0,0,0", retval.ToString (), "GetValue(PaddingProperty) does not match the default value");

            retval = widget.GetValue (TextBlock.LineHeightProperty);
            Assert.IsNotNull (retval, "GetValue(LineHeightProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(LineHeightProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(LineHeightProperty) does not match the default value");

            retval = widget.GetValue (TextBlock.LineStackingStrategyProperty);
            Assert.IsNotNull (retval, "GetValue(LineStackingStrategyProperty) should not have returned null");
            Assert.IsTrue (retval is LineStackingStrategy, "GetValue(LineStackingStrategyProperty) is not of the correct type");
            Assert.AreEqual (LineStackingStrategy.MaxHeight, retval, "GetValue(LineStackingStrategyProperty) does not match the default value");
        }

        [TestMethod]
        public void MediaElement_ReadLocalValue ()
        {
            MediaElement widget = new MediaElement ();
            object retval;

            retval = widget.ReadLocalValue (MediaElement.SourceProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SourceProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.StretchProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(StretchProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.IsMutedProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsMutedProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.AutoPlayProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(AutoPlayProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.VolumeProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(VolumeProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.BalanceProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(BalanceProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.NaturalVideoHeightProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(NaturalVideoHeightProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.NaturalVideoWidthProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(NaturalVideoWidthProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.NaturalDurationProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(NaturalDurationProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.PositionProperty);
            Assert.IsNotNull (retval, "ReadLocalValue(PositionProperty) should not have returned null");
            Assert.IsTrue (retval is TimeSpan, "ReadLocalValue(PositionProperty) is not of the correct type");
            Assert.AreEqual ("00:00:00", retval.ToString (), "ReadLocalValue(PositionProperty) does not match the default value");

            retval = widget.ReadLocalValue (MediaElement.DownloadProgressProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(DownloadProgressProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.BufferingProgressProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(BufferingProgressProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.DownloadProgressOffsetProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(DownloadProgressOffsetProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.CurrentStateProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CurrentStateProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.BufferingTimeProperty);
            Assert.IsNotNull (retval, "ReadLocalValue(BufferingTimeProperty) should not have returned null");
            Assert.IsTrue (retval is TimeSpan, "ReadLocalValue(BufferingTimeProperty) is not of the correct type");
            Assert.AreEqual ("00:00:05", retval.ToString (), "ReadLocalValue(BufferingTimeProperty) does not match the default value");

            retval = widget.ReadLocalValue (MediaElement.CanSeekProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CanSeekProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.CanPauseProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CanPauseProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.AudioStreamCountProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(AudioStreamCountProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.AudioStreamIndexProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(AudioStreamIndexProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.RenderedFramesPerSecondProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RenderedFramesPerSecondProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MediaElement.DroppedFramesPerSecondProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(DroppedFramesPerSecondProperty) should not have a value by default");

            Assert.Throws<Exception>(delegate {
                retval = widget.ReadLocalValue (MediaElement.AttributesProperty);
            }, "ReadLocalValue(AttributesProperty) should thow an exception");
        }

        [TestMethod]
        public void MediaElement_GetValue ()
        {
            MediaElement widget = new MediaElement ();
            object retval;

            retval = widget.GetValue (MediaElement.SourceProperty);
            Assert.IsNull (retval, "GetValue(SourceProperty) should have returned null");

            retval = widget.GetValue (MediaElement.StretchProperty);
            Assert.IsNotNull (retval, "GetValue(StretchProperty) should not have returned null");
            Assert.IsTrue (retval is Stretch, "GetValue(StretchProperty) is not of the correct type");
            Assert.AreEqual (Stretch.Uniform, retval, "GetValue(StretchProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.IsMutedProperty);
            Assert.IsNotNull (retval, "GetValue(IsMutedProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsMutedProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsMutedProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.AutoPlayProperty);
            Assert.IsNotNull (retval, "GetValue(AutoPlayProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(AutoPlayProperty) is not of the correct type");
            Assert.AreEqual (true, retval, "GetValue(AutoPlayProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.VolumeProperty);
            Assert.IsNotNull (retval, "GetValue(VolumeProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(VolumeProperty) is not of the correct type");
            Assert.AreEqual (0.5, retval, "GetValue(VolumeProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.BalanceProperty);
            Assert.IsNotNull (retval, "GetValue(BalanceProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(BalanceProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(BalanceProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.NaturalVideoHeightProperty);
            Assert.IsNotNull (retval, "GetValue(NaturalVideoHeightProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(NaturalVideoHeightProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(NaturalVideoHeightProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.NaturalVideoWidthProperty);
            Assert.IsNotNull (retval, "GetValue(NaturalVideoWidthProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(NaturalVideoWidthProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(NaturalVideoWidthProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.NaturalDurationProperty);
            Assert.IsNotNull (retval, "GetValue(NaturalDurationProperty) should not have returned null");
            Assert.IsTrue (retval is Duration, "GetValue(NaturalDurationProperty) is not of the correct type");
            Assert.AreEqual ("00:00:00", retval.ToString (), "GetValue(NaturalDurationProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.PositionProperty);
            Assert.IsNotNull (retval, "GetValue(PositionProperty) should not have returned null");
            Assert.IsTrue (retval is TimeSpan, "GetValue(PositionProperty) is not of the correct type");
            Assert.AreEqual ("00:00:00", retval.ToString (), "GetValue(PositionProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.DownloadProgressProperty);
            Assert.IsNotNull (retval, "GetValue(DownloadProgressProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(DownloadProgressProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(DownloadProgressProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.BufferingProgressProperty);
            Assert.IsNotNull (retval, "GetValue(BufferingProgressProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(BufferingProgressProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(BufferingProgressProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.DownloadProgressOffsetProperty);
            Assert.IsNotNull (retval, "GetValue(DownloadProgressOffsetProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(DownloadProgressOffsetProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(DownloadProgressOffsetProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.CurrentStateProperty);
            Assert.IsNotNull (retval, "GetValue(CurrentStateProperty) should not have returned null");
            Assert.IsTrue (retval is MediaElementState, "GetValue(CurrentStateProperty) is not of the correct type");
            Assert.AreEqual (MediaElementState.Closed, retval, "GetValue(CurrentStateProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.BufferingTimeProperty);
            Assert.IsNotNull (retval, "GetValue(BufferingTimeProperty) should not have returned null");
            Assert.IsTrue (retval is TimeSpan, "GetValue(BufferingTimeProperty) is not of the correct type");
            Assert.AreEqual ("00:00:05", retval.ToString (), "GetValue(BufferingTimeProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.CanSeekProperty);
            Assert.IsNotNull (retval, "GetValue(CanSeekProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(CanSeekProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(CanSeekProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.CanPauseProperty);
            Assert.IsNotNull (retval, "GetValue(CanPauseProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(CanPauseProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(CanPauseProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.AudioStreamCountProperty);
            Assert.IsNotNull (retval, "GetValue(AudioStreamCountProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(AudioStreamCountProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(AudioStreamCountProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.AudioStreamIndexProperty);
            Assert.IsNull (retval, "GetValue(AudioStreamIndexProperty) should have returned null");

            retval = widget.GetValue (MediaElement.RenderedFramesPerSecondProperty);
            Assert.IsNotNull (retval, "GetValue(RenderedFramesPerSecondProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(RenderedFramesPerSecondProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(RenderedFramesPerSecondProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.DroppedFramesPerSecondProperty);
            Assert.IsNotNull (retval, "GetValue(DroppedFramesPerSecondProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(DroppedFramesPerSecondProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(DroppedFramesPerSecondProperty) does not match the default value");

            retval = widget.GetValue (MediaElement.AttributesProperty);
            Assert.IsNotNull (retval, "GetValue(AttributesProperty) should not have returned null");
        }

        [TestMethod]
        public void InkPresenter_ReadLocalValue ()
        {
            InkPresenter widget = new InkPresenter ();
            object retval;

            retval = widget.ReadLocalValue (InkPresenter.StrokesProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(StrokesProperty) should not have a value by default");
        }

        [TestMethod]
        public void InkPresenter_GetValue ()
        {
            InkPresenter widget = new InkPresenter ();
            object retval;

            retval = widget.GetValue (InkPresenter.StrokesProperty);
            Assert.IsNotNull (retval, "GetValue(StrokesProperty) should not have returned null");
            Assert.IsTrue (retval is StrokeCollection, "GetValue(StrokesProperty) is not of the correct type");
        }

        [TestMethod]
        public void MultiScaleImage_ReadLocalValue ()
        {
            MultiScaleImage widget = new MultiScaleImage ();
            object retval;

            retval = widget.ReadLocalValue (MultiScaleImage.SourceProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SourceProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MultiScaleImage.ViewportOriginProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ViewportOriginProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MultiScaleImage.ViewportWidthProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ViewportWidthProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MultiScaleImage.AspectRatioProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(AspectRatioProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MultiScaleImage.UseSpringsProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(UseSpringsProperty) should not have a value by default");

            //Assert.Throws<Exception>(delegate {
            //    retval = widget.ReadLocalValue (MultiScaleImage.SubImagesProperty);
            //}, "ReadLocalValue(SubImagesProperty) should thow an exception");
        }

        [TestMethod]
        public void MultiScaleImage_GetValue ()
        {
            MultiScaleImage widget = new MultiScaleImage ();
            object retval;

            retval = widget.GetValue (MultiScaleImage.SourceProperty);
            Assert.IsNull (retval, "GetValue(SourceProperty) should have returned null");

            retval = widget.GetValue (MultiScaleImage.ViewportOriginProperty);
            Assert.IsNotNull (retval, "GetValue(ViewportOriginProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(ViewportOriginProperty) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(ViewportOriginProperty) does not match the default value");

            retval = widget.GetValue (MultiScaleImage.ViewportWidthProperty);
            Assert.IsNotNull (retval, "GetValue(ViewportWidthProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(ViewportWidthProperty) is not of the correct type");
            Assert.AreEqual (1, retval, "GetValue(ViewportWidthProperty) does not match the default value");

            retval = widget.GetValue (MultiScaleImage.AspectRatioProperty);
            Assert.IsNotNull (retval, "GetValue(AspectRatioProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(AspectRatioProperty) is not of the correct type");
            Assert.AreEqual (1, retval, "GetValue(AspectRatioProperty) does not match the default value");

            retval = widget.GetValue (MultiScaleImage.UseSpringsProperty);
            Assert.IsNotNull (retval, "GetValue(UseSpringsProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(UseSpringsProperty) is not of the correct type");
            Assert.AreEqual (true, retval, "GetValue(UseSpringsProperty) does not match the default value");

            retval = widget.GetValue (MultiScaleImage.SubImagesProperty);
            Assert.IsNotNull (retval, "GetValue(SubImagesProperty) should not have returned null");
        }

        [TestMethod]
        public void MultiScaleSubImage_ReadLocalValue ()
        {
            MultiScaleSubImage widget = new MultiScaleSubImage ();
            object retval;

            retval = widget.ReadLocalValue (MultiScaleSubImage.ViewportOriginProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ViewportOriginProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MultiScaleSubImage.ViewportWidthProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ViewportWidthProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MultiScaleSubImage.AspectRatioProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(AspectRatioProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MultiScaleSubImage.ZIndexProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ZIndexProperty) should not have a value by default");

            retval = widget.ReadLocalValue (MultiScaleSubImage.OpacityProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(OpacityProperty) should not have a value by default");
        }

        [TestMethod]
        public void MultiScaleSubImage_GetValue ()
        {
            MultiScaleSubImage widget = new MultiScaleSubImage ();
            object retval;

            retval = widget.GetValue (MultiScaleSubImage.ViewportOriginProperty);
            Assert.IsNotNull (retval, "GetValue(ViewportOriginProperty) should not have returned null");
            Assert.IsTrue (retval is Point, "GetValue(ViewportOriginProperty) is not of the correct type");
            Assert.AreEqual ("0,0", retval.ToString (), "GetValue(ViewportOriginProperty) does not match the default value");

            retval = widget.GetValue (MultiScaleSubImage.ViewportWidthProperty);
            Assert.IsNotNull (retval, "GetValue(ViewportWidthProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(ViewportWidthProperty) is not of the correct type");
            Assert.AreEqual (1, retval, "GetValue(ViewportWidthProperty) does not match the default value");

            retval = widget.GetValue (MultiScaleSubImage.AspectRatioProperty);
            Assert.IsNotNull (retval, "GetValue(AspectRatioProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(AspectRatioProperty) is not of the correct type");
            Assert.AreEqual (1, retval, "GetValue(AspectRatioProperty) does not match the default value");

            retval = widget.GetValue (MultiScaleSubImage.ZIndexProperty);
            Assert.IsNotNull (retval, "GetValue(ZIndexProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(ZIndexProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(ZIndexProperty) does not match the default value");

            retval = widget.GetValue (MultiScaleSubImage.OpacityProperty);
            Assert.IsNotNull (retval, "GetValue(OpacityProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(OpacityProperty) is not of the correct type");
            Assert.AreEqual (1, retval, "GetValue(OpacityProperty) does not match the default value");
        }

        [TestMethod]
        public void StackPanel_ReadLocalValue ()
        {
            StackPanel widget = new StackPanel ();
            object retval;

            retval = widget.ReadLocalValue (StackPanel.OrientationProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(OrientationProperty) should not have a value by default");
        }

        [TestMethod]
        public void StackPanel_GetValue ()
        {
            StackPanel widget = new StackPanel ();
            object retval;

            retval = widget.GetValue (StackPanel.OrientationProperty);
            Assert.IsNotNull (retval, "GetValue(OrientationProperty) should not have returned null");
            Assert.IsTrue (retval is Orientation, "GetValue(OrientationProperty) is not of the correct type");
            Assert.AreEqual (Orientation.Vertical, retval, "GetValue(OrientationProperty) does not match the default value");
        }

        [TestMethod]
        public void TextBox_ReadLocalValue ()
        {
            TextBox widget = new TextBox ();
            object retval;

            retval = widget.ReadLocalValue (TextBox.TextProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TextProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBox.MaxLengthProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(MaxLengthProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBox.IsReadOnlyProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsReadOnlyProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBox.AcceptsReturnProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(AcceptsReturnProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBox.SelectionBackgroundProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SelectionBackgroundProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBox.SelectionForegroundProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SelectionForegroundProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBox.TextAlignmentProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TextAlignmentProperty) should not have a value by default");

            retval = widget.ReadLocalValue (TextBox.TextWrappingProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TextWrappingProperty) should not have a value by default");
        }

        [TestMethod]
        public void TextBox_GetValue ()
        {
            TextBox widget = new TextBox ();
            object retval;

            retval = widget.GetValue (TextBox.TextProperty);
            Assert.IsNull (retval, "GetValue(TextProperty) should have returned null");

            retval = widget.GetValue (TextBox.MaxLengthProperty);
            Assert.IsNotNull (retval, "GetValue(MaxLengthProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(MaxLengthProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(MaxLengthProperty) does not match the default value");

            retval = widget.GetValue (TextBox.IsReadOnlyProperty);
            Assert.IsNotNull (retval, "GetValue(IsReadOnlyProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsReadOnlyProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsReadOnlyProperty) does not match the default value");

            retval = widget.GetValue (TextBox.AcceptsReturnProperty);
            Assert.IsNotNull (retval, "GetValue(AcceptsReturnProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(AcceptsReturnProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(AcceptsReturnProperty) does not match the default value");

            retval = widget.GetValue (TextBox.SelectionBackgroundProperty);
            Assert.IsNull (retval, "GetValue(SelectionBackgroundProperty) should have returned null");

            retval = widget.GetValue (TextBox.SelectionForegroundProperty);
            Assert.IsNull (retval, "GetValue(SelectionForegroundProperty) should have returned null");

            retval = widget.GetValue (TextBox.TextAlignmentProperty);
            Assert.IsNotNull (retval, "GetValue(TextAlignmentProperty) should not have returned null");
            Assert.IsTrue (retval is TextAlignment, "GetValue(TextAlignmentProperty) is not of the correct type");
            Assert.AreEqual (TextAlignment.Left, retval, "GetValue(TextAlignmentProperty) does not match the default value");

            retval = widget.GetValue (TextBox.TextWrappingProperty);
            Assert.IsNotNull (retval, "GetValue(TextWrappingProperty) should not have returned null");
            Assert.IsTrue (retval is TextWrapping, "GetValue(TextWrappingProperty) is not of the correct type");
            Assert.AreEqual (TextWrapping.NoWrap, retval, "GetValue(TextWrappingProperty) does not match the default value");
        }

        [TestMethod]
        public void PasswordBox_ReadLocalValue ()
        {
            PasswordBox widget = new PasswordBox ();
            object retval;

            retval = widget.ReadLocalValue (PasswordBox.MaxLengthProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(MaxLengthProperty) should not have a value by default");

            retval = widget.ReadLocalValue (PasswordBox.SelectionBackgroundProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SelectionBackgroundProperty) should not have a value by default");

            retval = widget.ReadLocalValue (PasswordBox.SelectionForegroundProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(SelectionForegroundProperty) should not have a value by default");

            retval = widget.ReadLocalValue (PasswordBox.PasswordCharProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(PasswordCharProperty) should not have a value by default");
        }

        [TestMethod]
        public void PasswordBox_GetValue ()
        {
            PasswordBox widget = new PasswordBox ();
            object retval;

            retval = widget.GetValue (PasswordBox.MaxLengthProperty);
            Assert.IsNotNull (retval, "GetValue(MaxLengthProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(MaxLengthProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(MaxLengthProperty) does not match the default value");

            retval = widget.GetValue (PasswordBox.SelectionBackgroundProperty);
            Assert.IsNull (retval, "GetValue(SelectionBackgroundProperty) should have returned null");

            retval = widget.GetValue (PasswordBox.SelectionForegroundProperty);
            Assert.IsNull (retval, "GetValue(SelectionForegroundProperty) should have returned null");

            retval = widget.GetValue (PasswordBox.PasswordCharProperty);
            Assert.IsNotNull (retval, "GetValue(PasswordCharProperty) should not have returned null");
            Assert.IsTrue (retval is char, "GetValue(PasswordCharProperty) is not of the correct type");
            Assert.AreEqual ("●", retval.ToString (), "GetValue(PasswordCharProperty) does not match the default value");
        }

        [TestMethod]
        public void RowDefinition_ReadLocalValue ()
        {
            RowDefinition widget = new RowDefinition ();
            object retval;

            retval = widget.ReadLocalValue (RowDefinition.HeightProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(HeightProperty) should not have a value by default");

            retval = widget.ReadLocalValue (RowDefinition.MaxHeightProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(MaxHeightProperty) should not have a value by default");

            retval = widget.ReadLocalValue (RowDefinition.MinHeightProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(MinHeightProperty) should not have a value by default");
        }

        [TestMethod]
        public void RowDefinition_GetValue ()
        {
            RowDefinition widget = new RowDefinition ();
            object retval;

            retval = widget.GetValue (RowDefinition.HeightProperty);
            Assert.IsNotNull (retval, "GetValue(HeightProperty) should not have returned null");
            Assert.IsTrue (retval is GridLength, "GetValue(HeightProperty) is not of the correct type");
            Assert.AreEqual ("1*", retval.ToString (), "GetValue(HeightProperty) does not match the default value");

            retval = widget.GetValue (RowDefinition.MaxHeightProperty);
            Assert.IsNotNull (retval, "GetValue(MaxHeightProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(MaxHeightProperty) is not of the correct type");
            Assert.IsTrue (Double.IsPositiveInfinity ((double) retval), "GetValue(MaxHeightProperty) does not match the default value");

            retval = widget.GetValue (RowDefinition.MinHeightProperty);
            Assert.IsNotNull (retval, "GetValue(MinHeightProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(MinHeightProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(MinHeightProperty) does not match the default value");
        }

        [TestMethod]
        public void ColumnDefinition_ReadLocalValue ()
        {
            ColumnDefinition widget = new ColumnDefinition ();
            object retval;

            retval = widget.ReadLocalValue (ColumnDefinition.WidthProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(WidthProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ColumnDefinition.MaxWidthProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(MaxWidthProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ColumnDefinition.MinWidthProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(MinWidthProperty) should not have a value by default");
        }

        [TestMethod]
        public void ColumnDefinition_GetValue ()
        {
            ColumnDefinition widget = new ColumnDefinition ();
            object retval;

            retval = widget.GetValue (ColumnDefinition.WidthProperty);
            Assert.IsNotNull (retval, "GetValue(WidthProperty) should not have returned null");
            Assert.IsTrue (retval is GridLength, "GetValue(WidthProperty) is not of the correct type");
            Assert.AreEqual ("1*", retval.ToString (), "GetValue(WidthProperty) does not match the default value");

            retval = widget.GetValue (ColumnDefinition.MaxWidthProperty);
            Assert.IsNotNull (retval, "GetValue(MaxWidthProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(MaxWidthProperty) is not of the correct type");
            Assert.IsTrue (Double.IsPositiveInfinity ((double) retval), "GetValue(MaxWidthProperty) does not match the default value");

            retval = widget.GetValue (ColumnDefinition.MinWidthProperty);
            Assert.IsNotNull (retval, "GetValue(MinWidthProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(MinWidthProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(MinWidthProperty) does not match the default value");
        }

        [TestMethod]
        public void Grid_ReadLocalValue ()
        {
            Grid widget = new Grid ();
            object retval;

            retval = widget.ReadLocalValue (Grid.ShowGridLinesProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ShowGridLinesProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Grid.RowProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RowProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Grid.ColumnProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ColumnProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Grid.RowSpanProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(RowSpanProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Grid.ColumnSpanProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ColumnSpanProperty) should not have a value by default");
        }

        [TestMethod]
        public void Grid_GetValue ()
        {
            Grid widget = new Grid ();
            object retval;

            retval = widget.GetValue (Grid.ShowGridLinesProperty);
            Assert.IsNotNull (retval, "GetValue(ShowGridLinesProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(ShowGridLinesProperty) is not of the correct type");
            Assert.AreEqual (true, retval, "GetValue(ShowGridLinesProperty) does not match the default value");

            retval = widget.GetValue (Grid.RowProperty);
            Assert.IsNotNull (retval, "GetValue(RowProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(RowProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(RowProperty) does not match the default value");

            retval = widget.GetValue (Grid.ColumnProperty);
            Assert.IsNotNull (retval, "GetValue(ColumnProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(ColumnProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(ColumnProperty) does not match the default value");

            retval = widget.GetValue (Grid.RowSpanProperty);
            Assert.IsNotNull (retval, "GetValue(RowSpanProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(RowSpanProperty) is not of the correct type");
            Assert.AreEqual (1, retval, "GetValue(RowSpanProperty) does not match the default value");

            retval = widget.GetValue (Grid.ColumnSpanProperty);
            Assert.IsNotNull (retval, "GetValue(ColumnSpanProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(ColumnSpanProperty) is not of the correct type");
            Assert.AreEqual (1, retval, "GetValue(ColumnSpanProperty) does not match the default value");
        }

        [TestMethod]
        public void ItemsControl_ReadLocalValue ()
        {
            ItemsControl widget = new ItemsControl ();
            object retval;

            retval = widget.ReadLocalValue (ItemsControl.ItemTemplateProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ItemTemplateProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ItemsControl.ItemsPanelProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ItemsPanelProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ItemsControl.DisplayMemberPathProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(DisplayMemberPathProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ItemsControl.ItemsSourceProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ItemsSourceProperty) should not have a value by default");
        }

        [TestMethod]
        public void ItemsControl_GetValue ()
        {
            ItemsControl widget = new ItemsControl ();
            object retval;

            retval = widget.GetValue (ItemsControl.ItemTemplateProperty);
            Assert.IsNull (retval, "GetValue(ItemTemplateProperty) should have returned null");

            retval = widget.GetValue (ItemsControl.ItemsPanelProperty);
            Assert.IsNull (retval, "GetValue(ItemsPanelProperty) should have returned null");

            retval = widget.GetValue (ItemsControl.DisplayMemberPathProperty);
            Assert.IsNull (retval, "GetValue(DisplayMemberPathProperty) should have returned null");

            retval = widget.GetValue (ItemsControl.ItemsSourceProperty);
            Assert.IsNull (retval, "GetValue(ItemsSourceProperty) should have returned null");
        }

        [TestMethod]
        public void Border_ReadLocalValue ()
        {
            Border widget = new Border ();
            object retval;

            retval = widget.ReadLocalValue (Border.BorderBrushProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(BorderBrushProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Border.BorderThicknessProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(BorderThicknessProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Border.BackgroundProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(BackgroundProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Border.CornerRadiusProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CornerRadiusProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Border.PaddingProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(PaddingProperty) should not have a value by default");
        }

        [TestMethod]
        public void Border_GetValue ()
        {
            Border widget = new Border ();
            object retval;

            retval = widget.GetValue (Border.BorderBrushProperty);
            Assert.IsNull (retval, "GetValue(BorderBrushProperty) should have returned null");

            retval = widget.GetValue (Border.BorderThicknessProperty);
            Assert.IsNotNull (retval, "GetValue(BorderThicknessProperty) should not have returned null");
            Assert.IsTrue (retval is Thickness, "GetValue(BorderThicknessProperty) is not of the correct type");
            Assert.AreEqual ("0,0,0,0", retval.ToString (), "GetValue(BorderThicknessProperty) does not match the default value");

            retval = widget.GetValue (Border.BackgroundProperty);
            Assert.IsNull (retval, "GetValue(BackgroundProperty) should have returned null");

            retval = widget.GetValue (Border.CornerRadiusProperty);
            Assert.IsNotNull (retval, "GetValue(CornerRadiusProperty) should not have returned null");
            Assert.IsTrue (retval is CornerRadius, "GetValue(CornerRadiusProperty) is not of the correct type");
            Assert.AreEqual ("0,0,0,0", retval.ToString (), "GetValue(CornerRadiusProperty) does not match the default value");

            retval = widget.GetValue (Border.PaddingProperty);
            Assert.IsNotNull (retval, "GetValue(PaddingProperty) should not have returned null");
            Assert.IsTrue (retval is Thickness, "GetValue(PaddingProperty) is not of the correct type");
            Assert.AreEqual ("0,0,0,0", retval.ToString (), "GetValue(PaddingProperty) does not match the default value");
        }

        [TestMethod]
        public void ContentControl_ReadLocalValue ()
        {
            ContentControl widget = new ContentControl ();
            object retval;

            retval = widget.ReadLocalValue (ContentControl.ContentTemplateProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ContentTemplateProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ContentControl.ContentProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ContentProperty) should not have a value by default");
        }

        [TestMethod]
        public void ContentControl_GetValue ()
        {
            ContentControl widget = new ContentControl ();
            object retval;

            retval = widget.GetValue (ContentControl.ContentTemplateProperty);
            Assert.IsNull (retval, "GetValue(ContentTemplateProperty) should have returned null");

            retval = widget.GetValue (ContentControl.ContentProperty);
            Assert.IsNull (retval, "GetValue(ContentProperty) should have returned null");
        }

        [TestMethod]
        public void ContentPresenter_ReadLocalValue ()
        {
            ContentPresenter widget = new ContentPresenter ();
            object retval;

            retval = widget.ReadLocalValue (ContentPresenter.ContentTemplateProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ContentTemplateProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ContentPresenter.ContentProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ContentProperty) should not have a value by default");
        }

        [TestMethod]
        public void ContentPresenter_GetValue ()
        {
            ContentPresenter widget = new ContentPresenter ();
            object retval;

            retval = widget.GetValue (ContentPresenter.ContentTemplateProperty);
            Assert.IsNull (retval, "GetValue(ContentTemplateProperty) should have returned null");

            retval = widget.GetValue (ContentPresenter.ContentProperty);
            Assert.IsNull (retval, "GetValue(ContentProperty) should have returned null");
        }

        [TestMethod]
        public void BitmapImage_ReadLocalValue ()
        {
            BitmapImage widget = new BitmapImage ();
            object retval;

            retval = widget.ReadLocalValue (BitmapImage.UriSourceProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(UriSourceProperty) should not have a value by default");
        }

        [TestMethod]
        public void BitmapImage_GetValue ()
        {
            BitmapImage widget = new BitmapImage ();
            object retval;

            retval = widget.GetValue (BitmapImage.UriSourceProperty);
            Assert.IsNotNull (retval, "GetValue(UriSourceProperty) should not have returned null");
            Assert.IsTrue (retval is Uri, "GetValue(UriSourceProperty) is not of the correct type");
            Assert.AreEqual ("", retval.ToString (), "GetValue(UriSourceProperty) does not match the default value");
        }

        [TestMethod]
        public void Popup_ReadLocalValue ()
        {
            Popup widget = new Popup ();
            object retval;

            retval = widget.ReadLocalValue (Popup.ChildProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ChildProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Popup.IsOpenProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsOpenProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Popup.HorizontalOffsetProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(HorizontalOffsetProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Popup.VerticalOffsetProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(VerticalOffsetProperty) should not have a value by default");
        }

        [TestMethod]
        public void Popup_GetValue ()
        {
            Popup widget = new Popup ();
            object retval;

            retval = widget.GetValue (Popup.ChildProperty);
            Assert.IsNull (retval, "GetValue(ChildProperty) should have returned null");

            retval = widget.GetValue (Popup.IsOpenProperty);
            Assert.IsNotNull (retval, "GetValue(IsOpenProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsOpenProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsOpenProperty) does not match the default value");

            retval = widget.GetValue (Popup.HorizontalOffsetProperty);
            Assert.IsNotNull (retval, "GetValue(HorizontalOffsetProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(HorizontalOffsetProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(HorizontalOffsetProperty) does not match the default value");

            retval = widget.GetValue (Popup.VerticalOffsetProperty);
            Assert.IsNotNull (retval, "GetValue(VerticalOffsetProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(VerticalOffsetProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(VerticalOffsetProperty) does not match the default value");
        }

        [TestMethod]
        public void ToggleButton_ReadLocalValue ()
        {
            ToggleButton widget = new ToggleButton ();
            object retval;

            retval = widget.ReadLocalValue (ToggleButton.IsCheckedProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsCheckedProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ToggleButton.IsThreeStateProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsThreeStateProperty) should not have a value by default");
        }

        [TestMethod]
        public void ToggleButton_GetValue ()
        {
            ToggleButton widget = new ToggleButton ();
            object retval;

            retval = widget.GetValue (ToggleButton.IsCheckedProperty);
            Assert.IsNotNull (retval, "GetValue(IsCheckedProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsCheckedProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsCheckedProperty) does not match the default value");

            retval = widget.GetValue (ToggleButton.IsThreeStateProperty);
            Assert.IsNotNull (retval, "GetValue(IsThreeStateProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsThreeStateProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsThreeStateProperty) does not match the default value");
        }

        [TestMethod]
        public void ComboBox_ReadLocalValue ()
        {
            ComboBox widget = new ComboBox ();
            object retval;

            retval = widget.ReadLocalValue (ComboBox.IsDropDownOpenProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsDropDownOpenProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ComboBox.IsSelectionActiveProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsSelectionActiveProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ComboBox.ItemContainerStyleProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ItemContainerStyleProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ComboBox.MaxDropDownHeightProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(MaxDropDownHeightProperty) should not have a value by default");
        }

        [TestMethod]
        public void ComboBox_GetValue ()
        {
            ComboBox widget = new ComboBox ();
            object retval;

            retval = widget.GetValue (ComboBox.IsDropDownOpenProperty);
            Assert.IsNotNull (retval, "GetValue(IsDropDownOpenProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsDropDownOpenProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsDropDownOpenProperty) does not match the default value");

            retval = widget.GetValue (ComboBox.IsSelectionActiveProperty);
            Assert.IsNotNull (retval, "GetValue(IsSelectionActiveProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsSelectionActiveProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsSelectionActiveProperty) does not match the default value");

            retval = widget.GetValue (ComboBox.ItemContainerStyleProperty);
            Assert.IsNull (retval, "GetValue(ItemContainerStyleProperty) should have returned null");

            retval = widget.GetValue (ComboBox.MaxDropDownHeightProperty);
            Assert.IsNotNull (retval, "GetValue(MaxDropDownHeightProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(MaxDropDownHeightProperty) is not of the correct type");
            Assert.IsTrue (Double.IsPositiveInfinity ((double) retval), "GetValue(MaxDropDownHeightProperty) does not match the default value");
        }

        [TestMethod]
        public void ListBoxItem_ReadLocalValue ()
        {
            ListBoxItem widget = new ListBoxItem ();
            object retval;

            retval = widget.ReadLocalValue (ListBoxItem.IsSelectedProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsSelectedProperty) should not have a value by default");
        }

        [TestMethod]
        public void ListBoxItem_GetValue ()
        {
            ListBoxItem widget = new ListBoxItem ();
            object retval;

            retval = widget.GetValue (ListBoxItem.IsSelectedProperty);
            Assert.IsNotNull (retval, "GetValue(IsSelectedProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsSelectedProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsSelectedProperty) does not match the default value");
        }

        [TestMethod]
        public void HyperlinkButton_ReadLocalValue ()
        {
            HyperlinkButton widget = new HyperlinkButton ();
            object retval;

            retval = widget.ReadLocalValue (HyperlinkButton.NavigateUriProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(NavigateUriProperty) should not have a value by default");

            retval = widget.ReadLocalValue (HyperlinkButton.TargetNameProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(TargetNameProperty) should not have a value by default");
        }

        [TestMethod]
        public void HyperlinkButton_GetValue ()
        {
            HyperlinkButton widget = new HyperlinkButton ();
            object retval;

            retval = widget.GetValue (HyperlinkButton.NavigateUriProperty);
            Assert.IsNull (retval, "GetValue(NavigateUriProperty) should have returned null");

            retval = widget.GetValue (HyperlinkButton.TargetNameProperty);
            Assert.IsNull (retval, "GetValue(TargetNameProperty) should have returned null");
        }

        [TestMethod]
        public void ListBox_ReadLocalValue ()
        {
            ListBox widget = new ListBox ();
            object retval;

            retval = widget.ReadLocalValue (ListBox.IsSelectionActiveProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsSelectionActiveProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ListBox.ItemContainerStyleProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ItemContainerStyleProperty) should not have a value by default");
        }

        [TestMethod]
        public void ListBox_GetValue ()
        {
            ListBox widget = new ListBox ();
            object retval;

            retval = widget.GetValue (ListBox.IsSelectionActiveProperty);
            Assert.IsNotNull (retval, "GetValue(IsSelectionActiveProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsSelectionActiveProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsSelectionActiveProperty) does not match the default value");

            retval = widget.GetValue (ListBox.ItemContainerStyleProperty);
            Assert.IsNull (retval, "GetValue(ItemContainerStyleProperty) should have returned null");
        }

        [TestMethod]
        public void RepeatButton_ReadLocalValue ()
        {
            RepeatButton widget = new RepeatButton ();
            object retval;

            retval = widget.ReadLocalValue (RepeatButton.DelayProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(DelayProperty) should not have a value by default");

            retval = widget.ReadLocalValue (RepeatButton.IntervalProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IntervalProperty) should not have a value by default");
        }

        [TestMethod]
        public void RepeatButton_GetValue ()
        {
            RepeatButton widget = new RepeatButton ();
            object retval;

            retval = widget.GetValue (RepeatButton.DelayProperty);
            Assert.IsNotNull (retval, "GetValue(DelayProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(DelayProperty) is not of the correct type");
            Assert.AreEqual (500, retval, "GetValue(DelayProperty) does not match the default value");

            retval = widget.GetValue (RepeatButton.IntervalProperty);
            Assert.IsNotNull (retval, "GetValue(IntervalProperty) should not have returned null");
            Assert.IsTrue (retval is int, "GetValue(IntervalProperty) is not of the correct type");
            Assert.AreEqual (33, retval, "GetValue(IntervalProperty) does not match the default value");
        }

        [TestMethod]
        public void ScrollBar_ReadLocalValue ()
        {
            ScrollBar widget = new ScrollBar ();
            object retval;

            retval = widget.ReadLocalValue (ScrollBar.OrientationProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(OrientationProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollBar.ViewportSizeProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ViewportSizeProperty) should not have a value by default");
        }

        [TestMethod]
        public void ScrollBar_GetValue ()
        {
            ScrollBar widget = new ScrollBar ();
            object retval;

            retval = widget.GetValue (ScrollBar.OrientationProperty);
            Assert.IsNotNull (retval, "GetValue(OrientationProperty) should not have returned null");
            Assert.IsTrue (retval is Orientation, "GetValue(OrientationProperty) is not of the correct type");
            Assert.AreEqual (Orientation.Vertical, retval, "GetValue(OrientationProperty) does not match the default value");

            retval = widget.GetValue (ScrollBar.ViewportSizeProperty);
            Assert.IsNotNull (retval, "GetValue(ViewportSizeProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(ViewportSizeProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(ViewportSizeProperty) does not match the default value");
        }

        [TestMethod]
        public void Thumb_ReadLocalValue ()
        {
            Thumb widget = new Thumb ();
            object retval;

            retval = widget.ReadLocalValue (Thumb.IsDraggingProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsDraggingProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Thumb.IsFocusedProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsFocusedProperty) should not have a value by default");
        }

        [TestMethod]
        public void Thumb_GetValue ()
        {
            Thumb widget = new Thumb ();
            object retval;

            retval = widget.GetValue (Thumb.IsDraggingProperty);
            Assert.IsNotNull (retval, "GetValue(IsDraggingProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsDraggingProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsDraggingProperty) does not match the default value");

            retval = widget.GetValue (Thumb.IsFocusedProperty);
            Assert.IsNotNull (retval, "GetValue(IsFocusedProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsFocusedProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsFocusedProperty) does not match the default value");
        }

        [TestMethod]
        public void RadioButton_ReadLocalValue ()
        {
            RadioButton widget = new RadioButton ();
            object retval;

            retval = widget.ReadLocalValue (RadioButton.GroupNameProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(GroupNameProperty) should not have a value by default");
        }

        [TestMethod]
        public void RadioButton_GetValue ()
        {
            RadioButton widget = new RadioButton ();
            object retval;

            retval = widget.GetValue (RadioButton.GroupNameProperty);
            Assert.IsNull (retval, "GetValue(GroupNameProperty) should have returned null");
        }

        [TestMethod]
        public void ScrollViewer_ReadLocalValue ()
        {
            ScrollViewer widget = new ScrollViewer ();
            object retval;

            retval = widget.ReadLocalValue (ScrollViewer.HorizontalScrollBarVisibilityProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(HorizontalScrollBarVisibilityProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollViewer.VerticalScrollBarVisibilityProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(VerticalScrollBarVisibilityProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollViewer.HorizontalOffsetProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(HorizontalOffsetProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollViewer.ViewportWidthProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ViewportWidthProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollViewer.ScrollableWidthProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ScrollableWidthProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollViewer.ComputedHorizontalScrollBarVisibilityProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ComputedHorizontalScrollBarVisibilityProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollViewer.VerticalOffsetProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(VerticalOffsetProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollViewer.ViewportHeightProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ViewportHeightProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollViewer.ScrollableHeightProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ScrollableHeightProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollViewer.ComputedVerticalScrollBarVisibilityProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ComputedVerticalScrollBarVisibilityProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollViewer.ExtentHeightProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ExtentHeightProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ScrollViewer.ExtentWidthProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(ExtentWidthProperty) should not have a value by default");
        }

        [TestMethod]
        public void ScrollViewer_GetValue ()
        {
            ScrollViewer widget = new ScrollViewer ();
            object retval;

            retval = widget.GetValue (ScrollViewer.HorizontalScrollBarVisibilityProperty);
            Assert.IsNotNull (retval, "GetValue(HorizontalScrollBarVisibilityProperty) should not have returned null");
            Assert.IsTrue (retval is ScrollBarVisibility, "GetValue(HorizontalScrollBarVisibilityProperty) is not of the correct type");
            Assert.AreEqual (ScrollBarVisibility.Disabled, retval, "GetValue(HorizontalScrollBarVisibilityProperty) does not match the default value");

            retval = widget.GetValue (ScrollViewer.VerticalScrollBarVisibilityProperty);
            Assert.IsNotNull (retval, "GetValue(VerticalScrollBarVisibilityProperty) should not have returned null");
            Assert.IsTrue (retval is ScrollBarVisibility, "GetValue(VerticalScrollBarVisibilityProperty) is not of the correct type");
            Assert.AreEqual (ScrollBarVisibility.Disabled, retval, "GetValue(VerticalScrollBarVisibilityProperty) does not match the default value");

            retval = widget.GetValue (ScrollViewer.HorizontalOffsetProperty);
            Assert.IsNotNull (retval, "GetValue(HorizontalOffsetProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(HorizontalOffsetProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(HorizontalOffsetProperty) does not match the default value");

            retval = widget.GetValue (ScrollViewer.ViewportWidthProperty);
            Assert.IsNotNull (retval, "GetValue(ViewportWidthProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(ViewportWidthProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(ViewportWidthProperty) does not match the default value");

            retval = widget.GetValue (ScrollViewer.ScrollableWidthProperty);
            Assert.IsNotNull (retval, "GetValue(ScrollableWidthProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(ScrollableWidthProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(ScrollableWidthProperty) does not match the default value");

            retval = widget.GetValue (ScrollViewer.ComputedHorizontalScrollBarVisibilityProperty);
            Assert.IsNotNull (retval, "GetValue(ComputedHorizontalScrollBarVisibilityProperty) should not have returned null");
            Assert.IsTrue (retval is Visibility, "GetValue(ComputedHorizontalScrollBarVisibilityProperty) is not of the correct type");
            Assert.AreEqual (Visibility.Visible, retval, "GetValue(ComputedHorizontalScrollBarVisibilityProperty) does not match the default value");

            retval = widget.GetValue (ScrollViewer.VerticalOffsetProperty);
            Assert.IsNotNull (retval, "GetValue(VerticalOffsetProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(VerticalOffsetProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(VerticalOffsetProperty) does not match the default value");

            retval = widget.GetValue (ScrollViewer.ViewportHeightProperty);
            Assert.IsNotNull (retval, "GetValue(ViewportHeightProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(ViewportHeightProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(ViewportHeightProperty) does not match the default value");

            retval = widget.GetValue (ScrollViewer.ScrollableHeightProperty);
            Assert.IsNotNull (retval, "GetValue(ScrollableHeightProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(ScrollableHeightProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(ScrollableHeightProperty) does not match the default value");

            retval = widget.GetValue (ScrollViewer.ComputedVerticalScrollBarVisibilityProperty);
            Assert.IsNotNull (retval, "GetValue(ComputedVerticalScrollBarVisibilityProperty) should not have returned null");
            Assert.IsTrue (retval is Visibility, "GetValue(ComputedVerticalScrollBarVisibilityProperty) is not of the correct type");
            Assert.AreEqual (Visibility.Visible, retval, "GetValue(ComputedVerticalScrollBarVisibilityProperty) does not match the default value");

            retval = widget.GetValue (ScrollViewer.ExtentHeightProperty);
            Assert.IsNotNull (retval, "GetValue(ExtentHeightProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(ExtentHeightProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(ExtentHeightProperty) does not match the default value");

            retval = widget.GetValue (ScrollViewer.ExtentWidthProperty);
            Assert.IsNotNull (retval, "GetValue(ExtentWidthProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(ExtentWidthProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(ExtentWidthProperty) does not match the default value");
        }

        [TestMethod]
        public void Slider_ReadLocalValue ()
        {
            Slider widget = new Slider ();
            object retval;

            retval = widget.ReadLocalValue (Slider.OrientationProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(OrientationProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Slider.IsFocusedProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsFocusedProperty) should not have a value by default");

            retval = widget.ReadLocalValue (Slider.IsDirectionReversedProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsDirectionReversedProperty) should not have a value by default");
        }

        [TestMethod]
        public void Slider_GetValue ()
        {
            Slider widget = new Slider ();
            object retval;

            retval = widget.GetValue (Slider.OrientationProperty);
            Assert.IsNotNull (retval, "GetValue(OrientationProperty) should not have returned null");
            Assert.IsTrue (retval is Orientation, "GetValue(OrientationProperty) is not of the correct type");
            Assert.AreEqual (Orientation.Horizontal, retval, "GetValue(OrientationProperty) does not match the default value");

            retval = widget.GetValue (Slider.IsFocusedProperty);
            Assert.IsNotNull (retval, "GetValue(IsFocusedProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsFocusedProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsFocusedProperty) does not match the default value");

            retval = widget.GetValue (Slider.IsDirectionReversedProperty);
            Assert.IsNotNull (retval, "GetValue(IsDirectionReversedProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsDirectionReversedProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsDirectionReversedProperty) does not match the default value");
        }

        [TestMethod]
        public void ToolTip_ReadLocalValue ()
        {
            ToolTip widget = new ToolTip ();
            object retval;

            retval = widget.ReadLocalValue (ToolTip.HorizontalOffsetProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(HorizontalOffsetProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ToolTip.IsOpenProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsOpenProperty) should not have a value by default");

            retval = widget.ReadLocalValue (ToolTip.VerticalOffsetProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(VerticalOffsetProperty) should not have a value by default");
        }

        [TestMethod]
        public void ToolTip_GetValue ()
        {
            ToolTip widget = new ToolTip ();
            object retval;

            retval = widget.GetValue (ToolTip.HorizontalOffsetProperty);
            Assert.IsNotNull (retval, "GetValue(HorizontalOffsetProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(HorizontalOffsetProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(HorizontalOffsetProperty) does not match the default value");

            retval = widget.GetValue (ToolTip.IsOpenProperty);
            Assert.IsNotNull (retval, "GetValue(IsOpenProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsOpenProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsOpenProperty) does not match the default value");

            retval = widget.GetValue (ToolTip.VerticalOffsetProperty);
            Assert.IsNotNull (retval, "GetValue(VerticalOffsetProperty) should not have returned null");
            Assert.IsTrue (retval is double, "GetValue(VerticalOffsetProperty) is not of the correct type");
            Assert.AreEqual (0, retval, "GetValue(VerticalOffsetProperty) does not match the default value");
        }

        [TestMethod]
        public void ProgressBar_ReadLocalValue ()
        {
            ProgressBar widget = new ProgressBar ();
            object retval;

            retval = widget.ReadLocalValue (ProgressBar.IsIndeterminateProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(IsIndeterminateProperty) should not have a value by default");
        }

        [TestMethod]
        public void ProgressBar_GetValue ()
        {
            ProgressBar widget = new ProgressBar ();
            object retval;

            retval = widget.GetValue (ProgressBar.IsIndeterminateProperty);
            Assert.IsNotNull (retval, "GetValue(IsIndeterminateProperty) should not have returned null");
            Assert.IsTrue (retval is bool, "GetValue(IsIndeterminateProperty) is not of the correct type");
            Assert.AreEqual (false, retval, "GetValue(IsIndeterminateProperty) does not match the default value");
        }

        [TestMethod]
        public void VisualStateManager_ReadLocalValue ()
        {
            VisualStateManager widget = new VisualStateManager ();
            object retval;

            retval = widget.ReadLocalValue (VisualStateManager.CustomVisualStateManagerProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(CustomVisualStateManagerProperty) should not have a value by default");
        }

        [TestMethod]
        public void VisualStateManager_GetValue ()
        {
            VisualStateManager widget = new VisualStateManager ();
            object retval;

            retval = widget.GetValue (VisualStateManager.CustomVisualStateManagerProperty);
            Assert.IsNull (retval, "GetValue(CustomVisualStateManagerProperty) should have returned null");
        }

        [TestMethod]
        public void DeepZoomImageTileSource_ReadLocalValue ()
        {
            DeepZoomImageTileSource widget = new DeepZoomImageTileSource ();
            object retval;

            retval = widget.ReadLocalValue (DeepZoomImageTileSource.UriSourceProperty);
            Assert.AreEqual (DependencyProperty.UnsetValue, retval, "ReadLocalValue(UriSourceProperty) should not have a value by default");
        }

        [TestMethod]
        public void DeepZoomImageTileSource_GetValue ()
        {
            DeepZoomImageTileSource widget = new DeepZoomImageTileSource ();
            object retval;

            retval = widget.GetValue (DeepZoomImageTileSource.UriSourceProperty);
            Assert.IsNull (retval, "GetValue(UriSourceProperty) should have returned null");
        }

    }
}
