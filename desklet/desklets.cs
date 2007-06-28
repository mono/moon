//
// desklets.cs: basic functionality for moonlight desklets
//
// Authors:
//   Miguel de Icaza (miguel@novell.com)
//   Marek Habersack (mhabersack@novell.com)
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
using System.Windows;
using Gtk;
using GConf;

namespace Mono.Desklets {

	/// <summary>
	///   Provides an abstract base for configuration storage providers
	/// </summary>
	public abstract class ConfigStorage
	{
		string deskletName;
		int deskletInstance;
		string storageName;
		
		/// <summary>
		///   Desklet name
		/// </summary>
		/// <remarks>
		///   This is the value used to identify the desklet data in the config storage
		/// </remarks>
		public string DeskletName {
			get { return deskletName; }
		}

		/// <summary>
		///   Desklet instance number
		/// </summary>
		/// <remarks>
		///   As desklets can exist in several instances, each instance might need to have
		///   its own config storage data. This instance number, if different to -1, will
		///   be appended to the DeskletName in order to create a unique config storage
		///   designator.
		/// </remarks>
		public int DeskletInstance {
			get { return deskletInstance; }
		}

		protected string StorageName {
			get { return storageName; }
		}
				
		protected ConfigStorage () : this (null, -1)
		{}
		
		/// <summary>
		///   Construct new storage class using the specified desklet name
		/// </summary>
		/// <param name="deskletName">The desklet name</param>
		/// <remarks>
		///   You should make sure your desklet name is unique. It might be a good idea to
		///   use the common desklet name concatenated with a GUID (or UUID) value.
		/// </remarks>
		public ConfigStorage (string deskletName) : this (deskletName, -1)
		{}

		/// <summary>
		///   Construct new storage class using the specified desklet name and instance number.
		/// </summary>
		/// <param name="deskletName">The desklet name</param>
		/// <param name="deskletInstance">The desklet instance number</param>
		/// <remarks>
		///   If your desklet can exist in several instances, and each of them might have a
		///   different configuration, you should use different instance number for each copy
		///   of the desklet, thus separating their configuration storage.
		///
		///   You should make sure your desklet name is unique. It might be a good idea to
		///   use the common desklet name concatenated with a GUID (or UUID) value.
		/// </remarks>
		public ConfigStorage (string deskletName, int deskletInstance)
		{
			this.deskletName = deskletName;
			this.deskletInstance = deskletInstance;

			if (deskletInstance >= 0)
				storageName = String.Format ("{0}_{1}", deskletName, deskletInstance);
			else
				storageName = deskletName;
		}

		protected void CheckValid ()
		{
			if (String.IsNullOrEmpty (deskletName))
				throw new ApplicationException ("DeskletName must not be empty");
			if (String.IsNullOrEmpty (storageName))
				throw new ApplicationException ("No storage name specified");
		}
		
		/// <summary>
		///    Stores the named configuration item with the specified value.
		/// </summary>
		/// <param name="name">Item name</param>
		/// <param name="value">Item value</param>
		/// <remarks>
		///   This method will store the item in the desklet instance config
		///   storage area.
		/// </remarks>
		public abstract void Store (string name, object value);

		/// <summary>
		///    Stores the named configuration item with the specified value.
		/// </summary>
		/// <param name="name">Item name</param>
		/// <param name="value">Item value</param>
		/// <remarks>
		///   This method will store the item in the desklet common config
		///   storage area. The common area is named using only the desklet name
		///   without the instance number.
		/// </remarks>
		public abstract void StoreCommon (string name, object value);
		
		/// <summary>
		///    Retrieves the named configuration item from the storage media.
		/// </summary>
		/// <param name="name">Item name</param>
		/// <remarks>
		///    This method retrieves the item from the desklet instance config
		///    storage area.
		/// </remarks>
		/// <returns>
		///   The requested item's value or null if not found
		/// </returns>
		public abstract object Retrieve (string name);

		/// <summary>
		///    Retrieves the named configuration item from the storage media.
		/// </summary>
		/// <param name="name">Item name</param>
		/// <remarks>
		///    This method retrieves the item from the desklet common config
		///    storage area. The common area is named using only the desklet name
		///    without the instance number.
		/// </remarks>
		/// <returns>
		///   The requested item's value or null if not found
		/// </returns>
		public abstract object RetrieveCommon (string name);		
	}

	public class GConfConfigStorage : ConfigStorage {
		readonly string baseKeyPath = "/apps/mono/desklets";
		
		GConf.Client client;

		string keyPathCommon;
		string keyPathInstance;

		public string KeyPathCommon {
			get {
				if (keyPathCommon == null)
					keyPathCommon = String.Format ("{0}/{1}", baseKeyPath, DeskletName);
				
				return keyPathCommon;
			}
		}

		public string KeyPathInstance {
			get {
				if (keyPathInstance == null)
					if (DeskletInstance >= 0)
						keyPathInstance = String.Format ("{0}/{1}/{2}", baseKeyPath, DeskletName,
										 DeskletInstance);
					else
						keyPathInstance = KeyPathCommon;
				
				return keyPathInstance;
			}
		}

		public GConfConfigStorage (string deskletName) : this (deskletName, -1)
		{}

		public GConfConfigStorage (string deskletName, int deskletInstance)
			: base (deskletName, deskletInstance)
		{
			client = new GConf.Client ();
		}
		
		public override void Store (string name, object value)
		{
			CheckValid ();

			string key = String.Format ("{0}/{1}", KeyPathInstance, name);
			client.Set (key, value);
		}
		
		public override void StoreCommon (string name, object value)
		{
			string key = String.Format ("{0}/{1}", KeyPathCommon, name);
			client.Set (key, value);
			CheckValid ();
		}
		
		public override object Retrieve (string name)
		{
			CheckValid ();

			object ret = null;
			string key = String.Format ("{0}/{1}", KeyPathInstance, name);
			try {
				ret = client.Get (key);
			} catch (GConf.NoSuchKeyException) {
				ret = null;
			}
			
			return ret;
		}
		
		public override object RetrieveCommon (string name)
		{
			CheckValid ();

			object ret = null;
			string key = String.Format ("{0}/{1}", KeyPathCommon, name);
			try {
				ret = client.Get (key);
			} catch (GConf.NoSuchKeyException) {
				ret = null;
			}
			
			return ret;
		}
	}
	
	/// <summary>
	///   Provides utility functions for Desklet developers
	/// </summary>
        /// <remarks>
	/// </remarks>
	public class Desklet {

		/// <summary>
		///    Invokes the given EventHandler on the GUI thread.
		/// </summary>
		/// <param name="handler">The handler to invoke on the main thread</param>
		/// <remarks>
		///    This routine should be used when your application needs
		///    to invoke methods or access properties and events from Moonlight
		///    or Gtk# from a thread that is not the main thread. 
		/// </remarks>
		public static void Invoke (EventHandler handler)
		{
			Application.Invoke (handler);
		}

		/// <summary>
		///    Invokes the given EventHandler on the GUI thread.
		/// </summary>
		/// <param name="sender">The sender parameter to pass to the handler</param>
		/// <param name="args">Arguments to pass to the handler</param>
		/// <param name="handler">The handler to invoke on the main thread</param>
		/// <remarks>
		///    This routine should be used when your application needs
		///    to invoke methods or access properties and events from Moonlight
		///    or Gtk# from a thread that is not the main thread. 
		/// </remarks>
                public static void Invoke (object sender, EventArgs args, EventHandler handler)
		{
			Application.Invoke (sender, args, handler);
		}

		static void SetupOnClick (UIElement element, EventHandler handler)
		{
			bool down = false, inside = false;
			
			element.MouseLeftButtonDown += delegate {
				down = true;
			};

			element.MouseEnter += delegate {
				inside = true;
			};

			element.MouseLeave += delegate {
				inside = false;
			};

			element.MouseLeftButtonUp += delegate {
				if (inside && down){
					handler (element, EventArgs.Empty);
				}
				down = false;
			};
		}
		
		/// <summary>
		///   Configures various elements found on the given UIElement
		///   to become control points. 
		/// </summary>
		/// <param name="root">
		///    The UIElement that might contain the specially named objects to
		///    be hooked up.
		/// </param>
		/// <remarks>
		///   This looks up elements named "desklet-close" and "desklet-drag"
		///   elements from the given root element and hooks up mouse events
		///   so that they trigger a close action and a drag action on the
		///   desklet.
		/// </remarks>
		public static void SetupToolbox (UIElement root)
		{
			UIElement close = root.FindName ("desklet-close") as UIElement;
			if (close != null){
				SetupOnClick (close, delegate {

					// When we move to multiple desklets on an
					// appdomain we will need to change this to
					// use some sort of global reference count and
					// only shut Gtk when the count reaches zero.
					Application.Quit ();
				});
			}

			UIElement drag = root.FindName ("desklet-drag") as UIElement;
			if (drag != null){
				//
				// TODO: we need to pass the Gtk.Window, sort out
				// a way of getting this information from mopen or
				// from the surface
				//
				SetupOnClick (drag, delegate {
					Console.WriteLine ("On drag not enabled, as we do not know our Gtk.Window yet");
				} );
			}
		}
	}
}
