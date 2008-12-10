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
using System.Windows.Markup;

namespace Mono.Moonlight.UnitTesting
{
    [TestClass]
    public class PasswordBoxTest
    {
        PasswordBox box;

        [TestInitialize]
        public void Setup()
        {
            box = new PasswordBox();
        }

        [TestMethod]
        public void Defaults()
        {
            Assert.AreEqual(null, box.FontSource, "#1");
            Assert.AreEqual(0, box.MaxLength, "#2");
            Assert.AreEqual("", box.Password, "#3");
            Assert.AreEqual((char)9679, box.PasswordChar, "#4");
            Assert.AreEqual(null, box.SelectionBackground, "#5");
            Assert.AreEqual(null, box.SelectionForeground, "#6");
        }

        [TestMethod]
        public void SetInvalidValues()
        {
            Assert.Throws<ArgumentNullException>(delegate {
                box.Password = null;
            });
            Assert.Throws<ArgumentOutOfRangeException>(delegate {
                box.MaxLength = -1;
            });
            Assert.Throws<ArgumentException>(delegate {
                box.SetValue(PasswordBox.MaxLengthProperty, -1);
            });
            box.MaxLength = Int32.MaxValue;

            Assert.Throws<XamlParseException>(delegate {
                object block = XamlReader.Load(@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">

	<PasswordBox MaxLength=""-1"" />
</Canvas>");
            });
        }
    }
}
