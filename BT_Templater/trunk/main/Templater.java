//----------------------------------------------------------------------------
// Templater.java
// ---------
// Main class for the templater.
//----------------------------------------------------------------------------

package main;

import units.*;
import crits.*; 	// This only needs to be here while we're debugging.

public class Templater {
   
   public static PartData pdata = new PartData();

	public static void main(String[] args) {
		Unit test = new Unit();
		
		/*
		UnitSection testSection = new UnitSection();
		Crit crit1 = new Weapon("Some Multi-Crit");
		Crit crit2 = new Weapon("Some Single-Crit");
		testSection.addCrit(crit1,5);
		testSection.addCrit(crit1,6);
		testSection.addCrit(crit2,8);
		System.out.println(testSection.toString());
		*/
		
        test.setReference("TST-1");
        test.setName("Test Unit");
        test.setTonnage(80);
        test.setWalkMP(3);
        test.setJumpMP(2);
        test.setHeatSinks(20);
        test.setEngType(2);
        test.addTech("Radar");
        test.addTech("ECM");
        test.listTechs();
        System.out.println("Name: " + test.getName() + "("
          + test.getReference() + ")");
        System.out.println("Tonnage: " + test.getTonnage());
        System.out.println("Movement: " + test.getWalkMP() + "/"
          + test.getRunMP() + "/" + test.getJumpMP());
        System.out.println("Engine Rating: "
          + test.getEngRating() + "  Engine Tonnage: "
          + test.getEngTonnage());
        System.out.println("HeatSinks: " + test.getHeatSinks());
        test.setArmor("head",5);
        test.addCrit(new Crit("IS.LargeLaser"), "head", 1);
        System.out.println(test.getSection("head"));
        //test.printEngTable();
	} // end main
} // end class Templater
