//----------------------------------------------------------------------------
// Crit.java
// ---------
// Generic crit class.
//----------------------------------------------------------------------------

package crits;

import main.Templater;

public class Crit {
	public static int totalCrits;		// Total number of crits.
				  int  critNumber;		// Unique crit number. This is used
				  						      // to group together multi-crit stuff.
			   String critName;			// The name of the item in the crit.
			   int    critType;			// The type of crit. (0 = Part, 1 = Weap, 2 = Ammo)
	
	// Default constructor, shouldn't be used.
	public Crit() {
		critName = "Unknown";
		critNumber = totalCrits;
		totalCrits++;
	} // end constructor Crit
	
	// Constructor with name argument.
	public Crit(String name) {
		critName = name;
		critType = Templater.pdata.getType(name);
		critNumber = totalCrits;
		totalCrits++;
	} // end constructor Crit
	
   public String getName() {
      return this.critName;
   }
   
   public int getNumber() {
      return this.critNumber;
   }
   
   public int getDamage() {
      return Templater.pdata.getDamage(critName);
   }
   
   public int getHeat() {
      return Templater.pdata.getHeat(critName);
   }
   
   public int getMinRange() {
      return Templater.pdata.getMinRange(critName);
   }
   
   public int getShortRange() {
      return Templater.pdata.getShortRange(critName);
   }
   
   public int getMediumRange() {
      return Templater.pdata.getMediumRange(critName);
   }
   
   public int getLongRange() {
      return Templater.pdata.getLongRange(critName);
   }
   
   public int getNumCrits() {
      return Templater.pdata.getNumCrits(critName);
   }
   
   public int getAmmoPerTon() {
      return Templater.pdata.getAmmoPerTon(critName);
   }

   public float getWeight() {
      return Templater.pdata.getWeight(critName);
   }
   
   public int getBattleValue() {
      return Templater.pdata.getBattleValue(critName);
   }
   
   public int getType() {
      return this.critType;
   }
   
   public int getCost() {
      return Templater.pdata.getCost(critName);
   }
  
	public String toString() {
		return critName + "(#"  + critNumber + ")";
	} // end toString
} // end class Crit
