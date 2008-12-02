// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
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
        /// An interactive control for looking at the details.
        /// </summary>
        private class Details : HtmlDiv
        {
            /// <summary>
            /// Gets the parent ResultInspector instance.
            /// </summary>
            protected ResultInspector Inspector { get; private set; }

            /// <summary>
            /// Gets a value indicating whether the data has already been 
            /// loaded.
            /// </summary>
            protected bool DataLoaded { get; private set; }

            /// <summary>
            /// Gets the unit test harness.
            /// </summary>
            protected UnitTestHarness UnitTestHarness
            {
                get { return Inspector.Result.TestClass.Assembly.TestHarness as UnitTestHarness; }
            }

            /// <summary>
            /// Initializes a new Details object.
            /// </summary>
            /// <param name="inspector">The parent inspector control.</param>
            public Details(ResultInspector inspector)
                : base()
            {
                Inspector = inspector;
                InitializeComponent();
            }

            /// <summary>
            /// Toggles the visibility.
            /// </summary>
            public void ToggleVisibility()
            {
                if (!DataLoaded)
                {
                    SetStyleAttribute(CssAttribute.Display, "block");
                    PopulateData();
                }
                Visible = !Visible;
            }

            /// <summary>
            /// Allow the Details control to decorate the parent container.
            /// </summary>
            /// <returns>Returns a set of control bases to decorate with.</returns>
            public virtual IEnumerable<HtmlControlBase> DecorateContainer()
            {
                return Enumerable.Empty<HtmlControlBase>();
            }

            /// <summary>
            /// Initializes the control.
            /// </summary>
            private void InitializeComponent()
            {
                Visible = false;
                BackgroundColor = Color.White;
                Padding.All = 4;
                BorderWidth = 1;
                BorderStyle = BorderStyle.Solid;
                BorderColor = Color.LightGray;
                Padding.All = 2;
                Margin.Left = 2;
            }

            /// <summary>
            /// Populates and creates the controls for showing the detailed 
            /// information about the result.
            /// </summary>
            protected virtual void PopulateData()
            {
                HtmlControl actions = CreateContainer("Actions:");
                AddRetryAction(actions);
                AddCopyAction(actions);
            
                Controls.Add(TempCreateInformationGrid());
                if (Inspector.Result.Exception != null) 
                {
                    HtmlControl trace = CreateContainer("Stack trace:");
                    HtmlDiv traceText = new HtmlDiv();
                    traceText.InnerHtml = HttpUtility.HtmlEncode(Inspector.Result.Exception.StackTrace);
                    traceText.Height = 80;
                    traceText.SetStyleAttribute(CssAttribute.Overflow, "auto");
                    // TODO: Style as fixed text
                    trace.Controls.Add(traceText);

                    Controls.Add(trace);
                }
                Controls.Add(actions);

                DataLoaded = true;
            }

            /// <summary>
            /// Temporary method to create the grid.
            /// </summary>
            /// <returns>Returns the grid control.</returns>
            private HtmlControl TempCreateInformationGrid()
            {
                ScenarioResult result = Inspector.Result;
                Dictionary<string, string> info = new Dictionary<string, string>
                {
                    { "Assembly:", result.TestClass.Assembly.Name },
                    { "Namespace:", result.TestClass.Type.Namespace },
                    { "Test class:", result.TestClass.Name },
                    { "Test method:", result.TestMethod.Name }
                };
                if (!string.IsNullOrEmpty(result.TestMethod.Description))
                {
                    info.Add("Description:", result.TestMethod.Description);
                }
                return CreateInformationGrid(info);
            }

            /// <summary>
            /// Apply bold formatting to all sub-controls in a row.
            /// </summary>
            /// <param name="row">Row control.</param>
            private static void BoldRow(HtmlControl row)
            {
                for (int i = 0; i < row.Controls.Count; ++i)
                {
                    HtmlControl c = row.Controls[i] as HtmlControl;
                    if (c != null)
                    {
                        c.Font.Bold = true;
                    }
                }
            }

            /// <summary>
            /// Adds the retry action link.
            /// </summary>
            /// <param name="actions">The action container.</param>
            private void AddRetryAction(HtmlControl actions)
            {
                HtmlAnchor rerun = new HtmlAnchor(
                    "Retry this test",
                    delegate(object sender, HtmlEventArgs args)
                    {
                        RetryTestRunFilter retryFilter = new RetryTestRunFilter(Inspector.Result.TestClass, Inspector.Result.TestMethod);
                        UnitTestHarness ut = UnitTestHarness;
                        if (ut != null)
                        {
                            ut.RestartRunDispatcher();
                            ut.EnqueueTestAssembly(Inspector.Result.TestClass.Assembly, retryFilter);
                        }
                    });
                StyleActionLink(rerun);
                actions.Controls.Add(rerun);
            }

            /// <summary>
            /// Adds the copy information action.
            /// </summary>
            /// <param name="actions">The action container.</param>
            private void AddCopyAction(HtmlControl actions)
            {
                HtmlAnchor copy = new HtmlAnchor(
                    "Copy result details",
                    delegate(object sender, HtmlEventArgs args)
                    {
                        HtmlPage.Window.Alert(Inspector.Result.ToString());
                    });
                StyleActionLink(copy);
                actions.Controls.Add(copy);
            }

            /// <summary>
            /// Customizes the anchor for action use.
            /// </summary>
            /// <param name="anchor">The anchor.</param>
            private static void StyleActionLink(HtmlAnchor anchor)
            {
                anchor.SetStyleAttribute(CssAttribute.Display, CssDisplay.Block);
            }

            /// <summary>
            /// Insert time information for the scenario.
            /// </summary>
            /// <param name="grid">The property grid.</param>
            private void InsertScenarioTimeInformation(HtmlPropertyGrid grid)
            {
                ScenarioResult result = Inspector.Result;
                string elapsed = UnitTestWebpageLog.ElapsedReadableTime(result.Started, result.Finished);
                HtmlControl row = grid.AddRow("Elapsed time:", elapsed);
                grid.AddRow("Started:", result.Started.ToString(CultureInfo.CurrentUICulture) + " +" + result.Started.Millisecond.ToString(System.Globalization.CultureInfo.CurrentCulture) + " ms");
                grid.AddRow("Finished:", result.Finished.ToString(CultureInfo.CurrentUICulture) + " +" + result.Finished.Millisecond.ToString(System.Globalization.CultureInfo.CurrentCulture) + " ms");
                BoldRow(row);
            }

            /// <summary>
            /// Creates the informational property grid for the failure's 
            /// details.
            /// </summary>
            /// <param name="gridInformation">The information to display inside 
            /// the grid.</param>
            /// <returns>Returns the information grid.</returns>
            private HtmlControl CreateInformationGrid(IDictionary<string, string> gridInformation)
            {
                HtmlPropertyGrid grid = new HtmlPropertyGrid(gridInformation);
                InsertScenarioTimeInformation(grid);
                grid.SetAttribute(HtmlAttribute.Cellpadding, "2");
                grid.SetAttribute(HtmlAttribute.Cellspacing, "0");
                grid.SetAttribute(HtmlAttribute.Border, "0");
                return grid;
            }

            /// <summary>
            /// Creates a boxed container.
            /// </summary>
            /// <returns>Returns the new container.</returns>
            protected static HtmlControl CreateContainer()
            {
                HtmlDiv container = new HtmlDiv();
                container.BorderColor = Color.VeryLightGray;
                container.BorderStyle = BorderStyle.Solid;
                container.BorderWidth = 1;
                container.Padding.All = 2;
                container.Margin.Bottom = container.Margin.Top = 1;
                return container;
            }

            /// <summary>
            /// Creates a boxed container.
            /// </summary>
            /// <param name="header">The header for the container.</param>
            /// <returns>Returns the new container.</returns>
            protected static HtmlControl CreateContainer(string header)
            {
                HtmlControl control = CreateContainer();
                HtmlSpan txt = new HtmlSpan();
                txt.InnerText = header;
                txt.SetStyleAttribute(CssAttribute.Display, CssDisplay.Block);
                control.Controls.Add(txt);
                return control;
            }
        }
    }
}