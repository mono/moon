using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Data;
using MoonTest.System.ComponentModel;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;

namespace MoonTest.System.Windows.Data {

    [TestClass]
    public class PropertyGroupDescriptionTest {

        [TestMethod]
        public void Constructor_1 ()
        {
            var p = new PropertyGroupDescription ();
            ConstructorCore (p, null, null, StringComparison.Ordinal);
        }

        [TestMethod]
        public void Constructor_2 ()
        {
            var p = new PropertyGroupDescription ("");
            ConstructorCore (p, "", null, StringComparison.Ordinal);
        }

        [TestMethod]
        public void Constructor_3 ()
        {
            var p = new PropertyGroupDescription (null);
            ConstructorCore (p, null, null, StringComparison.Ordinal);
        }

        [TestMethod]
        public void Constructor_4 ()
        {
            var p = new PropertyGroupDescription (null, null);
            ConstructorCore (p, null, null, StringComparison.Ordinal);
        }

        [TestMethod]
        public void Constructor_5 ()
        {
            var p = new PropertyGroupDescription (null, null, StringComparison.OrdinalIgnoreCase);
            ConstructorCore (p, null, null, StringComparison.OrdinalIgnoreCase);
        }

        [TestMethod]
        public void ConstructorDoesNotRaisePropertyChanged ()
        {
            var p = new ConcretePropertyGroupDescription ();
            Assert.AreEqual (0, p.OnPropertyChangedCalled.Count, "#1");
        }

        [TestMethod]
        public void GroupNameFromItem_Converter ()
        {
            // An invalid name means return null
            var ob = new Rectangle { Width = 100 };
            var converter = new ValueConverter {
                Converter = (value, targetType, parameter, culture) => {
                    return Convert.ToInt32 (value) - 50;
                }
            };

            var p = new PropertyGroupDescription ("Width", converter);
            var result = p.GroupNameFromItem (ob, 0, null);
            Assert.IsInstanceOfType<int> (result, "#1");
            Assert.AreEqual (50, (int) result, "#2");
        }

        [TestMethod]
        public void GroupNameFromItem_Converter_CheckParameters()
        {
            // An invalid name means return null
            var ob = new Rectangle { Width = 100 };
            var converter = new ValueConverter {
                Converter = (value, targetType, parameter, culture) => {
                    Assert.IsInstanceOfType<double> (value, "#1");
                    Assert.AreEqual (100.0, (double) value, "#2");

                    Assert.AreSame (typeof (object), targetType, "#3");

                    Assert.IsInstanceOfType<int> (parameter, "#4");
                    Assert.AreEqual (77, (int) parameter, "#5");

                    Assert.IsNull (culture, "#6");
                    return 50;
                }
            };

            var p = new PropertyGroupDescription ("Width", converter);
            p.GroupNameFromItem (ob, 77, null);
        }

        [TestMethod]
        public void GroupNameFromItem_NullPropertyName ()
        {
            // A null name means 'use the object'
            var ob = new object ();
            var p = new ConcretePropertyGroupDescription (null);
            Assert.AreSame (ob, p.GroupNameFromItem (ob, 0, null));
        }

        [TestMethod]
        public void GroupNameFromItem_EmptyPropertyName ()
        {
            // An empty name means 'use the object'
            var ob = new object ();
            var p = new ConcretePropertyGroupDescription ("");
            Assert.AreSame (ob, p.GroupNameFromItem (ob, 0, null));
        }

        [TestMethod]
        public void GroupNameFromItem_InvalidName ()
        {
            // An invalid name means return null
            var ob = new object ();
            var p = new ConcretePropertyGroupDescription ("invalid");
            Assert.IsNull (p.GroupNameFromItem (ob, 0, null));
        }

        [TestMethod]
        public void GroupNameFromItem_ValidName ()
        {
            // An invalid name means return null
            var ob = new Rectangle { Width = 100 };
            var p = new ConcretePropertyGroupDescription ("Width");
            var result = p.GroupNameFromItem (ob, 0, null);
            Assert.IsInstanceOfType<double> (result, "#1");
            Assert.AreEqual (100.0, (double) result, "#2");
        }

        [TestMethod]
        public void GroupNameFromItem_Indexer ()
        {
            // An invalid name means return null
            var o = new Dictionary<string, string> ();
            o.Add ("test", "result");
            var p = new ConcretePropertyGroupDescription ("[test]");
            Assert.AreEqual ("result", p.GroupNameFromItem (o, 0, null));
        }

        [TestMethod]
        public void NamesMatch_IgnoreCase ()
        {
            var p = new PropertyGroupDescription (null, null, StringComparison.OrdinalIgnoreCase);
            Assert.IsTrue (p.NamesMatch ("a", "A"), "#1");
        }

        [TestMethod]
        public void NamesMatch_NotConverterToString ()
        {
            var p = new PropertyGroupDescription (null, null, StringComparison.OrdinalIgnoreCase);
            Assert.IsFalse (p.NamesMatch ('a', 'A'), "#1");
        }

        void ConstructorCore (PropertyGroupDescription p, string name, IValueConverter converter, StringComparison comparison)
        {
            Assert.AreEqual (p.PropertyName, name, "#1");
            Assert.AreEqual (p.Converter, converter, "#2");
            Assert.AreEqual (p.StringComparison, comparison, "#3");
        }
    }

    class ValueConverter : IValueConverter {

        public Func<object, Type, object, CultureInfo, object> Converter { get; set; }
        public Func<object, Type, object, CultureInfo, object> ConverterBack { get; set; }

        public object Convert (object value, Type targetType, object parameter, CultureInfo culture)
        {
            return Converter (value, targetType, parameter, culture);
        }

        public object ConvertBack (object value, Type targetType, object parameter, CultureInfo culture)
        {
            return ConverterBack (value, targetType, parameter, culture);
        }
    }

    class ConcretePropertyGroupDescription : PropertyGroupDescription {
        public List<string> OnPropertyChangedCalled = new List<string> ();
        public List<string> PropertyChangedFired = new List<string> ();

        public ConcretePropertyGroupDescription ()
            : this (null)
        {

        }
        
        public ConcretePropertyGroupDescription (string name)
            : base (name)
        {
            PropertyChanged += (o, e) => {
                PropertyChangedFired.Add (e.PropertyName);
            };
        }

        protected override void OnPropertyChanged (PropertyChangedEventArgs e)
        {
            OnPropertyChangedCalled.Add (e.PropertyName);
            base.OnPropertyChanged (e);
        }
    }
}
