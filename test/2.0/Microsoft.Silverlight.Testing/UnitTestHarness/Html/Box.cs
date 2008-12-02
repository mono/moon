// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A simple type that tracks the 4 coordinates in a box - left, top, 
    /// bottom, right, for CSS use.
    /// </summary>
    public class Box
    {
        /// <summary>
        /// Gets or sets the left Unit.
        /// </summary>
        public virtual Unit Left 
        { 
            get; 
            set; 
        }

        /// <summary>
        /// Gets or sets the top Unit.
        /// </summary>
        public virtual Unit Top
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the bottom Unit.
        /// </summary>
        public virtual Unit Bottom
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the right Unit.
        /// </summary>
        public virtual Unit Right
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets all four box components at once, to the same value.
        /// </summary>
        public Unit All
        {
            get
            {
                if (Bottom == Left && Left == Right && Right == Top)
                {
                    return Bottom;
                }
                return Unit.Empty;
            }

            set
            {
                Bottom = Left = Right = Top = value;
            }
        }
    }
}