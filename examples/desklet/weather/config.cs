using System;

	/// <summary>
	///   Provides an abstract base for configuration storage providers
	/// </summary>
	public abstract class ConfigStorage
	{
		string deskletName;
		int deskletInstance;
		
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
				
		protected ConfigStorage () : this (null, -1)
		{}
		
		/// <summary>
		///   Construct new storage object using the specified desklet name
		/// </summary>
		/// <param name="deskletName">The desklet name</param>
		/// <remarks>
		///   You should make sure your desklet name is unique. It might be a good idea to
		///   use the common desklet name concatenated with a GUID (or UUID) value.
		/// </remarks>
		public ConfigStorage (string deskletName) : this (deskletName, -1)
		{}

		/// <summary>
		///   Construct new storage object using the specified desklet name and instance number.
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
		}

		/// <summary>
		///   Check if the internal state of the storage object is valid
		/// </summary>
		/// <remarks>
		///   This method should be called at the start of every public method, to ensure
		///   that the object state is correct.
		/// </remarks>
		protected virtual void CheckValid ()
		{
			if (String.IsNullOrEmpty (deskletName))
				throw new ApplicationException ("DeskletName must not be empty");
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

	/// <summary>
	///   Config store implementation using the GNOME GConf backend
	/// </summary>
	public class GConfConfigStorage : ConfigStorage {
		readonly string baseKeyPath = "/apps/mono/desklets";
		
		GConf.Client client;

		string keyPathCommon;
		string keyPathInstance;

		/// <summary>
		///   GConf path to the common desklet storage area. This configuration area is shared
		///   among all the instances of the desklet.
		/// </summary>
		public string KeyPathCommon {
			get {
				if (keyPathCommon == null)
					keyPathCommon = String.Format ("{0}/{1}", baseKeyPath, DeskletName);
				
				return keyPathCommon;
			}
		}

		/// <summary>
		///   GConf path to the instance desklet storage area. This configuration area is private
		///   for each desklet instance.
		/// </summary>
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

		/// <summary>
		///   Construct new GConf storage object using the specified desklet name
		/// </summary>
		/// <param name="deskletName">The desklet name</param>
		/// <remarks>
		///   You should make sure your desklet name is unique. It might be a good idea to
		///   use the common desklet name concatenated with a GUID (or UUID) value.
		/// </remarks>
		public GConfConfigStorage (string deskletName) : this (deskletName, -1)
		{}

		/// <summary>
		///   Construct new GConf storage object using the specified desklet name and instance number.
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
		public GConfConfigStorage (string deskletName, int deskletInstance)
			: base (deskletName, deskletInstance)
		{
			client = new GConf.Client ();
		}

		/// <summary>
		///    Stores the named configuration item with the specified value.
		/// </summary>
		/// <param name="name">Item name</param>
		/// <param name="value">Item value</param>
		/// <remarks>
		///   This method will store the item in the desklet instance config
		///   storage area. GConf path /apps/mono/desklets/DESKLET_NAME/DESKLET_INSTANCE_NUMBER is
		///   used as the storage base.
		/// </remarks>
		public override void Store (string name, object value)
		{
			CheckValid ();

			string key = String.Format ("{0}/{1}", KeyPathInstance, name);
			client.Set (key, value);
		}

		/// <summary>
		///    Stores the named configuration item with the specified value.
		/// </summary>
		/// <param name="name">Item name</param>
		/// <param name="value">Item value</param>
		/// <remarks>
		///   This method will store the item in the desklet common config
		///   storage area. GConf path /apps/mono/desklets/DESKLET_NAME/ is
		///   used as the storage base.
		/// </remarks>
		public override void StoreCommon (string name, object value)
		{
			string key = String.Format ("{0}/{1}", KeyPathCommon, name);
			client.Set (key, value);
			CheckValid ();
		}

		/// <summary>
		///    Retrieves the named configuration item from the storage media.
		/// </summary>
		/// <param name="name">Item name</param>
		/// <remarks>
		///    This method retrieves the item from the desklet instance config
		///    storage area. GConf path /apps/mono/desklets/DESKLET_NAME/DESKLET_INSTANCE_NUMBER is
		///    used as the storage base.
		/// </remarks>
		/// <returns>
		///   The requested item's value or null if not found
		/// </returns>
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

		/// <summary>
		///    Retrieves the named configuration item from the storage media.
		/// </summary>
		/// <param name="name">Item name</param>
		/// <remarks>
		///    This method retrieves the item from the desklet common config
		///    storage area. GConf path /apps/mono/desklets/DESKLET_NAME/ is
		///   used as the storage base.
		/// </remarks>
		/// <returns>
		///   The requested item's value or null if not found
		/// </returns>
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
	
