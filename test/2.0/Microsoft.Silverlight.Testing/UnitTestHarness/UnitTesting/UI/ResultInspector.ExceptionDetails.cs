// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.Html;
using Microsoft.Silverlight.Testing.UI;
using Microsoft.Silverlight.Testing.UnitTesting.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// An interactive web control for examining a test method's results and 
    /// detailed information.
    /// </summary>
    public partial class ResultInspector : HtmlDiv
    {
        /// <summary>
        /// An interactive control for looking at failure details.
        /// </summary>
        private class ExceptionDetails : Details
        {
            /// <summary>
            /// A standard unit test framework namespace for stripping out.
            /// </summary>
            private const string Utf = "Microsoft.VisualStudio.TestTools.UnitTesting.";

            /// <summary>
            /// Initializes a new FailureDetails control.
            /// </summary>
            /// <param name="inspector">The parent inspector object.</param>
            public ExceptionDetails(ResultInspector inspector)
                : base(inspector)
            {
            }

            /// <summary>
            /// Allow the Details control to decorate the parent container.
            /// </summary>
            /// <returns>Returns a set of control bases to decorate with.</returns>
            public override IEnumerable<HtmlControlBase> DecorateContainer()
            {
                string type = Inspector.Result.Exception.GetType().ToString();
                if (type.Contains(Utf))
                {
                    type = type.Remove(0, Utf.Length);
                }
                if (type.StartsWith("System.", StringComparison.OrdinalIgnoreCase))
                {
                    type = type.Substring(7);
                }
                string message = Inspector.Result.Exception.Message;

                HtmlSpan ce = new HtmlSpan();
                ce.InnerHtml = type;
                ce.SetStyleAttribute(CssAttribute.Display, CssDisplay.InlineBlock);
                ce.Margin.All = 2;
                ce.Padding.All = 4;
                ce.BackgroundColor = Color.Tan;
                yield return ce;

                HtmlSpan oc = new HtmlSpan();
                oc.InnerText = " occurred";
                yield return oc;

                HtmlDiv cm = new HtmlDiv();
                cm.Margin.All = 4;
                cm.Padding.All = 1;
                cm.InnerHtml = message;
                yield return cm;
            }
        }
    }
}