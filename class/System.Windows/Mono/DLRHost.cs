//
// DLRHost.cs
//
// Authors:
//   Zoltan Varga (vargaz@gmail.com)
//
// Copyright 2007 Novell, Inc.
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
using System.IO;
using System.Text;
using System.Collections.Generic;
using System.Reflection;

using Microsoft.Scripting;
using Microsoft.Scripting.Hosting;
using IronPython.Hosting; 

namespace Mono
{
	//
	// This class hosts the MS Dynamic Language Runtime inside the Silverlight 
	// environment
	//
	internal class DLRHost {

		private SilverlightScriptHost host;
		private PythonEngine engine;
		private ScriptModule main_module;

		public DLRHost () {
			// From a posting on the IronPython list
			ScriptEnvironmentSetup setup = new ScriptEnvironmentSetup(true);
			setup.PALType = typeof(SilverlightPAL);
			setup.HostType = typeof (SilverlightScriptHost);
			IScriptEnvironment environment = ScriptEnvironment.Create(setup); 
			host = (SilverlightScriptHost)environment.Host;

			engine = PythonEngine.CurrentEngine;
            main_module = engine.CreateModule("__main__", ModuleOptions.PublishModule);

			string prefix = "";
			string executable = "";
			string version = engine.VersionString;

            engine.InitializeModules(prefix, executable, version);

			// Add references to default assemblies
			ClrModule clr_module = ClrModule.GetInstance ();

			clr_module.AddReference (typeof (int).Assembly,    // mscorlib
									 typeof (DLRHost).Assembly, // agclr
									 typeof (Uri).Assembly // System
									 );
			// System.Silverlight
            AssemblyName aname = new AssemblyName ();
			aname.Name = "System.SilverLight";
			clr_module.AddReference (Assembly.Load (aname));
			// System.Xml.Core
			aname.Name = "System.Xml.Core";
			clr_module.AddReference (Assembly.Load (aname));

			// The documentation doesn't say this, but Microsoft.Scripting is also loaded
			clr_module.AddReference (typeof (ScriptEnvironment).Assembly);
		}			

		//
		// Loads a source file from the given stream
		// Throws MissingReferenceException if the file imports another file which is not
		// yet loaded.
		// fileMappings maps source file names to downloaded local file names
		//
		public void LoadSource (Stream stream, String type, Dictionary<string, string> fileMappings) {
			if (type != "text/python")
				throw new Exception ("Currently only mime type text/python is supported.");

			SourceUnit source_unit = SourceUnit.Create (engine, new ContentProvider (stream), "", SourceCodeKind.File);

			host.FileMappings = fileMappings;
			// FIXME: If the script references another file which needs to be downloaded
			// then part of the file is executed before a MissingReferenceException is 
			// thrown. When the missing file is downloaded, and we are called again, that
			// part is executed again.
			engine.ExecuteSourceUnit (source_unit, main_module);
		}

		//
		// Set a global variable in the script environment
		//
		public void SetVariable (string name, object value) {
			main_module.SetVariable (name, value);
		}

		//
		// Hook up the event EVENT_NAME on TARGET to the script function HANDLER_NAME
		//
		public void HookupEvent (object target, string event_name, string handler_name) {
			object o;
			main_module.TryLookupVariable (handler_name, out o);
			if (o == null)
				throw new Exception ("No function named '" + handler_name + "' was found.");
				
			// FIXME: Is there a way to do this from C# code ?
			try {
				main_module.SetVariable ("__HookupEvent", target);
				engine.Execute ("__HookupEvent." + event_name + " += " + handler_name, main_module);
			}
			finally {
				main_module.RemoveVariable ("__HookupEvent");
			}
		}
	}

	class ContentProvider : SourceContentProvider {
		TextReader reader;

		public ContentProvider (Stream stream) {
			reader = new StreamReader (stream);
		}

		public ContentProvider (Stream stream, Encoding encoding) {
			reader = new StreamReader (stream, encoding, true);
		}

		public ContentProvider (TextReader reader) {
			this.reader = reader;
		}

		public override TextReader GetReader () {
			return reader;
		}
	}

	class SilverlightScriptHost : ScriptHost
	{
		Dictionary<string, string> fileMappings;

		public SilverlightScriptHost (ScriptEnvironment environment) : base (environment) {
		}

        public override SourceUnit TryGetSourceFileUnit(IScriptEngine engine, string path, Encoding encoding) {
			try {
				Stream s = Moonlight.LoadResource (path);
                ContentProvider provider = new ContentProvider (LanguageContext.FromEngine (engine).GetSourceReader (s, encoding));
				return SourceUnit.Create (engine, provider, "", SourceCodeKind.File);
			}
			catch {
				return null;
			}
		}

        public override SourceUnit ResolveSourceFileUnit(string name) {
			// FIXME: Other languages ?
			string extension = ".py";
			name += extension;

			if (fileMappings.ContainsKey (name)) {
				string local = fileMappings [name];

				Stream s = new FileStream (local, FileMode.Open, FileAccess.Read);
				LanguageProvider lang_provider;
				ScriptDomainManager.CurrentManager.TryGetLanguageProviderByFileExtension (extension, out lang_provider);
				return SourceUnit.Create (lang_provider.GetEngine (), new ContentProvider (s), "", SourceCodeKind.File);
			} else {
				throw new MissingReferenceException (name);
			}
		}

		public Dictionary<string, string> FileMappings {
			set {
				fileMappings = value;
			}
		}
	}
	
	class SilverlightPAL : PlatformAdaptationLayer
    {
        public override Assembly LoadAssembly(string name)
        {
            return Assembly.Load(LookupFullName(name));
        }

        private Dictionary<string, string> _assemblyFullNames = new Dictionary<string, string>();

        public SilverlightPAL()
        {
            LoadSilverlightAssemblyNameMapping();
        }

        // TODO: This will not be necessary as it will eventually move down into the host
        private void LoadSilverlightAssemblyNameMapping()
        {
            AssemblyName clrAssembly = new AssemblyName(typeof(object).Assembly.FullName);
            foreach (string asm in new string[] { "mscorlib", "System", "System.Core", "System.Xml.Core" })
            {
                clrAssembly.Name = asm;
                _assemblyFullNames.Add(asm.ToLower(), clrAssembly.FullName);
            }

            _assemblyFullNames.Add("system.silverlight", "System.SilverLight, Version=1.0.0.0, PublicKeyToken=b03f5f7f11d50a3a");
            _assemblyFullNames.Add("agclr", "agclr, Version=0.0.0.0, PublicKeyToken=b03f5f7f11d50a3a");
            _assemblyFullNames.Add("microsoft.visualbasic", "Microsoft.VisualBasic, Version=8.1.0.0, PublicKeyToken=b03f5f7f11d50a3a");

            AssemblyName dlrAssembly = new AssemblyName(typeof(PlatformAdaptationLayer).Assembly.FullName);
            foreach (string asm in new string[] {
                "Microsoft.Scripting",
                "Microsoft.Scripting.Silverlight",
                "IronPython",
                "IronPython.Modules",
                "Microsoft.JScript.Compiler",
                "Microsoft.JScript.Runtime",
                "Microsoft.VisualBasic.Compiler",
                "Microsoft.VisualBasic.Scripting",
                "Ruby"})
            {
                dlrAssembly.Name = asm;
                _assemblyFullNames.Add(asm.ToLower(), dlrAssembly.FullName);
            }
        }

        protected new string LookupFullName(string name)
        {
            AssemblyName asm = new AssemblyName(name);
            if (asm.Version != null || asm.GetPublicKeyToken() != null || asm.GetPublicKey() != null)
            {
                return name;
            }
            return _assemblyFullNames.ContainsKey(name.ToLower()) ? _assemblyFullNames[name.ToLower()] : name;
        }

    }

	class MissingReferenceException : Exception {
		string reference;

		public MissingReferenceException (string reference) : base () {
			this.reference = reference;
		}

		public string Reference {
			get {
				return reference;
			}
			set {
				reference = value;
			}
		}
	}
}


//
// FIXME:
// - in MS.NET, the x:Code directive should come before other elements
// - changes to the x:Name property need to be propagated to the DLR
//
