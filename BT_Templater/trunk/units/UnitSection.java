//----------------------------------------------------------------------------
// UnitSection.java
// ----------------
// Stores information regarding sections of a unit. Holds the contents of
// crits and armor values.
//----------------------------------------------------------------------------

package units;
import crits.Crit;

public class UnitSection {
	private int	armor;				// Number of armor points in this section.
	private int rearArmor;			// In case the section has rear armor.
	Crit[]		sectionCrits;		// The array of crits in the section.
	Unit		   unit;				   // Holds reference to the unit.
	
	// Default default constructor. Don't use it!
	UnitSection() {
		// Assume a 12-crit section.
		sectionCrits = new Crit[12];
		unit = null;
	} // end constructor UnitSection
	
	// Default constructor
	UnitSection(Unit inUnit) {
		// Assume a 12-crit section.
		sectionCrits = new Crit[12];
		unit = inUnit;
	} // end constructor UnitSection
	
	// Constructor with argument for variable crit count
	UnitSection(Unit inUnit, int numCrits) {
		// Allow setting of an arbitrary number of crits.
		sectionCrits = new Crit[numCrits];
		unit = inUnit;
	} // end constructor UnitSection
	
/*-----------------------------------------------------------*
   Low-level Methods - Direct Variable Get/Set
 *-----------------------------------------------------------*/
	
	// Returns the amount of armor in this section.
	public int getArmor() {
		return armor;
	} // end getArmor
	
	// Sets the section's armor level.
	public void setArmor(int newArmorVal) {
		armor = newArmorVal;
	} // end getArmor
	
	// Returns the amount of rear armor in this section.
	public int getRearArmor() {
		return rearArmor;
	} // end getRearArmor
	
	// Sets the section's rear armor level.
	public void setRearArmor(int newRearArmorVal) {
		rearArmor = newRearArmorVal;
	} // end getRearArmor

/*-----------------------------------------------------------*
	Mid-Level Methods
 *-----------------------------------------------------------*/
	
	// Adds a crit to the section's crit array. Currently just
	// over-writes existing crits if you specify an occupied.
	public void addCrit(Crit newCrit, int location) {
		int numCrits = newCrit.getNumCrits();
      
      for (int x = 0; x < numCrits; x++) {
         sectionCrits[location + x] = newCrit;
      }
	} // end addCrit
   
   public void setCrit(Crit newCrit, int location) {
      sectionCrits[location] = newCrit;
   } // end setCrit
	
	// Returns a string representation of the section.
	public String toString() {
		String output;		// Output to return.
		
		output = "------------------------------\n" +
				 " Section Overview | Armor: " + armor + "\n" +
				 "------------------------------";
				 
		// Go through the section and show the crits.
		for (int x = 0; x < sectionCrits.length; x++) {
			output += "\n" + x + " " + sectionCrits[x];
		} // end for
				 
		output += "\n------------------------------";
		
		return output;
	} // end toString
	
/*-----------------------------------------------------------*
   Main Method - For Testing Only
 *-----------------------------------------------------------*/
	
	public static void main(String[] args) {
      UnitSection test = new UnitSection();
      test.addCrit(new Crit("IS.LargeLaser"), 5);
      test.addCrit(new Crit("IS.MediumLaser"), 8);
      System.out.println(test);
	} // end main
} // end class UnitSection
