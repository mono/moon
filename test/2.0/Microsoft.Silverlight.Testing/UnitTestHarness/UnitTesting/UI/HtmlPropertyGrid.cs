// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using Microsoft.Silverlight.Testing.Html;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// A simple grid.
    /// </summary>
    public class HtmlPropertyGrid : HtmlControl
    {
        /// <summary>
        /// Table body.
        /// </summary>
        private HtmlControl _body;

        /// <summary>
        /// Initializes the property grid.
        /// </summary>
        public HtmlPropertyGrid() : base(HtmlTag.Table)
        {
            InitializeComponent();
        }

        /// <summary>
        /// Initializes the property grid with an initial set of string data.
        /// </summary>
        /// <param name="initialData">The initial string data.</param>
        public HtmlPropertyGrid(IDictionary<string, string> initialData) : this()
        {
            if (initialData != null)
            {
                foreach (string key in initialData.Keys)
                {
                    AddRow(key, initialData[key]);
                }
            }
        }

        /// <summary>
        /// Adds a row to the table.
        /// </summary>
        /// <param name="property">The property cell.</param>
        /// <param name="value">The value cell.</param>
        /// <returns>Returns the row HtmlControl.</returns>
        public HtmlControl AddRow(HtmlControl property, HtmlControl value)
        {
            HtmlControl row = new HtmlControl(HtmlTag.Tr);
            row.Controls.Add(CreateCell(property));
            row.Controls.Add(CreateCell(value));
            _body.Controls.Add(row);
            return row;
        }

        /// <summary>
        /// Adds a row to the table.
        /// </summary>
        /// <param name="property">The property value.</param>
        /// <param name="value">The string value.</param>
        /// <returns>Returns the row HtmlControl.</returns>
        public HtmlControl AddRow(string property, string value)
        {
            return AddRow(new Paragraph(property), new Paragraph(value));
        }

        /// <summary>
        /// Adds a row to the table.
        /// </summary>
        /// <param name="property">The property value.</param>
        /// <param name="value">The string value.</param>
        /// <returns>Returns the row HtmlControl.</returns>
        public HtmlControl AddRow(string property, HtmlControl value)
        {
            return AddRow(new Paragraph(property), value);
        }

        /// <summary>
        /// Creates a cell element with content.
        /// </summary>
        /// <param name="content">The content of the cell.</param>
        /// <returns>Returns the cell.</returns>
        private static HtmlControl CreateCell(HtmlControl content)
        {
            HtmlControl cell = new HtmlControl(HtmlTag.Td);
            cell.Controls.Add(content);
            return cell;
        }

        /// <summary>
        /// Initializes the component.
        /// </summary>
        private void InitializeComponent()
        {
            _body = new HtmlControl(HtmlTag.Tbody);
            Controls.Add(_body);
        }
    }
}