//----------------------------------------------------------------------------
// PartData.java
// ---------
// Holds information about each part for retrieval in calculations. This is
// insanely messy, but we'll worry about it later :)
//----------------------------------------------------------------------------

package crits;
import java.util.Hashtable;

public class PartData {
   // Holds the part hashtable.
	public static Hashtable<String,part> partInfo = new Hashtable<String,part>();
   
   // Aliases for Tech Type.
   final int IS = 0;
   final int CL = 1;
   
   // Aliases for Weapon Type.
   final int LASER     = 0;
   final int MISSILE   = 1;
   final int BALLISTIC = 2;
   final int ARTILLERY = 3;
	
	public PartData() {
		// Cost, BV, TechLevel Damage, Heat, MinRange, ShortRange, MediumRange, 
      // LongRange, Crits, AmmoPT, Weight
      
      // Weapon Data                                     Cost   BV  TL  TT  DG  HT  M  S   M   L  C   APT         Weight   Type
      partInfo.put("IS.ERPPC",             new part(     500,   46,  2, IS, 10, 15, 0, 7, 14, 23, 3,   -1,          7,     LASER));
      partInfo.put("IS.ERLargeLaser",      new part(     500,  124,  2, IS,  8, 12, 0, 7, 14, 19, 2,   -1,          5,     LASER));
      partInfo.put("IS.ERMediumLaser",     new part(     500,  124,  2, IS,  5,  5, 0, 4,  8, 12, 1,   -1,          1,     LASER));
      partInfo.put("IS.ERSmallLaser",      new part(     500,  124,  2, IS,  3,  2, 0, 2,  4,  5, 1,   -1, (float)0.5,     LASER));
      partInfo.put("IS.LargePulseLaser",   new part(     500,   46,  2, IS,  9, 10, 0, 3,  7, 10, 2,   -1,          7,     LASER));
      partInfo.put("IS.MediumPulseLaser",  new part(     500,   46,  2, IS,  6,  4, 0, 2,  4,  6, 1,   -1,          2,     LASER));
      partInfo.put("IS.SmallPulseLaser",   new part(     500,   46,  2, IS,  3,  2, 0, 1,  2,  3, 1,   -1,          1,     LASER));
      partInfo.put("IS.LargeLaser",        new part(     500,  124,  1, IS,  8,  8, 0, 5, 10, 15, 2,   -1,          2,     LASER));
		partInfo.put("IS.MediumLaser",       new part(     500,   46,  1, IS,  5,  3, 0, 3,  6,  9, 1,   -1,          1,     LASER));
      partInfo.put("IS.SmallLaser",        new part(     500,   46,  1, IS,  3,  1, 0, 1,  2,  3, 1,   -1, (float)0.5,     LASER));
      partInfo.put("IS.PPC",               new part(     500,   46,  1, IS, 10, 10, 3, 6, 12, 18, 3,   -1,          7,     LASER));
	}
	
	public int getDamage(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.damage;
	}
	
	public int getHeat(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.heat;
	}
	
	public int getMinRange(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.minRange;
	}
	
	public int getShortRange(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.shortRange;
	}
	
	public int getMediumRange(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.mediumRange;
	}
	
	public int getLongRange(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.longRange;
	}
	
	public int getNumCrits(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.crits;
	}
	
	public int getAmmoPerTon(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.ammoPerTon;
	}
	
	public float getWeight(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.weight;
	}
	
	public int getBattleValue(String partName) {
		part partTemp = (part) partInfo.get(partName);
		return partTemp.battleValue;
	}
   
   public int getType(String partName) {
      part partTemp = (part) partInfo.get(partName);
      return partTemp.type;
   }
	
   public int getCost(String partName) {
      part partTemp = (part) partInfo.get(partName);
      return partTemp.cost;
   }
   
   public int getTechLevel(String partName) {
      part partTemp = (part) partInfo.get(partName);
      return partTemp.techLevel;
   }
   
   // Returns an integer representation of a weapon's type.
   public int getWeaponTypeNum(String partName) {
      part partTemp = (part) partInfo.get(partName);
      return partTemp.weaponType;
   }
   
   // Returns the string representation of a weapon's type.
   public String getWeaponTypeStr(String partName) {
      int weaponType;        // Holder for Weapon Type Number
      part partTemp = (part) partInfo.get(partName);
      
      weaponType = partTemp.weaponType;
      
      switch(weaponType) {
         case LASER:
            return "Laser";
         case MISSILE:
            return "Missile";
         case BALLISTIC:
            return "Ballistic";
         case ARTILLERY:
            return "Artillery";
      } // end switch
      
      // Default case
      return "Unknown";
   } // end getWeaponTypeStr
   
	class part {
      public int type;     // 0 = Part, 1 = Weapon, 2 = Ammo
      public int techType; // 0 = IS, 1 = Clan
		public int cost, battleValue, techLevel, damage, heat, minRange, shortRange;
      public int mediumRange, longRange, crits, ammoPerTon, weaponType;
      float weight;

      // Generic Part (Type 0) Constructor
      part(int cost, int battleValue, int techLevel, int techType) {
         this.type = 0;
         this.techType = techType;
         this.cost = cost;
         this.battleValue = battleValue;
         this.techLevel = techLevel;
      }
		
      // Weapon (Type 1) Constructor
		part(int cost, int battleValue, int techLevel, int techType, int damage, int heat, 
            int minRange, int shortRange, int mediumRange, int longRange, int crits, 
            int ammoPerTon, float weight, int weaponType) {
         this.type = 1;
         this.techType = techType;
         this.weaponType = weaponType;
         this.cost = cost;
         this.battleValue = battleValue;
         this.techLevel = techLevel;
			this.damage = damage;
			this.heat = heat;
			this.minRange = minRange;
			this.shortRange = shortRange;
			this.mediumRange = mediumRange;
			this.longRange = longRange;
			this.crits = crits;
			this.ammoPerTon = ammoPerTon;
			this.weight = weight;
		} // end constructor part
      
      // Ammo (Type 2) Constructor
      part(int cost, int battleValue, int techLevel, int techType, int ammoPerTon) {
         this.type = 2;
         this.techType = techType;
         this.cost = cost;
         this.battleValue = battleValue;
         this.techLevel = techLevel;
         this.ammoPerTon = ammoPerTon;
      }
      
	} // end class part
} // end class PartData
