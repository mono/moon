// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections;
using System.Diagnostics.CodeAnalysis;
using System.Text;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.Harness.Service;

namespace Microsoft.Silverlight.Testing
{
    /// <summary>
    /// The CodeCoverage class is used to collect code coverage information from
    /// assemblies that have been instrumented to call the Visit function at the
    /// beginning of every basic block.
    /// </summary>
    public static partial class CodeCoverage
    {
        /// <summary>
        /// A bit array used to track which basic blocks have been executed.
        /// </summary>
        private static BitArray _blocks = new BitArray(0);

        /// <summary>
        /// Record that a basic block is being executed.
        /// </summary>
        /// <param name="id">Id of the basic block.</param>
        public static void Visit(uint id)
        {
            int block = (int) id;
            if (_blocks.Length <= block)
            {
                _blocks.Length = block + 1;
            }
            _blocks[block] = true;
        }

        /// <summary>
        /// Gets the current size of the blocks counter. This is not actually 
        /// the number of hit blocks, but it should return 0 always except 
        /// when at least one block is hit.
        /// </summary>
        public static int HitBlockCount
        {
            get { return _blocks.Count; }
        }

        /// <summary>
        /// Get the coverage data serialized as a string for easy transport.
        /// </summary>
        /// <returns>Coverage data serialized as a string.</returns>
        [SuppressMessage("Microsoft.Design", "CA1024:UsePropertiesWhereAppropriate", Justification = "A method is more appropriate.")]
        public static string GetCoverageData()
        {
            StringBuilder data = new StringBuilder(_blocks.Length);
            foreach (bool bit in _blocks)
            {
                data.Append(bit ? '1' : '0');
            }
            return data.ToString();
        }
    }
}