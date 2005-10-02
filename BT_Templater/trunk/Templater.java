//----------------------------------------------------------------------------
// Templater.java
// ---------
// Main class for the templater.
//----------------------------------------------------------------------------

public class Templater {

	public static void main(String[] args) {
		Unit test = new Unit();
		PartData pdata = new PartData();
		PartData.getWeight("IS.MediumLaser");
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
        System.out.println(test.getArmor("head"));
        //test.printEngTable();
	} // end main
} // end class Templater
