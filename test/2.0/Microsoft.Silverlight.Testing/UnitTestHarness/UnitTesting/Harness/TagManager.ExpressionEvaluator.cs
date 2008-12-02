// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// A helper class that manages tags and associated metadata. Tag
    /// expressions are evaluated at the TestClass level.
    /// </summary>
    public partial class TagManager
    {
        /// <summary>
        /// Evaluate tag expressions.
        /// </summary>
        /// <remarks>
        /// Tag expressions are derived from the following EBNF grammar:
        ///     {Expression} :=
        ///         {Expression} + {Term} |
        ///         {Expression} - {Term} |
        ///         {Term}
        ///     {Term} :=
        ///         {Term} * {Factor} |
        ///         {Factor}
        ///     {Factor} :=
        ///         !{Factor} |
        ///         ({Expression}) |
        ///         {Tag}
        ///     {Tag} :=
        ///         All |
        ///         [^InvalidCharacters]+
        ///  
        /// The non-terminals for {Expression} and {Term} will be left factored
        /// in the recursive descent parser below.
        /// </remarks>
        private class ExpressionEvaluator
        {
            /// <summary>
            /// Union character.
            /// </summary>
            private const string Union = "+";

            /// <summary>
            /// Intersection character.
            /// </summary>
            private const string Intersection = "*";

            /// <summary>
            /// Complement character.
            /// </summary>
            private const string Complement = "!";

            /// <summary>
            /// Difference character.
            /// </summary>
            private const string Difference = "-";

            /// <summary>
            /// The "All" string constant.
            /// </summary>
            private const string All = "all";

            /// <summary>
            /// Invalid characters in a tag name.
            /// </summary>
            private static char[] InvalidCharacters = new char[] 
            { 
                Union[0],
                Intersection[0], 
                Complement[0], 
                Difference[0], 
                '(', 
                ')', 
                '/' 
            };

            /// <summary>
            /// Evaluate a tag expression.
            /// </summary>
            /// <param name="owner">The owner object.</param>
            /// <param name="tagExpression">Tag expression.</param>
            /// <returns>Test methods associated with the tag expression.</returns>
            public static IEnumerable<ITestMethod> Evaluate(TagManager owner, string tagExpression)
            {
                return new ExpressionEvaluator(owner, tagExpression).Evaluate();
            }

            /// <summary>
            /// The owning TagManager instance.
            /// </summary>
            private TagManager _owner;

            /// <summary>
            /// Expression being evaluated.
            /// </summary>
            private string _tagExpression;

            /// <summary>
            /// Current position in the expression.
            /// </summary>
            private int _position;

            /// <summary>
            /// Create an expression evaluator.
            /// </summary>
            /// <param name="owner">The owner object.</param>
            /// <param name="tagExpression">Expression object.</param>
            private ExpressionEvaluator(TagManager owner, string tagExpression)
            {
                if (tagExpression == null)
                {
                    throw new ArgumentNullException("tagExpression");
                }
                else if (tagExpression.Length == 0)
                {
                    throw new ArgumentException(Properties.UnitTestMessage.TagManager_ExpressionEvaluator_EmptyTagExpression, "tagExpression");
                }
                _owner = owner;
                _tagExpression = tagExpression;
            }

            /// <summary>
            /// Match a sequence of characters.
            /// </summary>
            /// <param name="expected">String to match.</param>
            private void Match(string expected)
            {
                if (!TryMatch(expected))
                {
                    throw new FormatException(string.Format(CultureInfo.CurrentCulture, Properties.UnitTestMessage.TagManager_ExpressionEvaluator_InvalidTagExpression, _tagExpression, expected, _position));
                }
            }

            /// <summary>
            /// Try to match a sequence of characters.
            /// </summary>
            /// <param name="expected">String to match.</param>
            /// <returns>Returns a value indicating whether the match was 
            /// successful.</returns>
            private bool TryMatch(string expected)
            {
                if (_position + expected.Length > _tagExpression.Length)
                {
                    return false;
                }

                for (int i = 0; i < expected.Length; i++)
                {
                    if (_tagExpression[i + _position] != expected[i])
                    {
                        return false;
                    }
                }

                _position += expected.Length;
                return true;
            }

            /// <summary>
            /// Evaluate an expression.
            /// </summary>
            /// <returns>Test methods described by the expression.</returns>
            private IEnumerable<ITestMethod> Evaluate()
            {
                IEnumerable<ITestMethod> expression = ReadExpression();
                if (_position >= 0 && _position < _tagExpression.Length)
                {
                    throw new FormatException(string.Format(CultureInfo.CurrentCulture, Properties.UnitTestMessage.TagManager_ExpressionEvaluator_ExpectedEndOfTagExpression, _tagExpression, _position));
                }
                return expression;
            }

            /// <summary>
            /// Evaluate an expression.
            /// </summary>
            /// <returns>Test methods described by the expression.</returns>
            /// <remarks>
            /// We need to factor out left recursion, so:
            ///     {Expression} :=
            ///         {Expression} + {Term} |
            ///         {Expression} - {Term} |
            ///         {Term}
            /// becomes:
            ///     {Expression} :=
            ///     	{Term}{Expression'}
            ///     
            ///     {Expression'} :=
            ///     	#empty#
            ///     	+ {Term}{Expression'}
            ///     	- {Term}{Expression'}
            /// </remarks>
            private IEnumerable<ITestMethod> ReadExpression()
            {
                return ReadExpression(ReadTerm());
            }

            /// <summary>
            /// Evaluate an expression.
            /// </summary>
            /// <param name="term">
            /// Left term already read as part of the expression.
            /// </param>
            /// <returns>Test methods described by the expression.</returns>
            /// <remarks>
            /// Non-terminal created for left-factoring:
            ///     {Expression'} :=
            ///     	#empty#
            ///     	+ {Term}{Expression'}
            ///     	- {Term}{Expression'}
            /// </remarks>
            private IEnumerable<ITestMethod> ReadExpression(IEnumerable<ITestMethod> term)
            {
                if (TryMatch(Union))
                {
                    return ReadExpression(term.Union(ReadTerm()));
                }
                else if (TryMatch(Difference))
                {
                    return ReadExpression(term.Except(ReadTerm()));
                }
                else
                {
                    return term;
                }
            }

            /// <summary>
            /// Evaluate a term.
            /// </summary>
            /// <returns>Test methods described by the expression.</returns>
            /// <remarks>
            /// We need to factor out left recursion, so:
            ///     {Term} :=
            ///         {Factor} * {Term} |
            ///         {Factor}
            /// becomes:
            ///     {Term} :=
            ///         {Factor}{Term'}
            /// 
            ///     {Term'} :=
            ///     	#empty#
            ///     	^ {Factor}{Term'}
            /// </remarks>
            private IEnumerable<ITestMethod> ReadTerm()
            {
                return ReadTerm(ReadFactor());
            }

            /// <summary>
            /// Evaluate a term.
            /// </summary>
            /// <param name="factor">
            /// Left term already read as part of the expression.
            /// </param>
            /// <returns>Test methods described by the expression.</returns>
            /// <remarks>
            /// Non-terminal created for left-factoring:
            ///     {Term'} :=
            ///     	#empty#
            ///     	^ {Factor}{Term'}
            /// </remarks>
            private IEnumerable<ITestMethod> ReadTerm(IEnumerable<ITestMethod> factor)
            {
                if (TryMatch(Intersection))
                {
                    return ReadTerm(factor.Intersect(ReadFactor()));
                }
                else
                {
                    return factor;
                }
            }

            /// <summary>
            /// Evaluate a factor.
            /// </summary>
            /// <returns>Test methods described by the expression.</returns>
            /// <remarks>
            /// {Factor} :=
            ///     !{Factor} |
            ///     ({Expression}) |
            ///     {Tag}
            /// </remarks>
            private IEnumerable<ITestMethod> ReadFactor()
            {
                if (TryMatch(Complement))
                {
                    IEnumerable<ITestMethod> allMethods = _owner.Universe;
                    return allMethods.Except(ReadFactor());
                }
                else if (TryMatch("("))
                {
                    IEnumerable<ITestMethod> expression = ReadExpression();
                    Match(")");
                    return expression;
                }
                else
                {
                    return ReadTag();
                }
            }

            /// <summary>
            /// Creates a new empty collection.
            /// </summary>
            /// <returns>Returns an empty collection.</returns>
            private static List<ITestMethod> CreateEmptyList()
            {
                return new List<ITestMethod>();
            }

            /// <summary>
            /// Evaluate a tag.
            /// </summary>
            /// <returns>Test methods described by the expression.</returns>
            /// <remarks>
            /// {Tag} :=
            ///     All |
            ///     [^InvalidCharacters]+
            /// </remarks>
            private IEnumerable<ITestMethod> ReadTag()
            {
                int end = _tagExpression.IndexOfAny(InvalidCharacters, _position);
                if (end < 0)
                {
                    end = _tagExpression.Length;
                }
                string tag = _tagExpression.Substring(_position, end - _position);
                if (string.IsNullOrEmpty(tag))
                {
                    throw new FormatException(string.Format(CultureInfo.CurrentCulture, Properties.UnitTestMessage.TagManager_ExpressionEvaluator_ExpectedTag, _tagExpression, _position));
                }
                _position = end;
                if (string.Compare(tag, All, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    return new List<ITestMethod>(_owner.Universe);
                }
                else
                {
                    List<ITestMethod> methods;
                    if (!_owner._tagsToMethods.TryGetValue(tag, out methods))
                    {
                        methods = CreateEmptyList();
                    }
                    return methods;
                }
            }
        }
    }
}