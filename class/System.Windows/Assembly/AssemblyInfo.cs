//
// AssemblyInfo.cs
//
// Author:
//   Andreas Nahr (ClassDevelopment@A-SoftTech.com)
//
// (C) 2003 Ximian, Inc.  http://www.ximian.com
//

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

using System;
using System.Diagnostics;
using System.Reflection;
using System.Resources;
using System.Security;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Windows.Markup;

// General Information about the System.Windows assembly

#if !NET_2_1
[assembly: AssemblyVersion (Consts.FxVersion)]
[assembly: SatelliteContractVersion (Consts.FxVersion)]
#else
[assembly: AssemblyVersion ("2.0.5.0")]
[assembly: SatelliteContractVersion ("2.0.5.0")]
#endif

//[assembly: AssemblyDelaySign(true)]
[assembly: AssemblyFileVersion ("2.0.31005.0")]
[assembly: AssemblyTitle ("System.Windows.dll")]
[assembly: AssemblyDescription ("System.Windows.dll")]
[assembly: AssemblyCompany ("MONO development team")]
[assembly: AssemblyProduct ("MONO CLI")]
[assembly: AssemblyCopyright ("(c) 2007 - 2008 Various Authors")]

[assembly: AssemblyDefaultAlias ("System.Windows.dll")]
[assembly: AssemblyInformationalVersion ("0.0.0.1")]
[assembly: NeutralResourcesLanguage ("en-US")]

[assembly: Debuggable (DebuggableAttribute.DebuggingModes.IgnoreSymbolStoreSequencePoints)]
//[assembly: CLSCompliant (true)]
[assembly: ComVisible (false)]
[assembly: CompilationRelaxations (CompilationRelaxations.NoStringInterning)]

[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Controls")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Controls.Primitives")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Data")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Documents")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Ink")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Input")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Markup")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Media")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Media.Animation")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Media.Imaging")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Shapes")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/client/2007", "System.Windows.Automation")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Controls")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Controls.Primitives")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Data")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Documents")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Ink")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Input")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Markup")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Media")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Media.Animation")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Media.Imaging")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Shapes")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Automation")]
[assembly: XmlnsDefinition ("http://schemas.microsoft.com/winfx/2006/xaml", "System.Windows.Markup")]
[assembly: XmlnsPrefix ("http://schemas.microsoft.com/winfx/2006/xaml", "x")]
[assembly: XmlnsPrefix ("http://schemas.microsoft.com/client/2007", "x")]

#if NET_2_1
[assembly: InternalsVisibleTo ("System.Windows.Browser, PublicKey=00240000048000009400000006020000002400005253413100040000010001008D56C76F9E8649383049F383C44BE0EC204181822A6C31CF5EB7EF486944D032188EA1D3920763712CCB12D75FB77E9811149E6148E5D32FBAAB37611C1878DDC19E20EF135D0CB2CFF2BFEC3D115810C3D9069638FE4BE215DBF795861920E5AB6F7DB2E2CEEF136AC23D5DD2BF031700AEC232F6C6B1C785B4305C123B37AB")]
[assembly: InternalsVisibleTo ("MoonAtkBridge, PublicKey=00240000048000009400000006020000002400005253413100040000110000004bb98b1af6c1df0df8c02c380e116b7a7f0c8c827aecfccddc6e29b7c754cd608b49dfcef4df9699ad182e50f66afa4e68dabc7b6aeeec0aa4719a5f8e0aae8c193080a706adc3443a8356b1f254142034995532ac176398e12a30f6a74a119a89ac47672c9ae24d7e90de686557166e3b873cd707884431a0451d9d6f7fe795")]
#else
// the 3.0 (desktop) case
[assembly: InternalsVisibleTo ("System.Windows.Browser, PublicKey=002400000480000094000000060200000024000052534131000400000100010079159977d2d03a8e6bea7a2e74e8d1afcc93e8851974952bb480a12c9134474d04062447c37e0e68c080536fcf3c3fbe2ff9c979ce998475e506e8ce82dd5b0f350dc10e93bf2eeecf874b24770c5081dbea7447fddafa277b22de47d6ffea449674a4f9fccf84d15069089380284dbdd35f46cdff12a1bd78e4ef0065d016df")]
[assembly: InternalsVisibleTo ("Moonlight.Gtk, PublicKey=002400000480000094000000060200000024000052534131000400001100000005E62DA51722818A2ADC73D5CE64289260012A442031582E808F5C290EF155F10AB93441F92A7A59736D3481245ED4E0E864F5E1ACCADD217D53EE0263E6E3852FE94AB6B708984C6C69BA79F40A0896E1FFF820B7C55D4968C8F41CAE2AABC136B16B8AF83D013946CE190BC03C2A6C8DE8C0CB135ED656F46BF9A2D03E8188")]
#endif
