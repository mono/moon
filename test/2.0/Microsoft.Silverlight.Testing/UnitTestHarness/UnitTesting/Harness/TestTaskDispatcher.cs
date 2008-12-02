//------------------------------------------------------------------------------
// <copyright file="TestTaskDispatcher.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    using System;
    using System.Collections.Generic;

    public class TestWorkItemDispatcher : TestWorkItem, ITestWorkItemDispatcher
    {

        /// <summary>
        /// 
        /// </summary>
        private Queue<ITestWorkItem> _tasks;

        private bool _finishWhenEmpty;

        /// <summary>
        /// 
        /// </summary>
        public TestWorkItemDispatcher()
            : base()
        {
            _tasks = new Queue<ITestWorkItem>();
            _finishWhenEmpty = true;
        }

        public bool FinishWhenEmpty
        {
            get { return _finishWhenEmpty; }
            protected set { _finishWhenEmpty = value; }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public override bool Invoke() // return false when complete + event
        {
            if (_tasks.Count == 0) {
                if (_finishWhenEmpty) {
                    TestWorkItemComplete();
                    return false;
                }
                else { 
                    return true; /* still running, doing nothing explicitly */ 
                }
            }

            ITestWorkItem tt = _tasks.Peek();

            if (tt == null) {
                throw new NullReferenceException();
            }

            if (false == tt.Invoke()) {
                Dequeue();
            }

            return HasTasks();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public bool HasTasks()
        {
            return _tasks.Count > 0;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public ITestWorkItem Dequeue()
        {
            return _tasks.Dequeue();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="testTask"></param>
        public void Enqueue(ITestWorkItem testTask)
        {
            _tasks.Enqueue(testTask);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public ITestWorkItem Peek()
        {
            return _tasks.Peek();
        }

        /// <summary>
        /// 
        /// </summary>
        public event EventHandler Complete;

        /// <summary>
        /// 
        /// </summary>
        protected override void TestWorkItemComplete()
        {
            base.TestWorkItemComplete();
            OnComplete(EventArgs.Empty);
        }

        /// <summary>
        /// Internal-only version which can be called during a test completion 
        /// through the relation - not necessarily the best design; events 
        /// may make more sense long-term
        /// </summary>
        internal virtual void TestWorkItemCompleteInternal()
        {
            // TestWorkItemComplete(); // ??? BYPASS
            //REVIEW: TestWorkItemCompleteInternal ???
            IsComplete = true;
            _finishWhenEmpty = true; // ??
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="e"></param>
        private void OnComplete(EventArgs e)
        {
            if (Complete != null) {
                Complete(this, e);
            }
        }

    }
}
