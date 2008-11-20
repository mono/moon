/*
 * TestContext.cs.
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
using System.Collections;
using System.Data;
using System.Data.Common;

namespace Microsoft.VisualStudio.TestTools.UnitTesting
{
	public abstract class TestContext {
		protected TestContext ()
		{
		}

		public abstract void AddResultFile (string filename);
		public abstract void BeginTimer (string timerName);
		public abstract void EndTimer (string timerName);

		public abstract DbConnection DataConnection { get; }
		public abstract DataRow DataRow { get; }
		public abstract IDictionary Properties { get; }

		public virtual UnitTestOutcome CurrentTestOutcome { 
			get { throw new NotImplementedException (); }
		}
		
		public abstract void WriteLine (string format, params object [] args);
		
		public virtual string TestDeploymentDir {
			get { throw new NotImplementedException (); }
		}

		public virtual string TestDir {
			get { throw new NotImplementedException (); }
		}

		public virtual string TestLogsDir {
			get { throw new NotImplementedException (); }
		}

		public virtual string TestName {
			get { throw new NotImplementedException (); }
		}
	}
}