//----------------------------------------------------------------------------
// Crit.java
// ---------
// Generic crit class.
//----------------------------------------------------------------------------

package crits;

public class Crit {
	public static int totalCrits;		// Total number of crits.
				  int critNumber;		// Unique crit number. This is used
				  						// to group together multi-crit stuff.
			   String critName;			// The name of the item in the crit.
			   String critType;			// The type of crit.
	
	// Default constructor, shouldn't be used.
	Crit() {
		critName = "Unknown";
		critNumber = totalCrits;
		totalCrits++;
	} // end constructor Crit
	
	// Constructor with name argument.
	Crit(String name, String type) {
		critName = name;
		critType = type;
		critNumber = totalCrits;
		totalCrits++;
	} // end constructor Crit
	
	public float getWeight() {
		return PartData.getWeight(critName);
	}
	
	public String toString() {
		return critName + "("+ critType + " #"  + critNumber + ")";
	} // end toString
} // end class Crit
