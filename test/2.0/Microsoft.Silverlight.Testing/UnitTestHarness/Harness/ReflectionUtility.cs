// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// A set of helper methods for interacting with methods and types that are
    /// marked with attributes.
    /// </summary>
    public static class ReflectionUtility
    {
        /// <summary>
        /// Retrieve all types in an assembly that are decorated with a specific
        /// attribute.
        /// </summary>
        /// <param name="assembly">Assembly to search.</param>
        /// <param name="decoratingAttribute">
        /// Instance of the Type of attribute that marks interesting methods.
        /// </param>
        /// <returns>
        /// A collection of types from the assembly.  An empty collection is
        /// returned if no types were found matching the constraints.
        /// </returns>
        public static ICollection<Type> GetTypesWithAttribute(Assembly assembly, Type decoratingAttribute)
        {
            if (assembly == null)
            {
                throw new ArgumentNullException("assembly");
            }

            ICollection<Type> types = new List<Type>();
            foreach (Type type in assembly.GetExportedTypes())
            {
                if (!type.IsClass)
                {
                    continue;
                }

                if (type.IsDefined(decoratingAttribute, false))
                {
                    types.Add(type);
                }
            }
            return types;
        }

        /// <summary>
        /// Given a MethodInfo type, returns the attributes (if any) that are of 
        /// the decoratingAttribute parameter's type.
        /// </summary>
        /// <param name="member">MemberInfo instance.</param>
        /// <param name="decoratingAttribute">Attribute of interest.</param>
        /// <returns>
        /// A collection populated with the Attribute instances.
        /// </returns>
        public static ICollection<Attribute> GetAttributes(MemberInfo member, Type decoratingAttribute)
        {
            return GetAttributes(member, decoratingAttribute, false);
        }

        /// <summary>
        /// Given a MethodInfo type, returns the attributes (if any) that are of 
        /// the decoratingAttribute parameter's type.
        /// </summary>
        /// <param name="member">MemberInfo instance.</param>
        /// <param name="decoratingAttribute">Attribute of interest.</param>
        /// <param name="inherit">A value indicating whether to look for 
        /// inheriting custom attributes.</param>
        /// <returns>
        /// A collection populated with the Attribute instances.
        /// </returns>
        public static ICollection<Attribute> GetAttributes(MemberInfo member, Type decoratingAttribute, bool inherit)
        {
            if (member == null)
            {
                throw new ArgumentNullException("member");
            }
            return new List<Attribute>(member.GetCustomAttributes(decoratingAttribute, inherit).OfType<Attribute>());
        }

        /// <summary>
        /// Given a test method, returns the attributes (if any) that are of 
        /// the decoratingAttribute parameter's type.
        /// </summary>
        /// <param name="method">ITestMethod instance.</param>
        /// <param name="decoratingAttribute">Attribute of interest.</param>
        /// <param name="inherit">Whether to inherit attributes.</param>
        /// <returns>
        /// A collection populated with the Attribute instances.
        /// </returns>
        public static ICollection<Attribute> GetAttributes(ITestMethod method, Type decoratingAttribute, bool inherit)
        {
            if (method == null)
            {
                throw new ArgumentNullException("method");
            }
            else if (decoratingAttribute == null)
            {
                throw new ArgumentNullException("decoratingAttribute");
            }
            
            // Get the attributes defined on the method
            ICollection<Attribute> attributes = GetAttributes(method.Method, decoratingAttribute, inherit);
            
            // Get any dynamic attributes for the method
            IEnumerable<Attribute> dynamicAttributes = method.GetDynamicAttributes();
            if (dynamicAttributes != null)
            {
                foreach (Attribute attribute in dynamicAttributes)
                {
                    if (decoratingAttribute.IsAssignableFrom(attribute.GetType()))
                    {
                        attributes.Add(attribute);
                    }
                }
            }

            return attributes;
        }

        /// <summary>
        /// Retrieve a list of Reflection.Assembly types from a set of 
        /// instances and/or types.
        /// </summary>
        /// <param name="objects">Set of instances or types.</param>
        /// <returns>A set of Assembly instances from the instances or types.</returns>
        public static IList<Assembly> GetAssembliesFromInstances(IEnumerable<object> objects)
        {
            List<Assembly> assemblies = new List<Assembly>();
            foreach (object o in objects ?? new object[] { })
            {
                if (o == null)
                {
                    continue;
                }

                Type type = o.GetType();
                if (assemblies.Contains(type.Assembly))
                {
                    continue;
                }

                assemblies.Add(type.Assembly);
            }
            return assemblies;
        }

        /// <summary>
        /// Returns a list of unique assemblies from a set of types.
        /// </summary>
        /// <param name="types">Set of types.</param>
        /// <returns>
        /// A list of unique Assembly instances from the input types.
        /// </returns>
        public static IList<Assembly> GetAssemblies(params Type[] types)
        {
            List<Assembly> assemblies = new List<Assembly>();
            foreach (Type type in types ?? new Type[] { })
            {
                if (type == null || assemblies.Contains(type.Assembly))
                {
                    continue;
                }

                assemblies.Add(type.Assembly);
            }
            return assemblies;
        }

        /// <summary>
        /// Given a list of attributes, retrieves a single Attribute instance.
        /// Will throw an exception if multiple attributes exist on the method.
        /// </summary>
        /// <param name="attributes">List of attributes.</param>
        /// <returns>
        /// The attribute instance, or null if it does not exist.
        /// </returns>
        private static Attribute GetAttribute(ICollection<Attribute> attributes)
        {
            if (attributes.Count > 1)
            {
                // TODO: Resource string for too many N > 1 entries
                throw new InvalidOperationException();
            }

            // Return first attribute only
            return attributes.FirstOrDefault();
        }

        /// <summary>
        /// Given a method and a decorative attribute of interest, retrieves a
        /// single Attribute instance.  Will throw an exception if multiple
        /// attributes exist on the method.
        /// </summary>
        /// <param name="member">MemberInfo instance.</param>
        /// <param name="decoratingAttribute">
        /// Attribute type of interest.
        /// </param>
        /// <returns>
        /// The attribute instance, or null if it does not exist.
        /// </returns>
        public static Attribute GetAttribute(MemberInfo member, Type decoratingAttribute)
        {
            return GetAttribute(GetAttributes(member, decoratingAttribute));
        }

        /// <summary>
        /// Given a test method and a decorative attribute of interest,
        /// retrieves a single Attribute instance.  Will throw an exception if
        /// multiple attributes exist on the method.
        /// </summary>
        /// <param name="method">ITestMethod instance.</param>
        /// <param name="decoratingAttribute">
        /// Attribute type of interest.
        /// </param>
        /// <returns>
        /// The attribute instance, or null if it does not exist.
        /// </returns>
        public static Attribute GetAttribute(ITestMethod method, Type decoratingAttribute)
        {
            return GetAttribute(method, decoratingAttribute, false);
        }

        /// <summary>
        /// Given a test method and a decorative attribute of interest,
        /// retrieves a single Attribute instance.  Will throw an exception if
        /// multiple attributes exist on the method.
        /// </summary>
        /// <param name="method">ITestMethod instance.</param>
        /// <param name="decoratingAttribute">
        /// Attribute type of interest.
        /// </param>
        /// <param name="inherit">A value indicating whether to look for custom 
        /// inherited attributes.</param>
        /// <returns>
        /// The attribute instance, or null if it does not exist.
        /// </returns>
        public static Attribute GetAttribute(ITestMethod method, Type decoratingAttribute, bool inherit)
        {
            return GetAttribute(GetAttributes(method, decoratingAttribute, inherit));
        }

        /// <summary>
        /// Returns a value indicating whether a method has the attribute.
        /// </summary>
        /// <param name="member">MemberInfo instance.</param>
        /// <param name="decoratingAttribute">Attribute of interest.</param>
        /// <returns>
        /// A value indicating whether the type has the decorating attribute or
        /// not.
        /// </returns>
        public static bool HasAttribute(MemberInfo member, Type decoratingAttribute)
        {
            return GetAttributes(member, decoratingAttribute).Count > 0;
        }

        /// <summary>
        /// Returns a value indicating whether a method has the attribute.
        /// </summary>
        /// <param name="method">ITestMethod instance.</param>
        /// <param name="decoratingAttribute">Attribute of interest.</param>
        /// <returns>
        /// A value indicating whether the type has the decorating attribute or
        /// not.
        /// </returns>
        public static bool HasAttribute(ITestMethod method, Type decoratingAttribute)
        {
            return GetAttributes(method, decoratingAttribute, false).Count > 0;
        }

        /// <summary>
        /// Return a collection of MethodInfo instances given a type to look
        /// through  and the attribute of interest.
        /// </summary>
        /// <param name="type">Type to look through for methods.</param>
        /// <param name="decoratingAttribute">Attribute of interest.</param>
        /// <returns>
        /// A collection of the method reflection objects, if any, with the
        /// marked attribute present.
        /// </returns>
        public static ICollection<MethodInfo> GetMethodsWithAttribute(Type type, Type decoratingAttribute)
        {
            if (type == null)
            {
                throw new ArgumentNullException("type");
            }

            ICollection<MethodInfo> methods = new List<MethodInfo>(2);
            foreach (MethodInfo method in type.GetMethods(/*BindingFlags.Public*/))
            {
                if (method == null)
                {
                    continue;
                }

                if (method.IsDefined(decoratingAttribute, true))
                {
                    methods.Add(method);
                }
            }
            return methods;
        }

        /// <summary>
        /// Retrieve a MethodInfo from a single decorated method inside a type,
        /// if any.  Throws an Exception if there are > 1 methods that are
        /// decorated with the attribute.
        /// </summary>
        /// <param name="type">Type of interest.</param>
        /// <param name="decoratingAttribute">Attribute of interest.</param>
        /// <returns>
        /// MethodInfo reflection object.  Null if none are found.
        /// </returns>
        public static MethodInfo GetOneMethodWithAttribute(Type type, Type decoratingAttribute)
        {
            ICollection<MethodInfo> methods = GetMethodsWithAttribute(type, decoratingAttribute);
            if (methods.Count > 1) 
            {
                // TODO: Exception message resource for n > 1 attributes
                throw new InvalidOperationException();
            }
            return methods.FirstOrDefault();
        }
    }
}