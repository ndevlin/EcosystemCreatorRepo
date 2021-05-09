#include "PlantType.h"


PlantType::PlantType(const char* path)
	: prototypeSet(new PrototypeSet(path))
{}


PlantType::~PlantType()
{}

// Generate a prototype copy for editing in branch node
BranchPrototype* PlantType::copyPrototypeFromList(float lambda, float determ) {
	return prototypeSet->selectNewPrototype(lambda, determ);
}