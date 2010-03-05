/*
 * LicenseAcquirer.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.IO;
using System.Windows.Controls;

namespace System.Windows.Media
{
	public class LicenseAcquirer
	{
		Uri license_server_uri_override;
		public LicenseAcquirer ()
		{
		}

		public Uri LicenseServerUriOverride {
			get { return license_server_uri_override; }
			set { license_server_uri_override = value; }
		}

		protected internal virtual void OnAcquireLicense (Stream licenseChallenge, Uri licenseServerUri)
		{
			throw new NotImplementedException ();
		}

		protected void SetLicenseResponse (Stream licenseResponse)
		{
			throw new NotImplementedException ();
		}

		public void AcquireLicenseAsync (Guid keyId, ContentKeyType keyType, Guid serviceId)
		{
			Console.WriteLine ("System.Windows.Media.LicenseAcquirer.AcquireLicenseAsync: NIEX");
			throw new NotImplementedException ();
		}

		public void AcquireLicenseAsync (Stream mediaStream)
		{
			Console.WriteLine ("System.Windows.Media.LicenseAcquirer.AcquireLicenseAsync: NIEX");
			throw new NotImplementedException ();
		}

		public void AcquireLicenseAsyncCancel ()
		{
			Console.WriteLine ("System.Windows.Media.LicenseAcquirer.AcquireLicenseAsyncCancel: NIEX");
			throw new NotImplementedException ();
		}

		protected virtual void OnCancel ()
		{
			Console.WriteLine ("System.Windows.Media.LicenseAcquirer.OnCancel: NIEX");
			throw new NotImplementedException ();
		}

		public string CustomData {
			get {
				Console.WriteLine ("System.Windows.Media.LicenseAcquirer.get_CustomData: NIEX");
				throw new NotImplementedException ();
			}
			set {
				Console.WriteLine ("System.Windows.Media.LicenseAcquirer.set_CustomData: NIEX");
				throw new NotImplementedException ();
			}
		}

		public DomainAcquirer DomainAcquirer {
			get {
				Console.WriteLine ("System.Windows.Media.LicenseAcquirer.get_DomainAcquirer: NIEX");
				throw new NotImplementedException ();
			}
			set {
				Console.WriteLine ("System.Windows.Media.LicenseAcquirer.set_DomainAcquirer: NIEX");
				throw new NotImplementedException ();
			}
		}

		public event EventHandler <AcquireLicenseCompletedEventArgs> AcquireLicenseCompleted;
	}
}
