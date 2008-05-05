//
// Deployment.cs
//
// Authors:
//   Miguel de Icaza (miguel@novell.com)
//
// Copyright 2008 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
using Mono;

namespace System.Windows {

	public sealed class Deployment : DependencyObject {

		static Deployment ()
		{
			AllowInboundCallsFromXDomainProperty = DependencyProperty.Lookup (Kind.DEPLOYMENT, "AllowInboundCallsFromXDomain", typeof (bool));
			EntryPointAssemblyProperty = DependencyProperty.Lookup (Kind.DEPLOYMENT, "EntryPointAssembly", typeof (string));
			EntryPointTypeProperty = DependencyProperty.Lookup (Kind.DEPLOYMENT, "EntryPointType", typeof (string));
			NeutralResourcesLanguageProperty = DependencyProperty.Lookup (Kind.DEPLOYMENT, "NeutralResourcesLanguage", typeof (string));
			PartsProperty = DependencyProperty.Lookup (Kind.DEPLOYMENT, "Parts", typeof (AssemblyPartCollection));;
			SupportedCulturesProperty = DependencyProperty.Lookup (Kind.DEPLOYMENT, "SupportedCultures", typeof (SupportedCultureCollection));
		}

		public Deployment () : base (NativeMethods.deployment_new ())
		{
		}

		internal Deployment (IntPtr raw) : base (raw)
		{
		}

		public bool AllowInboundCallsFromXDomain {
			get {
				return (bool) GetValue (AllowInboundCallsFromXDomainProperty);
			}

			set {
				SetValue (AllowInboundCallsFromXDomainProperty, value);
			}
		}

		public string EntryPointAssembly {
			get {
				return (string) GetValue (EntryPointAssemblyProperty);
			}

			set {
				SetValue (EntryPointAssemblyProperty, value);
			}
		}
		
		public string EntryPointType {
			get {
				return (string) GetValue (EntryPointTypeProperty);
			}

			set {
				SetValue (EntryPointTypeProperty, value);
			}
		}
		
		public string NeutralResourcesLanguage {
			get {
				return (string) GetValue (NeutralResourcesLanguageProperty);
			}

			set {
				SetValue (NeutralResourcesLanguageProperty, value);
			}
		}
			
		public AssemblyPartCollection Parts {
			get {
				return (AssemblyPartCollection) GetValue (PartsProperty);
			}

			set {
				SetValue (PartsProperty, value);
			}
		}
		
		public SupportedCultureCollection SupportedCultures {
			get {
				return (SupportedCultureCollection) GetValue (SupportedCulturesProperty);
			}

			set {
				SetValue (SupportedCulturesProperty, value);
			}
		}

		public static readonly DependencyProperty AllowInboundCallsFromXDomainProperty;
		public static readonly DependencyProperty EntryPointAssemblyProperty;
		public static readonly DependencyProperty EntryPointTypeProperty;
		public static readonly DependencyProperty NeutralResourcesLanguageProperty;
		public static readonly DependencyProperty PartsProperty;
		public static readonly DependencyProperty SupportedCulturesProperty;

		internal override Kind GetKind ()
		{
			return Kind.DEPLOYMENT;
		}
	}
}