#ifndef __PLANT_SPECIES_h__
#define __PLANT_SPECIES_h__

#include <BranchPrototype.h>

class PlantSpecies
{
public:
	PlantSpecies(const char* path);
	~PlantSpecies();

	/// Generate a prototype copy to store as an editable tree in a SOP_Branch
	BranchPrototype*         copyPrototypeFromList(float lambda, float determ);

private:
	PrototypeSet* prototypeSet; // TODO Make unique pointer
};

#endif