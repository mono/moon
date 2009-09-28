using System;
using Mono.Unix;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Globalization;
using System.Diagnostics;

namespace moonvisi
{
	class main
	{

		class Frame {
			public int ptr;
			public string name;
			public int type;
		}


		class action {
			public int ptr;
			public string type;
			public string name;
			public int refcount;
			public List<Frame> frames;

			public override string ToString () {
				string ret = type + ":" + name + ":" + refcount;
				return ret;
			}
		}

		class obj {
			public string type;
			public int ptr;
			public int refcount;
			public bool leaked;
			public List<action> actions;

			
			public obj (int p) {
				actions = new List<action>();
				ptr = p;
			}
			
			public override string ToString () {
				string ret = String.Format ("0x{0:x}\t" + type + "\t" + refcount, ptr);
				return ret;
			}
		}

		class leak {
			public int ptr;
			public string name;
		}
		
		Dictionary<int, obj> objects;
		Dictionary<int, leak> leaks;
		StreamReader sr;

		public static void Main(string[] args)
		{
			if (args.Length == 0 || args.Length > 2) {
				Console.WriteLine ("Usage: moonvisi logfile [output path]");
				return;
			}
			
			string file = args[0];
			string path = "";
			if (args.Length == 2)
				path = args[1];
			else {
				path = System.IO.Path.GetRandomFileName ();
				path = path.Substring (0, path.IndexOf (".") - 1);
				Directory.CreateDirectory (path);
			}
			Console.WriteLine ("Using output path: ./" + path);
			new main ().run(file, path);
		}

		public void run (string file, string path)
		{
			parse (file, path);
			show (path);
		}

		void show (string path) {
			foreach (KeyValuePair<int, leak> l in leaks) {
				int ptr = l.Key;
				if (!objects.ContainsKey (ptr) || !objects[ptr].leaked)
					continue;
				ProcessStartInfo oInfo = new ProcessStartInfo("dot", String.Format ("-Tplain " + path + "/" + "0x{0:x}_raw.dot", ptr)); 
				oInfo.UseShellExecute = false;
				oInfo.ErrorDialog = false; 
				oInfo.CreateNoWindow = true; 
				oInfo.RedirectStandardOutput = true; 				

				Process p = Process.Start (oInfo);
				string ret = p.StandardOutput.ReadToEnd ();
				p.Close ();
				Mono.Unix.StdioFileStream sfs = new Mono.Unix.StdioFileStream (String.Format (path + "/" + "0x{0:x}.plain", ptr), 
				                                                               FileMode.OpenOrCreate, FileAccess.Write);
				StreamWriter sw = new StreamWriter (sfs);
				sw.Write (ret);
				sw.Close ();
			}
		}
		
		void parse (string file, string path) {
			objects = new Dictionary<int, obj> ();
			leaks = new Dictionary<int, leak> ();

			Mono.Unix.StdioFileStream sfs = new Mono.Unix.StdioFileStream (file, FileMode.Open, FileAccess.Read);

			sr = new StreamReader(sfs);
			string s;
			int count = 0;
			bool parsingLeaks = false;
			while (!sr.EndOfStream) {
				count++;
				s = sr.ReadLine ().Trim();
				if (s.StartsWith ("Deployment leak report:"))
					parsingLeaks = true;
				if (s.StartsWith ("End of leak report"))
					break;
				if (parsingLeaks)
					parseReport (s);
				else
					parseline (s);
				
			}
			sfs.Close ();

			if (!parsingLeaks) {
				foreach (KeyValuePair<int, obj> o in objects) {
					leaks.Add (o.Value.ptr, new leak {ptr = o.Value.ptr, name = o.Value.type});
					o.Value.leaked = true;
				}
			} else {
				foreach (KeyValuePair<int, obj> o in objects) {
					if (leaks.ContainsKey (o.Key))
						o.Value.leaked = true;
				}
			}
			
			StreamWriter sw;
			foreach (KeyValuePair<int, obj> list in objects) {
				obj o = list.Value;
				if (parsingLeaks && !o.leaked)
					continue;

				sfs = new Mono.Unix.StdioFileStream (String.Format (path + "/" + "0x{0:x}_raw.dot", o.ptr), 
				                                     FileMode.OpenOrCreate, FileAccess.Write);
				sw = new StreamWriter (sfs);
				
				sw.WriteLine ("digraph " + o.type + " {");
				int cc = 0;
				
				foreach (action act in o.actions) {
					sw.WriteLine ("subgraph " + act.name + "_" + cc++ + " {");
					int c = act.frames.Count;
					for (int i = 0; i < act.frames.Count; i++) {
						Frame frame = act.frames[i];
						if (i == 0)
							sw.WriteLine (act.name + " [shape=box,style=bold,label=\"" + act.name + String.Format (" {0:x}", act.ptr) + "\"];");
						else
							sw.WriteLine (frame.name + "_{0:x} [label=\"{0:x}\\n" + frame.name + "\"];", frame.ptr);
					}
					for (int i = 0; i < act.frames.Count; i++) {
						Frame frame = act.frames[i];
						if (i == 0)
							sw.Write (act.name + "->");
						if (i > 0)
							sw.Write ("->");
						sw.Write (frame.name + "_{0:x}", frame.ptr);
						if (i == 0)
							sw.Write (" [label=\"ref=" + act.refcount  +"\"]");
						if (i < c - 1)
							sw.Write (";" + frame.name + "_{0:x}", frame.ptr);
					}
					sw.WriteLine ("\n}");
				}
				sw.WriteLine ("\n}");
				sw.Close ();
			}

		}

		void parseReport (string s)
		{
			if (s.StartsWith ("0x")) {
				string[] str = s.Split (' ');
				int ptr = int.Parse (str[0].Substring(2), NumberStyles.AllowHexSpecifier);
				leaks.Add (ptr, new leak {ptr = ptr, name = str[3].Replace (',',' ').Trim().Replace ('.', '_').Replace ('+','_')});
				
			}
		}

		void parseline (string s) {
			if (!s.StartsWith ("trace:"))
				return;

			s = s.Replace ("~", "dtor_");
			
			List<Frame> tmpstack = new List<Frame>();
			string[] act = s.Substring (6, s.IndexOf (";") - 6).Split ('|');
			string[] frames = s.Substring(s.IndexOf (";")+1).Split (';');
			int ptr = 0;
			string prev = "";
			
			action a = new action ();
			a.name = act[0];
			a.type = act[1];
			a.refcount = int.Parse (act[2]);

			foreach (string frame in frames) {
				string[] parts = frame.Split ('|');
				if (parts.Length != 3)
					break;
				if (prev == parts[2])
					continue;
				Frame f = new Frame ();
				if (!int.TryParse (parts[0], out f.type)) {
					Console.WriteLine (frame);
					continue;
				}
				f.ptr = int.Parse (parts[1], NumberStyles.AllowHexSpecifier);
				if (ptr == 0)
					ptr = f.ptr;
				if (f.type == 1)
					if (parts[2].IndexOf("_ref") > 0 || parts[2].IndexOf("_unref") > 0)
						f.name = parts[2];
					else
						f.name = parts[2];//.Substring (0, parts[2].IndexOf ("_"));
				else
					f.name = parts[2];

				if (f.name.IndexOf ("Track") > 0)
					continue;
				f.name = Regex.Replace (f.name, "[:(),.-/&`]", "_");
				f.name = f.name.Replace (" ", "_");
				f.name = f.name.Replace ("-", "_");
				
				prev = f.name;				
				tmpstack.Add (f);
			}
			a.frames = tmpstack;
			a.ptr = ptr;
			if (!objects.ContainsKey (ptr)) {
				objects[ptr] = new obj (ptr);
			} else if (objects[ptr].type == null && a.type != "")
				objects[ptr].type = a.type;

			objects[ptr].actions.Add (a);
		}
	}
}