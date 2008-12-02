//------------------------------------------------------------------------------
// <copyright file="TestTaskDispatcherWithMethod.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    using System;
    using System.Collections.Generic;
    using System.Reflection;

    public abstract class TestTaskDispatcherWithMethod : TestWorkItemDispatcher
    {
        public TestTaskDispatcherWithMethod()
            : base()
        {
            _invoked = false;
        }

        public override bool Invoke()
        {
            if (!_invoked) {
                _invoked = true;
                FirstInvoke();

                // REVIEW: may not actually be the case
                //return true; // more to run
            }

            return base.Invoke();
        }

        protected abstract void FirstInvoke();

        private bool _invoked;

    }
}
