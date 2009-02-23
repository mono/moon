using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace ApplyTemplateInMeasure
{
    class TestPanel : Panel
    {
        protected override Size MeasureOverride(Size availableSize)
        {
            Console.WriteLine("before calling child.Measure");
            Size s = base.MeasureOverride(availableSize);

            foreach (UIElement ui in Children)
            {
                ui.Measure(availableSize);
            }

            Console.WriteLine("After calling child.Measure");

            return s;
        }
    }

    class TestButton : Button
    {
        public TestButton()
        {
            Loaded += delegate
            {
                Console.WriteLine("TestButton.Loaded");
            };
        }

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();
            Console.WriteLine("TestButton.OnApplyTemplate");
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            Console.WriteLine("TestButton.MeasureOverride");
            return base.MeasureOverride(availableSize);
        }
    }
    public partial class Page : UserControl
    {
        public Page()
        {
            InitializeComponent();

            TestButton b = new TestButton();
            TestPanel p = new TestPanel();

            LayoutRoot.Children.Add(p);

            p.Children.Add(b);

            Console.WriteLine("hi");
        }
    }
}
