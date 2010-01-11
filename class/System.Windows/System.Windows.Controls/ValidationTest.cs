using System.Windows.Controls;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public partial class ValidationTest {

		[TestMethod]
		public void GetErrors_Null ()
		{
			Assert.Throws <ArgumentNullException> (() => {
				Validation.GetErrors (null);
			}, "#1");
		}

		[TestMethod]
		public void GetHasErrors_Null ()
		{
			Assert.Throws <ArgumentNullException> (() => {
				Validation.GetHasError (null);
			}, "#1");
		}
	}
}