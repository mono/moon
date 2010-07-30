//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 20010 Novell, Inc (http://www.novell.com)
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

using System.Reflection;
using System.IO;
using System.Windows.Resources;
using System.Runtime.Serialization.Json;
using System.Text;

namespace System.Windows.Browser {

	internal class HostServices {

		ScriptObject jsonSerializer;

		[ScriptableMember(ScriptAlias="createObject")]
		public ScriptObject CreateObject (string name)
		{
			if (!HtmlPage.ScriptableTypes.ContainsKey (name))
				return null;

			object o = Activator.CreateInstance (HtmlPage.ScriptableTypes[name]);

			return new ManagedObject (o);
		}

		[ScriptableMember(ScriptAlias="createObject")]
		public ScriptObject CreateObject (string name, object obj)
		{
			if (!HtmlPage.ScriptableTypes.ContainsKey (name))
				return null;

			if (obj is double || obj is int) {
				int size = int.Parse (obj.ToString());
				Type t = HtmlPage.ScriptableTypes[name].GetElementType ();
				if (t == null)
					return null;
				return new ManagedObject (Array.CreateInstance (t, size));
			}
			return new ManagedObject (JsonDeserialize (obj, HtmlPage.ScriptableTypes[name]));
		}

		[ScriptableMember(ScriptAlias="jsonSerialize")]
		public string JsonSerialize (ScriptObject obj)
		{
			if (obj is HtmlObject)
				return "{}";

			if (obj.ManagedObject != null)
			{
				DataContractJsonSerializer serializer = new DataContractJsonSerializer(obj.ManagedObject.GetType());
				using (MemoryStream ms = new MemoryStream()) {
					serializer.WriteObject(ms, obj.ManagedObject);
					ms.Position = 0L;
					using (StreamReader reader = new StreamReader(ms)) {
						return reader.ReadToEnd();
					}
				}
			}

			return (string) JsonSerializer.Invoke("serialize", new object[] { obj });
		}

		[ScriptableMember(ScriptAlias="requiresManagedSerializer")]
		public bool RequiresManagedSerializer (ScriptObject obj)
		{
			if (obj.ManagedObject == null)
				return (obj is HtmlObject);
			return true;
		}

		public override string ToString ()
		{
			return "[ManagedServices]";
		}


		string JsonSerialize (object obj)
		{
			ScriptObject o = obj as ScriptObject;
			if (o != null)
				return JsonSerialize (o);
			return (string) JsonSerializer.Invoke("serialize", new object[] { obj });
		}

		public object JsonDeserialize (object obj, Type type)
		{
			string jsgraph = JsonSerialize (obj);
			DataContractJsonSerializer deserializer = new DataContractJsonSerializer (type);
			using (MemoryStream ms = new MemoryStream(Encoding.UTF8.GetBytes (jsgraph))) {
				return deserializer.ReadObject (ms);
			}
		}

		static HostServices services = new HostServices ();

		public static HostServices Services {
			get { return services; }
		}


		ScriptObject JsonSerializer {
			get {
				if (jsonSerializer == null) {
					Deployment.RegisterAssembly (this.GetType().Assembly);
					StreamResourceInfo info = Application.GetResourceStream (new Uri ("/System.Windows.Browser;component/json.js", UriKind.Relative));

					string json;
					using (StreamReader sr = new StreamReader (info.Stream)) {
						json = sr.ReadToEnd();
					}
					jsonSerializer = (ScriptObject) HtmlPage.Window.Eval (json);
					jsonSerializer.SetProperty("_managedServices", this);
				}
				return jsonSerializer;
			}
		}
	}

}