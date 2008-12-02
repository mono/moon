//------------------------------------------------------------------------------
// <copyright file="UtMethodDispatcher.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    using System;
    using System.Collections.Generic;
    using System.Reflection;
    using Microsoft.Silverlight.Testing.Utility;
    using LogMessages;
    using Microsoft.Silverlight.Testing.TestHarness.LogMessageTypes;

    /// <summary>
    /// 
    /// </summary>
    public class UtMethodDispatcher : MethodDispatcher
    {
        public UtMethodDispatcher(
            
            TestHarness.ITestHarness testHarness,
            object instance, 
            MethodInfo method, 
            TestGranularity granularity
            )

            : base(instance, method)
        {
            _granularity = granularity;
            _harness = testHarness as UnitTestHarness;
        }

        private UnitTestHarness _harness;

        private TestGranularity _granularity;

        protected override void FirstInvoke()
        {
            _harness.LogMessage(new StartMessage(_granularity, MethodInfo.Name));

            if (_harness != null && SupportsWorkItemQueue())
            {
                // Disable automatic completion of this dispatcher queue
                FinishWhenEmpty = false;

                // Connect
                _harness.DispatcherStack.Enqueue(this);

                // Disconnect
                Complete += delegate(object sender, EventArgs e)
                {
                    _harness.DispatcherStack.Dequeue();
                };
            }

            base.FirstInvoke();
        }

        private bool SupportsWorkItemQueue()
        {
            return (MethodInfo != null && ReflectionUtility.HasAttribute(MethodInfo,
                typeof(AsynchronousAttribute)));
        }

    }
}
