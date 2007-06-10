using System;

class Moonlight {
	static int count;

	//
	// This method should be invoked in the new AppDomain
	static void OnNewAppDomain ()
	{
		// 1. Load XAML file
		// 2. Make sure XAML file exposes a few new properites:
		//    a. Loaded  (this is the method to call)
		//    b. x:Class (it specifies the assembly to request from the server, and the class to load).
		// 3. Request the browser to download the files
		// 4. From the assembly, lookup the specified class
		// 5. From the class lookup the specified method
		// 6. Run the method
		// 7. If none of those properties are there, we can return
	}
	
	static void CreateInstance ()
	{
		// 1. Create a new AppDomain.
		// 2. Invoke a method in the new AppDomain to load the XAML file
	}
	
	static void Main ()
	{
		Console.WriteLine ("Running Moonlight.cs {0}", count++);
	}
	
}
