// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Reflection;
using System.Resources;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Windows.Markup;

// General Information about an assembly is controlled through the following 
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
[assembly: AssemblyTitle("System.Windows.Controls")]
[assembly: AssemblyDescription("")]
[assembly: AssemblyConfiguration("")]
[assembly: AssemblyCompany("Microsoft")]
[assembly: AssemblyProduct("System.Windows.Controls")]
[assembly: AssemblyCopyright("Copyright © Microsoft 2007")]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]
[assembly: NeutralResourcesLanguage("en-us")]

[assembly: XmlnsPrefix("clr-namespace:System.Windows.Controls;assembly=System.Windows.Controls", "basics")]
[assembly: XmlnsDefinitionAttribute("clr-namespace:System.Windows.Controls;assembly=System.Windows.Controls", "System.Windows.Controls")]

// 
[assembly: CLSCompliant(false)]

// Setting ComVisible to false makes the types in this assembly not visible 
// to COM components.  If you need to access a type in this assembly from 
// COM, set the ComVisible attribute to true on that type.
[assembly: ComVisible(false)]

// The following GUID is for the ID of the typelib if this project is exposed to COM
[assembly: Guid("3d5900ae-111a-45be-96b3-d9e4606ca793")]

// Provide unit tests with access to internal members
// Provide unit tests with access to internal members
#if DEBUG
[assembly: InternalsVisibleTo("ControlsExtended.Test, PublicKey=00240000048000009400000006020000002400005253413100040000010001000fed916f5fe67a46ed73e4151623c8572c7ffdd000cda61e0142afdf721bbd11b7d77367a6a9a5f20e1a930b1f480d0499280e63ab23e052692cdd117da8158ebe924d6b91d05a64a1b96fd87f3259e9223dcff8fe5915540e5386982227310de2a3cb8f78e78062bfe4012fac307a507f5a89d76d19c9845cf62fa6693d7cde")]
[assembly: InternalsVisibleTo("Controls.Extended.TestHooks, PublicKey=00240000048000009400000006020000002400005253413100040000010001000fed916f5fe67a46ed73e4151623c8572c7ffdd000cda61e0142afdf721bbd11b7d77367a6a9a5f20e1a930b1f480d0499280e63ab23e052692cdd117da8158ebe924d6b91d05a64a1b96fd87f3259e9223dcff8fe5915540e5386982227310de2a3cb8f78e78062bfe4012fac307a507f5a89d76d19c9845cf62fa6693d7cde")]
#else
[assembly: InternalsVisibleTo("ControlsExtended.Test, PublicKey=00240000048000009400000006020000002400005253413100040000010001000fed916f5fe67a46ed73e4151623c8572c7ffdd000cda61e0142afdf721bbd11b7d77367a6a9a5f20e1a930b1f480d0499280e63ab23e052692cdd117da8158ebe924d6b91d05a64a1b96fd87f3259e9223dcff8fe5915540e5386982227310de2a3cb8f78e78062bfe4012fac307a507f5a89d76d19c9845cf62fa6693d7cde")]
[assembly: InternalsVisibleTo("Controls.Extended.TestHooks, PublicKey=00240000048000009400000006020000002400005253413100040000010001000fed916f5fe67a46ed73e4151623c8572c7ffdd000cda61e0142afdf721bbd11b7d77367a6a9a5f20e1a930b1f480d0499280e63ab23e052692cdd117da8158ebe924d6b91d05a64a1b96fd87f3259e9223dcff8fe5915540e5386982227310de2a3cb8f78e78062bfe4012fac307a507f5a89d76d19c9845cf62fa6693d7cde")]
#endif

// Version information for an assembly consists of the following four values:
//
//      Major Version
//      Minor Version 
//      Build Number
//      Revision
//
// You can specify all the values or you can default the Revision and Build Numbers 
// by using the '*' as shown below:
[assembly: AssemblyVersion("2.0.5.0")]
[assembly: AssemblyFileVersion("1.0.0.0")]
