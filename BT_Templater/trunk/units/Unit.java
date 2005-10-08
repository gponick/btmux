//----------------------------------------------------------------------------
// Unit.java
// ---------
// Master class to be inherited by specific unit types (Mech, Tank, VTOL,
// etc.)
//----------------------------------------------------------------------------

package units;
import java.util.*;

import crits.Crit;

public class Unit {
   private String unitReference; 	// The reference of the unit.
   private String unitName;      	// The full name of the unit.
   private int    unitTonnage;      // Tonnage
   private int    unitWalkMP;       // Walk Movement Points
   private int    unitJumpMP;       // Unit's jump MP.
   private int    unitHeatSinks;    // Number of HeatSinks
   private int    unitEngineType;   // Holds engine type number.
   
   Hashtable<String,UnitSection> sectionList = new Hashtable<String,UnitSection>();
   
   // Holds the unit's special tech listing.
   private Vector<String> unitTechs = new Vector<String>();
      
   // Holds a list of possible special techs.
   String[] techTable =
   { 
     "SearchLight", 
     "ArtemisIV", 
     "FerroFibrous", 
     "EndoSteel",
     "CompositeInternals", 
     "ECM", 
     "Radar", 
     "DoubleHS",
     "MASC", 
     "FlipArms", 
     "C3M", 
     "C3S", 
     "Beagle", 
     "ReinforcedInternals",
     "HardenedArmor", 
     "StealthArmor", 
     "HeavyFerroFibrous", 
     "LaserReflectiveArmor",
     "ReactiveArmor"
   }; // end techTable
   
   // Holds the names of the various engine types.
   String[] engineNameTable =
   { 
     "Standard",
     "Light",
     "XL",
     "XXL",
     "Compact",
     "Large",
     "ICE",
     "Large ICE",
     "Large XL",
     "Large XXL",
     "Clan XL",
     "Clan XXL",
     "Clan Large XL"
   }; // end engineNameTable
   
   double[] engineWeightMulTable =
   {
     1.00, // 0  Standard
     0.75, // 1  Light
     0.50, // 2  XL
     0.30, // 3  XXL
     1.50, // 4  Compact
     1.00, // 5  Large
     2.00, // 6  ICE
     2.00, // 7  Large ICE
     0.50, // 8  Large XL
     0.30, // 9  Large XXL
     0.50, // 10 Clan XL
     0.30, // 11 Clan XXL
     0.30  // 12 Clan Large XXL
   }; // end engineWeightMulTable
   
   // Holds standard engine weights.
   double[] engineTable =
   {  0.5,  0.5,  0.5,  0.5,  1.0,  1.0,  1.0,  1.0,  1.5,  1.5, 
      1.5,  2.0,  2.0,  2.0,  2.5,  2.5,  3.0,  3.0,  3.0,  3.5, 
      3.5,  4.0,  4.0,  4.0,  4.5,  4.5,  5.0,  5.0,  5.5,  5.5,
      6.0,  6.0,  6.0,  7.0,  7.0,  7.5,  7.5,  8.0,  8.5,  8.5,
      9.0,  9.5, 10.0, 10.0, 10.5, 11.0, 11.5, 12.0, 12.0, 13.0,
     13.5, 14.0, 14.5, 15.5, 16.0, 16.5, 17.5, 18.0, 19.0, 19.5,
     20.5, 21.5, 22.5, 23.5, 24.5, 25.5, 27.0, 28.5, 29.5, 31.5,
     33.0, 34.5, 36.5, 38.5, 41.0, 43.5, 46.0, 49.0, 52.5 
   }; // end engineTable
   
   // Default unit constructor.
   public Unit() {
      // Initialize default values.
      unitReference  = "NEW";       // Fill with Something
      unitName       = "New Unit";  // Fill with Something
      unitTonnage    = 20;          // Minimum Legal Mech Tons           
      unitWalkMP     = 0;           // Nothing Just Yet
      unitHeatSinks  = 10;          // Default Heatsink Loadout
      unitEngineType = 0;           // Standard Fusion Engine
      
      // Set up the sections. Need to ponder a better way to do this.
      sectionList.put("head",        new UnitSection(this));
      sectionList.put("leftTorso",   new UnitSection(this));
      sectionList.put("centerTorso", new UnitSection(this));
      sectionList.put("rightTorso",  new UnitSection(this));
      sectionList.put("leftLeg",     new UnitSection(this));
      sectionList.put("rightLeg",    new UnitSection(this));
   } // end constructor unit
   
/*-----------------------------------------------------------*
       Low-level Methods - Direct Variable Get/Set
 *-----------------------------------------------------------*/
   
   // Retrives the unit's tonnage.
   public int getTonnage() {
      return unitTonnage;
   } // end getTonnage
   
   // Sets the unit's tonnage to a new value.
   public void setTonnage(int newTonnage) {
      unitTonnage = newTonnage;
   } // end setTonnage
   
   // Retrives the unit's walk MP.
   public int getWalkMP() {
      return unitWalkMP;
   } // end getWalkMP
   
   // Sets the unit's walk MP/
   public void setWalkMP(int newWalkMP) {
      unitWalkMP = newWalkMP;
   } // end setWalkMP
   
   // Retrieves the unit's number of heatsinks.
   public int getHeatSinks() {
      return unitHeatSinks;
   } // end getHeatSinks
   
   // Sets the unit's number of heatsinks.
   public void setHeatSinks(int newHeatSinks) {
      unitHeatSinks = newHeatSinks;
   } // end setHeatSinks
   
   // Returns the unit's reference.
   public String getReference() {
      return unitReference;
   } // end getReference
   
   // Sets the unit's unit reference.
   public void setReference(String newReference) {
      unitReference = newReference;
   } // end setReference
   
   // Returns the unit's full name.
   public String getName() {
      return unitName;
   } // end getName
   
   // Sets the unit's full name.
   public void setName(String newName) {
      unitName = newName;
   } // end setName
   
   // Returns the unit's jump MP.
   public int getJumpMP() {
      return unitJumpMP;
   } // end getJumpMP
   
   // Sets the unit's jump MP.
   public void setJumpMP(int newJumpMP) {
      unitJumpMP = newJumpMP;
   } // end setJumpMP
   
   // Sets the unit's engine type. See top of class for
   // engine types and their numerical format.
   public void setEngType(int newEngType) {
      unitEngineType = newEngType;
   } // end setEngType
   
   // Given a string, returns a section.
   public UnitSection getSection(String sectionStr) {
      UnitSection section = (UnitSection) sectionList.get(sectionStr);
      return section;
   } // end UnitSection
   
/*-----------------------------------------------------------*
   Mid-Level Methods - Requires some computation
 *-----------------------------------------------------------*/
   
   // Returns the unit's engine rating.
   public int getEngRating() {
      return unitTonnage * unitWalkMP;
   } // end getEngRating
   
   // Returns the unit's engine tonnage.
   public double getEngTonnage() {
      // Holds the array key to grab from.
      int arrayRow = (this.getEngRating() / 5) - 2;
      // Engine Type Multiplier Storage
      double tonMul;
      
      // Figure out how to modify standard tonnage based on engine type.
      tonMul = engineWeightMulTable[unitEngineType];
      
      // Engine Tonnage * Engine Type Multiplier
      return (engineTable[arrayRow] * tonMul);
   } // end getEngTonnage
   
   // Returns the unit's running MP.
   public int getRunMP() {
      return (int) Math.round(unitWalkMP * 1.5);
   } // end getRunMP
   
   // Sets the amount of armor on a particular section.
   public void setArmor(String location, int armorValue) {
	   UnitSection temp = (UnitSection) sectionList.get(location);
	   temp.setArmor(armorValue);
   } // end setArmor
   
   // Retrieves the armor value of a particular section.
   public int getArmor(String location) {
	   UnitSection temp = (UnitSection) sectionList.get(location);
	   return temp.getArmor();
   } // end getArmor

/*-----------------------------------------------------------*
   Part Adding/Removing
*-----------------------------------------------------------*/
   
   // Adds a the specified crit to the specified section/critNum.
   public void addCrit(Crit newCrit, String sectionStr, int critNum) {
      UnitSection section = (UnitSection) sectionList.get(sectionStr);
      section.addCrit(newCrit, critNum);
   } // end addCrit
   
/*-----------------------------------------------------------*
   Special Tech Management Functions
 *-----------------------------------------------------------*/
   
   // Adds a special tech to Vector unitTechs.
   public void addTech(String techToAdd) {
      if (unitTechs.indexOf(techToAdd) == -1) {
         unitTechs.add(techToAdd);
      }
   } // end addTech
   
   // Returns true if the unit has the specified tech.
   public boolean hasTech(String techToFind) {
	   if (unitTechs.indexOf(techToFind) != -1) {
		   return true;
	   } else {
		   return false;
	   }
   } // end hasTech
   
 /*-----------------------------------------------------------*
   Private Helper Functions
 *-----------------------------------------------------------*/
   
/*-----------------------------------------------------------*
   Debug Functions
 *-----------------------------------------------------------*/   
   
   // Prints the contents of the engine rating/size table.
   // Debugging function.
   public void printEngTable() {
      for(int x = 0; x < engineTable.length; x++) {
         System.out.println(((x * 5) + 10) + "\t" 
               + engineTable[x] + "\t"
               + engineTable[x] * .75 + "\t"
               + engineTable[x] * .50 + "\t"
               + engineTable[x] * .25);
      } // end for
   } // end printEngTable
   
   // Shows a string representation of the unit's techs.
   public void listTechs() {
	   System.out.println("TECHS: " + unitTechs.toString());
   } // end listTechs
} // end class Unit
