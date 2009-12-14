// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Globalization;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Test
{
    /// <summary>
    /// Base class for overridden method tests.
    /// </summary>
    public abstract partial class OverriddenMethodBase
    {
        /// <summary>
        /// Number of times the method has been called.
        /// </summary>
        public int NumberOfTimesCalled { get; private set; }

        /// <summary>
        /// Test action to peform before and after the other tests.
        /// </summary>
        public Action InvariantTest { get; private set; }

        /// <summary>
        /// Initializes a new instance of the OverriddenMethodBase class.
        /// </summary>
        public OverriddenMethodBase()
            : this(null)
        {
        }

        /// <summary>
        /// Initializes a new instance of the OverriddenMethodBase class.
        /// </summary>
        /// <param name="invariantTest">
        /// Test action to peform before and after the other tests.
        /// </param>
        public OverriddenMethodBase(Action invariantTest)
        {
            InvariantTest = invariantTest;
            if (invariantTest != null)
            {
                invariantTest();
            }
        }

        /// <summary>
        /// Perform the invariant test action.
        /// </summary>
        public void DoInvariantTest()
        {
            if (InvariantTest != null)
            {
                InvariantTest();
            }
        }

        /// <summary>
        /// Perform the test action before the base implementation is invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public virtual void DoPreTest(params object[] parameters)
        {
            NumberOfTimesCalled++;
        }

        /// <summary>
        /// Perform the test action after the base implementation was invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public virtual void DoTest(params object[] parameters)
        {
        }

        /// <summary>
        /// Validate the parameters for a test action.
        /// </summary>
        /// <param name="parameters">
        /// Parameters supplied to a test action.
        /// </param>
        /// <param name="expectedLength">
        /// Expected number of parameters for the test action.
        /// </param>
        protected void ValidateParameters(object[] parameters, int expectedLength)
        {
            if (parameters == null)
            {
                if (expectedLength != 0)
                {
                    throw new ArgumentNullException("parameters");
                }
            }
            else if (parameters.Length != expectedLength)
            {
                throw new ArgumentException(
                    string.Format(CultureInfo.InvariantCulture,
                        "Expected {0} parameters, not {1}!",
                        expectedLength,
                        parameters.Length),
                    "parameters");
            }
        }

        /// <summary>
        /// Get a parameter for a test action of a specific type.
        /// </summary>
        /// <typeparam name="T">
        /// Type of the parameters for the test action.
        /// </typeparam>
        /// <param name="value">Parameter for a test action.</param>
        /// <returns>Parameter for a test action as a specific type.</returns>
        protected T GetParameter<T>(object[] parameters, int index)
        {
            Debug.Assert(parameters != null, "parameters should not be null!");
            Debug.Assert(index >= 0 && index < parameters.Length, "index is out of range!");

            object value = parameters[index];
            try
            {
                return (T) value;
            }
            catch (InvalidCastException)
            {
                throw new ArgumentException(
                    string.Format(CultureInfo.InvariantCulture,
                        "Cannot cast parameter {0} of type {2} to type {1}!",
                        index,
                        typeof(T).FullName,
                        value == null ? "null" : value.GetType().FullName),
                    "parameters");
            }
        }

        /// <summary>
        /// Create a method call monitor to track whether the method is called.
        /// </summary>
        /// <returns></returns>
        public MethodCallMonitor CreateMonitor()
        {
            return new MethodCallMonitor(this);
        }
    }

    /// <summary>
    /// Monitor whether a method is called.
    /// </summary>
    public sealed class MethodCallMonitor
    {
        /// <summary>
        /// Number of times the method had been called when we start monitoring
        /// (which we use to determine if it was called or not).
        /// </summary>
        private int _initialNumberOfTimesCalled;

        /// <summary>
        /// Method to monitor.
        /// </summary>
        public OverriddenMethodBase Method { get; private set; }

        /// <summary>
        /// Prevent external instantiation of the MethodCallMonitor class.
        /// </summary>
        private MethodCallMonitor()
        {
        }

        /// <summary>
        /// Initializes a new instance of the MethodCallMonitor class.
        /// </summary>
        /// <param name="method">Method to monitor.</param>
        internal MethodCallMonitor(OverriddenMethodBase method)
        {
            Debug.Assert(method != null,
                "Method to monitor cannot be null!");

            Method = method;
            Reset();
        }

        /// <summary>
        /// Require that the method is called.
        /// </summary>
        public void AssertCalled()
        {
            AssertCalled("The required method call did not occur!");
        }

        /// <summary>
        /// Require that the method is called.
        /// </summary>
        /// <param name="message">Assertion message.</param>
        public void AssertCalled(string message)
        {
            Assert.AreNotEqual(
                _initialNumberOfTimesCalled,
                Method.NumberOfTimesCalled,
                message);
        }

        /// <summary>
        /// Require that the method is not called.
        /// </summary>
        public void AssertNotCalled()
        {
            AssertNotCalled("The forbidden method call occured!");
        }

        /// <summary>
        /// Require that the method is not called.
        /// </summary>
        /// <param name="message">Assertion message.</param>
        public void AssertNotCalled(string message)
        {
            Assert.AreEqual(
                _initialNumberOfTimesCalled,
                Method.NumberOfTimesCalled,
                message);
        }

        /// <summary>
        /// Reset the monitor.
        /// </summary>
        public void Reset()
        {
            _initialNumberOfTimesCalled = Method.NumberOfTimesCalled;
        }
    }

    /// <summary>
    /// Overridden method tests for methods with no parameters.
    /// </summary>
    public sealed partial class OverriddenMethod : OverriddenMethodBase
    {
        /// <summary>
        /// Test action to peform before the base implementation is invoked.
        /// </summary>
        public event Action PreTest;

        /// <summary>
        /// Test action to peform after the base implementation was invoked.
        /// </summary>
        public event Action Test;

        /// <summary>
        /// Initializes a new instance of the OverriddenMethod class.
        /// </summary>
        public OverriddenMethod()
            : this(null)
        {
        }

        /// <summary>
        /// Initializes a new instance of the OverriddenMethod class.
        /// </summary>
        /// <param name="invariantTest">
        /// Test action to peform before and after the other tests.
        /// </param>
        public OverriddenMethod(Action invariantTest)
            : base(invariantTest)
        {
        }

        /// <summary>
        /// Invoke the test action.
        /// </summary>
        /// <param name="test">Test action to invoke.</param>
        private void InvokeTest(Action test, object[] parameters)
        {
            DoInvariantTest();

            ValidateParameters(parameters, 0);
            if (test != null)
            {
                test();
            }

            DoInvariantTest();
        }

        /// <summary>
        /// Perform the test action before the base implementation is invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public override void DoPreTest(params object[] parameters)
        {
            base.DoPreTest();
            InvokeTest(PreTest, parameters);
        }

        /// <summary>
        /// Perform the test action after the base implementation was invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public override void DoTest(params object[] parameters)
        {
            base.DoTest();
            InvokeTest(Test, parameters);
        }
    }

    /// <summary>
    /// Overridden method tests for methods with 1 parameter.
    /// </summary>
    /// <typeparam name="T1">Type of the method's parameter.</typeparam>
    public sealed partial class OverriddenMethod<T1> : OverriddenMethodBase
    {
        /// <summary>
        /// Test action to peform before the base implementation is invoked.
        /// </summary>
        public event Action<T1> PreTest;

        /// <summary>
        /// Test action to peform after the base implementation was invoked.
        /// </summary>
        public event Action<T1> Test;

        /// <summary>
        /// Initializes a new instance of the OverriddenMethod class.
        /// </summary>
        public OverriddenMethod()
            : this(null)
        {
        }

        /// <summary>
        /// Initializes a new instance of the OverriddenMethod class.
        /// </summary>
        /// <param name="invariantTest">
        /// Test action to peform before and after the other tests.
        /// </param>
        public OverriddenMethod(Action invariantTest)
            : base(invariantTest)
        {
        }

        /// <summary>
        /// Invoke the test action.
        /// </summary>
        /// <param name="test">Test action to invoke.</param>
        private void InvokeTest(Action<T1> test, object[] parameters)
        {
            DoInvariantTest();

            ValidateParameters(parameters, 1);
            T1 first = GetParameter<T1>(parameters, 0);
            if (test != null)
            {
                test(first);
            }

            DoInvariantTest();
        }

        /// <summary>
        /// Perform the test action before the base implementation is invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public override void DoPreTest(params object[] parameters)
        {
            base.DoPreTest();
            InvokeTest(PreTest, parameters);
        }

        /// <summary>
        /// Perform the test action after the base implementation was invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public override void DoTest(params object[] parameters)
        {
            base.DoTest();
            InvokeTest(Test, parameters);
        }
    }

    /// <summary>
    /// Overridden method tests for methods with 2 parameters.
    /// </summary>
    /// <typeparam name="T1">Type of the method's first parameter.</typeparam>
    /// <typeparam name="T2">Type of the method's second parameter.</typeparam>
    public sealed partial class OverriddenMethod<T1, T2> : OverriddenMethodBase
    {
        /// <summary>
        /// Test action to peform before the base implementation is invoked.
        /// </summary>
        public event Action<T1, T2> PreTest;

        /// <summary>
        /// Test action to peform after the base implementation was invoked.
        /// </summary>
        public event Action<T1, T2> Test;

        /// <summary>
        /// Initializes a new instance of the OverriddenMethod class.
        /// </summary>
        public OverriddenMethod()
            : this(null)
        {
        }

        /// <summary>
        /// Initializes a new instance of the OverriddenMethod class.
        /// </summary>
        /// <param name="invariantTest">
        /// Test action to peform before and after the other tests.
        /// </param>
        public OverriddenMethod(Action invariantTest)
            : base(invariantTest)
        {
        }

        /// <summary>
        /// Invoke the test action.
        /// </summary>
        /// <param name="test">Test action to invoke.</param>
        private void InvokeTest(Action<T1, T2> test, object[] parameters)
        {
            DoInvariantTest();

            ValidateParameters(parameters, 2);
            T1 first = GetParameter<T1>(parameters, 0);
            T2 second = GetParameter<T2>(parameters, 1);
            if (test != null)
            {
                test(first, second);
            }

            DoInvariantTest();
        }

        /// <summary>
        /// Perform the test action before the base implementation is invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public override void DoPreTest(params object[] parameters)
        {
            base.DoPreTest();
            InvokeTest(PreTest, parameters);
        }

        /// <summary>
        /// Perform the test action after the base implementation was invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public override void DoTest(params object[] parameters)
        {
            base.DoTest();
            InvokeTest(Test, parameters);
        }
    }

    /// <summary>
    /// Overridden method tests for methods with 3 parameters.
    /// </summary>
    /// <typeparam name="T1">Type of the method's first parameter.</typeparam>
    /// <typeparam name="T2">Type of the method's second parameter.</typeparam>
    /// <typeparam name="T3">Type of the method's third parameter.</typeparam>
    public sealed partial class OverriddenMethod<T1, T2, T3> : OverriddenMethodBase
    {
        /// <summary>
        /// Test action to peform before the base implementation is invoked.
        /// </summary>
        public event Action<T1, T2, T3> PreTest;

        /// <summary>
        /// Test action to peform after the base implementation was invoked.
        /// </summary>
        public event Action<T1, T2, T3> Test;

        /// <summary>
        /// Initializes a new instance of the OverriddenMethod class.
        /// </summary>
        public OverriddenMethod()
            : this(null)
        {
        }

        /// <summary>
        /// Initializes a new instance of the OverriddenMethod class.
        /// </summary>
        /// <param name="invariantTest">
        /// Test action to peform before and after the other tests.
        /// </param>
        public OverriddenMethod(Action invariantTest)
            : base(invariantTest)
        {
        }

        /// <summary>
        /// Invoke the test action.
        /// </summary>
        /// <param name="test">Test action to invoke.</param>
        private void InvokeTest(Action<T1, T2, T3> test, object[] parameters)
        {
            DoInvariantTest();

            ValidateParameters(parameters, 3);
            T1 first = GetParameter<T1>(parameters, 0);
            T2 second = GetParameter<T2>(parameters, 1);
            T3 third = GetParameter<T3>(parameters, 2);
            if (test != null)
            {
                test(first, second, third);
            }

            DoInvariantTest();
        }

        /// <summary>
        /// Perform the test action before the base implementation is invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public override void DoPreTest(params object[] parameters)
        {
            base.DoPreTest();
            InvokeTest(PreTest, parameters);
        }

        /// <summary>
        /// Perform the test action after the base implementation was invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public override void DoTest(params object[] parameters)
        {
            base.DoTest();
            InvokeTest(Test, parameters);
        }
    }

    /// <summary>
    /// Overridden method tests for methods with 3 parameters.
    /// </summary>
    /// <typeparam name="T1">Type of the method's first parameter.</typeparam>
    /// <typeparam name="T2">Type of the method's second parameter.</typeparam>
    /// <typeparam name="T3">Type of the method's third parameter.</typeparam>
    /// <typeparam name="T4">Type of the method's fourth parameter.</typeparam>
    public sealed partial class OverriddenMethod<T1, T2, T3, T4> : OverriddenMethodBase
    {
        /// <summary>
        /// Test action to peform before the base implementation is invoked.
        /// </summary>
        public event Action<T1, T2, T3, T4> PreTest;

        /// <summary>
        /// Test action to peform after the base implementation was invoked.
        /// </summary>
        public event Action<T1, T2, T3, T4> Test;

        /// <summary>
        /// Initializes a new instance of the OverriddenMethod class.
        /// </summary>
        public OverriddenMethod()
            : this(null)
        {
        }

        /// <summary>
        /// Initializes a new instance of the OverriddenMethod class.
        /// </summary>
        /// <param name="invariantTest">
        /// Test action to peform before and after the other tests.
        /// </param>
        public OverriddenMethod(Action invariantTest)
            : base(invariantTest)
        {
        }

        /// <summary>
        /// Invoke the test action.
        /// </summary>
        /// <param name="test">Test action to invoke.</param>
        private void InvokeTest(Action<T1, T2, T3, T4> test, object[] parameters)
        {
            DoInvariantTest();

            ValidateParameters(parameters, 4);
            T1 first = GetParameter<T1>(parameters, 0);
            T2 second = GetParameter<T2>(parameters, 1);
            T3 third = GetParameter<T3>(parameters, 2);
            T4 fourth = GetParameter<T4>(parameters, 3);
            if (test != null)
            {
                test(first, second, third, fourth);
            }

            DoInvariantTest();
        }

        /// <summary>
        /// Perform the test action before the base implementation is invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public override void DoPreTest(params object[] parameters)
        {
            base.DoPreTest();
            InvokeTest(PreTest, parameters);
        }

        /// <summary>
        /// Perform the test action after the base implementation was invoked.
        /// </summary>
        /// <param name="parameters">Parameters to the test action.</param>
        public override void DoTest(params object[] parameters)
        {
            base.DoTest();
            InvokeTest(Test, parameters);
        }
    }
}
