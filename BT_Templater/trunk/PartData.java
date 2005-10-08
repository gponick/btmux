//----------------------------------------------------------------------------
// PartData.java
// ---------
// Holds information about each part for retrieval in calculations. This is
// insanely messy, but we'll worry about it later :)
//----------------------------------------------------------------------------

package crits;

import java.util.Hashtable;

public class PartData {
	static Hashtable partInfo = new Hashtable();
	
	public PartData() {
		// Damage, Heat, MinRange, ShortRange, MediumRange, LongRange, Crits,
		// AmmoPT, Weight, BV
		partInfo.put("IS.LargeLaser", new part(8, 8, 0, 5, 10, 15, 2, -1, 5, 124));
		partInfo.put("IS.MediumLaser", new part(5, 3, 0, 3, 6, 9, 1, -1, 1, 46));
	}
	
	public static int getDamage(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.damage;
	}
	
	public static int getHeat(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.heat;
	}
	
	public static int getMinRange(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.minRange;
	}
	
	public static int getShortRange(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.shortRange;
	}
	
	public static int getMediumRange(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.mediumRange;
	}
	
	public static int getLongRange(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.longRange;
	}
	
	public static int getCrits(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.crits;
	}
	
	public static int getAmmoPerTon(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.ammoPerTon;
	}
	
	public static float getWeight(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.weight;
	}
	
	public static int getBattleValue(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.battleValue;
	}
	
	class part {
		public int damage, heat, minRange, shortRange, mediumRange, longRange;
		public int crits, ammoPerTon, weight, battleValue;
		
		part(int damage, int heat, int minRange, int shortRange, int mediumRange,
				int longRange, int crits, int ammoPerTon, int weight,
				int battleValue) {
			this.damage = damage;
			this.heat = heat;
			this.minRange = minRange;
			this.shortRange = shortRange;
			this.mediumRange = mediumRange;
			this.longRange = longRange;
			this.crits = crits;
			this.ammoPerTon = ammoPerTon;
			this.weight = weight;
			this.battleValue = battleValue;
		} // end constructor part
	} // end class part
} // end class PartData
