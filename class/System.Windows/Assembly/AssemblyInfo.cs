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

[assembly: AssemblyTitle ("System.Windows.dll")]
[assembly: AssemblyDescription ("System.Windows.dll")]
[assembly: AssemblyConfiguration ("Development version")]
[assembly: AssemblyCompany ("MONO development team")]
[assembly: AssemblyProduct ("MONO CLI")]
[assembly: AssemblyCopyright ("(c) 2007 Various Authors")]
[assembly: AssemblyTrademark ("")]

[assembly: AssemblyDefaultAlias ("System.Windows.dll")]
[assembly: AssemblyInformationalVersion ("0.0.0.1")]
[assembly: NeutralResourcesLanguage ("en-US")]

[assembly: ComVisible (false)]

[assembly: XmlnsDefinition("http://schemas.microsoft.com/client/2007", "System.Windows.Controls")]
[assembly: XmlnsDefinition("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Controls")]
[assembly: XmlnsDefinition("http://schemas.microsoft.com/client/2007", "System.Windows.Controls.Primitives")] 
[assembly: XmlnsDefinition("http://schemas.microsoft.com/winfx/2006/xaml/presentation", "System.Windows.Controls.Primitives")]

#if !NET_2_1
	[assembly: InternalsVisibleTo ("gtksilver, PublicKey=002400000480000094000000060200000024000052534131000400001100000005E62DA51722818A2ADC73D5CE64289260012A442031582E808F5C290EF155F10AB93441F92A7A59736D3481245ED4E0E864F5E1ACCADD217D53EE0263E6E3852FE94AB6B708984C6C69BA79F40A0896E1FFF820B7C55D4968C8F41CAE2AABC136B16B8AF83D013946CE190BC03C2A6C8DE8C0CB135ED656F46BF9A2D03E8188")]
#endif