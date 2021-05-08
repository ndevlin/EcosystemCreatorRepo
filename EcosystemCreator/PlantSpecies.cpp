#include "PlantSpecies.h"


PlantSpecies::PlantSpecies(const char* path)
	: prototypeSet(new PrototypeSet(path))
{}


PlantSpecies::~PlantSpecies()
{}

/// Generate a prototype copy to store as an editable tree in a SOP_Branch
BranchPrototype* PlantSpecies::copyPrototypeFromList(float lambda, float determ) {
	return prototypeSet->selectNewPrototype(lambda, determ);
}