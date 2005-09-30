//----------------------------------------------------------------------------
// Crit.java
// ---------
// Generic crit class.
//----------------------------------------------------------------------------

public class Crit {
	public static int totalCrits;		// Total number of crits.
				  int critNumber;		// Unique crit number.
			   String critName;			// The name of the item in the crit.
	
	// Default constructor, shouldn't be used.
	Crit() {
		critName = "Unknown";
		critNumber = totalCrits;
		totalCrits++;
	} // end constructor Crit
	
	// Constructor with name argument.
	Crit(String name) {
		critName = name;
		critNumber = totalCrits;
		totalCrits++;
	} // end constructor Crit
	
	public String toString() {
		return critName + "(" + critNumber + ")";
	}
} // end class Crit
