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
	}
}
