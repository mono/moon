Layout
------

### NB: Some of the changes are to accomodate an Out-of-Memory condition that exists due to known memory leaks.  See the
###		notes regarding the TestClasses folder.  Once the leaks are fixed, this may be adjusted for time, rather than memory.

In an attempt to maximize the possible conditions we can test against, the unit tests have been refactored to allow us to
combine sequence types with element types and cardinalities, and to run them against a common set of unit tests as appropriate.
The new files have been organized as follows, and may be adjusted pending further work on the testing infrastructure itself.

DataClasses\
	: the types of individual items in the data source.  For example, Customer.
	
	- In general, it's preferable to add properties to DataTypes or Customer, without creating anything new.

DataClassSources\
	: the types of the data sources.  For example, zero-, one-, and many-item sequences 
	: (IList, IEnumerable, etc) of the DataClass types.  These sources allow us to emphasize
	: the interfaces of collections, rather than silently depend on, for example, List<T> when we
	: really want to test IList<T> (and where we may provide shortcuts internally for users of
	: List<T>).
	
	- Like data types, data sources multiply the number of tests greatly, so avoid creating a new data source if it's 
		sufficient to adjust an existing one without invalidating anything.  New sources should derive from 
		DataClassSource<TDataClass> (or GenericDataClassSource<TDataClass>) where TDataClass: new(), call base(n) in 
		their constructor, and use the inherited Items property to fetch a randomly-generated sequence (constant for 
		the lifetime of the data source instance) of TDataClass objects.
	
		For example, a cut-down version of a non-generic list looks like:
		
		    public abstract class NonGenericList<TDataClass> : DataClassSource<TDataClass>, IList
			where TDataClass : new()
			{
				protected NonGenericList(int count)
					: base(count)
				{
				}

				#region IList Members

				public virtual int Add(object value)
				{
					this.Items.Add(value);

					return this.Items.IndexOf(value);
				}
				
				// ...

				#endregion

				#region ICollection Members

				public void CopyTo(Array array, int index)
				{
					// ### NOTE LINQ Cast<T> OPERATOR
					
					this.Items.CopyTo(array.Cast<object>().ToArray(), index);
				}

				// ...
				
				private object syncRoot = new object();

				public object SyncRoot
				{
					get { return this.syncRoot; }
				}

				#endregion

				#region IEnumerable Members

				public IEnumerator GetEnumerator()
				{
					return this.Items.GetEnumerator();
				}

				#endregion
			}
			
TestCases\
	: where the code for unit tests lives, currently partitioned according to test requirements which may change 
	: based on rewriting tests to remove restrictions
	
	- While the existing files are 1:1 with their partitions, it seems reasonable to split them further into functional
		groups (eg, Editting, etc) as we have the opportunity.
		
		The current set of partitions are:
		
		DataGridTests_Unrestricted:  reserved for tests that do not depend on data items.  This class is run for 
		all data sources.
		
		DataGridTests_Generic: reserved for tests which require generic data sources of any length.  
		Likely to go away after investigation.  Most tests should not depend on IList<T>, when 
		IList (or even IEnumerable) would be sufficient.
		
		DataGridTests_GT0: reserved for tests which require more than zero items in the data source.  Don't gratuitously
		add an item to a list just to be able to move it to Unrestricted.  Consider that tests here are tests that we
		don't have to run for empty data sources (1/3 of the potential test cases), which reduces testing time.
		
		DataGridTests_GT1: reserved for tests which require more than one item in the data source.  Same caveats as GT0.
	
TestClasses\
	: the [TestClass] implementations that encode combinations of the above.  Place overrides, 
	: if necessary, here.  Prefer not to override (ask why the override should exist).
	: You can also tweak the set of test classes that run.
	
	- A test class is declared as (example):
	
			[TestClass]
		    public partial class DataGridUnitTest_Customer_GenericEnumerable_0 : DataGridUnitTest_Unrestricted<Customer, GenericEnumerable_0<Customer>>
			{
			}
		
		The parent type is constructed as:
		
			PARTITION < DATATYPE , DATASOURCE < DATATYPE > >
			
		So, for the Unrestricted partition, using Customer as the data type, and the zero-length generic enumerable data source:
		
			DataGridUnitTest_Unrestricted<Customer, GenericEnumerable_0<Customer>>
			
		Test classes have already been created for the existing combinations.  Unless a new partition is created, or a new test
		class or data source, you shouldn't need to add anything here.
		
	- To avoid the OOM issue, the set of test classes is conditionally reduced using #if blocks.  For example: TestZero (for tests
		involving zero-item sources), TestOne (one-item sources), etc.  To include these test classes, either set the definition 
		in the build config, or just negate the #if condition.  Blocks have not be included for all test classes -- just those that
		are at this stage disabled.  For a MUCH larger test run (after the OOM issue is resolved), all these blocks could be enabled.

